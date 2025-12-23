if type(data) == "string" and data ~= nil then
    local cmd, value = data:match("([^=]+)=([^=]+)")
    if cmd == nil then
        data = data:lower()
        if data == "ver" then return string.format("rds95 v. %s - (C) 2025 radio95 - lua parser", core_version)
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
        elseif data == "lps" then return string.format("LPS=%s\r\n", get_rds_lps())
        elseif data == "ert" then return string.format("ERT=%s\r\n", get_rds_ert())
        elseif data == "grpseq" then return string.format("GRPSEQ=%s\r\n", get_rds_grpseq())
        elseif data == "grpseq2" then return string.format("GRPSEQ2=%s\r\n", get_rds_grpseq2())
        else
            local eon_cmd, eon_num = data:match("^eon(%d+)([a-z]+)$")
            if eon_cmd then
                local eon_idx = tonumber(eon_cmd)
                if not eon_idx or eon_idx < 1 or eon_idx > eon_count then return "?" end
                eon_idx = eon_idx - 1
                local enabled, pi, tp, ta, pty, ps, afs, data_val = get_rds_eon(eon_idx)

                if eon_num == "en" then return string.format("EON%dEN=%d\r\n", eon_idx + 1, enabled and 1 or 0)
                elseif eon_num == "pi" then return string.format("EON%dPI=%x\r\n", eon_idx + 1, pi)
                elseif eon_num == "ps" then return string.format("EON%dPS=%s\r\n", eon_idx + 1, ps)
                elseif eon_num == "pty" then return string.format("EON%dPTY=%d\r\n", eon_idx + 1, pty)
                elseif eon_num == "ta" then return string.format("EON%dTA=%d\r\n", eon_idx + 1, ta and 1 or 0)
                elseif eon_num == "tp" then return string.format("EON%dTP=%d\r\n", eon_idx + 1, tp and 1 or 0)
                elseif eon_num == "dt" then return string.format("EON%dDT=%x\r\n", eon_idx + 1, data_val)
                end
            end
            return "?"
        end
    end

    cmd = cmd:lower()

    local eon_num, eon_type = cmd:match("^eon(%d+)([a-z]+)$")
    if eon_num then
        local eon_idx = tonumber(eon_num)
        if not eon_idx or eon_idx < 1 or eon_idx > eon_count then return "?" end
        eon_idx = eon_idx - 1

        local enabled, pi, tp, ta, pty, ps, afs, data_val = get_rds_eon(eon_idx)
        if eon_type == "en" then
            local en_val = tonumber(value)
            if not en_val then return "-" end
            enabled = (en_val ~= 0)
            set_rds_eon(eon_idx, enabled, pi, tp, ta, pty, ps, afs, data_val)
            return "+"
        elseif eon_type == "pi" then
            local pi_val = tonumber(value, 16)
            if not pi_val then return "-" end
            set_rds_eon(eon_idx, enabled, pi_val, tp, ta, pty, ps, afs, data_val)
            return "+"
        elseif eon_type == "ps" then
            local ps_val = value:sub(1, 24)
            set_rds_eon(eon_idx, enabled, pi, tp, ta, pty, ps_val, afs, data_val)
            return "+"
        elseif eon_type == "pty" then
            local pty_val = tonumber(value)
            if not pty_val then return "-" end
            set_rds_eon(eon_idx, enabled, pi, tp, ta, pty_val, ps, afs, data_val)
            return "+"
        elseif eon_type == "ta" then
            if not enabled or not tp then return "-" end
            local ta_val = tonumber(value)
            if not ta_val then return "-" end
            ta = (ta_val ~= 0)
            set_rds_eon(eon_idx, enabled, pi, tp, ta, pty, ps, afs, data_val)
            if ta then set_rds_ta(true) end
            return "+"
        elseif eon_type == "tp" then
            local tp_val = tonumber(value)
            if not tp_val then return "-" end
            tp = (tp_val ~= 0)
            set_rds_eon(eon_idx, enabled, pi, tp, ta, pty, ps, afs, data_val)
            return "+"
        elseif eon_type == "af" then
            local af_table = {}
            if value == "" or value == "0" then
                set_rds_eon(eon_idx, enabled, pi, tp, ta, pty, ps, {}, data_val)
                return "+"
            end
            for freq_str in value:gmatch("([^,]+)") do
                local f = tonumber(freq_str)
                if f then table.insert(af_table, f)
                else return "-" end
            end
            if #af_table > 25 then return "-" end
            set_rds_eon(eon_idx, enabled, pi, tp, ta, pty, ps, af_table, data_val)
            return "+"
        elseif eon_type == "dt" then
            local dt_val = tonumber(value, 16)
            if not dt_val then return "-" end
            set_rds_eon(eon_idx, enabled, pi, tp, ta, pty, ps, afs, dt_val)
            return "+"
        else return "?" end
    end

    local udg_num = cmd:match("^udg([12])$")
    local udg2_num = cmd:match("^2udg([12])$")
    if udg_num then
        local xy = (udg_num == "1")
        local groups = {}

        for segment in value:gmatch("([^,]+)") do
            local b, c, d = segment:match("^(%x%x%x%x)(%x%x%x%x)(%x%x%x%x)$")
            if not (b and c and d) then return "-" end
            table.insert(groups, {tonumber(b, 16), tonumber(c, 16), tonumber(d, 16)})
        end

        if #groups > 8 or #groups == 0 then return "-" end
        set_rds_udg(xy, groups)
        return "+"
    end
    if udg2_num then
        local xy = (udg2_num == "1")
        local groups = {}

        for segment in value:gmatch("([^,]+)") do
            local a, b, c, d = segment:match("^(%x%x%x%x)(%x%x%x%x)(%x%x%x%x)(%x%x%x%x)$")
            if not (a and b and c and d) then return "-" end
            table.insert(groups, {tonumber(a, 16), tonumber(b, 16), tonumber(c, 16), tonumber(d, 16)})
        end

        if #groups > 8 or #groups == 0 then return "-" end
        set_rds_udg2(xy, groups)
        return "+"
    end

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