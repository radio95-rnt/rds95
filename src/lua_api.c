#include "lua_api.h"
#include "lua_rds.h"

static int in_set_defaults = 0;

extern lua_State *L;
extern RDSModulator* mod;
extern uint8_t unload_refs[33];

int lua_get_userdata(lua_State *localL) {
    lua_pushlstring(localL, (const char*)&mod->enc->data[mod->enc->program].lua_data, LUA_USER_DATA);
    return 1;
}
int lua_get_userdata_offset(lua_State *localL) {
    uint16_t offset = luaL_checkinteger(localL, 1);
    uint16_t size = luaL_checkinteger(localL, 2);
    if((offset+size) > LUA_USER_DATA) return luaL_error(localL, "data exceeds limit");
    lua_pushlstring(localL, (const char*)&mod->enc->data[mod->enc->program].lua_data[offset], size);
    return 1;
}
int lua_set_userdata(lua_State *localL) {
    size_t len;
    const char *data = luaL_checklstring(localL, 1, &len);
    if(len > LUA_USER_DATA) return luaL_error(localL, "data exceeds limit");
    memset(mod->enc->data[mod->enc->program].lua_data, 0, LUA_USER_DATA);
    memcpy(mod->enc->data[mod->enc->program].lua_data, data, len);

    return 0;
}
int lua_set_userdata_offset(lua_State *localL) {
    uint16_t offset = luaL_checkinteger(localL, 1);
    uint16_t size = luaL_checkinteger(localL, 2);

    size_t len;
    const char *data = luaL_checklstring(localL, 3, &len);
    if(len > size || (offset + size) > LUA_USER_DATA) return luaL_error(localL, "data exceeds limit");
    memset(mod->enc->data[mod->enc->program].lua_data + offset, 0, size);
    memcpy(mod->enc->data[mod->enc->program].lua_data + offset, data, len);

    return 0;
}

int lua_force_save(lua_State *localL) {
    (void)localL;
    encoder_saveToFile(mod->enc);
    Modulator_saveToFile(&mod->params);
    return 0;
}

int lua_set_rds_program_defaults(lua_State *localL) {
    (void)localL;
    if (in_set_defaults) {
        fprintf(stderr, "Warning: Recursive call to lua_set_rds_program_defaults blocked\n");
        return 0;
    }
    in_set_defaults = 1;
    for (int i = 1; i < *unload_refs; i++) luaL_unref(L, LUA_REGISTRYINDEX, unload_refs[i]);
    unload_refs[0] = 1;
    set_rds_defaults(mod->enc, mod->enc->program);
    lua_call_tfunction_nolock("on_init");
    lua_call_tfunction_nolock("on_state");
    in_set_defaults = 0;
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
    lua_call_tfunction_nolock("on_state");
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

int lua_set_rds2_mode(lua_State *localL) {
	mod->enc->encoder_data.rds2_mode = luaL_checkinteger(localL, 1);
    return 0;
}

int lua_get_rds2_mode(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->encoder_data.rds2_mode);
    return 1;
}

int lua_set_rds_streams(lua_State *localL) {
	mod->params.rdsgen = luaL_checkinteger(localL, 1);
    return 0;
}

int lua_get_rds_streams(lua_State *localL) {
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

STR_SETTER(ptyn, set_rds_ptyn)
STR_SETTER(ps, set_rds_ps)
STR_SETTER(tps, set_rds_tps)
STR_SETTER(rt1, set_rds_rt1)
STR_SETTER(rt2, set_rds_rt2)
STR_SETTER(default_rt, set_rds_default_rt)

STR_RAW_SETTER(lps, set_rds_lps)
STR_RAW_GETTER(lps, LPS_LENGTH)

STR_RAW_SETTER(grp_sqc_rds2, set_rds_grpseq2)
STR_RAW_GETTER(grp_sqc_rds2, 32)

int lua_set_rds_grp_sqc(lua_State *localL) {
	const char* str = luaL_checklstring(localL, 1, NULL);
    if(_strnlen(str, 2) < 1) set_rds_grpseq(mod->enc, DEFAULT_GRPSQC);
    else set_rds_grpseq(mod->enc, str);
    return 0;
}

STR_RAW_GETTER(grp_sqc, 32)

AF_SETTER(af_group0, af, RDSAFs, add_rds_af)

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

int lua_get_available_rds_streams(lua_State *localL) {
    lua_pushinteger(localL, mod->num_streams);
    return 1;
}

int lua_crc16(lua_State *localL) {
    size_t len;
    const char* data = luaL_checklstring(localL, 1, &len);
    lua_pushinteger(localL, crc16_ccitt(data, len));
    return 1;
}