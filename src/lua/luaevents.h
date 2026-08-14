/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       luaevents.h
 Description:  events a project's Lua script can define a handler for
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/14/2026
 ******************************************************************************
*/

#ifdef LUA_SCRIPTING
#ifndef LUAEVENTS_H
#define LUAEVENTS_H

#include "types.h"

typedef enum  {
  LUA_EVENT_OPEN,
  LUA_EVENT_CLOSE,
  LUA_EVENT_HYDRAULICS_SOLVED,
  LUA_EVENT_HYDRAULIC_STEP,
  LUA_EVENT_MAX
} LuaEvent;

int luascript_onEvent(Project *pr, LuaEvent event, int *changed);

#endif // LUAEVENTS_H
#endif // LUA_SCRIPTING