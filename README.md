# constraint-theory-engine-cpp-lua

C++20 constraint engine with LuaJIT orchestration — includes a CDCL SAT solver, AVX-512 vectorized batch checking, and an LLVM IR emitter for JIT compilation of constraint checks.

## What It Does

This engine provides three major capabilities:

1. **Constraint checking** — INT8-saturated flat-bounds checking with DO-178C/ISO 26262 severity classification
2. **CDCL SAT solving** — Conflict-Driven Clause Learning solver for constraint satisfiability
3. **AVX-512 SIMD** — 16-value parallel batch checking with scalar fallback
4. **LuaJIT bridge** — All engine functions callable from Lua scripts
5. **LLVM IR emission** — Generates LLVM IR from solver traces for JIT compilation

## Building

### Prerequisites

- CMake ≥ 3.16, C++20 compiler (GCC 10+, Clang 12+)
- LuaJIT ≥ 2.1 (or Lua 5.3/5.4) — optional, for Lua bridge
- CPU with AVX-512 support — optional, enables SIMD path

```bash
mkdir build && cd build

# Basic build (no AVX-512, with Lua)
cmake .. -DENABLE_AVX512=OFF

# Full build (AVX-512 enabled)
cmake .. -DENABLE_AVX512=ON

# Build without Lua
cmake .. -DENABLE_LUA=OFF

make -j$(nproc)
ctest --output-on-failure
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `ENABLE_AVX512` | OFF | AVX-512 SIMD vectorization |
| `ENABLE_LUA` | ON | LuaJIT/Lua bridge |
| `ENABLE_TESTS` | ON | Test suite |
| `ENABLE_BENCHMARKS` | ON | Throughput benchmarks |

## Architecture

```
include/flux/
├── constraint.hpp    — INT8-saturated constraint types and checker
├── solver.hpp        — CDCL SAT solver
├── avx512_check.hpp  — AVX-512 vectorized batch checking
├── lua_bridge.hpp    — LuaJIT/C++ bridge
└── emitter.hpp       — LLVM IR emitter

src/
├── constraint.cpp    — Constraint implementation
├── solver.cpp        — CDCL solver implementation
├── avx512_check.cpp  — AVX-512 implementation
├── lua_bridge.cpp    — Lua bridge implementation
└── emitter.cpp       — LLVM emitter implementation

lua/
├── flux.lua          — Lua module wrapping C++ functions
├── presets.lua       — Industry preset definitions
└── examples/         — Lua usage examples (automotive, marine, battery)
```

## C++ API

### Constraint Checking

```cpp
#include <flux/constraint.hpp>

flux::Constraint temp_check(15, 55, flux::Severity::CRITICAL, "battery_temp");

auto result = flux::ConstraintChecker::check(60, temp_check);
// result.pass, result.error_mask, result.severity, result.saturated_value

// Check against multiple constraints
flux::Constraint constraints[] = {
    {15, 55, flux::Severity::CRITICAL, "temp"},
    {0, 100, flux::Severity::WARNING, "charge"},
};
auto result = flux::ConstraintChecker::check_all(60, constraints, 2);

// Batch: N values × M constraints
int32_t values[] = {20, 40, 60, 80};
auto batch = flux::ConstraintChecker::check_batch(values, 4, constraints, 2);
```

### CDCL Solver

```cpp
#include <flux/solver.hpp>

flux::CDCLSolver solver;
solver.add_clause({1, 2, -3});   // (x1 ∨ x2 ∨ ¬x3)
solver.add_clause({-1, 2});      // (¬x1 ∨ x2)
solver.add_clause({-2, 3});      // (¬x2 ∨ x3)

flux::SolverResult result = solver.solve();
// result.satisfiable, result.assignment, result.learned_clauses
// result.decisions, result.propagations, result.conflicts
```

### AVX-512 Batch Checking

```cpp
#include <flux/avx512_check.hpp>

int32_t values[16] = {...};
int32_t lo[16] = {...};
int32_t hi[16] = {...};

// Check 16 values in a single SIMD instruction
uint16_t mask = flux::AVX512Checker::check_16(values, lo, hi);
// bit i = 1 means values[i] passes
```

### LLVM IR Emission

```cpp
#include <flux/emitter.hpp>

flux::EmitterConfig config;
config.avx512 = true;
flux::LLVMEmitter emitter(config);

std::string ir = emitter.emit_module(solver.clauses());
// Produces LLVM IR with AVX-512 vectorized check functions
```

## Lua API

```lua
local flux = require("flux")

-- Single constraint check
local result = flux.check(60, 15, 55, 3, "battery_temp")
print(result.pass, result.error_mask, result.severity)

-- Batch check
local results = flux.batch_check(
    {20, 40, 60, 80},
    {{lo=15, hi=55}, {lo=0, hi=100}}
)

-- SAT solving
local solution = flux.solve({
    {1, 2, -3},
    {-1, 2},
    {-2, 3}
})
print(solution.satisfiable)

-- INT8 saturation
local saturated = flux.saturate(200)  -- returns 127
```

### Lua Presets

```lua
local presets = require("presets")

-- Automotive: ISO 26262 ASIL-D constraints
local auto = presets.automotive()
-- Aviation: DO-178C DAL A
local aero = presets.aviation()
```

## Error Mask Bits

| Bit | Name | Meaning |
|-----|------|---------|
| 0x01 | `ERR_LO` | Below lower bound |
| 0x02 | `ERR_HI` | Above upper bound |
| 0x04 | `ERR_SATURATED` | Input was INT8-saturated |
| 0x08 | `ERR_SEVERITY` | Severity threshold exceeded |

## Testing

```bash
cd build && ctest --output-on-failure
```

Test suites: constraint checking, CDCL solver, AVX-512 (with scalar fallback), Lua bridge.

## Benchmarks

```bash
./build/bench_throughput
```

Measures throughput for single-value, batch, and AVX-512 paths.

## Related Repos

- **[flux-check-py](https://github.com/SuperInstance/flux-check-py)** — Python constraint CLI
- **[flux-fracture-c](https://github.com/SuperInstance/flux-fracture-c)** — C99 fracture-coalesce library
- **[constraint-theory-rust-python](https://github.com/SuperInstance/constraint-theory-rust-python)** — Rust engine with PyO3 Python bindings
- **[polln](https://github.com/SuperInstance/polln)** — Tile-based AI system using constraint theory

## License

MIT
