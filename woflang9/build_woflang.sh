#!/bin/bash
# build_woflang.sh - Quick build script for Woflang

echo "🔨 Building Woflang..."
echo "====================="

# Create directory structure
echo "📁 Creating directory structure..."
mkdir -p src/core src/io src/repl src/tests
mkdir -p plugins tests
mkdir -p build

# Check if files are in place
echo "🔍 Checking core files..."
if [ ! -f "src/core/woflang.hpp" ]; then
    echo "⚠️  Warning: src/core/woflang.hpp not found"
fi

if [ ! -f "src/core/woflang.cpp" ]; then
    echo "⚠️  Warning: src/core/woflang.cpp not found"
fi

# Build with CMake
echo "🏗️  Configuring with CMake..."
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "🔧 Building..."
cmake --build . --config Release -j$(nproc)

echo ""
echo "✅ Build complete!"
echo ""
echo "To run Woflang:"
echo "  ./bin/woflang              # Start REPL"
echo "  ./bin/woflang script.wof   # Run a script"
echo "  ./bin/woflang --test       # Run tests"
echo "  ./bin/woflang --help       # Show help"
echo ""
echo "🌟 Enjoy your Unicode-native stack language!"