_Rft_oda_id = nil
_Rft_file = ""
_Rft_file_segment = 0
_Rft_toggle = false
_Rft_last_id = -1
_Rft_version = 0 -- TODO

local function start_rft()
    if _Rft_oda_id == nil then
        _Rft_oda_id = register_oda_rds2(0xFF7F, 0, true)
        set_oda_handler_rds2(_Rft_oda_id, function ()
            if #_Rft_file == 0 then return false, 0, 0, 0, 0 end

            local total_segments = math.ceil(#_Rft_file / 5)
            local seg = _Rft_file_segment
            local base = seg * 5 + 1

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
function load_station_logo(path, id)
    if id == _Rft_last_id then 
        _Rft_toggle = not _Rft_toggle
        _Rft_version = _Rft_version + 1
        if _Rft_version > 7 then _Rft_version = 0 end
    end
    local file = io.open(path, "rb")
    if not file then error("Could not open file") end
    _Rft_file = file:read("*a")
    file:close()

    if #_Rft_file > 262143 then error("The file is too large", 2) end
    if _Rft_oda_id == nil then start_rft() end
---@diagnostic disable-next-line: param-type-mismatch
    set_oda_id_data_rds2(_Rft_oda_id, #_Rft_file | (id & 63) << 18 | (_Rft_version & 7) << 24 | 0 << 27)
    _Rft_last_id = id
end