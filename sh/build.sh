#!/bin/bash

# ソースファイルの決定
SOURCE_FILE="${1:-main}.cpp" # 引数が無い場合はmain.cppを使用

# "atcoder"の出現回数をカウント
ATCODER_COUNT=$(grep -o "atcoder" "$SOURCE_FILE" | wc -l)

# "gmpxx.h"をカウント (gmpxx.hのため)
GMP_COUNT=$(grep -o "gmpxx.h" "$SOURCE_FILE" | wc -l)

# コンパイルオプションの決定
CXX_FLAGS="-std=gnu++20 -O2 -Wall -Wextra"
LINK_FLAGS=""

# 第二引数に"debug"を指定でコンパイルオプションを追加
if [ "$2" == "debug" ]; then
    CXX_FLAGS+=" -fsanitize=address -fsanitize=undefined"
fi
if [ "$ATCODER_COUNT" -ge 1 ]; then
    CXX_FLAGS+=" -I./ac-library"
fi
if [ "$GMP_COUNT" -ge 1 ]; then
    LINK_FLAGS+=" -lgmpxx -lgmp"
fi

# コンパイル実行
g++ $CXX_FLAGS "$SOURCE_FILE" $LINK_FLAGS