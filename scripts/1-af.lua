_Af_Oda_id = nil
_Af_Oda_state = 0
_Af_Oda_len = 0
_Af_Oda_afs = {}

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
        table.insert(out, (freq - 153) / 9 + 1)
    elseif freq >= 531 and freq <= 1602 then
        table.insert(out, 250) -- LFMF incoming
        table.insert(out, (freq - 531) / 9 + 16)
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

---Sets the AFs included in the ODA
---@param afs table List of numbers (e.g., {98.1, 102.5})
function set_rds_af_oda(afs)
    _Af_Oda_afs = {}
    _Af_Oda_len = #afs
    for _, f in ipairs(afs) do
        local codes = encode_af(f)
        for _, code in ipairs(codes) do
            table.insert(_Af_Oda_afs, code)
        end
    end
    _Af_Oda_state = 0
    if _Af_Oda_id == nil then init_af_oda() end
end

function on_state()
    init_af_oda()
end