-- examples/automotive.lua — ISO 26262 automotive constraint example
--
-- Demonstrates multi-constraint checking for an automotive system
-- using ISO 26262 ASIL-D severity levels.

local flux = require("flux")

print("=== Automotive ISO 26262 Constraint Example ===\n")

-- Load automotive presets
local presets = flux.presets()
local auto = presets.automotive

-- Create constraint objects
local constraints = {}
local names = {}
for name, spec in pairs(auto) do
    table.insert(constraints, {lo=spec.lo, hi=spec.hi})
    table.insert(names, spec.name)
end

-- Simulate sensor readings at various states
local scenarios = {
    {
        desc = "Highway cruise (normal)",
        values = {vehicle_speed=120, engine_rpm=3000, brake_pressure=5,
                  steering_angle=0, battery_voltage=13.8, cabin_temp=22}
    },
    {
        desc = "Emergency braking",
        values = {vehicle_speed=180, engine_rpm=7500, brake_pressure=170,
                  steering_angle=-30, battery_voltage=12.1, cabin_temp=35}
    },
    {
        desc = "Extreme cold start",
        values = {vehicle_speed=0, engine_rpm=1200, brake_pressure=0,
                  steering_angle=0, battery_voltage=8.5, cabin_temp=-45}
    },
}

for _, scenario in ipairs(scenarios) do
    print("Scenario: " .. scenario.desc)
    for name, spec in pairs(auto) do
        local val = scenario.values[name]
        if val then
            local result = flux.check(val, spec.lo, spec.hi)
            local status = result.pass and "PASS" or "FAIL"
            print(string.format("  %-20s: %4d [%3d,%3d] → %s",
                name, val, spec.lo, spec.hi, status))
        end
    end
    print()
end

-- CDCL solve example
print("CDCL Solver Example:")
print("  Solving: (x1 ∨ x2 ∨ ¬x3) ∧ (¬x1 ∨ x2) ∧ (¬x2 ∨ x3)\n")

local solution = flux.solve({
    {1, 2, -3},
    {-1, 2},
    {-2, 3},
})

print("  Satisfiable: " .. tostring(solution.satisfiable))
if solution.satisfiable then
    print("  Assignment:")
    for var, val in pairs(solution.assignment) do
        print(string.format("    x%d = %s", var, tostring(val)))
    end
end
