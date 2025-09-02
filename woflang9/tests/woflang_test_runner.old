#include "../core/woflang.hpp"
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
            auto plugin_dir = std::filesystem::path("plugins");
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
        
        for (const auto& entry : std::filesystem::directory_iterator(test_dir)) {
            if (entry.path().extension() == ".wof") {
                run_test(entry.path());
            }
        }
    }
    
    void print_results() {
        int passed = 0;
        int failed = 0;
        
        for (const auto& result : results) {
            if (result.passed) {
                std::cout << "✓ " << result.name << " (" 
                         << result.duration_ms << " ms)\n";
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
    
    if (!std::filesystem::exists(test_dir)) {
        std::cerr << "Test directory not found: " << test_dir << "\n";
        return 1;
    }
    
    TestRunner runner;
    runner.run_all_tests(test_dir);
    runner.print_results();
    
    return runner.all_passed() ? 0 : 1;
}