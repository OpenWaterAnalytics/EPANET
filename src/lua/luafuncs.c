#ifdef LUA_SCRIPTING
#include <string.h>
#include "luascript.h"
#include "funcs.h"
#include "epanet2_2.h"
#include "luafuncs.h"

#define READ_ONLY FALSE
#define WRITABLE  TRUE
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
// index is unused
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
    { "elevation",       EN_ELEVATION,      WRITABLE  },
    { "base_demand",     EN_BASEDEMAND,     WRITABLE  },
    { "pattern",         EN_PATTERN,        WRITABLE  },
    { "emitter",         EN_EMITTER,        WRITABLE  },
    { "init_quality",    EN_INITQUAL,       WRITABLE  },
    { "source_quality",  EN_SOURCEQUAL,     WRITABLE  },
    { "source_pattern",  EN_SOURCEPAT,      WRITABLE  },
    { "source_type",     EN_SOURCETYPE,     WRITABLE  },
    { "tank_level",      EN_TANKLEVEL,      WRITABLE  },
    { "demand",          EN_DEMAND,         READ_ONLY },
    { "head",            EN_HEAD,           READ_ONLY },
    { "pressure",        EN_PRESSURE,       READ_ONLY },
    { "quality",         EN_QUALITY,        READ_ONLY },
    { "source_mass",     EN_SOURCEMASS,     READ_ONLY },
    { "init_volume",     EN_INITVOLUME,     READ_ONLY },
    { "mix_model",       EN_MIXMODEL,       WRITABLE  },
    { "mix_zone_volume", EN_MIXZONEVOL,     READ_ONLY },
    { "tank_diameter",   EN_TANKDIAM,       WRITABLE  },
    { "min_volume",      EN_MINVOLUME,      WRITABLE  },
    { "volume_curve",    EN_VOLCURVE,       WRITABLE  },
    { "min_level",       EN_MINLEVEL,       WRITABLE  },
    { "max_level",       EN_MAXLEVEL,       WRITABLE  },
    { "mix_fraction",    EN_MIXFRACTION,    WRITABLE  },
    { "bulk_coeff",      EN_TANK_KBULK,     WRITABLE  },
    { "tank_volume",     EN_TANKVOLUME,     READ_ONLY },
    { "max_volume",      EN_MAXVOLUME,      READ_ONLY },
    { "can_overflow",    EN_CANOVERFLOW,    WRITABLE  },
    { "demand_deficit",  EN_DEMANDDEFICIT,  READ_ONLY },
    { "in_control",      EN_NODE_INCONTROL, READ_ONLY },
    { "emitter_flow",    EN_EMITTERFLOW,    READ_ONLY },
    { "leakage_flow",    EN_LEAKAGEFLOW,    READ_ONLY },
    { "demand_flow",     EN_DEMANDFLOW,     READ_ONLY },
    { "full_demand",     EN_FULLDEMAND,     READ_ONLY },
    { NULL,              0,                 READ_ONLY }
};

static const PropDesc LinkProps[] = {
    { "diameter",        EN_DIAMETER,       WRITABLE  },
    { "length",          EN_LENGTH,         WRITABLE  },
    { "roughness",       EN_ROUGHNESS,      WRITABLE  },
    { "minor_loss",      EN_MINORLOSS,      WRITABLE  },
    { "init_status",     EN_INITSTATUS,     WRITABLE  },
    { "init_setting",    EN_INITSETTING,    WRITABLE  },
    { "bulk_coeff",      EN_KBULK,          WRITABLE  },
    { "wall_coeff",      EN_KWALL,          WRITABLE  },
    { "flow",            EN_FLOW,           READ_ONLY },
    { "velocity",        EN_VELOCITY,       READ_ONLY },
    { "headloss",        EN_HEADLOSS,       READ_ONLY },
    { "status",          EN_STATUS,         WRITABLE  },
    { "setting",         EN_SETTING,        WRITABLE  },
    { "energy",          EN_ENERGY,         READ_ONLY },
    { "quality",         EN_LINKQUAL,       READ_ONLY },
    { "pattern",         EN_LINKPATTERN,    WRITABLE  },
    { "pump_state",      EN_PUMP_STATE,     READ_ONLY },
    { "pump_efficiency", EN_PUMP_EFFIC,     READ_ONLY },
    { "pump_power",      EN_PUMP_POWER,     WRITABLE  },
    { "pump_hcurve",     EN_PUMP_HCURVE,    WRITABLE  },
    { "pump_ecurve",     EN_PUMP_ECURVE,    WRITABLE  },
    { "pump_ecost",      EN_PUMP_ECOST,     WRITABLE  },
    { "pump_epattern",   EN_PUMP_EPAT,      WRITABLE  },
    { "in_control",      EN_LINK_INCONTROL, READ_ONLY },
    { "gpv_curve",       EN_GPV_CURVE,      WRITABLE  },
    { "pcv_curve",       EN_PCV_CURVE,      WRITABLE  },
    { "leak_area",       EN_LEAK_AREA,      WRITABLE  },
    { "leak_expansion",  EN_LEAK_EXPAN,     WRITABLE  },
    { "leakage",         EN_LINK_LEAKAGE,   READ_ONLY },
    { "valve_type",      EN_VALVE_TYPE,     READ_ONLY },
    { NULL,              0,                 READ_ONLY }
};

