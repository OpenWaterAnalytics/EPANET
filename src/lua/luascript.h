#ifndef LUASCRIPT_H
#define LUASCRIPT_H
#ifdef LUA_SCRIPTING

#include "types.h"

typedef enum  {
  LUA_EVENT_OPEN,
  LUA_EVENT_CLOSE,
  LUA_EVENT_REPORT,
  LUA_EVENT_ITERATION,
  LUA_EVENT_MAX
} LuaEvent;

int luascript_addScriptLine(Project *pr, char *line);
int luascript_open(Project *pr);
int luascript_run(Project *pr);
int luascript_onEvent(Project *pr, LuaEvent event);
void luascript_setChanged(Project *pr);
void luascript_close(Project *pr);

#endif // LUA_SCRIPTING
#endif // LUASCRIPT_H