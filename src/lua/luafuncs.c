/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.3
 Module:       luafuncs.c
 Description:  exposes a project's network to its Lua script
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 08/14/2026
 ******************************************************************************
*/

#ifdef LUA_SCRIPTING
#include <string.h>
#include "luascript.h"
#include "luatypes.h"
#include "funcs.h"
#include "text.h"
#include "epanet2_2.h"
#include "luafuncs.h"

#define LUA_READ_ONLY FALSE
#define LUA_WRITABLE  TRUE
#define NUM_API_FUNCS (sizeof(LuaApi) / sizeof(LuaApi[0]))

// An object exposed to Lua as userdata; the metatable attached to it
// determines whether it is a node, a link or the project's options.
typedef struct
{
    int index;
} LuaElem;

// A named property mapped to its EN_ property code
typedef struct
{
    const char *name;
    int code;
    int writable;
} PropDesc;

// An object type: its Lua-facing names, its property table and the
// EPANET functions used to look it up and read/write its properties.
// A NULL find marks a project-wide object, which takes no id and whose
// index is unused; a NULL set marks one that is read only throughout
typedef struct
{
    const char *metatable;
    const char *global;
    const PropDesc *props;
    int (*find)(Network *, const char *);
    int (*get)(EN_Project, int, int, double *);
    int (*set)(EN_Project, int, int, double);
} LuaApiFunc;

// Writability follows what EN_setnodevalue / EN_setlinkvalue accept
static const PropDesc NodeProps[] = {
    { "elevation",       EN_ELEVATION,      LUA_WRITABLE  },
    { "base_demand",     EN_BASEDEMAND,     LUA_WRITABLE  },
    { "pattern",         EN_PATTERN,        LUA_WRITABLE  },
    { "emitter",         EN_EMITTER,        LUA_WRITABLE  },
    { "init_quality",    EN_INITQUAL,       LUA_WRITABLE  },
    { "source_quality",  EN_SOURCEQUAL,     LUA_WRITABLE  },
    { "source_pattern",  EN_SOURCEPAT,      LUA_WRITABLE  },
    { "source_type",     EN_SOURCETYPE,     LUA_WRITABLE  },
    { "tank_level",      EN_TANKLEVEL,      LUA_WRITABLE  },
    { "demand",          EN_DEMAND,         LUA_READ_ONLY },
    { "head",            EN_HEAD,           LUA_READ_ONLY },
    { "pressure",        EN_PRESSURE,       LUA_READ_ONLY },
    { "quality",         EN_QUALITY,        LUA_READ_ONLY },
    { "source_mass",     EN_SOURCEMASS,     LUA_READ_ONLY },
    { "init_volume",     EN_INITVOLUME,     LUA_READ_ONLY },
    { "mix_model",       EN_MIXMODEL,       LUA_WRITABLE  },
    { "mix_zone_volume", EN_MIXZONEVOL,     LUA_READ_ONLY },
    { "tank_diameter",   EN_TANKDIAM,       LUA_WRITABLE  },
    { "min_volume",      EN_MINVOLUME,      LUA_WRITABLE  },
    { "volume_curve",    EN_VOLCURVE,       LUA_WRITABLE  },
    { "min_level",       EN_MINLEVEL,       LUA_WRITABLE  },
    { "max_level",       EN_MAXLEVEL,       LUA_WRITABLE  },
    { "mix_fraction",    EN_MIXFRACTION,    LUA_WRITABLE  },
    { "bulk_coeff",      EN_TANK_KBULK,     LUA_WRITABLE  },
    { "tank_volume",     EN_TANKVOLUME,     LUA_READ_ONLY },
    { "max_volume",      EN_MAXVOLUME,      LUA_READ_ONLY },
    { "can_overflow",    EN_CANOVERFLOW,    LUA_WRITABLE  },
    { "demand_deficit",  EN_DEMANDDEFICIT,  LUA_READ_ONLY },
    { "in_control",      EN_NODE_INCONTROL, LUA_READ_ONLY },
    { "emitter_flow",    EN_EMITTERFLOW,    LUA_READ_ONLY },
    { "leakage_flow",    EN_LEAKAGEFLOW,    LUA_READ_ONLY },
    { "demand_flow",     EN_DEMANDFLOW,     LUA_READ_ONLY },
    { "full_demand",     EN_FULLDEMAND,     LUA_READ_ONLY },
    { NULL,              0,                 LUA_READ_ONLY }
};

