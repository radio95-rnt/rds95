uecp = {}
uecp.site_addreses = {}
uecp.encoder_addreses = {}
uecp.rt_buffer = {}
uecp.rt_buffer_index = 1
uecp.rt_tx_remaining = 0

hooks.rt_transmission[#hooks.rt_transmission + 1] = function ()
    if #uecp.rt_buffer == 0 then return end

    if #uecp.rt_buffer == 1 then return end

    if uecp.rt_tx_remaining > 1 then
        uecp.rt_tx_remaining = uecp.rt_tx_remaining - 1
        return
    end

    uecp.rt_buffer_index = (uecp.rt_buffer_index % #uecp.rt_buffer) + 1
    local entry = uecp.rt_buffer[uecp.rt_buffer_index]
    uecp.rt_tx_remaining = entry.number_tx

    rds.set_rt_raw(entry.text)
    if entry.toggle_ab then rds.toggle_rt_ab() end
end


local function dsn_helper(dsn, write)
    if dsn == 0 then write()
    elseif dsn == 254 then
        local start = dp.get_writing_program()
        for i = 1, dp.max_programs do
            local p = i - 1
            if p ~= start then
                dp.set_writing_program(p)
                write()
            end
        end
    elseif dsn == 255 then
        for i = 1, dp.max_programs do
            dp.set_writing_program(i - 1)
            write()
        end
    else
        dp.set_writing_program(dsn - 1)
        write()
    end
end

local mec_handlers = {}
mec_handlers[1] = function(data)
    -- PI
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local pi_msb = string.byte(data, 4)
    local pi_lsb = string.byte(data, 5)

    dsn_helper(dsn, function()
        rds.set_pi((pi_msb << 8) | pi_lsb)
    end)
    return 5
end
mec_handlers[2] = function(data)
    -- PS
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local ps = string.sub(data, 4, 11) -- Static len
    dsn_helper(dsn, function()
        rds.set_ps(ps)
    end)
    return 11
end
mec_handlers[0x21] = function(data)
    -- LPS
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local mel = string.byte(data, 4)
    local lps = string.sub(data, 5, 5+mel)

    local cut = string.find(lps, "\r", 1, true)
    if cut then lps = string.sub(lps, 1, cut - 1) end

    dsn_helper(dsn, function()
        rds.set_lps(lps)
    end)
    return 4 + mel
end
mec_handlers[4] = function(data)
    -- PTYI
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local data = string.byte(data, 4)
    dsn_helper(dsn, function ()
        rds.set_dpty((data & 8) ~= 0)
    end)
    return 4
end
mec_handlers[3] = function(data)
    -- TP/TA
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local data = string.byte(data, 4)
    dsn_helper(dsn, function ()
        rds.set_ta((data & 1) ~= 0)
        rds.set_tp((data & 2) ~= 0)
    end)
    return 4
end
mec_handlers[7] = function(data)
    -- PTY
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local data = string.byte(data, 4)
    dsn_helper(dsn, function ()
        rds.set_pty(data)
    end)
    return 4
end
mec_handlers[0x3E] = function(data)
    -- PTYN
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local ptyn = string.sub(data, 4, 11) -- Static
    dsn_helper(dsn, function ()
        rds.set_ptyn(ptyn)
    end)
    return 11
end
mec_handlers[0x0A] = function(data)
    -- RT
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local mel = string.byte(data, 4)

    if mel == 0 then
        uecp.rt_buffer = {}
        uecp.rt_buffer_index = 1
        uecp.rt_tx_remaining = 0
        dsn_helper(dsn, function ()
            rds.set_rt_enabled(false)
        end)
        return 4
    end

    local config = string.byte(data, 5)
    local toggle_ab = (config & 1) ~= 0
    local number_tx = (config >> 1) & 0xF
    local buffer_config = (config >> 5) & 3

    if mel == 1 and buffer_config == 0 then
        uecp.rt_buffer = {}
        uecp.rt_buffer_index = 1
        uecp.rt_tx_remaining = 0
        dsn_helper(dsn, function ()
            rds.set_rt_enabled(false)
        end)
        return 5
    end

    local rt_text = string.sub(data, 6, 4 + mel)
    local cr = string.find(rt_text, "\r", 1, true)
    if cr then rt_text = string.sub(rt_text, 1, cr - 1) end

    if buffer_config == 0 then
        uecp.rt_buffer = {}
        uecp.rt_buffer_index = 1
        uecp.rt_buffer[1] = { text = rt_text, number_tx = number_tx, toggle_ab = toggle_ab }
        uecp.rt_tx_remaining = number_tx
        -- Seed the encoder immediately with the first entry
        dsn_helper(dsn, function ()
            rds.set_rt_raw(rt_text)
            if toggle_ab then rds.toggle_rt_ab() end
            rds.set_rt_enabled(true)
        end)
    elseif buffer_config == 2 then
        uecp.rt_buffer[#uecp.rt_buffer + 1] = { text = rt_text, number_tx = number_tx, toggle_ab = toggle_ab }
    end

    return 4 + mel
end
mec_handlers[0x13] = function (data)
    -- AF
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local mel = string.sub(data, 4)

    -- TODO: decode - i have zero clue what the hell is the meaning of the docs, it might as well be in chinese (traditional)

    return 4+mel
end
mec_handlers[0x14] = function (data)
    -- EON-AF
    local dsn = string.byte(data, 2)
    local psn = string.byte(data, 3)
    local mel = string.sub(data, 4)

    -- TODO: decode - haven't read this one

    return 4+mel
end
mec_handlers[0x1A] = function (data)
    -- ECC / SLS
    local dsn = string.byte(data, 2)
    -- No PSN
    local data = string.sub(data, 3)
    local data_lsb = string.sub(data, 4)

    local full_data = ((data & 15) << 8) | data_lsb
    local variant = (data >> 4) & 3
    if variant == 0 then
        -- TODO: maybe make it more than just ecc? idk why but would be cool
        dsn_helper(dsn, function ()
            rds.set_ecc(full_data)
        end)
    end

    return 4
end
mec_handlers[0x2E] = function (data)
    -- Linkage Information
    -- What? What is the main PSN? Does the server tell me that or am i to fucking guess?
    return 5
end
mec_handlers[0x24] = function (data)
    -- Free-format data in type A or B group
    local group = string.byte(data, 2)
    local bufferdata = string.byte(data, 3)
    local buffer = (bufferdata >> 5) & 3 -- No clue
    local blockb = bufferdata & 31
    local blockc_msb = string.byte(data, 4)
    local blockc_lsb = string.byte(data, 5)
    local blockd_msb = string.byte(data, 6)
    local blockd_lsb = string.byte(data, 7)
    rds.put_custom_group((group << 11) | blockb, (blockc_msb << 8) | blockc_lsb, (blockd_msb << 8) | blockd_lsb)
    return 7
end
-- Fuck you mean, i have to implement some fucking "spinning wHELLs"? may the lord have mercy
-- Also no time setting via UECP - this is linux, just install fucking chrony
mec_handlers[0x19] = function (data)
    -- CT
    local data = string.byte(data, 2)
    dsn_helper(255, function ()
        rds.set_ct(data ~= 0)
    end)
    return 2
end
mec_handlers[0x16] = function (data)
    -- Group sequence for stream 0
    local dsn = string.byte(data, 2)
    local mel = string.byte(data, 3)
    local group_sequence = string.sub(data, 4, 3+mel)
    dsn_helper(dsn, function ()
        rds.set_grpseq(group_sequence)
    end)
    return 4 + mel
end
mec_handlers[0x23] = function (data)
    -- Site address
    local control_bits = string.byte(data, 2)
    local high = string.byte(data, 3)
    local low = string.byte(data, 4)
    local data = high << 8 | low
    if control_bits == 0 then
        for i, v in ipairs(uecp.site_addreses) do
            if v == data then table.remove(uecp.site_addreses, i) end
        end
    elseif control_bits == 1 then uecp.site_addreses[#uecp.site_addreses+1] = data
    elseif control_bits == 2 then uecp.site_addreses = {} end
    return 4
end
mec_handlers[0x27] = function (data)
    -- Encoder address
    local control_bits = string.byte(data, 2)
    local data = string.byte(data, 3)
    if control_bits == 0 then
        for i, v in ipairs(uecp.encoder_addreses) do
            if v == data then table.remove(uecp.encoder_addreses, i) end
        end
    elseif control_bits == 1 then uecp.encoder_addreses[#uecp.encoder_addreses+1] = data
    elseif control_bits == 2 then uecp.encoder_addreses = {} end
    return 3
end
mec_handlers[0x1C] = function (data)
    -- DS select
    local dsn = string.byte(data, 2)
    dp.set_output_program(dsn)
    dp.set_writing_program(dsn)
    return 2
end
mec_handlers[0x2D] = function (data)
    -- Manufacturer
    local mel = string.byte(data, 2)
    local designation = string.sub(data, 3, 4)
    if string.byte(designation, 1) == 0x39 and string.byte(designation, 2) == 0x35 and mel > 2 then
        pcall(hooks.parse_ascii, string.sub(data, 5, 4+mel))
    end
    return 2+mel
end

---@param data string
local function undo_byte_stuff(data)
    local output = {}
    local i = 1
    while i <= #data do
        local d0 = string.byte(data, i)
        if d0 == 0xFD then
            local d1 = string.byte(data, i + 1)
            if not d1 then error("truncated escape sequence") end
            output[#output + 1] = string.char(0xFD + d1)
            i = i + 2
        else
            output[#output + 1] = string.char(d0)
            i = i + 1
        end
    end
    return table.concat(output)
end

---@param site integer
---@param encoder integer
---@return boolean
local function check_address(site, encoder)
    if site == 0 and encoder == 0 then return true end

    local site_allowed = false
    if site ~= 0 then
        for i, v in ipairs(uecp.site_addreses) do
            if v == site then
                site_allowed = true
                break
            end
        end
    else site_allowed = true end

    local encoder_allowed = false
    if encoder ~= 0 then
        for i, v in ipairs(uecp.encoder_addreses) do
            if v == encoder then
                encoder_allowed = true
                break
            end
        end
    else encoder_allowed = true end

    return site_allowed and encoder_allowed
end

---@param packet string
function uecp.parse_uecp(packet)
    if not packet or #packet == 0 then return end
    if string.byte(packet, 1) ~= 0xfe or string.byte(packet, #packet) ~= 0xff then return end

    local unstuffed = undo_byte_stuff(string.sub(packet, 2, #packet - 1))

    local addr1 = string.byte(unstuffed, 1)
    local addr2 = string.byte(unstuffed, 2)
    local site_addr = (addr1 << 2) | (addr2 >> 6)

    if not check_address(site_addr, addr2 & 0x3F) then return end

    local sequence_count = string.byte(unstuffed, 3)
    local mfl = string.byte(unstuffed, 4)

    local data = string.sub(unstuffed, 5, 4 + mfl)

    if mfl ~= #data then
        print("data len not right")
        return
    end

    local crc_calculated = dp.crc16(string.sub(unstuffed, 1, 4 + mfl))
    local crc_hi = string.byte(unstuffed, 4 + mfl + 1)
    local crc_stored = (crc_hi << 8) | string.byte(unstuffed, 4 + mfl + 2)

    if crc_calculated ~= crc_stored then
        print("bad crc")
        return
    end

    local consumed = 1
    while consumed <= #data do
        local mec = string.byte(data, consumed)
        local handler = mec_handlers[mec]
        if not handler then
            print(string.format("unknown MEC 0x%02X, cannot continue", mec))
            break
        end
        local advance = handler(string.sub(data, consumed))
        consumed = consumed + advance
        dp.set_writing_program(dp.get_output_program())
    end
end