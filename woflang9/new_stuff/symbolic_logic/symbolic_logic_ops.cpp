// symbolic_ops.cpp - Woflang Symbolic Mathematics Plugin for v1.1.0

#include "../../src/core/woflang.hpp"
#include <iostream>
#include <stack>
#include <stdexcept>

extern "C" {

// Helper to convert to boolean
bool to_bool(const woflang::WofValue& val) {
    return val.d != 0.0;
}

void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    // Logical Operations
    (*op_table)["forall"] = [](std::stack<woflang::WofValue>& stack) {
        throw std::runtime_error("forall quantifier evaluation not implemented in stack mode");
    };

    (*op_table)["exists"] = [](std::stack<woflang::WofValue>& stack) {
        throw std::runtime_error("exists quantifier evaluation not implemented in stack mode");
    };

    (*op_table)["implies"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) throw std::runtime_error("implies requires two values");
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        bool result = !to_bool(a) || to_bool(b);
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };

    (*op_table)["iff"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) throw std::runtime_error("iff requires two values");
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        bool result = to_bool(a) == to_bool(b);
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };

    (*op_table)["and"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) throw std::runtime_error("and requires two values");
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        bool result = to_bool(a) && to_bool(b);
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };

    (*op_table)["or"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) throw std::runtime_error("or requires two values");
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        bool result = to_bool(a) || to_bool(b);
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };

    (*op_table)["not"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) throw std::runtime_error("not requires a value");
        auto a = stack.top(); stack.pop();
        bool result = !to_bool(a);
        woflang::WofValue res;
        res.d = result ? 1.0 : 0.0;
        stack.push(res);
    };

    // Educational operations for demo
    (*op_table)["tautology_demo"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "tautology demo: A OR NOT A\n";
        for (int i = 0; i < 2; i++) {
            bool a = (i == 1);
            bool not_a = !a;
            bool result = a || not_a;
            std::cout << "A=" << (a ? "T" : "F") << " | NOT A=" << (not_a ? "T" : "F") << " | A OR NOT A=" << (result ? "T" : "F") << "\n";
        }
        std::cout << "This is a tautology - always true!\n";
        woflang::WofValue res;
        res.d = 1.0;
        stack.push(res);
    };

    (*op_table)["contradiction_demo"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "contradiction demo: A AND NOT A\n";
        for (int i = 0; i < 2; i++) {
            bool a = (i == 1);
            bool not_a = !a;
            bool result = a && not_a;
            std::cout << "A=" << (a ? "T" : "F") << " | NOT A=" << (not_a ? "T" : "F") << " | A AND NOT A=" << (result ? "T" : "F") << "\n";
        }
        std::cout << "This is a contradiction - always false!\n";
        woflang::WofValue res;
        res.d = 0.0;
        stack.push(res);
    };

    std::cout << "symbolic_ops plugin loaded.\n";
}

} // extern "C"