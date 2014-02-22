#include "process_judger.h"

/*
 *  @parameter
 *      -h --help       print help
 *      -i --input      input file path
 *      -o --output     output file path
 *      -t --time       process time(second) limit
 *      -m --memory     process memory(Mb) limit
 *      cmd args
 *
 *  @return
 *      PROCESS_RETURN_ENUM value
 *      NORMAL_RETURN(0), or
 *      TIME_LIMIT_EXCEED(2) / MEM_LIMIT_EXCEED(3) / RUNTIME_ERROR(4) / OUTPUT_LIMIT_EXCEED(5) / SYSTEM_ERROR(6) 
 */

int main (int argc, char** argv)
{
    ProcessJudger judger;
    if (judger.ParseArg(argc, argv)) {
        printf("ParseArg failed!\n");
        return -1;
    }

    return judger.Judge();
}
