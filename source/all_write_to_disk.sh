#!/bin/bash
DIR=`dirname $0`
function read_dir(){
    for file in `ls $1`       #注意此处这是两个反引号，表示运行系统命令
    do
        if [ -d $1"/"$file ]  #注意此处之间一定要加上空格，否则会报错
        then
            cd $1"/"$file
            ./write_to_disk.sh
            cd ..
        fi
    done
}   
#读取第一个参数
cd $DIR
read_dir .
