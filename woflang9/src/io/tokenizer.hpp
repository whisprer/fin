#pragma once
#include <string>
#include <vector>
#include <utility>

namespace woflang {
    void register_symbol(const std::string& symbol, const std::string& name);
    void register_keyword(const std::string& keyword, const std::string& name);
    std::vector<std::pair<std::string, std::string>> tokenize(const std::string& input);
}
