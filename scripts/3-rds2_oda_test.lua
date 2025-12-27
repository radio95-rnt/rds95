local _rds2_test_counter = 0

local function rds2_oda_test()
    local id = register_oda_rds2(0xfffe, 0xfffff, false)
    set_oda_handler_rds2(id, function ()
        _rds2_test_counter = _rds2_test_counter + 1
        if _rds2_test_counter > 0xffffffffffff then _rds2_test_counter = 0 end
        return true, 16, (_rds2_test_counter & 0xffff00000000) >> 32, (_rds2_test_counter & 0xffff0000) >> 16, _rds2_test_counter & 0xffff
    end)
end