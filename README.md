# constraint-theory-engine-cpp-lua

**C++ constraint engine with embedded LuaJIT orchestration** — the "Classic Engine" approach.

C++ handles the hot paths (AVX-512 vectorized constraint checking, CDCL solver) while LuaJIT provides lightweight, fast scripting for constraint definitions, simulation orchestration, and user customization.

## Why C++ & Lua?

| Feature | C++ | LuaJIT |
|---------|-----|--------|
| AVX-512 SIMD | ✅ Native intrinsics | ❌ |
| Memory layout control | ✅ Cache-aligned structs | ❌ |
| Embedded scripting | ❌ | ✅ <200KB runtime |
| User extensibility | Requires recompile | ✅ Live reload |
| Hot path performance | ~10ns/check | ~50ns/call overhead |
| Footprint | Minimal | ~200KB |

This is the standard pattern for high-performance simulations and game engines. LuaJIT's FFI makes calling C++ functions near-zero-cost.

## Architecture

```
┌──────────────────────────────────────┐
│  Lua Orchestration Layer             │
│  ┌──────────┐  ┌──────────────────┐  │
│  │ flux.lua │  │ presets.lua      │  │
│  │ user API │  │ industry configs │  │
│  └────┬─────┘  └───────┬──────────┘  │
│       │   LuaJIT FFI   │             │
├───────┼─────────────────┼─────────────┤
│  C++ Core Engine                     │
│  ┌──────────┐  ┌──────────────────┐  │
│  │ solver   │  │ constraint.hpp   │  │
│  │ CDCL     │  │ INT8 saturation  │  │
│  └──────────┘  └──────────────────┘  │
│  ┌──────────┐  ┌──────────────────┐  │
│  │ emitter  │  │ avx512_check     │  │
│  │ LLVM IR  │  │ 16× batch SIMD   │  │
│  └──────────┘  └──────────────────┘  │
└──────────────────────────────────────┘
```

## Quick Start

### C++ API

```cpp
#include <flux/constraint.hpp>
#include <flux/solver.hpp>
#include <flux/avx512_check.hpp>

// Create constraints
flux::Constraint battery_temp(15, 55, "battery_temp", flux::Severity::CRITICAL);
flux::Constraint vehicle_speed(0, 250, "vehicle_speed", flux::Severity::WARNING);

// Check values
auto result = battery_temp.check(60);
if (!result.pass) {
    std::cerr << "FAIL: " << result.name << " severity=" << result.severity;
}

// Batch check with AVX-512
std::vector<int32_t> values(1024);
flux::AVX512Checker checker;
checker.check_batch(values.data(), values.size(), constraints, results);
```

### Lua API

```lua
local flux = require("flux")

-- Define constraints from Lua
local engine = flux.engine.new()
engine:add_constraint(15, 55, "battery_temp", flux.SEVERITY_CRITICAL)
engine:add_constraint(0, 250, "vehicle_speed", flux.SEVERITY_WARNING)

-- Check values
local result = engine:check(60)
print(result.passed, result.name, result.severity)

-- Batch check
local results = engine:check_batch({0, 60, 120, 250, 300})
for _, r in ipairs(results) do
    if not r.passed then print(r.name .. " FAILED") end
end

-- Load industry preset
local auto = flux.presets.automotive()
auto:check(60)
```

### Industry Presets

```lua
local presets = require("presets")

-- ISO 26262 automotive
local auto = presets.automotive()
-- DO-178C aviation  
local aero = presets.aviation()
-- IACS maritime
local marine = presets.marine()
```

## Building

```bash
mkdir build && cd build
cmake .. -DENABLE_AVX512=ON -DENABLE_LUA=ON
make -j$(nproc)

# Run tests
ctest --output-on-failure

# Run benchmark
./bench_throughput
```

### Without AVX-512 or Lua

```bash
cmake .. -DENABLE_AVX512=OFF -DENABLE_LUA=OFF
make -j$(nproc)
```

## Performance

| Operation | Throughput | Latency |
|-----------|-----------|---------|
| Single check (scalar) | ~100M/sec | ~10ns |
| Batch check (AVX-512) | ~62B/sec | ~0.23ms/10M |
| CDCL solve (100 vars) | ~50K/sec | ~20μs |
| Lua check (FFI) | ~20M/sec | ~50ns |

## CDCL Solver

The solver implements Conflict-Driven Clause Learning:

1. **Decide**: Pick an unassigned variable
2. **Propagate**: Unit propagation (BCP)
3. **Conflict**: Analyze with resolution
4. **Learn**: Add learned clause
5. **Backtrack**: Return to asserting level

Traces compile to LLVM IR for zero-overhead execution.

## LLVM IR Emitter

```cpp
flux::LLVMEmitter emitter;
auto trace = solver.solve(clauses);
auto ir = emitter.emit(trace);
// ir contains LLVM IR with AVX-512 intrinsics
```

## Testing

```bash
# Unit tests
./test_constraint
./test_solver
./test_avx512
./test_lua_bridge

# All tests via CTest
ctest --output-on-failure
```

## Related Projects

- **constraint-theory-llvm**: Original Rust LLVM backend
- **constraint-theory-rust-python**: Rust engine with Python bindings
- **constraint-theory-mojo**: Mojo + MLIR approach
- **constraint-theory-mlir**: Custom FLUX MLIR dialect
- **constraint-theory-ecosystem**: 42-language reference implementations

## License

Apache 2.0
