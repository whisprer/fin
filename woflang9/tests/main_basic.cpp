#include "src/core/woflang.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <cstring>  // Added for strcmp

#ifdef _WIN32
    #include <windows.h>
#endif

// ASCII art banner
const char* WOFLANG_BANNER = R"(
╦ ╦┌─┐┌─┐┬  ┌─┐┌┐┌┌─┐
║║║│ │├┤ │  ├─┤││││ ┬
╚╩╝└─┘└  ┴─┘┴ ┴┘└┘└─┘ v1.1.0
A Unicode-native stack language
)";

// Function declarations
void show_help();
void run_tests();

int main(int argc, char* argv[]) {
    // Enable UTF-8 support on Windows
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    if (argc > 1) {
        if (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
            show_help();
            return 0;
        }
        if (strcmp(argv[1], "--test") == 0) {
            run_tests();
            return 0;
        }
        if (strcmp(argv[1], "--version") == 0 || strcmp(argv[1], "-v") == 0) {
            std::cout << "wofLang v1.1.0\n";
            std::cout << "Built: " << __DATE__ << " " << __TIME__ << "\n";
            std::cout << "Compiler: " << __VERSION__ << "\n";
            return 0;
        }
    }

    std::cout << WOFLANG_BANNER << std::endl;

    woflang::WoflangInterpreter interp;

    // Load plugins from the plugins directory
    std::filesystem::path plugin_dir = "plugins";
    if (std::filesystem::exists(plugin_dir)) {
        interp.load_plugins(plugin_dir);
    } else {
        std::cout << "No plugins directory found. Running with built-in operations only.\n";
    }
    
    std::cout << "Welcome to woflang!\n";
    std::string line;
    while (std::cout << "wof> ", std::getline(std::cin, line)) {
        if (line == "quit" || line == "exit") {
            std::cout << "Goodbye from woflang! 🐺\n";
            break;
        }
        interp.execute_line(line);
    }
    return 0;
}

void show_help() {
    std::cout << "WofLang - Stack-based Programming Language\n\n";
    std::cout << "Usage: woflang [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -v, --version  Show version information\n";
    std::cout << "  --test         Run test suite\n\n";
    std::cout << "Interactive Commands:\n";
    std::cout << "  exit, quit     Exit the interpreter\n";
    std::cout << "  <number>       Push number onto stack\n";
    std::cout << "  +, -, *, /     Basic arithmetic\n";
    std::cout << "  dup, drop      Stack manipulation\n";
    std::cout << "  .              Show stack contents\n\n";
    std::cout << "Plugin Operations (if loaded):\n";
    std::cout << "  stack_slayer   Clear the entire stack dramatically\n";
    std::cout << "  resurrect      Restore mystical constants\n";
    std::cout << "  |0⟩, |1⟩       Quantum states (if quantum_ops loaded)\n";
    std::cout << "  H, X, Z        Quantum gates (if quantum_ops loaded)\n";
    std::cout << "  prime_check    Check if number is prime (if crypto_ops loaded)\n";
}

void run_tests() {
    woflang::WoflangInterpreter interp;
    
    int passed = 0, total = 0;
    
    auto test = [&](const std::string& name, const std::string& code, bool should_succeed = true) {
        total++;
        std::cout << "🔬 Testing " << name << ": ";
        try {
            interp.execute_line(code);
            if (should_succeed) {
                std::cout << "✅ PASS\n";
                passed++;
            } else {
                std::cout << "❌ FAIL (should have failed)\n";
            }
        } catch (const std::exception& e) {
            if (!should_succeed) {
                std::cout << "✅ PASS (expected failure)\n";
                passed++;
            } else {
                std::cout << "❌ FAIL: " << e.what() << "\n";
            }
        }
    };

# basic tesing code
    std::cout << "Running WofLang Test Suite...\n\n";
    
    woflang::WoflangInterpreter interp;
    
    // Load plugins for testing
    std::filesystem::path plugin_dir = "plugins";
    if (std::filesystem::exists(plugin_dir)) {
        interp.load_plugins(plugin_dir);
    }
    
    // Test basic operations
    std::cout << "=== Basic Stack Operations ===\n";
    try {
        interp.execute_line("42");
        interp.execute_line(".");
        interp.execute_line("drop");
        std::cout << "✓ Basic stack operations working\n";
    } catch (const std::exception& e) {
        std::cout << "✗ Basic operations failed: " << e.what() << "\n";
    }
    
    // Test arithmetic
    std::cout << "\n=== Arithmetic Operations ===\n";
    try {
        interp.execute_line("5 3 +");
        interp.execute_line(".");
        interp.execute_line("drop");
        std::cout << "✓ Arithmetic operations working\n";
    } catch (const std::exception& e) {
        std::cout << "✗ Arithmetic failed: " << e.what() << "\n";
    }
    
    // Test plugin operations
    std::cout << "\n=== Plugin Operations ===\n";
    try {
        interp.execute_line("stack_slayer");
        interp.execute_line("resurrect");
        interp.execute_line(".");
        std::cout << "✓ Stack slayer plugin working\n";
    } catch (const std::exception& e) {
        std::cout << "✗ Stack slayer plugin failed: " << e.what() << "\n";
    }
    
    // Test quantum operations if available
    std::cout << "\n=== Quantum Operations (if available) ===\n";
    try {
        interp.execute_line("|0⟩");
        interp.execute_line("show");
        std::cout << "✓ Quantum operations working\n";
    } catch (const std::exception& e) {
        std::cout << "ℹ Quantum operations not available or failed: " << e.what() << "\n";
    }
    
    // Test crypto operations if available
    std::cout << "\n=== Crypto Operations (if available) ===\n";
    try {
        interp.execute_line("17 prime_check");
        interp.execute_line(".");
        std::cout << "✓ Crypto operations working\n";
    } catch (const std::exception& e) {
        std::cout << "ℹ Crypto operations not available or failed: " << e.what() << "\n";
    }
    
    std::cout << "\nTest suite completed!\n";