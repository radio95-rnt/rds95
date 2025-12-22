#include "lua_rds.h"

static RDSModulator* mod = NULL;
static lua_State *L = NULL;

int lua_set_rds_program_defaults(lua_State *localL) {
    (void)localL;
	set_rds_defaults(mod->enc, mod->enc->program);
    return 0;
}

int lua_reset_rds(lua_State *localL) {
    (void)localL;
    encoder_saveToFile(mod->enc);
	encoder_loadFromFile(mod->enc);
	for(int i = 0; i < PROGRAMS; i++) reset_rds_state(mod->enc, i);
    Modulator_saveToFile(&mod->params);
	Modulator_loadFromFile(&mod->params);
    return 0;
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

#define INT_GETTER(name) \
int lua_get_rds_##name(lua_State *localL) { \
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].name); \
    return 1; \
}
#define STR_RAW_GETTER(name) \
int lua_get_rds_##name(lua_State *localL) { \
    lua_pushstring(localL, mod->enc->data[mod->enc->program].name); \
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
int lua_set_rds_rt_text_timeout(lua_State *localL) {
	mod->enc->data[mod->enc->program].rt_text_timeout = luaL_checkinteger(localL, 1);
	mod->enc->state[mod->enc->program].rt_text_timeout_state = mod->enc->data[mod->enc->program].rt_text_timeout;
    return 0;
}
int lua_get_rds_rt_text_timeout(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->data[mod->enc->program].rt_text_timeout);
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

int lua_set_rds_rtplus_tags(lua_State *localL) {
    uint8_t tags[6];
	tags[0] = luaL_checkinteger(localL, 1);
	tags[1] = luaL_checkinteger(localL, 2);
	tags[2] = luaL_checkinteger(localL, 3);
	tags[3] = luaL_checkinteger(localL, 4);
	tags[4] = luaL_checkinteger(localL, 5);
	tags[5] = luaL_checkinteger(localL, 6);
    set_rds_rtplus_tags(mod->enc, tags);
    return 0;
}
int lua_get_rds_rtplus_tags(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][0].type[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][0].start[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][0].len[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][0].type[1]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][0].start[1]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][0].len[1]);
    return 6;
}
int lua_set_rds_ertplus_tags(lua_State *localL) {
    uint8_t tags[6];
	tags[0] = luaL_checkinteger(localL, 1);
	tags[1] = luaL_checkinteger(localL, 2);
	tags[2] = luaL_checkinteger(localL, 3);
	tags[3] = luaL_checkinteger(localL, 4);
	tags[4] = luaL_checkinteger(localL, 5);
	tags[5] = luaL_checkinteger(localL, 6);
    set_rds_ertplus_tags(mod->enc, tags);
    return 0;
}
int lua_get_rds_ertplus_tags(lua_State *localL) {
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][1].type[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][1].start[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][1].len[0]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][1].type[1]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][1].start[1]);
    lua_pushinteger(localL, mod->enc->rtpData[mod->enc->program][1].len[1]);
    return 6;
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

STR_RAW_SETTER(grpseq2, set_rds_grpseq2)
int lua_get_rds_grpseq2(lua_State *localL) {
    lua_pushstring(localL, mod->enc->data[mod->enc->program].grp_sqc_rds2);
    return 1;
}

int lua_set_rds_grpseq(lua_State *localL) {
	const char* str = luaL_checklstring(localL, 1, NULL);
    if(_strnlen(str, 2) < 1) set_rds_grpseq(mod->enc, DEFAULT_GRPSQC);
    else set_rds_grpseq(mod->enc, str);
    return 0;
}
int lua_get_rds_grpseq(lua_State *localL) {
    lua_pushstring(localL, mod->enc->data[mod->enc->program].grp_sqc);
    return 1;
}

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
    lua_pushinteger(L, PROGRAMS);
    lua_setglobal(L, "max_programs");

    lua_register(L, "set_rds_program_defaults", lua_set_rds_program_defaults);
    lua_register(L, "reset_rds", lua_reset_rds);

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

    lua_register(L, "set_rds_grpseq", lua_set_rds_grpseq);
    lua_register(L, "get_rds_grpseq", lua_get_rds_grpseq);

    lua_register(L, "set_rds_grpseq2", lua_set_rds_grpseq2);
    lua_register(L, "get_rds_grpseq2", lua_get_rds_grpseq2);

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

    lua_register(L, "set_rds_ertplus_tags", lua_set_rds_ertplus_tags);
    lua_register(L, "get_rds_ertplus_tags", lua_get_rds_ertplus_tags);
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

void lua_on_init() {
    char path[128];
    snprintf(path, sizeof(path), "%s/.rds95.command.lua", getenv("HOME"));

    if (luaL_loadfilex(L, path, NULL) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        fprintf(stderr, "Lua error loading file: %s\n", err);
        lua_pop(L, 1);
        return;
    }

    lua_pushnil(L);
    lua_setglobal(L, "data");

    if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
        const char *err = lua_tostring(L, -1);
        fprintf(stderr, "Lua error running script: %s\n", err);
        lua_pop(L, 1);
        return;
    }

    lua_getglobal(L, "on_init");

    if (lua_isfunction(L, -1)) {
        if (lua_pcall(L, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "Lua error running 'on_init': %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    } else {
        // printf("Note: 'on_init' function not found in Lua script. Skipping.\n");
        lua_pop(L, 1);
    }
}

void destroy_lua(void) {
    if (L) {
        lua_close(L);
        L = NULL;
    }
    mod = NULL;
}
