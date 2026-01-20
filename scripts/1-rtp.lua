_Rtp_oda_id = nil
_Ertp_oda_id = nil
_Rtp_toggle = false
_Ertp_toggle = false

--- Size: 15 bytes
local USERDATA_RTP_OFFSET = 259

local function init_rtp()
    if _Rtp_oda_id == nil then
        _Rtp_oda_id = ext.register_oda(11, false, 0x4BD7, 0)
        ext.set_oda_handler(_Rtp_oda_id, function ()
            local b = (_Rtp_toggle and 1 or 0) << 4 | string.byte(userdata.get_offset(USERDATA_RTP_OFFSET, 1)) << 3
            local data_0 = userdata.get_offset(USERDATA_RTP_OFFSET+1, 3)
            local data_1 = userdata.get_offset(USERDATA_RTP_OFFSET+4, 3)
            b = b | (string.byte(data_0, 1) & 0xf8) >> 3

            local c = (string.byte(data_0, 1) & 0x7) << 13
            c = c | (string.byte(data_0, 2) & 0x3f) << 7
            c = c | (string.byte(data_0, 3) & 0x3f) << 1
            c = c | (string.byte(data_1, 1) & 0xe0) >> 5

            local d = (string.byte(data_1, 1) & 0x1f) << 11
            d = d | (string.byte(data_1, 2) & 0x3f) << 5
            d = d | (string.byte(data_1, 3) & 0x1f)

            return true, b, c, d
        end)
    end
end

local function init_ertp()
    if _Ertp_oda_id == nil then
        _Ertp_oda_id = ext.register_oda(12, false, 0x4BD8, 0)
        ext.set_oda_handler(_Ertp_oda_id, function ()
            local b = (_Ertp_toggle and 1 or 0) << 4 | string.byte(userdata.get_offset(USERDATA_RTP_OFFSET+7, 1)) << 3
            local data_0 = userdata.get_offset(USERDATA_RTP_OFFSET+8, 3)
            local data_1 = userdata.get_offset(USERDATA_RTP_OFFSET+11, 3)
            b = b | (string.byte(data_0, 1) & 0xf8) >> 3

            local c = (string.byte(data_0, 1) & 0x7) << 13
            c = c | (string.byte(data_0, 2) & 0x3f) << 7
            c = c | (string.byte(data_0, 3) & 0x3f) << 1
            c = c | (string.byte(data_1, 1) & 0xe0) >> 5

            local d = (string.byte(data_1, 1) & 0x1f) << 11
            d = d | (string.byte(data_1, 2) & 0x3f) << 5
            d = d | (string.byte(data_1, 3) & 0x1f)

            return true, b, c, d
        end)
    end
end

function rds.ext.set_rtp_meta(ertp, running)
    if ertp then
        if running and _Ertp_oda_id == nil then init_ertp() end
        userdata.set_offset(USERDATA_RTP_OFFSET+7, 1, string.char(running and 1 or 0))
    else
        if running and _Rtp_oda_id == nil then init_rtp() end
        userdata.set_offset(USERDATA_RTP_OFFSET, 1, string.char(running and 1 or 0))
    end
end
function rds.ext.get_rtp_meta(ertp)
    local offset = ertp and (USERDATA_RTP_OFFSET+7) or USERDATA_RTP_OFFSET
    return string.byte(userdata.get_offset(offset, 1)) ~= 0
end
function rds.ext.toggle_rtp(ertp)
    if ertp then _Ertp_toggle = not _Ertp_toggle
    else _Rtp_toggle = not _Rtp_toggle end
end

function rds.ext.set_rtplus_tags(ertp, t1, s1, l1, t2, s2, l2)
    rds.ext.set_rds_rtp_meta(ertp, true)
    rds.ext.toggle_rds_rtp(ertp)
    userdata.set_offset(ertp and (USERDATA_RTP_OFFSET+8) or (USERDATA_RTP_OFFSET+1), 6, string.char(t1, s1, l1, t2, s2, l2))
end
function rds.ext.get_rtplus_tags(ertp)
    return string.byte(userdata.get_offset(ertp and (USERDATA_RTP_OFFSET+8) or (USERDATA_RTP_OFFSET+1), 6), 1, 6)
end

function unregister_rtp(ertp)
    if ertp and _Ertp_oda_id ~= nil then
        ext.unregister_oda(_Ertp_oda_id)
        _Ertp_oda_id = nil
    elseif _Rtp_oda_id ~= nil then
        ext.unregister_oda(_Rtp_oda_id)
        _Rtp_oda_id = nil
    end
end

table.insert(on_states, function ()
    if rds.ext.get_rtp_meta(false) then init_rtp() end
    if rds.ext.get_rtp_meta(true) then init_ertp() end
end)