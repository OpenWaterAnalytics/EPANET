#ifdef LUA_SCRIPTING
#ifndef LUAEVENTS_H
#define LUAEVENTS_H

#include "types.h"

typedef enum  {
  LUA_EVENT_OPEN,
  LUA_EVENT_CLOSE,
  LUA_EVENT_REPORT,
  LUA_EVENT_ITERATION,
  LUA_EVENT_MAX
} LuaEvent;

int luascript_onEvent(Project *pr, LuaEvent event);

#endif // LUAEVENTS_H
#endif // LUA_SCRIPTING