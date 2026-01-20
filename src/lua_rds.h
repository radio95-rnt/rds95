#pragma once
#include "common.h"
#include "rds.h"
#include "modulator.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

void init_lua(RDSModulator* rds_mod);
void run_lua(char *str, char *cmd_output, size_t* out_len);
int lua_group(RDSGroup* group, const char grp);
int lua_rds2_group(RDSGroup* group, int stream);
void lua_call_function_nolock(const char* function);
void lua_call_function(const char* function);
void lua_call_table_nolock(const char *table_name);
void lua_call_table(const char* function);
void lua_call_tfunction_nolock(const char* name);
void lua_call_tfunction(const char* name);
void lua_group_ref(RDSGroup* group, int ref);
void destroy_lua();