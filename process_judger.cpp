#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

enum PROCESS_RETURN_ENUM
{
    NOMRAL_RETURN = 0,
    TIME_LIMIT_EXCEED,
    MEM_LIMIT_EXCEED,
    RUNTIME_ERROR,
    OUTPUT_LIMIT_EXCEED,
    SYSTEM_ERROR
};


//time和memory参数的范围
#define MIN_TIME_LIMIT 1
#define MAX_TIME_LIMIT 100
#define MIN_MEM_LIMIT 4
#define MAX_MEM_LIMIT 1024

#define FILE_SIZE_LIMIT (20 * 1024 * 1024)
#define FILE_NUM_LIMIT 6
#define CHILD_NUM_LIMIT 0


struct rlimit tLimit, mLimit, fileLimit, cldLimit, fsizeLimit, dataLimit;

void my_alarm_handler(int);
int judge (char* inFile, char* outFile, int timeLimit, int memLimit, char** cmdLine);
int redirect(int* inFd, int* outFd, char* inFile, char* outFile);
int status2result(int status);
void printResult(int status, struct rusage childRusage);

/*
 *  @parameter
 *      -i input file path
 *      -o output file path
 *      -t process time(second) limit
 *      -m process memory(Mb) limit
 *      -c commend line as a string format
 *
 *  @return
 *      PROCESS_RETURN_ENUM value
 *      NORMAL_RETURN(0), or
 *      TIME_LIMIT_EXCEED(2) / MEM_LIMIT_EXCEED(3) / RUNTIME_ERROR(4) / OUTPUT_LIMIT_EXCEED(5) / SYSTEM_ERROR(6) 
 */

int main (int argc, char** argv)
{
    if (argc < 6) {
        printf("argument number error!\n");
        printf("usage: inputFile outputFile timeLimt(%d~%ds) memoryLimit(%d~%dMb) commandLine\n", 
                MIN_TIME_LIMIT, MAX_TIME_LIMIT, MIN_MEM_LIMIT, MAX_MEM_LIMIT);
        return SYSTEM_ERROR;
    }
    int timeLimit = 0, memLimit = 0, ret = 0, tmpFd = 0;
    timeLimit = atoi(argv[3]);  
    if (timeLimit < MIN_TIME_LIMIT || timeLimit > MAX_TIME_LIMIT) {
        printf("time limit argument error!(%d~%ds)\n", MIN_TIME_LIMIT, MAX_TIME_LIMIT);
        return SYSTEM_ERROR;
    }
    memLimit = atoi(argv[4]);
    if (memLimit < MIN_MEM_LIMIT || memLimit > MAX_MEM_LIMIT) {
        printf("memory limit argument error!(%d~%dMb)\n", MIN_MEM_LIMIT, MAX_MEM_LIMIT);
        return SYSTEM_ERROR;
    }
    memLimit = memLimit * 1024 * 1024;

    if ((tmpFd=open(argv[1], O_RDONLY)) == -1) {        //check input file
        printf("input file doesn't exist!\n");
        return SYSTEM_ERROR;
    }
    close(tmpFd);
    if ((tmpFd=open(argv[2], O_RDWR | O_CREAT)) == -1) { //check output file
        printf("output file can't be created!\n");
        return SYSTEM_ERROR;
    }
    close(tmpFd);

    ret = judge(
            argv[1],        //input file 
            argv[2],        //output file
            timeLimit, 
            memLimit,
            argv+5);        //command line

    return ret;

}


