#!/usr/bin/sh

mkdir -p bin

gcc src/*.c -I src/ -g -o bin/parens
