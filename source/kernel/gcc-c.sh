#!/bin/bash
last=".o"
gcc -I ../lib/kernel/ -c -o ${1%.*}$last $1 -m32 -fno-asynchronous-unwind-tables -std=c99

