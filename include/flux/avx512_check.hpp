#pragma once
/// flux::AVX512Check — AVX-512 vectorized constraint checking
///
/// Checks 16 values simultaneously using AVX-512 SIMD intrinsics.
/// Falls back to scalar when AVX-512 is unavailable.

#include "constraint.hpp"

#ifdef __AVX512F__
#include <immintrin.h>
#endif

namespace flux {

/// AVX-512 batch checker — 16 constraints at once
class AVX512Checker {
public:
    /// Check 16 values against 16 [lo,hi] pairs simultaneously
    /// Returns 16-bit mask where bit i = 1 means values[i] passes
    static uint16_t check_16(
        const int32_t* values,
        const int32_t* lo,
        const int32_t* hi
    ) noexcept {
#ifdef __AVX512F__
        __m512i v = _mm512_loadu_si512(reinterpret_cast<const void*>(values));
        __m512i l = _mm512_loadu_si512(reinterpret_cast<const void*>(lo));
        __m512i h = _mm512_loadu_si512(reinterpret_cast<const void*>(hi));

        __mmask16 pass_lo = _mm512_cmpge_epi32_mask(v, l);
        __mmask16 pass_hi = _mm512_cmple_epi32_mask(v, h);
        __mmask16 result  = pass_lo & pass_hi;

        return static_cast<uint16_t>(result);
#else
        // Scalar fallback
        uint16_t mask = 0;
        for (int i = 0; i < 16; ++i) {
            int32_t sv = sat8(values[i]);
            if (sv >= lo[i] && sv <= hi[i]) {
                mask |= (1 << i);
            }
        }
        return mask;
#endif
    }

    /// Saturate 16 values to INT8 range simultaneously
    static void saturate_16(int32_t* values) noexcept {
#ifdef __AVX512F__
        __m512i v = _mm512_loadu_si512(reinterpret_cast<void*>(values));
        __m512i lo = _mm512_set1_epi32(-127);
        __m512i hi = _mm512_set1_epi32(127);
        __m512i result = _mm512_min_epi32(_mm512_max_epi32(v, lo), hi);
        _mm512_storeu_si512(reinterpret_cast<void*>(values), result);
#else
        for (int i = 0; i < 16; ++i) {
            values[i] = sat8(values[i]);
        }
#endif
    }

    /// Generate error masks for 16 checks
    static void check_with_errors(
        const int32_t* values,
        const Constraint* constraints,
        size_t n,
        ConstraintResult* results
    ) {
        size_t batches = n / 16;
        size_t remainder = n % 16;

        for (size_t b = 0; b < batches; ++b) {
            size_t base = b * 16;

            // Extract lo/hi arrays for batch
            alignas(64) int32_t lo[16], hi[16];
            for (int i = 0; i < 16; ++i) {
                lo[i] = constraints[base + i].lo;
                hi[i] = constraints[base + i].hi;
            }

            uint16_t mask = check_16(values + base, lo, hi);

            for (int i = 0; i < 16; ++i) {
                results[base + i].saturated_value = sat8(values[base + i]);
                results[base + i].pass = (mask >> i) & 1;
                if (!results[base + i].pass) {
                    results[base + i].error_mask = ERR_LO | ERR_HI;
                    results[base + i].severity = constraints[base + i].severity;
                }
            }
        }

        // Handle remainder
        for (size_t i = batches * 16; i < n; ++i) {
            results[i] = ConstraintChecker::check(values[i], constraints[i]);
        }
    }
};

} // namespace flux