static const PropDesc LinkProps[] = {
    { "diameter",        EN_DIAMETER,       LUA_WRITABLE  },
    { "length",          EN_LENGTH,         LUA_WRITABLE  },
    { "roughness",       EN_ROUGHNESS,      LUA_WRITABLE  },
    { "minor_loss",      EN_MINORLOSS,      LUA_WRITABLE  },
    { "init_status",     EN_INITSTATUS,     LUA_WRITABLE  },
    { "init_setting",    EN_INITSETTING,    LUA_WRITABLE  },
    { "bulk_coeff",      EN_KBULK,          LUA_WRITABLE  },
    { "wall_coeff",      EN_KWALL,          LUA_WRITABLE  },
    { "flow",            EN_FLOW,           LUA_READ_ONLY },
    { "velocity",        EN_VELOCITY,       LUA_READ_ONLY },
    { "headloss",        EN_HEADLOSS,       LUA_READ_ONLY },
    { "status",          EN_STATUS,         LUA_WRITABLE  },
    { "setting",         EN_SETTING,        LUA_WRITABLE  },
    { "energy",          EN_ENERGY,         LUA_READ_ONLY },
    { "quality",         EN_LINKQUAL,       LUA_READ_ONLY },
    { "pattern",         EN_LINKPATTERN,    LUA_WRITABLE  },
    { "pump_state",      EN_PUMP_STATE,     LUA_READ_ONLY },
    { "pump_efficiency", EN_PUMP_EFFIC,     LUA_READ_ONLY },
    { "pump_power",      EN_PUMP_POWER,     LUA_WRITABLE  },
    { "pump_hcurve",     EN_PUMP_HCURVE,    LUA_WRITABLE  },
    { "pump_ecurve",     EN_PUMP_ECURVE,    LUA_WRITABLE  },
    { "pump_ecost",      EN_PUMP_ECOST,     LUA_WRITABLE  },
    { "pump_epattern",   EN_PUMP_EPAT,      LUA_WRITABLE  },
    { "in_control",      EN_LINK_INCONTROL, LUA_READ_ONLY },
    { "gpv_curve",       EN_GPV_CURVE,      LUA_WRITABLE  },
    { "pcv_curve",       EN_PCV_CURVE,      LUA_WRITABLE  },
    { "leak_area",       EN_LEAK_AREA,      LUA_WRITABLE  },
    { "leak_expansion",  EN_LEAK_EXPAN,     LUA_WRITABLE  },
    { "leakage",         EN_LINK_LEAKAGE,   LUA_READ_ONLY },
    { "valve_type",      EN_VALVE_TYPE,     LUA_READ_ONLY },
    { NULL,              0,                 LUA_READ_ONLY }
};

static const PropDesc OptionProps[] = {
    { "trials",               EN_TRIALS,        LUA_READ_ONLY },
    { "accuracy",             EN_ACCURACY,      LUA_READ_ONLY },
    { "tolerance",            EN_TOLERANCE,     LUA_READ_ONLY },
    { "emitter_exponent",     EN_EMITEXPON,     LUA_READ_ONLY },
    { "demand_multiplier",    EN_DEMANDMULT,    LUA_READ_ONLY },
    { "head_error",           EN_HEADERROR,     LUA_READ_ONLY },
    { "flow_change",          EN_FLOWCHANGE,    LUA_READ_ONLY },
    { "headloss_form",        EN_HEADLOSSFORM,  LUA_READ_ONLY },
    { "global_efficiency",    EN_GLOBALEFFIC,   LUA_READ_ONLY },
    { "global_price",         EN_GLOBALPRICE,   LUA_READ_ONLY },
    { "global_pattern",       EN_GLOBALPATTERN, LUA_READ_ONLY },
    { "demand_charge",        EN_DEMANDCHARGE,  LUA_READ_ONLY },
    { "specific_gravity",     EN_SP_GRAVITY,    LUA_READ_ONLY },
    { "specific_viscosity",   EN_SP_VISCOS,     LUA_READ_ONLY },
    { "unbalanced",           EN_UNBALANCED,    LUA_READ_ONLY },
    { "check_frequency",      EN_CHECKFREQ,     LUA_READ_ONLY },
    { "max_check",            EN_MAXCHECK,      LUA_READ_ONLY },
    { "damp_limit",           EN_DAMPLIMIT,     LUA_READ_ONLY },
    { "specific_diffusivity", EN_SP_DIFFUS,     LUA_READ_ONLY },
    { "bulk_order",           EN_BULKORDER,     LUA_READ_ONLY },
    { "wall_order",           EN_WALLORDER,     LUA_READ_ONLY },
    { "tank_order",           EN_TANKORDER,     LUA_READ_ONLY },
    { "concentration_limit",  EN_CONCENLIMIT,   LUA_READ_ONLY },
    { "demand_pattern",       EN_DEMANDPATTERN, LUA_READ_ONLY },
    { "emitter_backflow",     EN_EMITBACKFLOW,  LUA_READ_ONLY },
    { "pressure_units",       EN_PRESS_UNITS,   LUA_READ_ONLY },
    { "status_report",        EN_STATUS_REPORT, LUA_READ_ONLY },
    { NULL,                   0,                LUA_READ_ONLY }
};

