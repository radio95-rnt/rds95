#include "lua_rds.h"

static RDSModulator* mod = NULL;
static lua_State *L = NULL;

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

#define INT_GETTER(name) \
int lua_get_rds_##name(lua_State *localL) { \
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].name); \
    return 1; \
}
#define STR_RAW_GETTER(name) \
int lua_get_rds_##name(lua_State *localL) { \
    lua_pushstring(localL, mod->enc->data[mod->enc->program].name); \
    return 0; \
}
INT_SETTER(pi)
INT_GETTER(pi)

INT_SETTER(pty)
INT_GETTER(pty)

INT_SETTER(ecc)
INT_GETTER(ecc)

INT_SETTER(slc_data)
INT_GETTER(slc_data)

INT_SETTER(ct)
INT_GETTER(ct)

INT_SETTER(dpty)
INT_GETTER(dpty)

INT_SETTER(tp)
INT_GETTER(tp)

INT_SETTER(ta)
INT_GETTER(ta)

INT_SETTER(rt1_enabled)
INT_GETTER(rt1_enabled)

INT_SETTER(rt2_enabled)
INT_GETTER(rt2_enabled)

INT_SETTER(ptyn_enabled)
INT_GETTER(ptyn_enabled)

INT_SETTER(rt_type)
INT_GETTER(rt_type)

int lua_set_rds_rds2mod(lua_State *localL) {
	mod->enc->encoder_data.rds2_mode = luaL_checkinteger(localL, 1);
    return 0;
}
int lua_get_rds_rds2mod(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->encoder_data.rds2_mode);
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
	mod->enc->state[mod->enc->program].eon_linkage = luaL_checkinteger(localL, 1);
    return 0;
}
int lua_get_rds_link(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->state[mod->enc->program].eon_linkage);
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
int lua_get_rds_rt_switching_period(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].rt_switching_period);
    return 1;
}

int lua_set_rds_level(lua_State *localL) {
	mod->params.level = luaL_checknumber(localL, 1);
    return 0;
}
int lua_get_rds_level(lua_State *localL) {
    lua_pushnumber(localL, mod->params.level);
    return 1;
}

STR_SETTER(ptyn, set_rds_ptyn)
STR_SETTER(ps, set_rds_ps)
STR_SETTER(tps, set_rds_tps)
STR_SETTER(rt1, set_rds_rt1)
STR_SETTER(rt2, set_rds_rt2)

STR_RAW_SETTER(lps, set_rds_lps)
STR_RAW_GETTER(lps)

STR_RAW_SETTER(ert, set_rds_ert)
STR_RAW_GETTER(ert)

void init_lua(RDSModulator* rds_mod) {
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

    lua_register(L, "set_rds_link", lua_set_rds_link);
    lua_register(L, "get_rds_link", lua_get_rds_link);

    lua_register(L, "set_rds_program", lua_set_rds_program);
    lua_register(L, "get_rds_program", lua_get_rds_program);

    lua_register(L, "set_rds_rt_switching_period", lua_set_rds_rt_switching_period);
    lua_register(L, "get_rds_rt_switching_period", lua_get_rds_rt_switching_period);

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
}

void run_lua(char *str, char *cmd_output) {
    lua_pushstring(L, str);
    lua_setglobal(L, "data");

    int top = lua_gettop(L);

    char path[128];
	snprintf(path, sizeof(path), "%s/.rds95.command.lua", getenv("HOME"));
    if (luaL_loadfilex(L, path, NULL) == LUA_OK && lua_pcall(L, 0, 1, 0) == LUA_OK) {
        if (lua_isstring(L, -1)) {
            const char * message = lua_tostring(L, -1);
            if(cmd_output) strcpy(cmd_output, message);
        }
        lua_pop(L, 1);
    } else {
        const char *err = lua_tostring(L, -1);
        fprintf(stderr, "Lua error: %s\n", err);
        lua_pop(L, 1);
        lua_settop(L, top);
    }
}

void destroy_lua(void) {
    if (L) {
        lua_close(L);
        L = NULL;
    }
    mod = NULL;
}
