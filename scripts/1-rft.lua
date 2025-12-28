_Rft_oda_id = nil
_Rft_file = ""
_Rft_file_segment = 0
_Rft_toggle = false

local function start_rft()
    if _Rft_oda_id == nil then
        _Rft_oda_id = register_oda_rds2(0xFF7F, 0, true)
        set_oda_handler_rds2(_Rft_oda_id, function ()
            if #_Rft_file == 0 then
                return false, 0, 0, 0, 0
            end

            local total_segments = math.ceil(#_Rft_file / 5)
            local seg = _Rft_file_segment
            local base = seg * 5 + 1

            local function b(i)
                return string.byte(_Rft_file, base + i) or 0
            end

            local word1 =
                (((_Rft_toggle and 1 or 0) << 7) |
                ((seg >> 8) & 0x7F))

            local word2 =
                ((seg & 0xFF) << 8) | b(0)

            local word3 =
                (b(1) << 8) | b(2)

            local word4 =
                (b(3) << 8) | b(4)

            _Rft_file_segment = seg + 1
            if _Rft_file_segment >= total_segments then
                _Rft_file_segment = 0
                _Rft_toggle = not _Rft_toggle
            end

            return true, (2 << 12) | word1, word2, word3, word4
        end)
        set_rds2_mode(2)
    end
end

local function load_rft_file()
    local file = io.open("/tmp/rft_test", "rb")
    if not file then error("Could not open file") end
    _Rft_file = file:read("*a")
    file:close()
    if #_Rft_file > 262143 then error("The file is too large", 2) end
    if _Rft_oda_id == nil then start_rft() end
---@diagnostic disable-next-line: param-type-mismatch
    set_oda_id_data_rds2(_Rft_oda_id, #_Rft_file | 48 << 18) --- 48 for testing, change to 0 or something else in the future
end

-- local _old_on_state_rft = on_state
-- function on_state()
--     start_rft()
--     if type(_old_on_state_rft) == "function" then _old_on_state_rft() end
-- end