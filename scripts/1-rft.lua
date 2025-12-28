_Rft_oda_id = nil
_Rft_file = ""
_Rft_crc_data = ""
_Rft_file_segment = 0
_Rft_crc_segment = 0
_Rft_toggle = false
_Rft_last_id = -1
_Rft_version = 0
_Rft_crc = false
_Rft_crc_mode = 0
_Rft_crc_sent = false
_Rft_aid = 0xFF7F

local function start_rft()
    if _Rft_oda_id == nil then
        _Rft_oda_id = register_oda_rds2(_Rft_aid, 0, true)
        set_oda_handler_rds2(_Rft_oda_id, function ()
            if #_Rft_file == 0 then return false, 0, 0, 0, 0 end

            local total_segments = math.ceil(#_Rft_file / 5)
            local seg = _Rft_file_segment
            local base = seg * 5 + 1

            if not _Rft_crc_sent and _Rft_crc and (seg % 16 == 0) then
                local c = (1 << 12) | (_Rft_crc_mode & 7) << 9 | (_Rft_crc_segment & 0x1ff)
                _Rft_crc_segment = _Rft_crc_segment + 1
                if _Rft_crc_segment > #_Rft_crc_segment then _Rft_crc_segment = 0 end
                return true, (2 << 14), _Rft_aid, c, crc16(_Rft_file)
            else
                _Rft_crc_sent = false
            end

            local function b(i) return string.byte(_Rft_file, base + i) or 0 end

            local word1 = (((_Rft_toggle and 1 or 0) << 7) | ((seg >> 8) & 0x7F))
            local word2 = ((seg & 0xFF) << 8) | b(0)
            local word3 = (b(1) << 8) | b(2)
            local word4 = (b(3) << 8) | b(4)

            _Rft_file_segment = seg + 1
            if _Rft_file_segment >= total_segments then
                _Rft_file_segment = 0
            end

            return true, (2 << 12) | word1, word2, word3, word4
        end)
    end
end

---This function is defined externally
---Loads the file into RFT and initializes it if needed, note that this needs RDR2 mode 2
---@param path string
---@param id integer
---@param crc integer|boolean
function load_station_logo(path, id, crc)
    local file = io.open(path, "rb")
    if not file then error("Could not open file") end
    _Rft_file = file:read("*a")
    file:close()

    if id == _Rft_last_id then
        _Rft_toggle = not _Rft_toggle
        _Rft_crc_sent = 0
        _Rft_version = _Rft_version + 1
        if _Rft_version > 7 then _Rft_version = 0 end
    end

    _Rft_crc = (crc ~= false)
    if crc and (crc == 0 or crc == true) then
        _Rft_crc_data = string.char(crc16(_Rft_file))
        _Rft_crc_mode = 0
    elseif crc and crc == 1 then
        for i = 1, #_Rft_file, 5*16 do _Rft_crc_data = _Rft_crc_data + string.char(string.byte(_Rft_file, i, 5*16)) end
        _Rft_crc_mode = 1
    elseif crc and crc == 2 then
        for i = 1, #_Rft_file, 5*32 do _Rft_crc_data = _Rft_crc_data + string.char(string.byte(_Rft_file, i, 5*32)) end
        _Rft_crc_mode = 2
    elseif crc and crc == 3 then
        for i = 1, #_Rft_file, 5*64 do _Rft_crc_data = _Rft_crc_data + string.char(string.byte(_Rft_file, i, 5*64)) end
        _Rft_crc_mode = 3
    elseif crc and crc == 4 then
        for i = 1, #_Rft_file, 5*128 do _Rft_crc_data = _Rft_crc_data + string.char(string.byte(_Rft_file, i, 5*128)) end
    end

    if #_Rft_file > 262143 then error("The file is too large", 2) end
    if _Rft_oda_id == nil then start_rft() end
---@diagnostic disable-next-line: param-type-mismatch
    set_oda_id_data_rds2(_Rft_oda_id, #_Rft_file | (id & 63) << 18 | (_Rft_version & 7) << 24 | (_Rft_crc and 1 or 0) << 27)
    _Rft_last_id = id
end