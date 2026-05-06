cmake_minimum_required(VERSION 3.16)
project(flux-engine VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Options ──────────────────────────────────────────────────────────────────
option(ENABLE_AVX512 "Enable AVX-512 vectorized checking" OFF)
option(ENABLE_LUA "Enable Lua bridge (requires Lua/LuaJIT)" ON)
option(ENABLE_TESTS "Build test suite" ON)
option(ENABLE_BENCHMARKS "Build throughput benchmarks" ON)

# ── AVX-512 ──────────────────────────────────────────────────────────────────
if(ENABLE_AVX512)
    include(CheckCXXCompilerFlag)
    check_cxx_compiler_flag("-mavx512f" COMPILER_SUPPORTS_AVX512)
    if(COMPILER_SUPPORTS_AVX512)
        add_compile_definitions(__AVX512F__)
        add_compile_options(-mavx512f)
        message(STATUS "AVX-512: ENABLED")
    else()
        message(WARNING "Compiler does not support AVX-512, falling back to scalar")
        set(ENABLE_AVX512 OFF)
    endif()
else()
    message(STATUS "AVX-512: DISABLED (scalar fallback)")
endif()

# ── Find Lua ─────────────────────────────────────────────────────────────────
if(ENABLE_LUA)
    # Try LuaJIT first, then Lua 5.4, then Lua 5.3
    find_package(PkgConfig)

    if(PkgConfig_FOUND)
        pkg_check_modules(LUAJIT luajit>=2.1)
    endif()

    if(LUAJIT_FOUND)
        message(STATUS "Lua: LuaJIT ${LUAJIT_VERSION}")
        set(LUA_INCLUDE_DIRS ${LUAJIT_INCLUDE_DIRS})
        set(LUA_LIBRARIES ${LUAJIT_LIBRARIES})
        set(LUA_DEFINITIONS USE_LUAJIT)
    elseif(PkgConfig_FOUND)
        pkg_check_modules(LUA54 lua5.4>=5.4)
        if(LUA54_FOUND)
            message(STATUS "Lua: Lua ${LUA54_VERSION}")
            set(LUA_INCLUDE_DIRS ${LUA54_INCLUDE_DIRS})
            set(LUA_LIBRARIES ${LUA54_LIBRARIES})
            set(LUA_DEFINITIONS USE_LUA)
        else()
            pkg_check_modules(LUA53 lua5.3>=5.3)
            if(LUA53_FOUND)
                message(STATUS "Lua: Lua ${LUA53_VERSION}")
                set(LUA_INCLUDE_DIRS ${LUA53_INCLUDE_DIRS})
                set(LUA_LIBRARIES ${LUA53_LIBRARIES})
                set(LUA_DEFINITIONS USE_LUA)
            else()
                message(STATUS "Lua: NOT FOUND — bridge disabled")
                set(ENABLE_LUA OFF)
            endif()
        endif()
    else()
        message(STATUS "Lua: NOT FOUND — bridge disabled")
        set(ENABLE_LUA OFF)
    endif()
endif()

# ── Library: flux_engine ─────────────────────────────────────────────────────
set(FLUX_SOURCES
    src/constraint.cpp
    src/solver.cpp
    src/emitter.cpp
    src/avx512_check.cpp
)

if(ENABLE_LUA)
    list(APPEND FLUX_SOURCES src/lua_bridge.cpp)
endif()

add_library(flux_engine STATIC ${FLUX_SOURCES})
target_include_directories(flux_engine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

if(ENABLE_LUA)
    target_include_directories(flux_engine PRIVATE ${LUA_INCLUDE_DIRS})
    target_compile_definitions(flux_engine PRIVATE ${LUA_DEFINITIONS})
    target_link_libraries(flux_engine PRIVATE ${LUA_LIBRARIES})
endif()

# ── Executable: flux_demo ────────────────────────────────────────────────────
add_executable(flux_demo examples/demo.cpp)
target_link_libraries(flux_demo PRIVATE flux_engine)

# ── Tests ────────────────────────────────────────────────────────────────────
if(ENABLE_TESTS)
    enable_testing()

    add_executable(test_constraint tests/test_constraint.cpp)
    target_link_libraries(test_constraint PRIVATE flux_engine)
    add_test(NAME constraint COMMAND test_constraint)

    add_executable(test_solver tests/test_solver.cpp)
    target_link_libraries(test_solver PRIVATE flux_engine)
    add_test(NAME solver COMMAND test_solver)

    add_executable(test_avx512 tests/test_avx512.cpp)
    target_link_libraries(test_avx512 PRIVATE flux_engine)
    add_test(NAME avx512 COMMAND test_avx512)

    if(ENABLE_LUA)
        add_executable(test_lua_bridge tests/test_lua_bridge.cpp)
        target_link_libraries(test_lua_bridge PRIVATE flux_engine)
        target_include_directories(test_lua_bridge PRIVATE ${LUA_INCLUDE_DIRS})
        target_compile_definitions(test_lua_bridge PRIVATE ${LUA_DEFINITIONS})
        target_link_libraries(test_lua_bridge PRIVATE ${LUA_LIBRARIES})
        add_test(NAME lua_bridge COMMAND test_lua_bridge)
    endif()
endif()

# ── Benchmarks ───────────────────────────────────────────────────────────────
if(ENABLE_BENCHMARKS)
    add_executable(bench_throughput benches/bench_throughput.cpp)
    target_link_libraries(bench_throughput PRIVATE flux_engine)
endif()

# ── Install ──────────────────────────────────────────────────────────────────
install(TARGETS flux_engine flux_demo
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)
install(DIRECTORY include/flux DESTINATION include)
install(DIRECTORY lua/ DESTINATION share/flux/lua)
