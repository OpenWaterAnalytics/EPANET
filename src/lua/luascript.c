/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       luascript.c
 Description:  manages a project's Lua scripting engine
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/14/2026
 ******************************************************************************
*/

#ifdef LUA_SCRIPTING
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "luatypes.h"
#include "luascript.h"
#include "luafuncs.h"
#include "luaevents.h"
#include "funcs.h"
#include "text.h"

void luascript_setChanged(Project *pr)
{
    if (pr->lua != NULL) pr->lua->changed = TRUE;
}

int luascript_addScriptLine(Project *pr, char *line)
{
    size_t line_len = strlen(line);

    if (pr->lua == NULL)
    {
        return 311;
    }
    
    if (pr->lua->script == NULL)
    {
        pr->lua->script = malloc(line_len+1);
        memcpy(pr->lua->script, line, line_len);
        pr->lua->script[line_len] = '\0';
    }
    else
    {
        size_t prev_len = strlen(pr->lua->script);
        char *buffer = realloc(pr->lua->script, prev_len + line_len + 1);
        if (buffer == NULL)
        {
            return 101;
        }

        memcpy(buffer + prev_len, line, line_len);
        buffer[line_len + prev_len] = '\0';
        pr->lua->script = buffer;
    }

    return 0;
}

int luascript_open(Project *pr)
{
    pr->lua = calloc(1, sizeof(LuaEngine));
    if (pr->lua == NULL)
    {
        return 101;
    }

    pr->lua->global_closure_ref = LUA_NOREF;

    pr->lua->engine = luaL_newstate();
    if (pr->lua->engine == NULL)
    {
        return 310;
    }

    luaL_openlibs(pr->lua->engine);
    luafuncs_register(pr->lua->engine, pr);

    return 0;
}

static int run_lua_script(Project *pr, int *changed)
{
    if (changed != NULL) *changed = FALSE;

    if (pr->lua == NULL || pr->lua->engine == NULL || pr->lua->global_closure_ref == LUA_NOREF)
    {
        return 0;
    }

    pr->lua->changed = FALSE;

    lua_rawgeti(pr->lua->engine, LUA_REGISTRYINDEX, pr->lua->global_closure_ref);
    if (lua_pcall(pr->lua->engine, 0, 0, 0) != LUA_OK)
    {
        if (pr->report.Statflag != FALSE)
        {
            char msg[MAXMSG + 1];
            snprintf(msg, MAXMSG, FMT87, lua_tostring(pr->lua->engine, -1));
            writeline(pr, msg);
        }
        lua_pop(pr->lua->engine, 1);
        return 313;
    }

    if (changed != NULL) *changed = pr->lua->changed;
    return 0;
}

int luascript_parseScript(Project *pr)
{
    if (pr->lua == NULL || pr->lua->engine == NULL)
    {
        return 311;
    }

    if (pr->lua->script == NULL)
    {
        return 0;
    }

    if (luaL_loadstring(pr->lua->engine, pr->lua->script) != LUA_OK)
    {
        if (pr->report.Statflag != FALSE)
        {
            char msg[MAXMSG + 1];
            snprintf(msg, MAXMSG, FMT86, lua_tostring(pr->lua->engine, -1));
            writeline(pr, msg);
        }
        lua_pop(pr->lua->engine, 1);
        return 312;
    }
    pr->lua->global_closure_ref = luaL_ref(pr->lua->engine, LUA_REGISTRYINDEX);

    // The load-time evaluation runs against a network that has not been
    // solved yet, so an error in it is reported but left to be raised by
    // the first pass of the run proper
    run_lua_script(pr, NULL);
    pr->lua->changed = FALSE;

    return 0;
}

int luascript_runIteration(Project *pr, int *changed)
{
    if (changed != NULL) *changed = FALSE;
    if (pr->lua == NULL || pr->lua->engine == NULL) return 0;

    lua_getglobal(pr->lua->engine, "on_hydraulic_step");
    int hasHandler = lua_isfunction(pr->lua->engine, -1);
    lua_pop(pr->lua->engine, 1);

    if (hasHandler)
    {
        return luascript_onEvent(pr, LUA_EVENT_HYDRAULIC_STEP, changed);
    }
    return run_lua_script(pr, changed);
}

void luascript_close(Project *pr)
{
    if (pr->lua != NULL)
    {
        if (pr->lua->engine != NULL)
        {
            if (pr->lua->global_closure_ref != LUA_NOREF)
            {
                luaL_unref(pr->lua->engine, LUA_REGISTRYINDEX,
                           pr->lua->global_closure_ref);
            }
            lua_close(pr->lua->engine);
        }
        free(pr->lua->script);
        free(pr->lua);
        pr->lua = NULL;
    }
}
#endif // LUA_SCRIPTING