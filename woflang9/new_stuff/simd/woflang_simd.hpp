// parse_woflang_simd.hpp
#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include "simd_selector.hpp"

// Token structure for downstream use
struct WofToken {
    std::u32string token; // Full token text (Unicode)
    uint16_t opcode;      // Token ID (optional; fill with 0 if not using)
    size_t position;      // Offset in original input
};

// UTF-8 to UTF-32 converter (from v3)
inline std::u32string utf8_to_utf32(const std::string& input) {
    std::u32string out;
    size_t i = 0;
    while (i < input.size()) {
        char32_t ch = 0;
        unsigned char c = static_cast<unsigned char>(input[i]);
        if (c < 0x80) {
            ch = c;
            ++i;
        } else if ((c >> 5) == 0x6) {
            ch = ((c & 0x1F) << 6) | (static_cast<unsigned char>(input[i + 1]) & 0x3F);
            i += 2;
        } else if ((c >> 4) == 0xE) {
            ch = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 6) |
                 (static_cast<unsigned char>(input[i + 2]) & 0x3F);
            i += 3;
        } else if ((c >> 3) == 0x1E) {
            ch = ((c & 0x07) << 18) | ((static_cast<unsigned char>(input[i + 1]) & 0x3F) << 12) |
                 ((static_cast<unsigned char>(input[i + 2]) & 0x3F) << 6) | (static_cast<unsigned char>(input[i + 3]) & 0x3F);
            i += 4;
        }
        out.push_back(ch);
    }
    return out;
}

// The main SIMD parser entry point (UTF-8 or UTF-32 input)
inline std::vector<WofToken> parseWoflangSIMD(const std::string& input_utf8) {
    std::u32string input = utf8_to_utf32(input_utf8);
    std::vector<std::u32string> tokens = simd_tokenize(input);

    std::vector<WofToken> out;
    size_t pos = 0;
    for (const auto& t : tokens) {
        out.push_back(WofToken{ t, 0, pos });
        pos += t.size();
    }
    return out;
}

// Optional: Overload for already-UTF32 input
inline std::vector<WofToken> parseWoflangSIMD(const std::u32string& input) {
    std::vector<std::u32string> tokens = simd_tokenize(input);

    std::vector<WofToken> out;
    size_t pos = 0;
    for (const auto& t : tokens) {
        out.push_back(WofToken{ t, 0, pos });
        pos += t.size();
    }
    return out;
}
