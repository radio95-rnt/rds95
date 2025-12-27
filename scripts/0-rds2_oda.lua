local RDS2_ODA = { aid = 0, data = 0, handler = false }

function RDS2_ODA.new(aid, data, handler)
    local instance = { aid = aid or 0, data = data or 0, handler = handler or false }
    setmetatable(instance, { __index = RDS2_ODA })
    return instance
end

local RDS2_ODAs = {}
local RDS2_ODA_aid = true
local RDS2_ODA_pointer = 1

---This function is defined externally
---@param aid integer
---@param data integer
---@return integer oda_id
function register_oda_rds2(aid, data)
    local oda = RDS2_ODA.new(aid, data, false)
    table.insert(RDS2_ODAs, oda)
    return #RDS2_ODAs
end

---This function is defined externally
---@param oda_id integer
---@param data integer
function set_oda_id_data_rds2(oda_id, data)
    if oda_id > #RDS2_ODAs then return end
    RDS2_ODAs[oda_id].data = data
end

---This function is defined externally
---@param oda_id integer
---@param func RDS2_ODAHandler
function set_oda_handler_rds2(oda_id, func)
    if oda_id > #RDS2_ODAs then return end
    RDS2_ODAs[oda_id].handler = func
end

function rds2_group(stream)
    if #RDS2_ODAs == 0 then return false, 0, 0, 0, 0 end
    if RDS2_ODA_pointer > #RDS2_ODAs then RDS2_ODA_pointer = 1 end
    local oda = RDS2_ODAs[RDS2_ODA_pointer]
    local channel = (RDS2_ODA_pointer & 0x40) << 8
    RDS2_ODA_pointer = RDS2_ODA_pointer + 1
    if RDS2_ODA_aid then
        -- TODO: add support for the multi aid thing (page 49)
        RDS2_ODA_aid = not RDS2_ODA_aid
        return true, 1 << 15 | channel, oda.aid, (oda.data & 0xffff0000) >> 16, (oda.data & 0xffff)
    else
        RDS2_ODA_aid = not RDS2_ODA_aid
        if oda.handler then
            local generated, a, b, c, d = oda.handler(stream)
            return generated, 1 << 14 | channel | a, b, c, d
        end
        return true, 1 << 15 | channel, oda.aid, (oda.data & 0xffff0000) >> 16, (oda.data & 0xffff)
    end
end