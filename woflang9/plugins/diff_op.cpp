// plugins/diff_ops.cpp
//
// Woflang Numerical Differentiation Plugin
// ----------------------------------------
// Ops provided (stack signatures shown as: input ... -> output):
//
//   expr x h diff             -> f'(x)   using central difference
//   expr x     sdiff          -> f'(x)   central difference with safe default h
//   expr x h diff2            -> f''(x)  second derivative (central)
//   expr x h diff_fwd         -> f'(x)   forward difference
//   expr x h diff_bwd         -> f'(x)   backward difference
//
// Conventions:
//   - expr : String or Symbol containing a Woflang expression that consumes x on the stack.
//            Example expr: "dup * sin" (interpreted as: push x, then run this).
//   - x    : Numeric point at which to differentiate.
//   - h    : Numeric step size (positive).
//
// Notes:
//   - We sandbox evaluation by snapshotting/restoring the interpreter stack.
//   - The expression must leave exactly one numeric result on the stack.
//   - sdiff picks h = max(1e-6, 1e-6*abs(x)) to balance scale.
//
// Example usage in Woflang REPL:
//   "dup *" 3 1e-5 diff        # derivative of f(x)=x^2 at x=3  => ~6
//   "sin"   0 1e-6 diff        # cos(0) = 1                    => ~1
//   "exp"   2 sdiff            # e^2                           => ~7.389
//
#include "core/woflang.hpp"
#include <cmath>
#include <sstream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <utility>

