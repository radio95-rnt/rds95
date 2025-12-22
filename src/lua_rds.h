#pragma once
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include "rds.h"
#include "fs.h"
#include "modulator.h"

void init_lua(RDSModulator* rds_mod);
void run_lua(char *str, char *cmd_output);
void lua_on_init();
void destroy_lua();