#include "lua_rds.h"
#include <pthread.h>

static RDSModulator* mod = NULL;
static lua_State *L = NULL;
static pthread_mutex_t lua_mutex;
static uint8_t unload_refs[33] = {LUA_REFNIL};
static uint8_t lua_reload_scheduled = 0;

int lua_reload_command(lua_State* localL) {
    (void)localL;
    lua_reload_scheduled = 1;
    return 0;
}

int lua_set_rds_program_defaults(lua_State *localL) {
    (void)localL;
    for (int i = 1; i < *unload_refs; i++) luaL_unref(L, LUA_REGISTRYINDEX, unload_refs[i]);
    unload_refs[0] = 1;
	set_rds_defaults(mod->enc, mod->enc->program);
    lua_call_function("on_init");
    lua_call_function("on_state");
    return 0;
}

int lua_reset_rds(lua_State *localL) {
    (void)localL;
    for (int i = 1; i < *unload_refs; i++) luaL_unref(L, LUA_REGISTRYINDEX, unload_refs[i]);
    unload_refs[0] = 1;
    encoder_saveToFile(mod->enc);
    Modulator_saveToFile(&mod->params);

	encoder_loadFromFile(mod->enc);
	for(int i = 0; i < PROGRAMS; i++) reset_rds_state(mod->enc, i);
	Modulator_loadFromFile(&mod->params);
    lua_call_function("on_state");
    return 0;
}

#define BOOL_SETTER(name) \
int lua_set_rds_##name(lua_State *localL) { \
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1)); \
	mod->enc->data[mod->enc->program].name = lua_toboolean(localL, 1); \
    return 0; \
}
#define INT_SETTER(name) \
int lua_set_rds_##name(lua_State *localL) { \
	mod->enc->data[mod->enc->program].name = luaL_checkinteger(localL, 1); \
    return 0; \
}
#define STR_SETTER(name, function) \
int lua_set_rds_##name(lua_State *localL) { \
	const char* str = luaL_checklstring(localL, 1, NULL); \
    function(mod->enc, convert_to_rdscharset(str)); \
    return 0; \
}
#define STR_RAW_SETTER(name, function) \
int lua_set_rds_##name(lua_State *localL) { \
	const char* str = luaL_checklstring(localL, 1, NULL); \
    function(mod->enc, str); \
    return 0; \
}
#define AF_SETTER(name, af_field, af_struct, add_func) \
int lua_set_rds_##name(lua_State *localL) { \
    luaL_checktype(localL, 1, LUA_TTABLE); \
    \
    int n = lua_rawlen(localL, 1); \
    if (n == 0) { \
        memset(&(mod->enc->data[mod->enc->program].af_field), 0, sizeof(af_struct)); \
        return 0; \
    } \
    if(n > 25) return luaL_error(localL, "table length over 25"); \
    \
    af_struct new_af; \
    memset(&new_af, 0, sizeof(af_struct)); \
    \
    for (int i = 1; i <= n; i++) { \
        lua_rawgeti(localL, 1, i); \
        if (lua_isnumber(localL, -1)) add_func(&new_af, lua_tonumber(localL, -1)); \
        else return luaL_error(localL, "number expected, got %s", luaL_typename(localL, -1)); \
        lua_pop(localL, 1); \
    } \
	memcpy(&(mod->enc->data[mod->enc->program].af_field), &new_af, sizeof(new_af)); \
    \
    return 0; \
}

#define INT_GETTER(name) \
int lua_get_rds_##name(lua_State *localL) { \
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].name); \
    return 1; \
}
#define BOOL_GETTER(name) \
int lua_get_rds_##name(lua_State *localL) { \
    lua_pushboolean(localL, mod->enc->data[mod->enc->program].name); \
    return 1; \
}
#define STR_RAW_GETTER(name, length) \
int lua_get_rds_##name(lua_State *localL) { \
    lua_pushlstring(localL, mod->enc->data[mod->enc->program].name, length); \
    return 1; \
}
INT_SETTER(pi)
INT_GETTER(pi)

INT_SETTER(pty)
INT_GETTER(pty)

INT_SETTER(ecc)
INT_GETTER(ecc)

INT_SETTER(slc_data)
INT_GETTER(slc_data)

BOOL_SETTER(ct)
BOOL_GETTER(ct)

BOOL_SETTER(dpty)
BOOL_GETTER(dpty)

