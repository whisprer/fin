Build Instructions
Here's how to build the project from scratch:
bash# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build . --config Release

# Run REPL
./woflang_repl

# Run tests
ctest -C Release
