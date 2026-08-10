#ifndef LUASCRIPT_H
#define LUASCRIPT_H
#ifdef LUA_SCRIPTING

#include "types.h"

int luascript_open(Project *pr);
void luascript_close(Project *pr);
int luascript_runIteration(Project *pr);

int luascript_addScriptLine(Project *pr, char *line);
int luascript_parseScript(Project *pr);
void luascript_setChanged(Project *pr);

#endif // LUA_SCRIPTING
#endif // LUASCRIPT_H