#!/bin/bash
# 设置项目目录

# 检查并初始化Go模块（如果不存在）
if [ ! -f "go.mod" ]; then
    echo "初始化Go模块..."
    go mod init screen_share
    if [ $? -ne 0 ]; then
        echo "Go模块初始化失败"
        exit 1
    fi
else
    echo "Go模块已存在，跳过初始化"
fi

# 构建项目（移除src/参数，因为它是空目录）
echo "开始构建项目..."
go build -o screen_share src/*
if [ $? -eq 0 ]; then
    echo "构建成功！可执行文件：$dir/screen_share"
    # 检查并提醒执行权限
    if [ ! -x "screen_share" ]; then
        echo "提示：可执行文件可能需要添加执行权限，可以运行 'chmod +x screen_share'"
    fi
else
    echo "构建失败"
    exit 1
fi