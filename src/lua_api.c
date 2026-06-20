#include "lua_api.h"
#include "lua_rds.h"
#include <string.h>
#include <stddef.h>

static int in_set_defaults = 0;

extern lua_State *globalL;
extern RDSEncoder* enc;
static int writing_program = 0;

int lua_get_userdata(lua_State *L) {
    lua_pushlstring(L, (const char*)&enc->data[writing_program].lua_data, LUA_USER_DATA);
    return 1;
}
int lua_get_userdata_offset(lua_State *L) {
    uint16_t offset = luaL_checkinteger(L, 1);
    uint16_t size = luaL_checkinteger(L, 2);
    if((offset+size) > LUA_USER_DATA) return luaL_error(L, "data exceeds limit");
    lua_pushlstring(L, (const char*)&enc->data[writing_program].lua_data[offset], size);
    return 1;
}
int lua_set_userdata(lua_State *L) {
    size_t len;
    const char *data = luaL_checklstring(L, 1, &len);
    if(len > LUA_USER_DATA) return luaL_error(L, "data exceeds limit");
    memset(enc->data[writing_program].lua_data, 0, LUA_USER_DATA);
    memcpy(enc->data[writing_program].lua_data, data, len);

    return 0;
}
int lua_set_userdata_offset(lua_State *L) {
    uint16_t offset = luaL_checkinteger(L, 1);
    uint16_t size = luaL_checkinteger(L, 2);

    size_t len;
    const char *data = luaL_checklstring(L, 3, &len);
    if(len > size || (offset + size) > LUA_USER_DATA) return luaL_error(L, "data exceeds limit");
    memset(enc->data[writing_program].lua_data + offset, 0, size);
    memcpy(enc->data[writing_program].lua_data + offset, data, len);

    return 0;
}

int lua_force_save(lua_State *L) {
    (void)L;
    encoder_saveToFile(enc);
    return 0;
}

int lua_set_rds_program_defaults(lua_State *L) {
    (void)L;
    if (in_set_defaults) {
        fprintf(stderr, "Warning: Recursive call to lua_set_rds_program_defaults blocked\n");
        return 0;
    }
    in_set_defaults = 1;
    set_rds_defaults(enc, writing_program);
    lua_call_tfunction_nolock("on_init");
    lua_call_tfunction_nolock("on_state");
    in_set_defaults = 0;
    return 0;
}

int lua_reset_rds(lua_State *L) {
    (void)L;
    encoder_saveToFile(enc);

	encoder_loadFromFile(enc);
	for(int i = 0; i < PROGRAMS; i++) reset_rds_state(enc, i);
    lua_call_tfunction_nolock("on_state");
    return 0;
}

#define STR_SETTER(name, function, buffer_size) \
int lua_set_rds_##name(lua_State *L) { \
    const char* str = luaL_checklstring(L, 1, NULL); \
    char converted[buffer_size+1]; \
    convert_to_rdscharset(str, converted, buffer_size+1); \
    function(enc, converted, writing_program); \
    return 0; \
}
#define STR_RAW_SETTER(name, function) \
int lua_set_rds_##name(lua_State *L) { \
	const char* str = luaL_checklstring(L, 1, NULL); \
    function(enc, str, writing_program); \
    return 0; \
}
#define STR_RAW_SETTER_LEN(name, function) \
int lua_set_rds_##name(lua_State *L) { \
    size_t len; \
	const char* str = luaL_checklstring(L, 1, &len); \
    function(enc, str, len, writing_program); \
    return 0; \
}
#define AF_SETTER(name, af_field, af_struct, add_func) \
int lua_set_rds_##name(lua_State *L) { \
    luaL_checktype(L, 1, LUA_TTABLE); \
    \
    int n = lua_rawlen(L, 1); \
    if (n == 0) { \
        memset(&(enc->data[writing_program].af_field), 0, sizeof(af_struct)); \
        return 0; \
    } \
    if(n > 25) return luaL_error(L, "table length over 25"); \
    \
    af_struct new_af; \
    memset(&new_af, 0, sizeof(af_struct)); \
    \
    for (int i = 1; i <= n; i++) { \
        lua_rawgeti(L, 1, i); \
        if (lua_isnumber(L, -1)) add_func(&new_af, lua_tonumber(L, -1)); \
        else return luaL_error(L, "number expected, got %s", luaL_typename(L, -1)); \
        lua_pop(L, 1); \
    } \
	memcpy(&(enc->data[writing_program].af_field), &new_af, sizeof(new_af)); \
    \
    return 0; \
}

