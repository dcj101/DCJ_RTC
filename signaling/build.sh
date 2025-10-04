#!/bin/bash

# 确保在正确的目录执行脚本
signaling_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$signaling_dir"
echo "Current directory: $signaling_dir"

# 检查go.mod文件是否存在，不存在则初始化
if [ ! -f go.mod ]; then
    echo "Initializing Go module..."
    go mod init signaling
    if [ $? -ne 0 ]; then
        echo "Failed to initialize Go module"
        exit 1
    fi
fi

# 更新依赖
echo "Updating dependencies..."
go mod tidy

# 使用正确的编译方式
echo "Building signaling server..."
go build -o signaling ./src

# 检查编译结果
if [ $? -eq 0 ]; then
    echo "Build successful! Binary created: signaling"
else
    echo "Build failed!"
    exit 1
fi