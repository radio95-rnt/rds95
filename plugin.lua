---@meta

-- Global Variables
---@type string
core_version = ""

--#region Functions implemented or used in C

dp = {}

---@type integer
dp.max_programs = 0

---Executes a CRC-16 CCIIT on given data
---@param data string
---@return integer
function dp.crc16(data) end

---Starts the initialization sequence, also calls the on_init function
---@return nil
function dp.set_program_defaults() end

---Saves, loads and resets the state of the data, you might as well restart the whole program
---@return nil
function dp.reset_rds() end

---Forces encoder and modulator data to be saved to disc
---@return nil
function dp.force_save() end

---@param program_idx integer 0 to (max_programs - 1)
function dp.set_program(program_idx) end
---@return integer
function dp.get_program() end

---This function is called by the C core after we reset data, or have no data in general
---It should be defined by the user in the script.
---@return nil
function on_init() end
on_inits = {}

---This function is called by the C core after we initialize the encoder (always, every start)
---It should be defined by the user in the script.
---@return nil
function on_start() end
on_starts = {}

---This function is called every time when the state resets, register your odas here
---It should be defined by the user in the script.
---@return nil
function on_state() end
on_states = {}

---This function is called every second
---It should be defined by the user in the script.
---@return nil
function tick() end
ticks = {}

-- Every function with with the s suffixed table means that other versions of that function can be in that list-table, and each one will be called

---This function is called in order to handle UDP data
---It should be defined by the user in the script.
---@param data string
---@return string
function data_handle(data) end

---This function is called when the group "L" is in the sequence
---Please remember that the core always fills in PTY and TP and PI in C if this is an B group
---It should be defined by the user in the script.
---@param group string group this was called in from the group sequence
---@return boolean generated
---@return integer b
---@return integer c
---@return integer d
function group(group) end

---This function is called when an RDS2 group is to be generated on mode 3
---If a was returned 0, PTY and TP will be filled in, along with the PI code in C if needed
---It should be defined by the user in the script.
---@param stream integer
---@return boolean generated
---@return integer a
---@return integer b
---@return integer c
---@return integer d
function rds2_group(stream) end

rds = {}

---@type integer
rds.eon_count = 0

---@param pi integer
function rds.set_pi(pi) end
---@return integer
function rds.get_pi() end

---@param pty integer
function rds.set_pty(pty) end
---@return integer
function rds.get_pty() end

---@param ecc integer
function rds.set_ecc(ecc) end
---@return integer
function rds.get_ecc() end

---@param slc_data integer
function rds.set_slc_data(slc_data) end
---@return integer
function rds.get_slc_data() end

---@param ct boolean
function rds.set_ct(ct) end
---@return boolean
function rds.get_ct() end

---@param dpty boolean
function rds.set_dpty(dpty) end
---@return boolean
function rds.get_dpty() end

---@param tp boolean
function rds.set_tp(tp) end
---@return boolean
function rds.get_tp() end

---@param ta boolean
function rds.set_ta(ta) end
---@return boolean
function rds.get_ta() end

-- Feature Flags
---@param enabled boolean
function rds.set_rt1_enabled(enabled) end
---@return boolean
function rds.get_rt1_enabled() end

---@param enabled boolean
function rds.set_rt2_enabled(enabled) end
---@return boolean
function rds.get_rt2_enabled() end

---@param enabled boolean
function rds.set_ptyn_enabled(enabled) end
---@return boolean
function rds.get_ptyn_enabled() end

---@param rt_type integer
function rds.set_rt_type(rt_type) end
---@return integer
function rds.get_rt_type() end

-- Modulation & Generation
---@param mode integer
function rds.set_rds2_mode(mode) end
---@return integer
function rds.get_rds2_mode() end

---@param streams integer
function rds.set_streams(streams) end
---@return integer
function rds.get_streams() end

---This function returns the absolute number of streams that can be output without restarting the core
---@return integer
function rds.get_available_streams() end

---@param level number
function rds.set_level(level) end
---@return number
function rds.get_level() end

-- Program & Linking
---@param linkage boolean
function rds.set_link(linkage) end
---@return boolean
function rds.get_link() end

-- Timeouts and Periods
---@param period integer in seconds
function rds.set_rt_switching_period(period) end
---@return integer
function rds.get_rt_switching_period() end

---For a RT1, this sets the timeout period before setting RT1 into "Default RT" (not literally)
---@param timeout integer in seconds
function rds.set_rt_text_timeout(timeout) end
---@return integer
function rds.get_rt_text_timeout() end

-- String Setters (Charset converted)
---@param ptyn string Program Type Name (max 8 chars)
function rds.set_ptyn(ptyn) end
---@param ps string Program Service (8 chars)
function rds.set_ps(ps) end
---@param tps string Traffic PS
function rds.set_tps(tps) end
---@param rt1 string Radio Text 1 (max 64 chars)
function rds.set_rt1(rt1) end
---@param rt2 string Radio Text 2 (max 64 chars)
function rds.set_rt2(rt2) end
---@param rt string Default radio text - max 64 characters
function rds.set_default_rt(rt) end

---@param lps string
function rds.set_lps(lps) end
---@return string
function rds.get_lps() end

---@param grpseq string
function rds.set_grpseq(grpseq) end
---@return string
function rds.get_grpseq() end

