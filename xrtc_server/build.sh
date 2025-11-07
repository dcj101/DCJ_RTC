#!/bin/bash

# 只有目录不存在时才创建
[ ! -d "out" ] && mkdir out

cd out && cmake ../ || exit 1

if [ $# -gt 0 ] && [ "$1" = "clean" ]; then
    echo "make clean"
    make clean
else
    echo "make"
    make
fi