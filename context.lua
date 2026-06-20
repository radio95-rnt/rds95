---@meta

---@class Data
Data = {}

---@type string
Data.core_version = ""

---@type integer
Data.max_programs = 0

---Executes a CRC-16 CCIIT on given data
---@param data string
---@return integer
function Data.crc16(data) end

---Starts the initialization sequence, also calls the on_init function
---@return nil
function Data.set_program_defaults() end

---Saves, loads and resets the state of the data, you might as well restart the whole program. this does NOT cause data loss other than the current internal state
---@return nil
function Data.reset_rds() end

---Forces encoder and modulator data to be saved to disc
---@return nil
function Data.force_save() end

---Set the program which is actually being modulated. Setting this does not affect the writing program
---@param program_idx integer 0 to (max_programs - 1)
function Data.set_output_program(program_idx) end
---@return integer
function Data.get_output_program() end

---Sets the program all of the set/get functions affect
---@param program_idx integer 0 to (max_programs - 1)
function Data.set_writing_program(program_idx) end
---@return integer
function Data.get_writing_program() end

---Encodes the given UTF-8 string into the unnamed RDS character set
---@param data string
---@return string
function Data.encode_charset(data) end

hooks = {}

---This function is called by the C core after we reset data, or have no data in general
---It should be defined by the user in the script.
---This is a table of functions. Each will be called
---@type function[]
hooks.on_init = {}

---This function is called by the C core after we initialize the encoder (always, every start)
---It should be defined by the user in the script.
---This is a table of functions. Each will be called
---@type function[]
hooks.on_start = {}

---This function is called every time when the state resets, register your odas here
---It should be defined by the user in the script.
---This is a table of functions. Each will be called
---@type function[]
hooks.on_state = {}

---This function is called every second (or rather if the second has changed since the last stream 0 group)
---It should be defined by the user in the script.
---This is a table of functions. Each will be called
---@type function[]
hooks.tick = {}

---This function is called every minute (or rather if the minute has changed since the last stream 0 group)
---It should be defined by the user in the script.
---This is a table of functions. Each will be called
---@type function[]
hooks.minute_tick = {}

---This function is called every time a RT transmits start to end - Useful for UECP's RT buffer shenanigans
---It should be defined by the user in the script.
---This is a table of functions. Each will be called
---@type function[]
hooks.rt_transmission = {}

---This function is calld every time a PS transmits start to end
---It should be defined by the user in the script.
---This is a table of functions. Each will be called
---@type function[]
hooks.ps_transmission = {}

---This function is called in order to handle UDP data. The string returned is sent back to the UDP peer as a response
---It should be defined by the user in the script.
---@param data string
---@return string
function hooks.data_handle(data) end

---@alias group_handler fun(group: string): boolean, integer, integer, integer

---Called when an unknown group is seen in the group sequence. See set_grpseq.
---This is also called on recognized groups if they can't be properly encoded by the core.
---The core always fills in PTY, TP, and PI in C if this is a B group.
---Should be defined by the user in the script.
---@param group string
---@return boolean generated
---@return integer b
---@return integer c
---@return integer d
local function group_handler(group) end

---@type group_handler[]
hooks.group = {}

---This function is called when an RDS2 group. Full control of every RDS2 subcarrier is under this function
---If a was returned 0, PTY and TP will be filled in, along with the PI code in C if needed
---If generated is false, then the group will be replaced with a tunnel of the last RDS1 group
---It should be defined by the user in the script.
---@param stream integer
---@return boolean generated
---@return integer a
---@return integer b
---@return integer c
---@return integer d
function hooks.rds2_group(stream) end

---@class RDS
RDS = {}

---Returns an encoded group by the core - groups that could not be encoded decay to PS
---@param group string One character, just one
---@return boolean generated Was this properly generated, or is the PS?
---@return integer b
---@return integer c
---@return integer d
function RDS.encode_group(group) end

---@type integer
RDS.eon_count = 0

---@type integer
RDS.pi = nil
---@type integer
RDS.ecc = nil
---@type integer
RDS.pty = nil
---@type integer
RDS.slc_data = nil
---@type boolean
RDS.ct = nil
---@type boolean
RDS.dpty = nil
---@type boolean
RDS.tp = nil
---@type boolean
RDS.ta = nil
---@type boolean
RDS.rt_enabled = nil
---@type boolean
RDS.ptyn_enabled = nil

-- Feature Flags
-- Modulation & Generation
---@param streams integer
function RDS.set_streams(streams) end
---@return integer
function RDS.get_streams() end

-- Program & Linking
---@param linkage boolean
function RDS.set_link(linkage) end
---@return boolean
function RDS.get_link() end

-- String Setters
---@param ptyn string Program Type Name (max 8 chars)
function RDS.set_ptyn(ptyn) end
---@param ptyn string Program Type Name (max 8 chars) which is expected to be already encoded in the RDS charset
function RDS.set_ptyn_raw(ptyn) end

---@param ps string Program Service (8 chars)
function RDS.set_ps(ps) end
---@param ps string Program Service (8 chars) which is expected to be already encoded in the RDS charset
function RDS.set_ps_raw(ps) end

---@param tps string Traffic PS
function RDS.set_tps(tps) end
---@param tps string Traffic PS which is expected to be already encoded in the RDS charset
function RDS.set_tps_raw(tps) end

