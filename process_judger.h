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
        input_file_(NULL),
        output_file_(NULL),
        time_limit_(1),
        mem_limit_(128),
        cmd_line_(NULL)
    {}

    ~ProcessJudger() {}

    int ParseArg(int argc, char** argv);
    int Judge();

private:
    ProcessJudger(const ProcessJudger&);
    ProcessJudger& operator=(const ProcessJudger& rhs);

    int StatusToResult(int status);
    int Redirect(int* inFd, int* outFd, char* inFile, char* outFile);

    struct rlimit t_limit_, m_limit_, file_limit_, cld_limit_, fsize_limit_, data_limit_;
    char* input_file_;
    char* output_file_;
    int time_limit_;
    int mem_limit_;
    char** cmd_line_;

    static void PrintHelp();
    static void VoidHandler(int);
    void PrintResult(int status, struct rusage childRusage);

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

    static int verbose_flag_;
    const static struct option long_options[];
};
