#pragma once
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "rds.h"
#include "modulator.h"

int lua_set_rds_pi(lua_State *L);
void init_lua(RDSModulator* rds_mod);
void run_lua(char *str, char *cmd_output);
void destroy_lua();