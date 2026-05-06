-- examples/battery.lua — Battery temperature constraint example
--
-- Demonstrates constraint checking for a lithium-ion battery pack.
-- The BMS must keep cell temperature between 15°C and 55°C.

local flux = require("flux")

print("=== Battery Temperature Constraint Example ===\n")

-- Define the battery temperature constraint (CRITICAL severity)
local battery = flux.constraint({
    lo = 15,
    hi = 55,
    name = "battery_temp",
    severity = "CRITICAL"
})

-- Test cases
local test_temps = {
    {value = 10,  desc = "Too cold (10°C)"},
    {value = 15,  desc = "Lower bound (15°C)"},
    {value = 25,  desc = "Normal (25°C)"},
    {value = 55,  desc = "Upper bound (55°C)"},
    {value = 60,  desc = "Overheating (60°C)"},
    {value = 128, desc = "Saturated INT8 (128→127)"},
}

print("Single constraint checks:")
for _, test in ipairs(test_temps) do
    local result = battery:check(test.value)
    local status = result.pass and "PASS ✓" or "FAIL ✗"
    print(string.format("  %s: %s (mask=0x%02X, sev=%d)",
        test.desc, status, result.error_mask, result.severity))
end

-- Batch check
print("\nBatch check (5 values × 1 constraint):")
local values = {10, 25, 40, 55, 60}
local constraints = {{lo=15, hi=55}}

local result = flux.batch_check(values, constraints)
print(string.format("  Total: %d, Passed: %d, Failed: %d",
    result.total, result.passed, result.failed))

for i, r in ipairs(result.results) do
    print(string.format("  Value %d: %s", values[i], r.pass and "PASS" or "FAIL"))
end
