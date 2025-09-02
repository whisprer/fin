# Woflang - A Unicode-Native Stack Language

Woflang is a modular, plugin-driven, Unicode-native stack language, inspired by Forth, J, and symbolic math languages. It supports dynamic loading of operations and plugins, symbolic computation, unit handling, and various special features including quantum, fractal, and cryptographic operations.

## Features

- **Stack-based symbolic interpreter**: Intuitive and powerful stack-based programming
- **Full Unicode/glyph token support**: Use symbols like π, σ, Δ, and more directly in your code
- **Plugin system**: Dynamically load operations and extensions
- **SIMD-accelerated tokenization**: Fast Unicode text processing
- **Symbolic math**: Pattern matching, simplification, and equation solving
- **Unit handling**: Built-in support for physical units and conversions
- **Specialized domains**: Quantum computing, fractals, cryptography, and more
- **Interactive REPL**: With history, suggestions, and special commands
- **Sacred/special operations**: Unique philosophical features and Easter eggs
- **Cross-platform**: Works on Windows, Linux, and macOS

## Building from Source

### Prerequisites

- CMake 3.16 or newer
- C++23 compatible compiler:
  - GCC 11 or newer
  - Clang 14 or newer
  - MSVC 2019 or newer
- (Optional) SIMD support: AVX2, AVX512, or NEON

### Build Instructions

#### Windows (Visual Studio)

```cmd
git clone https://github.com/yourusername/woflang.git
cd woflang
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

#### Linux/macOS

```bash
git clone https://github.com/yourusername/woflang.git
cd woflang
mkdir build
cd build
cmake ..
cmake --build . -- -j$(nproc)
```

The built executables will be in the `build/bin` directory, and the plugins will be in the `build/plugins` directory.

### Installation

```bash
cmake --install .
```

This will install Woflang to your system. The default installation directory is:
- Windows: `C:\Program Files\woflang`
- Linux/macOS: `/usr/local`

## Usage

### Running the REPL

```
woflang
```

This will start the Woflang REPL, allowing you to interact with the language interactively.

### Running a Script

```
woflang script.wof
```

This will execute the specified script file.

### Command-Line Options

```
Usage: woflang [options] [script.wof]
Options:
  -h, --help            Display this help message
  -v, --version         Show version information
  -p, --plugins DIR     Specify an alternative plugins directory
  -l, --list-plugins    List all available plugins
  -n, --no-plugins      Run without loading any plugins
  -i, --interactive     Start REPL after executing script
  --no-sacred           Disable sacred/special operations
  --test                Run all test scripts in the tests directory

If no script is provided, the REPL will start automatically.
```

## Language Examples

### Basic Stack Operations

```
1 2 + print    # Add 1 and 2, then print the result (3)
5 dup * print   # Square 5 by duplicating it, then print (25)
10 2 swap / print  # Swap 10 and 2, then divide (0.2)
```

### Math with Greek Symbols

```
π 2 * print    # Print 2π (approximately 6.28)
5 σ print      # Calculate standard deviation of the stack
```

### Symbolic Computation

```
x x + simplify print    # Simplify x + x to get 2 * x
2 x = 4 solve_linear    # Solve the linear equation 2x = 4
```

### Unit Arithmetic

```
5 m unit 10 m unit * print    # Multiply 5m by 10m to get 50m²
```

### Fractals

```
-0.5 0.5 100 mandelbrot    # Check if the point (-0.5, 0.5) is in the Mandelbrot set
sierpinski                 # Generate and display the Sierpinski triangle
```

### Quantum Operations

```
|0⟩             # Create a |0⟩ qubit state
H               # Apply the Hadamard gate
measure         # Measure the qubit
```

### Cryptography

```
"Hello" "key" xor_cipher    # Apply XOR cipher to "Hello" with key "key"
1234 hash                   # Calculate hash of the integer 1234
```

## Adding Plugins

Plugins are shared libraries (DLLs on Windows, .so files on Linux, .dylib files on macOS) that extend Woflang with new operations. To create a new plugin:

1. Create a C++ file in the `plugins` directory, e.g., `plugins/my_plugin.cpp`:

```cpp
#include "../../src/core/woflang.hpp"

namespace woflang {

class MyPlugin : public WoflangPlugin {
public:
    void register_ops(WoflangInterpreter& interp) override {
        // Register your operations here
        interp.register_op("my_op", [](WoflangInterpreter& interp) {
            // Implementation of your operation
            std::cout << "Hello from my_op!" << std::endl;
        });
    }
};

} // namespace woflang

// Plugin registration function
WOFLANG_PLUGIN_EXPORT void register_plugin(woflang::WoflangInterpreter& interp) {
    static woflang::MyPlugin plugin;
    plugin.register_ops(interp);
}
```

2. Build the plugin:

```bash
cd build
cmake --build .
```

3. The plugin will be automatically built and placed in the `build/plugins` directory.

## License

This project is licensed under the hybrid MIT and CC0 license - see the [LICENSE](LICENSE) file for details.

## Acknowledgements

Woflang draws inspiration from:
- Forth and other stack-based languages
- J and APL for symbolic operations
- Unicode and glyph-based notation
- Various mathematical, physical, and philosophical concepts