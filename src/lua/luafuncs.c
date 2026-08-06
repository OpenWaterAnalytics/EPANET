#ifdef LUA_SCRIPTING
#include <string.h>
#include "luafuncs.h"
#include "funcs.h"
#include "epanet2_2.h"

#define READ_ONLY FALSE
#define WRITABLE  TRUE
#define NUM_API_FUNCS (sizeof(LuaApi) / sizeof(LuaApi[0]))

// A network element exposed to Lua as userdata; the metatable
// attached to it determines whether it is a node or a link.
typedef struct
{
    int index;
} LuaElem;

// A named element property mapped to its EN_ property code
typedef struct
{
    const char *name;
    int code;
    int writable;
} PropDesc;

// An element type: its Lua-facing names, its property table and the
// EPANET functions used to look it up and read/write its properties
typedef struct
{
    const char *metatable;
    const char *global;
    const PropDesc *props;
    int (*find)(Network *, const char *);
    int (*get)(EN_Project, int, int, double *);
    int (*set)(EN_Project, int, int, double);
} LuaApiFunc;

static const PropDesc NodeProps[] = {
    { "pressure", EN_PRESSURE, READ_ONLY },
    { "demand",   EN_DEMAND,   READ_ONLY },
    { "head",     EN_HEAD,     READ_ONLY },
    { NULL,       0,           READ_ONLY }
};

static const PropDesc LinkProps[] = {
    { "flow",     EN_FLOW,     READ_ONLY },
    { "velocity", EN_VELOCITY, READ_ONLY },
    { "status",   EN_STATUS,   WRITABLE  },
    { "setting",  EN_SETTING,  WRITABLE  },
    { NULL,       0,           READ_ONLY }
};

static const LuaApiFunc LuaApi[] = {
    { "epanet.node", "node", NodeProps, findnode, EN_getnodevalue, EN_setnodevalue },
    { "epanet.link", "link", LinkProps, findlink, EN_getlinkvalue, EN_setlinkvalue }
};

static const PropDesc *findElementProperty(const PropDesc *props, const char *name)
{
    for (; props->name != NULL; props++)
    {
        if (strcmp(props->name, name) == 0) return props;
    }

    return NULL;
}


static const char *stringOrEmpty(lua_State *lua, int arg)
{
    const char *s = luaL_tolstring(lua, arg, NULL);
    if (s == NULL) return "";
    return s;
}

static int lua_epanet_print(lua_State *lua)
{
    char buf[1024];
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    int nargs = lua_gettop(lua);
    int pos = 0;

    for (int i = 1; i <= nargs; i++)
    {
        const char *argAsString = stringOrEmpty(lua, i);
        if (i > 1 && pos < sizeof(buf) - 1)
        {
            buf[pos++] = '\t';
        }

        size_t len = strlen(argAsString);
        if (pos + len >= sizeof(buf) - 1)
        {
            len = sizeof(buf) - 1 - pos;
        }
        memcpy(buf + pos, argAsString, len);
        pos += len;
        lua_pop(lua, 1);
    }

    buf[pos] = '\0';
    writeline(pr, buf);

    return 0;
}

static int lua_elem_new(lua_State *lua)
{
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    const LuaApiFunc *d = lua_touserdata(lua, lua_upvalueindex(2));
    const char *id = luaL_checkstring(lua, 1);

    int index = d->find(&pr->network, id);
    if (index == 0) return luaL_error(lua, "%s not found: %s", d->global, id);

    LuaElem *e = lua_newuserdata(lua, sizeof(LuaElem));
    e->index = index;
    luaL_getmetatable(lua, d->metatable);
    lua_setmetatable(lua, -2);
    return 1;
}

static int lua_elem_index(lua_State *lua)
{
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    const LuaApiFunc *d = lua_touserdata(lua, lua_upvalueindex(2));
    LuaElem *e = luaL_checkudata(lua, 1, d->metatable);

    const char *key = luaL_checkstring(lua, 2);
    double value;

    const PropDesc *p = findElementProperty(d->props, key);
    if (p == NULL) return luaL_error(lua, "unknown %s property: %s", d->global, key);

    int err = d->get(pr, e->index, p->code, &value);
    if (err) return luaL_error(lua, "error %d reading %s.%s", err, d->global, key);

    lua_pushnumber(lua, value);
    return 1;
}

static int lua_elem_newindex(lua_State *lua)
{
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    const LuaApiFunc *d = lua_touserdata(lua, lua_upvalueindex(2));
    LuaElem *e = luaL_checkudata(lua, 1, d->metatable);

    const char *key = luaL_checkstring(lua, 2);
    double value = luaL_checknumber(lua, 3);

    const PropDesc *p = findElementProperty(d->props, key);
    if (p == NULL) return luaL_error(lua, "unknown %s property: %s", d->global, key);
    if (!p->writable) return luaL_error(lua, "%s property is read only: %s", d->global, key);

    int err = d->set(pr, e->index, p->code, value);
    if (err) return luaL_error(lua, "error %d writing %s.%s", err, d->global, key);

    return 0;
}

static void registerFunction(lua_State *lua, const LuaApiFunc *fn, Project *pr)
{
    luaL_newmetatable(lua, fn->metatable);
    lua_pushlightuserdata(lua, pr);
    lua_pushlightuserdata(lua, (void *)fn);
    lua_pushcclosure(lua, lua_elem_index, 2);
    lua_setfield(lua, -2, "__index");

    lua_pushlightuserdata(lua, pr);
    lua_pushlightuserdata(lua, (void *)fn);
    lua_pushcclosure(lua, lua_elem_newindex, 2);
    lua_setfield(lua, -2, "__newindex");
    lua_pop(lua, 1);

    lua_pushlightuserdata(lua, pr);
    lua_pushlightuserdata(lua, (void *)fn);
    lua_pushcclosure(lua, lua_elem_new, 2);
    lua_setglobal(lua, fn->global);
}

void luafuncs_register(lua_State *lua, Project *pr)
{
    lua_pushlightuserdata(lua, pr);
    lua_pushcclosure(lua, lua_epanet_print, 1);
    lua_setglobal(lua, "print");

    for (size_t i = 0; i < NUM_API_FUNCS; i++)
    {
        registerFunction(lua, &LuaApi[i], pr);
    }
    return 0;
}
#endif // LUA_SCRIPTING
