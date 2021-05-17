#!/bin/bash
DIR=$(dirname $0)
cd $DIR
./source/all_write_to_disk.sh
cd ./disk
./run.sh
#$DIR/source/all_write_to_disk.sh
#$DIR/disk/run.sh
