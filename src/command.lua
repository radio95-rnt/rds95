cmd_output = ""

if type(cmd) == "string" then
    if cmd:sub(1, 3) == "PI=" then
        local hex = cmd:sub(7)
        local pi = tonumber(hex, 16)

        if pi then
            set_rds_pi(pi)
            cmd_output = string.format("+", pi)
        else
            cmd_output = "Invalid hex PI"
        end
    end
end