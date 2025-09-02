// tests/benchmarking.cpp - Fixed C++ benchmark runner
#include "../src/core/woflang.hpp"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace woflang;

struct BenchmarkResult {
    std::string test_name;
    uint64_t number;
    bool is_prime;
    double duration_ms;
    bool correct;
};

class PrimeBenchmark {
private:
    std::vector<BenchmarkResult> results;
    WoflangInterpreter interp;
    
public:
    PrimeBenchmark() {
        // Load plugins
        auto plugin_dir = std::filesystem::path("../plugins");
        if (std::filesystem::exists(plugin_dir)) {
            interp.load_plugins(plugin_dir);
            std::cout << "Plugins loaded successfully\n";
        } else {
            std::cout << "Warning: No plugins directory found at ../plugins\n";
        }
    }
    
    void benchmark_prime(const std::string& name, uint64_t number, bool expected_prime) {
        std::cout << "Testing " << name << " (" << number << ")... ";
        std::cout.flush();
        
        auto start = std::chrono::high_resolution_clock::now();
        
        try {
            // Clear stack first
            interp.execute_line("stack_slayer");
            
            // Run prime check
            std::string command = std::to_string(number) + " prime_check";
            interp.execute_line(command);
            
            auto end = std::chrono::high_resolution_clock::now();
            double duration = std::chrono::duration<double, std::milli>(end - start).count();
            
            // Get result from stack
            bool result = false;
            if (!interp.stack.empty()) {
                // Get the top value from stack
                auto top_value = interp.stack.top();
                
                // Check the value - assuming prime_check pushes 1 for prime, 0 for composite
                if (top_value.d == 1.0) {
                    result = true;
                } else if (top_value.d == 0.0) {
                    result = false;
                }
            }
            
            // Check if result matches expected
            bool correct = (result == expected_prime);
            
            std::cout << (result ? "PRIME" : "COMPOSITE") 
                      << " (" << std::fixed << std::setprecision(2) << duration << "ms)"
                      << (correct ? " ✓" : " ✗") << "\n";
            
            results.push_back({name, number, result, duration, correct});
            
        } catch (const std::exception& e) {
            std::cout << "ERROR: " << e.what() << "\n";
            results.push_back({name, number, false, 0.0, false});
        }
    }
    
    void run_benchmark_suite() {
        std::cout << "🔢 WofLang Prime Benchmarking Suite\n";
        std::cout << "===================================\n\n";
        
        // Small primes (fast tests)
        std::cout << "--- Small Primes ---\n";
        benchmark_prime("Small Prime 1", 97, true);
        benchmark_prime("Small Prime 2", 997, true);
        benchmark_prime("Small Prime 3", 9973, true);
        
        // Medium primes
        std::cout << "\n--- Medium Primes ---\n";
        benchmark_prime("Medium Prime 1", 982451653, true);
        benchmark_prime("Medium Prime 2", 2147483647, true);  // 2^31-1 (Mersenne)
        
        // Large primes
        std::cout << "\n--- Large Primes ---\n";
        benchmark_prime("Large Prime 1", 1000000007, true);
        benchmark_prime("Large Prime 2", 1000000009, true);
        benchmark_prime("Large Prime 3", 10000000019ULL, true);
        
        // Composite numbers (should be faster)
        std::cout << "\n--- Composite Numbers ---\n";
        benchmark_prime("Composite 1", 1000000000, false);
        benchmark_prime("Composite 2", 999999999999ULL, false);
        benchmark_prime("Composite 3", 1000000000001ULL, false);
        
        // Stress tests
        std::cout << "\n--- Stress Tests ---\n";
        benchmark_prime("13-digit Prime", 1000000000039ULL, true);
        benchmark_prime("12-digit Prime", 100000000003ULL, true);
        
        // Pseudoprimes (tricky cases)
        std::cout << "\n--- Pseudoprimes (Tricky Cases) ---\n";
        benchmark_prime("Carmichael 1", 561, false);
        benchmark_prime("Carmichael 2", 1105, false);
        benchmark_prime("Carmichael 3", 1729, false);  // Ramanujan's number
        benchmark_prime("Pseudoprime", 2047, false);
        
        print_summary();
    }
    
    void print_summary() {
        std::cout << "\n" << std::string(50, '=') << "\n";
        std::cout << "BENCHMARK SUMMARY\n";
        std::cout << std::string(50, '=') << "\n";
        
        double total_time = 0;
        int correct = 0;
        
        std::cout << std::left << std::setw(20) << "Test Name" 
                  << std::setw(15) << "Number"
                  << std::setw(10) << "Result"
                  << std::setw(10) << "Time (ms)" 
                  << std::setw(5) << "OK" << "\n";
        std::cout << std::string(60, '-') << "\n";
        
        for (const auto& result : results) {
            std::cout << std::left << std::setw(20) << result.test_name
                      << std::setw(15) << result.number
                      << std::setw(10) << (result.is_prime ? "PRIME" : "COMPOSITE")
                      << std::setw(10) << std::fixed << std::setprecision(2) << result.duration_ms
                      << std::setw(5) << (result.correct ? "✓" : "✗")
                      << "\n";
            total_time += result.duration_ms;
            if (result.correct) correct++;
        }
        
        std::cout << std::string(60, '-') << "\n";
        std::cout << "Total time: " << std::fixed << std::setprecision(2) << total_time << " ms\n";
        std::cout << "Average time: " << std::fixed << std::setprecision(2) << total_time / results.size() << " ms\n";
        std::cout << "Tests completed: " << results.size() << "\n";
        std::cout << "Correct results: " << correct << "/" << results.size() << "\n";
        
        // Find slowest test
        auto slowest = std::max_element(results.begin(), results.end(),
            [](const BenchmarkResult& a, const BenchmarkResult& b) {
                return a.duration_ms < b.duration_ms;
            });
        
        if (slowest != results.end()) {
            std::cout << "Slowest test: " << slowest->test_name 
                      << " (" << slowest->duration_ms << " ms)\n";
        }
        
        // Find fastest test
        auto fastest = std::min_element(results.begin(), results.end(),
            [](const BenchmarkResult& a, const BenchmarkResult& b) {
                return a.duration_ms < b.duration_ms;
            });
        
        if (fastest != results.end()) {
            std::cout << "Fastest test: " << fastest->test_name 
                      << " (" << fastest->duration_ms << " ms)\n";
        }
        
        std::cout << "\n🐺 Benchmark complete! 🐺\n";
    }
};

int main() {
    PrimeBenchmark benchmark;
    benchmark.run_benchmark_suite();
    return 0;
}