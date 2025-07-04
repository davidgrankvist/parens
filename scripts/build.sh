#!/usr/bin/sh

mkdir -p bin/parens

gcc src/*.c -I src/ -g -o bin/parens/parens
