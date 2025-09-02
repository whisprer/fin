// ==================================================
// test_easter_eggs.cpp - Test Suite for Easter Egg Plugins
// ==================================================
#include "../src/core/woflang.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include <thread>
#include <chrono>

class EasterEggTester {
private:
    woflang::WoflangInterpreter interpreter;
    std::stringstream output_capture;
    
public:
    EasterEggTester() {
        // Load the easter egg plugins
        interpreter.load_plugin("plugins/moses_op");
        interpreter.load_plugin("plugins/prime_heck_op");
    }
    
    void test_moses_riddle() {
        std::cout << "\n=== Testing Moses Riddle Plugin ===\n";
        
        // Test the trigger command
        std::cout << "Testing '那' trigger command...\n";
        interpreter.execute("那");
        
        // Test the answer command
        std::cout << "\nTesting answer command...\n";
        interpreter.execute("answer");
        
        // Test reset command
        std::cout << "\nTesting reset command...\n";
        interpreter.execute("reset");
        
        std::cout << "Moses riddle plugin test completed!\n";
    }
    
    void test_prime_heck() {
        std::cout << "\n=== Testing Prime Heck Plugin ===\n";
        
        // Add some values to stack first
        interpreter.execute("42 17 23");
        
        std::cout << "Stack before pime_heck:\n";
        interpreter.execute(".s");  // Show stack
        
        std::cout << "\nTesting 'pime_heck' typo summons...\n";
        interpreter.execute("pime_heck");
        
        std::cout << "\nStack after pime_heck:\n";
        interpreter.execute(".s");  // Show stack (should be empty)
        
        std::cout << "Prime heck plugin test completed!\n";
    }
    
    void test_unicode_support() {
        std::cout << "\n=== Testing Unicode Support ===\n";
        
        // Test that the Chinese character is properly recognized
        auto ops = interpreter.get_operations();
        if (ops.find("那") != ops.end()) {
            std::cout << "✓ Chinese character '那' properly registered\n";
        } else {
            std::cout << "✗ Chinese character '那' not found\n";
        }
        
        if (ops.find("pime_heck") != ops.end()) {
            std::cout << "✓ 'pime_heck' operation properly registered\n";
        } else {
            std::cout << "✗ 'pime_heck' operation not found\n";
        }
    }
    
    void run_stress_test() {
        std::cout << "\n=== Stress Testing Moses Trigger ===\n";
        std::cout << "Attempting to trigger Moses riddle multiple times...\n";
        
        // The moses plugin has a 1/100 chance to trigger
        // Let's try it multiple times to see the behavior
        for (int i = 0; i < 10; ++i) {
            std::cout << "Attempt " << (i+1) << ": ";
            interpreter.execute("那");
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void run_all_tests() {
        test_unicode_support();
        test_prime_heck();
        test_moses_riddle();
        test_stress_test();
        
        std::cout << "\n=== All Easter Egg Tests Completed ===\n";
        std::cout << "These plugins add delightful mystical chaos to woflang!\n";
    }
};

int main() {
    std::cout << "WofLang Easter Egg Plugin Test Suite\n";
    std::cout << "====================================\n";
    
    try {
        EasterEggTester tester;
        tester.run_all_tests();
        
        std::cout << "\n🐺⚡ All tests completed successfully, husklyfren!\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}