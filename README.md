# constraint-theory-engine-cpp-lua

High-performance C++17 constraint engine with LuaJIT scripting — AVX-512 batch checking, CDCL solver, and LLVM IR emission for constraint programs.

## What This Gives You

- **AVX-512 batch checking** — evaluate 16 constraints simultaneously with SIMD intrinsics
- **CDCL solver** — conflict-driven clause learning for Boolean constraint satisfaction
- **LLVM IR emitter** — compile constraint programs to optimized machine code
- **LuaJIT bridge** — script constraint pipelines from Lua with near-C performance
- **83+ constraint primitives** — snap, funnel, Laman, consensus, holonomy, and more

## Quick Start

### C++ Engine

```cpp
#include <constraint_engine/engine.hpp>

int main() {
    constraint_engine::Engine engine;
    
    // Add constraints
    engine.add_lattice_snap("x", "y", LatticeType::EisensteinA2);
    engine.add_funnel("x", 0.1, 0.001);  // decay=0.1, tolerance=0.001
    engine.add_holonomy_check("cycle_1");
    
    // Batch evaluate with AVX-512
    auto result = engine.evaluate_batch(points);
    std::cout << "Passed: " << result.passed << "/" << result.total << std::endl;
}
```

### LuaJIT Scripting

```lua
local engine = require("constraint_engine")

-- Build constraint pipeline
engine.snap("eisenstein_a2", points)
engine.funnel(0.1, 0.001)
engine.verify("holonomy")

-- Run and inspect
local result = engine.run()
print(string.format("Constraints satisfied: %d/%d", result.passed, result.total))
```

### LLVM IR Emission

```cpp
// Compile constraints to LLVM IR for maximum performance
auto ir = engine.emit_llvm();
ir.optimize(2);  // -O2
ir.compile_and_execute();
```

## Architecture

```
┌─────────────────────────────────────────┐
│            LuaJIT Bridge                │
│   (script constraint pipelines)         │
├─────────────────────────────────────────┤
│         Constraint Engine               │
│  ┌──────────┐ ┌──────────┐ ┌─────────┐ │
│  │  CDCL    │ │  Batch   │ │  LLVM   │ │
│  │  Solver  │ │  AVX-512 │ │  Emitter│ │
│  └──────────┘ └──────────┘ └─────────┘ │
├─────────────────────────────────────────┤
│       Constraint Primitives             │
│  snap · funnel · laman · holonomy       │
└─────────────────────────────────────────┘
```

## Building

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run tests
./constraint_engine_tests
```

### Dependencies

- C++17 compiler (GCC 9+, Clang 10+, MSVC 19.28+)
- LuaJIT 2.1+
- LLVM 14+ (optional, for IR emission)

## How It Fits

The **high-performance engine** of the constraint theory ecosystem:

- [constraint-theory-core](https://github.com/SuperInstance/constraint-theory-core) — Python/Rust reference implementation
- [constraint-dialect](https://github.com/SuperInstance/constraint-dialect) — MLIR dialect for compiler integration
- [constraint-theory-rust-python](https://github.com/SuperInstance/constraint-theory-rust-python) — Rust alternative engine
- [constraint-dsl](https://github.com/SuperInstance/constraint-dsl) — declarative pipeline language

## Testing

```bash
cd build
ctest --output-on-failure
```

## Installation

```bash
# From source
git clone https://github.com/SuperInstance/constraint-theory-engine-cpp-lua.git
cd constraint-theory-engine-cpp-lua
mkdir build && cd build && cmake .. && make -j$(nproc)
```

## License

MIT
