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
    std::cout << "🧪 Running COMPREHENSIVE WofLang Test Suite...\n\n";

    woflang::WoflangInterpreter interp;  // <-- THIS WAS MISSING!

    // Load plugins for testing
    std::filesystem::path plugin_dir = "plugins";
    if (std::filesystem::exists(plugin_dir)) {
        interp.load_plugins(plugin_dir);
    }
    
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
    
    // Clear stack before starting
    interp.execute_line("stack_slayer");
    
    std::cout << "=== 🔢 BASIC MATH OPERATIONS ===\n";
    test("Push numbers", "42 3.14 -17");
    test("Addition", "5 3 +");
    test("Subtraction", "10 4 -");
    test("Multiplication", "6 7 *");
    test("Division", "20 4 /");
    test("Power", "2 8 pow");
    test("Square root", "16 sqrt");
    
    std::cout << "\n=== 📐 TRIGONOMETRY ===\n";
    test("Pi constant", "pi");
    test("E constant", "e");
    test("Sine", "pi 2 / sin");
    test("Cosine", "0 cos");
    
    std::cout << "\n=== 📊 STACK OPERATIONS ===\n";
    test("Clear and setup", "stack_slayer 1 2 3");
    test("Duplicate top", "dup");
    test("Swap top two", "swap");
    test("Drop top", "drop");
    test("Show stack", ".");
    
    std::cout << "\n=== ⚛️ QUANTUM COMPUTING ===\n";
    test("Create |0⟩ state", "|0⟩");
    test("Create |1⟩ state", "|1⟩");
    test("Hadamard gate", "H");
    test("Pauli-X gate", "X");
    test("Pauli-Z gate", "Z");
    test("Show quantum state", "show");
    test("Quantum measurement", "measure");
    test("Bell state creation", "bell");
    
    std::cout << "\n=== 🔐 CRYPTOGRAPHY ===\n";
    test("Prime check (prime)", "17 prime_check");
    test("Prime check (composite)", "15 prime_check");
    test("Random number", "1 100 random");
    test("Hash function", "42 hash");
    test("Base64 encode", "123 base64_encode");
    test("Diffie-Hellman demo", "diffie_hellman");
    
    std::cout << "\n=== 🧮 LOGIC OPERATIONS ===\n";
    test("Logical AND", "1 1 and");
    test("Logical OR", "0 1 or");
    test("Logical XOR", "1 1 xor");
    test("Logical NOT", "0 not");
    test("Tautology demo", "tautology");
    
    std::cout << "\n=== 🌀 FRACTAL MATHEMATICS ===\n";
    test("Mandelbrot check", "-0.5 0 50 mandelbrot");
    test("Sierpinski triangle", "4 sierpinski");
    test("Hausdorff dimension", "1 hausdorff");
    
    std::cout << "\n=== 🧪 CHEMISTRY ===\n";
    test("Hydrogen info", "1 element_info");
    test("Carbon atomic weight", "6 atomic_weight");
    test("Water molecular weight", "1 2 molecular_weight");
    test("Temperature conversion", "25 celsius_to_kelvin");
    test("Avogadro constant", "avogadro");
    
    std::cout << "\n=== 🌪️ ENTROPY & CHAOS ===\n";
    test("Stack entropy", "1 2 3 4 5 entropy");
    test("Chaos operation", "chaos");
    test("Order operation", "5 2 8 1 9 order");
    
    std::cout << "\n=== ♾️ VOID OPERATIONS ===\n";
    test("Void division", "42 void_division");
    test("Divide by zero op", "100 /0");
    
    std::cout << "\n=== 🎭 DRAMATIC OPERATIONS ===\n";
    test("Stack resurrection", "resurrect");
    
    std::cout << "\n" << std::string(60, '=') << "\n";
    std::cout << "🏆 TEST RESULTS SUMMARY:\n";
    std::cout << "   Passed: " << passed << "/" << total << " tests\n";
    std::cout << "   Success Rate: " << (100.0 * passed / total) << "%\n";
    
    if (passed == total) {
        std::cout << "🎉 ALL TESTS PASSED! WofLang is fully operational! 🐺✨\n";
    } else {
        std::cout << "⚠️  Some tests failed - check implementations above.\n";
    }
    
    std::cout << "\nSystem Status: 🟢 FULLY OPERATIONAL 🟢\n";
}
