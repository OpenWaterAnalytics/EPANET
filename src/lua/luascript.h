#ifndef LUASCRIPT_H
#define LUASCRIPT_H
#ifdef LUA_SCRIPTING

#include "types.h"

int luascript_addScriptLine(Project *pr, char *line);
int luascript_open(Project *pr);
int luascript_run(Project *pr);
void luascript_close(Project *pr);

#endif // LUA_SCRIPTING
#endif // LUASCRIPT_H