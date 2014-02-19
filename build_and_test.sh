#!/bin/sh

function clear_and_exit
{
    cd $WORK_BASE
    make clean
    if [ -z $1 ]; then
        exit $1
    else
        exit 0
    fi
}


WORK_BASE=`pwd`
make clean && make
if [ $? -ne 0 ]; then
    echo "make failed!"
    clear_and_exit 1
fi

cd $WORK_BASE/test
for test_dir in `ls`; do
    if [ ! -d $test_dir ]; then
        continue
    fi
    ./run.sh $test_dir
    if [ $? -ne 0 ]; then
        echo "run test_case $test_dir failed!"
        clear_and_exit 2
    fi
done

echo "build and test success!"
clear_and_exit 0

