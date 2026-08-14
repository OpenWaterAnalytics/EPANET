/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       luafuncs.h
 Description:  prototype for registering the toolkit's Lua-facing API
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/14/2026
 ******************************************************************************
*/

#ifdef LUA_SCRIPTING
#ifndef LUA_FUNCS_H
#define LUA_FUNCS_H

#include "minilua.h"
#include "types.h"

void luafuncs_register(lua_State *L, Project *pr);

#endif // LUA_FUNCS_H
#endif // LUA_SCRIPTING
