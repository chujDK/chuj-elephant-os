#!/bin/bash
DIR=$(dirname $0)
cd $DIR
make
cd $DIR/disk
./run.sh