BOOL_SETTER(tp)
BOOL_GETTER(tp)

BOOL_SETTER(ta)
BOOL_GETTER(ta)

BOOL_SETTER(rt1_enabled)
BOOL_GETTER(rt1_enabled)

BOOL_SETTER(rt2_enabled)
BOOL_GETTER(rt2_enabled)

BOOL_SETTER(ptyn_enabled)
BOOL_GETTER(ptyn_enabled)

INT_SETTER(rt_type)
INT_GETTER(rt_type)

int lua_set_rds_rds2mod(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
	mod->enc->encoder_data.rds2_mode = lua_toboolean(localL, 1);
    return 0;
}
int lua_get_rds_rds2mod(lua_State *localL) {
    lua_pushboolean(localL, mod->enc->encoder_data.rds2_mode);
    return 1;
}
int lua_set_rds_rdsgen(lua_State *localL) {
	mod->params.rdsgen = luaL_checkinteger(localL, 1);
    return 0;
}
int lua_get_rds_rdsgen(lua_State *localL) {
	lua_pushinteger(localL, mod->params.rdsgen);
    return 1;
}

int lua_set_rds_link(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
	mod->enc->state[mod->enc->program].eon_linkage = lua_toboolean(localL, 1);
    return 0;
}
int lua_get_rds_link(lua_State *localL) {
    lua_pushboolean(localL, mod->enc->state[mod->enc->program].eon_linkage);
    return 1;
}

int lua_set_rds_program(lua_State *localL) {
    int program = luaL_checkinteger(localL, 1);
    if(program >= PROGRAMS) program = (PROGRAMS-1);
	if(program < 0) program = 0;

    if(mod->enc->program == program) return 0;

    mod->enc->data[mod->enc->program].ta = 0;
	mod->enc->data[(uint8_t)program].ta = 0;
	mod->enc->program = (uint8_t)program;
    return 0;
}
int lua_get_rds_program(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->program);
    return 1;
}

int lua_set_rds_rt_switching_period(lua_State *localL) {
	mod->enc->data[mod->enc->program].rt_switching_period = luaL_checkinteger(localL, 1);
	mod->enc->state[mod->enc->program].rt_switching_period_state = mod->enc->data[mod->enc->program].rt_switching_period;
    return 0;
}
INT_GETTER(rt_switching_period)
int lua_set_rds_rt_text_timeout(lua_State *localL) {
	mod->enc->data[mod->enc->program].rt_text_timeout = luaL_checkinteger(localL, 1);
	mod->enc->state[mod->enc->program].rt_text_timeout_state = mod->enc->data[mod->enc->program].rt_text_timeout;
    return 0;
}
INT_GETTER(rt_text_timeout)

int lua_set_rds_level(lua_State *localL) {
	mod->params.level = luaL_checknumber(localL, 1);
    return 0;
}
int lua_get_rds_level(lua_State *localL) {
    lua_pushnumber(localL, mod->params.level);
    return 1;
}

int lua_set_rds_rtplus_tags(lua_State *localL) {
    uint8_t tags[6];
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
	int ertp = lua_toboolean(localL, 1);
	tags[0] = luaL_checkinteger(localL, 2);
	tags[1] = luaL_checkinteger(localL, 3);
	tags[2] = luaL_checkinteger(localL, 4);
	tags[3] = luaL_checkinteger(localL, 5);
	tags[4] = luaL_checkinteger(localL, 6);
	tags[5] = luaL_checkinteger(localL, 7);
    if(ertp == 1) set_rds_ertplus_tags(mod->enc, tags);
    else set_rds_rtplus_tags(mod->enc, tags);
    return 0;
}
int lua_get_rds_rtplus_tags(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
	int ertp = lua_toboolean(localL, 1);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][ertp].type[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][ertp].start[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][ertp].len[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][ertp].type[1]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][ertp].start[1]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][ertp].len[1]);
    return 6;
}

