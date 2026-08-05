/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.4
 Module:       lua.c
 Description:  instantiates declaration of Lua scripting engine
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: xx/xx/2026
 ******************************************************************************
*/

#ifdef LUA_SCRIPTING
#include "minilua.h"
#define LUA_IMPL    1
#endif // LUA_SCRIPTING