// ===================================== 
// YET ANOTHER greek_math_op.cpp FIX
// =====================================
#include "core/woflang.hpp"
#include <iostream>
#include <cmath>
#include <limits>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

extern "C" {

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    // π (pi) - Mathematical constant - multiple ways to access
    (*op_table)["π"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue val;
        val.d = M_PI;
        stack.push(val);
        std::cout << "π = " << M_PI << "\n";
    };
    
    (*op_table)["PI"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue val;
        val.d = M_PI;
        stack.push(val);
        std::cout << "π = " << M_PI << "\n";
    };
    
    (*op_table)["pi"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue val;
        val.d = M_PI;
        stack.push(val);
        std::cout << "π = " << M_PI << "\n";
    };
    
    // Σ (sigma) - Summation - multiple ways to access
    (*op_table)["Σ"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Σ: Stack is empty\n";
            return;
        }
        
        double sum = 0.0;
        while (!stack.empty()) {
            auto val = stack.top(); stack.pop();
            sum += val.as_numeric();
        }
        
        woflang::WofValue result;
        result.d = sum;
        stack.push(result);
        std::cout << "Σ = " << sum << "\n";
    };
    
    (*op_table)["sum"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "sum: Stack is empty\n";
            return;
        }
        
        double sum = 0.0;
        while (!stack.empty()) {
            auto val = stack.top(); stack.pop();
            sum += val.as_numeric();
        }
        
        woflang::WofValue result;
        result.d = sum;
        stack.push(result);
        std::cout << "sum = " << sum << "\n";
    };
    
    // Π (pi) - Product
    (*op_table)["Π"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Π: Stack is empty\n";
            return;
        }
        
        double product = 1.0;
        while (!stack.empty()) {
            auto val = stack.top(); stack.pop();
            product *= val.as_numeric();
        }
        
        woflang::WofValue result;
        result.d = product;
        stack.push(result);
        std::cout << "Π = " << product << "\n";
    };
    
    (*op_table)["product"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "product: Stack is empty\n";
            return;
        }
        
        double product = 1.0;
        while (!stack.empty()) {
            auto val = stack.top(); stack.pop();
            product *= val.as_numeric();
        }
        
        woflang::WofValue result;
        result.d = product;
        stack.push(result);
        std::cout << "product = " << product << "\n";
    };
    
    // Δ (delta) - Difference
    (*op_table)["Δ"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "Δ: Need at least 2 values\n";
            return;
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        double delta = std::abs(a.as_numeric() - b.as_numeric());
        woflang::WofValue result;
        result.d = delta;
        stack.push(result);
        std::cout << "Δ = " << delta << "\n";
    };
    
    (*op_table)["delta"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "delta: Need at least 2 values\n";
            return;
        }
        
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        double delta = std::abs(a.as_numeric() - b.as_numeric());
        woflang::WofValue result;
        result.d = delta;
        stack.push(result);
        std::cout << "delta = " << delta << "\n";
    };
    
    // √ (square root)
    (*op_table)["√"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "√: Stack underflow\n";
            return;
        }
        
        auto val = stack.top(); stack.pop();
        double x = val.as_numeric();
        if (x >= 0) {
            double result = std::sqrt(x);
            woflang::WofValue res;
            res.d = result;
            stack.push(res);
            std::cout << "√" << x << " = " << result << "\n";
        } else {
            std::cout << "√: Cannot take square root of negative number\n";
            stack.push(val); // Put it back
        }
    };
    
    // ∞ (infinity)
    (*op_table)["∞"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue val;
        val.d = std::numeric_limits<double>::infinity();
        stack.push(val);
        std::cout << "∞: Infinity pushed to stack\n";
    };
    
    (*op_table)["inf"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue val;
        val.d = std::numeric_limits<double>::infinity();
        stack.push(val);
        std::cout << "inf: Infinity pushed to stack\n";
    };
    
    (*op_table)["infinity"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue val;
        val.d = std::numeric_limits<double>::infinity();
        stack.push(val);
        std::cout << "infinity: Infinity pushed to stack\n";
    };
    
    // ∅ (empty set / void)
    (*op_table)["∅"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "∅: The void consumes all. Stack cleared.\n";
        while (!stack.empty()) stack.pop();
    };
    
    (*op_table)["void"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "void: The void consumes all. Stack cleared.\n";
        while (!stack.empty()) stack.pop();
    };
    
    (*op_table)["empty"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "empty: Stack cleared.\n";
        while (!stack.empty()) stack.pop();
    };
}
