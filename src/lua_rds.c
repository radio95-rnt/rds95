#include "lua_rds.h"

static RDSModulator* mod = NULL;
static lua_State *L = NULL;

#define INT_NONRETURN_HANDLER(name) \
int lua_set_rds_##name(lua_State *localL) { \
	mod->enc->data[mod->enc->program].name = luaL_checkinteger(localL, 1); \
    return 0; \
}
#define STR_NONRETURN_HANDLER(name, function) \
int lua_set_rds_##name(lua_State *localL) { \
	const char* str = luaL_checklstring(localL, 1, NULL); \
    function(mod->enc, str); \
    return 0; \
}


INT_NONRETURN_HANDLER(pi)
INT_NONRETURN_HANDLER(pty)
INT_NONRETURN_HANDLER(ecc)
INT_NONRETURN_HANDLER(slc_data)
INT_NONRETURN_HANDLER(ct)
INT_NONRETURN_HANDLER(dpty)
INT_NONRETURN_HANDLER(tp)
INT_NONRETURN_HANDLER(ta)
INT_NONRETURN_HANDLER(rt1_enabled)
INT_NONRETURN_HANDLER(rt2_enabled)
INT_NONRETURN_HANDLER(ptyn_enabled)
INT_NONRETURN_HANDLER(rt_type)
int lua_set_rds_rds2mod(lua_State *localL) {
	mod->enc->encoder_data.rds2_mode = luaL_checkinteger(localL, 1);
    return 0;
}
int lua_set_rds_rdsgen(lua_State *localL) {
	mod->params.rdsgen = luaL_checkinteger(localL, 1);
    return 0;
}

STR_NONRETURN_HANDLER(ptyn, set_rds_ptyn)

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

    lua_register(L, "set_rds_pi", lua_set_rds_pi);
    lua_register(L, "set_rds_pty", lua_set_rds_pty);
    lua_register(L, "set_rds_ecc", lua_set_rds_ecc);
    lua_register(L, "set_rds_slc_data", lua_set_rds_slc_data);
    lua_register(L, "set_rds_ct", lua_set_rds_ct);
    lua_register(L, "set_rds_dpty", lua_set_rds_dpty);
    lua_register(L, "set_rds_tp", lua_set_rds_tp);
    lua_register(L, "set_rds_ta", lua_set_rds_ta);
    lua_register(L, "set_rds_rt1_enabled", lua_set_rds_rt1_enabled);
    lua_register(L, "set_rds_rt2_enabled", lua_set_rds_rt2_enabled);
    lua_register(L, "set_rds_ptyn_enabled", lua_set_rds_ptyn_enabled);
    lua_register(L, "set_rds_rt_type", lua_set_rds_rt_type);
    lua_register(L, "set_rds_rds2mod", lua_set_rds_rds2mod);
    lua_register(L, "set_rds_rdsgen", lua_set_rds_rdsgen);
    lua_register(L, "set_rds_ptyn", lua_set_rds_ptyn);
}

void run_lua(char *str, char *cmd_output) {
    lua_pushstring(L, str);
    lua_setglobal(L, "cmd");

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
