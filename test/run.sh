#!/bin/sh

ARG_NUM=1
EXE_PATH="../.."
EXE_NAME="process_judger"
IN_NAME="input.data"
OUT_NAME="output.data"
TIME_LIMIT=1
MEM_LIMIT=4
TEST_EXE_NAME="test_exe"
EXCEPTED_RET_NAME="expected_return"

if [ $# -ne $ARG_NUM ]; then
    echo "arg num($#) != $ARG_NUM"
    echo "usage: sh run.sh test_dir_name"
    exit 1
fi

cd $1
if [ $? -ne 0 ]; then
    echo "cannot cd $1"
    exit 2
fi

make clean && make
if [ $? -ne 0 ]; then
    echo "make failed"
    exit 3
fi

cp $EXE_PATH/$EXE_NAME .
if [ $? -ne 0 ]; then
    echo "cp $EXE_PATH/$EXE_NAME failed"
    exit 4
fi

if [ -f ./$EXCEPTED_RET_NAME ]; then
    EXPECTED_RET=`cat $EXCEPTED_RET_NAME`
else
    echo "$EXCEPTED_RET_NAME doesn't exist"
    rm ./$EXE_NAME
    make clean
    exit 5
fi

# to prevent Permission denied
touch $OUT_NAME

./$EXE_NAME $IN_NAME $OUT_NAME $TIME_LIMIT $MEM_LIMIT ./$TEST_EXE_NAME
if [ "$?" -ne "$EXPECTED_RET" ]; then
    echo "ret($?) != expected return value($EXPECTED_RET)"
    rm ./$EXE_NAME
    make clean
    exit 6
fi

rm ./$EXE_NAME
make clean
exit 0
