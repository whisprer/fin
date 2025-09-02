#!/bin/bash

# Absolute project root (this script lives directly in project root)
PROJECT_ROOT="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "🛠 WofLang Rebuild - Root at $PROJECT_ROOT"

cd "$PROJECT_ROOT"

# Full cleanup
if [ -d "build" ]; then
    echo "⚠️ Removing old build directory and CMake cache..."
    rm -rf build
fi

mkdir build
cd build

echo "⚙️ Running CMake configuration..."
cmake ..

echo "🔨 Building project..."
cmake --build . --target all

echo "🚀 Entering bin directory..."
cd bin

echo "🧪 Running WofLang Test Suite..."
./woflang --test
