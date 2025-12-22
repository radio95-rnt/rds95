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
        elseif data == "ct" then return string.format("CT=%s\r\n", string.format("%d", (get_rds_ct() and 1 or 0)))
        elseif data == "dpty" then return string.format("DPTY=%s\r\n", string.format("%d", (get_rds_dpty() and 1 or 0)))
        elseif data == "tp" then return string.format("TP=%s\r\n", string.format("%d", (get_rds_tp() and 1 or 0)))
        elseif data == "ta" then return string.format("TA=%s\r\n", string.format("%d", (get_rds_ta() and 1 or 0)))
        elseif data == "rt1en" then return string.format("RT1EN=%s\r\n", string.format("%d", (get_rds_rt1_enabled() and 1 or 0)))
        elseif data == "rt2en" then return string.format("RT2EN=%s\r\n", string.format("%d", (get_rds_rt2_enabled() and 1 or 0)))
        elseif data == "ptynen" then return string.format("PTYNEN=%s\r\n", string.format("%d", (get_rds_ptyn_enabled() and 1 or 0)))
        elseif data == "rttype" then return string.format("RTTYPE=%s\r\n", string.format("%d", get_rds_rt_type()))
        elseif data == "rds2mod" then return string.format("RDS2MOD=%s\r\n", string.format("%d", (get_rds_rds2mod() and 1 or 0)))
        elseif data == "rdsgen" then return string.format("RDSGEN=%s\r\n", string.format("%d",get_rds_rdsgen()))
        elseif data == "level" then return string.format("LEVEL=%s\r\n", string.format("%d", get_rds_level() * 255))
        elseif data == "link" then return string.format("LINK=%s\r\n", string.format("%d", (get_rds_link() and 1 or 0)))
        elseif data == "rtp" then
            local t1, s1, l1, t2, s2, l2 = get_rds_rtplus_tags(false)
            return string.format("RTP=%d,%d,%d,%d,%d,%d\r\n", t1, s1, l1, t2, s2, l2)
        elseif data == "ertp" then
            local t1, s1, l1, t2, s2, l2 = get_rds_rtplus_tags(true)
            return string.format("ERTP=%d,%d,%d,%d,%d,%d\r\n", t1, s1, l1, t2, s2, l2)
        elseif data == "rtprun" then
            local enabled, running = get_rds_rtp_meta(false)
            local f1 = enabled and 2 or (running and 1 or 0)
            return string.format("RTPRUN=%d\r\n", f1)
        elseif data == "ertprun" then
            local enabled, running = get_rds_rtp_meta(true)
            local f1 = enabled and 2 or (running and 1 or 0)
            return string.format("ERTPRUN=%d\r\n", f1)
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
        set_rds_ct(ct ~= 0)
        return "+"
    elseif cmd == "dpty" then
        local dpty = tonumber(value)
        if not dpty then return "-" end
        set_rds_dpty(dpty ~= 0)
        return "+"
    elseif cmd == "tp" then
        local tp = tonumber(value)
        if not tp then return "-" end
        set_rds_tp(tp ~= 0)
        return "+"
    elseif cmd == "ta" then
        local ta = tonumber(value)
        if not ta then return "-" end
        set_rds_ta(ta ~= 0)
        return "+"
    elseif cmd == "rt1en" then
        local en = tonumber(value)
        if not en then return "-" end
        set_rds_rt1_enabled(en ~= 0)
        return "+"
    elseif cmd == "rt2en" then
        local en = tonumber(value)
        if not en then return "-" end
        set_rds_rt2_enabled(en ~= 0)
        return "+"
    elseif cmd == "ptynen" then
        local en = tonumber(value)
        if not en then return "-" end
        set_rds_ptyn_enabled(en ~= 0)
        return "+"
    elseif cmd == "rttype" then
        local type = tonumber(value)
        if not type then return "-" end
        set_rds_rt_type(type)
        return "+"
    elseif cmd == "rds2mod" then
        local type = tonumber(value)
        if not type then return "-" end
        set_rds_rds2mod(type ~= 0)
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
        set_rds_link(link ~= 0)
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
    elseif cmd == "rtp" or cmd == "ertp" then
        local is_ertp = (cmd == "ertp")
        local t1, s1, l1, t2, s2, l2 = value:match("(%d+),(%d+),(%d+),(%d+),(%d+),(%d+)")

        if not l2 then return "-" end
        set_rds_rtplus_tags(
            is_ertp,
---@diagnostic disable-next-line: param-type-mismatch
            tonumber(t1), tonumber(s1), tonumber(l1), tonumber(t2), tonumber(s2), tonumber(l2)
        )
        return "+"
    elseif cmd == "g" then
        local a, b, c, d = value:match("^(%x%x%x%x)(%x%x%x%x)(%x%x%x%x)(%x%x%x%x)$")
        if a and b and c and d then
            put_rds2_custom_group(tonumber(a, 16), tonumber(b, 16), tonumber(c, 16), tonumber(d, 16))
            return "+"
        end

        local b1, c1, d1 = value:match("^(%x%x%x%x)(%x%x%x%x)(%x%x%x%x)$")
        if b1 and c1 and d1 then
            put_rds_custom_group(tonumber(b1, 16), tonumber(c1, 16), tonumber(d1, 16))
            return "+"
        end

        return "-"
    elseif cmd == "rtprun" or cmd == "ertprun" then
        local is_ertp = (cmd == "ertprun")

        local f1_str, f2_str = value:match("([^,]+),?([^,]*)")
        if not f1_str then return "-" end

        local f1 = tonumber(f1_str) or 0
        local f2 = tonumber(f2_str) or 0
        local enabled = (f1 == 2)
        local running = (f1 & 1) ~= 0

        set_rds_rtp_meta(is_ertp, enabled, running)
        if f2 ~= 0 then toggle_rds_rtp(is_ertp) end
        return "+"
    elseif cmd == "af" then
        local af_table = {}

        if value == "" or value == "0" then
            set_rds_af_group0({})
            return "+"
        end

        for freq_str in value:gmatch("([^,]+)") do
            local f = tonumber(freq_str)
            if f then table.insert(af_table, f)
            else return "-" end
        end

        if #af_table > 25 then return "-" end

        set_rds_af_group0(af_table)
        return "+"
    elseif cmd == "afo" then
        local af_table = {}

        if value == "" or value == "0" then
            set_rds_af_oda({})
            return "+"
        end

        for freq_str in value:gmatch("([^,]+)") do
            local f = tonumber(freq_str)
            if f then table.insert(af_table, f)
            else return "-" end
        end

        if #af_table > 25 then return "-" end

        set_rds_af_oda(af_table)
        return "+"
    else
        return "?"
    end
end