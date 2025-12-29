_RDS2_ODA = { aid = 0, data = 0, handler = false, file_related = false }

function _RDS2_ODA.new(aid, data, handler, file_related)
    local instance = { aid = aid or 0, data = data or 0, handler = handler or false, file_related = file_related or false }
    setmetatable(instance, { __index = _RDS2_ODA })
    return instance
end

_RDS2_ODAs = {}
_RDS2_ODA_aid = 0
_RDS2_ODA_pointer = 1

---This function is defined externally
---@param aid integer
---@param data integer
---@param file_related boolean
---@return integer oda_id
function register_oda_rds2(aid, data, file_related)
    local oda = _RDS2_ODA.new(aid, data, false, file_related)
    for i = 1, #_RDS2_ODAs do
        if _RDS2_ODAs[i] == false then
            _RDS2_ODAs[i] = oda
            return i
        end
    end
    table.insert(_RDS2_ODAs, oda)
    return #_RDS2_ODAs
end

---Unregisters an RDS 2 ODA, this stops the handler or AID being called/sent
---@param oda_id integer
function unregister_oda_rds2(oda_id)
    if oda_id < 1 or oda_id > #_RDS2_ODAs or _RDS2_ODAs[oda_id] == false then error("Invalid ODA ID: " .. tostring(oda_id), 2) end

    _RDS2_ODAs[oda_id] = false

    if _RDS2_ODA_pointer == oda_id then _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1 end
end


---This function is defined externally
---@param oda_id integer
---@param data integer
function set_oda_id_data_rds2(oda_id, data)
    if oda_id < 1 or oda_id > #_RDS2_ODAs or _RDS2_ODAs[oda_id] == false then error("Invalid ODA ID: " .. tostring(oda_id), 2) end
    _RDS2_ODAs[oda_id].data = data
end

---This function is defined externally
---@param oda_id integer
---@param func RDS2_ODAHandler
function set_oda_handler_rds2(oda_id, func)
    if oda_id < 1 or oda_id > #_RDS2_ODAs or _RDS2_ODAs[oda_id] == false then error("Invalid ODA ID: " .. tostring(oda_id), 2) end
    _RDS2_ODAs[oda_id].handler = func
end

function rds2_group(stream)
    if #_RDS2_ODAs == 0 then return false, 0, 0, 0, 0 end

    local checked = 0
    while checked < #_RDS2_ODAs and _RDS2_ODAs[_RDS2_ODA_pointer] == false do
        _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1
        if _RDS2_ODA_pointer > #_RDS2_ODAs then _RDS2_ODA_pointer = 1 end
        checked = checked + 1
    end

    if checked == #_RDS2_ODAs then return false, 0, 0, 0, 0 end

    if _RDS2_ODA_pointer > #_RDS2_ODAs then _RDS2_ODA_pointer = 1 end

    local oda = _RDS2_ODAs[_RDS2_ODA_pointer]
    local channel_offset = 16 * ((not oda.file_related) and 1 or 0)
    local channel = ((_RDS2_ODA_pointer - 1 + channel_offset) & 0x3F)
    if oda.file_related then channel = channel & 0xF end

    _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1

    local next_oda = nil
    if _RDS2_ODA_pointer <= #_RDS2_ODAs then next_oda = _RDS2_ODAs[_RDS2_ODA_pointer] end

    if _RDS2_ODA_aid == 0 and stream == 1 then
        _RDS2_ODA_aid = 1
        local block1_base = (2 << 14) | channel

        if next_oda and oda.data > 0 and oda.data <= 0xFFFF and next_oda.data == 0 then
            _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1
            return true, block1_base | (1 << 6), oda.aid, oda.data, next_oda.aid
        elseif next_oda and oda.data == 0 and next_oda.data > 0 and next_oda.data <= 0xFFFF then
            _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1
            return true, block1_base | (2 << 6), oda.aid, next_oda.aid, next_oda.data
        elseif next_oda ~= nil and oda.data == 0 and next_oda.data == 0 then
            _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1
            if _RDS2_ODA_pointer <= #_RDS2_ODAs then
                local third_oda = _RDS2_ODAs[_RDS2_ODA_pointer]
                _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1
                return true, block1_base | (3 << 6), oda.aid, next_oda.aid, third_oda.aid
            else return true, block1_base, oda.aid, (oda.data >> 16) & 0xffff, oda.data & 0xffff end
        end

        return true, block1_base, oda.aid, (oda.data >> 16) & 0xffff, oda.data & 0xffff
    else
        _RDS2_ODA_aid = _RDS2_ODA_aid + 1
        if _RDS2_ODA_aid > 8 then _RDS2_ODA_aid = 0 end
        if oda.handler then
            local generated = false
            checked = 0
            while generated == false and checked < #_RDS2_ODAs do
                local generated, a, b, c, d = oda.handler(stream)
                if not generated then
                    _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1
                    if _RDS2_ODA_pointer > #_RDS2_ODAs then _RDS2_ODA_pointer = 1 end
                    oda = _RDS2_ODAs[_RDS2_ODA_pointer]
                    checked = checked + 1
                else
                    local channel_bitshift = 8
                    local fid = (a & 0xC000) >> 14
                    local fn_msb = (a >> 13) & 1
                    if fid == 0 and fn_msb == 0 then
                        warn("RDS2 ODA is tunneling (A or B) over C")
                        return true, 0, b, c, d -- Tunnel, not sure why but sure
                    --FID = 1 means a normal ODA group
                    --FID = 2 means a RFT file
                    elseif fid == 2 and fn_msb == 0 then channel_bitshift = 0 end -- This is AID
                    return generated, (channel << channel_bitshift) | a, b, c, d
                end
            end
            if checked == #_RDS2_ODAs then return false, 0, 0, 0, 0 end
        end
        return true, (2 << 14) | channel, oda.aid, (oda.data >> 16) & 0xffff, oda.data & 0xffff
    end
end

local _old_on_state_oda_rds2 = on_state
function on_state()
    _RDS2_ODAs = {}
    _RDS2_ODA_aid = 0
    _RDS2_ODA_pointer = 1
    if type(_old_on_state_oda_rds2) == "function" then _old_on_state_oda_rds2() end
end