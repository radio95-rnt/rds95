hooks.registered_groups = {}

function hooks.group(group)
    if hooks.registered_groups[group] then
        local ok, generated, b, c, d = pcall(hooks.registered_groups[group], group)
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
function rds.ext.register_group(designator, handler)
    for i = 1, #designator do
        local char = designator:sub(i, i)
        hooks.registered_groups[char] = handler
    end
end