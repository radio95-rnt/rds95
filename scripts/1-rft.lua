_Rft_oda_id = nil
_Rft_file = ""
_Rft_crc_data = ""
_Rft_file_segment = 0
_Rft_crc_segment = 0
_Rft_toggle = false
_Rft_last_id = -1
_Rft_version = 0
_Rft_crc = false
_Rft_crc_full_file = 0
_Rft_crc_mode = 0
_Rft_crc_sent = false
_Rft_aid = 0
_Rft_send_once = false
_Rft_paused = false

local function stop_rft()
    if _Rft_oda_id ~= nil and _Rft_aid ~= 0 then
        unregister_oda_rds2(_Rft_oda_id)
        _Rft_oda_id = nil
        _Rft_aid = 0
    end

    _Rft_file = ""
    _Rft_crc_data = ""
    _Rft_file_segment = 0
    _Rft_crc_segment = 0
    _Rft_toggle = false
    _Rft_last_id = -1
    _Rft_version = 0
    _Rft_crc = false
    _Rft_crc_full_file = 0
    _Rft_crc_mode = 0
    _Rft_crc_sent = false
    _Rft_paused = false
end

local function start_rft()
    if _Rft_oda_id == nil and _Rft_aid ~= 0 then
        _Rft_oda_id = register_oda_rds2(_Rft_aid, 0, true)
        set_oda_handler_rds2(_Rft_oda_id, function (stream)
            if #_Rft_file == 0 or _Rft_paused then return false, 0, 0, 0, 0 end

            local total_segments = math.ceil(#_Rft_file / 5)
            local seg = _Rft_file_segment
            local base = seg * 5 + 1

            if not _Rft_crc_sent and _Rft_crc and (seg % 16 == 0) and stream == 1 then
                _Rft_crc_sent = true
                local chunk_address = math.floor((_Rft_crc_segment - 1) / 2)
                local c = (1 << 12) | (_Rft_crc_mode & 7) << 9 | (chunk_address & 0x1ff)

                local high_byte = 0
                local low_byte = 0
                if _Rft_crc_mode ~= 0 then
                    high_byte = string.byte(_Rft_crc_data, _Rft_crc_segment) or 0
                    low_byte = string.byte(_Rft_crc_data, _Rft_crc_segment + 1) or 0
                else
                    high_byte = _Rft_crc_full_file >> 8
                    low_byte = _Rft_crc_full_file & 0xff
                end

                _Rft_crc_segment = _Rft_crc_segment + 2
                if _Rft_crc_segment > #_Rft_crc_data then _Rft_crc_segment = 1 end

                return true, (2 << 14), _Rft_aid, c, (high_byte << 8) | low_byte
            else _Rft_crc_sent = false end

            local function b(i) return string.byte(_Rft_file, base + i) or 0 end

            local word1 = (((_Rft_toggle and 1 or 0) << 7) | ((seg >> 8) & 0x7F))
            local word2 = ((seg & 0xFF) << 8) | b(0)
            local word3 = (b(1) << 8) | b(2)
            local word4 = (b(3) << 8) | b(4)

            _Rft_file_segment = seg + 1
            if _Rft_file_segment >= total_segments then
                _Rft_file_segment = 0
                if _Rft_send_once then stop_rft() end
            end

            return true, (2 << 12) | word1, word2, word3, word4
        end)
    end
end

---This function is defined externally
---Loads the file into RFT and initializes it if needed, note that this needs RDS2 mode 2
---@param aid integer for station logo use 0xFF7F
---@param path string filesystem path on the os
---@param id integer mostly use 0 here
---@param crc integer|boolean false for disabled, true for mode 7, and an integer for any of the modes
---@param once boolean true means that this file will be sent once and then unregistered
---@return boolean interrupted
function send_rft_file(aid, path, id, crc, once)
    local interrupted = (#_Rft_file ~= 0)

    if _Rft_aid ~= aid then stop_rft() end
    _Rft_aid = aid

    local file = io.open(path, "rb")
    if not file then error("Could not open file") end
    _Rft_file = file:read("*a")
    file:close()

    _Rft_send_once = once

    if id == _Rft_last_id then
        _Rft_toggle = not _Rft_toggle
        _Rft_crc_sent = 0
        _Rft_version = _Rft_version + 1
        if _Rft_version > 7 then _Rft_version = 0 end
    end

    _Rft_crc_data = ""
    _Rft_crc = (crc ~= false)

    local chunk_size = 0
    if crc and crc == 0 then
        _Rft_crc_mode = 0
        _Rft_crc_full_file = crc16(_Rft_file)
    elseif crc and crc == 1 and #_Rft_file <= 40960 then
        _Rft_crc_mode = 1
        chunk_size = 5 * 16
    elseif crc and crc == 2 and #_Rft_file < 40960 and #_Rft_file >= 81920 then
        _Rft_crc_mode = 2
        chunk_size = 5 * 32
    elseif crc and crc == 3 and #_Rft_file > 81960 then
        _Rft_crc_mode = 3
        chunk_size = 5 * 64
    elseif crc and crc == 4 and #_Rft_file > 81960 then
        _Rft_crc_mode = 4
        chunk_size = 5 * 128
    elseif crc and crc == 5 and #_Rft_file > 81960 then
        _Rft_crc_mode = 5
        chunk_size = 5 * 256
    elseif crc and (crc == 7 or crc == true) then
        if #_Rft_file <= 40960 then
            _Rft_crc_mode = 1
            chunk_size = 5*16
        elseif #_Rft_file > 40960 and #_Rft_file <= 81920 then
            _Rft_crc_mode = 2
            chunk_size = 5*32
        elseif #_Rft_file > 81960 then
            _Rft_crc_mode = 3
            chunk_size = 5*64
        end
    else
        _Rft_crc = false
    end

    if _Rft_crc and chunk_size ~= 0 then
        for i = 1, #_Rft_file, chunk_size do
            local chunk = string.sub(_Rft_file, i, i + chunk_size - 1)
            local crc_val = crc16(chunk)
            _Rft_crc_data = _Rft_crc_data .. string.char(math.floor(crc_val / 256), crc_val % 256)
        end
    end

    if #_Rft_file > 262143 then error("The file is too large", 2) end
    if _Rft_oda_id == nil then start_rft() end
---@diagnostic disable-next-line: param-type-mismatch
    set_oda_id_data_rds2(_Rft_oda_id, #_Rft_file | (id & 63) << 18 | (_Rft_version & 7) << 24 | (_Rft_crc and 1 or 0) << 27)
    _Rft_last_id = id

    _Rft_paused = false
    return interrupted
end

---Pauses or resumes the process of sending of the RFT file
---@param aid integer
---@param paused boolean
function set_rft_paused(aid, paused)
    if aid ~= _Rft_aid then error("AID does not match", 2) end
    _Rft_paused = paused
end

local _old_on_state_oda_rft = on_state
function on_state()
    stop_rft()
    if type(_old_on_state_oda_rft) == "function" then _old_on_state_oda_rft() end
end
