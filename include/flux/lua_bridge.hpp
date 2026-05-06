#pragma once
/// flux::LuaBridge — Bridge between C++ constraint engine and LuaJIT
///
/// Registers C++ constraint functions in a Lua state.
/// Exposes: flux.check(), flux.batch_check(), flux.solve(), flux.constraint()

struct lua_State;

namespace flux {

class Constraint;
class ConstraintChecker;
class CDCLSolver;
struct ConstraintResult;

/// Initialize the flux module in a Lua state
/// Call this after creating/obtaining the Lua state.
void register_flux_module(lua_State* L);

/// Push a ConstraintResult onto the Lua stack as a table
void push_result(lua_State* L, const ConstraintResult& r);

/// Push a table of results from batch checking
void push_batch_results(lua_State* L, const ConstraintResult* results, size_t n);

namespace lua {

/// Lua: flux.check(value, lo, hi [, severity [, name]])
/// Returns: { pass=bool, error_mask=int, severity=int }
int l_check(lua_State* L);

/// Lua: flux.batch_check(values_table, constraints_table)
/// values_table: { v1, v2, ... }
/// constraints_table: { {lo=x, hi=y}, ... }
/// Returns: { total=int, passed=int, failed=int, results={...} }
int l_batch_check(lua_State* L);

/// Lua: flux.solve(clauses_table)
/// clauses_table: { {1, 2, -3}, {-1, 2}, ... }
/// Returns: { satisfiable=bool, assignment={1=true,2=false,...}, learned={...} }
int l_solve(lua_State* L);

/// Lua: flux.emit_ir(clauses_table)
/// Returns: string of LLVM IR
int l_emit_ir(lua_State* L);

/// Lua: flux.saturate(value)
/// Returns: saturated INT8 value
int l_saturate(lua_State* L);

} // namespace lua
} // namespace flux