static const PropDesc TimeProps[] = {
    { "duration",             EN_DURATION,      LUA_WRITABLE  },
    { "hydraulic_step",       EN_HYDSTEP,       LUA_WRITABLE  },
    { "quality_step",         EN_QUALSTEP,      LUA_WRITABLE  },
    { "pattern_step",         EN_PATTERNSTEP,   LUA_WRITABLE  },
    { "pattern_start",        EN_PATTERNSTART,  LUA_WRITABLE  },
    { "report_step",          EN_REPORTSTEP,    LUA_WRITABLE  },
    { "report_start",         EN_REPORTSTART,   LUA_WRITABLE  },
    { "rule_step",            EN_RULESTEP,      LUA_WRITABLE  },
    { "statistic",            EN_STATISTIC,     LUA_WRITABLE  },
    { "periods",              EN_PERIODS,       LUA_READ_ONLY },
    { "start_time",           EN_STARTTIME,     LUA_WRITABLE  },
    { "hydraulic_time",       EN_HTIME,         LUA_WRITABLE  },
    { "quality_time",         EN_QTIME,         LUA_WRITABLE  },
    { "halt_flag",            EN_HALTFLAG,      LUA_READ_ONLY },
    { "next_event",           EN_NEXTEVENT,     LUA_READ_ONLY },
    { "next_event_tank",      EN_NEXTEVENTTANK, LUA_READ_ONLY },
    { NULL,                   0,                LUA_READ_ONLY }
};

static int getOptionValue(EN_Project pr, int index, int code, double *value)
{
    return EN_getoption(pr, code, value);
}

static int getTimeValue(EN_Project pr, int index, int code, double *value)
{
    long seconds = 0;
    int err = EN_gettimeparam(pr, code, &seconds);

    *value = (double)seconds;
    return err;
}

static int setTimeValue(EN_Project pr, int index, int code, double value)
{
    return EN_settimeparam(pr, code, (long)ROUND(value));
}

static const LuaApiFunc LuaApi[] = {
    { "epanet.node",    "node",    NodeProps,   findnode, EN_getnodevalue, EN_setnodevalue },
    { "epanet.link",    "link",    LinkProps,   findlink, EN_getlinkvalue, EN_setlinkvalue },
    { "epanet.options", "options", OptionProps, NULL,     getOptionValue,  NULL            },
    { "epanet.times",   "times",   TimeProps,   NULL,     getTimeValue,    setTimeValue    }
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
    char buf[MAXMSG + 1];
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    int nargs = lua_gettop(lua);
    int pos = 0;

    if (pr->report.Statflag == FALSE) return 0;

    if (pr->lua->timed_event)
    {
        pos += sprintf(buf, FMT83, clocktime(pr->report.Atime,
                                             pr->times.Htime));
    }
    else pos += sprintf(buf, FMT84);
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

static int lua_curve_points(lua_State *lua)
{
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    const char *id = luaL_checkstring(lua, 1);

    int index = findcurve(&pr->network, id);
    if (index == 0) return luaL_error(lua, "curve not found: %s", id);

    int npoints = 0;
    int err = EN_getcurvelen(pr, index, &npoints);
    if (err) return luaL_error(lua, "error %d reading curve %s", err, id);

    lua_createtable(lua, npoints, 0);
    for (int i = 1; i <= npoints; i++)
    {
        double x, y;
        err = EN_getcurvevalue(pr, index, i, &x, &y);
        if (err)
        {
            return luaL_error(lua, "error %d reading point %d of curve %s",
                              err, i, id);
        }

        lua_createtable(lua, 2, 0);
        lua_pushnumber(lua, x);
        lua_rawseti(lua, -2, 1);
        lua_pushnumber(lua, y);
        lua_rawseti(lua, -2, 2);
        lua_rawseti(lua, -2, i);
    }

    return 1;
}

static int lua_elem_new(lua_State *lua)
{
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    const LuaApiFunc *d = lua_touserdata(lua, lua_upvalueindex(2));
    int index = 0;

    if (d->find != NULL)
    {
        const char *id = luaL_checkstring(lua, 1);
        index = d->find(&pr->network, id);
        if (index == 0) return luaL_error(lua, "%s not found: %s", d->global, id);
    }

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
    if (!p->writable || d->set == NULL)
    {
        return luaL_error(lua, "%s property is read only: %s", d->global, key);
    }

    double before, after;
    int hadValueBefore = (d->get(pr, e->index, p->code, &before) == 0);

    int err = d->set(pr, e->index, p->code, value);
    if (err) return luaL_error(lua, "error %d writing %s.%s", err, d->global, key);

    // Flag the change so the hydraulic solver re-converges, but only if
    // the stored value actually changed: scripts re-run on every solver
    // convergence, so no-op rewrites must not keep it iterating forever
    if (!hadValueBefore || d->get(pr, e->index, p->code, &after) != 0
        || ABS(after - before) > TINY * (1.0 + ABS(before)))
    {
        luascript_setChanged(pr);
    }
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

    lua_pushlightuserdata(lua, pr);
    lua_pushcclosure(lua, lua_curve_points, 1);
    lua_setglobal(lua, "curve");

    for (size_t i = 0; i < NUM_API_FUNCS; i++)
    {
        registerFunction(lua, &LuaApi[i], pr);
    }
}
#endif // LUA_SCRIPTING
