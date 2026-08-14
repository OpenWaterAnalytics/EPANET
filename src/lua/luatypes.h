/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       luatypes.h
 Description:  data types used by a project's Lua scripting engine
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/14/2026
 ******************************************************************************
*/

#ifdef LUA_SCRIPTING
#ifndef LUATYPES_H
#define LUATYPES_H

#include "minilua.h"

struct LuaEngine {
    lua_State *engine;
    char *script;
    int changed;
    int global_closure_ref;
    int timed_event;
};

#endif // LUATYPES_H
#endif // LUA_SCRIPTING