namespace woflang {

static inline std::string fmt_double(double v) {
    // Print with enough precision to survive round-trips without noisy tails.
    std::ostringstream oss;
    oss << std::setprecision(17) << std::scientific << v;
    return oss.str();
}

static double eval_expr_with_x(WoflangInterpreter& interp, const std::string& expr, double x) {
    // Snapshot current stack
    auto saved = interp.stack;

    // Clear for a clean evaluation context
    interp.stack.clear();

    // Run: "<x> <expr>"
    std::string line = fmt_double(x) + " " + expr;
    interp.exec_line(line);

    if (interp.stack.empty()) {
        // Restore and error
        interp.stack = std::move(saved);
        throw std::runtime_error("diff: expression produced no result");
    }
    auto out = interp.stack.back();
    interp.stack.pop_back();

    // Restore caller stack
    interp.stack = std::move(saved);

    if (!out.is_numeric()) {
        throw std::runtime_error("diff: expression result is not numeric");
    }
    return out.as_numeric();
}

static inline std::string expect_expr_string(const WofValue& v, const char* opname) {
    if (v.type == WofType::String || v.type == WofType::Symbol) {
        return std::get<std::string>(v.value);
    }
    std::ostringstream oss;
    oss << opname << ": expected expression (string or symbol)";
    throw std::runtime_error(oss.str());
}

class DiffOpsPlugin : public WoflangPlugin {
public:
    void register_ops(WoflangInterpreter& interp) override {

        // expr x h diff  -> f'(x) (central difference)
        interp.register_op("diff", [](WoflangInterpreter& I) {
            if (I.stack.size() < 3) throw std::runtime_error("diff: need expr x h");
            auto hV   = I.stack.back(); I.stack.pop_back();
            auto xV   = I.stack.back(); I.stack.pop_back();
            auto exprV= I.stack.back(); I.stack.pop_back();

            if (!xV.is_numeric() || !hV.is_numeric())
                throw std::runtime_error("diff: x and h must be numeric");
            double x = xV.as_numeric();
            double h = hV.as_numeric();
            if (!(h > 0.0)) throw std::runtime_error("diff: h must be > 0");

            std::string expr = expect_expr_string(exprV, "diff");
            double fxph = eval_expr_with_x(I, expr, x + h);
            double fxmh = eval_expr_with_x(I, expr, x - h);
            I.stack.emplace_back( (fxph - fxmh) / (2.0 * h) );
        });

        // expr x sdiff -> f'(x) with safe default h
        interp.register_op("sdiff", [](WoflangInterpreter& I) {
            if (I.stack.size() < 2) throw std::runtime_error("sdiff: need expr x");
            auto xV   = I.stack.back(); I.stack.pop_back();
            auto exprV= I.stack.back(); I.stack.pop_back();

            if (!xV.is_numeric())
                throw std::runtime_error("sdiff: x must be numeric");
            double x = xV.as_numeric();
            // scale-aware default step
            double h = std::max(1e-6, 1e-6 * std::abs(x));

            std::string expr = expect_expr_string(exprV, "sdiff");
            double fxph = eval_expr_with_x(I, expr, x + h);
            double fxmh = eval_expr_with_x(I, expr, x - h);
            I.stack.emplace_back( (fxph - fxmh) / (2.0 * h) );
        });

        // expr x h diff2 -> f''(x) (second derivative, central)
        interp.register_op("diff2", [](WoflangInterpreter& I) {
            if (I.stack.size() < 3) throw std::runtime_error("diff2: need expr x h");
            auto hV   = I.stack.back(); I.stack.pop_back();
            auto xV   = I.stack.back(); I.stack.pop_back();
            auto exprV= I.stack.back(); I.stack.pop_back();

            if (!xV.is_numeric() || !hV.is_numeric())
                throw std::runtime_error("diff2: x and h must be numeric");
            double x = xV.as_numeric();
            double h = hV.as_numeric();
            if (!(h > 0.0)) throw std::runtime_error("diff2: h must be > 0");

            std::string expr = expect_expr_string(exprV, "diff2");
            double fxph = eval_expr_with_x(I, expr, x + h);
            double fx   = eval_expr_with_x(I, expr, x);
            double fxmh = eval_expr_with_x(I, expr, x - h);
            I.stack.emplace_back( (fxph - 2.0*fx + fxmh) / (h*h) );
        });

        // expr x h diff_fwd -> forward difference
        interp.register_op("diff_fwd", [](WoflangInterpreter& I) {
            if (I.stack.size() < 3) throw std::runtime_error("diff_fwd: need expr x h");
            auto hV   = I.stack.back(); I.stack.pop_back();
            auto xV   = I.stack.back(); I.stack.pop_back();
            auto exprV= I.stack.back(); I.stack.pop_back();

            if (!xV.is_numeric() || !hV.is_numeric())
                throw std::runtime_error("diff_fwd: x and h must be numeric");
            double x = xV.as_numeric();
            double h = hV.as_numeric();
            if (!(h > 0.0)) throw std::runtime_error("diff_fwd: h must be > 0");

            std::string expr = expect_expr_string(exprV, "diff_fwd");
            double fx   = eval_expr_with_x(I, expr, x);
            double fxph = eval_expr_with_x(I, expr, x + h);
            I.stack.emplace_back( (fxph - fx) / h );
        });

        // expr x h diff_bwd -> backward difference
        interp.register_op("diff_bwd", [](WoflangInterpreter& I) {
            if (I.stack.size() < 3) throw std::runtime_error("diff_bwd: need expr x h");
            auto hV   = I.stack.back(); I.stack.pop_back();
            auto xV   = I.stack.back(); I.stack.pop_back();
            auto exprV= I.stack.back(); I.stack.pop_back();

            if (!xV.is_numeric() || !hV.is_numeric())
                throw std::runtime_error("diff_bwd: x and h must be numeric");
            double x = xV.as_numeric();
            double h = hV.as_numeric();
            if (!(h > 0.0)) throw std::runtime_error("diff_bwd: h must be > 0");

            std::string expr = expect_expr_string(exprV, "diff_bwd");
            double fx   = eval_expr_with_x(I, expr, x);
            double fxmh = eval_expr_with_x(I, expr, x - h);
            I.stack.emplace_back( (fx - fxmh) / h );
        });
    }
};

} // namespace woflang

// Plugin entry point
WOFLANG_PLUGIN_EXPORT void register_plugin(woflang::WoflangInterpreter& interp) {
    static woflang::DiffOpsPlugin plugin;
    plugin.register_ops(interp);
}
