#include "types.h"
#include "funcs.h"
#include "luatypes.h"
#include "luaevents.h"

static const char *event_name[LUA_EVENT_MAX] = {
    [LUA_EVENT_OPEN] = "on_open",
    [LUA_EVENT_CLOSE] = "on_close",
    [LUA_EVENT_REPORT] = "on_report",
    [LUA_EVENT_ITERATION] = "on_iteration"
};

int luascript_onEvent(Project *pr, LuaEvent event)
{
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
    
    int execution_result = lua_pcall(pr->lua->engine, 0, 0, 0);
    if (execution_result != LUA_OK)
    {
        char msg[MAXMSG + 1];
        snprintf(msg, MAXMSG, "Lua script error in %s: %s",
                 event_name[event], lua_tostring(pr->lua->engine, -1));
        writeline(pr, msg);
        lua_pop(pr->lua->engine, 1);
    }

    return pr->lua->changed;
}