#define STR_RAW_GETTER(name, length) \
int lua_get_rds_##name(lua_State *L) { \
    lua_pushlstring(L, enc->data[writing_program].name, length); \
    return 1; \
}

int lua_set_rds_streams(lua_State *L) {
	enc->enabled_streams = luaL_checkinteger(L, 1);
    return 0;
}

int lua_get_rds_streams(lua_State *L) {
	lua_pushinteger(L, enc->enabled_streams);
    return 1;
}

int lua_set_rds_link(lua_State *L) {
    if (!lua_isboolean(L, 1)) return luaL_error(L, "boolean expected, got %s", luaL_typename(L, 1));
	enc->state[writing_program].eon_linkage = lua_toboolean(L, 1);
    return 0;
}

int lua_get_rds_link(lua_State *L) {
    lua_pushboolean(L, enc->state[writing_program].eon_linkage);
    return 1;
}

int lua_set_rds_program(lua_State *L) {
    int program = luaL_checkinteger(L, 1);
    if(program >= PROGRAMS) program = (PROGRAMS-1);
	if(program < 0) program = 0;

    if(enc->program == program) return 0;

    enc->data[enc->program].ta = 0;
	enc->data[(uint8_t)program].ta = 0;
	enc->program = (uint8_t)program;
    writing_program = program;
    return 0;
}

int lua_get_rds_program(lua_State *L) {
    lua_pushinteger(L, enc->program);
    return 1;
}

int lua_set_rds_writing_program(lua_State *L) {
    int program = luaL_checkinteger(L, 1);
    if(program >= PROGRAMS) program = (PROGRAMS-1);
	if(program < 0) program = 0;

    writing_program = program;
    return 0;
}

int lua_get_rds_writing_program(lua_State *L) {
    lua_pushinteger(L, writing_program);
    return 1;
}

int lua_put_rds_custom_group(lua_State *L) {
    if(enc->state[writing_program].custom_group[0]) return luaL_error(locaL, "group buffer full");
	enc->state[writing_program].custom_group[0] = 1;
	enc->state[writing_program].custom_group[1] = luaL_checkinteger(L, 1);
	enc->state[writing_program].custom_group[2] = luaL_checkinteger(L, 2);
	enc->state[writing_program].custom_group[3] = luaL_checkinteger(L, 3);
    return 0;
}

int lua_put_rds2_custom_group(lua_State *L) {
    if(enc->state[writing_program].custom_group2[0]) return luaL_error(locaL, "group buffer full");
	enc->state[writing_program].custom_group2[0] = 1;
	enc->state[writing_program].custom_group2[1] = luaL_checkinteger(L, 1);
	enc->state[writing_program].custom_group2[2] = luaL_checkinteger(L, 2);
	enc->state[writing_program].custom_group2[3] = luaL_checkinteger(L, 3);
	enc->state[writing_program].custom_group2[4] = luaL_checkinteger(L, 4);
    return 0;
}

STR_SETTER(ptyn, set_rds_ptyn, PTYN_LENGTH)
STR_SETTER(ps, set_rds_ps, PS_LENGTH)
STR_SETTER(tps, set_rds_tps, PS_LENGTH)
STR_SETTER(rt, set_rds_rt, RT_LENGTH)

STR_RAW_SETTER(ptyn_raw, set_rds_ptyn)
STR_RAW_SETTER(ps_raw, set_rds_ps)
STR_RAW_SETTER(tps_raw, set_rds_tps)
STR_RAW_SETTER(rt_raw, set_rds_rt)

STR_RAW_SETTER(lps, set_rds_lps)
STR_RAW_GETTER(lps, LPS_LENGTH)

STR_RAW_SETTER_LEN(grp_sqc, set_rds_grpseq)
STR_RAW_GETTER(grp_sqc, 32)