---@param rt string Radio Text (max 64 chars)
function RDS.set_rt(rt) end
---@param rt string Radio Text (max 64 chars) which is expected to be already encoded in the RDS charset
function RDS.set_rt_raw(rt) end

---@return nil
function RDS.toggle_rt_ab() end

---@param lps string
function RDS.set_lps(lps) end
---@return string
function RDS.get_lps() end

---The format is a binary format: here are the groups recognized by the core
---0x0 - PS
---0x2 - SLC/ECC
---0x4 - RT
---0x14 - PTYN
---0x1C - EON
---0x1E - LPS
---0x1F - Fast tuning
---Rest are handled by the hooks.group function
---The designation sent to hooks.group is 100% binary safe
---This format is implemented, for an easier UECP inplementation
---@param grpseq string
function RDS.set_grpseq(grpseq) end
---@return string
function RDS.get_grpseq() end

---Puts in a RDS1 group in the buffer
---@param b integer
---@param c integer
---@param d integer
function RDS.put_custom_group(b, c, d) end

---Puts in a RDS2 group in the buffer
---@param a integer
---@param b integer
---@param c integer
---@param d integer
function RDS.put_rds2_custom_group(a, b, c, d) end

---Sets the AFs included in group 0
---@param afs table
function RDS.set_af(afs) end

---@class EON_Data
---@field enabled? boolean
---@field pi? integer
---@field tp? boolean
---@field ta? boolean
---@field pty? integer
---@field ps? string
---@field afs? table Empty on the getter - empty table does not mean the same thing as nil
---@field data? integer

---Sets data about the EON
---@param eon integer Index of the EON we are setting
---@param data EON_Data
function RDS.set_eon(eon, data) end

---Gets the same data set_rds_eon sets
---@param eon integer
---@return EON_Data
function RDS.get_eon(eon) end

userdata = {}

---@type integer
userdata.len = 0

---Data is allocated in each program's data for lua data (per program, diffrent program, diffrent data), note that this overwrites existing data over the whole userdata string
---@param data string
function userdata.set(data) end
---Writes to the userdata at the offset, size does not have to match the length of the string, if the string is less than size then the rest of the string will be padded with zeroes until it is size
---@param offset integer
---@param padded_size integer
---@param data string
function userdata.set_offset(offset, padded_size, data) end

---Returns all of the data saved as user data
---@return string
function userdata.get() end
---Gets data from userdata but at the specified offset
---@param offset integer
---@param size integer
---@return string
function userdata.get_offset(offset, size) end

---@class ext
ext = {}
---@class RDSext
RDS.ext = {}

-- RT Plus Tags
---Sets RT+ tags: type1, start1, len1, type2, start2, len2
---@param ertp boolean
---@param t1 integer
---@param s1 integer
---@param l1 integer
---@param t2 integer
---@param s2 integer
---@param l2 integer
function RDS.ext.set_rtplus_tags(ertp, t1, s1, l1, t2, s2, l2) end

---Gets RT+ tags: type1, start1, len1, type2, start2, len2
---@param ertp boolean
---@return integer type1, integer start1, integer len1, integer type2, integer start2, integer len2
function RDS.ext.get_rtplus_tags(ertp) end

---Toggles RTP or ERTP's toggle switch
---@param ertp boolean
function RDS.ext.toggle_rtp(ertp) end

---Sets the metadata of RTP or ERTP
---@param ertp boolean
---@param running boolean
function RDS.ext.set_rtp_meta(ertp, running) end
---Gets the metadata of RTP and ERTP
---@param ertp boolean
---@return boolean running
function RDS.ext.get_rtp_meta(ertp) end

---Sets the AFs included in the ODA
---@param afs table
function RDS.ext.set_af_oda(afs) end

---Registers an ODA to be used in the 0x6 of the group sequence. ODAs are stored as state data, thus running reset_rds will clear it
---Groups 14, 15, 2, 0 cannot be registered either version, groups 10, 4, 1 can be only registered as B, any other is free to take
---Group 3A will mean that there will be no group handler for this ODA, meaning it can only be interacted with via the 3A AID group, handler set is not possible with such groups
---@param group integer
---@param group_version boolean
---@param aid integer
---@param data integer
---@param temp boolean Send AID once (no data) and destroy
---@return integer oda_id
function ext.register_oda(group, group_version, aid, data, temp) end

---Unregisters an ODA, this stops the handler or AID being called/sent
---@param oda_id integer
function ext.unregister_oda(oda_id) end

---Sets the data for a existing ODA group
---@param oda_id integer
---@param data integer
function ext.set_oda_id_data(oda_id, data) end

---The callback function for an ODA handler
---@alias ODAHandler fun(): (boolean, integer, integer, integer)

---Sets a function to handle the ODA data generation. 
---The handler is called when the group sequence '0xff' slot is processed.
---The function must return 3 integers representing RDS Blocks B, C, and D.
---Please note that you do not need to compute the block B to indentify the group and group version, that will be done for you and EVERY SINGLE group has PTY and TP inserted (and also PI if its a B inside block C)
---You are asked to set groups B last 5 bits, leave rest 0
---@param oda_id integer The ID returned by register_oda
---@param fun ODAHandler
function ext.set_oda_handler(oda_id, fun) end

---@param ert string
function RDS.ext.set_ert(ert) end
---@return string
function RDS.ext.get_ert() end