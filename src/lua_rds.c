#include "lua_rds.h"

static RDSModulator* mod = NULL;
static lua_State *L = NULL;

int lua_set_rds_pi(lua_State *localL) {
    int pi_value = luaL_checkinteger(localL, 1);
	mod->enc->data[mod->enc->program].pi = pi_value;
    return 0;
}

void init_lua(RDSModulator* rds_mod) {
    mod = rds_mod;
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_register(L, "set_rds_pi", lua_set_rds_pi);
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
