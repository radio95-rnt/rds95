_Ert_state = 0
_Ert_oda_id = nil

--- Size: 259 bytes
local USERDATA_ERT_OFFSET = 0

local function init_ert()
    if _Ert_oda_id == nil then
        _Ert_oda_id = ext.register_oda(13, false, 0x6552, 1)
        ext.set_oda_handler(_Ert_oda_id, function ()
            if string.byte(userdata.get_offset(USERDATA_ERT_OFFSET+258, 1)) == 1 then
                local new_data = userdata.get_offset(USERDATA_ERT_OFFSET, 128)
                local new_segments = string.byte(userdata.get_offset(USERDATA_ERT_OFFSET+128, 1))
                userdata.set_offset(USERDATA_ERT_OFFSET+129, 128, new_data)
                userdata.set_offset(USERDATA_ERT_OFFSET+257, 1, string.char(new_segments))
                userdata.set_offset(USERDATA_ERT_OFFSET+258, 1, string.char(0))
                _Ert_state = 0
            end

            local segments = string.byte(userdata.get_offset(USERDATA_ERT_OFFSET+257, 1))

            if segments == 0 then return false, 0, 0, 0 end

            if _Ert_state >= segments then _Ert_state = 0 end

            local b = _Ert_state & 31
            local chunk = userdata.get_offset(USERDATA_ERT_OFFSET + 129 + _Ert_state * 4, 4)
            local c = (string.byte(chunk, 1) << 8) | string.byte(chunk, 2)
            local d = (string.byte(chunk, 3) << 8) | string.byte(chunk, 4)

            _Ert_state = (_Ert_state + 1) % segments
            return true, b, c, d
        end)
    end
end

function unregister_ert()
    if _Ert_oda_id ~= nil then
        ext.unregister_oda(_Ert_oda_id)
        _Ert_oda_id = nil
    end
end

function rds.ext.set_ert(ert)
    if #ert == 0 then
        userdata.set_offset(USERDATA_ERT_OFFSET, 128, "")
        userdata.set_offset(USERDATA_ERT_OFFSET+128, 1, string.char(0))
        userdata.set_offset(USERDATA_ERT_OFFSET+258, 1, string.char(1))
        return
    end

    local data = ert .. "\r"
    data = string.sub(data, 1, 128)

    local padding = (4 - (#data % 4)) % 4
    data = data .. string.rep("\0", padding)

    userdata.set_offset(USERDATA_ERT_OFFSET, 128, data)

    local segments = #data // 4
    if segments > 32 then segments = 32 end

    userdata.set_offset(USERDATA_ERT_OFFSET+128, 1, string.char(segments))

    if string.byte(userdata.get_offset(USERDATA_ERT_OFFSET+257, 1)) == 0 then
        init_ert()
        userdata.set_offset(USERDATA_ERT_OFFSET+129, 128, data)
        userdata.set_offset(USERDATA_ERT_OFFSET+257, 1, string.char(segments))
        _Ert_state = 0
    else userdata.set_offset(USERDATA_ERT_OFFSET+258, 1, string.char(1)) end

    if _Ert_oda_id == nil then init_ert() end
end

function rds.ext.get_ert()
    local segments = string.byte(userdata.get_offset(USERDATA_ERT_OFFSET+128, 1))
    if segments == 0 then return "" end

    local data = userdata.get_offset(USERDATA_ERT_OFFSET, 128)
    return data:match("^(.-)[\r%z]*") or ""
end

table.insert(on_states, function ()
    if string.byte(userdata.get_offset(USERDATA_ERT_OFFSET+257, 1)) ~= 0 then init_ert() end
end)