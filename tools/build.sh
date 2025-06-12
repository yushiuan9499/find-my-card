#!/bin/bash

# 檢查Dependencies
if ! dpkg -s libjsoncpp-dev >/dev/null 2>&1; then
  echo "Error: libjsoncpp-dev is not installed. Please install it using 'sudo apt-get install libjsoncpp-dev'."
  exit 1
fi

# 創建必要的資料夾
mkdir -p build
cd build
mkdir -p obj
mkdir -p obj/Core
cd ..

# 編譯
if ! command -v bear >/dev/null 2>&1; then
  make
else
  bear --output build/compile_commands.json -- make
fi
