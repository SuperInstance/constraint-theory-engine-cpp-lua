-- flux.lua — Lua constraint API for the FLUX engine
--
-- Provides a high-level Lua interface to the C++ constraint engine.
-- Usage:
--   local flux = require("flux")
--   local battery = flux.constraint({lo=15, hi=55, name="battery_temp", severity="CRITICAL"})
--   local result = battery:check(60)

local flux_core = flux  -- Set by C++ bridge via register_flux_module

local flux = {}
flux.__index = flux

-- Severity mapping
local SEVERITY = {
    PASS     = 0,
    CAUTION  = 1,
    WARNING  = 2,
    CRITICAL = 3,
}

-- Create a new constraint object
function flux.constraint(opts)
    local c = {
        lo = opts.lo or -127,
        hi = opts.hi or 127,
        severity = SEVERITY[opts.severity] or SEVERITY.WARNING,
        name = opts.name or "unnamed",
    }
    setmetatable(c, {__index = {
        check = function(self, value)
            return flux_core.check(value, self.lo, self.hi, self.severity, self.name)
        end,
    }})
    return c
end

-- Check a single value against bounds
function flux.check(value, lo, hi)
    return flux_core.check(value, lo, hi)
end

-- Batch check values against multiple constraints
function flux.batch_check(values, constraints)
    return flux_core.batch_check(values, constraints)
end

-- CDCL SAT solve
function flux.solve(clauses)
    return flux_core.solve(clauses)
end

-- Emit LLVM IR from clauses
function flux.emit_ir(clauses)
    return flux_core.emit_ir(clauses)
end

-- Saturate a value to INT8
function flux.saturate(value)
    return flux_core.saturate(value)
end

-- Preset constraints for common industries
function flux.presets()
    return require("presets")
end

return flux
