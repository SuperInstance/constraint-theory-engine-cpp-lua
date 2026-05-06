// flux::constraint.cpp — Implementation (mostly header-only, this provides AVX-512)

#include "flux/constraint.hpp"
#include "flux/avx512_check.hpp"

namespace flux {

#ifdef __AVX512F__
void ConstraintChecker::check_batch_avx512(
    const int32_t* values, size_t n,
    const Constraint* constraints, size_t m,
    ConstraintResult* results
) {
    AVX512Checker::check_with_errors(values, constraints, n * m, results);
}
#endif

} // namespace flux
