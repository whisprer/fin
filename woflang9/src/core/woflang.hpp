#pragma once
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <functional>
#include <filesystem>
#include <memory>
#include <variant>

namespace woflang {

// Enhanced WofValue with proper methods that plugins expect
struct WofValue {
    // Core data storage
    int i = 0;
    double d = 0.0;
    std::string s;
    
    // Constructors for convenience
    WofValue() = default;
    WofValue(double val) : i(0), d(val) {}
    WofValue(int val) : i(val), d(0.0) {}
    WofValue(const std::string& val) : i(0), d(0.0), s(val) {}
    WofValue(const char* val) : i(0), d(0.0), s(val) {}
    
    // Plugin-expected methods
    bool is_numeric() const {
        return d != 0.0 || i != 0;
    }
    
    bool is_string() const {
        return !s.empty();
    }
    
    double as_numeric() const {
        if (d != 0.0) return d;
        if (i != 0) return static_cast<double>(i);
        return 0.0;
    }
    
    std::string to_string() const {
        if (!s.empty()) return s;
        if (d != 0.0) return std::to_string(d);
        if (i != 0) return std::to_string(i);
        return "0";
    }
};

// Plugin base class for the more complex plugins
class WoflangPlugin {
public:
    virtual ~WoflangPlugin() = default;
    virtual void register_ops(class WoflangInterpreter& interp) = 0;
};

class WoflangInterpreter {
public:
    using OpHandler = std::function<void(std::stack<WofValue>&)>;
    using OpTable = std::map<std::string, OpHandler>;

    WoflangInterpreter();

    void register_op(const std::string& name, OpHandler handler);
    void execute_line(const std::string& code);
    void loadPlugin(const std::string& path);
    void load_plugins(const std::filesystem::path& plugin_dir);

    // Stack access for plugin compatibility
    std::stack<WofValue> stack;
    
    // Helper methods that some plugins might expect
    void push(const WofValue& val) { stack.push(val); }
    WofValue pop() { 
        if (stack.empty()) throw std::runtime_error("Stack underflow");
        auto val = stack.top(); 
        stack.pop(); 
        return val; 
    }
    void clear_stack() { while (!stack.empty()) stack.pop(); }

private:
    OpTable op_table_;
};

} // namespace woflang

// Plugin export macro for Windows/cross-platform compatibility
#ifdef _WIN32
    #define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#else
    #define WOFLANG_PLUGIN_EXPORT extern "C"
#endif