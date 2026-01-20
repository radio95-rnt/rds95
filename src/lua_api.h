#pragma once

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "rds.h"
#include "fs.h"
#include "modulator.h"

int lua_get_userdata(lua_State *localL);
int lua_get_userdata_offset(lua_State *localL);
int lua_set_userdata(lua_State *localL);
int lua_set_userdata_offset(lua_State *localL);
int lua_force_save(lua_State *localL);
int lua_set_rds_program_defaults(lua_State *localL);
int lua_reset_rds(lua_State *localL);

int lua_set_rds_pi(lua_State *localL);
int lua_get_rds_pi(lua_State *localL);

int lua_set_rds_pty(lua_State *localL);
int lua_get_rds_pty(lua_State *localL);

int lua_set_rds_ecc(lua_State *localL);
int lua_get_rds_ecc(lua_State *localL);

int lua_set_rds_slc_data(lua_State *localL);
int lua_get_rds_slc_data(lua_State *localL);

int lua_set_rds_ct(lua_State *localL);
int lua_get_rds_ct(lua_State *localL);

int lua_set_rds_dpty(lua_State *localL);
int lua_get_rds_dpty(lua_State *localL);

int lua_set_rds_tp(lua_State *localL);
int lua_get_rds_tp(lua_State *localL);

int lua_set_rds_ta(lua_State *localL);
int lua_get_rds_ta(lua_State *localL);

int lua_set_rds_rt1_enabled(lua_State *localL);
int lua_get_rds_rt1_enabled(lua_State *localL);

int lua_set_rds_rt2_enabled(lua_State *localL);
int lua_get_rds_rt2_enabled(lua_State *localL);

int lua_set_rds_ptyn_enabled(lua_State *localL);
int lua_get_rds_ptyn_enabled(lua_State *localL);

int lua_set_rds_rt_type(lua_State *localL);
int lua_get_rds_rt_type(lua_State *localL);

int lua_set_rds2_mode(lua_State *localL);
int lua_get_rds2_mode(lua_State *localL);

int lua_set_rds_streams(lua_State *localL);
int lua_get_rds_streams(lua_State *localL);
int lua_get_available_rds_streams(lua_State *localL);

int lua_set_rds_grp_sqc(lua_State *localL);
int lua_get_rds_grp_sqc(lua_State *localL);

int lua_set_rds_grp_sqc_rds2(lua_State *localL);
int lua_get_rds_grp_sqc_rds2(lua_State *localL);

int lua_set_rds_link(lua_State *localL);
int lua_get_rds_link(lua_State *localL);

int lua_set_rds_program(lua_State *localL);
int lua_get_rds_program(lua_State *localL);

int lua_set_rds_rt_switching_period(lua_State *localL);
int lua_get_rds_rt_switching_period(lua_State *localL);

int lua_set_rds_rt_text_timeout(lua_State *localL);
int lua_get_rds_rt_text_timeout(lua_State *localL);

int lua_set_rds_level(lua_State *localL);
int lua_get_rds_level(lua_State *localL);

int lua_put_rds_custom_group(lua_State *localL);
int lua_put_rds2_custom_group(lua_State *localL);

int lua_set_rds_lps(lua_State *localL);
int lua_get_rds_lps(lua_State *localL);
int lua_set_rds_af_group0(lua_State *localL);
int lua_set_rds_default_rt(lua_State *localL);
int lua_set_rds_rt2(lua_State *localL);
int lua_set_rds_rt1(lua_State *localL);
int lua_set_rds_tps(lua_State *localL);
int lua_set_rds_ps(lua_State *localL);
int lua_set_rds_ptyn(lua_State *localL);
int lua_set_rds_grp_sqc(lua_State *localL);

int lua_set_rds_eon(lua_State *localL);
int lua_get_rds_eon(lua_State *localL);
int lua_set_rds_udg(lua_State *localL);
int lua_set_rds_udg2(lua_State *localL);

int lua_crc16(lua_State *localL);