int lua_put_rds_custom_group(lua_State *localL) {
	mod->enc->state[mod->enc->program].custom_group[0] = 1;
	mod->enc->state[mod->enc->program].custom_group[1] = luaL_checkinteger(localL, 1);
	mod->enc->state[mod->enc->program].custom_group[2] = luaL_checkinteger(localL, 2);
	mod->enc->state[mod->enc->program].custom_group[3] = luaL_checkinteger(localL, 3);
    return 0;
}
int lua_put_rds2_custom_group(lua_State *localL) {
	mod->enc->state[mod->enc->program].custom_group2[0] = 1;
	mod->enc->state[mod->enc->program].custom_group2[1] = luaL_checkinteger(localL, 1);
	mod->enc->state[mod->enc->program].custom_group2[2] = luaL_checkinteger(localL, 2);
	mod->enc->state[mod->enc->program].custom_group2[3] = luaL_checkinteger(localL, 3);
	mod->enc->state[mod->enc->program].custom_group2[4] = luaL_checkinteger(localL, 4);
    return 0;
}

int lua_toggle_rds_rtp(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
	int ertp = lua_toboolean(localL, 1);
    TOGGLE(mod->enc->rtpState[mod->enc->program][ertp].toggle);
    return 0;
}

int lua_set_rds_rtp_meta(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
    if (!lua_isboolean(localL, 2)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 2));
    if (!lua_isboolean(localL, 3)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 3));
	int ertp = lua_toboolean(localL, 1);
    mod->enc->rtpData[mod->enc->program][ertp].enabled = lua_toboolean(localL, 2);
    mod->enc->rtpData[mod->enc->program][ertp].running = lua_toboolean(localL, 3);
    return 0;
}
int lua_get_rds_rtp_meta(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
	int ertp = lua_toboolean(localL, 1);
    lua_pushboolean(localL, mod->enc->rtpData[mod->enc->program][ertp].enabled);
    lua_pushboolean(localL, mod->enc->rtpData[mod->enc->program][ertp].running);
    return 2;
}

STR_SETTER(ptyn, set_rds_ptyn)
STR_SETTER(ps, set_rds_ps)
STR_SETTER(tps, set_rds_tps)
STR_SETTER(rt1, set_rds_rt1)
STR_SETTER(rt2, set_rds_rt2)

STR_RAW_SETTER(lps, set_rds_lps)
STR_RAW_GETTER(lps, LPS_LENGTH)

STR_RAW_SETTER(ert, set_rds_ert)
STR_RAW_GETTER(ert, ERT_LENGTH)

STR_RAW_SETTER(grp_sqc_rds2, set_rds_grpseq2)
STR_RAW_GETTER(grp_sqc_rds2, 24)

int lua_set_rds_grp_sqc(lua_State *localL) {
	const char* str = luaL_checklstring(localL, 1, NULL);
    if(_strnlen(str, 2) < 1) set_rds_grpseq(mod->enc, DEFAULT_GRPSQC);
    else set_rds_grpseq(mod->enc, str);
    return 0;
}
STR_RAW_GETTER(grp_sqc, 24)

AF_SETTER(af_group0, af, RDSAFs, add_rds_af)
AF_SETTER(af_oda, af_oda, RDSAFsODA, add_rds_af_oda)

int lua_set_rds_eon(lua_State *localL) {
    int eon = luaL_checkinteger(localL, 1);
    if(eon >= EONs) return luaL_error(localL, "eon index exceeded");
    if (!lua_isboolean(localL, 2)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 2));
    if (!lua_isboolean(localL, 4)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 4));
    if (!lua_isboolean(localL, 5)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 5));
    luaL_checktype(localL, 8, LUA_TTABLE);
    mod->enc->data[mod->enc->program].eon[eon].enabled = lua_toboolean(localL, 2);
    mod->enc->data[mod->enc->program].eon[eon].pi = luaL_checkinteger(localL, 3);
    mod->enc->data[mod->enc->program].eon[eon].tp = lua_toboolean(localL, 4);
    mod->enc->data[mod->enc->program].eon[eon].ta = lua_toboolean(localL, 5);
    mod->enc->data[mod->enc->program].eon[eon].pty = luaL_checkinteger(localL, 6);
	_strncpy(mod->enc->data[mod->enc->program].eon[eon].ps, luaL_checklstring(localL, 7, NULL), 8);

    int n = lua_rawlen(localL, 8);
    if (n == 0) {
        memset(&(mod->enc->data[mod->enc->program].eon[eon].af), 0, sizeof(RDSAFs));
        return 0;
    }
    if(n > 25) return luaL_error(localL, "table length over 25");

    RDSAFs new_af;
    memset(&new_af, 0, sizeof(RDSAFs));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(localL, 8, i);
        if (lua_isnumber(localL, -1)) add_rds_af(&new_af, lua_tonumber(localL, -1));
        else return luaL_error(localL, "number expected, got %s", luaL_typename(localL, -1));
        lua_pop(localL, 1);
    }
	memcpy(&(mod->enc->data[mod->enc->program].eon[eon].af), &new_af, sizeof(new_af));

    mod->enc->data[mod->enc->program].eon[eon].data = luaL_checkinteger(localL, 9);
    return 0;
}
int lua_get_rds_eon(lua_State *localL) {
    int eon = luaL_checkinteger(localL, 1);
    if(eon >= EONs) return luaL_error(localL, "eon index exceeded");
    lua_pushboolean(localL, mod->enc->data[mod->enc->program].eon[eon].enabled);
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].eon[eon].pi);
    lua_pushboolean(localL, mod->enc->data[mod->enc->program].eon[eon].tp);
    lua_pushboolean(localL, mod->enc->data[mod->enc->program].eon[eon].ta);
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].eon[eon].pty);
    lua_pushlstring(localL, mod->enc->data[mod->enc->program].eon[eon].ps, 8);
    lua_createtable(localL, 0, 0); // don't have decoding for AF, so just return empty table
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].eon[eon].data);
    return 8;
}

