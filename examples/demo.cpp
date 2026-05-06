// examples/demo.cpp — Quick start demo
#include <iostream>
#include "../include/flux/constraint.hpp"
#include "../include/flux/solver.hpp"
#include "../include/flux/emitter.hpp"
#include "../include/flux/avx512_check.hpp"

using namespace flux;

int main() {
    std::cout << "╔══════════════════════════════════════════════════╗\n";
    std::cout << "║   FLUX Constraint Engine — C++ / Lua Demo       ║\n";
    std::cout << "║   CDCL Solver + AVX-512 + LLVM IR Emitter       ║\n";
    std::cout << "╚══════════════════════════════════════════════════╝\n\n";

    // 1. Basic constraint checking
    std::cout << "1. Constraint Checking\n";
    std::cout << "   ─────────────────────\n";

    Constraint battery_temp(15, 55, Severity::CRITICAL, "battery_temp");

    int32_t test_vals[] = {10, 25, 60};
    for (auto v : test_vals) {
        auto r = ConstraintChecker::check(v, battery_temp);
        std::cout << "   " << v << "°C → " << (r.pass ? "PASS" : "FAIL")
                  << " (mask=0x" << std::hex << (int)r.error_mask << std::dec
                  << ", sev=" << (int)r.severity << ")\n";
    }

    // 2. AVX-512 batch check
    std::cout << "\n2. AVX-512 Batch Check (16 values)\n";
    std::cout << "   ────────────────────────────────\n";

    alignas(64) int32_t values[16], lo[16], hi[16];
    for (int i = 0; i < 16; ++i) {
        values[i] = i * 10;
        lo[i] = 20;
        hi[i] = 120;
    }

    uint16_t mask = AVX512Checker::check_16(values, lo, hi);
    int pass_count = __builtin_popcount(mask);
    std::cout << "   Mask: 0x" << std::hex << mask << std::dec
              << " (" << pass_count << "/16 pass)\n";

    // 3. CDCL Solver
    std::cout << "\n3. CDCL SAT Solver\n";
    std::cout << "   ────────────────\n";

    CDCLSolver solver;
    solver.add_clause({1, 2, -3});
    solver.add_clause({-1, 2});
    solver.add_clause({-2, 3});

    auto result = solver.solve();
    std::cout << "   Clauses: (x1∨x2∨¬x3) ∧ (¬x1∨x2) ∧ (¬x2∨x3)\n";
    std::cout << "   Result: " << (result.satisfiable ? "SATISFIABLE" : "UNSATISFIABLE") << "\n";

    if (result.satisfiable) {
        std::cout << "   Assignment: ";
        for (const auto& [var, val] : result.assignment) {
            std::cout << "x" << var << "=" << (val ? "T" : "F") << " ";
        }
        std::cout << "\n";
    }

    // 4. LLVM IR Emission
    std::cout << "\n4. LLVM IR Generation\n";
    std::cout << "   ───────────────────\n";

    LLVMEmitter emitter;
    std::string ir = emitter.emit_module(solver.clauses());

    // Print first few lines
    size_t lines = 0;
    for (char c : ir) {
        if (c == '\n') { ++lines; if (lines > 8) break; }
        std::cout << c;
    }
    std::cout << "   ... (" << ir.size() << " bytes total)\n";

    std::cout << "\n✓ Demo complete.\n";
    return 0;
}
