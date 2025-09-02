// ==================================================
// math_ops.cpp - Enhanced Mathematical Operations (Preserves Core Ops)
// ==================================================
#include "core/woflang.hpp"
#include <cmath>
#include <iostream>
#include <limits>
#include <iomanip>

// Define constants if not available
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_E
#define M_E 2.71828182845904523536
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
    // First, ensure we have the core display operation
    // This preserves the stack display functionality
    (*op_table)["."] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Stack empty" << std::endl;
            return;
        }
        
        // Display top of stack without popping it
        std::cout << std::fixed << std::setprecision(6) << stack.top().d << std::endl;
    };
    
    // Stack display that shows entire stack (useful for debugging)
    (*op_table)[".s"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Stack: <empty>" << std::endl;
            return;
        }
        
        std::cout << "Stack (top to bottom): ";
        std::stack<woflang::WofValue> temp;
        
        // Copy stack to temp to preserve original
        std::stack<woflang::WofValue> copy = stack;
        while (!copy.empty()) {
            temp.push(copy.top());
            copy.pop();
        }
        
        // Display from top to bottom
        bool first = true;
        while (!temp.empty()) {
            if (!first) std::cout << " ";
            std::cout << std::fixed << std::setprecision(6) << temp.top().d;
            temp.pop();
            first = false;
        }
        std::cout << std::endl;
    };
    
    // Basic Stack Operations
    (*op_table)["dup"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on dup");
        }
        stack.push(stack.top());
    };
    
    (*op_table)["drop"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on drop");
        }
        stack.pop();
    };
    
    (*op_table)["swap"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on swap");
        }
        auto a = stack.top(); stack.pop();
        auto b = stack.top(); stack.pop();
        stack.push(a);
        stack.push(b);
    };
    
    (*op_table)["over"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on over");
        }
        auto a = stack.top(); stack.pop();
        auto b = stack.top(); 
        stack.push(a);
        stack.push(b);
    };
    
    (*op_table)["rot"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 3) {
            throw std::runtime_error("Stack underflow on rot");
        }
        auto a = stack.top(); stack.pop();
        auto b = stack.top(); stack.pop();
        auto c = stack.top(); stack.pop();
        stack.push(b);
        stack.push(a);
        stack.push(c);
    };
    
    // Arithmetic Operations
    (*op_table)["+"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on +");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = a.d + b.d;
        stack.push(result);
    };
    
    (*op_table)["-"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on -");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = a.d - b.d;
        stack.push(result);
    };
    
    (*op_table)["*"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on *");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = a.d * b.d;
        stack.push(result);
    };
    
    (*op_table)["/"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on /");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        if (b.d == 0.0) {
            throw std::runtime_error("Division by zero");
        }
        
        woflang::WofValue result;
        result.d = a.d / b.d;
        stack.push(result);
    };
    
    // Modulo operation
    (*op_table)["mod"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on mod");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        if (b.d == 0.0) {
            throw std::runtime_error("Division by zero in mod");
        }
        
        woflang::WofValue result;
        result.d = std::fmod(a.d, b.d);
        stack.push(result);
    };
    
    // Power and roots
    (*op_table)["pow"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on pow");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::pow(a.d, b.d);
        stack.push(result);
    };
    
    (*op_table)["sqrt"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on sqrt");
        }
        auto a = stack.top(); stack.pop();
        
        if (a.d < 0.0) {
            throw std::runtime_error("Cannot take square root of negative value");
        }
        
        woflang::WofValue result;
        result.d = std::sqrt(a.d);
        stack.push(result);
    };
    
    (*op_table)["cbrt"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on cbrt");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::cbrt(a.d);
        stack.push(result);
    };
    
    // Trigonometric functions (radians)
    (*op_table)["sin"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on sin");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::sin(a.d);
        stack.push(result);
    };
    
    (*op_table)["cos"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on cos");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::cos(a.d);
        stack.push(result);
    };
    
    (*op_table)["tan"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on tan");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::tan(a.d);
        stack.push(result);
    };
    
    // Inverse trigonometric functions
    (*op_table)["asin"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on asin");
        }
        auto a = stack.top(); stack.pop();
        
        if (a.d < -1.0 || a.d > 1.0) {
            throw std::runtime_error("Domain error: asin argument must be in [-1, 1]");
        }
        
        woflang::WofValue result;
        result.d = std::asin(a.d);
        stack.push(result);
    };
    
    (*op_table)["acos"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on acos");
        }
        auto a = stack.top(); stack.pop();
        
        if (a.d < -1.0 || a.d > 1.0) {
            throw std::runtime_error("Domain error: acos argument must be in [-1, 1]");
        }
        
        woflang::WofValue result;
        result.d = std::acos(a.d);
        stack.push(result);
    };
    
    (*op_table)["atan"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on atan");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::atan(a.d);
        stack.push(result);
    };
    
    // Degree conversion helpers
    (*op_table)["deg2rad"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on deg2rad");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = a.d * M_PI / 180.0;
        stack.push(result);
    };
    
    (*op_table)["rad2deg"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on rad2deg");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = a.d * 180.0 / M_PI;
        stack.push(result);
    };
    
    // Hyperbolic functions
    (*op_table)["sinh"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on sinh");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::sinh(a.d);
        stack.push(result);
    };
    
    (*op_table)["cosh"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on cosh");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::cosh(a.d);
        stack.push(result);
    };
    
    (*op_table)["tanh"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on tanh");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::tanh(a.d);
        stack.push(result);
    };
    
    // Logarithmic functions
    (*op_table)["ln"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on ln");
        }
        auto a = stack.top(); stack.pop();
        
        if (a.d <= 0.0) {
            throw std::runtime_error("Domain error: ln argument must be positive");
        }
        
        woflang::WofValue result;
        result.d = std::log(a.d);
        stack.push(result);
    };
    
    (*op_table)["log10"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on log10");
        }
        auto a = stack.top(); stack.pop();
        
        if (a.d <= 0.0) {
            throw std::runtime_error("Domain error: log10 argument must be positive");
        }
        
        woflang::WofValue result;
        result.d = std::log10(a.d);
        stack.push(result);
    };
    
    (*op_table)["log2"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on log2");
        }
        auto a = stack.top(); stack.pop();
        
        if (a.d <= 0.0) {
            throw std::runtime_error("Domain error: log2 argument must be positive");
        }
        
        woflang::WofValue result;
        result.d = std::log2(a.d);
        stack.push(result);
    };
    
    (*op_table)["exp"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on exp");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::exp(a.d);
        stack.push(result);
    };
    
    // Utility functions
    (*op_table)["abs"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on abs");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::abs(a.d);
        stack.push(result);
    };
    
    (*op_table)["floor"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on floor");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::floor(a.d);
        stack.push(result);
    };
    
    (*op_table)["ceil"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on ceil");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::ceil(a.d);
        stack.push(result);
    };
    
    (*op_table)["round"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on round");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::round(a.d);
        stack.push(result);
    };
    
    (*op_table)["trunc"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on trunc");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::trunc(a.d);
        stack.push(result);
    };
    
    // Sign function
    (*op_table)["sign"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("Stack underflow on sign");
        }
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = (a.d > 0.0) ? 1.0 : (a.d < 0.0) ? -1.0 : 0.0;
        stack.push(result);
    };
    
    // Min/Max functions
    (*op_table)["min"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on min");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::min(a.d, b.d);
        stack.push(result);
    };
    
    (*op_table)["max"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("Stack underflow on max");
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        
        woflang::WofValue result;
        result.d = std::max(a.d, b.d);
        stack.push(result);
    };
    
    // Constants
    (*op_table)["pi"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = M_PI;
        stack.push(result);
    };
    
    (*op_table)["e"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = M_E;
        stack.push(result);
    };
    
    (*op_table)["tau"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = 2.0 * M_PI;
        stack.push(result);
    };
    
    (*op_table)["phi"] = [](std::stack<woflang::WofValue>& stack) {
        woflang::WofValue result;
        result.d = (1.0 + std::sqrt(5.0)) / 2.0;  // Golden ratio
        stack.push(result);
    };
}
