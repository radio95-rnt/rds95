_Rtp_oda_id = nil
_Ertp_oda_id = nil
_Rtp_toggle = true
_Ertp_toggle = true

local function init_rtp()
    if _Rtp_oda_id == nil then
        _Rtp_oda_id = register_oda(11, false, 0x4BD7, 0)
        set_oda_handler(_Rtp_oda_id, function ()
            local b = (_Rtp_toggle and 1 or 0) << 4 | string.byte(get_userdata_offset(259, 1)) << 3
            local data_0 = get_userdata_offset(260, 3)
            local data_1 = get_userdata_offset(263, 3)
            b = b | (string.byte(data_0, 1) & 0xf8) >> 3

            local c = (string.byte(data_0, 1) & 0x7) << 13
            c = c | (string.byte(data_0, 2) & 0x3f) << 7
            c = c | (string.byte(data_0, 3) & 0x3f) << 1
            c = c | (string.byte(data_1, 1) & 0xe0) >> 5

            local d = (string.byte(data_1, 1) & 0x1f) << 11
            d = d | (string.byte(data_1, 2) & 0x3f) << 5
            d = d | (string.byte(data_1, 3) & 0x1f)

            return b, c, d
        end)
    end
end

local function init_ertp()
    if _Ertp_oda_id == nil then
        _Ertp_oda_id = register_oda(12, false, 0x4BD8, 0)
        set_oda_handler(_Ertp_oda_id, function ()
            local b = (_Ertp_toggle and 1 or 0) << 4 | string.byte(get_userdata_offset(266, 1)) << 3
            local data_0 = get_userdata_offset(267, 3)
            local data_1 = get_userdata_offset(270, 3)
            b = b | (string.byte(data_0, 1) & 0xf8) >> 3

            local c = (string.byte(data_0, 1) & 0x7) << 13
            c = c | (string.byte(data_0, 2) & 0x3f) << 7
            c = c | (string.byte(data_0, 3) & 0x3f) << 1
            c = c | (string.byte(data_1, 1) & 0xe0) >> 5

            local d = (string.byte(data_1, 1) & 0x1f) << 11
            d = d | (string.byte(data_1, 2) & 0x3f) << 5
            d = d | (string.byte(data_1, 3) & 0x1f)

            return b, c, d
        end)
    end
end

function set_rds_rtp_meta(ertp, running)
    if ertp then
        if running and _Ertp_oda_id == nil then init_ertp() end
        set_userdata_offset(266, 1, string.char(running and 1 or 0))
    else
        if running and _Rtp_oda_id == nil then init_rtp() end
        set_userdata_offset(259, 1, string.char(running and 1 or 0))
    end
end
function get_rds_rtp_meta(ertp)
    local offset = ertp and 266 or 259
    return string.byte(get_userdata_offset(offset, 1)) ~= 0
end
function toggle_rds_rtp(ertp)
    if ertp then _Ertp_toggle = not _Ertp_toggle
    else _Rtp_toggle = not _Rtp_toggle end
end

function set_rds_rtplus_tags(ertp, t1, s1, l1, t2, s2, l2)
    set_userdata_offset(ertp and 267 or 260, 6, string.char(t1, s1, l1, t2, s2, l2))
end
function get_rds_rtplus_tags(ertp)
    return string.byte(get_userdata_offset(ertp and 267 or 260, 6), 1, 6)
end

function on_state()
    if get_rds_rtp_meta(false) then init_rtp() end
    if get_rds_rtp_meta(true) then init_ertp() end
end