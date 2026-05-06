// flux::emitter.cpp — LLVM IR emitter implementation
#include "flux/emitter.hpp"

namespace flux {
// Emitter is fully inline in header for compile-time string generation.
// This TU provides compilation check.
static_assert(sizeof(LLVMEmitter) > 0, "LLVMEmitter must be instantiable");
} // namespace flux
