#ifdef LUA_SCRIPTING
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "minilua.h"
#include "luascript.h"

struct LuaEngine {
    lua_State *engine;
};

int luascript_setScript(Project *pr, char *code)
{
    return 0;
}

int luascript_open(Project *pr)
{
    pr->lua = calloc(1, sizeof(LuaEngine));

    pr->lua->engine = luaL_newstate();
    if (pr->lua == NULL)
    {
        return 310;
    }
    
    luaL_openlibs(pr->lua->engine);
    lua_pushlightuserdata(pr->lua->engine, pr);

    return 0;
}

void luascript_run(Project *pr)
{
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