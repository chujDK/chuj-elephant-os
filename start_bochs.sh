#!/bin/bash
make
DIR=$(dirname $0)
cd $DIR/disk
./run.sh
