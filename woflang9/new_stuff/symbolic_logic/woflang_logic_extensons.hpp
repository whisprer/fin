// woflang_logic_extensions.hpp - Mathematical Logic Symbol Support
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>

// Extended token types for mathematical logic
enum class WofTokenType : uint16_t {
    // Basic tokens
    IDENTIFIER = 0,
    NUMBER = 1,
    STRING = 2,
    WHITESPACE = 3,
    
    // Mathematical Logic Operators
    FORALL = 100,           // ∀ (for all)
    EXISTS = 101,           // ∃ (there exists)
    EXISTS_UNIQUE = 102,    // ∃! (there exists unique)
    ELEMENT_OF = 103,       // ∈ (element of)
    NOT_ELEMENT_OF = 104,   // ∉ (not element of)
    SUBSET = 105,           // ⊆ (subset)
    PROPER_SUBSET = 106,    // ⊂ (proper subset)
    SUPERSET = 107,         // ⊇ (superset)
    UNION = 108,            // ∪ (union)
    INTERSECTION = 109,     // ∩ (intersection)
    
    // Logic Connectives
    IMPLIES = 200,          // → (implies)
    IFF = 201,              // ↔ (if and only if)
    LOGICAL_AND = 202,      // ∧ (logical and)
    LOGICAL_OR = 203,       // ∨ (logical or)
    LOGICAL_NOT = 204,      // ¬ (logical not)
    
    // Comparison and Relations
    LESS_EQUAL = 300,       // ≤ (less than or equal)
    GREATER_EQUAL = 301,    // ≥ (greater than or equal)
    NOT_EQUAL = 302,        // ≠ (not equal)
    EQUIVALENT = 303,       // ≡ (equivalent)
    APPROXIMATELY = 304,    // ≈ (approximately)
    PRECEDES = 305,         // ≺ (precedes)
    
    // Special Mathematical Symbols
    INFINITY = 400,         // ∞ (infinity)
    ALEPH_NULL = 401,       // ℵ₀ (aleph null)
    LAMBDA = 402,           // λ (lambda)
    THEREFORE = 403,        // ∴ (therefore)
    BECAUSE = 404,          // ∵ (because)
    QED = 405,              // ∎ (QED)
    
    // WofLang Keywords
    AXIOM = 500,
    THEOREM = 501,
    PROOF = 502,
    LEMMA = 503,
    COROLLARY = 504,
    DEFINITION = 505,
    TYPE = 506,
    FUNCTION = 507,
    WHERE = 508,
    LET = 509,
    ASSERT = 510,
    CONTRADICTION = 511,
    ASSUME = 512,
    RETURN = 513
};

// Enhanced WofToken with type information
struct LogicWofToken {
    std::u32string token;
    WofTokenType type;
    uint16_t opcode;
    size_t position;
    size_t length;
};

// Unicode symbol to token type mapping
class WofLogicSymbols {
private:
    std::unordered_map<std::u32string, WofTokenType> symbol_map;
    std::unordered_set<std::u32string> keywords;
    
public:
    WofLogicSymbols() {
        // Mathematical Logic Symbols
        symbol_map[U"∀"] = WofTokenType::FORALL;
        symbol_map[U"∃"] = WofTokenType::EXISTS;
        symbol_map[U"∃!"] = WofTokenType::EXISTS_UNIQUE;
        symbol_map[U"∈"] = WofTokenType::ELEMENT_OF;
        symbol_map[U"∉"] = WofTokenType::NOT_ELEMENT_OF;
        symbol_map[U"⊆"] = WofTokenType::SUBSET;
        symbol_map[U"⊂"] = WofTokenType::PROPER_SUBSET;
        symbol_map[U"⊇"] = WofTokenType::SUPERSET;
        symbol_map[U"∪"] = WofTokenType::UNION;
        symbol_map[U"∩"] = WofTokenType::INTERSECTION;
        
        // Logic Connectives
        symbol_map[U"→"] = WofTokenType::IMPLIES;
        symbol_map[U"↔"] = WofTokenType::IFF;
        symbol_map[U"∧"] = WofTokenType::LOGICAL_AND;
        symbol_map[U"∨"] = WofTokenType::LOGICAL_OR;
        symbol_map[U"¬"] = WofTokenType::LOGICAL_NOT;
        
        // Comparison Operators
        symbol_map[U"≤"] = WofTokenType::LESS_EQUAL;
        symbol_map[U"≥"] = WofTokenType::GREATER_EQUAL;
        symbol_map[U"≠"] = WofTokenType::NOT_EQUAL;
        symbol_map[U"≡"] = WofTokenType::EQUIVALENT;
        symbol_map[U"≈"] = WofTokenType::APPROXIMATELY;
        symbol_map[U"≺"] = WofTokenType::PRECEDES;
        
        // Special Symbols
        symbol_map[U"∞"] = WofTokenType::INFINITY;
        symbol_map[U"ℵ₀"] = WofTokenType::ALEPH_NULL;
        symbol_map[U"ℵ"] = WofTokenType::ALEPH_NULL;  // Alternative
        symbol_map[U"λ"] = WofTokenType::LAMBDA;
        symbol_map[U"∴"] = WofTokenType::THEREFORE;
        symbol_map[U"∵"] = WofTokenType::BECAUSE;
        symbol_map[U"∎"] = WofTokenType::QED;
        
        // ASCII alternatives for common symbols
        symbol_map[U"->"] = WofTokenType::IMPLIES;
        symbol_map[U"<->"] = WofTokenType::IFF;
        symbol_map[U"<="] = WofTokenType::LESS_EQUAL;
        symbol_map[U">="] = WofTokenType::GREATER_EQUAL;
        symbol_map[U"!="] = WofTokenType::NOT_EQUAL;
        symbol_map[U"=="] = WofTokenType::EQUIVALENT;
        symbol_map[U"~="] = WofTokenType::APPROXIMATELY;
        
        // WofLang Keywords
        keywords.insert(U"axiom");
        keywords.insert(U"theorem");
        keywords.insert(U"proof");
        keywords.insert(U"lemma");
        keywords.insert(U"corollary");
        keywords.insert(U"definition");
        keywords.insert(U"type");
        keywords.insert(U"function");
        keywords.insert(U"where");
        keywords.insert(U"let");
        keywords.insert(U"assert");
        keywords.insert(U"contradiction");
        keywords.insert(U"assume");
        keywords.insert(U"return");
    }
    
