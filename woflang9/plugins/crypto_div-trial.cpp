// ==================================================
// crypto_ops.cpp - Cryptography Operations (Dynamic Loading)
// ==================================================
#include "core/woflang.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <ctime>
#include <cmath>
#include <functional>
#include <algorithm>
#include <iomanip>
#include <sstream>

extern "C" {

namespace {
    // Base64 encoding/decoding
    static const std::string base64_chars = 
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    
    std::string base64_encode(const std::string& input) {
        std::string encoded;
        int i = 0;
        int j = 0;
        unsigned char char_array_3[3];
        unsigned char char_array_4[4];
        
        for (char c : input) {
            char_array_3[i++] = c;
            if (i == 3) {
                char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
                char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
                char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
                char_array_4[3] = char_array_3[2] & 0x3f;
                
                for (i = 0; i < 4; i++) {
                    encoded += base64_chars[char_array_4[i]];
                }
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 3; j++) {
                char_array_3[j] = '\0';
            }
            
            char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
            char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
            char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
            
            for (j = 0; j < i + 1; j++) {
                encoded += base64_chars[char_array_4[j]];
            }
            
            while (i++ < 3) {
                encoded += '=';
            }
        }
        
        return encoded;
    }
    
    // Convert string to hexadecimal representation
    std::string to_hex(const std::string& text) {
        std::stringstream ss;
        for (unsigned char c : text) {
            ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        }
        return ss.str();
    }
    
    // Convert string to binary representation
    std::string text_to_binary(const std::string& text) {
        std::string binary;
        for (char c : text) {
            for (int i = 7; i >= 0; --i) {
                binary += ((c & (1 << i)) ? '1' : '0');
            }
            binary += ' ';
        }
        return binary;
    }
    
    // Generate a random integer in a range
    int64_t random_int(int64_t min, int64_t max) {
        static std::mt19937_64 rng(static_cast<unsigned long>(std::time(nullptr)));
        std::uniform_int_distribution<int64_t> dist(min, max);
        return dist(rng);
    }
    
    // Generate random bytes
    std::string random_bytes(size_t count) {
        static std::mt19937 rng(static_cast<unsigned long>(std::time(nullptr)));
        std::uniform_int_distribution<int> dist(0, 255);
        
        std::string bytes;
        for (size_t i = 0; i < count; ++i) {
            bytes += static_cast<char>(dist(rng));
        }
        
        return bytes;
    }
    
    // Modular exponentiation for large integers
    int64_t mod_pow(int64_t base, int64_t exp, int64_t modulus) {
        if (modulus == 1) return 0;
        
        int64_t result = 1;
        base = base % modulus;
        
        while (exp > 0) {
            if (exp % 2 == 1) {
                result = (result * base) % modulus;
            }
            exp = exp >> 1;
            base = (base * base) % modulus;
        }
        
        return result;
    }
    
    // Extended Euclidean Algorithm for modular inverse
    int64_t mod_inverse(int64_t a, int64_t m) {
        int64_t m0 = m;
        int64_t y = 0, x = 1;
        
        if (m == 1) return 0;
        
        while (a > 1) {
            int64_t q = a / m;
            int64_t t = m;
            
            m = a % m;
            a = t;
            t = y;
            
            y = x - q * y;
            x = t;
        }
        
        if (x < 0) {
            x += m0;
        }
        
        return x;
    }
    
    // Simple Caesar cipher
    std::string caesar_cipher(const std::string& text, int shift) {
        std::string result;
        
        for (char c : text) {
            if (std::isalpha(c)) {
                char base = std::islower(c) ? 'a' : 'A';
                result += static_cast<char>(((c - base + shift) % 26 + 26) % 26 + base);
            } else {
                result += c;
            }
        }
        
        return result;
    }
    
    // FNV-1a hash (simplified)
    uint64_t fnv1a_hash(const std::string& data) {
        const uint64_t FNV_PRIME = 1099511628211ULL;
        const uint64_t FNV_OFFSET_BASIS = 14695981039346656037ULL;
        
        uint64_t hash = FNV_OFFSET_BASIS;
        
        for (unsigned char c : data) {
            hash ^= c;
            hash *= FNV_PRIME;
        }
        
        return hash;
    }
    
    // Simple XOR cipher
    std::string xor_cipher(const std::string& text, const std::string& key) {
        std::string result;
        
        for (size_t i = 0; i < text.size(); ++i) {
            result += text[i] ^ key[i % key.size()];
        }
        
        return result;
    }
}

void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
    // Convert string value on stack to base64
    (*op_table)["base64_encode"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("base64_encode requires a value");
        }
        
        auto value = stack.top(); stack.pop();
        
        // Convert the numeric value to string for encoding
        std::string str = std::to_string(value.d);#ifndef WOFLANG_PLUGIN_EXPORT

#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {
        std::string encoded = base64_encode(str);
        
        std::cout << "Base64 encoded: " << encoded << std::endl;
        
        // Store the hash of the encoded string as a numeric value
        uint64_t hash = fnv1a_hash(encoded);
        woflang::WofValue result;
        result.d = static_cast<double>(static_cast<int64_t>(hash));
        stack.push(result);
    };
    
    // Convert to hexadecimal
    (*op_table)["to_hex"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("to_hex requires a value");
        }
        
        auto value = stack.top(); stack.pop();
        
        std::string str = std::to_string(value.d);
        std::string hex = to_hex(str);
        
        std::cout << "Hex: " << hex << std::endl;
        
        // Store the hash of the hex string as a numeric value
        uint64_t hash = fnv1a_hash(hex);
        woflang::WofValue result;
        result.d = static_cast<double>(static_cast<int64_t>(hash));
        stack.push(result);
    };
    
    // Convert to binary
    (*op_table)["to_binary"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("to_binary requires a value");
        }
        
        auto value = stack.top(); stack.pop();
        
        std::string str = std::to_string(value.d);
        std::string binary = text_to_binary(str);
        
        std::cout << "Binary: " << binary << std::endl;
        
        // Store the hash of the binary string as a numeric value
        uint64_t hash = fnv1a_hash(binary);
        woflang::WofValue result;
        result.d = static_cast<double>(static_cast<int64_t>(hash));
        stack.push(result);
    };
    
    // Generate random number in range
    (*op_table)["random"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.size() < 2) {
            throw std::runtime_error("random requires min and max values");
        }
        
        auto max = stack.top(); stack.pop();
        auto min = stack.top(); stack.pop();
        
        int64_t min_val = static_cast<int64_t>(min.d);
        int64_t max_val = static_cast<int64_t>(max.d);
        
        if (min_val > max_val) {
            std::swap(min_val, max_val);
        }
        
        int64_t result = random_int(min_val, max_val);
        
        woflang::WofValue res;
        res.d = static_cast<double>(result);
        stack.push(res);
        
        std::cout << "Random number: " << result << std::endl;
    };
    
    // Generate random bytes and return their count
    (*op_table)["random_bytes"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("random_bytes requires a count");
        }
        
        auto count = stack.top(); stack.pop();
        
        int64_t count_val = static_cast<int64_t>(count.d);
        
        if (count_val <= 0 || count_val > 1024) {
            throw std::runtime_error("count must be between 1 and 1024");
        }
        
        std::string bytes = random_bytes(static_cast<size_t>(count_val));
        
        std::cout << "Random bytes (hex): " << to_hex(bytes) << std::endl;
        
        woflang::WofValue result;
        result.d = static_cast<double>(count_val);
        stack.push(result);
    };
    
    // Check if a number is prime
    (*op_table)["prime_check"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("prime_check requires a number");
        }
        
        auto value = stack.top(); stack.pop();
        
        int64_t n = static_cast<int64_t>(value.d);
        
        if (n <= 1) {
            woflang::WofValue result;
            result.d = 0.0;
            stack.push(result);
            std::cout << n << " is not prime" << std::endl;
            return;
        }
        
        if (n == 2 || n == 3) {
            woflang::WofValue result;
            result.d = 1.0;
            stack.push(result);
            std::cout << n << " is prime" << std::endl;
            return;
        }
        
        if (n % 2 == 0 || n % 3 == 0) {
            woflang::WofValue result;
            result.d = 0.0;
            stack.push(result);
            std::cout << n << " is not prime" << std::endl;
            return;
        }
        
        for (int64_t i = 5; i * i <= n; i += 6) {
            if (n % i == 0 || n % (i + 2) == 0) {
                woflang::WofValue result;
                result.d = 0.0;
                stack.push(result);
                std::cout << n << " is not prime" << std::endl;
                return;
            }
        }
        
        woflang::WofValue result;
        result.d = 1.0;
        stack.push(result);
        std::cout << n << " is prime" << std::endl;
    };
    
    // Calculate entropy of stack values
    (*op_table)["crypto_entropy"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            std::cout << "No data for entropy calculation" << std::endl;
            woflang::WofValue result;
            result.d = 0.0;
            stack.push(result);
            return;
        }
        
        // Convert stack values to string
        std::string data;
        std::vector<woflang::WofValue> values;
        
        while (!stack.empty()) {
            values.push_back(stack.top());
            data += std::to_string(stack.top().d);
            stack.pop();
        }
        
        // Restore stack
        for (auto it = values.rbegin(); it != values.rend(); ++it) {
            stack.push(*it);
        }
        
        // Calculate frequency
        std::array<int, 256> freq{};
        for (unsigned char c : data) {
            freq[c]++;
        }
        
        double entropy = 0.0;
        for (int count : freq) {
            if (count > 0) {
                double p = static_cast<double>(count) / data.size();
                entropy -= p * std::log2(p);
            }
        }
        
        std::cout << "Entropy: " << entropy << " bits per byte" << std::endl;
        std::cout << "Perfect entropy for a byte is 8 bits." << std::endl;
        
        woflang::WofValue result;
        result.d = entropy;
        stack.push(result);
    };
    
    // Simple Diffie-Hellman demonstration
    (*op_table)["diffie_hellman"] = [](std::stack<woflang::WofValue>& stack) {
        int64_t p = 23;
        int64_t g = 5;
        
        if (stack.size() >= 2) {
            auto g_val = stack.top(); stack.pop();
            auto p_val = stack.top(); stack.pop();
            
            p = static_cast<int64_t>(p_val.d);
            g = static_cast<int64_t>(g_val.d);
        }
        
        int64_t a = random_int(2, p - 2);
        int64_t b = random_int(2, p - 2);
        
        int64_t A = mod_pow(g, a, p);
        int64_t B = mod_pow(g, b, p);
        
        int64_t secret_a = mod_pow(B, a, p);
        int64_t secret_b = mod_pow(A, b, p);
        
        std::cout << "Diffie-Hellman Key Exchange (Educational)" << std::endl;
        std::cout << "----------------------------------------" << std::endl;
        std::cout << "Parameters:" << std::endl;
        std::cout << "  p = " << p << " (prime modulus)" << std::endl;
        std::cout << "  g = " << g << " (generator)" << std::endl << std::endl;
        
        std::cout << "Private keys:" << std::endl;
        std::cout << "  a = " << a << " (Alice's private key)" << std::endl;
        std::cout << "  b = " << b << " (Bob's private key)" << std::endl << std::endl;
        
        std::cout << "Public keys:" << std::endl;
        std::cout << "  A = g^a mod p = " << A << " (Alice's public key)" << std::endl;
        std::cout << "  B = g^b mod p = " << B << " (Bob's public key)" << std::endl << std::endl;
        
        std::cout << "Shared secret computation:" << std::endl;
        std::cout << "  Alice: B^a mod p = " << secret_a << std::endl;
        std::cout << "  Bob:   A^b mod p = " << secret_b << std::endl << std::endl;
        
        if (secret_a == secret_b) {
            std::cout << "Shared secret: " << secret_a << std::endl;
        } else {
            std::cout << "Error: Shared secrets don't match!" << std::endl;
        }
        
        woflang::WofValue result;
        result.d = static_cast<double>(secret_a);
        stack.push(result);
    };
    
    // Simple RSA demonstration
    (*op_table)["rsa_demo"] = [](std::stack<woflang::WofValue>& stack) {
        std::cout << "RSA Encryption/Decryption Demo (Educational)" << std::endl;
        std::cout << "-------------------------------------------" << std::endl;
        
        int64_t p = 61;
        int64_t q = 53;
        
        int64_t n = p * q;
        int64_t phi = (p - 1) * (q - 1);
        int64_t e = 17;
        int64_t d = mod_inverse(e, phi);
        
        int64_t message = 42;
        
        if (!stack.empty()) {
            auto msg = stack.top(); stack.pop();
            message = static_cast<int64_t>(msg.d);
            if (message >= n) {
                std::cout << "Message must be less than " << n << std::endl;
                message = message % n;
                std::cout << "Using message modulo n: " << message << std::endl;
            }
        }
        
        int64_t ciphertext = mod_pow(message, e, n);
        int64_t decrypted = mod_pow(ciphertext, d, n);
        
        std::cout << "Key generation:" << std::endl;
        std::cout << "  p = " << p << ", q = " << q << " (prime factors)" << std::endl;
        std::cout << "  n = p * q = " << n << " (modulus)" << std::endl;
        std::cout << "  phi(n) = (p-1) * (q-1) = " << phi << std::endl;
        std::cout << "  e = " << e << " (public exponent)" << std::endl;
        std::cout << "  d = " << d << " (private exponent)" << std::endl << std::endl;
        
        std::cout << "Encryption/decryption:" << std::endl;
        std::cout << "  Message: " << message << std::endl;
        std::cout << "  Ciphertext = message^e mod n = " << ciphertext << std::endl;
        std::cout << "  Decrypted = ciphertext^d mod n = " << decrypted << std::endl << std::endl;
        
        if (message == decrypted) {
            std::cout << "Decryption successful!" << std::endl;
        } else {
            std::cout << "Error: Decryption failed!" << std::endl;
        }
        
        woflang::WofValue result1, result2;
        result1.d = static_cast<double>(ciphertext);
        result2.d = static_cast<double>(decrypted);
        stack.push(result1);
        stack.push(result2);
    };
    
    // Calculate hash of a value
    (*op_table)["hash"] = [](std::stack<woflang::WofValue>& stack) {
        if (stack.empty()) {
            throw std::runtime_error("hash requires a value");
        }
        
        auto value = stack.top(); stack.pop();
        
        std::string str = std::to_string(value.d);
        uint64_t hash = fnv1a_hash(str);
        
        woflang::WofValue result;
        result.d = static_cast<double>(static_cast<int64_t>(hash));
        stack.push(result);
        
        std::stringstream ss;
        ss << std::hex << std::setw(16) << std::setfill('0') << hash;
        std::cout << "Hash: 0x" << ss.str() << std::endl;
    };
    
    // Crypto tutorial
    (*op_table)["crypto_tutorial"] = [](std::stack<woflang::WofValue>&) {
        std::cout << "=== Cryptography Tutorial ===" << std::endl << std::endl;
        
        std::cout << "1. Encoding vs Encryption:" << std::endl;
        std::cout << "   - Encoding: Reversible data transformation (Base64, hex)" << std::endl;
        std::cout << "   - Encryption: Secure data transformation requiring a key" << std::endl << std::endl;
        
        std::cout << "2. Hash Functions:" << std::endl;
        std::cout << "   - One-way functions that produce fixed-size output" << std::endl;
        std::cout << "   - Used for integrity checking and digital signatures" << std::endl << std::endl;
        
        std::cout << "3. Public Key Cryptography:" << std::endl;
        std::cout << "   - RSA: Different keys for encryption and decryption" << std::endl;
        std::cout << "   - Diffie-Hellman: Secure key agreement over insecure channels" << std::endl << std::endl;
        
        std::cout << "4. Entropy:" << std::endl;
        std::cout << "   - Measure of randomness in data" << std::endl;
        std::cout << "   - Higher entropy = more secure cryptographic keys" << std::endl << std::endl;
        
        std::cout << "Available operations:" << std::endl;
        std::cout << "  - Encoding: to_hex, base64_encode, to_binary" << std::endl;
        std::cout << "  - Crypto: diffie_hellman, rsa_demo, hash" << std::endl;
        std::cout << "  - Analysis: crypto_entropy, prime_check" << std::endl;
        std::cout << "  - Random: random, random_bytes" << std::endl;
    };
}

