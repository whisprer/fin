// ==================================================
// 1. void_division_ops.cpp - Forbidden Mathematical Operations
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <limits>

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    (*op_table)["void_division"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "⚠️  FORBIDDEN OPERATION DETECTED ⚠️\n";
        std::cout << "Attempting to divide by the void...\n";
        
        if (stack.size() < 2) {
            std::cout << "The void requires a sacrifice.\n";
            return;
        }
        
        auto divisor = stack.top(); stack.pop();
        auto dividend = stack.top(); stack.pop();
        
        std::cout << "Dividing " << dividend.to_string() 
                 << " by the essence of nothingness...\n";
        
        // The void consumes all
        while (!stack.empty()) stack.pop();
        
        // But leaves behind infinity
        woflang::WofValue result;
        result.d = std::numeric_limits<double>::infinity();
        stack.push(result);
        
        std::cout << "The operation succeeds. Infinity remains.\n";
        std::cout << "You have gazed into the abyss.\n";
    };
    
    (*op_table)["/0"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Even the void requires something to consume.\n";
            return;
        }
        
        auto value = stack.top(); stack.pop();
        
        std::cout << "÷0: " << value.d << " → ∞\n";
        woflang::WofValue result;
        result.d = std::numeric_limits<double>::infinity();
        stack.push(result);
    };
}
