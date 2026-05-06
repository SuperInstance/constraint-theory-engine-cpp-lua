-- examples/marine.lua — Maritime constraint example
--
-- Demonstrates maritime engine room monitoring with IACS constraints.

local flux = require("flux")
local presets = flux.presets()

print("=== Maritime Engine Room Monitoring ===\n")

local marine = presets.maritime

-- Simulate engine room sensor feed
local sensor_readings = {
    engine_coolant_temp = 78,
    exhaust_gas_temp    = 320,
    fuel_pressure       = 3.5,
    bilge_level_cm      = 12,
    rudder_angle        = 15,
    wind_speed_knots    = 35,
}

print("Current readings:")
for name, spec in pairs(marine) do
    local val = sensor_readings[name] or 0
    local result = flux.check(val, spec.lo, spec.hi)
    local bar = result.pass and "█" or "▓ FAIL"
    local pct = (val - spec.lo) / (spec.hi - spec.lo) * 100
    print(string.format("  %-20s: %6.1f [%4d,%4d] %3d%% %s",
        name, val, spec.lo, spec.hi, pct, bar))
end

-- Alarm cascade: if coolant temp high, check related systems
print("\nCascade check (coolant temp elevated):")
local coolant = flux.check(92, marine.engine_coolant_temp.lo, marine.engine_coolant_temp.hi)
if not coolant.pass then
    print("  ⚠ COOLANT HIGH — checking exhaust temp...")
    local exhaust = flux.check(540, marine.exhaust_gas_temp.lo, marine.exhaust_gas_temp.hi)
    if not exhaust.pass then
        print("  🚨 EXHAUST ALSO HIGH — ENGINE SHUTDOWN RECOMMENDED")
    end
end