int judge (char* inFile, char* outFile, int timeLimit, int memLimit, char** cmd)
{
    int i;
#ifdef  MY_DEBUG
    printf("input file: %s\n", inFile);
    printf("output file: %s\n", outFile);
    printf("time limit: %d\n", timeLimit);
    printf("memory limit: %d\n", memLimit);
    printf("cmd: ");
    for (i = 0; cmd[i] != NULL; i ++)
        printf("%s ", cmd[i]);
    printf("\n");
#endif
    pid_t pid;
    int s;
    struct rusage childRusage;
    int inFd, outFd;

    if ((pid=fork()) < 0) {
        printf("fork error!\n");
        return SYSTEM_ERROR;
    }
    else if ( pid == 0 ) {       /*child process*/
        getrlimit(RLIMIT_CPU, &tLimit);
        getrlimit(RLIMIT_AS, &mLimit);
        getrlimit(RLIMIT_NOFILE, &fileLimit);
        getrlimit(RLIMIT_NPROC, &cldLimit);
        getrlimit(RLIMIT_DATA, &dataLimit);
        getrlimit(RLIMIT_FSIZE, &fsizeLimit);

//      printf("file size: %d\n", fsizeLimit.rlim_cur);
//      printf("data limit:%d\n", dataLimit.rlim_cur);
        tLimit.rlim_cur = timeLimit;
        mLimit.rlim_cur = memLimit;
        fileLimit.rlim_cur = FILE_NUM_LIMIT;
        cldLimit.rlim_cur = CHILD_NUM_LIMIT;
        fsizeLimit.rlim_cur = FILE_SIZE_LIMIT;

        if (setrlimit(RLIMIT_CPU, &tLimit) == -1) {     /*时间限制*/
            printf("setrlimit time limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_AS, &mLimit) == -1) {     /*内存限制*/
            printf("setrlimit memory limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_NOFILE, &fileLimit) == -1) {
            printf("setrlimit file num limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_NPROC, &cldLimit) == -1) {
            printf("setrlimit child process num limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_FSIZE, &fsizeLimit) == -1)  {
            printf("setrlimti file size limit error!\n");
            return SYSTEM_ERROR;
        }

        if (redirect(&inFd, &outFd, inFile, outFile)) {
            printf("redirect error!\n");
            return SYSTEM_ERROR;
        }

        /*因为程序中的sleep()不计入进程时间，所以用信号作为辅助超时判断*/
        signal(SIGALRM, my_alarm_handler);
        alarm(timeLimit * 2);   /*为了能与真正超时区别开来，故设置为时限的两倍*/

        execvp(cmd[0], cmd);
    }
    else
    {
        waitpid(pid, &s, 0);
        getrusage(RUSAGE_CHILDREN, &childRusage);
        printResult(s, childRusage);
        return status2result(s);
    }

    return 0;
}

void my_alarm_handler(int x)
{
    printf("time out!\n");
}

int redirect (int* pInFd, int* pOutFd, char* inFile, char* outFile)
{
    *pInFd = open(inFile, O_RDONLY);
    if (*pInFd < 0) {
        printf("inFd open error!\n%s\n",strerror(errno));
        return 1;
    }
    *pOutFd = open(outFile, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU | S_IRGRP | S_IROTH);
    if (*pOutFd < 0) {
        printf("outFd open error!\n%s\n", strerror(errno));
        return 1;
    }

    /*输入输出重定向*/
    if (dup2(*pOutFd, STDOUT_FILENO) < 0) {
        printf("outFd dup2 error!\n");
        return 1;
    }       
    if (dup2(*pInFd, STDIN_FILENO) < 0) {
        printf("inFd dup2 error!\n");
        return 1;
    }

    return 0;
}

int status2result (int status)
{
    if (WIFEXITED(status))
        return NOMRAL_RETURN;
    if (SIGALRM == status ||SIGXCPU == status)
        return TIME_LIMIT_EXCEED;
    if (SIGSEGV == status)
        return MEM_LIMIT_EXCEED;
    if (SIGXFSZ == status)
        return OUTPUT_LIMIT_EXCEED;
    return RUNTIME_ERROR;
}

void printResult (int status, struct rusage childRusage)
{
    double passTime;

    passTime = (childRusage.ru_utime.tv_sec + childRusage.ru_stime.tv_sec) * 1000 + (float)(childRusage.ru_stime.tv_usec + childRusage.ru_utime.tv_usec) / 1000;
    printf("time: %d ms\n", (int)passTime);
    printf("memory: %lu kb\n", childRusage.ru_maxrss);

    printf("=====================================================\n");
    printf("child exit status: %d\n", status);
    if (WIFEXITED(status)) {
        printf("exit normally.\n");
    } else {
        printf("exit abnormally.\n");
    }
    if (WIFSIGNALED(status)) {
        printf("exit by signal: %d | %d\n", status, WTERMSIG(status));
    }

    printf("child process time: %d ms\n", static_cast<int>(passTime));
    printf("maximum resident set size:\t%ld\n", childRusage.ru_maxrss);
    printf("integral shared memory size:\t%ld\n", childRusage.ru_ixrss);
    printf("integral unshared data size:\t%ld\n", childRusage.ru_idrss);
    printf("integral unshared stack size:\t%ld\n", childRusage.ru_isrss);
    printf("page reclaims:\t%ld\n", childRusage.ru_minflt);
    printf("page faults\t%ld\n", childRusage.ru_majflt);
    printf("swaps:\t%ld\n", childRusage.ru_nswap);
    printf("block input operations:\t%ld\n", childRusage.ru_inblock);
    printf("block output operations:\t%ld\n", childRusage.ru_oublock);
    printf("messages sent:\t%ld\n", childRusage.ru_msgsnd);
    printf("messages received:\t%ld\n", childRusage.ru_msgrcv);
    printf("signals received:\t%ld\n", childRusage.ru_nsignals);
    printf("voluntary context switches:\t%ld\n", childRusage.ru_nvcsw);
    printf("involuntary context switches:\t%ld\n", childRusage.ru_nivcsw);
    printf("===================================================\n\n");
}
