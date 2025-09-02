// tests/woflang_test_runner.cpp - Test runner for Woflang
#include "../src/core/woflang.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <vector>
#include <chrono>

using namespace woflang;

struct TestResult {
    std::string name;
    bool passed;
    std::string error;
    double duration_ms;
};

class TestRunner {
private:
    std::vector<TestResult> results;
    
public:
    void run_test(const std::filesystem::path& test_file) {
        auto start = std::chrono::high_resolution_clock::now();
        
        TestResult result;
        result.name = test_file.stem().string();
        result.passed = true;
        
        try {
            WoflangInterpreter interp;
            
            // Load core plugins for tests
            auto plugin_dir = std::filesystem::path("../bin/plugins");
            if (std::filesystem::exists(plugin_dir)) {
                interp.load_plugins(plugin_dir);
            }
            
            // Execute test script
            interp.exec_script(test_file);
            
            // Check if test left "PASS" on stack
            if (!interp.stack.empty() && 
                interp.stack.back().type == WofType::String &&
                std::get<std::string>(interp.stack.back().value) == "PASS") {
                result.passed = true;
            } else {
                result.passed = false;
                result.error = "Test did not leave 'PASS' on stack";
            }
            
        } catch (const std::exception& e) {
            result.passed = false;
            result.error = e.what();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        results.push_back(result);
    }
    
    void run_all_tests(const std::filesystem::path& test_dir) {
        std::cout << "Running Woflang Test Suite\n";
        std::cout << "==========================\n\n";
        
        if (!std::filesystem::exists(test_dir)) {
            std::cout << "Test directory not found: " << test_dir << "\n";
            return;
        }
        
        for (const auto& entry : std::filesystem::directory_iterator(test_dir)) {
            if (entry.path().extension() == ".wof") {
                run_test(entry.path());
            }
        }
        
        // If no .wof files found, run built-in tests
        if (results.empty()) {
            run_builtin_tests();
        }
    }
    
    void run_builtin_tests() {
        std::cout << "Running built-in tests...\n";
        
        // Test 1: Basic arithmetic
        {
            TestResult result;
            result.name = "basic_arithmetic";
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                WoflangInterpreter interp;
                interp.execute("2 3 + 5 =");
                
                if (!interp.stack.empty() && 
                    interp.stack.back().type == WofType::Boolean &&
                    std::get<bool>(interp.stack.back().value)) {
                    result.passed = true;
                } else {
                    result.passed = false;
                    result.error = "2 + 3 != 5";
                }
            } catch (const std::exception& e) {
                result.passed = false;
                result.error = e.what();
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back(result);
        }
        
        // Test 2: Stack operations
        {
            TestResult result;
            result.name = "stack_operations";
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                WoflangInterpreter interp;
                interp.execute("1 2 3 rot");
                
                if (interp.stack.size() == 3 &&
                    interp.stack[0].type == WofType::Integer &&
                    interp.stack[1].type == WofType::Integer &&
                    interp.stack[2].type == WofType::Integer &&
                    std::get<int64_t>(interp.stack[0].value) == 2 &&
                    std::get<int64_t>(interp.stack[1].value) == 3 &&
                    std::get<int64_t>(interp.stack[2].value) == 1) {
                    result.passed = true;
                } else {
                    result.passed = false;
                    result.error = "rot operation failed";
                }
            } catch (const std::exception& e) {
                result.passed = false;
                result.error = e.what();
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back(result);
        }
        
        // Test 3: Variables
        {
            TestResult result;
            result.name = "variables";
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                WoflangInterpreter interp;
                interp.execute("42 !answer $answer $answer +");
                
                if (!interp.stack.empty() && 
                    interp.stack.back().type == WofType::Integer &&
                    std::get<int64_t>(interp.stack.back().value) == 84) {
                    result.passed = true;
                } else {
                    result.passed = false;
                    result.error = "Variable storage/retrieval failed";
                }
            } catch (const std::exception& e) {
                result.passed = false;
                result.error = e.what();
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back(result);
        }
        
        // Test 4: String operations
        {
            TestResult result;
            result.name = "string_operations";
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                WoflangInterpreter interp;
                interp.execute("\"Hello\" \" World\" +");
                
                if (!interp.stack.empty() && 
                    interp.stack.back().type == WofType::String &&
                    std::get<std::string>(interp.stack.back().value) == "Hello World") {
                    result.passed = true;
                } else {
                    result.passed = false;
                    result.error = "String concatenation failed";
                }
            } catch (const std::exception& e) {
                result.passed = false;
                result.error = e.what();
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back(result);
        }
        
        // Test 5: Plugin loading (if available)
        {
            TestResult result;
            result.name = "plugin_loading";
            auto start = std::chrono::high_resolution_clock::now();
            
            try {
                WoflangInterpreter interp;
                
                // Try to load plugins
                auto plugin_dir = std::filesystem::path("../bin/plugins");
                if (std::filesystem::exists(plugin_dir)) {
                    interp.load_plugins(plugin_dir);
                    
                    // Test if π (pi) operation is available (from math_greek plugin)
                    interp.execute("π");
                    
                    if (!interp.stack.empty() && 
                        interp.stack.back().type == WofType::Double) {
                        double pi_val = std::get<double>(interp.stack.back().value);
                        if (pi_val > 3.14 && pi_val < 3.15) {
                            result.passed = true;
                        } else {
                            result.passed = false;
                            result.error = "π value incorrect";
                        }
                    } else {
                        result.passed = false;
                        result.error = "π operation not found";
                    }
                } else {
                    result.passed = false;
                    result.error = "Plugin directory not found";
                }
            } catch (const std::exception& e) {
                result.passed = false;
                result.error = e.what();
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            result.duration_ms = std::chrono::duration<double, std::milli>(end - start).count();
            results.push_back(result);
        }
    }
    
    void print_results() {
        int passed = 0;
        int failed = 0;
        
        for (const auto& result : results) {
            if (result.passed) {
                std::cout << "✓ " << result.name << " (" 
                         << std::fixed << std::setprecision(2) << result.duration_ms << " ms)\n";
                passed++;
            } else {
                std::cout << "✗ " << result.name << " - " 
                         << result.error << "\n";
                failed++;
            }
        }
        
        std::cout << "\n";
        std::cout << "Tests: " << passed << " passed, " << failed << " failed, "
                  << (passed + failed) << " total\n";
        
        if (failed > 0) {
            std::cout << "Test suite FAILED\n";
        } else {
            std::cout << "Test suite PASSED\n";
        }
    }
    
    bool all_passed() const {
        for (const auto& result : results) {
            if (!result.passed) return false;
        }
        return true;
    }
};

int main(int argc, char* argv[]) {
    std::filesystem::path test_dir = "tests";
    
    if (argc > 1) {
        test_dir = argv[1];
    }
    
    TestRunner runner;
    runner.run_all_tests(test_dir);
    runner.print_results();
    
    return runner.all_passed() ? 0 : 1;
}