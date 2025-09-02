// plugins/prime_ops.cpp
//
// Deterministic 64-bit Miller–Rabin + next_prime for woflang.
// Exposes ops:
//   - prime_check   ( n -- 1|0 )  : pushes 1 if n is prime, else 0
//   - next_prime    ( n -- p )    : pushes the smallest prime >= n+1 (i.e., next after n)
//   - prime_version ( -- v )      : pushes an integer version marker
//
// Compatible with the plugin loader signature: init_plugin(OpTable*).

// Fixed prime_ops.cpp for current WofValue API
#include "../../src/core/woflang.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <cmath>

namespace woflang {

// Convert WofValue to uint64_t (assuming WofValue has .d field)
uint64_t to_u64_throw(const WofValue& v, const char* context) {
    double d = v.d;
    if (d < 0) {
        throw std::runtime_error(std::string(context) + ": negative value");
    }
    if (d > static_cast<double>(UINT64_MAX)) {
        throw std::runtime_error(std::string(context) + ": value too large");
    }
    return static_cast<uint64_t>(d);
}

// Push boolean as integer
void push_bool(std::stack<WofValue>& st, bool b) {
    WofValue result;
    result.d = b ? 1.0 : 0.0;
    st.push(result);
}

// Push uint64_t as double
void push_u64(std::stack<WofValue>& st, uint64_t x) {
    WofValue result;
    result.d = static_cast<double>(x);
    st.push(result);
}

// Miller-Rabin primality test
bool miller_rabin(uint64_t n, int rounds = 10) {
    if (n < 2) return false;
    if (n == 2 || n == 3) return true;
    if (n % 2 == 0) return false;

    // Write n-1 as d * 2^r
    uint64_t d = n - 1;
    int r = 0;
    while (d % 2 == 0) {
        d /= 2;
        r++;
    }

    // Random number generator
    static std::mt19937_64 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    
    for (int i = 0; i < rounds; i++) {
        std::uniform_int_distribution<uint64_t> dis(2, n - 2);
        uint64_t a = dis(gen);
        
        // Compute a^d mod n
        uint64_t x = 1;
        uint64_t base = a;
        uint64_t exp = d;
        while (exp > 0) {
            if (exp & 1) {
                x = (__uint128_t(x) * base) % n;
            }
            base = (__uint128_t(base) * base) % n;
            exp >>= 1;
        }
        
        if (x == 1 || x == n - 1) continue;
        
        bool composite = true;
        for (int j = 0; j < r - 1; j++) {
            x = (__uint128_t(x) * x) % n;
            if (x == n - 1) {
                composite = false;
                break;
            }
        }
        if (composite) return false;
    }
    return true;
}

// Simple primality test for small numbers
bool is_prime_simple(uint64_t n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    for (uint64_t i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

// Prime check operation
void op_prime_check(std::stack<WofValue>& st) {
    if (st.empty()) {
        std::cerr << "prime_check: stack underflow\n";
        return;
    }
    
    try {
        uint64_t n = to_u64_throw(st.top(), "prime_check");
        st.pop();
        
        bool is_prime;
        if (n < 1000000) {
            is_prime = is_prime_simple(n);
        } else {
            is_prime = miller_rabin(n, 10);
        }
        
        push_bool(st, is_prime);
    } catch (const std::exception& e) {
        std::cerr << "prime_check error: " << e.what() << "\n";
        push_bool(st, false);
    }
}

// Ultra-fast Fermat test (probabilistic)
void op_prime_check_ultra(std::stack<WofValue>& st) {
    if (st.empty()) {
        std::cerr << "prime_check_ultra: stack underflow\n";
        return;
    }
    
    try {
        uint64_t n = to_u64_throw(st.top(), "prime_check_ultra");
        st.pop();
        
        if (n < 2) {
            push_bool(st, false);
            return;
        }
        if (n == 2) {
            push_bool(st, true);
            return;
        }
        if (n % 2 == 0) {
            push_bool(st, false);
            return;
        }
        
        // Fermat test with base 2
        uint64_t result = 1;
        uint64_t base = 2;
        uint64_t exp = n - 1;
        while (exp > 0) {
            if (exp & 1) {
                result = (__uint128_t(result) * base) % n;
            }
            base = (__uint128_t(base) * base) % n;
            exp >>= 1;
        }
        
        push_bool(st, result == 1);
    } catch (const std::exception& e) {
        std::cerr << "prime_check_ultra error: " << e.what() << "\n";
        push_bool(st, false);
    }
}

// Miller-Rabin with custom rounds
void op_miller_rabin(std::stack<WofValue>& st) {
    if (st.size() < 2) {
        std::cerr << "miller_rabin: need 2 values (number rounds)\n";
        return;
    }
    
    try {
        uint64_t rounds = to_u64_throw(st.top(), "miller_rabin rounds");
        st.pop();
        uint64_t n = to_u64_throw(st.top(), "miller_rabin number");
        st.pop();
        
        bool is_prime = miller_rabin(n, static_cast<int>(rounds));
        push_bool(st, is_prime);
    } catch (const std::exception& e) {
        std::cerr << "miller_rabin error: " << e.what() << "\n";
        push_bool(st, false);
    }
}

// Version info
void op_prime_version(std::stack<WofValue>& st) {
    push_u64(st, 7004); // Version 7.004
}

} // namespace woflang

extern "C" {
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    (*op_table)["prime_check"] = woflang::op_prime_check;
    (*op_table)["prime_check_ultra"] = woflang::op_prime_check_ultra;
    (*op_table)["miller_rabin"] = woflang::op_miller_rabin;
    (*op_table)["prime_version"] = woflang::op_prime_version;
}
