#ifdef LUA_SCRIPTING
#ifndef LUA_FUNCS_H
#define LUA_FUNCS_H

#include "minilua.h"
#include "types.h"

void luafuncs_register(lua_State *L, Project *pr);

#endif // LUA_FUNCS_H
#endif // LUA_SCRIPTING
