#include "woflang.hpp"
#include "../io/tokenizer.hpp"
#include <iostream>
#include <sstream>
#include <cctype>
#include <cmath>
#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

namespace woflang {

WoflangInterpreter::WoflangInterpreter() {
    // Register built-in ops and eggs
    register_op("cursed", [](std::stack<WofValue>& stack) {
        std::cout << "You have invoked the ancient Woflang curse! 👻\n";
    });
    register_op("egg", [](std::stack<WofValue>& stack) {
        std::cout << "🥚 You found the Woflang Easter Egg!\n";
    });
    
    // Essential built-in operations
    register_op("exit", [](std::stack<WofValue>& stack) {
        std::cout << "Goodbye from Woflang! 🐺\n";
        std::exit(0);
    });
    
    register_op(".", [](std::stack<WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Stack is empty\n";
            return;
        }
        auto val = stack.top();
        stack.pop();
        // Safe output without using potentially broken to_string()
        if (!val.s.empty()) {
            std::cout << val.s << "\n";
        } else if (val.d != 0.0) {
            std::cout << val.d << "\n";
        } else {
            std::cout << val.i << "\n";
        }
    });
    
    register_op("stack", [](std::stack<WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Stack: [empty]\n";
            return;
        }
        
        std::cout << "Stack (top to bottom): ";
        std::stack<WofValue> temp = stack;
        std::vector<std::string> values;
        
        while (!temp.empty()) {
            auto val = temp.top();
            if (!val.s.empty()) {
                values.push_back(val.s);
            } else if (val.d != 0.0) {
                values.push_back(std::to_string(val.d));
            } else {
                values.push_back(std::to_string(val.i));
            }
            temp.pop();
        }
        
        for (size_t i = 0; i < values.size(); ++i) {
            std::cout << values[i];
            if (i < values.size() - 1) std::cout << " ";
        }
        std::cout << "\n";
    });
    
    register_op("clear", [](std::stack<WofValue>& stack) {
        while (!stack.empty()) stack.pop();
        std::cout << "Stack cleared\n";
    });
    
    // Basic arithmetic
    register_op("+", [](std::stack<WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "Error: + requires 2 values\n";
            return;
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        WofValue result;
        result.d = a.as_numeric() + b.as_numeric();
        stack.push(result);
    });
    
    register_op("-", [](std::stack<WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "Error: - requires 2 values\n";
            return;
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        WofValue result;
        result.d = a.as_numeric() - b.as_numeric();
        stack.push(result);
    });
    
    register_op("*", [](std::stack<WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "Error: * requires 2 values\n";
            return;
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        WofValue result;
        result.d = a.as_numeric() * b.as_numeric();
        stack.push(result);
    });
    
    register_op("/", [](std::stack<WofValue>& stack) {
        if (stack.size() < 2) {
            std::cout << "Error: / requires 2 values\n";
            return;
        }
        auto b = stack.top(); stack.pop();
        auto a = stack.top(); stack.pop();
        double b_val = b.as_numeric();
        if (b_val == 0.0) {
            std::cout << "Error: Division by zero\n";
            stack.push(a);
            stack.push(b);
            return;
        }
        WofValue result;
        result.d = a.as_numeric() / b_val;
        stack.push(result);
    });
    
    register_op("sqrt", [](std::stack<WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "Error: sqrt requires 1 value\n";
            return;
        }
        auto a = stack.top(); stack.pop();
        double val = a.as_numeric();
        if (val < 0) {
            std::cout << "Error: sqrt of negative number\n";
            stack.push(a);
            return;
        }
        WofValue result;
        result.d = std::sqrt(val);
        stack.push(result);
    });
    
    register_op("pi", [](std::stack<WofValue>& stack) {
        WofValue pi;
        pi.d = 3.14159265358979323846;
        stack.push(pi);
    });
    
    register_op("π", [](std::stack<WofValue>& stack) {
        WofValue pi;
        pi.d = 3.14159265358979323846;
        stack.push(pi);
    });
}

// Helper function to check if a string is a number
bool is_number(const std::string& str) {
    if (str.empty()) return false;
    
    size_t start = 0;
    if (str[0] == '-' || str[0] == '+') {
        if (str.length() == 1) return false;
        start = 1;
    }
    
    bool has_dot = false;
    for (size_t i = start; i < str.length(); ++i) {
        if (str[i] == '.') {
            if (has_dot) return false; // Multiple dots
            has_dot = true;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}

// Safe number parsing
WofValue parse_number(const std::string& str) {
    WofValue val;
    try {
        if (str.find('.') != std::string::npos) {
            val.d = std::stod(str);
        } else {
            val.i = std::stoi(str);
            val.d = static_cast<double>(val.i);
        }
    } catch (const std::exception&) {
        // If parsing fails, treat as zero
        val.i = 0;
        val.d = 0.0;
    }
    return val;
}

void WoflangInterpreter::register_op(const std::string& name, OpHandler handler) {
    op_table_[name] = handler;
}

void WoflangInterpreter::execute_line(const std::string& line) {
    auto tokens = tokenize(line);
    for (const auto& [type, token] : tokens) {
        try {
            if (is_number(token)) {
                WofValue val = parse_number(token);
                stack.push(val);
            }
            else {
                auto it = op_table_.find(token);
                if (it != op_table_.end()) {
                    it->second(stack);
                } else {
                    std::cout << "Unknown op: " << token << "\n";
                }
            }
        } catch (const std::exception& e) {
            std::cout << "Error executing '" << token << "': " << e.what() << "\n";
        }
    }
}

void WoflangInterpreter::loadPlugin(const std::string& path) {
#ifdef _WIN32
    HMODULE handle = LoadLibraryA(path.c_str());
    if (!handle) {
        std::cout << "Failed to load plugin: " << path << "\n";
        return;
    }
    using InitFunc = void(*)(OpTable*);
    auto init_func = reinterpret_cast<InitFunc>(GetProcAddress(handle, "init_plugin"));
    if (init_func) {
        init_func(&op_table_);
        std::cout << "Loaded plugin: " << path << "\n";
    } else {
        std::cout << "Plugin missing init_plugin function: " << path << "\n";
    }
#else
    void* handle = dlopen(path.c_str(), RTLD_LAZY);
    if (!handle) {
        std::cout << "Failed to load plugin: " << path << " - " << dlerror() << "\n";
        return;
    }
    using InitFunc = void(*)(OpTable*);
    auto init_func = reinterpret_cast<InitFunc>(dlsym(handle, "init_plugin"));
    if (init_func) {
        init_func(&op_table_);
        std::cout << "Loaded plugin: " << path << "\n";
    } else {
        std::cout << "Plugin missing init_plugin function: " << path << "\n";
    }
#endif
}

void WoflangInterpreter::load_plugins(const std::filesystem::path& plugin_dir) {
    if (!std::filesystem::exists(plugin_dir)) {
        std::cout << "Plugin directory doesn't exist: " << plugin_dir << "\n";
        return;
    }
    
    std::cout << "Loading plugins from: " << plugin_dir << "\n";
    for (auto& entry : std::filesystem::directory_iterator(plugin_dir)) {
        if (entry.is_regular_file()) {
            auto path = entry.path();
            // Load .dll files on Windows, .so files on Linux
#ifdef _WIN32
            if (path.extension() == ".dll") {
#else
            if (path.extension() == ".so") {
#endif
                loadPlugin(path.string());
            }
        }
    }
}

}