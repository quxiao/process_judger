# Process Judger

[![Build Status](https://travis-ci.org/quxiao/process_judger.png?branch=master)](https://travis-ci.org/quxiao/process_judger)

A program which can run other process under some time and memory constraints. It's been used as a tool for algorithm programming contest training platform in NJUST.

## Usage 

    ./process_judger [options] command

    options:
    -h --help       print help
    -i --input      input file path
    -o --output     output file path
    -t --time       process time(second) limit
    -m --memory     process memory(Mb) limit

## Example
    
    ./process_judger -i input.data -i output.data -t 1 -m 4 ./my_exe
