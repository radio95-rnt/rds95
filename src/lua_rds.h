#pragma once
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "rds.h"
#include "fs.h"
#include "modulator.h"

void init_lua(RDSModulator* rds_mod);
void run_lua(char *str, char *cmd_output);
void lua_group(RDSGroup* group);
void lua_call_function(const char* function);
void destroy_lua();