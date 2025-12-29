#pragma once
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "rds.h"
#include "fs.h"
#include "modulator.h"

void init_lua(RDSModulator* rds_mod);
void run_lua(char *str, char *cmd_output, size_t* out_len);
int lua_group(RDSGroup* group, const char grp);
int lua_rds2_group(RDSGroup* group, int stream);
void lua_call_function_nolock(const char* function);
void lua_call_function(const char* function);
void lua_group_ref(RDSGroup* group, int ref);
void destroy_lua();