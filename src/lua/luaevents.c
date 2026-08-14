/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       luaevents.c
 Description:  calls the handler functions defined by a project's Lua script
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/14/2026
 ******************************************************************************
*/

#ifdef LUA_SCRIPTING
#include "types.h"
#include "funcs.h"
#include "text.h"
#include "luatypes.h"
#include "luaevents.h"

static const char *event_name[LUA_EVENT_MAX] = {
    [LUA_EVENT_OPEN] = "on_open",
    [LUA_EVENT_CLOSE] = "on_close",
    [LUA_EVENT_HYDRAULICS_SOLVED] = "on_hydraulics_solved",
    [LUA_EVENT_HYDRAULIC_STEP] = "on_hydraulic_step"
};

int luascript_onEvent(Project *pr, LuaEvent event, int *changed)
{
    if (changed != NULL) *changed = FALSE;

    if (pr->lua == NULL || pr->lua->engine == NULL || pr->lua->script == NULL)
    {
        return 0;
    }

    pr->lua->changed = FALSE;

    lua_getglobal(pr->lua->engine, event_name[event]);
    int event_defined = lua_isfunction(pr->lua->engine, -1);
    if (!event_defined)
    {
        lua_pop(pr->lua->engine, 1);
        return 0;
    }

    pr->lua->timed_event = (event == LUA_EVENT_HYDRAULIC_STEP ||
                            event == LUA_EVENT_HYDRAULICS_SOLVED);
    int execution_result = lua_pcall(pr->lua->engine, 0, 0, 0);
    pr->lua->timed_event = FALSE;

    if (execution_result != LUA_OK)
    {
        if (pr->report.Statflag != FALSE)
        {
            char msg[MAXMSG + 1];
            snprintf(msg, MAXMSG, FMT88, event_name[event],
                lua_tostring(pr->lua->engine, -1));
            writeline(pr, msg);
        }
        lua_pop(pr->lua->engine, 1);
        return 313;
    }

    if (changed != NULL) *changed = pr->lua->changed;
    return 0;
}
#endif // LUA_SCRIPTING
