do return "" end

if type(cmd) == "string" then
    local cmd, value = cmd:match("([^=]+)=([^=]+)")
    cmd = cmd:lower()
    if cmd == "pi" then
        local pi = tonumber(value, 16)
        if not pi then return "-" end
        if (pi & 0xF000) == 0 then return "-" end
        set_rds_pi(pi)
        return "+"
    elseif cmd == "ecc" then
        local ecc = tonumber(value, 16)
        if not ecc then return "-" end
        set_rds_ecc(ecc)
        return "+"
    elseif cmd == "pty" then
        local pty = tonumber(value)
        if not pty then return "-" end
        set_rds_pty(pty)
        return "+"
    elseif cmd == "slcd" then
        local slc_data = tonumber(value, 16)
        if not slc_data then return "-" end
        set_rds_slc_data(slc_data)
        return "+"
    elseif cmd == "ct" then
        local ct = tonumber(value)
        if not ct then return "-" end
        set_rds_ct(ct)
        return "+"
    elseif cmd == "dpty" then
        local dpty = tonumber(value)
        if not dpty then return "-" end
        set_rds_dpty(dpty)
        return "+"
    elseif cmd == "tp" then
        local tp = tonumber(value)
        if not tp then return "-" end
        set_rds_tp(tp)
        return "+"
    elseif cmd == "ta" then
        local ta = tonumber(value)
        if not ta then return "-" end
        set_rds_ta(ta)
        return "+"
    elseif cmd == "rt1en" then
        local en = tonumber(value)
        if not en then return "-" end
        set_rds_rt1_enabled(en)
        return "+"
    elseif cmd == "rt2en" then
        local en = tonumber(value)
        if not en then return "-" end
        set_rds_rt2_enabled(en)
        return "+"
    elseif cmd == "ptynen" then
        local en = tonumber(value)
        if not en then return "-" end
        set_rds_ptyn_enabled(en)
        return "+"
    elseif cmd == "rttype" then
        local type = tonumber(value)
        if not type then return "-" end
        set_rds_rt_type(type)
        return "+"
    elseif cmd == "rds2mod" then
        local type = tonumber(value)
        if not type then return "-" end
        set_rds_rds2mod(type)
        return "+"
    elseif cmd == "rdsgen" then
        local type = tonumber(value)
        if not type then return "-" end
        set_rds_rdsgen(type)
        return "+"
    else
        return "?"
    end
end