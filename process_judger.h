#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
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

class ProcessJudger
{
public:
    ProcessJudger():
        input_file(NULL),
        output_file(NULL),
        time_limit(1),
        mem_limit(128),
        cmd_line(NULL)
    {}

    ~ProcessJudger() {}

    static void print_help();
    static void my_alarm_handler(int);
    int parse_arg(int argc, char** argv);
    int judge (char* inFile, char* outFile, int timeLimit, int memLimit, char** cmdLine);
    void printResult(int status, struct rusage childRusage);

private:
    ProcessJudger(const ProcessJudger&);
    ProcessJudger& operator=(const ProcessJudger& rhs);

    int status2result(int status);
    int redirect(int* inFd, int* outFd, char* inFile, char* outFile);

    struct rlimit tLimit, mLimit, fileLimit, cldLimit, fsizeLimit, dataLimit;
    char* input_file;
    char* output_file;
    int time_limit;
    int mem_limit;
    char** cmd_line;

    struct PROCESS_CONSTRAINT
    {
        const static int MIN_TIME_LIMIT = 1;
        const static int MAX_TIME_LIMIT = 100;
        const static int MIN_MEM_LIMIT = 4;
        const static int MAX_MEM_LIMIT = 1024;
        const static int FILE_SIZE_LIMIT = 20 * 1024 * 1024;
        const static int FILE_NUM_LIMIT = 6;
        const static int CHILD_NUM_LIMIT = 0;
    };

    enum PROCESS_RETURN_ENUM
    {
        NOMRAL_RETURN = 0,
        TIME_LIMIT_EXCEED,
        MEM_LIMIT_EXCEED,
        RUNTIME_ERROR,
        OUTPUT_LIMIT_EXCEED,
        SYSTEM_ERROR
    };

    static int verbose_flag;
    const static struct option long_options[];
};
