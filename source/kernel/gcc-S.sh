#!/bin/bash
last=".S"
gcc -S -o ${1%.*}$last $1 -m32 -fno-asynchronous-unwind-tables

