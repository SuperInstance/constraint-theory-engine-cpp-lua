// test_constraint_extended.cpp — Extended constraint checking tests
#include <cassert>
#include <iostream>
#include "../include/flux/constraint.hpp"

using namespace flux;

void test_named_constraint() {
    Constraint c(10, 50, Severity::CRITICAL, "test_sensor");

    auto r = ConstraintChecker::check(25, c);
    assert(r.pass);
    assert(c.name != nullptr);
    assert(std::string(c.name) == "test_sensor");

    std::cout << "  ✓ Named constraint\n";
}

void test_check_all_single_pass() {
    Constraint c(0, 100);
    auto r = ConstraintChecker::check_all(50, &c, 1);
    assert(r.pass);

    std::cout << "  ✓ check_all single constraint pass\n";
}

void test_check_all_single_fail() {
    Constraint c(0, 100);
    auto r = ConstraintChecker::check_all(150, &c, 1);
    assert(!r.pass);
    assert(r.error_mask & ERR_HI);

    std::cout << "  ✓ check_all single constraint fail\n";
}

void test_check_all_multiple_mixed() {
    Constraint constraints[] = {
        Constraint(0, 100),   // pass: 50 in [0,100]
        Constraint(60, 90),   // fail: 50 < 60
        Constraint(10, 80),   // pass: 50 in [10,80]
    };

    auto r = ConstraintChecker::check_all(50, constraints, 3);
    assert(!r.pass);
    assert(r.error_mask & ERR_LO); // failed constraint 2

    std::cout << "  ✓ check_all multiple constraints mixed\n";
}

void test_check_all_all_pass() {
    Constraint constraints[] = {
        Constraint(0, 100),
        Constraint(10, 90),
        Constraint(25, 75),
    };

    auto r = ConstraintChecker::check_all(50, constraints, 3);
    assert(r.pass);
    assert(r.error_mask == ERR_NONE);

    std::cout << "  ✓ check_all all pass\n";
}

void test_check_all_zero_constraints() {
    Constraint c(0, 100);
    auto r = ConstraintChecker::check_all(50, &c, 0);
    assert(r.pass);

    std::cout << "  ✓ check_all zero constraints\n";
}

void test_batch_empty() {
    Constraint constraints[] = {Constraint(0, 100)};
    int32_t values[] = {};

    // Batch with 0 values
    auto result = ConstraintChecker::check_batch(values, 0, constraints, 1);
    assert(result.total == 0);
    assert(result.passed == 0);
    assert(result.failed == 0);

    std::cout << "  ✓ Batch check empty\n";
}

void test_batch_all_pass() {
    Constraint constraints[] = {Constraint(0, 100)};
    int32_t values[] = {0, 25, 50, 75, 100};

    auto result = ConstraintChecker::check_batch(values, 5, constraints, 1);
    assert(result.total == 5);
    assert(result.passed == 5);
    assert(result.failed == 0);

    std::cout << "  ✓ Batch all pass\n";
}

void test_batch_all_fail() {
    Constraint constraints[] = {Constraint(10, 20)};
    int32_t values[] = {-5, 0, 5, 25, 30};

    auto result = ConstraintChecker::check_batch(values, 5, constraints, 1);
    assert(result.total == 5);
    assert(result.passed == 0);
    assert(result.failed == 5);

    std::cout << "  ✓ Batch all fail\n";
}

void test_sat8_boundary_values() {
    assert(sat8(-127) == -127);
    assert(sat8(127) == 127);
    assert(sat8(0) == 0);
    assert(sat8(126) == 126);
    assert(sat8(-126) == -126);
    assert(sat8(128) == 127);
    assert(sat8(-128) == -127);
    assert(sat8(9999) == 127);
    assert(sat8(-9999) == -127);

    std::cout << "  ✓ sat8 boundary values\n";
}

void test_constraint_constructor_saturation() {
    // Constraint constructor saturates lo and hi
    Constraint c1(-200, 200);
    assert(c1.lo == -127);
    assert(c1.hi == 127);

    Constraint c2(50, 50);
    assert(c2.lo == 50);
    assert(c2.hi == 50);

    std::cout << "  ✓ Constraint constructor saturation\n";
}

void test_severity_enums() {
    assert(static_cast<int>(Severity::PASS) == 0);
    assert(static_cast<int>(Severity::CAUTION) == 1);
    assert(static_cast<int>(Severity::WARNING) == 2);
    assert(static_cast<int>(Severity::CRITICAL) == 3);

    std::cout << "  ✓ Severity enum values\n";
}

void test_error_mask_flags() {
    assert(ERR_NONE == 0x00);
    assert(ERR_LO == 0x01);
    assert(ERR_HI == 0x02);
    assert(ERR_SATURATED == 0x04);
    assert(ERR_SEVERITY == 0x08);

    // Combinations
    assert((ERR_LO | ERR_HI) == 0x03);
    assert((ERR_LO | ERR_SATURATED) == 0x05);

    std::cout << "  ✓ Error mask flags\n";
}

void test_negative_range() {
    Constraint c(-50, -10);

    auto r_pass = ConstraintChecker::check(-25, c);
    assert(r_pass.pass);

    auto r_lo = ConstraintChecker::check(-60, c);
    assert(!r_lo.pass);
    assert(r_lo.error_mask & ERR_LO);

    auto r_hi = ConstraintChecker::check(0, c);
    assert(!r_hi.pass);
    assert(r_hi.error_mask & ERR_HI);

    std::cout << "  ✓ Negative range constraints\n";
}

void test_exact_bound_match() {
    Constraint c(42, 42);

    auto r_pass = ConstraintChecker::check(42, c);
    assert(r_pass.pass);

    auto r_lo = ConstraintChecker::check(41, c);
    assert(!r_lo.pass);

    auto r_hi = ConstraintChecker::check(43, c);
    assert(!r_hi.pass);

    std::cout << "  ✓ Exact bound match (lo == hi)\n";
}

void test_default_severity() {
    Constraint c(0, 100); // default severity
    assert(c.severity == Severity::WARNING);

    auto r = ConstraintChecker::check(150, c);
    assert(r.severity == Severity::WARNING);

    std::cout << "  ✓ Default severity is WARNING\n";
}

int main() {
    std::cout << "=== Extended Constraint Tests ===\n";
    test_named_constraint();
    test_check_all_single_pass();
    test_check_all_single_fail();
    test_check_all_multiple_mixed();
    test_check_all_all_pass();
    test_check_all_zero_constraints();
    test_batch_empty();
    test_batch_all_pass();
    test_batch_all_fail();
    test_sat8_boundary_values();
    test_constraint_constructor_saturation();
    test_severity_enums();
    test_error_mask_flags();
    test_negative_range();
    test_exact_bound_match();
    test_default_severity();
    std::cout << "\nAll extended constraint tests passed!\n";
    return 0;
}