AF_SETTER(af_group0, af, RDSAFs, add_rds_af)

int lua_toggle_rt_ab(lua_State *L) {
    (void)L;
    TOGGLE(enc->state[writing_program].rt_ab);
    return 0;
}

int lua_set_rds_eon(lua_State *L) {
    int eon = luaL_checkinteger(L, 1);
    if(eon >= EONs) return luaL_error(L, "eon index exceeded");

    if(!lua_istable(L, 2)) return luaL_error(L, "table expected, got %s", luaL_typename(L, 2));

    lua_getfield(L, 2, "enabled");
    if(!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        enc->data[writing_program].eon[eon].enabled = lua_toboolean(L, -1);
    } lua_pop(L, 1);

    lua_getfield(L, 2, "tp");
    if(!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        enc->data[writing_program].eon[eon].tp = lua_toboolean(L, -1);
    } lua_pop(L, 1);

    lua_getfield(L, 2, "ta");
    if(!lua_isnil(L, -1)) {
        luaL_checktype(L, -1, LUA_TBOOLEAN);
        enc->data[writing_program].eon[eon].ta = lua_toboolean(L, -1);
    } lua_pop(L, 1);

    lua_getfield(L, 2, "pi");
    if(!lua_isnil(L, -1)) {
        lua_Integer pi = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        enc->data[writing_program].eon[eon].pi = pi;
    } else lua_pop(L, 1);

    lua_getfield(L, 2, "pty");
    if(!lua_isnil(L, -1)) {
        lua_Integer pty = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        enc->data[writing_program].eon[eon].pty = pty;
    } else lua_pop(L, 1);

    lua_getfield(L, 2, "data");
    if(!lua_isnil(L, -1)) {
        lua_Integer data = luaL_checkinteger(L, -1);
        lua_pop(L, 1);
        enc->data[writing_program].eon[eon].data = data;
    } else lua_pop(L, 1);

    lua_getfield(L, 2, "ps");
	if(!lua_isnil(L, -1)) _strncpy(enc->data[writing_program].eon[eon].ps, luaL_checklstring(L, -1, NULL), 8);
    lua_pop(L, 1);

    lua_getfield(L, 2, "afs");
    luaL_checktype(L, -1, LUA_TTABLE);

    int n = lua_rawlen(L, -1);
    if (n == 0) {
        lua_pop(L, 1);
        memset(&(enc->data[writing_program].eon[eon].af), 0, sizeof(RDSAFs));
        return 0;
    }
    if(n > 25) return luaL_error(L, "table length over 25");

    RDSAFs new_af;
    memset(&new_af, 0, sizeof(RDSAFs));

    for (int i = 1; i <= n; i++) {
        lua_rawgeti(L, -1, i);
        if (!lua_isnumber(L, -1)) {
            const char *type = luaL_typename(L, -1);
            lua_pop(L, 1);
            return luaL_error(L, "number expected, got %s", type);
        }
        add_rds_af(&new_af, lua_tonumber(L, -1));
        lua_pop(L, 1);
    }
	memcpy(&(enc->data[writing_program].eon[eon].af), &new_af, sizeof(new_af));
    lua_pop(L, 1);
    return 0;
}

int lua_get_rds_eon(lua_State *L) {
    int eon = luaL_checkinteger(L, 1);
    if(eon >= EONs) return luaL_error(L, "eon index exceeded");

    lua_newtable(L);
    lua_pushboolean(L, enc->data[writing_program].eon[eon].enabled);
    lua_setfield(L, -2, "enabled");
    lua_pushinteger(L, enc->data[writing_program].eon[eon].pi);
    lua_setfield(L, -2, "pi");
    lua_pushboolean(L, enc->data[writing_program].eon[eon].tp);
    lua_setfield(L, -2, "tp");
    lua_pushboolean(L, enc->data[writing_program].eon[eon].ta);
    lua_setfield(L, -2, "ta");
    lua_pushinteger(L, enc->data[writing_program].eon[eon].pty);
    lua_setfield(L, -2, "pty");
    lua_pushlstring(L, enc->data[writing_program].eon[eon].ps, 8);
    lua_setfield(L, -2, "ps");
    lua_newtable(L); // don't have decoding for AF, so just return empty table
    lua_setfield(L, -2, "afs");
    lua_pushinteger(L, enc->data[writing_program].eon[eon].data);
    lua_setfield(L, -2, "data");
    return 1;
}

