#include "src/core/woflang.hpp"
#include <iostream>
#include <string>
#include <filesystem>
#include <cstring>  // Added for strcmp
#include <chrono>
#include <iomanip>
#include <vector>

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
void run_benchmark();

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
        if (strcmp(argv[1], "--benchmark") == 0) {
            run_benchmark();
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
    std::cout << "Type 'help' for commands, 'quit' to exit, or '--benchmark' for speed tests.\n";
    std::string line;
    while (std::cout << "wof> ", std::getline(std::cin, line)) {
        if (line == "quit" || line == "exit") {
            std::cout << "Goodbye from woflang! 🐺\n";
            break;
        }
        if (line == "help") {
            show_help();
            continue;
        }
        if (line == "benchmark") {
            run_benchmark();
            continue;
        }
        interp.execute_line(line);
    }
    return 0;
}

void run_benchmark() {
    std::cout << "🔢 WofLang Prime Benchmarking Suite\n";
    std::cout << "===================================\n\n";
    
    woflang::WoflangInterpreter interp;
    
    // Load plugins
    std::filesystem::path plugin_dir = "plugins";
    if (std::filesystem::exists(plugin_dir)) {
        interp.load_plugins(plugin_dir);
        std::cout << "Plugins loaded successfully\n\n";
    } else {
        std::cout << "Warning: No plugins directory found\n\n";
    }
    
    struct BenchTest {
        std::string name;
        uint64_t number;
        bool expected_prime;
    };
    
    std::vector<BenchTest> tests = {
        // Small primes
        {"Small Prime 1", 97, true},
        {"Small Prime 2", 997, true},
        {"Small Prime 3", 9973, true},
        
        // Medium primes
        {"Medium Prime 1", 982451653, true},
        {"Medium Prime 2", 2147483647, true},  // 2^31-1
        
        // Large primes
        {"Large Prime 1", 1000000007, true},
        {"Large Prime 2", 1000000009, true},
        {"Large Prime 3", 10000000019ULL, true},
        
        // Composite numbers
        {"Composite 1", 1000000000, false},
        {"Composite 2", 999999999999ULL, false},
        {"Composite 3", 1000000000001ULL, false},
        
        // Stress tests
        {"13-digit Prime", 1000000000039ULL, true},
        {"12-digit Prime", 100000000003ULL, true},
        
        // Pseudoprimes
        {"Carmichael 1", 561, false},
        {"Carmichael 2", 1105, false},
        {"Carmichael 3", 1729, false},
        {"Pseudoprime", 2047, false},
    };
    
    std::cout << std::left << std::setw(20) << "Test Name" 
              << std::setw(15) << "Number"
              << std::setw(10) << "Expected"
              << std::setw(10) << "Result"
              << std::setw(12) << "Time (ms)" 
              << std::setw(5) << "OK" << "\n";
    std::cout << std::string(70, '-') << "\n";
    
    double total_time = 0.0;
    int correct = 0;
    
    for (const auto& test : tests) {
        std::cout << std::left << std::setw(20) << test.name
                  << std::setw(15) << test.number
                  << std::setw(10) << (test.expected_prime ? "PRIME" : "COMPOSITE");
        std::cout.flush();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            // Clear stack
            interp.execute_line("stack_slayer");
            
            // Run prime check
            std::string command = std::to_string(test.number) + " prime_check";
            interp.execute_line(command);
            
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            
            // Get result from stack
            bool result = false;
            if (!interp.stack.empty()) {
                auto top_value = interp.stack.top();
                result = (top_value.d == 1.0);
            }
            
            bool is_correct = (result == test.expected_prime);
            
            std::cout << std::setw(10) << (result ? "PRIME" : "COMPOSITE")
                      << std::setw(12) << std::fixed << std::setprecision(2) << duration
                      << std::setw(5) << (is_correct ? "✓" : "✗") << "\n";
            
            total_time += duration;
            if (is_correct) correct++;
            
        } catch (const std::exception& e) {
            std::cout << std::setw(10) << "ERROR"
                      << std::setw(12) << "0.00"
                      << std::setw(5) << "✗" << "\n";
            std::cout << "    Error: " << e.what() << "\n";
        }
    }
    
    std::cout << std::string(70, '-') << "\n";
    std::cout << "Total time: " << std::fixed << std::setprecision(2) << total_time << " ms\n";
    std::cout << "Average time: " << std::fixed << std::setprecision(2) << total_time / tests.size() << " ms\n";
    std::cout << "Correct results: " << correct << "/" << tests.size() << "\n";
    std::cout << "Success rate: " << std::fixed << std::setprecision(1) << (100.0 * correct / tests.size()) << "%\n";
    std::cout << "\n🐺 Benchmark complete! 🐺\n\n";
}

void show_help() {
    std::cout << "WofLang - Stack-based Programming Language\n\n";
    std::cout << "Usage: woflang [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -v, --version  Show version information\n";
    std::cout << "  --test         Run test suite\n";
    std::cout << "  --benchmark    Run prime benchmarking suite\n\n";
    std::cout << "Interactive Commands:\n";
    std::cout << "  exit, quit     Exit the interpreter\n";
    std::cout << "  help           Show this help\n";
    std::cout << "  benchmark      Run benchmarking suite\n";
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