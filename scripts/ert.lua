_Ert_state = 0
_Ert_oda_id = nil

local function init_ert()
    if _Ert_oda_id == nil then
        _Ert_oda_id = register_oda(12, false, 0x6552, 1)
        set_oda_handler(_Ert_oda_id, function ()
            if string.byte(get_userdata_offset(257, 1)) == 1 then
                local new_data = get_userdata_offset(0, 128)
                local new_segments = string.byte(get_userdata_offset(128, 1))
                set_userdata_offset(129, 128, new_data)
                set_userdata_offset(257, 1, string.char(new_segments))
                set_userdata_offset(258, 1, string.char(0))
                _Ert_state = 0
            end

            local segments = string.byte(get_userdata_offset(257, 1))
            if segments == 0 then segments = 1 end

            local b = _Ert_state & 31
            local chunk = get_userdata_offset(129 + _Ert_state * 4, 4)
            local c = (string.byte(chunk, 1) << 8) | string.byte(chunk, 2)
            local d = (string.byte(chunk, 3) << 8) | string.byte(chunk, 4)

            _Ert_state = (_Ert_state + 1) % segments
            return b, c, d
        end)
    end
end

function set_rds_ert(ert)
    if #ert == 0 then
        set_userdata_offset(128, 1, string.char(0))
        set_userdata_offset(258, 1, string.char(1))
        return
    end

    local data = ert .. "\r"
    data = string.sub(data, 1, 128)

    local padding = (4 - (#data % 4)) % 4
    data = data .. string.rep("\0", padding)

    set_userdata_offset(0, 128, data)

    local segments = #data // 4
    if segments > 32 then segments = 32 end

    if string.byte(get_userdata_offset(257, 1)) == 0 then
        init_ert()
        set_userdata_offset(129, 128, data)
        set_userdata_offset(257, 1, string.char(segments))
    else set_userdata_offset(258, 1, string.char(1)) end

    set_userdata_offset(128, 1, string.char(segments))
end

function get_rds_ert()
    local data = get_userdata_offset(0, 128)
    if string.byte(get_userdata_offset(128, 1)) == 0 then return "" end
    return data:match("^(.-)[\r%z]*") or ""
end

function on_state()
    if string.byte(get_userdata_offset(257, 1)) ~= 0 then init_ert() end
end