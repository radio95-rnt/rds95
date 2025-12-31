---@class RftInstance
---@field oda_id integer|nil The internal ODA registration ID
---@field file_data string The raw binary content of the file
---@field crc_data string Calculated CRC chunks for Variant 1 groups
---@field file_segment integer Current segment index being sent
---@field crc_segment integer Current CRC byte index being sent
---@field toggle boolean The RDS2 toggle bit (flips when file content changes)
---@field last_id integer The file ID used in the previous transmission
---@field version integer The file version (0-7)
---@field crc_enabled boolean Whether CRC protection is active
---@field crc_full_file integer The CRC16 of the entire file (for mode 0)
---@field crc_mode integer The RDS2 RFT CRC mode (0-5)
---@field crc_sent_this_cycle boolean Internal flag to interleave CRCs with data
---@field aid integer The Application Identification (e.g., 0xFF7F for Logo)
---@field send_once boolean If true, unregisters after one full transmission
---@field paused boolean If true, the ODA handler will return empty groups
RftInstance = {}
RftInstance.__index = RftInstance

--- Creates a new RFT Manager instance.
---@return RftInstance
function RftInstance.new()
    local self = setmetatable({}, RftInstance)

    self.oda_id = nil
    self.file_data = ""
    self.crc_data = ""
    self.file_segment = 0
    self.crc_segment = 1
    self.toggle = false
    self.last_id = -1
    self.version = 0
    self.crc_enabled = false
    self.crc_full_file = 0
    self.crc_mode = 0
    self.crc_sent_this_cycle = false
    self.aid = 0
    self.send_once = false
    self.paused = false

    return self
end

function RftInstance:stop()
    if self.oda_id ~= nil and self.aid ~= 0 then
        unregister_oda_rds2(self.oda_id)
        self.oda_id = nil
    end

    self.file_data = ""
    self.crc_data = ""
    self.file_segment = 0
    self.crc_segment = 1
    self.paused = false
end

--- Internal method to start the ODA handler logic.
--- Processes RDS2 RFT Variants 0, 1 and Data Groups.
---@private
function RftInstance:start()
    if self.oda_id == nil and self.aid ~= 0 then
        self.oda_id = register_oda_rds2(self.aid, 0, true)

        set_oda_handler_rds2(self.oda_id, function(stream)
            if #self.file_data == 0 or self.paused then
                return false, 0, 0, 0, 0
            end

            local total_segments = math.ceil(#self.file_data / 5)

            local last_stream = false
            if stream == (math.min(get_available_rds_streams(), get_rds_streams()) - 1) then last_stream = true end

            if not self.crc_sent_this_cycle and self.crc_enabled and (self.file_segment % 16 == 0) and (not last_stream) then
                self.crc_sent_this_cycle = true
                local chunk_address = math.floor((self.crc_segment - 1) / 2)
                local c = (1 << 12) | (self.crc_mode & 7) << 9 | (chunk_address & 0x1ff)

                local high_byte, low_byte
                if self.crc_mode ~= 0 then
                    high_byte = string.byte(self.crc_data, self.crc_segment) or 0
                    low_byte = string.byte(self.crc_data, self.crc_segment + 1) or 0
                else
                    high_byte = self.crc_full_file >> 8
                    low_byte = self.crc_full_file & 0xff
                end

                self.crc_segment = self.crc_segment + 2
                if self.crc_segment > #self.crc_data then self.crc_segment = 1 end

                return true, (2 << 14), self.aid, c, (high_byte << 8) | low_byte
            else self.crc_sent_this_cycle = false end

            local base = self.file_segment * 5 + 1
            local function b(i) return string.byte(self.file_data, base + i) or 0 end

            local word1 = (((self.toggle and 1 or 0) << 7) | ((self.file_segment >> 8) & 0x7F))
            local word2 = ((self.file_segment & 0xFF) << 8) | b(0)
            local word3 = (b(1) << 8) | b(2)
            local word4 = (b(3) << 8) | b(4)

            self.file_segment = self.file_segment + 1
            if self.file_segment >= total_segments then
                self.file_segment = 0
                if self.send_once then self:stop() end
            end

            return true, (2 << 12) | word1, word2, word3, word4
        end)
    end
end

--- Loads a file and begins RDS2 transmission.
---@param aid integer Application ID (e.g. 0xFF7F for Station Logo)
---@param path string System path to the file
---@param id integer File ID (0-63). Use the same ID to update a file, different to trigger a reset.
---@param crc integer|boolean CRC Mode (0: Full, 1-5: Chunks, true/7: Auto)
---@param once boolean If true, file is sent once and the stream is closed
---@return boolean interrupted Returns true if a previous file was already being sent
function RftInstance:sendFile(aid, path, id, crc, once)
    local interrupted = (#self.file_data ~= 0)

    if self.aid ~= aid then self:stop() end
    self.aid = aid

    local file = io.open(path, "rb")
    if not file then error("Could not open file: " .. path) end
    self.file_data = file:read("*a")
    file:close()

    self.send_once = once

    if id == self.last_id then
        self.toggle = not self.toggle
        self.version = (self.version + 1) % 8
    end

    self.crc_data = ""
    self.crc_enabled = (crc ~= false)

    local f_size = #self.file_data
    if crc == 0 then
        self.crc_mode = 0
        self.crc_full_file = crc16(self.file_data)
    elseif crc == true or crc == 7 then
        if f_size <= 40960 then self.crc_mode = 1
        elseif f_size > 40960 and f_size <= 81920 then self.crc_mode = 2
        else self.crc_mode = 3 end
    else
        self.crc_mode = crc and 1 or 0
    end

    local chunk_multipliers = {16, 32, 64, 128, 256}
    local multiplier = chunk_multipliers[self.crc_mode]
    if self.crc_enabled and multiplier then
        local chunk_size = 5 * multiplier
        for i = 1, f_size, chunk_size do
            local chunk = string.sub(self.file_data, i, i + chunk_size - 1)
            local v = crc16(chunk)
            self.crc_data = self.crc_data .. string.char(v >> 8, v & 0xff)
        end
    end

    if f_size > (0x3ffff * 5) then error("File too large") end
    if self.oda_id == nil then self:start() end

    set_oda_id_data_rds2(self.oda_id, f_size | (id & 63) << 18 | (self.version & 7) << 24 | (self.crc_enabled and 1 or 0) << 27)
    self.last_id = id
    self.paused = false
    return interrupted
end