int lua_set_rds_udg(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
    int xy = lua_toboolean(localL, 1);
    luaL_checktype(localL, 2, LUA_TTABLE);
    int n = lua_rawlen(localL, 2);
    if(n > 8) return luaL_error(localL, "table length over 8");

    uint16_t blocks[8][3] = {0};

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(localL, 2, i);
        if(lua_istable(localL, -1)) {
            int n2 = lua_rawlen(localL, -1);
            if(n2 > 3) return luaL_error(localL, "table length over 3");
            for(int j = 1; j <= n2; j++) {
                lua_rawgeti(localL, -1, j);
                if (lua_isinteger(localL, -1)) blocks[i-1][j-1] = lua_tointeger(localL, -1);
                else return luaL_error(localL, "integer expected, got %s", luaL_typename(localL, -1));
                lua_pop(localL, 1);
            }
        }
        else return luaL_error(localL, "table expected, got %s", luaL_typename(localL, -1));
        lua_pop(localL, 1);
    }

    if(xy) {
        memcpy(&(mod->enc->data[mod->enc->program].udg2), blocks, n * sizeof(uint16_t[3]));
		mod->enc->data[mod->enc->program].udg2_len = n;
    } else {
        memcpy(&(mod->enc->data[mod->enc->program].udg1), blocks, n * sizeof(uint16_t[3]));
		mod->enc->data[mod->enc->program].udg1_len = n;
    }

    return 0;
}
int lua_set_rds_udg2(lua_State *localL) {
    if (!lua_isboolean(localL, 1)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 1));
    int xy = lua_toboolean(localL, 1);
    luaL_checktype(localL, 2, LUA_TTABLE);
    int n = lua_rawlen(localL, 2);
    if(n > 8) return luaL_error(localL, "table length over 8");

    uint16_t blocks[8][4] = {0};

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(localL, 2, i);
        if(lua_istable(localL, -1)) {
            int n2 = lua_rawlen(localL, -1);
            if(n2 > 4) return luaL_error(localL, "table length over 4");
            for(int j = 1; j <= n2; j++) {
                lua_rawgeti(localL, -1, j);
                if (lua_isinteger(localL, -1)) blocks[i-1][j-1] = lua_tointeger(localL, -1);
                else return luaL_error(localL, "integer expected, got %s", luaL_typename(localL, -1));
                lua_pop(localL, 1);
            }
        }
        else return luaL_error(localL, "table expected, got %s", luaL_typename(localL, -1));
        lua_pop(localL, 1);
    }

    if(xy) {
        memcpy(&(mod->enc->data[mod->enc->program].udg2_rds2), blocks, n * sizeof(uint16_t[4]));
		mod->enc->data[mod->enc->program].udg2_len_rds2 = n;
    } else {
        memcpy(&(mod->enc->data[mod->enc->program].udg1_rds2), blocks, n * sizeof(uint16_t[4]));
		mod->enc->data[mod->enc->program].udg1_len_rds2 = n;
    }

    return 0;
}

