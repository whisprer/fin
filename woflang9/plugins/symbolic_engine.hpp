/**
 * @file symbolic_engine.hpp
 * @brief Symbolic math operations for Woflang
 * 
 * This file provides basic symbolic mathematics operations, including
 * pattern matching, simplification rules, and unit handling.
 */

#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {

#pragma once

#include "core/woflang.hpp"
#include <regex>
#include <unordered_map>
#include <iostream>
#include <optional>
#include <cmath>

namespace woflang {

/**
 * @brief A symbolic pattern rule for pattern matching and simplification
 */
struct SymbolicPattern {
    std::vector<WofType> pattern;      ///< Pattern of value types to match
    std::vector<std::string> symbols;  ///< Symbol names to match (for Symbol types)
    
    // Handler that processes matched values and returns whether pattern was applied
    std::function<bool(WoflangInterpreter&, const std::vector<WofValue>&)> handler;
};

/**
 * @brief Unit representation for physical quantities
 */
class Unit {
public:
    std::string base_unit;                    ///< Base unit name (e.g., "m", "kg")
    std::unordered_map<std::string, int> dimensions;  ///< Dimensions with exponents
    
    /// Default constructor for dimensionless unit
    Unit() = default;
    
    /// Constructor with base unit
    explicit Unit(const std::string& base) : base_unit(base) {
        dimensions[base] = 1;
    }
    
    /// Multiply two units
    Unit operator*(const Unit& other) const {
        Unit result;
        result.dimensions = dimensions;
        
        // Add other unit's dimensions
        for (const auto& [dim, exp] : other.dimensions) {
            result.dimensions[dim] += exp;
        }
        
        // Cleanup: remove dimensions with zero exponent
        for (auto it = result.dimensions.begin(); it != result.dimensions.end();) {
            if (it->second == 0) {
                it = result.dimensions.erase(it);
            } else {
                ++it;
            }
        }
        
        // Format the base unit for display
        result.format_base_unit();
        
        return result;
    }
    
    /// Divide two units
    Unit operator/(const Unit& other) const {
        Unit result;
        result.dimensions = dimensions;
        
        // Subtract other unit's dimensions
        for (const auto& [dim, exp] : other.dimensions) {
            result.dimensions[dim] -= exp;
        }
        
        // Cleanup: remove dimensions with zero exponent
        for (auto it = result.dimensions.begin(); it != result.dimensions.end();) {
            if (it->second == 0) {
                it = result.dimensions.erase(it);
            } else {
                ++it;
            }
        }
        
        // Format the base unit for display
        result.format_base_unit();
        
        return result;
    }
    
    /// Raise a unit to a power
    Unit pow(int exponent) const {
        Unit result;
        
        // Multiply all exponents
        for (const auto& [dim, exp] : dimensions) {
            result.dimensions[dim] = exp * exponent;
        }
        
        // Cleanup: remove dimensions with zero exponent
        for (auto it = result.dimensions.begin(); it != result.dimensions.end();) {
            if (it->second == 0) {
                it = result.dimensions.erase(it);
            } else {
                ++it;
            }
        }
        
        // Format the base unit for display
        result.format_base_unit();
        
        return result;
    }
    
