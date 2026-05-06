// test_lua_bridge.cpp — Lua bridge tests (requires Lua installed)
#include <iostream>
#include <cstring>

// Include Lua headers
#if __has_include(<lua.hpp>)
#include <lua.hpp>
#elif __has_include(<lua5.4/lua.hpp>)
#include <lua5.4/lua.hpp>
#elif __has_include(<luajit-2.1/lua.hpp>)
#include <luajit-2.1/lua.hpp>
#else
// No Lua available — compile a stub test
#warning "No Lua headers found, compiling stub test"

int main() {
    std::cout << "=== Lua Bridge Tests ===\n";
    std::cout << "  ⚠ Lua headers not found — skipping bridge tests\n";
    std::cout << "  Install: apt install liblua5.4-dev or libluajit-5.1-dev\n";
    std::cout << "\nStub test passed (no Lua)\n";
    return 0;
}

// Skip the rest
#define LUA_BRIDGE_TEST_SKIP
#endif

#ifndef LUA_BRIDGE_TEST_SKIP
#include "../include/flux/lua_bridge.hpp"

void test_register_module() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    flux::register_flux_module(L);

    // Verify flux table exists
    lua_getglobal(L, "flux");
    assert(lua_istable(L, -1));

    // Verify functions exist
    lua_getfield(L, -1, "check");
    assert(lua_isfunction(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, -1, "batch_check");
    assert(lua_isfunction(L, -1));
    lua_pop(L, 1);

    lua_getfield(L, -1, "solve");
    assert(lua_isfunction(L, -1));
    lua_pop(L, 1);

    lua_close(L);
    std::cout << "  ✓ Module registration\n";
}

void test_check_via_lua() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    flux::register_flux_module(L);

    // flux.check(25, 10, 50) should pass
    const char* script = R"(
        local result = flux.check(25, 10, 50)
        return result.pass
    )";

    int ok = luaL_dostring(L, script);
    assert(ok == LUA_OK);
    assert(lua_toboolean(L, -1) == 1);

    lua_close(L);
    std::cout << "  ✓ Check via Lua (pass)\n";
}

void test_check_fail_via_lua() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    flux::register_flux_module(L);

    const char* script = R"(
        local result = flux.check(5, 10, 50)
        return result.pass
    )";

    int ok = luaL_dostring(L, script);
    assert(ok == LUA_OK);
    assert(lua_toboolean(L, -1) == 0);

    lua_close(L);
    std::cout << "  ✓ Check via Lua (fail)\n";
}

void test_saturate_via_lua() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    flux::register_flux_module(L);

    const char* script = R"(
        return flux.saturate(200)
    )";

    int ok = luaL_dostring(L, script);
    assert(ok == LUA_OK);
    assert(lua_tointeger(L, -1) == 127);

    lua_close(L);
    std::cout << "  ✓ Saturate via Lua (200→127)\n";
}

int main() {
    std::cout << "=== Lua Bridge Tests ===\n";
    test_register_module();
    test_check_via_lua();
    test_check_fail_via_lua();
    test_saturate_via_lua();
    std::cout << "\nAll Lua bridge tests passed!\n";
    return 0;
}
#endif
