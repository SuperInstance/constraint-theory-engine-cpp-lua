// test_constraint.cpp — Unit tests for constraint checking
#include <cassert>
#include <iostream>
#include "../include/flux/constraint.hpp"
#include "../include/flux/avx512_check.hpp"

using namespace flux;

void test_basic_check() {
    Constraint c(10, 50);
    auto r_pass = ConstraintChecker::check(25, c);
    assert(r_pass.pass == true);
    assert(r_pass.error_mask == ERR_NONE);

    auto r_lo = ConstraintChecker::check(5, c);
    assert(r_lo.pass == false);
    assert(r_lo.error_mask & ERR_LO);

    auto r_hi = ConstraintChecker::check(60, c);
    assert(r_hi.pass == false);
    assert(r_hi.error_mask & ERR_HI);

    std::cout << "  ✓ Basic check\n";
}

void test_boundary() {
    Constraint c(10, 50);

    auto r_lo = ConstraintChecker::check(10, c);
    assert(r_lo.pass == true);

    auto r_hi = ConstraintChecker::check(50, c);
    assert(r_hi.pass == true);

    auto r_lo_m1 = ConstraintChecker::check(9, c);
    assert(r_lo_m1.pass == false);

    auto r_hi_p1 = ConstraintChecker::check(51, c);
    assert(r_hi_p1.pass == false);

    std::cout << "  ✓ Boundary conditions\n";
}

void test_saturation() {
    Constraint c(-50, 50);

    // Value > 127 gets saturated
    auto r_high = ConstraintChecker::check(200, c);
    assert(r_high.saturated_value == 127);
    assert(r_high.pass == false); // 127 > 50
    assert(r_high.error_mask & ERR_SATURATED);

    // Value < -127 gets saturated
    auto r_low = ConstraintChecker::check(-200, c);
    assert(r_low.saturated_value == -127);
    assert(r_low.pass == false); // -127 < -50
    assert(r_low.error_mask & ERR_SATURATED);

    std::cout << "  ✓ INT8 saturation\n";
}

void test_severity() {
    Constraint c_caution(0, 100, Severity::CAUTION);
    Constraint c_warning(0, 100, Severity::WARNING);
    Constraint c_critical(0, 100, Severity::CRITICAL);

    auto r_c = ConstraintChecker::check(150, c_caution);
    assert(r_c.severity == Severity::CAUTION);

    auto r_w = ConstraintChecker::check(150, c_warning);
    assert(r_w.severity == Severity::WARNING);

    auto r_cr = ConstraintChecker::check(150, c_critical);
    assert(r_cr.severity == Severity::CRITICAL);

    std::cout << "  ✓ Severity levels\n";
}

void test_batch() {
    Constraint constraints[] = {
        Constraint(0, 100),
        Constraint(-50, 50),
        Constraint(10, 90),
    };

    int32_t values[] = {50, -60, 5, 200, 0};
    auto result = ConstraintChecker::check_batch(values, 5, constraints, 3);

    assert(result.total == 5);
    assert(result.passed > 0);
    assert(result.failed > 0);

    std::cout << "  ✓ Batch check (passed=" << result.passed
              << ", failed=" << result.failed << ")\n";
}

void test_avx512_scalar_fallback() {
    // Test the scalar fallback path (works with or without AVX-512)
    alignas(64) int32_t values[16], lo[16], hi[16];
    for (int i = 0; i < 16; ++i) {
        values[i] = i * 10;
        lo[i] = 20;
        hi[i] = 120;
    }

    uint16_t mask = AVX512Checker::check_16(values, lo, hi);

    // values[0]=0, values[1]=10, values[2]=20 (lo boundary), values[3]=30 ... values[12]=120 (hi boundary)
    assert(!(mask & (1 << 0)));  // 0 < 20 → fail
    assert(!(mask & (1 << 1)));  // 10 < 20 → fail
    assert(mask & (1 << 2));     // 20 ≥ 20 → pass
    assert(mask & (1 << 12));    // 120 ≤ 120 → pass
    assert(!(mask & (1 << 13))); // 130 > 120 → fail

    std::cout << "  ✓ AVX-512 / scalar fallback (mask=0x" << std::hex << mask << std::dec << ")\n";
}

int main() {
    std::cout << "=== Constraint Tests ===\n";
    test_basic_check();
    test_boundary();
    test_saturation();
    test_severity();
    test_batch();
    test_avx512_scalar_fallback();
    std::cout << "\nAll constraint tests passed!\n";
    return 0;
}
