#include "tokenizer.hpp"
#include <sstream>
#include <cctype>
#include <unordered_map>
#include <iostream>

std::string classify(const std::string& token) {
    if (token.empty()) return "UNKNOWN";
    if (std::isdigit(token[0])) return "NUMBER";
    if (std::isalpha(token[0])) return "IDENT";
    return "UNKNOWN";
}

namespace woflang {

static std::unordered_map<std::string, std::string> symbol_table;
static std::unordered_map<std::string, std::string> keyword_table;

void register_symbol(const std::string& symbol, const std::string& name) {
    symbol_table[symbol] = name;
}

void register_keyword(const std::string& keyword, const std::string& name) {
    keyword_table[keyword] = name;
}

std::vector<std::pair<std::string, std::string>> tokenize(const std::string& input) {
    std::vector<std::pair<std::string, std::string>> tokens;
    std::string token;
    std::istringstream iss(input);
    char c;

    while (iss.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!token.empty()) {
                tokens.emplace_back(classify(token), token);
                token.clear();
            }
        }
        else if (c == '(' || c == ')' || c == '[' || c == ']' ||
                 c == '{' || c == '}' || c == ',' || c == ';') {
            if (!token.empty()) {
                tokens.emplace_back(classify(token), token);
                token.clear();
            }
            tokens.emplace_back("PUNCT", std::string(1, c));
        }
        else {
            token += c;
        }
    }

    if (!token.empty()) {
        tokens.emplace_back(classify(token), token);
    }

    return tokens;
}

std::string classify(const std::string& token) {
    if (keyword_table.count(token)) return keyword_table[token];
    if (symbol_table.count(token)) return symbol_table[token];
    return "IDENT";
}

} // namespace woflang