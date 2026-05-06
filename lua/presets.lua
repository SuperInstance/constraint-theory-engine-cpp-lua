-- presets.lua — Industry constraint presets
--
-- Pre-defined constraint sets for automotive, marine, aviation,
-- medical, nuclear, and energy industries.

local presets = {}

-- Automotive (ISO 26262 ASIL-D)
presets.automotive = {
    vehicle_speed    = {lo=0,   hi=250,  severity="CRITICAL", name="vehicle_speed"},
    engine_rpm       = {lo=0,   hi=8000, severity="WARNING",  name="engine_rpm"},
    brake_pressure   = {lo=0,   hi=180,  severity="CRITICAL", name="brake_pressure"},
    steering_angle   = {lo=-540,hi=540,  severity="CRITICAL", name="steering_angle"},
    battery_voltage  = {lo=9,   hi=16,   severity="WARNING",  name="battery_voltage"},
    cabin_temp       = {lo=-40, hi=85,   severity="CAUTION",  name="cabin_temp"},
}

-- Maritime (IACS / SOLAS)
presets.maritime = {
    engine_coolant   = {lo=60,  hi=95,   severity="CRITICAL", name="engine_coolant_temp"},
    exhaust_temp     = {lo=100, hi=550,  severity="WARNING",  name="exhaust_gas_temp"},
    fuel_pressure    = {lo=0,   hi=7,    severity="CRITICAL", name="fuel_pressure"},
    bilge_level      = {lo=0,   hi=50,   severity="CRITICAL", name="bilge_level_cm"},
    rudder_angle     = {lo=-35, hi=35,   severity="WARNING",  name="rudder_angle"},
    wind_speed       = {lo=0,   hi=100,  severity="CAUTION",  name="wind_speed_knots"},
}

-- Aviation (DO-178C / DO-254)
presets.aviation = {
    altitude         = {lo=-1000,hi=45000,severity="CRITICAL", name="altitude_ft"},
    airspeed         = {lo=0,   hi=350,  severity="CRITICAL", name="airspeed_kts"},
    vertical_speed   = {lo=-6000,hi=6000,severity="WARNING",  name="vertical_speed_fpm"},
    engine_temp      = {lo=100, hi=950,  severity="CRITICAL", name="egt_celsius"},
    fuel_flow        = {lo=0,   hi=500,  severity="CRITICAL", name="fuel_flow_pph"},
    hydraulic_press  = {lo=1500,hi=3500, severity="WARNING",  name="hydraulic_psi"},
}

-- Medical (IEC 62304 / IEC 60601)
presets.medical = {
    heart_rate       = {lo=30,  hi=220,  severity="CRITICAL", name="heart_rate_bpm"},
    spo2             = {lo=70,  hi=100,  severity="CRITICAL", name="spo2_percent"},
    body_temp        = {lo=30,  hi=45,   severity="CRITICAL", name="body_temp_celsius"},
    infusion_rate    = {lo=0,   hi=1200, severity="CRITICAL", name="infusion_ml_hr"},
    blood_pressure_s = {lo=60,  hi=250,  severity="WARNING",  name="bp_systolic"},
}

-- Nuclear (NRC 10 CFR 50)
presets.nuclear = {
    reactor_power    = {lo=0,   hi=100,  severity="CRITICAL", name="reactor_power_pct"},
    coolant_temp     = {lo=260, hi=343,  severity="CRITICAL", name="coolant_temp_c"},
    pressure         = {lo=14,  hi=18,   severity="CRITICAL", name="pressure_mpa"},
    radiation        = {lo=0,   hi=100,  severity="WARNING",  name="radiation_msv_hr"},
    containment_temp = {lo=20,  hi=65,   severity="WARNING",  name="containment_temp_c"},
}

-- Energy (IEC 61850)
presets.energy = {
    voltage          = {lo=90,  hi=110,  severity="CRITICAL", name="voltage_pct"},
    frequency        = {lo=49,  hi=51,   severity="CRITICAL", name="frequency_hz"},
    current          = {lo=0,   hi=3000, severity="WARNING",  name="current_amps"},
    transformer_temp = {lo=20,  hi=95,   severity="CRITICAL", name="xfmr_temp_c"},
    power_factor     = {lo=80,  hi=100,  severity="CAUTION",  name="power_factor_pct"},
}

return presets
