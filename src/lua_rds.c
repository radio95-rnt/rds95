#include "lua_rds.h"
#include <pthread.h>
#include "lua_api.h"

RDSEncoder* enc = NULL;
lua_State *globalL = NULL;
int hooks_ref = LUA_NOREF;
static pthread_mutex_t lua_mutex;

#define lua_registertotable(L,n,f) (lua_pushcfunction(L, (f)), lua_setfield(L, -2, (n)))
int init_lua_userdata(lua_State* L) {
    lua_newtable(L);
    lua_registertotable(L, "get", lua_get_userdata);
    lua_registertotable(L, "get_offset", lua_get_userdata_offset);
    lua_registertotable(L, "set", lua_set_userdata);
    lua_registertotable(L, "set_offset", lua_set_userdata_offset);
    lua_pushinteger(L, LUA_USER_DATA);
    lua_setfield(L, -2, "len");
    return 1;
}

int init_lua_data(lua_State* L) {
    lua_newtable(L);
    lua_registertotable(L, "crc16", lua_crc16);
    lua_registertotable(L, "force_save", lua_force_save);
    lua_registertotable(L, "reset_rds", lua_reset_rds);
    lua_registertotable(L, "set_program_defaults", lua_set_rds_program_defaults);
    lua_pushinteger(L, PROGRAMS);
    lua_setfield(L, -2, "max_programs");
    lua_pushstring(L, VERSION);
    lua_setfield(L, -2, "core_version");
    lua_registertotable(L, "set_output_program", lua_set_rds_program);
    lua_registertotable(L, "get_output_program", lua_get_rds_program);
    lua_registertotable(L, "set_writing_program", lua_set_rds_writing_program);
    lua_registertotable(L, "get_writing_program", lua_get_rds_writing_program);
    lua_registertotable(L, "encode_charset", lua_convert_to_rdscharset);
    return 1;
}

static int my_searcher(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);

    #define PREFIX "/etc/rds95/"

    char path[512];
    snprintf(path, sizeof(path),
             PREFIX "%s.lua",
             name);

    for (char *p = path + strlen(PREFIX); *p; p++) {
        if (*p == '.') *p = '/';
    }

    FILE *f = fopen(path, "rb");
    if (!f) {
        lua_pushfstring(L, "\n\tno file: %s", path);
        return 1;
    }

    fclose(f);

    if (luaL_loadfile(L, path) == LUA_OK) {
        return 1;
    }

    lua_pushfstring(L, "\n\terror loading: %s", path);
    return 1;
}

