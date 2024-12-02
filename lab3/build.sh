#!/bin/bash

echo "开始编译程序..."


echo "编译 main.cpp..."
if g++ -g "./main.cpp" -o "main"; then
    echo "main.cpp 编译成功"
else
    echo "main.cpp 编译失败"
    exit 1
fi


echo -e "\n编译 read.cpp..."
if g++ -g "./read.cpp" -o "read"; then
    echo "read.cpp 编译成功"
else
    echo "read.cpp 编译失败"
    exit 1
fi


echo -e "\n编译 write.cpp..."
if g++ -g "./write.cpp" -o "write"; then
    echo "write.cpp 编译成功"
else
    echo "write.cpp 编译失败"
    exit 1
fi


echo -e "\n添加执行权限..."
if chmod +x ./main ./read ./write; then
    echo "成功添加执行权限"
else
    echo "添加执行权限失败"
    exit 1
fi


echo -e "\n开始运行 main 程序..."
if ./main; then
    echo "main 程序执行完成"
else
    echo "main 程序执行失败"
    exit 1
fi


echo -e "\n比较 input.txt 和 output.txt 的差异..."
if diff input.txt output.txt; then
    echo "文件完全一致"
else
    echo "文件存在差异"
fi


echo -e "\n所有操作已完成"

