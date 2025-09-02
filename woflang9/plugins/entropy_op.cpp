// ==================================================
// 2. entropy_ops.cpp - Chaos and Information Theory
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <chrono>
#include <map>
#include <algorithm>

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    (*op_table)["entropy"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "The void has no entropy. Only chaos remains.\n";
            return;
        }
        
        // Calculate Shannon entropy of the stack
        std::map<double, int> counts;
        double total = 0;
        
        // Copy stack to vector to analyze without destroying
        std::vector<woflang::WofValue> values;
        std::stack<woflang::WofValue> temp_stack = stack;
        while (!temp_stack.empty()) {
            values.push_back(temp_stack.top());
            temp_stack.pop();
        }
        
        for (const auto& val : values) {
            counts[val.d]++;
            total++;
        }
        
        double entropy = 0.0;
        for (const auto& [_, count] : counts) {
            double p = count / total;
            entropy -= p * std::log2(p);
        }
        
        std::cout << "Stack entropy: " << entropy << " bits\n";
        std::cout << "The universe tends toward maximum entropy...\n";
        
        woflang::WofValue result;
        result.d = entropy;
        stack.push(result);
    };
    
    (*op_table)["chaos"] = [](std::stack<woflang::WofValue>& stack) {
        static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
        
        // Generate chaotic values
        std::uniform_real_distribution<> dis(0.0, 1.0);
        double chaos_value = dis(gen);
        
        std::cout << "From chaos, order emerges: " << chaos_value << "\n";
        
        // Randomly shuffle the stack
        if (stack.size() > 1) {
            std::vector<woflang::WofValue> values;
            while (!stack.empty()) {
                values.push_back(stack.top());
                stack.pop();
            }
            std::shuffle(values.begin(), values.end(), gen);
            for (auto& val : values) {
                stack.push(val);
            }
            std::cout << "The stack has been touched by chaos.\n";
        }
        
        woflang::WofValue result;
        result.d = chaos_value;
        stack.push(result);
    };
    
    (*op_table)["order"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "Order requires at least two elements.\n";
            return;
        }
        
        // Sort stack elements
        std::vector<woflang::WofValue> values;
        while (!stack.empty()) {
            values.push_back(stack.top());
            stack.pop();
        }
        
        std::sort(values.begin(), values.end(), [](const woflang::WofValue& a, const woflang::WofValue& b) {
            return a.d < b.d;
        });
        
        for (const auto& val : values) {
            stack.push(val);
        }
        
        std::cout << "Order has been restored to the stack.\n";
    };
}
