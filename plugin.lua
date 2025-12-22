---@meta

-- Global Variables
---@type string
core_version = ""
---@type integer
max_programs = 0
---@type string|nil
data = "" -- Set during run_lua calls

---Starts the initialization sequence, also calls the on_init function
---@return nil
function set_rds_program_defaults() end

---Saves, loads and resets the state of the data
---@return nil
function reset_rds() end

---This function is called by the C core after we reset data, or have no data in general
---It should be defined by the user in the script.
---@return nil
function on_init() end

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

---@param ct integer 0 or 1
function set_rds_ct(ct) end
---@return integer
function get_rds_ct() end

---@param dpty integer 0 or 1
function set_rds_dpty(dpty) end
---@return integer
function get_rds_dpty() end

---@param tp integer 0 or 1
function set_rds_tp(tp) end
---@return integer
function get_rds_tp() end

---@param ta integer 0 or 1
function set_rds_ta(ta) end
---@return integer
function get_rds_ta() end

-- Feature Flags
---@param enabled integer 0 or 1
function set_rds_rt1_enabled(enabled) end
---@return integer
function get_rds_rt1_enabled() end

---@param enabled integer 0 or 1
function set_rds_rt2_enabled(enabled) end
---@return integer
function get_rds_rt2_enabled() end

---@param enabled integer 0 or 1
function set_rds_ptyn_enabled(enabled) end
---@return integer
function get_rds_ptyn_enabled() end

---@param rt_type integer 0 (RT A/B) or 1 (RT C)
function set_rds_rt_type(rt_type) end
---@return integer
function get_rds_rt_type() end

-- Modulation & Generation
---@param mode integer
function set_rds_rds2mod(mode) end
---@return integer
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
---@param linkage integer
function set_rds_link(linkage) end
---@return integer
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
---@param tps string Text PS (Scrolling PS)
function set_rds_tps(tps) end
---@param rt1 string Radio Text 1 (max 64 chars)
function set_rds_rt1(rt1) end
---@param rt2 string Radio Text 2 (max 64 chars)
function set_rds_rt2(rt2) end

---@param lps string
function set_rds_lps(lps) end
---@return string
function get_rds_lps() end

function set_rds_ert(ert) end
---@return string
function get_rds_ert() end

function set_rds_grpseq(grpseq) end
---@return string
function get_rds_grpseq() end


function set_rds_grpseq2(grpseq) end
---@return string
function get_rds_grpseq2() end

-- RT Plus Tags
---Sets RT+ tags: type1, start1, len1, type2, start2, len2
---@param t1 integer
---@param s1 integer
---@param l1 integer
---@param t2 integer
---@param s2 integer
---@param l2 integer
function set_rds_rtplus_tags(t1, s1, l1, t2, s2, l2) end

---Gets RT+ tags: type1, start1, len1, type2, start2, len2
---@return integer, integer, integer, integer, integer, integer
function get_rds_rtplus_tags() end

---Sets eRT+ tags: type1, start1, len1, type2, start2, len2
---@param t1 integer
---@param s1 integer
---@param l1 integer
---@param t2 integer
---@param s2 integer
---@param l2 integer
function set_rds_ertplus_tags(t1, s1, l1, t2, s2, l2) end

---Gets eRT+ tags: type1, start1, len1, type2, start2, len2
---@return integer, integer, integer, integer, integer, integer
function get_rds_ertplus_tags() end