int lua_crc16(lua_State *L) {
    size_t len;
    const char* data = luaL_checklstring(L, 1, &len);
    lua_pushinteger(L, crc16_ccitt(data, len));
    return 1;
}

int lua_convert_to_rdscharset(lua_State *L) {
    size_t len;
    const char *input = luaL_checklstring(L, 1, &len);
    
    char output[len + 1];
    convert_to_rdscharset(input, output, len + 1);
    
    lua_pushstring(L, output);
    return 1;
}

int lua_encode_group(lua_State *L) {
    size_t len;
    const char *grp = luaL_checklstring(L, 1, &len);
    if(len != 1) return luaL_error(L, "expected a length of 1");
    char PS_GROUP = 0;

    RDSGroup group;
    uint8_t good = check_rds_good_group(enc, grp);
    if(good == 0) {
        lua_pushboolean(L, 0);
        get_rds_sequence_group(enc, &group, 1, &PS_GROUP);
    } else {
        lua_pushboolean(L, 1);
        get_rds_sequence_group(enc, &group, 1, grp);
    }

    lua_pushinteger(L, group.b);
    lua_pushinteger(L, group.c);
    lua_pushinteger(L, group.d);
    
    return 4;
}

typedef void (*lua_push_fn)(lua_State *L, const void *value);
typedef void (*lua_pull_fn)(lua_State *L, int idx, void *value);

static void push_int(lua_State *L, const void *value) { lua_pushinteger(L, *(const int *)value); }
static void push_bool(lua_State *L, const void *value) { lua_pushboolean(L, *(const int *)value); }
static void pull_int(lua_State *L, int idx, void *value) { *(int *)value = luaL_checkinteger(L, idx); }
static void pull_bool(lua_State *L, int idx, void *value) { *(int *)value = lua_toboolean(L, idx); }

typedef struct {
    const char *name;
    int offset;
    lua_push_fn push;
    lua_pull_fn pull;
} field_t;

static const field_t fields[] = {
    {"pi", offsetof(RDSData, pi), push_int,  pull_int},
    {"pty", offsetof(RDSData, pty), push_int,  pull_int},
    {"ecc", offsetof(RDSData, ecc), push_int,  pull_int},
    {"slc_data", offsetof(RDSData, slc_data), push_int,  pull_int},
    {"ct", offsetof(RDSData, ct), push_bool, pull_bool},
    {"dpty", offsetof(RDSData, dpty), push_bool, pull_bool},
    {"tp", offsetof(RDSData, tp), push_bool, pull_bool},
    {"ta", offsetof(RDSData, ta), push_bool, pull_bool},
    {"rt_enabled", offsetof(RDSData, rt_enabled), push_bool, pull_bool},
    {"ptyn_enabled", offsetof(RDSData, ptyn_enabled), push_bool, pull_bool},
};

int lua_rds__index(lua_State *L) {
    const char *key = luaL_checkstring(L, 2);

    RDSData *data = &enc->data[writing_program];

    for (size_t i = 0; i < sizeof(fields)/sizeof(fields[0]); i++) {
        if (strcmp(key, fields[i].name) == 0) {
            const void *ptr = (const char *)data + fields[i].offset;
            fields[i].push(L, ptr);
            return 1;
        }
    }

    lua_rawget(L, 1);
    return 1;
}

int lua_rds__newindex(lua_State *L) {
    const char *key = luaL_checkstring(L, 2);

    RDSData *data = &enc->data[writing_program];

    for (size_t i = 0; i < sizeof(fields)/sizeof(fields[0]); i++) {
        if (strcmp(key, fields[i].name) == 0) {
            void *ptr = (char *)data + fields[i].offset;
            fields[i].pull(L, 3, ptr);
            return 0;
        }
    }

    lua_rawset(L, 1);
    return 0;
}