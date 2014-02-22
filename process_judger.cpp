#include "process_judger.h"


int ProcessJudger::Judge()
{
    if (verbose_flag_) {
        printf("input file: %s\n", input_file_);
        printf("output file: %s\n", output_file_);
        printf("time limit: %d\n", time_limit_);
        printf("memory limit: %d\n", mem_limit_);
        printf("cmd: ");
        if (cmd_line_) {
            for (int i = 0; cmd_line_[i] != NULL; i ++)
                printf("%s ", cmd_line_[i]);
            printf("\n");
        }
    }
    pid_t pid;
    int s;
    struct rusage childRusage;
    int inFd, outFd;

    if ((pid=fork()) < 0) {
        printf("fork error!\n");
        return SYSTEM_ERROR;
    }
    else if (pid == 0) {       /*child process*/
        getrlimit(RLIMIT_CPU, &t_limit_);
        getrlimit(RLIMIT_AS, &m_limit_);
        getrlimit(RLIMIT_NOFILE, &file_limit_);
        getrlimit(RLIMIT_NPROC, &cld_limit_);
        getrlimit(RLIMIT_DATA, &data_limit_);
        getrlimit(RLIMIT_FSIZE, &fsize_limit_);

//      printf("file size: %d\n", fsize_limit_.rlim_cur);
//      printf("data limit:%d\n", data_limit_.rlim_cur);
        t_limit_.rlim_cur = time_limit_;
        m_limit_.rlim_cur = mem_limit_;
        file_limit_.rlim_cur = PROCESS_CONSTRAINT::FILE_NUM_LIMIT;
        cld_limit_.rlim_cur = PROCESS_CONSTRAINT::CHILD_NUM_LIMIT;
        fsize_limit_.rlim_cur = PROCESS_CONSTRAINT::FILE_SIZE_LIMIT;

        if (setrlimit(RLIMIT_CPU, &t_limit_) == -1) {     /*时间限制*/
            printf("setrlimit time limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_AS, &m_limit_) == -1) {     /*内存限制*/
            printf("setrlimit memory limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_NOFILE, &file_limit_) == -1) {
            printf("setrlimit file num limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_NPROC, &cld_limit_) == -1) {
            printf("setrlimit child process num limit error!\n");
            return SYSTEM_ERROR;
        }
        if (setrlimit(RLIMIT_FSIZE, &fsize_limit_) == -1)  {
            printf("setrlimti file size limit error!\n");
            return SYSTEM_ERROR;
        }

        if (Redirect(&inFd, &outFd, input_file_, output_file_)) {
            printf("redirect error!\n");
            return SYSTEM_ERROR;
        }

        /*因为程序中的sleep()不计入进程时间，所以用信号作为辅助超时判断*/
        signal(SIGALRM, VoidHandler);
        alarm(time_limit_ * 2);   /*为了能与真正超时区别开来，故设置为时限的两倍*/

        execvp(cmd_line_[0], cmd_line_);
    }
    else
    {
        waitpid(pid, &s, 0);
        getrusage(RUSAGE_CHILDREN, &childRusage);
        PrintResult(s, childRusage);
        return StatusToResult(s);
    }

    return 0;
}

void ProcessJudger::VoidHandler(int x)
{
    printf("time out!\n");
}

int ProcessJudger::Redirect(int* pInFd, int* pOutFd, char* inFile, char* outFile)
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

int ProcessJudger::StatusToResult(int status)
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

void ProcessJudger::PrintResult (int status, struct rusage childRusage)
{
    double passTime;

    passTime = (childRusage.ru_utime.tv_sec + childRusage.ru_stime.tv_sec) * 1000 
                + (float)(childRusage.ru_stime.tv_usec + childRusage.ru_utime.tv_usec) / 1000;
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


int ProcessJudger::ParseArg(int argc, char** argv)
{
    int c;
    int option_index = 0;

    while (1) {
        c = getopt_long (argc, argv, "i:o:t:m:h",
            long_options, &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        switch (c) {
        case 0:
            /* If this option set a flag, do nothing else now. */
            if (long_options[option_index].flag != 0)
                break;
            break;
        case 'h':
            PrintHelp();
            exit(0);
            break;
        case 'i':
            input_file_ = optarg;
            break;
        case 'o':
            output_file_ = optarg;
            break;
        case 't':
            time_limit_ = atoi(optarg);
            break;
        case 'm':
            mem_limit_ = atoi(optarg);
            break;
        case '?':
            /* getopt_long already printed an error message. */
            return -1;
        default:
            return -1;
        }

    }

    cmd_line_ = argv + optind;

    //check arg
    if (time_limit_ < PROCESS_CONSTRAINT::MIN_TIME_LIMIT || 
        time_limit_ > PROCESS_CONSTRAINT::MAX_TIME_LIMIT) {
        printf("time limit argument error!(%d~%ds)\n", PROCESS_CONSTRAINT::MIN_TIME_LIMIT, PROCESS_CONSTRAINT::MAX_TIME_LIMIT);
        return SYSTEM_ERROR;
    }
    if (mem_limit_ < PROCESS_CONSTRAINT::MIN_MEM_LIMIT || mem_limit_ > PROCESS_CONSTRAINT::MAX_MEM_LIMIT) {
        printf("memory limit argument error!(%d~%dMb)\n", PROCESS_CONSTRAINT::MIN_MEM_LIMIT, PROCESS_CONSTRAINT::MAX_MEM_LIMIT);
        return SYSTEM_ERROR;
    }
    mem_limit_ = mem_limit_ * 1024 * 1024;

    int tmp_fd = NULL;
    if ((tmp_fd=open(input_file_, O_RDONLY)) == -1) {        //check input file
        printf("input file[%s] doesn't exist!\n", input_file_);
        return SYSTEM_ERROR;
    }
    close(tmp_fd);
    if ((tmp_fd=open(output_file_, O_RDWR | O_CREAT)) == -1) { //check output file
        printf("output file[%s] can't be created!\n", output_file_);
        return SYSTEM_ERROR;
    }
    close(tmp_fd);

    return 0;
}

void ProcessJudger::PrintHelp()
{
    printf(
        "This is a program which can run other process under some time and memory constraints.\n"
        "Usage: ./process_judger [options] commend\n"
        "options:\n"
        "   -h --help       print help\n"
        "   -i --input      input file path\n"
        "   -o --output     output file path\n"
        "   -t --time       process time(second) limit\n"
        "   -m --memory     process memory(Mb) limit\n"
        "\n"
        "Example:\n"
        "   ./process_judger -i input.data -i output.data -t 1 -m 4 ./my_exe\n"
        );
}

int ProcessJudger::verbose_flag_ = 0;
const struct option ProcessJudger::long_options[] =
{
    {"help",    no_argument,        0,              'h'},
    {"verbose", no_argument,        &verbose_flag_,  1},
    {"input",   required_argument,  0,              'i'},
    {"output",  required_argument,  0,              'o'},
    {"time",    required_argument,  0,              't'},
    {"memory",  required_argument,  0,              'm'},
    {"command", required_argument,  0,              'c'},
    {0, 0, 0, 0}
};