int lua_register_oda(lua_State *localL) {
    if (!lua_isboolean(localL, 2)) return luaL_error(localL, "boolean expected, got %s", luaL_typename(localL, 2));
    uint8_t id = mod->enc->state[mod->enc->program].user_oda.oda_len++;
	if(mod->enc->state[mod->enc->program].user_oda.oda_len >= 32) return luaL_error(localL, "There can't be more than 32 registered ODAs");
	if(mod->enc->state[mod->enc->program].user_oda.odas[id].group != 0) return luaL_error(localL, "internal error");
    mod->enc->state[mod->enc->program].user_oda.odas[id].group = luaL_checkinteger(localL, 1);
    if(mod->enc->state[mod->enc->program].user_oda.odas[id].group == 0) return luaL_error(localL, "Invalid group");
    mod->enc->state[mod->enc->program].user_oda.odas[id].group_version = lua_toboolean(localL, 2);
    mod->enc->state[mod->enc->program].user_oda.odas[id].id = luaL_checkinteger(localL, 3);
    mod->enc->state[mod->enc->program].user_oda.odas[id].id_data = luaL_checkinteger(localL, 4);
    lua_pushinteger(localL, id);
    return 1;
}

int lua_set_oda_handler(lua_State *localL) {
    uint8_t idx = luaL_checkinteger(localL, 1);
	if(idx >= 32) return luaL_error(localL, "There can't be more than 32 registered ODAs");
	if(mod->enc->state[mod->enc->program].user_oda.odas[idx].group == 0) return luaL_error(localL, "this oda is not registered");
    luaL_checktype(localL, 2, LUA_TFUNCTION);
    lua_pushvalue(localL, 2);
    if(mod->enc->state[mod->enc->program].user_oda.odas[idx].lua_handler != 0) luaL_unref(localL, LUA_REGISTRYINDEX, mod->enc->state[mod->enc->program].user_oda.odas[idx].lua_handler);
    mod->enc->state[mod->enc->program].user_oda.odas[idx].lua_handler = luaL_ref(localL, LUA_REGISTRYINDEX);
    int index = *unload_refs;
    unload_refs[index] = mod->enc->state[mod->enc->program].user_oda.odas[idx].lua_handler;
    (*unload_refs)++;
    return 0;
}