    WofTokenType getSymbolType(const std::u32string& token) const {
        auto it = symbol_map.find(token);
        return (it != symbol_map.end()) ? it->second : WofTokenType::IDENTIFIER;
    }
    
    bool isKeyword(const std::u32string& token) const {
        return keywords.find(token) != keywords.end();
    }
    
    WofTokenType getKeywordType(const std::u32string& token) const {
        if (!isKeyword(token)) return WofTokenType::IDENTIFIER;
        
        if (token == U"axiom") return WofTokenType::AXIOM;
        if (token == U"theorem") return WofTokenType::THEOREM;
        if (token == U"proof") return WofTokenType::PROOF;
        if (token == U"lemma") return WofTokenType::LEMMA;
        if (token == U"corollary") return WofTokenType::COROLLARY;
        if (token == U"definition") return WofTokenType::DEFINITION;
        if (token == U"type") return WofTokenType::TYPE;
        if (token == U"function") return WofTokenType::FUNCTION;
        if (token == U"where") return WofTokenType::WHERE;
        if (token == U"let") return WofTokenType::LET;
        if (token == U"assert") return WofTokenType::ASSERT;
        if (token == U"contradiction") return WofTokenType::CONTRADICTION;
        if (token == U"assume") return WofTokenType::ASSUME;
        if (token == U"return") return WofTokenType::RETURN;
        
        return WofTokenType::IDENTIFIER;
    }
    
    bool isNumeric(const std::u32string& token) const {
        if (token.empty()) return false;
        
        for (char32_t c : token) {
            if (!(c >= U'0' && c <= U'9') && c != U'.' && c != U'₀' && c != U'₁' && 
                c != U'₂' && c != U'₃' && c != U'₄' && c != U'₅' && c != U'₆' && 
                c != U'₇' && c != U'₈' && c != U'₉') {
                return false;
            }
        }
        return true;
    }
};

// Enhanced tokenizer with logic symbol recognition
inline std::vector<LogicWofToken> parseWoflangLogic(const std::string& input_utf8) {
    if (input_utf8.empty()) {
        return {};
    }
    
    std::u32string input = utf8_to_utf32(input_utf8);
    std::vector<size_t> pos_map = build_utf8_position_map(input_utf8);
    std::vector<std::u32string> raw_tokens = simd_tokenize(input);
    
    WofLogicSymbols logic_parser;
    std::vector<LogicWofToken> tokens;
    tokens.reserve(raw_tokens.size());
    
    size_t utf32_pos = 0;
    for (const auto& raw_token : raw_tokens) {
        // Skip whitespace to find actual token start
        while (utf32_pos < input.size() && 
               (input[utf32_pos] == U' ' || input[utf32_pos] == U'\t' || 
                input[utf32_pos] == U'\n' || input[utf32_pos] == U'\r')) {
            ++utf32_pos;
        }
        
        size_t byte_pos = (utf32_pos < pos_map.size()) ? pos_map[utf32_pos] : input_utf8.size();
        
        // Determine token type
        WofTokenType token_type;
        
        // Check for mathematical symbols first
        token_type = logic_parser.getSymbolType(raw_token);
        if (token_type == WofTokenType::IDENTIFIER) {
            // Check for keywords
            token_type = logic_parser.getKeywordType(raw_token);
            if (token_type == WofTokenType::IDENTIFIER) {
                // Check for numbers
                if (logic_parser.isNumeric(raw_token)) {
                    token_type = WofTokenType::NUMBER;
                }
            }
        }
        
        tokens.push_back(LogicWofToken{
            raw_token,
            token_type,
            static_cast<uint16_t>(token_type),
            byte_pos,
            raw_token.size()
        });
        
        utf32_pos += raw_token.size();
    }
    
    return tokens;
}

