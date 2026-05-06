// flux::LuaBridge — C++ ↔ LuaJIT bridge implementation
#include "flux/lua_bridge.hpp"
#include "flux/constraint.hpp"
#include "flux/solver.hpp"
#include "flux/emitter.hpp"

#ifdef USE_LUA
#include <lua.hpp>
#elif defined(USE_LUAJIT)
#include <luajit-2.1/lua.hpp>
#else
// Minimal Lua 5.4 compatible headers
#include <lua5.4/lua.hpp>
#endif

#include <vector>
#include <cstring>
#include <cstdlib>

namespace flux {

namespace lua {

static int l_check(lua_State* L) {
    int32_t value = static_cast<int32_t>(luaL_checkinteger(L, 1));
    int32_t lo    = static_cast<int32_t>(luaL_checkinteger(L, 2));
    int32_t hi    = static_cast<int32_t>(luaL_checkinteger(L, 3));
    int sev_int   = static_cast<int>(luaL_optinteger(L, 4, 2));
    const char* name = luaL_optstring(L, 5, "");

    Severity sev = static_cast<Severity>(sev_int);
    Constraint c(lo, hi, sev, name);
    ConstraintResult r = ConstraintChecker::check(value, c);

    push_result(L, r);
    return 1;
}

static int l_batch_check(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE); // values
    luaL_checktype(L, 2, LUA_TTABLE); // constraints

    // Extract values
    size_t n = lua_rawlen(L, 1);
    std::vector<int32_t> values(n);
    for (size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<lua_Integer>(i + 1));
        values[i] = static_cast<int32_t>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);
    }

    // Extract constraints
    size_t m = lua_rawlen(L, 2);
    std::vector<Constraint> constraints;
    constraints.reserve(m);
    for (size_t i = 0; i < m; ++i) {
        lua_rawgeti(L, 2, static_cast<lua_Integer>(i + 1));
        luaL_checktype(L, -1, LUA_TTABLE);

        lua_getfield(L, -1, "lo");
        int32_t lo = static_cast<int32_t>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);

        lua_getfield(L, -1, "hi");
        int32_t hi = static_cast<int32_t>(luaL_checkinteger(L, -1));
        lua_pop(L, 1);

        constraints.emplace_back(lo, hi);
        lua_pop(L, 1);
    }

    // Batch check
    BatchResult result = ConstraintChecker::check_batch(
        values.data(), values.size(),
        constraints.data(), constraints.size()
    );

    // Push result table
    lua_newtable(L);
    lua_pushinteger(L, static_cast<lua_Integer>(result.total));
    lua_setfield(L, -2, "total");
    lua_pushinteger(L, static_cast<lua_Integer>(result.passed));
    lua_setfield(L, -2, "passed");
    lua_pushinteger(L, static_cast<lua_Integer>(result.failed));
    lua_setfield(L, -2, "failed");

    // Results array
    lua_newtable(L);
    for (size_t i = 0; i < result.results.size(); ++i) {
        push_result(L, result.results[i]);
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    lua_setfield(L, -2, "results");

    return 1;
}

static int l_solve(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    size_t n = lua_rawlen(L, 1);
    CDCLSolver solver;

    for (size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<lua_Integer>(i + 1));
        luaL_checktype(L, -1, LUA_TTABLE);

        Clause clause;
        size_t clause_len = lua_rawlen(L, -1);
        for (size_t j = 0; j < clause_len; ++j) {
            lua_rawgeti(L, -1, static_cast<lua_Integer>(j + 1));
            clause.push_back(static_cast<Literal>(luaL_checkinteger(L, -1)));
            lua_pop(L, 1);
        }
        solver.add_clause(clause);
        lua_pop(L, 1);
    }

    SolverResult result = solver.solve();

    // Push result
    lua_newtable(L);
    lua_pushboolean(L, result.satisfiable);
    lua_setfield(L, -2, "satisfiable");

    // Assignment
    lua_newtable(L);
    for (const auto& [var, val] : result.assignment) {
        lua_pushboolean(L, val);
        lua_rawseti(L, -2, static_cast<lua_Integer>(var));
    }
    lua_setfield(L, -2, "assignment");

    // Learned clauses
    lua_newtable(L);
    for (size_t i = 0; i < result.learned_clauses.size(); ++i) {
        lua_newtable(L);
        for (size_t j = 0; j < result.learned_clauses[i].size(); ++j) {
            lua_pushinteger(L, static_cast<lua_Integer>(result.learned_clauses[i][j]));
            lua_rawseti(L, -2, static_cast<lua_Integer>(j + 1));
        }
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
    lua_setfield(L, -2, "learned");

    return 1;
}

static int l_emit_ir(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);

    std::vector<Clause> clauses;
    size_t n = lua_rawlen(L, 1);

    for (size_t i = 0; i < n; ++i) {
        lua_rawgeti(L, 1, static_cast<lua_Integer>(i + 1));
        luaL_checktype(L, -1, LUA_TTABLE);

        Clause clause;
        size_t clause_len = lua_rawlen(L, -1);
        for (size_t j = 0; j < clause_len; ++j) {
            lua_rawgeti(L, -1, static_cast<lua_Integer>(j + 1));
            clause.push_back(static_cast<Literal>(luaL_checkinteger(L, -1)));
            lua_pop(L, 1);
        }
        clauses.push_back(std::move(clause));
        lua_pop(L, 1);
    }

    LLVMEmitter emitter;
    std::string ir = emitter.emit_module(clauses);
    lua_pushstring(L, ir.c_str());
    return 1;
}

static int l_saturate(lua_State* L) {
    int32_t val = static_cast<int32_t>(luaL_checkinteger(L, 1));
    lua_pushinteger(L, static_cast<lua_Integer>(sat8(val)));
    return 1;
}

} // namespace lua

void push_result(lua_State* L, const ConstraintResult& r) {
    lua_newtable(L);
    lua_pushboolean(L, r.pass);
    lua_setfield(L, -2, "pass");
    lua_pushinteger(L, static_cast<lua_Integer>(r.error_mask));
    lua_setfield(L, -2, "error_mask");
    lua_pushinteger(L, static_cast<lua_Integer>(static_cast<uint8_t>(r.severity)));
    lua_setfield(L, -2, "severity");
    lua_pushinteger(L, static_cast<lua_Integer>(r.saturated_value));
    lua_setfield(L, -2, "saturated_value");
}

void push_batch_results(lua_State* L, const ConstraintResult* results, size_t n) {
    lua_newtable(L);
    for (size_t i = 0; i < n; ++i) {
        push_result(L, results[i]);
        lua_rawseti(L, -2, static_cast<lua_Integer>(i + 1));
    }
}

static const luaL_Reg flux_lib[] = {
    {"check",       lua::l_check},
    {"batch_check", lua::l_batch_check},
    {"solve",       lua::l_solve},
    {"emit_ir",     lua::l_emit_ir},
    {"saturate",    lua::l_saturate},
    {nullptr, nullptr}
};

void register_flux_module(lua_State* L) {
    luaL_newlib(L, flux_lib);
    lua_setglobal(L, "flux");
}

} // namespace flux
