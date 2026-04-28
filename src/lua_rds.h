#pragma once
#include "common.h"
#include "rds.h"
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

uint8_t init_lua(RDSEncoder* _enc);
void run_lua(char *str, size_t str_len, char *cmd_output, size_t* out_len);
int lua_group(RDSGroup* group, const char grp);
int lua_rds2_group(RDSGroup* group, int stream);
void lua_call_function_nolock(const char* function);
void lua_call_function(const char* function);
void lua_call_tfunction_nolock(const char* name);
void lua_call_tfunction(const char* name);
void destroy_lua();