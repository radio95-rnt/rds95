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
            else
                _Rft_crc_sent = false
            end

            local function b(i) return string.byte(_Rft_file, base + i) or 0 end

            local word1 = (((_Rft_toggle and 1 or 0) << 7) | ((seg >> 8) & 0x7F))
            local word2 = ((seg & 0xFF) << 8) | b(0)
            local word3 = (b(1) << 8) | b(2)
            local word4 = (b(3) << 8) | b(4)

            _Rft_file_segment = seg + 1
            if _Rft_file_segment >= total_segments then _Rft_file_segment = 0 end

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

    _Rft_crc_data = "" -- Clear previous CRC data
    _Rft_crc = (crc ~= false)

    if crc and crc == 0 then
        _Rft_crc_mode = 0
        _Rft_crc_full_file = crc16(_Rft_file)
    elseif crc and crc == 1 and #_Rft_file <= 40960 then
        _Rft_crc_mode = 1
        local chunk_size = 5 * 16 -- 80 bytes
        for i = 1, #_Rft_file, chunk_size do
            local chunk = string.sub(_Rft_file, i, i + chunk_size - 1)
            local crc_val = crc16(chunk)
            _Rft_crc_data = _Rft_crc_data .. string.char(math.floor(crc_val / 256), crc_val % 256)
        end
    elseif crc and crc == 2 and #_Rft_file < 40960 and #_Rft_file >= 81920 then
        _Rft_crc_mode = 2
        local chunk_size = 5 * 32
        for i = 1, #_Rft_file, chunk_size do
            local chunk = string.sub(_Rft_file, i, i + chunk_size - 1)
            local crc_val = crc16(chunk)
            _Rft_crc_data = _Rft_crc_data .. string.char(math.floor(crc_val / 256), crc_val % 256)
        end
    elseif crc and crc == 3 and #_Rft_file > 81960 then
        _Rft_crc_mode = 3
        local chunk_size = 5 * 64
        for i = 1, #_Rft_file, chunk_size do
            local chunk = string.sub(_Rft_file, i, i + chunk_size - 1)
            local crc_val = crc16(chunk)
            _Rft_crc_data = _Rft_crc_data .. string.char(math.floor(crc_val / 256), crc_val % 256)
        end
    elseif crc and crc == 4 and #_Rft_file > 81960 then
        _Rft_crc_mode = 4
        local chunk_size = 5 * 128
        for i = 1, #_Rft_file, chunk_size do
            local chunk = string.sub(_Rft_file, i, i + chunk_size - 1)
            local crc_val = crc16(chunk)
            _Rft_crc_data = _Rft_crc_data .. string.char(math.floor(crc_val / 256), crc_val % 256)
        end
    elseif crc and crc == 5 and #_Rft_file > 81960 then
        _Rft_crc_mode = 5
        local chunk_size = 5 * 256
        for i = 1, #_Rft_file, chunk_size do
            local chunk = string.sub(_Rft_file, i, i + chunk_size - 1)
            local crc_val = crc16(chunk)
            _Rft_crc_data = _Rft_crc_data .. string.char(math.floor(crc_val / 256), crc_val % 256)
        end
    elseif crc and (crc == 7 or crc == true) then
        _Rft_crc_mode = 7
        local chunk_size = 0
        if #_Rft_file <= 40960 then chunk_size = 5*16
        elseif #_Rft_file > 40960 and #_Rft_file <= 81920 then chunk_size = 5*32
        elseif #_Rft_file > 81960 then chunk_size = 5*64 end
        if chunk_size ~= 0 then
            for i = 1, #_Rft_file, chunk_size do
                local chunk = string.sub(_Rft_file, i, i + chunk_size - 1)
                local crc_val = crc16(chunk)
                _Rft_crc_data = _Rft_crc_data .. string.char(math.floor(crc_val / 256), crc_val % 256)
            end
        end
    else
        _Rft_crc = false
    end

    if #_Rft_file > 262143 then error("The file is too large", 2) end
    if _Rft_oda_id == nil then start_rft() end
---@diagnostic disable-next-line: param-type-mismatch
    set_oda_id_data_rds2(_Rft_oda_id, #_Rft_file | (id & 63) << 18 | (_Rft_version & 7) << 24 | (_Rft_crc and 1 or 0) << 27)
    _Rft_last_id = id
end