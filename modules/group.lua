local md = {}
md.registered_groups = {}

hooks.group[#hooks.group+1] = function (group)
    if md.registered_groups[group] then
        local ok, generated, b, c, d = pcall(md.registered_groups[group], group)
        if ok then return generated, b, c, d
        else
            print(generated)
            return false, 0, 0, 0
        end
    end
    return false, 0, 0, 0
end

---@param designator string
---@param handler function
function md.register_group(designator, handler)
    for i = 1, #designator do
        local char = designator:sub(i, i)
        md.registered_groups[char] = handler
    end
end

return md