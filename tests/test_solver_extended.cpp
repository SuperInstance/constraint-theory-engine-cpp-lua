// test_solver_extended.cpp — Extended CDCL solver tests
#include <cassert>
#include <iostream>
#include "../include/flux/solver.hpp"

using namespace flux;

void test_empty_solver() {
    CDCLSolver solver;
    auto result = solver.solve();
    assert(result.satisfiable);
    std::cout << "  ✓ Empty formula (trivially SAT)\n";
}

void test_single_var_true() {
    CDCLSolver solver;
    solver.add_clause({1});
    auto result = solver.solve();
    assert(result.satisfiable);
    assert(result.assignment[1] == true);
    std::cout << "  ✓ Single variable true\n";
}

void test_single_var_false() {
    CDCLSolver solver;
    solver.add_clause({-1});
    auto result = solver.solve();
    assert(result.satisfiable);
    assert(result.assignment[1] == false);
    std::cout << "  ✓ Single variable false\n";
}

void test_unit_propagation() {
    // {1}, {-1, 2} → x1=true, x2 must be true
    CDCLSolver solver;
    solver.add_clause({1});
    solver.add_clause({-1, 2});
    auto result = solver.solve();
    assert(result.satisfiable);
    assert(result.assignment[1] == true);
    assert(result.assignment[2] == true);
    assert(result.propagations >= 1);
    std::cout << "  ✓ Unit propagation\n";
}

void test_chain_propagation() {
    // {1}, {-1, 2}, {-2, 3}, {-3, 4}
    CDCLSolver solver;
    solver.add_clause({1});
    solver.add_clause({-1, 2});
    solver.add_clause({-2, 3});
    solver.add_clause({-3, 4});
    auto result = solver.solve();
    assert(result.satisfiable);
    assert(result.assignment[1] == true);
    assert(result.assignment[2] == true);
    assert(result.assignment[3] == true);
    assert(result.assignment[4] == true);
    std::cout << "  ✓ Chain propagation (4 vars)\n";
}

void test_multiple_unsat_pairs() {
    // {1, -1} → UNSAT (simplest)
    CDCLSolver solver;
    solver.add_clause({1});
    solver.add_clause({-1});
    auto result = solver.solve();
    assert(!result.satisfiable);
    assert(result.conflicts > 0);
    std::cout << "  ✓ Multiple UNSAT pairs\n";
}

void test_large_unsat() {
    // All combos of 2 vars are impossible
    CDCLSolver solver;
    solver.add_clause({1, 2});
    solver.add_clause({1, -2});
    solver.add_clause({-1, 2});
    solver.add_clause({-1, -2});
    auto result = solver.solve();
    assert(!result.satisfiable);
    std::cout << "  ✓ Large UNSAT (4 clauses, 2 vars)\n";
}

void test_large_sat() {
    // 10 variables, each has a positive unit clause
    CDCLSolver solver;
    for (int i = 1; i <= 10; ++i) {
        solver.add_clause({i});
    }
    auto result = solver.solve();
    assert(result.satisfiable);
    for (int i = 1; i <= 10; ++i) {
        assert(result.assignment[i] == true);
    }
    std::cout << "  ✓ Large SAT (10 unit clauses)\n";
}

void test_tautology_clause() {
    // {1, -1} is a tautology — always satisfied
    CDCLSolver solver;
    solver.add_clause({1, -1});
    auto result = solver.solve();
    assert(result.satisfiable);
    std::cout << "  ✓ Tautology clause\n";
}

void test_solver_result_stats() {
    CDCLSolver solver;
    solver.add_clause({1, 2});
    solver.add_clause({-1, 2});
    solver.add_clause({-2, 1});

    auto result = solver.solve();
    assert(result.satisfiable);
    // Should have some decisions/propagations recorded
    assert(result.decisions + result.propagations > 0);
    std::cout << "  ✓ Solver result stats (dec=" << result.decisions
              << ", prop=" << result.propagations
              << ", conf=" << result.conflicts << ")\n";
}

void test_learned_clauses_present() {
    // Formula that forces conflict-driven learning
    CDCLSolver solver;
    solver.add_clause({1, 2});
    solver.add_clause({1, -2});
    solver.add_clause({-1, 2});
    solver.add_clause({-1, -2});
    auto result = solver.solve();
    assert(!result.satisfiable);
    assert(!result.learned_clauses.empty());
    std::cout << "  ✓ Learned clauses present (" << result.learned_clauses.size() << ")\n";
}

void test_clauses_accessor() {
    CDCLSolver solver;
    solver.add_clause({1, 2});
    solver.add_clause({3, 4});

    auto& clauses = solver.clauses();
    assert(clauses.size() >= 2);
    std::cout << "  ✓ Clauses accessor\n";
}

void test_five_var_sat() {
    CDCLSolver solver;
    solver.add_clause({1, 2, 3});
    solver.add_clause({-1, 4});
    solver.add_clause({-2, 5});
    solver.add_clause({-3, -4, -5});
    solver.add_clause({4, 5});

    auto result = solver.solve();
    // Verify: if SAT, check assignments satisfy all original clauses
    if (result.satisfiable) {
        for (size_t i = 0; i < 5; ++i) {
            const auto& clause = solver.clauses()[i];
            bool sat = false;
            for (auto lit : clause) {
                auto var = std::abs(lit);
                auto it = result.assignment.find(var);
                if (it != result.assignment.end()) {
                    bool val = it->second;
                    if ((lit > 0 && val) || (lit < 0 && !val)) {
                        sat = true;
                        break;
                    }
                }
            }
            assert(sat);
        }
    }
    std::cout << "  ✓ Five-variable SAT with verification\n";
}

void test_horn_clauses() {
    // Horn clauses: at most one positive literal per clause
    CDCLSolver solver;
    solver.add_clause({-1, -2, 3});  // ¬x1 ∧ ¬x2 → x3
    solver.add_clause({-3, 4});      // ¬x3 → x4
    solver.add_clause({1});          // x1
    solver.add_clause({2});          // x2

    auto result = solver.solve();
    assert(result.satisfiable);
    assert(result.assignment[1] == true);
    assert(result.assignment[2] == true);
    assert(result.assignment[3] == true);
    assert(result.assignment[4] == true);
    std::cout << "  ✓ Horn clauses (forward chaining)\n";
}

int main() {
    std::cout << "=== Extended Solver Tests ===\n";
    test_empty_solver();
    test_single_var_true();
    test_single_var_false();
    test_unit_propagation();
    test_chain_propagation();
    test_multiple_unsat_pairs();
    test_large_unsat();
    test_large_sat();
    test_tautology_clause();
    test_solver_result_stats();
    test_learned_clauses_present();
    test_clauses_accessor();
    test_five_var_sat();
    test_horn_clauses();
    std::cout << "\nAll extended solver tests passed!\n";
    return 0;
}
