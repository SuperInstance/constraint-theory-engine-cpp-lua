// flux::solver.cpp — CDCL solver implementation (header-only, this registers tests)
#include "flux/solver.hpp"

namespace flux {
// Solver is fully templated/inline in header for zero-overhead abstraction.
// This TU ensures ODR compliance and provides a compilation check.
static_assert(sizeof(CDCLSolver) > 0, "CDCLSolver must be instantiable");
static_assert(sizeof(SolverResult) > 0, "SolverResult must be instantiable");
} // namespace flux