// Multi-character symbol tokenizer for sequences like "∃!" and "->"
inline std::vector<std::u32string> simd_tokenize_logic_aware(const std::u32string& line) {
    std::vector<std::u32string> tokens;
    const size_t n = line.size();
    
    if (n == 0) return tokens;
    
    size_t i = 0;
    
    while (i < n) {
        // Skip whitespace
        while (i < n && (line[i] == U' ' || line[i] == U'\t' || 
                        line[i] == U'\n' || line[i] == U'\r')) {
            ++i;
        }
        
        if (i >= n) break;
        
        // Check for multi-character symbols first
        if (i + 1 < n) {
            std::u32string two_char = line.substr(i, 2);
            if (two_char == U"∃!" || two_char == U"->" || two_char == U"<-" ||
                two_char == U"<=" || two_char == U">=" || two_char == U"!=" ||
                two_char == U"==" || two_char == U"~=") {
                tokens.push_back(two_char);
                i += 2;
                continue;
            }
        }
        
        if (i + 2 < n) {
            std::u32string three_char = line.substr(i, 3);
            if (three_char == U"<->") {
                tokens.push_back(three_char);
                i += 3;
                continue;
            }
        }
        
        // Single character or regular identifier
        if ((line[i] >= U'a' && line[i] <= U'z') || 
            (line[i] >= U'A' && line[i] <= U'Z') ||
            (line[i] >= U'0' && line[i] <= U'9') ||
            line[i] == U'_') {
            // Collect alphanumeric identifier
            size_t start = i;
            while (i < n && ((line[i] >= U'a' && line[i] <= U'z') ||
                            (line[i] >= U'A' && line[i] <= U'Z') ||
                            (line[i] >= U'0' && line[i] <= U'9') ||
                            line[i] == U'_')) {
                ++i;
            }
            tokens.emplace_back(line.substr(start, i - start));
        } else {
            // Single symbol
            tokens.emplace_back(line.substr(i, 1));
            ++i;
        }
    }
    
    return tokens;
}

// Token type to string for debugging
inline std::string tokenTypeToString(WofTokenType type) {
    switch (type) {
        case WofTokenType::IDENTIFIER: return "IDENTIFIER";
        case WofTokenType::NUMBER: return "NUMBER";
        case WofTokenType::FORALL: return "FORALL";
        case WofTokenType::EXISTS: return "EXISTS";
        case WofTokenType::EXISTS_UNIQUE: return "EXISTS_UNIQUE";
        case WofTokenType::ELEMENT_OF: return "ELEMENT_OF";
        case WofTokenType::IMPLIES: return "IMPLIES";
        case WofTokenType::IFF: return "IFF";
        case WofTokenType::LOGICAL_AND: return "LOGICAL_AND";
        case WofTokenType::LOGICAL_OR: return "LOGICAL_OR";
        case WofTokenType::LOGICAL_NOT: return "LOGICAL_NOT";
        case WofTokenType::AXIOM: return "AXIOM";
        case WofTokenType::THEOREM: return "THEOREM";
        case WofTokenType::PROOF: return "PROOF";
        case WofTokenType::QED: return "QED";
        default: return "UNKNOWN";
    }
}

// Test the logic tokenizer
#ifdef WOFLANG_LOGIC_TEST
#include <iostream>

void test_logic_tokenizer() {
    std::string test_input = "axiom WOFL_UNIQUE: ∀w ∈ Wofl → ∃!h ∈ HowlPattern : w ↔ h";
    
    std::cout << "🧠 Testing WofLang Logic Tokenizer\n";
    std::cout << "Input: " << test_input << "\n\n";
    
    auto tokens = parseWoflangLogic(test_input);
    
    std::cout << "📊 Found " << tokens.size() << " logic tokens:\n";
    for (const auto& token : tokens) {
        std::cout << "  [" << tokenTypeToString(token.type) << "] '";
        for (char32_t c : token.token) {
            if (c < 0x80) {
                std::cout << static_cast<char>(c);
            } else {
                std::cout << "U+" << std::hex << c << std::dec;
            }
        }
        std::cout << "' at position " << token.position << "\n";
    }
}

int main() {
    test_logic_tokenizer();
    return 0;
}
#endif