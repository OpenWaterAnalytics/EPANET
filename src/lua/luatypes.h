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