uint8_t init_lua(RDSEncoder* _enc) {
    static int mutex_initialized = 0;
    enc = _enc;
    globalL = luaL_newstate();
    printf("Initializing %s\n", LUA_VERSION);
    if(globalL == NULL) return 1;

    luaL_requiref(globalL, "_G", luaopen_base, 1);
    luaL_requiref(globalL, LUA_STRLIBNAME, luaopen_string, 1);
    luaL_requiref(globalL, LUA_TABLIBNAME, luaopen_table, 1);
    luaL_requiref(globalL, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    luaL_requiref(globalL, LUA_COLIBNAME, luaopen_coroutine, 1);
    luaL_requiref(globalL, LUA_MATHLIBNAME, luaopen_math, 1);
    luaL_requiref(globalL, LUA_IOLIBNAME, luaopen_io, 1);
    luaL_requiref(globalL, "userdata", init_lua_userdata, 1);
    luaL_requiref(globalL, "Data", init_lua_data, 1);
    lua_pop(globalL, 8);

    luaL_requiref(globalL, LUA_LOADLIBNAME, luaopen_package, 1);
    lua_newtable(globalL);
    lua_pushcfunction(globalL, my_searcher);
    lua_rawseti(globalL, -2, 1);
    lua_setfield(globalL, -2, "searchers");
    lua_pop(globalL, 1);

    lua_newtable(globalL);
    lua_newtable(globalL);
    lua_setfield(globalL, -2, "on_init");
    lua_newtable(globalL);
    lua_setfield(globalL, -2, "on_start");
    lua_newtable(globalL);
    lua_setfield(globalL, -2, "on_state");
    lua_newtable(globalL);
    lua_setfield(globalL, -2, "tick");
    lua_newtable(globalL);
    lua_setfield(globalL, -2, "minute_tick");
    lua_newtable(globalL);
    lua_setfield(globalL, -2, "rt_transmission");
    lua_newtable(globalL);
    lua_setfield(globalL, -2, "ps_transmission");
    lua_pushvalue(globalL, -1);
    hooks_ref = luaL_ref(globalL, LUA_REGISTRYINDEX);
    lua_setglobal(globalL, "hooks");

    lua_newtable(globalL);
    lua_setglobal(globalL, "ext");

    lua_newtable(globalL);

    lua_newtable(globalL);
    lua_setfield(globalL, -2, "ext");

    lua_pushinteger(globalL, EONs);
    lua_setfield(globalL, -2, "eon_count");

    lua_newtable(globalL);
    lua_pushcfunction(globalL, lua_rds__index);
    lua_setfield(globalL, -2, "__index");
    lua_pushcfunction(globalL, lua_rds__newindex);
    lua_setfield(globalL, -2, "__newindex");
    lua_setmetatable(globalL, -2);

    lua_registertotable(globalL, "set_link", lua_set_rds_link);
    lua_registertotable(globalL, "get_link", lua_get_rds_link);

    lua_registertotable(globalL, "set_ptyn", lua_set_rds_ptyn);
    lua_registertotable(globalL, "set_ptyn_raw", lua_set_rds_ptyn_raw);
    lua_registertotable(globalL, "set_ps", lua_set_rds_ps);
    lua_registertotable(globalL, "set_ps_raw", lua_set_rds_ps_raw);
    lua_registertotable(globalL, "set_tps", lua_set_rds_tps);
    lua_registertotable(globalL, "set_tps_raw", lua_set_rds_tps_raw);
    lua_registertotable(globalL, "set_rt", lua_set_rds_rt);
    lua_registertotable(globalL, "set_rt_raw", lua_set_rds_rt_raw);
    lua_registertotable(globalL, "toggle_rt_ab", lua_toggle_rt_ab);

    lua_registertotable(globalL, "set_lps", lua_set_rds_lps);
    lua_registertotable(globalL, "get_lps", lua_get_rds_lps);

    lua_registertotable(globalL, "set_grpseq", lua_set_rds_grp_sqc);
    lua_registertotable(globalL, "get_grpseq", lua_get_rds_grp_sqc);

    lua_registertotable(globalL, "put_custom_group", lua_put_rds_custom_group);
    lua_registertotable(globalL, "put_rds2_custom_group", lua_put_rds2_custom_group);

    lua_registertotable(globalL, "set_af", lua_set_rds_af_group0);

    lua_registertotable(globalL, "set_eon", lua_set_rds_eon);
    lua_registertotable(globalL, "get_eon", lua_get_rds_eon);

    lua_registertotable(globalL, "set_streams", lua_set_rds_streams);
    lua_registertotable(globalL, "get_streams", lua_get_rds_streams);

    lua_registertotable(globalL, "encode_group", lua_encode_group);

    lua_setglobal(globalL, "RDS");

    if (luaL_loadfile(globalL, "/etc/rds95.lua") != LUA_OK) {
        fprintf(stderr, "Lua error loading file: %s\n", lua_tostring(globalL, -1));
        lua_pop(globalL, 1);
        return 2;
    } else {
        if (lua_pcall(globalL, 0, 0, 0) != LUA_OK) {
            printf("Init error: %s\n", lua_tostring(globalL, -1));
            lua_pop(globalL, 1);
        }
    }
    if(mutex_initialized == 0) {
        pthread_mutex_init(&lua_mutex, NULL);
        mutex_initialized = 1;
    }

    return 0;
}

void run_lua(char *str, size_t str_len, char *cmd_output, size_t *out_len) {
    pthread_mutex_lock(&lua_mutex);

    lua_rawgeti(globalL, LUA_REGISTRYINDEX, hooks_ref);
    lua_getfield(globalL, -1, "data_handle");

    if (lua_isfunction(globalL, -1)) {
        lua_pushlstring(globalL, str, str_len);

        if (lua_pcall(globalL, 1, 1, 0) == LUA_OK) {
            if (lua_isstring(globalL, -1) && cmd_output) {
                const char *lua_str = lua_tolstring(globalL, -1, out_len);
                if (*out_len > 254) *out_len = 254;
                memcpy(cmd_output, lua_str, *out_len);
            }
        } else fprintf(stderr, "Lua error: %s at 'data_handle'\n", lua_tostring(globalL, -1));
    } else fprintf(stderr, "'data_handle' is not a function\n");

    lua_pop(globalL, 2);
    pthread_mutex_unlock(&lua_mutex);
}

int lua_group(RDSGroup* group, const char grp) {
    pthread_mutex_lock(&lua_mutex);
    lua_rawgeti(globalL, LUA_REGISTRYINDEX, hooks_ref);
    lua_getfield(globalL, -1, "group");

    if (!lua_isfunction(globalL, -1)) {
        lua_pop(globalL, 2);
        pthread_mutex_unlock(&lua_mutex);
        return 0;
    }

    lua_pushlstring(globalL, &grp, 1);

    if (lua_pcall(globalL, 1, 4, 0) != LUA_OK) {
        fprintf(stderr, "Lua error: %s\n", lua_tostring(globalL, -1));
        lua_pop(globalL, 2);
        pthread_mutex_unlock(&lua_mutex);
        return 0;
    }

    int success = 0;
    if (lua_isboolean(globalL, -4) && lua_toboolean(globalL, -4) &&
        lua_isinteger(globalL, -3) && lua_isinteger(globalL, -2) && lua_isinteger(globalL, -1)) {
        group->b = (uint16_t)lua_tointeger(globalL, -3);
        group->c = (uint16_t)lua_tointeger(globalL, -2);
        group->d = (uint16_t)lua_tointeger(globalL, -1);
        success = 1;
    }

    lua_pop(globalL, 5);
    pthread_mutex_unlock(&lua_mutex);
    return success;
}

int lua_rds2_group(RDSGroup* group, int stream) {
    pthread_mutex_lock(&lua_mutex);
    lua_rawgeti(globalL, LUA_REGISTRYINDEX, hooks_ref);
    lua_getfield(globalL, -1, "rds2_group");

    if (lua_isfunction(globalL, -1)) {
        lua_pushinteger(globalL, stream);
        if (lua_pcall(globalL, 1, 5, 0) == LUA_OK) {
            if (!lua_isboolean(globalL, -5)) {
                lua_pop(globalL, 6);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(globalL, -4)) {
                lua_pop(globalL, 6);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(globalL, -3)) {
                lua_pop(globalL, 6);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(globalL, -2)) {
                lua_pop(globalL, 6);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(globalL, -1)) {
                lua_pop(globalL, 6);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }

            if(lua_toboolean(globalL, -5) == 0) {
                lua_pop(globalL, 6);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }

            group->a = lua_tointeger(globalL, -4);
            group->b = lua_tointeger(globalL, -3);
            group->c = lua_tointeger(globalL, -2);
            group->d = lua_tointeger(globalL, -1);
            lua_pop(globalL, 6);
        } else {
            fprintf(stderr, "Lua error: %s at 'rds2_group'\n", lua_tostring(globalL, -1));
            lua_pop(globalL, 2);
        }
    } else lua_pop(globalL, 2);

    pthread_mutex_unlock(&lua_mutex);
    return 1;
}

void lua_call_function_nolock(const char* function) {
    lua_getglobal(globalL, function);

    if (lua_isfunction(globalL, -1)) {
        if (lua_pcall(globalL, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Lua error: %s at '%s'\n", lua_tostring(globalL, -1), function);
            lua_pop(globalL, 1);
        }
    } else lua_pop(globalL, 1);
}
void lua_call_function(const char* function) {
    int need_lock = (pthread_mutex_trylock(&lua_mutex) == 0);
    if (!need_lock) {
        fprintf(stderr, "Warning: lua_mutex already locked when calling %s\n", function);
        return;
    }
    lua_call_function_nolock(function);
    pthread_mutex_unlock(&lua_mutex);
}

void lua_call_tfunction_nolock(const char* name) {
    lua_rawgeti(globalL, LUA_REGISTRYINDEX, hooks_ref);
    if (!lua_istable(globalL, -1)) {
        lua_pop(globalL, 1);
        return;
    }

    lua_getfield(globalL, -1, name);

    if (!lua_istable(globalL, -1)) {
        lua_pop(globalL, 2);
        return;
    }

    lua_Integer len = lua_rawlen(globalL, -1);
    for (lua_Integer i = 1; i <= len; i++) {
        lua_rawgeti(globalL, -1, i);
        if (lua_isfunction(globalL, -1)) {
            if (lua_pcall(globalL, 0, 0, 0) != LUA_OK) {
                fprintf(stderr, "Lua error: %s at '%s[%lld]'\n",
                        lua_tostring(globalL, -1), name, (long long)i);
                lua_pop(globalL, 1);
            }
        } else lua_pop(globalL, 1);
    }
    lua_pop(globalL, 2); // pop table
}

void lua_call_tfunction(const char* name) {
    int need_lock = (pthread_mutex_trylock(&lua_mutex) == 0);
    if (!need_lock) {
        fprintf(stderr, "Warning: lua_mutex already locked when table tcalling %s\n", name);
        return;
    }
    lua_call_tfunction_nolock(name);
    pthread_mutex_unlock(&lua_mutex);
}

void destroy_lua() {
    if (globalL) {
        luaL_unref(globalL, LUA_REGISTRYINDEX, hooks_ref);
        lua_close(globalL);
        globalL = NULL;
    }
    enc = NULL;
    pthread_mutex_destroy(&lua_mutex);
}