void init_lua(RDSModulator* rds_mod) {
    static int mutex_initialized = 0;
    mod = rds_mod;
    L = luaL_newstate();

    luaL_requiref(L, "_G", luaopen_base, 1);
    luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, 1);
    luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, 1);
    luaL_requiref(L, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    luaL_requiref(L, LUA_COLIBNAME, luaopen_coroutine, 1);
    luaL_requiref(L, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(L, 6);

    lua_pushstring(L, VERSION);
    lua_setglobal(L, "core_version");
    lua_pushinteger(L, PROGRAMS);
    lua_setglobal(L, "max_programs");
    lua_pushinteger(L, EONs);
    lua_setglobal(L, "eon_count");

    lua_register(L, "set_rds_program_defaults", lua_set_rds_program_defaults);
    lua_register(L, "reset_rds", lua_reset_rds);
    lua_register(L, "reload", lua_reload_command);

    lua_register(L, "set_rds_pi", lua_set_rds_pi);
    lua_register(L, "get_rds_pi", lua_get_rds_pi);

    lua_register(L, "set_rds_pty", lua_set_rds_pty);
    lua_register(L, "get_rds_pty", lua_get_rds_pty);

    lua_register(L, "set_rds_ecc", lua_set_rds_ecc);
    lua_register(L, "get_rds_ecc", lua_get_rds_ecc);

    lua_register(L, "set_rds_slc_data", lua_set_rds_slc_data);
    lua_register(L, "get_rds_slc_data", lua_get_rds_slc_data);

    lua_register(L, "set_rds_ct", lua_set_rds_ct);
    lua_register(L, "get_rds_ct", lua_get_rds_ct);

    lua_register(L, "set_rds_dpty", lua_set_rds_dpty);
    lua_register(L, "get_rds_dpty", lua_get_rds_dpty);

    lua_register(L, "set_rds_tp", lua_set_rds_tp);
    lua_register(L, "get_rds_tp", lua_get_rds_tp);

    lua_register(L, "set_rds_ta", lua_set_rds_ta);
    lua_register(L, "get_rds_ta", lua_get_rds_ta);

    lua_register(L, "set_rds_rt1_enabled", lua_set_rds_rt1_enabled);
    lua_register(L, "get_rds_rt1_enabled", lua_get_rds_rt1_enabled);

    lua_register(L, "set_rds_rt2_enabled", lua_set_rds_rt2_enabled);
    lua_register(L, "get_rds_rt2_enabled", lua_get_rds_rt2_enabled);

    lua_register(L, "set_rds_ptyn_enabled", lua_set_rds_ptyn_enabled);
    lua_register(L, "get_rds_ptyn_enabled", lua_get_rds_ptyn_enabled);

    lua_register(L, "set_rds_rt_type", lua_set_rds_rt_type);
    lua_register(L, "get_rds_rt_type", lua_get_rds_rt_type);

    lua_register(L, "set_rds_rds2mod", lua_set_rds_rds2mod);
    lua_register(L, "get_rds_rds2mod", lua_get_rds_rds2mod);

    lua_register(L, "set_rds_rdsgen", lua_set_rds_rdsgen);
    lua_register(L, "get_rds_rdsgen", lua_get_rds_rdsgen);

    lua_register(L, "set_rds_grpseq", lua_set_rds_grp_sqc);
    lua_register(L, "get_rds_grpseq", lua_get_rds_grp_sqc);

    lua_register(L, "set_rds_grpseq2", lua_set_rds_grp_sqc_rds2);
    lua_register(L, "get_rds_grpseq2", lua_get_rds_grp_sqc_rds2);

    lua_register(L, "set_rds_link", lua_set_rds_link);
    lua_register(L, "get_rds_link", lua_get_rds_link);

    lua_register(L, "set_rds_program", lua_set_rds_program);
    lua_register(L, "get_rds_program", lua_get_rds_program);

    lua_register(L, "set_rds_rt_switching_period", lua_set_rds_rt_switching_period);
    lua_register(L, "get_rds_rt_switching_period", lua_get_rds_rt_switching_period);

    lua_register(L, "set_rds_rt_text_timeout", lua_set_rds_rt_text_timeout);
    lua_register(L, "get_rds_rt_text_timeout", lua_get_rds_rt_text_timeout);

    lua_register(L, "set_rds_level", lua_set_rds_level);
    lua_register(L, "get_rds_level", lua_get_rds_level);

    lua_register(L, "set_rds_ptyn", lua_set_rds_ptyn);
    lua_register(L, "set_rds_ps", lua_set_rds_ps);
    lua_register(L, "set_rds_tps", lua_set_rds_tps);
    lua_register(L, "set_rds_rt1", lua_set_rds_rt1);
    lua_register(L, "set_rds_rt2", lua_set_rds_rt2);

    lua_register(L, "set_rds_lps", lua_set_rds_lps);
    lua_register(L, "get_rds_lps", lua_get_rds_lps);

    lua_register(L, "set_rds_ert", lua_set_rds_ert);
    lua_register(L, "get_rds_ert", lua_get_rds_ert);

    lua_register(L, "set_rds_rtplus_tags", lua_set_rds_rtplus_tags);
    lua_register(L, "get_rds_rtplus_tags", lua_get_rds_rtplus_tags);
    lua_register(L, "toggle_rds_rtp", lua_toggle_rds_rtp);

    lua_register(L, "set_rds_rtp_meta", lua_set_rds_rtp_meta);
    lua_register(L, "get_rds_rtp_meta", lua_get_rds_rtp_meta);

    lua_register(L, "put_rds_custom_group", lua_put_rds_custom_group);
    lua_register(L, "put_rds2_custom_group", lua_put_rds2_custom_group);

    lua_register(L, "set_rds_af_group0", lua_set_rds_af_group0);
    lua_register(L, "set_rds_af_oda", lua_set_rds_af_oda);

    lua_register(L, "set_rds_eon", lua_set_rds_eon);
    lua_register(L, "get_rds_eon", lua_get_rds_eon);

    lua_register(L, "set_rds_udg", lua_set_rds_udg);
    lua_register(L, "set_rds_udg2", lua_set_rds_udg2);

    lua_register(L, "register_oda", lua_register_oda);
    lua_register(L, "set_oda_handler", lua_set_oda_handler);

    char path[128];
    const char *home = getenv("HOME");
    if (!home) return;
    snprintf(path, sizeof(path), "%s/.rds95.command.lua", home);

    if (luaL_loadfile(L, path) != LUA_OK) {
        fprintf(stderr, "Lua error loading file: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    } else {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            printf("Init error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    lua_pushvalue(L, 1);
    if(mutex_initialized == 0) {
        pthread_mutex_init(&lua_mutex, NULL);
        mutex_initialized = 1;
    }
}

void run_lua(char *str, char *cmd_output) {
    if(lua_reload_scheduled != 0) reload_lua();
    pthread_mutex_lock(&lua_mutex);
    lua_getglobal(L, "data_handle");

    if (lua_isfunction(L, -1)) {
        lua_pushstring(L, str);
        if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
            if (lua_isstring(L, -1) && cmd_output) _strncpy(cmd_output, lua_tostring(L, -1), 254);
        } else fprintf(stderr, "Lua error: %s at 'data_handle'\n", lua_tostring(L, -1));
    } else if (lua_isstring(L, -1) && cmd_output) _strncpy(cmd_output, lua_tostring(L, -1), 254);
    lua_pop(L, 1);
    pthread_mutex_unlock(&lua_mutex);
}

int lua_group(RDSGroup* group) {
    if(lua_reload_scheduled != 0) reload_lua();
    pthread_mutex_lock(&lua_mutex);
    lua_getglobal(L, "group");

    if (lua_isfunction(L, -1)) {
        lua_pushstring(L, "L");
        lua_pushinteger(L, group->b);
        lua_pushinteger(L, group->c);
        lua_pushinteger(L, group->d);
        if (lua_pcall(L, 4, 3, 0) == LUA_OK) {
            if (!lua_isinteger(L, -1)) {
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(L, -2)) {
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(L, -3)) {
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            group->d = luaL_checkinteger(L, -1);
            group->c = luaL_checkinteger(L, -2);
            group->b = luaL_checkinteger(L, -3);
            lua_pop(L, 2);
        } else fprintf(stderr, "Lua error: %s at 'group'\n", lua_tostring(L, -1));
        lua_pop(L, 1);
    } else lua_pop(L, 1);
    pthread_mutex_unlock(&lua_mutex);
    return 1;
}

void lua_group_ref(RDSGroup* group, int ref) {
    if(lua_reload_scheduled != 0) reload_lua();
    pthread_mutex_lock(&lua_mutex);
    lua_rawgeti(L, LUA_REGISTRYINDEX, ref);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 3, 0) == LUA_OK) {
            if (!lua_isinteger(L, -1)) {
                pthread_mutex_unlock(&lua_mutex);
                lua_pop(L, 1);
                return;
            }
            if (!lua_isinteger(L, -2)) {
                pthread_mutex_unlock(&lua_mutex);
                lua_pop(L, 1);
                return;
            }
            if (!lua_isinteger(L, -3)) {
                pthread_mutex_unlock(&lua_mutex);
                lua_pop(L, 1);
                return;
            }
            group->d = luaL_checkinteger(L, -1);
            group->c = luaL_checkinteger(L, -2);
            group->b = luaL_checkinteger(L, -3);
            lua_pop(L, 3);
        } else {
            fprintf(stderr, "Lua error: %s at ref %d\n", lua_tostring(L, -1), ref);
            lua_pop(L, 1);
        }
    } else lua_pop(L, 1);
    pthread_mutex_unlock(&lua_mutex);
}

