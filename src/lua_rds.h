#pragma once
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "rds.h"
#include "fs.h"
#include "modulator.h"

void init_lua(RDSModulator* rds_mod);
void run_lua(char *str, char *cmd_output);
int lua_group(RDSGroup* group);
void lua_call_function(const char* function);
void lua_group_ref(RDSGroup* group, int ref);
void destroy_lua();