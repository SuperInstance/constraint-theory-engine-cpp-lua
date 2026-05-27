-- test_lua_flux.lua — Tests for flux.lua module
-- Run with: lua test_lua_flux.lua

-- Since flux.lua depends on the C++ bridge (flux_core), we mock it for unit testing
local flux_core = {
    _results = {},
    _sat_val = 0,

    check = function(value, lo, hi, severity, name)
        local r = flux_core._results or {}
        return r[value] or {pass = value >= lo and value <= hi, error_mask = 0, severity = severity or 2, saturated_value = value}
    end,

    batch_check = function(values, constraints)
        local passed = 0
        local failed = 0
        local results = {}
        for i, v in ipairs(values) do
            local ok = true
            for _, c in ipairs(constraints) do
                if v < c.lo or v > c.hi then ok = false; break end
            end
            results[i] = {pass = ok, error_mask = ok and 0 or 1, severity = 2, saturated_value = v}
            if ok then passed = passed + 1 else failed = failed + 1 end
        end
        return {total = #values, passed = passed, failed = failed, results = results}
    end,

    solve = function(clauses)
        return {satisfiable = true, assignment = {[1] = true}, learned = {}}
    end,

    emit_ir = function(clauses)
        return "; mock IR output"
    end,

    saturate = function(val)
        if val < -127 then return -127 end
        if val > 127 then return 127 end
        return val
    end,
}

-- Set the global that flux.lua expects
flux = flux_core

-- Load flux.lua
local flux_mod = dofile("lua/flux.lua")

local passed = 0
local failed = 0

local function check(name, condition)
    if condition then
        print("  ✓ " .. name)
        passed = passed + 1
    else
        print("  ✗ " .. name .. " FAILED")
        failed = failed + 1
    end
end

-- Test flux.saturate
print("=== flux.saturate ===")
check("saturate(0) = 0", flux_mod.saturate(0) == 0)
check("saturate(127) = 127", flux_mod.saturate(127) == 127)
check("saturate(-127) = -127", flux_mod.saturate(-127) == -127)
check("saturate(200) = 127", flux_mod.saturate(200) == 127)
check("saturate(-200) = -127", flux_mod.saturate(-200) == -127)
check("saturate(128) = 127", flux_mod.saturate(128) == 127)
check("saturate(-128) = -127", flux_mod.saturate(-128) == -127)

-- Test flux.check
print("\n=== flux.check ===")
local r = flux_mod.check(50, 0, 100)
check("check(50, 0, 100) passes", r.pass == true)

local r2 = flux_mod.check(150, 0, 100)
check("check(150, 0, 100) fails", r2.pass == false)

-- Test flux.batch_check
print("\n=== flux.batch_check ===")
local br = flux_mod.batch_check({10, 50, 90}, {{lo = 0, hi = 100}})
check("batch_check 3 values all pass", br.passed == 3)
check("batch_check 3 values none fail", br.failed == 0)
check("batch_check total = 3", br.total == 3)

local br2 = flux_mod.batch_check({-5, 50, 150}, {{lo = 0, hi = 100}})
check("batch_check mixed: passed=1", br2.passed == 1)
check("batch_check mixed: failed=2", br2.failed == 2)

-- Test flux.solve
print("\n=== flux.solve ===")
local sol = flux_mod.solve({{1, 2, -3}, {-1, 2}})
check("solve returns satisfiable", sol.satisfiable == true)
check("solve returns assignment", sol.assignment ~= nil)

-- Test flux.emit_ir
print("\n=== flux.emit_ir ===")
local ir = flux_mod.emit_ir({{1, 2}, {-1, 3}})
check("emit_ir returns string", type(ir) == "string")
check("emit_ir non-empty", #ir > 0)

-- Test flux.constraint object
print("\n=== flux.constraint ===")
local c = flux_mod.constraint({lo = 15, hi = 55, name = "battery_temp", severity = "CRITICAL"})
check("constraint object has lo", c.lo == 15)
check("constraint object has hi", c.hi == 55)
check("constraint object has name", c.name == "battery_temp")
check("constraint object has severity", c.severity == 3) -- CRITICAL = 3

local cr = c:check(25)
check("constraint:check(25) passes", cr.pass == true)

local cr2 = c:check(60)
check("constraint:check(60) fails", cr2.pass == false)

-- Test defaults
local c2 = flux_mod.constraint({lo = 0, hi = 100})
check("constraint default name", c2.name == "unnamed")
check("constraint default severity (WARNING=2)", c2.severity == 2)

local c3 = flux_mod.constraint({})
check("constraint default lo=-127", c3.lo == -127)
check("constraint default hi=127", c3.hi == 127)

-- Test presets
print("\n=== presets ===")
local presets = dofile("lua/presets.lua")
check("presets has automotive", presets.automotive ~= nil)
check("presets has maritime", presets.maritime ~= nil)
check("presets has aviation", presets.aviation ~= nil)
check("presets has medical", presets.medical ~= nil)
check("presets has nuclear", presets.nuclear ~= nil)
check("presets has energy", presets.energy ~= nil)

-- Validate automotive preset structure
local auto = presets.automotive
check("automotive.vehicle_speed exists", auto.vehicle_speed ~= nil)
check("automotive.vehicle_speed has lo", auto.vehicle_speed.lo ~= nil)
check("automotive.vehicle_speed has hi", auto.vehicle_speed.hi ~= nil)
check("automotive.vehicle_speed has severity", auto.vehicle_speed.severity ~= nil)
check("automotive.vehicle_speed has name", auto.vehicle_speed.name ~= nil)

-- Validate maritime
local marine = presets.maritime
check("maritime.engine_coolant exists", marine.engine_coolant ~= nil)
check("maritime.rudder_angle lo=-35", marine.rudder_angle.lo == -35)
check("maritime.rudder_angle hi=35", marine.rudder_angle.hi == 35)

-- Validate aviation
local aero = presets.aviation
check("aviation.altitude lo=-1000", aero.altitude.lo == -1000)
check("aviation.altitude hi=45000", aero.altitude.hi == 45000)

-- Validate medical
local med = presets.medical
check("medical.heart_rate hi=220", med.heart_rate.hi == 220)

-- Validate nuclear
local nuc = presets.nuclear
check("nuclear.coolant_temp lo=260", nuc.coolant_temp.lo == 260)

-- Validate energy
local en = presets.energy
check("energy.frequency lo=49", en.frequency.lo == 49)
check("energy.frequency hi=51", en.frequency.hi == 51)

print(string.format("\n%d passed, %d failed", passed, failed))
if failed > 0 then os.exit(1) end