void lua_call_function(const char* function) {
    if(lua_reload_scheduled != 0) reload_lua();
    pthread_mutex_lock(&lua_mutex);
    lua_getglobal(L, function);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Lua error: %s at '%s'\n", lua_tostring(L, -1), function);
            lua_pop(L, 1);
        }
    } else lua_pop(L, 1);
    pthread_mutex_unlock(&lua_mutex);
}

void reload_lua() {
    pthread_mutex_lock(&lua_mutex);
    lua_reload_scheduled = 0;

    lua_getglobal(L, "on_unload");
    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Lua error in on_unload: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else lua_pop(L, 1);

    for (int i = 1; i < *unload_refs; i++) {
        luaL_unref(L, LUA_REGISTRYINDEX, unload_refs[i]);
    }
    *unload_refs = 1;

    if (L) {
        lua_close(L);
        L = NULL;
    }
    pthread_mutex_unlock(&lua_mutex);

    init_lua(mod);
}

void destroy_lua(void) {
    if (L) {
        for (int i = 1; i < *unload_refs; i++) luaL_unref(L, LUA_REGISTRYINDEX, unload_refs[i]);
        *unload_refs = 1;
        lua_close(L);
        L = NULL;
    }
    mod = NULL;
    pthread_mutex_destroy(&lua_mutex);
}