static const PropDesc OptionProps[] = {
    { "trials",               EN_TRIALS,        WRITABLE  },
    { "accuracy",             EN_ACCURACY,      WRITABLE  },
    { "tolerance",            EN_TOLERANCE,     WRITABLE  },
    { "emitter_exponent",     EN_EMITEXPON,     WRITABLE  },
    { "demand_multiplier",    EN_DEMANDMULT,    WRITABLE  },
    { "head_error",           EN_HEADERROR,     WRITABLE  },
    { "flow_change",          EN_FLOWCHANGE,    WRITABLE  },
    { "headloss_form",        EN_HEADLOSSFORM,  READ_ONLY },
    { "global_efficiency",    EN_GLOBALEFFIC,   WRITABLE  },
    { "global_price",         EN_GLOBALPRICE,   WRITABLE  },
    { "global_pattern",       EN_GLOBALPATTERN, WRITABLE  },
    { "demand_charge",        EN_DEMANDCHARGE,  WRITABLE  },
    { "specific_gravity",     EN_SP_GRAVITY,    WRITABLE  },
    { "specific_viscosity",   EN_SP_VISCOS,     WRITABLE  },
    { "unbalanced",           EN_UNBALANCED,    WRITABLE  },
    { "check_frequency",      EN_CHECKFREQ,     WRITABLE  },
    { "max_check",            EN_MAXCHECK,      WRITABLE  },
    { "damp_limit",           EN_DAMPLIMIT,     WRITABLE  },
    { "specific_diffusivity", EN_SP_DIFFUS,     WRITABLE  },
    { "bulk_order",           EN_BULKORDER,     WRITABLE  },
    { "wall_order",           EN_WALLORDER,     WRITABLE  },
    { "tank_order",           EN_TANKORDER,     WRITABLE  },
    { "concentration_limit",  EN_CONCENLIMIT,   WRITABLE  },
    { "demand_pattern",       EN_DEMANDPATTERN, WRITABLE  },
    { "emitter_backflow",     EN_EMITBACKFLOW,  WRITABLE  },
    { "pressure_units",       EN_PRESS_UNITS,   WRITABLE  },
    { "status_report",        EN_STATUS_REPORT, WRITABLE  },
    { NULL,                   0,                READ_ONLY }
};

static const PropDesc TimeProps[] = {
    { "duration",             EN_DURATION,      WRITABLE  },
    { "hydraulic_step",       EN_HYDSTEP,       WRITABLE  },
    { "quality_step",         EN_QUALSTEP,      WRITABLE  },
    { "pattern_step",         EN_PATTERNSTEP,   WRITABLE  },
    { "pattern_start",        EN_PATTERNSTART,  WRITABLE  },
    { "report_step",          EN_REPORTSTEP,    WRITABLE  },
    { "report_start",         EN_REPORTSTART,   WRITABLE  },
    { "rule_step",            EN_RULESTEP,      WRITABLE  },
    { "statistic",            EN_STATISTIC,     WRITABLE  },
    { "periods",              EN_PERIODS,       READ_ONLY },
    { "start_time",           EN_STARTTIME,     WRITABLE  },
    { "hydraulic_time",       EN_HTIME,         WRITABLE  },
    { "quality_time",         EN_QTIME,         WRITABLE  },
    { "halt_flag",            EN_HALTFLAG,      READ_ONLY },
    { "next_event",           EN_NEXTEVENT,     READ_ONLY },
    { "next_event_tank",      EN_NEXTEVENTTANK, READ_ONLY },
    { NULL,                   0,                READ_ONLY }
};

static int getOptionValue(EN_Project pr, int index, int code, double *value)
{
    return EN_getoption(pr, code, value);
}

static int setOptionValue(EN_Project pr, int index, int code, double value)
{
    return EN_setoption(pr, code, value);
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
    return EN_settimeparam(pr, code, (long)(value + (value < 0 ? -0.5 : 0.5)));
}

static const LuaApiFunc LuaApi[] = {
    { "epanet.node",    "node",    NodeProps,   findnode, EN_getnodevalue, EN_setnodevalue },
    { "epanet.link",    "link",    LinkProps,   findlink, EN_getlinkvalue, EN_setlinkvalue },
    { "epanet.options", "options", OptionProps, NULL,     getOptionValue,  setOptionValue  },
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
    char buf[1024];
    Project *pr = lua_touserdata(lua, lua_upvalueindex(1));
    int nargs = lua_gettop(lua);
    int pos = 0;

    buf[pos++] = '\t';
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
    if (!p->writable) return luaL_error(lua, "%s property is read only: %s", d->global, key);

    double before, after;
    int hadValueBefore = (d->get(pr, e->index, p->code, &before) == 0);

    int err = d->set(pr, e->index, p->code, value);
    if (err) return luaL_error(lua, "error %d writing %s.%s", err, d->global, key);

    // Flag the change so the hydraulic solver re-converges, but only if
    // the stored value actually changed: scripts re-run on every solver
    // convergence, so no-op rewrites must not keep it iterating forever
    if (!hadValueBefore || d->get(pr, e->index, p->code, &after) != 0
        || after != before)
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
