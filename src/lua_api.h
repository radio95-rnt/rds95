#pragma once

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "rds.h"
#include "fs.h"

int lua_get_userdata(lua_State *L);
int lua_get_userdata_offset(lua_State *L);
int lua_set_userdata(lua_State *L);
int lua_set_userdata_offset(lua_State *L);
int lua_force_save(lua_State *L);
int lua_set_rds_program_defaults(lua_State *L);
int lua_reset_rds(lua_State *L);

int lua_set_rds_streams(lua_State *L);
int lua_get_rds_streams(lua_State *L);

int lua_set_rds_grp_sqc(lua_State *L);
int lua_get_rds_grp_sqc(lua_State *L);

int lua_set_rds_grp_sqc_rds2(lua_State *L);
int lua_get_rds_grp_sqc_rds2(lua_State *L);

int lua_set_rds_link(lua_State *L);
int lua_get_rds_link(lua_State *L);

int lua_set_rds_program(lua_State *L);
int lua_get_rds_program(lua_State *L);

int lua_set_rds_writing_program(lua_State *L);
int lua_get_rds_writing_program(lua_State *L);

int lua_put_rds_custom_group(lua_State *L);
int lua_put_rds2_custom_group(lua_State *L);

int lua_set_rds_lps(lua_State *L);
int lua_get_rds_lps(lua_State *L);
int lua_set_rds_af_group0(lua_State *L);
int lua_set_rds_rt(lua_State *L);
int lua_set_rds_rt_raw(lua_State *L);
int lua_set_rds_tps(lua_State *L);
int lua_set_rds_tps_raw(lua_State *L);
int lua_set_rds_ps(lua_State *L);
int lua_set_rds_ps_raw(lua_State *L);
int lua_set_rds_ptyn(lua_State *L);
int lua_set_rds_ptyn_raw(lua_State *L);
int lua_set_rds_grp_sqc(lua_State *L);

int lua_toggle_rt_ab(lua_State *L);

int lua_set_rds_eon(lua_State *L);
int lua_get_rds_eon(lua_State *L);

int lua_crc16(lua_State *L);

int lua_convert_to_rdscharset(lua_State *L);
int lua_encode_group(lua_State *L);

int lua_rds__index(lua_State *L);
int lua_rds__newindex(lua_State *L);