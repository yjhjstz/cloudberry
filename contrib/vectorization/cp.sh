#!/bin/bash

# 检查是否提供了测试用例名称
if [ $# -ne 1 ]; then
    echo "用法: $0 <测试用例名称>"
    exit 1
fi

# 获取测试用例名称
testcase=$1

# 源目录和目标目录
src_dir="/code/cbdb_src/src/test/regress"
dest_dir="/code/cbdb_src/contrib/vectorization/src/test/regress"

# 创建目标目录（如果不存在）
mkdir -p "${dest_dir}/sql"
mkdir -p "${dest_dir}/expected"

# 复制SQL文件
if [ -f "${src_dir}/sql/${testcase}.sql" ]; then
    cp "${src_dir}/sql/${testcase}.sql" "${dest_dir}/sql/${testcase}.sql"
    echo "已复制: ${testcase}.sql"
else
    echo "错误: ${src_dir}/sql/${testcase}.sql 不存在"
fi

# 复制标准输出文件
if [ -f "${src_dir}/expected/${testcase}.out" ]; then
    cp "${src_dir}/expected/${testcase}.out" "${dest_dir}/expected/${testcase}.out"
    echo "已复制: ${testcase}.out"
else
    echo "错误: ${src_dir}/expected/${testcase}.out 不存在"
fi

# 复制优化器输出文件
if [ -f "${src_dir}/expected/${testcase}_optimizer.out" ]; then
    cp "${src_dir}/expected/${testcase}_optimizer.out" "${dest_dir}/expected/${testcase}_optimizer.out"
    echo "已复制: ${testcase}_optimizer.out"
else
    echo "错误: ${src_dir}/expected/${testcase}_optimizer.out 不存在"
fi

echo "复制完成！"
