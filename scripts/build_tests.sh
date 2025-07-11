#!/usr/bin/sh

mkdir -p bin/tests

srcNotMain=$(find src -name "*.c" ! -name "main.c")

gcc -DIS_RUNNING_TESTS tests/*.c $srcNotMain -I src/ -I tests/ -g -o bin/tests/tests
