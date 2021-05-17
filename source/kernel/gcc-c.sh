#!/bin/bash
last=".o"
gcc -c -o ${1%.*}$last $1 -m32 -fno-asynchronous-unwind-tables

