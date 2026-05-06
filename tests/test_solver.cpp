// test_solver.cpp — CDCL solver unit tests
#include <cassert>
#include <iostream>
#include "../include/flux/solver.hpp"

using namespace flux;

void test_simple_sat() {
    CDCLSolver solver;
    solver.add_clause({1, 2});      // x1 OR x2
    solver.add_clause({-1, 2});     // NOT x1 OR x2
    solver.add_clause({-2, 1});     // NOT x2 OR x1

    auto result = solver.solve();
    assert(result.satisfiable);
    std::cout << "  ✓ Simple SAT\n";
}

void test_simple_unsat() {
    CDCLSolver solver;
    solver.add_clause({1});    // x1 must be true
    solver.add_clause({-1});   // x1 must be false → contradiction

    auto result = solver.solve();
    assert(!result.satisfiable);
    std::cout << "  ✓ Simple UNSAT\n";
}

void test_three_var() {
    CDCLSolver solver;
    solver.add_clause({1, 2, -3});
    solver.add_clause({-1, 2});
    solver.add_clause({-2, 3});

    auto result = solver.solve();
    assert(result.satisfiable);

    // Verify assignment satisfies all clauses
    for (const auto& clause : solver.clauses()) {
        bool satisfied = false;
        for (auto lit : clause) {
            auto var = std::abs(lit);
            auto it = result.assignment.find(var);
            if (it != result.assignment.end()) {
                bool val = it->second;
                if ((lit > 0 && val) || (lit < 0 && !val)) {
                    satisfied = true;
                    break;
                }
            }
        }
        assert(satisfied);
    }

    std::cout << "  ✓ Three-variable SAT with verification\n";
}

void test_pigeonhole() {
    // Pigeonhole principle: can't fit 3 pigeons in 2 holes
    // Variables: p_i_h_j = pigeon i in hole j → literal (i*3 + j)
    // Clause: each pigeon must go somewhere
    // Clause: no two pigeons in same hole

    CDCLSolver solver;

    // Pigeon 1 goes in hole 1 or 2
    solver.add_clause({1, 2});
    // Pigeon 2 goes in hole 1 or 2
    solver.add_clause({3, 4});
    // Pigeon 3 goes in hole 1 or 2
    solver.add_clause({5, 6});

    // Not both pigeon 1 and 2 in hole 1
    solver.add_clause({-1, -3});
    // Not both pigeon 1 and 2 in hole 2
    solver.add_clause({-2, -4});
    // Not both pigeon 1 and 3 in hole 1
    solver.add_clause({-1, -5});
    // Not both pigeon 1 and 3 in hole 2
    solver.add_clause({-2, -6});
    // Not both pigeon 2 and 3 in hole 1
    solver.add_clause({-3, -5});
    // Not both pigeon 2 and 3 in hole 2
    solver.add_clause({-4, -6});

    auto result = solver.solve();
    assert(!result.satisfiable);
    std::cout << "  ✓ Pigeonhole UNSAT (3 pigeons, 2 holes)\n";
}

void test_learned_clauses() {
    CDCLSolver solver;
    solver.add_clause({1, 2});
    solver.add_clause({1, -2});
    solver.add_clause({-1, 2});
    solver.add_clause({-1, -2});

    auto result = solver.solve();
    assert(!result.satisfiable);
    assert(result.conflicts > 0);
    std::cout << "  ✓ Learned clauses from conflicts (conflicts=" << result.conflicts << ")\n";
}

int main() {
    std::cout << "=== CDCL Solver Tests ===\n";
    test_simple_sat();
    test_simple_unsat();
    test_three_var();
    test_pigeonhole();
    test_learned_clauses();
    std::cout << "\nAll solver tests passed!\n";
    return 0;
}
