// test_avx512.cpp — AVX-512 / scalar fallback tests
#include <cassert>
#include <iostream>
#include <cstdint>
#include "../include/flux/avx512_check.hpp"

using namespace flux;

void test_scalar_saturate() {
    assert(sat8(0) == 0);
    assert(sat8(127) == 127);
    assert(sat8(-127) == -127);
    assert(sat8(200) == 127);
    assert(sat8(-200) == -127);
    assert(sat8(128) == 127);
    assert(sat8(-128) == -127);
    std::cout << "  ✓ Scalar saturate\n";
}

void test_batch_saturate() {
    alignas(64) int32_t values[16] = {
        -200, -128, -127, -50, -1, 0, 1, 50,
        127, 128, 200, 0, 0, 0, 0, 0
    };
    AVX512Checker::saturate_16(values);

    assert(values[0] == -127);
    assert(values[1] == -127);
    assert(values[2] == -127);
    assert(values[3] == -50);
    assert(values[4] == -1);
    assert(values[5] == 0);
    assert(values[6] == 1);
    assert(values[7] == 50);
    assert(values[8] == 127);
    assert(values[9] == 127);
    assert(values[10] == 127);

    std::cout << "  ✓ Batch saturate (16 values)\n";
}

void test_check_16_all_pass() {
    alignas(64) int32_t values[16] = {10,20,30,40,50,60,70,80,10,20,30,40,50,60,70,80};
    alignas(64) int32_t lo[16]; alignas(64) int32_t hi[16];
    for (int i = 0; i < 16; ++i) { lo[i] = 0; hi[i] = 100; }

    uint16_t mask = AVX512Checker::check_16(values, lo, hi);
    assert(mask == 0xFFFF); // All pass

    std::cout << "  ✓ Check 16 all-pass (mask=0xFFFF)\n";
}

void test_check_16_all_fail() {
    alignas(64) int32_t values[16]; alignas(64) int32_t lo[16]; alignas(64) int32_t hi[16];
    for (int i = 0; i < 16; ++i) { values[i] = 200; lo[i] = 0; hi[i] = 100; }

    uint16_t mask = AVX512Checker::check_16(values, lo, hi);
    assert(mask == 0x0000); // All fail

    std::cout << "  ✓ Check 16 all-fail (mask=0x0000)\n";
}

void test_check_16_mixed() {
    alignas(64) int32_t values[16] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95, 105, 0, 50, 0, 0, 0};
    alignas(64) int32_t lo[16]; alignas(64) int32_t hi[16];
    for (int i = 0; i < 16; ++i) { lo[i] = 10; hi[i] = 100; }

    uint16_t mask = AVX512Checker::check_16(values, lo, hi);

    // 5<10 fail, 15 pass, 25 pass, ..., 95 pass, 105>100 fail, 0<10 fail, 50 pass, 0 fail, 0 fail, 0 fail
    assert(!(mask & (1<<0)));  // 5 → fail
    assert(mask & (1<<1));     // 15 → pass
    assert(!(mask & (1<<10))); // 105 → fail
    assert(mask & (1<<12));    // 50 → pass

    std::cout << "  ✓ Check 16 mixed (mask=0x" << std::hex << mask << std::dec << ")\n";
}

int main() {
    std::cout << "=== AVX-512 / Scalar Tests ===\n";
    test_scalar_saturate();
    test_batch_saturate();
    test_check_16_all_pass();
    test_check_16_all_fail();
    test_check_16_mixed();
    std::cout << "\nAll AVX-512 tests passed!\n";
    return 0;
}
