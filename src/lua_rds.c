#include "lua_rds.h"
#include <pthread.h>
#include "lua_api.h"

RDSModulator* mod = NULL;
lua_State *L = NULL;
static pthread_mutex_t lua_mutex;
uint8_t unload_refs[33] = {LUA_REFNIL};

#define lua_registertotable(L,n,f) (lua_pushcfunction(L, (f)), lua_setfield(L, -2, (n)))
void init_lua(RDSModulator* rds_mod) {
    static int mutex_initialized = 0;
    mod = rds_mod;
    L = luaL_newstate();
    printf("Initializing %s\n", LUA_COPYRIGHT);

    luaL_requiref(L, "_G", luaopen_base, 1);
    luaL_requiref(L, LUA_STRLIBNAME, luaopen_string, 1);
    luaL_requiref(L, LUA_TABLIBNAME, luaopen_table, 1);
    luaL_requiref(L, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    luaL_requiref(L, LUA_COLIBNAME, luaopen_coroutine, 1);
    luaL_requiref(L, LUA_MATHLIBNAME, luaopen_math, 1);
    luaL_requiref(L, LUA_IOLIBNAME, luaopen_io, 1);
    lua_pop(L, 6);

    lua_pushstring(L, VERSION);
    lua_setglobal(L, "core_version");

    lua_newtable(L);
    lua_setglobal(L, "ext");

    lua_newtable(L);
    lua_setglobal(L, "on_inits");
    lua_newtable(L);
    lua_setglobal(L, "on_starts");
    lua_newtable(L);
    lua_setglobal(L, "on_states");
    lua_newtable(L);
    lua_setglobal(L, "ticks");

    lua_newtable(L);
    lua_registertotable(L, "get", lua_get_userdata);
    lua_registertotable(L, "get_offset", lua_get_userdata_offset);
    lua_registertotable(L, "set", lua_set_userdata);
    lua_registertotable(L, "set_offset", lua_set_userdata_offset);
    lua_pushinteger(L, LUA_USER_DATA);
    lua_setfield(L, -2, "len");
    lua_setglobal(L, "userdata");

    lua_newtable(L);
    lua_registertotable(L, "crc16", lua_crc16);
    lua_registertotable(L, "force_save", lua_force_save);
    lua_registertotable(L, "reset_rds", lua_reset_rds);
    lua_registertotable(L, "set_program_defaults", lua_set_rds_program_defaults);
    lua_pushinteger(L, PROGRAMS);
    lua_setfield(L, -2, "max_programs");
    lua_registertotable(L, "set_program", lua_set_rds_program);
    lua_registertotable(L, "get_program", lua_get_rds_program);
    lua_setglobal(L, "dp");

    lua_newtable(L);

    lua_newtable(L);
    lua_setfield(L, -2, "ext");

    lua_pushinteger(L, EONs);
    lua_setfield(L, -2, "eon_count");

    lua_registertotable(L, "set_pi", lua_set_rds_pi);
    lua_registertotable(L, "get_pi", lua_get_rds_pi);

    lua_registertotable(L, "set_pty", lua_set_rds_pty);
    lua_registertotable(L, "get_pty", lua_get_rds_pty);

    lua_registertotable(L, "set_ecc", lua_set_rds_ecc);
    lua_registertotable(L, "get_ecc", lua_get_rds_ecc);

    lua_registertotable(L, "set_slc_data", lua_set_rds_slc_data);
    lua_registertotable(L, "get_slc_data", lua_get_rds_slc_data);

    lua_registertotable(L, "set_ct", lua_set_rds_ct);
    lua_registertotable(L, "get_ct", lua_get_rds_ct);

    lua_registertotable(L, "set_dpty", lua_set_rds_dpty);
    lua_registertotable(L, "get_dpty", lua_get_rds_dpty);

    lua_registertotable(L, "set_tp", lua_set_rds_tp);
    lua_registertotable(L, "get_tp", lua_get_rds_tp);

    lua_registertotable(L, "set_ta", lua_set_rds_ta);
    lua_registertotable(L, "get_ta", lua_get_rds_ta);

    lua_registertotable(L, "set_rt1_enabled", lua_set_rds_rt1_enabled);
    lua_registertotable(L, "get_rt1_enabled", lua_get_rds_rt1_enabled);

    lua_registertotable(L, "set_rt2_enabled", lua_set_rds_rt2_enabled);
    lua_registertotable(L, "get_rt2_enabled", lua_get_rds_rt2_enabled);

    lua_registertotable(L, "set_ptyn_enabled", lua_set_rds_ptyn_enabled);
    lua_registertotable(L, "get_ptyn_enabled", lua_get_rds_ptyn_enabled);

    lua_registertotable(L, "set_rt_type", lua_set_rds_rt_type);
    lua_registertotable(L, "get_rt_type", lua_get_rds_rt_type);

    lua_registertotable(L, "set_rds2_mode", lua_set_rds2_mode);
    lua_registertotable(L, "get_rds2_mode", lua_get_rds2_mode);

    lua_registertotable(L, "set_link", lua_set_rds_link);
    lua_registertotable(L, "get_link", lua_get_rds_link);

    lua_registertotable(L, "set_rt_switching_period", lua_set_rds_rt_switching_period);
    lua_registertotable(L, "get_rt_switching_period", lua_get_rds_rt_switching_period);

    lua_registertotable(L, "set_rt_text_timeout", lua_set_rds_rt_text_timeout);
    lua_registertotable(L, "get_rt_text_timeout", lua_get_rds_rt_text_timeout);

    lua_registertotable(L, "set_ptyn", lua_set_rds_ptyn);
    lua_registertotable(L, "set_ps", lua_set_rds_ps);
    lua_registertotable(L, "set_tps", lua_set_rds_tps);
    lua_registertotable(L, "set_rt1", lua_set_rds_rt1);
    lua_registertotable(L, "set_rt2", lua_set_rds_rt2);
    lua_registertotable(L, "set_default_rt", lua_set_rds_default_rt);

    lua_registertotable(L, "set_lps", lua_set_rds_lps);
    lua_registertotable(L, "get_lps", lua_get_rds_lps);

    lua_registertotable(L, "set_grpseq", lua_set_rds_grp_sqc);
    lua_registertotable(L, "get_grpseq", lua_get_rds_grp_sqc);

    lua_registertotable(L, "set_grpseq2", lua_set_rds_grp_sqc_rds2);
    lua_registertotable(L, "get_grpseq2", lua_get_rds_grp_sqc_rds2);

    lua_registertotable(L, "put_custom_group", lua_put_rds_custom_group);
    lua_registertotable(L, "put_rds2_custom_group", lua_put_rds2_custom_group);

    lua_registertotable(L, "set_af", lua_set_rds_af_group0);

    lua_registertotable(L, "set_eon", lua_set_rds_eon);
    lua_registertotable(L, "get_eon", lua_get_rds_eon);

    lua_registertotable(L, "set_udg", lua_set_rds_udg);
    lua_registertotable(L, "set_udg2", lua_set_rds_udg2);

    lua_registertotable(L, "set_level", lua_set_rds_level);
    lua_registertotable(L, "get_level", lua_get_rds_level);

    lua_registertotable(L, "set_streams", lua_set_rds_streams);
    lua_registertotable(L, "get_streams", lua_get_rds_streams);

    lua_registertotable(L, "get_available_streams", lua_get_available_rds_streams);

    lua_setglobal(L, "rds");


    if (luaL_loadfile(L, "/etc/rds95.lua") != LUA_OK) {
        fprintf(stderr, "Lua error loading file: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        return;
    } else {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            printf("Init error: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }
    if(mutex_initialized == 0) {
        pthread_mutex_init(&lua_mutex, NULL);
        mutex_initialized = 1;
    }
}

void run_lua(char *str, char *cmd_output, size_t* out_len) {
    pthread_mutex_lock(&lua_mutex);
    lua_getglobal(L, "data_handle");

    if (lua_isfunction(L, -1)) {
        lua_pushstring(L, str);
        if (lua_pcall(L, 1, 1, 0) == LUA_OK) {
            if (lua_isstring(L, -1) && cmd_output) _strncpy(cmd_output, lua_tolstring(L, -1, out_len), 254);
        } else fprintf(stderr, "Lua error: %s at 'data_handle'\n", lua_tostring(L, -1));
    } else if (lua_isstring(L, -1) && cmd_output) _strncpy(cmd_output, lua_tolstring(L, -1, out_len), 254);
    lua_pop(L, 1);
    pthread_mutex_unlock(&lua_mutex);
}

int lua_group(RDSGroup* group, const char grp) {
    pthread_mutex_lock(&lua_mutex);
    lua_getglobal(L, "group");

    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        pthread_mutex_unlock(&lua_mutex);
        return 0;
    }

    lua_pushlstring(L, &grp, 1);

    if (lua_pcall(L, 1, 4, 0) != LUA_OK) {
        fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
        lua_pop(L, 1);
        pthread_mutex_unlock(&lua_mutex);
        return 0;
    }

    int success = 0;
    if (lua_isboolean(L, -4) && lua_toboolean(L, -4) &&
        lua_isinteger(L, -3) && lua_isinteger(L, -2) && lua_isinteger(L, -1)) {

        group->b = (uint16_t)lua_tointeger(L, -3);
        group->c = (uint16_t)lua_tointeger(L, -2);
        group->d = (uint16_t)lua_tointeger(L, -1);
        success = 1;
    }

    lua_pop(L, 4);
    pthread_mutex_unlock(&lua_mutex);
    return success;
}

int lua_rds2_group(RDSGroup* group, int stream) {
    pthread_mutex_lock(&lua_mutex);
    lua_getglobal(L, "rds2_group");

    if (lua_isfunction(L, -1)) {
        lua_pushinteger(L, stream);
        if (lua_pcall(L, 1, 5, 0) == LUA_OK) {
            if (!lua_isboolean(L, -5)) {
                lua_pop(L, 5);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(L, -4)) {
                lua_pop(L, 5);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(L, -3)) {
                lua_pop(L, 5);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(L, -2)) {
                lua_pop(L, 5);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }
            if (!lua_isinteger(L, -1)) {
                lua_pop(L, 5);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }

            if(lua_toboolean(L, -5) == 0) {
                lua_pop(L, 5);
                pthread_mutex_unlock(&lua_mutex);
                return 0;
            }

            group->a = lua_tointeger(L, -4);
            group->b = lua_tointeger(L, -3);
            group->c = lua_tointeger(L, -2);
            group->d = lua_tointeger(L, -1);
            lua_pop(L, 5);
        } else {
            fprintf(stderr, "Lua error: %s at 'rds2_group'\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else lua_pop(L, 1);

    pthread_mutex_unlock(&lua_mutex);
    return 1;
}

void lua_group_ref(RDSGroup* group, int ref) {
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

void lua_call_function_nolock(const char* function) {
    lua_getglobal(L, function);

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Lua error: %s at '%s'\n", lua_tostring(L, -1), function);
            lua_pop(L, 1);
        }
    } else lua_pop(L, 1);
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

void lua_call_table_nolock(const char *table_name) {
    lua_getglobal(L, table_name);

    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    lua_Integer len = lua_rawlen(L, -1);
    for (lua_Integer i = 1; i <= len; i++) {
        lua_rawgeti(L, -1, i);
        if (lua_isfunction(L, -1)) {
            if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
                fprintf(stderr,
                        "Lua error: %s at '%s[%lld]'\n",
                        lua_tostring(L, -1),
                        table_name,
                        (long long)i);
                lua_pop(L, 1);
            }
        } else lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop table
}
void lua_call_table(const char* function) {
    int need_lock = (pthread_mutex_trylock(&lua_mutex) == 0);
    if (!need_lock) {
        fprintf(stderr, "Warning: lua_mutex already locked when table calling %s\n", function);
        return;
    }
    lua_call_table_nolock(function);
    pthread_mutex_unlock(&lua_mutex);
}

void lua_call_tfunction_nolock(const char* name) {
    char table_name[256];
    lua_call_function_nolock(name);
    snprintf(table_name, sizeof(table_name), "%ss", name);
    lua_call_table_nolock(table_name);
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
    if (L) {
        for (int i = 1; i < *unload_refs; i++) luaL_unref(L, LUA_REGISTRYINDEX, unload_refs[i]);
        *unload_refs = 1;
        lua_close(L);
        L = NULL;
    }
    mod = NULL;
    pthread_mutex_destroy(&lua_mutex);
}
