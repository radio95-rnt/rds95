if type(data) == "string" and data ~= nil then
    local cmd, value = data:match("([^=]+)=([^=]+)")
    if cmd == nil then
        data = data:lower()
        if data == "ver" then
            return string.format("rds95 v. %s - (C) 2025 radio95 - lua parser", core_version)
        elseif data == "init" then
            set_rds_program_defaults()
            return "+"
        elseif data == "reset" then
            reset_rds()
            return "+"
        elseif data == "pi" then return string.format("PI=%s\r\n", string.format("%x", get_rds_pi()))
        elseif data == "pty" then return string.format("PTY=%s\r\n", string.format("%d", get_rds_pty()))
        elseif data == "ecc" then return string.format("ECC=%s\r\n", string.format("%x", get_rds_ecc()))
        elseif data == "slcd" then return string.format("SLCD=%s\r\n", string.format("%x", get_rds_slc_data()))
        elseif data == "ct" then return string.format("CT=%s\r\n", string.format("%d", get_rds_ct()))
        elseif data == "dpty" then return string.format("DPTY=%s\r\n", string.format("%d", get_rds_dpty()))
        elseif data == "tp" then return string.format("TP=%s\r\n", string.format("%d", get_rds_tp()))
        elseif data == "ta" then return string.format("TA=%s\r\n", string.format("%d", get_rds_ta()))
        else return "?" end
        -- TODO: more
    end
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
    elseif cmd == "ptyn" then
        set_rds_ptyn(value)
        return "+"
    elseif cmd == "ps" then
        set_rds_ps(value)
        return "+"
    elseif cmd == "tps" then
        set_rds_tps(value)
        return "+"
    elseif cmd == "rt1" or cmd == "text" then
        set_rds_rt1(value)
        return "+"
    elseif cmd == "rt2" then
        set_rds_rt2(value)
        return "+"
    elseif cmd == "lps" then
        set_rds_lps(value)
        return "+"
    elseif cmd == "ert" then
        set_rds_ert(value)
        return "+"
    elseif cmd == "link" then
        local link = tonumber(value)
        if not link then return "-" end
        set_rds_link(link)
        return "+"
    elseif cmd == "rtper" then
        local period = tonumber(value)
        if not period then return "-" end
        set_rds_rt_switching_period(period)
        return "+"
    elseif cmd == "program" then
        local program = tonumber(value)
        if not program then return "-" end
        if program < 1 or program > max_programs then return "-" end
        set_rds_program(program-1)
        return "+"
    elseif cmd == "level" then
        local level = tonumber(value)
        if not level then return "-" end
        set_rds_level(level/255)
        return "+"
    elseif cmd == "dttmout" then
        local timeout = tonumber(value)
        if not timeout then return "-" end
        set_rds_rt_text_timeout(timeout)
        return "+"
    elseif cmd == "grpseq" then
        set_rds_grpseq(value)
        return "+"
    elseif cmd == "grpseq2" then
        set_rds_grpseq2(value)
        return "+"
    else
        return "?"
    end
end