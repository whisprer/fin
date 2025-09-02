// simd_selector.hpp
#pragma once

#include <vector>
#include <string>

#if defined(_MSC_VER)
    #include <intrin.h>
#elif defined(__GNUC__) || defined(__clang__)
    #include <cpuid.h>
#endif

// --- SIMD Backends ---
#include "simd_utf32_avx2.hpp"   // Your AVX2-accelerated parsing
// #include "simd_utf32_avx512.hpp" // (stub for future)
// #include "simd_utf32_neon.hpp"   // (stub for future)

// --- Feature Detection ---
inline bool isAVX2Supported() {
#if defined(_MSC_VER)
    int info[4];
    __cpuid(info, 0);
    int nIds = info[0];
    if (nIds >= 7) {
        __cpuidex(info, 7, 0);
        return (info[1] & ((int)1 << 5)) != 0;
    }
    return false;
#elif defined(__GNUC__) || defined(__clang__)
    unsigned int eax, ebx, ecx, edx;
    if (!__get_cpuid_max(0, nullptr)) return false;
    __cpuid_count(7, 0, eax, ebx, ecx, edx);
    return (ebx & (1 << 5)) != 0;
#else
    return false;
#endif
}

// --- API: Tokenize a UTF-32 string using best available SIMD, fallback if needed ---
inline std::vector<std::u32string> simd_tokenize(const std::u32string& line) {
    if (isAVX2Supported()) {
        return simd_tokenize_avx2(line); // Implement this in simd_utf32_avx2.hpp
    }
    // TODO: if (isAVX512Supported()) ... etc.
    // TODO: if (isNEONSupported()) ...
    // Fallback: scalar
    std::vector<std::u32string> tokens;
    size_t i = 0, start = 0;
    while (i < line.size()) {
        while (i < line.size() && std::iswspace(line[i])) ++i;
        start = i;
        while (i < line.size() && !std::iswspace(line[i])) ++i;
        if (start < i)
            tokens.emplace_back(line.substr(start, i - start));
    }
    return tokens;
}

