#pragma once
/// flux::Constraint — Core constraint types for the FLUX engine
///
/// INT8-saturated flat-bounds constraint checking.
/// Every constraint is a {lo, hi} pair checked against a saturated int8 value.
/// Severity levels: PASS=0, CAUTION=1, WARNING=2, CRITICAL=3.

#include <cstdint>
#include <string>
#include <vector>
#include <cstring>

namespace flux {

/// Saturate a value to INT8 range [-127, 127]
inline int32_t sat8(int32_t val) noexcept {
    if (val < -127) return -127;
    if (val > 127)  return 127;
    return val;
}

/// Severity levels matching DO-178C / ISO 26262 / IEC 61508
enum class Severity : uint8_t {
    PASS     = 0,
    CAUTION  = 1,
    WARNING  = 2,
    CRITICAL = 3
};

/// Error mask bits for batch checking
enum ErrorMask : uint8_t {
    ERR_NONE       = 0x00,
    ERR_LO         = 0x01,  // Below lower bound
    ERR_HI         = 0x02,  // Above upper bound
    ERR_SATURATED  = 0x04,  // Input was saturated
    ERR_SEVERITY   = 0x08,  // Severity threshold exceeded
};

/// A single constraint: value must be in [lo, hi] (INT8 saturated)
struct Constraint {
    int32_t lo;
    int32_t hi;
    Severity severity;
    const char* name;

    Constraint(int32_t lo_, int32_t hi_, Severity sev = Severity::WARNING, const char* name_ = "")
        : lo(sat8(lo_)), hi(sat8(hi_)), severity(sev), name(name_) {}
};

/// Result of checking a single value against a constraint
struct ConstraintResult {
    bool     pass;
    uint8_t  error_mask;
    Severity severity;
    int32_t  saturated_value;

    ConstraintResult() noexcept
        : pass(true), error_mask(ERR_NONE), severity(Severity::PASS), saturated_value(0) {}
};

/// Batch result for multiple constraints
struct BatchResult {
    size_t total;
    size_t passed;
    size_t failed;
    std::vector<ConstraintResult> results;

    BatchResult(size_t n) : total(n), passed(0), failed(0), results(n) {}
};

/// Core constraint checker — single and batch operations
class ConstraintChecker {
public:
    /// Check a single value against a single constraint
    static ConstraintResult check(int32_t value, const Constraint& c) noexcept {
        ConstraintResult r;
        int32_t sv = sat8(value);
        r.saturated_value = sv;

        if (sv < c.lo) {
            r.pass = false;
            r.error_mask |= ERR_LO;
            r.severity = c.severity;
        } else if (sv > c.hi) {
            r.pass = false;
            r.error_mask |= ERR_HI;
            r.severity = c.severity;
        }

        if (sv != value) {
            r.error_mask |= ERR_SATURATED;
        }

        return r;
    }

    /// Check a single value against multiple constraints
    static ConstraintResult check_all(int32_t value, const Constraint* constraints, size_t n) noexcept {
        ConstraintResult worst;
        worst.saturated_value = sat8(value);
        bool any_fail = false;

        for (size_t i = 0; i < n; ++i) {
            ConstraintResult r = check(value, constraints[i]);
            if (!r.pass) {
                any_fail = true;
                worst.error_mask |= r.error_mask;
                if (static_cast<uint8_t>(r.severity) > static_cast<uint8_t>(worst.severity)) {
                    worst.severity = r.severity;
                }
            }
        }

        worst.pass = !any_fail;
        return worst;
    }

    /// Batch check: N values × M constraints
    static BatchResult check_batch(
        const int32_t* values, size_t n,
        const Constraint* constraints, size_t m
    ) {
        BatchResult result(n);
        for (size_t i = 0; i < n; ++i) {
            result.results[i] = check_all(values[i], constraints, m);
            if (result.results[i].pass) {
                ++result.passed;
            } else {
                ++result.failed;
            }
        }
        return result;
    }

#ifdef __AVX512F__
    /// AVX-512 vectorized batch checking (16 values at once)
    static void check_batch_avx512(
        const int32_t* values, size_t n,
        const Constraint* constraints, size_t m,
        ConstraintResult* results
    );
#endif
};

} // namespace flux
