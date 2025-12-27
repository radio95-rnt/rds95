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
    table.insert(_RDS2_ODAs, oda)
    return #_RDS2_ODAs
end

---This function is defined externally
---@param oda_id integer
---@param data integer
function set_oda_id_data_rds2(oda_id, data)
    if oda_id > #_RDS2_ODAs then return end
    _RDS2_ODAs[oda_id].data = data
end

---This function is defined externally
---@param oda_id integer
---@param func RDS2_ODAHandler
function set_oda_handler_rds2(oda_id, func)
    if oda_id > #_RDS2_ODAs then return end
    _RDS2_ODAs[oda_id].handler = func
end

function rds2_group(stream)
    if #_RDS2_ODAs == 0 then return false, 0, 0, 0, 0 end
    if _RDS2_ODA_pointer > #_RDS2_ODAs then _RDS2_ODA_pointer = 1 end

    local oda = _RDS2_ODAs[_RDS2_ODA_pointer]
    local channel_offset = 16 * ((not oda.file_related) and 1 or 0)
    local channel = ((_RDS2_ODA_pointer - 1 + channel_offset) & 0x3F)

    _RDS2_ODA_pointer = _RDS2_ODA_pointer + 1

    local next_oda = nil
    if _RDS2_ODA_pointer <= #_RDS2_ODAs then next_oda = _RDS2_ODAs[_RDS2_ODA_pointer] end

    if _RDS2_ODA_aid == 0 then
        _RDS2_ODA_aid = 1
        local block1_base = (2 << 14) | (0 << 8) | channel

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
        if _RDS2_ODA_aid > 2 then _RDS2_ODA_aid = 0 end
        if oda.handler then
            local generated, a, b, c, d = oda.handler(stream)
            return generated, (1 << 14) | (channel << 8) | a, b, c, d
        end
        return true, (1 << 15) | channel, oda.aid, (oda.data >> 16) & 0xffff, oda.data & 0xffff
    end
end