---@param grpseq2 string
function rds.set_grpseq2(grpseq2) end
---@return string
function rds.get_grpseq2() end

---Puts in a RDS1 group in the buffer, note that block A is filled in always
---@param b integer
---@param c integer
---@param d integer
function rds.put_custom_group(b, c, d) end

---Puts in a RDS2 group in the buffer
---@param a integer
---@param b integer
---@param c integer
---@param d integer
function rds.put_rds2_custom_group(a, b, c, d) end

---Sets the AFs included in group 0
---@param afs table
function rds.set_af(afs) end

---Sets data about the EON
---@param eon integer Index of the EON we are setting
---@param enabled boolean
---@param pi integer
---@param tp boolean
---@param ta boolean
---@param pty integer
---@param ps string
---@param afs table
---@param data integer
function rds.set_eon(eon, enabled, pi, tp, ta, pty, ps, afs, data) end

---Gets the same data set_rds_eon sets, yes this returns 8 arguments
---@param eon integer
---@return boolean enabled
---@return integer pi
---@return boolean tp
---@return boolean ta
---@return integer pty
---@return string ps
---@return table _ this is empty, getting afs is not supported yet
---@return integer data
function rds.get_eon(eon) end

---Sets the X/Y of the UDG
---@param xy boolean
---@param groups table Table of tables, this should be up to 8 tables containing 3 integers
function rds.set_udg(xy, groups) end
---Sets the X/Y of the UDG for RDS2
---@param xy boolean
---@param groups table Table of tables, this should be up to 8 tables containing 4 integers
function rds.set_udg2(xy, groups) end

userdata = {}

---@type integer
userdata.len = 0

---Data is allocated in each program's data for lua data (per program, diffrent program, diffrent data), note that this overwrites existing data over the whole userdata string
---@param data string
function userdata.set(data) end
---Writes to the userdata at the offset, size does not have to match the length of the string, if the string is less than size then the rest of the string will be padded with zeroes until it is size
---@param offset integer
---@param size integer
---@param data string
function userdata.set_offset(offset, size, data) end

---Returns all of the data saved as user data
---@return string
function userdata.get() end
---Gets data from userdata but at the specified offset
---@param offset integer
---@param size integer
---@return string
function userdata.get_offset(offset, size) end

--#endregion

ext = {}
rds.ext = {}

-- RT Plus Tags
---Sets RT+ tags: type1, start1, len1, type2, start2, len2
---@param ertp boolean
---@param t1 integer
---@param s1 integer
---@param l1 integer
---@param t2 integer
---@param s2 integer
---@param l2 integer
function rds.ext.set_rtplus_tags(ertp, t1, s1, l1, t2, s2, l2) end

---Gets RT+ tags: type1, start1, len1, type2, start2, len2
---@param ertp boolean
---@return integer type1, integer start1, integer len1, integer type2, integer start2, integer len2
function rds.ext.get_rtplus_tags(ertp) end

---Toggles RTP or ERTP's toggle switch
---@param ertp boolean
function rds.ext.toggle_rtp(ertp) end

---Sets the metadata of RTP or ERTP
---@param ertp boolean
---@param running boolean
function rds.ext.set_rtp_meta(ertp, running) end
---Gets the metadata of RTP and ERTP
---@param ertp boolean
---@return boolean running
function rds.ext.get_rtp_meta(ertp) end

---Sets the AFs included in the ODA
---@param afs table
function rds.ext.set_af_oda(afs) end

---Registers an ODA to be used in the O of the group sequence. ODAs are stored as state data, thus running reset_rds will clear it
---Groups 14, 15, 2, 0 cannot be registered either version, groups 10, 4, 1 can be only registered as B, any other is free to take
---Group 3A will mean that there will be no group handler for this ODA, meaning it can only be interacted with via the 3A AID group, handler set is not possible with such groups
---@param group integer
---@param group_version boolean
---@param aid integer
---@param data integer
---@return integer oda_id
function ext.register_oda(group, group_version, aid, data) end

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
---The handler is called when the group sequence 'K' slot is processed.
---The function must return 3 integers representing RDS Blocks B, C, and D.
---Please note that you do not need to compute the block B to indentify the group and group version, that will be done for you and EVERY SINGLE group has PTY and TP inserted (and also PI if its a B inside block C)
---You are asked to set groups B last 5 bits, leave rest 0
---@param oda_id integer The ID returned by register_oda
---@param fun ODAHandler
function ext.set_oda_handler(oda_id, fun) end

---The callback function for an ODA handler
---@alias RDS2_ODAHandler fun(integer): (boolean, integer, integer, integer, integer)

---You are asked to not fill in the channel id in block A, however you are asked to fill in the function number (if you do not know what is that, just OR block A with (1 << 14))
---@param oda_id integer
---@param func RDS2_ODAHandler
function ext.set_oda_handler_rds2(oda_id, func) end

---@param oda_id integer
---@param data integer
function ext.set_oda_id_data_rds2(oda_id, data) end

---@param aid integer
---@param data integer
---@param file_related boolean
---@return integer oda_id
function ext.register_oda_rds2(aid, data, file_related) end

---Unregisters an RDS 2 ODA, this stops the handler or AID being called/sent
---@param oda_id integer
function ext.unregister_oda_rds2(oda_id) end

---@param ert string
function rds.ext.set_ert(ert) end
---@return string
function rds.ext.get_ert() end