#!/bin/bash

# 創建必要的資料夾
mkdir -p build
cd build
mkdir -p obj
cd ..

# 編譯
if ! command -v bear >/dev/null 2>&1; then
  make
else
  bear --output build/compile_commands.json -- make
fi
