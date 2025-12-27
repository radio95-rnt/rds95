---@diagnostic disable: undefined-global
function on_state()
    if get_rds_rtp_meta(false) then init_rtp() end
    if get_rds_rtp_meta(true) then init_ertp() end
    if string.byte(get_userdata_offset(257, 1)) ~= 0 then init_ert() end
end