// simd_utf32_avx2.hpp
#pragma once

#include <immintrin.h>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>

inline std::vector<std::u32string> simd_tokenize_avx2(const std::u32string& line) {
    std::vector<std::u32string> tokens;
    const size_t n = line.size();
    const size_t stride = 8;
    size_t i = 0;

    // Set up whitespace delimiters (space, tab, newline, carriage return)
    const char32_t ws[4] = { U' ', U'\t', U'\n', U'\r' };
    __m256i ws0 = _mm256_set1_epi32(ws[0]);
    __m256i ws1 = _mm256_set1_epi32(ws[1]);
    __m256i ws2 = _mm256_set1_epi32(ws[2]);
    __m256i ws3 = _mm256_set1_epi32(ws[3]);

    size_t token_start = 0;
    bool in_token = false;

    while (i + stride <= n) {
        __m256i chars = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(&line[i]));
        // Compare each char in the vector against all whitespace options
        __m256i mask0 = _mm256_cmpeq_epi32(chars, ws0);
        __m256i mask1 = _mm256_cmpeq_epi32(chars, ws1);
        __m256i mask2 = _mm256_cmpeq_epi32(chars, ws2);
        __m256i mask3 = _mm256_cmpeq_epi32(chars, ws3);
        __m256i mask = _mm256_or_si256(_mm256_or_si256(mask0, mask1), _mm256_or_si256(mask2, mask3));

        uint32_t bitmask = _mm256_movemask_ps(_mm256_castsi256_ps(mask));

        for (int j = 0; j < stride; ++j) {
            bool is_ws = ((bitmask >> j) & 1);
            if (is_ws) {
                if (in_token) {
                    tokens.emplace_back(line.substr(token_start, i + j - token_start));
                    in_token = false;
                }
            } else {
                if (!in_token) {
                    token_start = i + j;
                    in_token = true;
                }
            }
        }
        i += stride;
    }

    // Handle any remaining chars (tail)
    while (i < n) {
        if (line[i] == U' ' || line[i] == U'\t' || line[i] == U'\n' || line[i] == U'\r') {
            if (in_token) {
                tokens.emplace_back(line.substr(token_start, i - token_start));
                in_token = false;
            }
        } else {
            if (!in_token) {
                token_start = i;
                in_token = true;
            }
        }
        ++i;
    }
    if (in_token) {
        tokens.emplace_back(line.substr(token_start, i - token_start));
    }
    return tokens;
}
