// plugins/assert_ops.cpp
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

#include "core/woflang.hpp"
#include <cmath>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>

namespace woflang {

static inline double need_num(const WofValue& v, const char* op) {
    if (!v.is_numeric()) throw std::runtime_error(std::string(op) + ": numeric required");
    return v.as_numeric();
}

class AssertOpsPlugin : public WoflangPlugin {
public:
    void register_ops(WoflangInterpreter& I) override {
        // actual expected            expect_eq
        I.register_op("expect_eq", [](std::stack<WofValue>& S){
            if (S.size() < 2) throw std::runtime_error("expect_eq: need actual expected");
            auto expected = S.top(); S.pop();
            auto actual   = S.top(); S.pop();

            if (actual.is_numeric() && expected.is_numeric()) {
                double a = actual.as_numeric();
                double e = expected.as_numeric();
                if (a != e) {
                    throw std::runtime_error("expect_eq failed: got " + std::to_string(a) +
                                             ", expected " + std::to_string(e));
                }
                return;
            }
            // fallback: string compare of printed values
            std::string a = actual.to_string();
            std::string e = expected.to_string();
            if (a != e) {
                throw std::runtime_error("expect_eq failed: got \"" + a + "\" != \"" + e + "\"");
            }
        });

        // actual expected tol        expect_approx
        I.register_op("expect_approx", [](std::stack<WofValue>& S){
            if (S.size() < 3) throw std::runtime_error("expect_approx: need actual expected tol");
            double tol = need_num(S.top(), "expect_approx"); S.pop();
            double e   = need_num(S.top(), "expect_approx"); S.pop();
            double a   = need_num(S.top(), "expect_approx"); S.pop();
            if (!(std::isfinite(tol) && tol >= 0.0)) throw std::runtime_error("expect_approx: bad tol");
            if (std::isnan(a) || std::isnan(e) || std::fabs(a - e) > tol) {
                throw std::runtime_error("expect_approx failed: got " + std::to_string(a) +
                                         ", expected " + std::to_string(e) +
                                         " (tol " + std::to_string(tol) + ")");
            }
        });

        // cond                       expect_true      (nonzero numeric == true)
        I.register_op("expect_true", [](std::stack<WofValue>& S){
            if (S.size() < 1) throw std::runtime_error("expect_true: need cond");
            double c = need_num(S.top(), "expect_true"); S.pop();
            if (c == 0.0) throw std::runtime_error("expect_true failed: condition is false (0)");
        });

        // "message"                  note             (prints to stdout)
        I.register_op("note", [](std::stack<WofValue>& S){
            if (S.size() < 1) throw std::runtime_error("note: need message");
            std::string m = S.top().to_string(); S.pop();
            std::cout << "[NOTE] " << m << std::endl;
        });
    }
};

} // namespace woflang

WOFLANG_PLUGIN_EXPORT void register_plugin(woflang::WoflangInterpreter& interp) {
    static woflang::AssertOpsPlugin plugin;
    plugin.register_ops(interp);
}
