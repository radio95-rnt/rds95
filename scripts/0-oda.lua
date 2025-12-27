local ODA = { group = 0, group_version = false, aid = 0, data = 0, handler = false }

function ODA.new(group, group_version, aid, data, handler)
    local instance = { group = group or 0, group_version = group_version or false, aid = aid or 0, data = data or 0, handler = handler or false }
    setmetatable(instance, { __index = ODA })
    return instance
end

local RDS_ODAs = {}
local RDS_ODA_pointer = 1

---Registers an ODA to be used in the O of the group sequence. ODAs are stored as state data, thus running reset_rds will clear it
---Groups 14, 15, 2, 0 cannot be registered either version, groups 10, 4, 1 can be only registered as B, any other is free to take
---Group 3A will mean that there will be no group handler for this ODA, meaning it can only be interacted with via the 3A AID group, handler set is not possible with such groups
---@param group integer
---@param group_version boolean
---@param aid integer
---@param data integer
---@return integer oda_id
function register_oda(group, group_version, aid, data)
    if group == 14 or group == 15 or group == 2 or group == 0 then error("Group is incorrect", 2) end
    if (group == 10 or group == 4 or group == 1) and group_version then error("Group is incorrect", 2) end
    local oda = ODA.new(group, group_version, aid, data, false)
    table.insert(RDS_ODAs, oda)
    return #RDS_ODAs
end

---Sets the id_data for a existing ODA group
---@param oda_id integer
---@param data integer
function set_oda_id_data(oda_id, data)
    if oda_id > #RDS_ODAs then return end
    RDS_ODAs[oda_id].data = data
end

---Sets a function to handle the ODA data generation. 
---The handler is called when the group sequence 'K' slot is processed.
---The function must return 3 integers representing RDS Blocks B, C, and D.
---Please note that you do not need to compute the block A to indentify the group and group version, that will be done for you and EVERY SINGLE group has PTY and TP inserted (and also PI if its a B)
---You are asked to set groups B last 5 bits, leave rest 0
---@param oda_id integer The ID returned by register_oda
---@param fun ODAHandler
function set_oda_handler(oda_id, fun)
    if oda_id > #RDS_ODAs then return end
    RDS_ODAs[oda_id].handler = fun
end

local function get_aid()
    local oda = RDS_ODAs[RDS_ODA_pointer]
    local b = 3 << 12 | oda.group << 1 | (oda.group_version and 1 or 0)
    local data, aid = oda.data, oda.aid

    RDS_ODA_pointer = (RDS_ODA_pointer % #RDS_ODAs) + 1

    return b, data, aid
end

local function get_data()
    local checked_count = 0
    local total_odas = #RDS_ODAs

    while checked_count < total_odas do
        local oda = RDS_ODAs[RDS_ODA_pointer]

        if type(oda.handler) == "function" then
            local generated, b, c, d = oda.handler()
            RDS_ODA_pointer = (RDS_ODA_pointer % #RDS_ODAs) + 1
            b = b | oda.group << 12
            b = b | (oda.group_version and 1 or 0) << 11
            return generated, b, c, d
        end

        RDS_ODA_pointer = (RDS_ODA_pointer % #RDS_ODAs) + 1
        checked_count = checked_count + 1
    end

    return false, 0, 0, 0
end

function group(group_type)
    if group_type == "O" or group_type == "K" then
        if #RDS_ODAs == 0 then return false, 0, 0, 0 end
        if RDS_ODA_pointer > #RDS_ODAs or RDS_ODA_pointer < 1 then RDS_ODA_pointer = 1 end

        if group_type == "O" then
            local b, c, d = get_aid()
            return true, b, c, d
        elseif group_type == "K" then
            return get_data()
        end
    end
    return false, 0, 0, 0
end