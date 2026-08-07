#ifdef LUA_SCRIPTING
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "minilua.h"
#include "luascript.h"
#include "luafuncs.h"
#include "funcs.h"

struct LuaEngine {
    lua_State *engine;
    char *script;
    int changed;
};

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

    pr->lua->engine = luaL_newstate();
    if (pr->lua->engine == NULL)
    {
        return 310;
    }

    luaL_openlibs(pr->lua->engine);
    luafuncs_register(pr->lua->engine, pr);

    return 0;
}

static const char *event_name[LUA_EVENT_MAX] = {
    [LUA_EVENT_OPEN] = "on_init",
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
    if (lua_isfunction(pr->lua->engine, -1))
    {
        lua_pcall(pr->lua->engine, 1, 0, 0);
    }

    return pr->lua->changed;
}

int luascript_run(Project *pr)
{
    if (pr->lua == NULL || pr->lua->engine == NULL || pr->lua->script == NULL)
    {
        return 0;
    }

    pr->lua->changed = FALSE;
    if (luaL_dostring(pr->lua->engine, pr->lua->script) != LUA_OK)
    {
        char msg[MAXMSG + 1];
        snprintf(msg, MAXMSG, "Lua script error: %s",
                 lua_tostring(pr->lua->engine, -1));
        writeline(pr, msg);
        lua_pop(pr->lua->engine, 1);
    }
    return pr->lua->changed;
}

void luascript_close(Project *pr)
{
    if (pr->lua != NULL)
    {
        lua_close(pr->lua->engine);
        free(pr->lua);
        pr->lua = NULL;
    }
}
#endif // LUA_SCRIPTING