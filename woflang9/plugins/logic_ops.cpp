// ==================================================
// 4. logic_ops.cpp - Boolean Logic Operations
// ==================================================
#include "core/woflang.hpp"
#include <iostream>

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    auto to_bool = [](const woflang::WofValue& val) -> bool {
        return val.d != 0.0;
    };
    
    // Boolean Operations
    (*op_table)["and"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("and requires two values");
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        bool result = to_bool(a) && to_bool(b);
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    (*op_table)["or"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("or requires two values");
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        bool result = to_bool(a) || to_bool(b);
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    (*op_table)["xor"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("xor requires two values");
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        bool result = to_bool(a) != to_bool(b);
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    (*op_table)["not"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("not requires a value");
        }
        
        auto a = stack.top(); stack.pop();
        
        bool result = !to_bool(a);
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    (*op_table)["implies"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("implies requires two values");
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        bool result = !to_bool(a) || to_bool(b);
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    (*op_table)["equivalent"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("equivalent requires two values");
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        bool result = to_bool(a) == to_bool(b);
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    (*op_table)["nand"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("nand requires two values");
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        bool result = !(to_bool(a) && to_bool(b));
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    (*op_table)["nor"] = [to_bool](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("nor requires two values");
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        bool result = !(to_bool(a) || to_bool(b));
        
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };
    
    // Educational operations
    (*op_table)["tautology"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "tautology demo: A OR NOT A\n";
        
        // Demonstrate A OR NOT A is always true
        for (int i = 0; i < 2; i++) {
            bool a = (i == 1);
            bool not_a = !a;
            bool result = a || not_a;
            
            std::cout << "A=" << (a ? "T" : "F") 
                     << " | NOT A=" << (not_a ? "T" : "F") 
                     << " | A OR NOT A=" << (result ? "T" : "F") << "\n";
        }
        
        std::cout << "This is a tautology - always true!\n";
        
        woflang::WofValue res;
        res.d = 1.0;
        stack.push(res);
    };
    
    (*op_table)["contradiction"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "contradiction demo: A AND NOT A\n";
        
        // Demonstrate A AND NOT A is always false
        for (int i = 0; i < 2; i++) {
            bool a = (i == 1);
            bool not_a = !a;
            bool result = a && not_a;
            
            std::cout << "A=" << (a ? "T" : "F") 
                     << " | NOT A=" << (not_a ? "T" : "F") 
                     << " | A AND NOT A=" << (result ? "T" : "F") << "\n";
        }
        
        std::cout << "This is a contradiction - always false!\n";
        
        woflang::WofValue res;
        res.d = 0.0;
        stack.push(res);
    };
}