    /// Format the unit as a string
    [[nodiscard]] std::string to_string() const {
        return base_unit;
    }

private:
    /// Format the base unit from dimensions
    void format_base_unit() {
        base_unit.clear();
        bool first = true;
        
        // Numerator (positive exponents)
        for (const auto& [dim, exp] : dimensions) {
            if (exp > 0) {
                if (!first) {
                    base_unit += "·";
                }
                base_unit += dim;
                if (exp > 1) {
                    base_unit += "^" + std::to_string(exp);
                }
                first = false;
            }
        }
        
        // If no positive exponents, start with 1
        if (first) {
            base_unit = "1";
            first = false;
        }
        
        // Denominator (negative exponents)
        bool has_neg = false;
        for (const auto& [dim, exp] : dimensions) {
            if (exp < 0) {
                has_neg = true;
                break;
            }
        }
        
        if (has_neg) {
            base_unit += "/";
            first = true;
            for (const auto& [dim, exp] : dimensions) {
                if (exp < 0) {
                    if (!first) {
                        base_unit += "·";
                    }
                    base_unit += dim;
                    if (exp < -1) {
                        base_unit += "^" + std::to_string(-exp);
                    }
                    first = false;
                }
            }
        }
    }
};

/**
 * @brief Add a unit to a value
 * @param interp Interpreter instance
 * @param value Value to add unit to
 * @param unit_str Unit name
 * @return Value with unit
 */
inline WofValue add_unit(const WofValue& value, const std::string& unit_str) {
    WofValue result = value;
    result.unit = unit_str;
    return result;
}

/**
 * @brief Register symbolic math operations with the interpreter
 * @param interp Interpreter instance
 */
inline void register_symbolic_ops(WoflangInterpreter& interp) {
    // ----- Symbolic simplification patterns -----
    
    // Keep track of pattern rules
    static std::vector<SymbolicPattern> simplify_patterns;
    
    // Add default simplify patterns
    
    // Pattern: X + X -> 2 * X
    simplify_patterns.push_back({
        {WofType::Symbol, WofType::Symbol, WofType::Symbol},  // Types: symbol symbol op
        {"", "", "+"},                              // Match + operator
        [](WoflangInterpreter& interp, const std::vector<WofValue>& values) -> bool {
            // Check if both symbols match and op is +
            if (values.size() == 3 && 
                values[0].type == WofType::Symbol && 
                values[1].type == WofType::Symbol &&
                values[2].type == WofType::Symbol &&
                std::get<std::string>(values[0].value) == std::get<std::string>(values[1].value) &&
                std::get<std::string>(values[2].value) == "+") {
                // Replace with 2 * X
                interp.stack.emplace_back(2LL);
                interp.stack.push_back(values[0]);
                interp.stack.emplace_back("*", WofType::Symbol);
                return true;
            }
            return false;
        }
    });
    
    // Pattern: X * 1 -> X
    simplify_patterns.push_back({
        {WofType::Symbol, WofType::Integer, WofType::Symbol},  // Types: symbol integer op
        {"", "", "*"},                                // Match * operator
        [](WoflangInterpreter& interp, const std::vector<WofValue>& values) -> bool {
            // Check if constant is 1 and op is *
            if (values.size() == 3 && 
                values[0].type == WofType::Symbol && 
                values[1].type == WofType::Integer &&
                values[2].type == WofType::Symbol &&
                std::get<int64_t>(values[1].value) == 1 &&
                std::get<std::string>(values[2].value) == "*") {
                // Replace with X
                interp.stack.push_back(values[0]);
                return true;
            }
            return false;
        }
    });
    
    // Pattern: X + 0 -> X
    simplify_patterns.push_back({
        {WofType::Symbol, WofType::Integer, WofType::Symbol},  // Types: symbol integer op
        {"", "", "+"},                                // Match + operator
        [](WoflangInterpreter& interp, const std::vector<WofValue>& values) -> bool {
            // Check if constant is 0 and op is +
            if (values.size() == 3 && 
                values[0].type == WofType::Symbol && 
                values[1].type == WofType::Integer &&
                values[2].type == WofType::Symbol &&
                std::get<int64_t>(values[1].value) == 0 &&
                std::get<std::string>(values[2].value) == "+") {
                // Replace with X
                interp.stack.push_back(values[0]);
                return true;
            }
            return false;
        }
    });
    
    // Pattern: X * 0 -> 0
    simplify_patterns.push_back({
        {WofType::Symbol, WofType::Integer, WofType::Symbol},  // Types: symbol integer op
        {"", "", "*"},                                // Match * operator
        [](WoflangInterpreter& interp, const std::vector<WofValue>& values) -> bool {
            // Check if constant is 0 and op is *
            if (values.size() == 3 && 
                values[0].type == WofType::Symbol && 
                values[1].type == WofType::Integer &&
                values[2].type == WofType::Symbol &&
                std::get<int64_t>(values[1].value) == 0 &&
                std::get<std::string>(values[2].value) == "*") {
                // Replace with 0
                interp.stack.emplace_back(0LL);
                return true;
            }
            return false;
        }
    });
    
    // simplify: Apply symbolic simplification patterns
    interp.register_op("simplify", [&simplify_patterns](WoflangInterpreter& interp) {
        if (interp.stack.size() < 3) {
            return;  // Need at least 3 elements for most patterns
        }
        
        // Copy the stack for pattern matching
        auto stack_copy = interp.stack;
        
        // Try to apply each pattern
        for (const auto& pattern : simplify_patterns) {
            if (stack_copy.size() >= pattern.pattern.size()) {
                // Extract the relevant part of the stack
                std::vector<WofValue> values(
                    stack_copy.end() - pattern.pattern.size(),
                    stack_copy.end()
                );
                
                // Check if types match
                bool match = true;
                for (size_t i = 0; i < pattern.pattern.size(); ++i) {
                    if (values[i].type != pattern.pattern[i]) {
                        match = false;
                        break;
                    }
                }
                
                // Check if symbols match (if specified)
                if (match) {
                    for (size_t i = 0; i < pattern.symbols.size(); ++i) {
                        if (!pattern.symbols[i].empty() &&
                            (values[i].type != WofType::Symbol ||
                             std::get<std::string>(values[i].value) != pattern.symbols[i])) {
                            match = false;
                            break;
                        }
                    }
                }
                
                // Apply the pattern if matched
                if (match) {
                    // Remove the matched elements from the stack
                    for (size_t i = 0; i < pattern.pattern.size(); ++i) {
                        interp.stack.pop_back();
                    }
                    
                    // Apply the pattern handler
                    if (pattern.handler(interp, values)) {
                        return;  // Pattern applied, we're done
                    }
                    
                    // If pattern handler returned false, restore the stack
                    for (auto it = values.rbegin(); it != values.rend(); ++it) {
                        interp.stack.push_back(*it);
                    }
                }
            }
        }
    });
    
    // ----- Symbolic equation solving -----
    
    // solve_linear: Solve a simple linear equation (e.g., 2 X = 4)
    interp.register_op("solve_linear", [](WoflangInterpreter& interp) {
        if (interp.stack.size() < 4) {
            std::cout << "solve_linear: needs 'coefficient variable = value'\n";
            return;
        }
        
        // Pop the values from the stack in reverse order
        auto rhs = interp.stack.back(); interp.stack.pop_back();         // e.g., 4
        auto eq_op = interp.stack.back(); interp.stack.pop_back();       // e.g., "="
        auto var = interp.stack.back(); interp.stack.pop_back();         // e.g., "X"
        auto coeff = interp.stack.back(); interp.stack.pop_back();       // e.g., 2
        
        // Check if the types match the expected pattern
        if (coeff.type != WofType::Integer && coeff.type != WofType::Double) {
            std::cout << "solve_linear: coefficient must be a number\n";
            interp.stack.push_back(coeff);
            interp.stack.push_back(var);
            interp.stack.push_back(eq_op);
            interp.stack.push_back(rhs);
            return;
        }
        
        if (var.type != WofType::Symbol) {
            std::cout << "solve_linear: variable must be a symbol\n";
            interp.stack.push_back(coeff);
            interp.stack.push_back(var);
            interp.stack.push_back(eq_op);
            interp.stack.push_back(rhs);
            return;
        }
        
        if (eq_op.type != WofType::Symbol || std::get<std::string>(eq_op.value) != "=") {
            std::cout << "solve_linear: expected '=' operator\n";
            interp.stack.push_back(coeff);
            interp.stack.push_back(var);
            interp.stack.push_back(eq_op);
            interp.stack.push_back(rhs);
            return;
        }
        
        if (rhs.type != WofType::Integer && rhs.type != WofType::Double) {
            std::cout << "solve_linear: right-hand side must be a number\n";
            interp.stack.push_back(coeff);
            interp.stack.push_back(var);
            interp.stack.push_back(eq_op);
            interp.stack.push_back(rhs);
            return;
        }
        
        // Get numeric values
        double coeff_val = coeff.type == WofType::Integer ? 
            static_cast<double>(std::get<int64_t>(coeff.value)) : 
            std::get<double>(coeff.value);
        
        double rhs_val = rhs.type == WofType::Integer ? 
            static_cast<double>(std::get<int64_t>(rhs.value)) : 
            std::get<double>(rhs.value);
        
        // Check for division by zero
        if (coeff_val == 0.0) {
            std::cout << "solve_linear: coefficient cannot be zero\n";
            interp.stack.push_back(coeff);
            interp.stack.push_back(var);
            interp.stack.push_back(eq_op);
            interp.stack.push_back(rhs);
            return;
        }
        
        // Solve: coeff * var = rhs => var = rhs / coeff
        double result = rhs_val / coeff_val;
        
        // Display the result
        std::string var_name = std::get<std::string>(var.value);
        std::cout << var_name << " = ";
        
        // Store as integer if it's an exact integer
        if (result == std::floor(result)) {
            std::cout << static_cast<int64_t>(result) << std::endl;
            interp.stack.emplace_back(static_cast<int64_t>(result));
        } else {
            std::cout << result << std::endl;
            interp.stack.emplace_back(result);
        }
    });
    
    // ----- Unit operations -----
    
    // unit: Add a unit to a value (e.g., 5 m unit -> 5 m)
    interp.register_op("unit", [](WoflangInterpreter& interp) {
        if (interp.stack.size() < 2) {
            std::cout << "unit: needs 'value unit_name'\n";
            return;
        }
        
        auto unit = interp.stack.back(); interp.stack.pop_back();
        auto value = interp.stack.back(); interp.stack.pop_back();
        
        if (unit.type != WofType::Symbol && unit.type != WofType::String) {
            std::cout << "unit: unit name must be a symbol or string\n";
            interp.stack.push_back(value);
            interp.stack.push_back(unit);
            return;
        }
        
        if (value.type != WofType::Integer && value.type != WofType::Double) {
            std::cout << "unit: value must be a number\n";
            interp.stack.push_back(value);
            interp.stack.push_back(unit);
            return;
        }
        
        // Get the unit name
        std::string unit_name = std::get<std::string>(unit.value);
        
        // Create a new value with unit
        WofValue result = value;
        result.unit = unit_name;
        
        // Push the result back to the stack
        interp.stack.push_back(result);
    });
    
    // mul_unit: Multiply two values with units (e.g., 2 m * 3 m -> 6 m^2)
    interp.register_op("mul_unit", [](WoflangInterpreter& interp) {
        if (interp.stack.size() < 2) {
            std::cout << "mul_unit: needs two values with units\n";
            return;
        }
        
        auto b = interp.stack.back(); interp.stack.pop_back();
        auto a = interp.stack.back(); interp.stack.pop_back();
        
        if ((a.type != WofType::Integer && a.type != WofType::Double) ||
            (b.type != WofType::Integer && b.type != WofType::Double)) {
            std::cout << "mul_unit: both values must be numbers\n";
            interp.stack.push_back(a);
            interp.stack.push_back(b);
            return;
        }
        
        if (!a.unit || !b.unit) {
            std::cout << "mul_unit: both values must have units\n";
            interp.stack.push_back(a);
            interp.stack.push_back(b);
            return;
        }
        
        // Get numeric values
        double a_val = a.type == WofType::Integer ? 
            static_cast<double>(std::get<int64_t>(a.value)) : 
            std::get<double>(a.value);
        
        double b_val = b.type == WofType::Integer ? 
            static_cast<double>(std::get<int64_t>(b.value)) : 
            std::get<double>(b.value);
        
        // Multiply values
        double result = a_val * b_val;
        
        // Combine units
        std::string unit_result;
        if (*a.unit == *b.unit) {
            // Same unit: m * m = m^2
            unit_result = *a.unit + "^2";
        } else {
            // Different units: m * s = m·s
            unit_result = *a.unit + "·" + *b.unit;
        }
        
        // Create result with combined unit
        WofValue result_value(result);
        result_value.unit = unit_result;
        
        // Push the result
        interp.stack.push_back(result_value);
    });
    
    // div_unit: Divide two values with units (e.g., 6 m^2 / 2 m -> 3 m)
    interp.register_op("div_unit", [](WoflangInterpreter& interp) {
        if (interp.stack.size() < 2) {
            std::cout << "div_unit: needs two values with units\n";
            return;
        }
        
        auto b = interp.stack.back(); interp.stack.pop_back();
        auto a = interp.stack.back(); interp.stack.pop_back();
        
        if ((a.type != WofType::Integer && a.type != WofType::Double) ||
            (b.type != WofType::Integer && b.type != WofType::Double)) {
            std::cout << "div_unit: both values must be numbers\n";
            interp.stack.push_back(a);
            interp.stack.push_back(b);
            return;
        }
        
        if (!a.unit) {
            std::cout << "div_unit: first value must have a unit\n";
            interp.stack.push_back(a);
            interp.stack.push_back(b);
            return;
        }
        
        // Get numeric values
        double a_val = a.type == WofType::Integer ? 
            static_cast<double>(std::get<int64_t>(a.value)) : 
            std::get<double>(a.value);
        
        double b_val = b.type == WofType::Integer ? 
            static_cast<double>(std::get<int64_t>(b.value)) : 
            std::get<double>(b.value);
        
        // Check for division by zero
        if (b_val == 0.0) {
            std::cout << "div_unit: division by zero\n";
            interp.stack.push_back(a);
            interp.stack.push_back(b);
            return;
        }
        
        // Divide values
        double result = a_val / b_val;
        
        // Combine units
        std::string unit_result = *a.unit;
        if (b.unit) {
            if (*a.unit == *b.unit) {
                // Same unit: m^2 / m = m
                // Simple heuristic for removing exponents
                if (unit_result.find("^2") != std::string::npos) {
                    unit_result = unit_result.substr(0, unit_result.find("^2"));
                } else if (unit_result.find("^") != std::string::npos) {
                    // More complex case, would need proper parsing
                    unit_result = unit_result + "/" + *b.unit;
                } else {
                    // No exponent, units cancel
                    unit_result = "";
                }
            } else {
                // Different units: m / s = m/s
                unit_result = *a.unit + "/" + *b.unit;
            }
        }
        
        // Create result with combined unit
        WofValue result_value(result);
        if (!unit_result.empty()) {
            result_value.unit = unit_result;
        }
        
        // Push the result
        interp.stack.push_back(result_value);
    });
}
