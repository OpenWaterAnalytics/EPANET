#ifndef LUASCRIPT_H
#define LUASCRIPT_H
#ifdef LUA_SCRIPTING

#include "types.h"

int luascript_setScript(Project *pr, char *code);
int luascript_open(Project *pr);
void luascript_run(Project *pr);
void luascript_close(Project *pr);

#endif // LUA_SCRIPTING
#endif // LUASCRIPT_H