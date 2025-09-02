// ==================================================
// FIXED: stack_slayer_op.cpp - Simple extern C style
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <thread>
#include <chrono>

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    (*op_table)["stack_slayer"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "The Stack Slayer finds nothing to slay.\n";
            return;
        }
        
        std::cout << "⚔️  THE STACK SLAYER AWAKENS! ⚔️\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        size_t victims = stack.size();
        
        // Dramatically clear the stack
        for (size_t i = 0; i < victims; ++i) {
            std::cout << "💀 ";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << "\n";
        
        while (!stack.empty()) stack.pop();
        
        std::cout << "The Stack Slayer has claimed " << victims 
                 << " victims. The stack lies empty.\n";
    };
    
    (*op_table)["resurrect"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "✨ Attempting resurrection ritual...\n";
        
        // Resurrect with mystical values
        woflang::WofValue pi, e, phi;
        pi.d = 3.14159;   // π
        e.d = 2.71828;    // e
        phi.d = 1.61803;  // φ (golden ratio)
        
        stack.push(pi);
        stack.push(e);
        stack.push(phi);
        
        std::cout << "Three sacred constants have risen from the void.\n";
    };
}
