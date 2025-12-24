---@meta

-- Global Variables
---@type string
core_version = ""
---@type integer
eon_count = 0
---@type integer
max_programs = 0
---@type string|nil
data = "" -- Set during run_lua calls

---Starts the initialization sequence, also calls the on_init function
---@return nil
function set_rds_program_defaults() end

---Saves, loads and resets the state of the data, you might as well restart the whole program
---@return nil
function reset_rds() end

---This function is called by the C core after we reset data, or have no data in general
---It should be defined by the user in the script.
---@return nil
function on_init() end
---This function is called by the C core after we initialize the encoder (always, every start)
---It should be defined by the user in the script.
---@return nil
function on_start() end
---This function is called every minute
---It should be defined by the user in the script.
---@return nil
function tick() end
---This function is called in order to handle UDP data
---It should be defined by the user in the script.
---@param data string
---@return string
function data_handle(data) end
---This function is called when the group "L" is in the sequence
---It should be defined by the user in the script.
---@param b integer
---@param c integer
---@param d integer
---@return integer b
---@return integer c
---@return integer d
function group(a, b, c, d) end

---@param pi integer
function set_rds_pi(pi) end
---@return integer
function get_rds_pi() end

---@param pty integer
function set_rds_pty(pty) end
---@return integer
function get_rds_pty() end

---@param ecc integer
function set_rds_ecc(ecc) end
---@return integer
function get_rds_ecc() end

---@param slc_data integer
function set_rds_slc_data(slc_data) end
---@return integer
function get_rds_slc_data() end

---@param ct boolean
function set_rds_ct(ct) end
---@return boolean
function get_rds_ct() end

---@param dpty boolean
function set_rds_dpty(dpty) end
---@return boolean
function get_rds_dpty() end

---@param tp boolean
function set_rds_tp(tp) end
---@return boolean
function get_rds_tp() end

---@param ta boolean
function set_rds_ta(ta) end
---@return boolean
function get_rds_ta() end

-- Feature Flags
---@param enabled boolean
function set_rds_rt1_enabled(enabled) end
---@return boolean
function get_rds_rt1_enabled() end

---@param enabled boolean
function set_rds_rt2_enabled(enabled) end
---@return boolean
function get_rds_rt2_enabled() end

---@param enabled boolean
function set_rds_ptyn_enabled(enabled) end
---@return boolean
function get_rds_ptyn_enabled() end

---@param rt_type integer
function set_rds_rt_type(rt_type) end
---@return integer
function get_rds_rt_type() end

-- Modulation & Generation
---@param mode boolean
function set_rds_rds2mod(mode) end
---@return boolean
function get_rds_rds2mod() end

---@param rdsgen integer
function set_rds_rdsgen(rdsgen) end
---@return integer
function get_rds_rdsgen() end

---@param level number
function set_rds_level(level) end
---@return number
function get_rds_level() end

-- Program & Linking
---@param linkage boolean
function set_rds_link(linkage) end
---@return boolean
function get_rds_link() end

---@param program_idx integer 0 to (max_programs - 1)
function set_rds_program(program_idx) end
---@return integer
function get_rds_program() end

-- Timeouts and Periods
---@param period integer
function set_rds_rt_switching_period(period) end
---@return integer
function get_rds_rt_switching_period() end

---@param timeout integer
function set_rds_rt_text_timeout(timeout) end
---@return integer
function get_rds_rt_text_timeout() end

-- String Setters (Charset converted)
---@param ptyn string Program Type Name (max 8 chars)
function set_rds_ptyn(ptyn) end
---@param ps string Program Service (8 chars)
function set_rds_ps(ps) end
---@param tps string Traffic PS
function set_rds_tps(tps) end
---@param rt1 string Radio Text 1 (max 64 chars)
function set_rds_rt1(rt1) end
---@param rt2 string Radio Text 2 (max 64 chars)
function set_rds_rt2(rt2) end

---@param lps string
function set_rds_lps(lps) end
---@return string
function get_rds_lps() end

---@param ert string
function set_rds_ert(ert) end
---@return string
function get_rds_ert() end

---@param grpseq string
function set_rds_grpseq(grpseq) end
---@return string
function get_rds_grpseq() end

---@param grpseq2 string
function set_rds_grpseq2(grpseq2) end
---@return string
function get_rds_grpseq2() end

-- RT Plus Tags
---Sets RT+ tags: type1, start1, len1, type2, start2, len2
---@param ertp boolean
---@param t1 integer
---@param s1 integer
---@param l1 integer
---@param t2 integer
---@param s2 integer
---@param l2 integer
function set_rds_rtplus_tags(ertp, t1, s1, l1, t2, s2, l2) end

---Gets RT+ tags: type1, start1, len1, type2, start2, len2
---@param ertp boolean
---@return integer type1, integer start1, integer len1, integer type2, integer start2, integer len2
function get_rds_rtplus_tags(ertp) end

---Puts in a RDS1 group in the buffer, note that block A is filled in always
---@param b integer
---@param c integer
---@param d integer
function put_rds_custom_group(b, c, d) end

---Puts in a RDS2 group in the buffer
---@param a integer
---@param b integer
---@param c integer
---@param d integer
function put_rds2_custom_group(a, b, c, d) end

---Toggles RTP or ERTP's toggle switch
---@param ertp boolean
function toggle_rds_rtp(ertp) end

---Sets the metadata of RTP or ERTP
---@param ertp boolean
---@param enabled boolean
---@param running boolean
function set_rds_rtp_meta(ertp, enabled, running) end
---Gets the metadata of RTP and ERTP
---@param ertp boolean
---@return boolean enabled, boolean running
function get_rds_rtp_meta(ertp) end

---Sets the AFs included in group 0
---@param afs table
function set_rds_af_group0(afs) end
---Sets the AFs included in the ODA
---@param afs table
function set_rds_af_oda(afs) end

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
function set_rds_eon(eon, enabled, pi, tp, ta, pty, ps, afs, data) end

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
function get_rds_eon(eon) end

---Sets the X/Y of the UDG
---@param xy boolean
---@param groups table Table of tables, this should be up to 8 tables containing 3 integers
function set_rds_udg(xy, groups) end
---Sets the X/Y of the UDG for RDS2
---@param xy boolean
---@param groups table Table of tables, this should be up to 8 tables containing 4 integers
function set_rds_udg2(xy, groups) end

---Registers an ODA to be used in the O of the group sequence
---@param group integer
---@param group_version boolean
---@param id integer
---@param id_data integer
function register_oda(group, group_version, id, id_data) end