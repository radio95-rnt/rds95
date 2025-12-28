_Af_Oda_id = nil
_Af_Oda_state = 0
_Af_Oda_len = 0
_Af_Oda_afs = {}

local USERDATA_OFFSET = 270

---@param freq number
---@return table
local function encode_af(freq)
    local out = {}
    if(freq >= 87.6 and freq <= 107.9) then
        table.insert(out, math.tointeger((freq * 10) - 875))
    elseif freq >= 64.1 and freq <= 88.0 then
        table.insert(out, math.tointeger((freq * 10) - 384))
    elseif freq >= 153 and freq <= 279 then
        table.insert(out, 250) -- LFMF incoming
        table.insert(out, math.tointeger((freq - 153) / 9 + 1))
    elseif freq >= 531 and freq <= 1602 then
        table.insert(out, 250) -- LFMF incoming
        table.insert(out, math.tointeger((freq - 531) / 9 + 16))
    end
    return out
end

local function get_next_af_oda_group()
    local af = {0, 0, 0, 0}
    local num_entries = #_Af_Oda_afs

    local i = 1
    if _Af_Oda_state == 0 then
        af[1] = 224 + math.min(_Af_Oda_len, 25)
        _Af_Oda_state = 1
        i = 2
    end

    while i <= 4 do
        if _Af_Oda_state <= num_entries then
            af[i] = _Af_Oda_afs[_Af_Oda_state]
            _Af_Oda_state = _Af_Oda_state + 1
        else af[i] = 205 end
        i = i + 1
    end

    if _Af_Oda_state > num_entries then _Af_Oda_state = 0 end

    local b = (7 << 12)
    for j = 1, 4 do
        local bit8 = (math.tointeger(af[j]) >> 8) & 0x01
        b = b | (bit8 << (j - 1))
    end

    return b, ((math.tointeger(af[1]) & 0xFF) << 8) | (math.tointeger(af[2]) & 0xFF), ((math.tointeger(af[3]) & 0xFF) << 8) | (math.tointeger(af[4]) & 0xFF)
end

local function init_af_oda()
    if _Af_Oda_id == nil then
        _Af_Oda_id = register_oda(7, false, 0x6365, 0)
        set_oda_handler(_Af_Oda_id, function()
            local b, c, d = get_next_af_oda_group()
            return true, b, c, d
        end)
    end
end

local function save_af_to_userdata(afs)
    local count = #afs
    if count > 25 then count = 25 end

    local payload = string.pack("B", count)
    for i = 1, count do payload = payload .. string.pack("f", afs[i]) end
    set_userdata_offset(USERDATA_OFFSET, #payload, payload)
end

local function _process_af_list(afs)
    _Af_Oda_afs = {}
    _Af_Oda_len = #afs
    for _, f in ipairs(afs) do
        local codes = encode_af(f)
        for _, code in ipairs(codes) do
            table.insert(_Af_Oda_afs, code)
        end
    end
    _Af_Oda_state = 0
    init_af_oda()
end

local function load_af_from_userdata()
    local header = get_userdata_offset(USERDATA_OFFSET, 1)
    if header == "" or header == nil then return end

    local count = string.unpack("B", header)
    if count == 0 or count > 25 then return end

    local data = get_userdata_offset(USERDATA_OFFSET + 1, count * 4)
    if #data < (count * 4) then return end

    local afs = {}
    for i = 1, count do
        local val = string.unpack("f", data, (i-1)*4 + 1)
        table.insert(afs, val)
    end

    _process_af_list(afs)
end

---Sets the AFs included in the ODA and saves them
---@param afs table List of numbers (e.g., {98.1, 102.5})
function set_rds_af_oda(afs)
    _process_af_list(afs)
    save_af_to_userdata(afs)
end

local _old_on_state_af = on_state
function on_state()
    load_af_from_userdata()
    if _Af_Oda_len ~= 0 then init_af_oda() end
    if type(_old_on_state_af) == "function" then _old_on_state_af() end
end