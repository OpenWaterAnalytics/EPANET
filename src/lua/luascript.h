/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       luascript.h
 Description:  prototypes of the Lua scripting engine's interface to the toolkit
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/14/2026
 ******************************************************************************
*/

#ifndef LUASCRIPT_H
#define LUASCRIPT_H
#ifdef LUA_SCRIPTING

#include "types.h"

int luascript_open(Project *pr);
void luascript_close(Project *pr);
int luascript_runIteration(Project *pr, int *changed);

int luascript_addScriptLine(Project *pr, char *line);
const char *luascript_getScript(Project *pr);
int luascript_parseScript(Project *pr);
void luascript_setChanged(Project *pr);

#endif // LUA_SCRIPTING
#endif // LUASCRIPT_H