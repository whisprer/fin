// ==================================================
// FIXED: prophecy_op.cpp - Simple extern C style
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <vector>

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    (*op_table)["prophecy"] = [](std::stack<woflang::WofValue>& stack) {
        static const std::vector<std::string> prophecies = {
            "The stack shall overflow with wisdom.",
            "A great recursion approaches.",
            "Beware the null pointer of destiny.",
            "The garbage collector comes for us all.",
            "In the end, all returns to void.",
            "The algorithm of fate is O(∞).",
            "Your code compiles, but at what cost?",
            "The segfault was within you all along.",
            "Stack and heap, forever in balance.",
            "The undefined behavior defines us."
        };
        
        static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_int_distribution<> dis(0, prophecies.size() - 1);
        
        std::cout << "\n🔮 The Oracle speaks:\n";
        std::cout << "   \"" << prophecies[dis(gen)] << "\"\n\n";
        
        woflang::WofValue val;
        val.d = 42.0;
        stack.push(val);
    };
    
    (*op_table)["oracle"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "The Oracle requires an offering.\n";
            return;
        }
        
        auto offering = stack.top(); stack.pop();
        
        std::cout << "The Oracle contemplates your offering of " 
                 << offering.as_numeric() << "...\n";
        
        double divination = offering.as_numeric();
        divination = std::sin(divination) * std::cos(divination * 3.14159);
        
        std::cout << "The Oracle reveals: " << divination << "\n";
        woflang::WofValue result;
        result.d = divination;
        stack.push(result);
    };
}
