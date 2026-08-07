/*
******************************************************************************
Project:      OWA EPANET
Version:      2.4
Module:       lua_test_utils.hpp
Description:  Utilities for the Lua scripting acceptance tests
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
******************************************************************************
*/

#ifndef LUA_TEST_UTILS_HPP
#define LUA_TEST_UTILS_HPP

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "epanet2_2.h"


// OPTIONS and TIMES are project-wide, reached through options() and
// times() rather than by id, so their elementId is unused
enum ElementKind { NODE, LINK, OPTIONS, TIMES };

struct PropertyWrite
{
    ElementKind kind;
    const char *elementId;
    const char *luaProperty;
    int enProperty;
    double value;
};

struct LuaProperty
{
    const char *luaName;
    int enProperty;
};

struct ElementToDump
{
    ElementKind kind;
    const char *elementId;
    const char *reportTag;
    const std::vector<LuaProperty> *properties;
};


inline std::string readWholeFile(const char *path)
{
    std::ifstream in(path);
    std::stringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

// Builds an INP file by inserting a [SCRIPT] section, and optionally
// other extra sections, right before the [END] of a base INP file
inline bool buildInpWithScript(const char *basePath, const char *outPath,
                               const std::string &script,
                               const std::string &extraSections = "")
{
    std::string inp = readWholeFile(basePath);
    size_t endSection = inp.rfind("[END]");
    if (inp.empty() || endSection == std::string::npos) return false;

    inp.insert(endSection, extraSections + "[SCRIPT]\n" + script + "\n");

    std::ofstream out(outPath);
    out << inp;
    return out.good();
}

// The Lua expression that yields an element: node("11"), link("9"),
// options() or times()
inline std::string luaElementRef(ElementKind kind, const char *elementId)
{
    if (kind == OPTIONS) return "options()";
    if (kind == TIMES) return "times()";
    return std::string(kind == NODE ? "node" : "link")
         + "(\"" + elementId + "\")";
}

// Wraps statements in an event handler. The [SCRIPT] chunk is evaluated
// once, when the project opens, so anything that has to see or change a
// solved network belongs in a handler rather than at the top level
inline std::string luaEventHandler(const char *event, const std::string &body)
{
    return "function " + std::string(event) + "()\n" + body + "end\n";
}

inline std::string luaAssignment(const PropertyWrite &write)
{
    std::ostringstream statement;
    statement << luaElementRef(write.kind, write.elementId) << "."
              << write.luaProperty << " = " << write.value << "\n";
    return statement.str();
}

inline std::string luaStringList(const std::vector<const char *> &values)
{
    std::string list = "{";
    for (size_t i = 0; i < values.size(); i++)
    {
        if (i > 0) list += ",";
        list += "\"" + std::string(values[i]) + "\"";
    }
    return list + "}";
}

inline std::string luaStringList(const std::vector<LuaProperty> &properties)
{
    std::vector<const char *> names;
    for (const LuaProperty &property : properties)
    {
        names.push_back(property.luaName);
    }
    return luaStringList(names);
}

// Builds a script whose handler prints one "tag.property=value" line per
// readable property of each element; properties whose read raises an
// error are skipped
inline std::string luaDumpScript(const std::vector<ElementToDump> &elements,
                                 const char *event)
{
    std::string body;
    for (const ElementToDump &element : elements)
    {
        body += std::string("    dump(\"") + element.reportTag + "\", "
              + luaElementRef(element.kind, element.elementId) + ", "
              + luaStringList(*element.properties) + ")\n";
    }

    return "local function dump(tag, element, propertyNames)\n"
           "    for _, name in ipairs(propertyNames) do\n"
           "        local ok, value = pcall(function() return element[name] end)\n"
           "        if ok then print(tag .. \".\" .. name .. \"=\" .. tostring(value)) end\n"
           "    end\n"
           "end\n"
         + luaEventHandler(event, body);
}

inline bool reportMentionsLuaError(const std::string &report)
{
    return report.find("Lua script error") != std::string::npos;
}

inline int countOccurrences(const std::string &report, const std::string &label)
{
    int count = 0;
    for (size_t at = report.find(label); at != std::string::npos;
         at = report.find(label, at + label.size()))
    {
        count++;
    }
    return count;
}

// The script prints once per solver convergence; the last occurrence
// matches the solved state
inline bool findLastPrintedValue(const std::string &report,
                                 const std::string &label, double *value)
{
    size_t position = report.rfind(label);
    if (position == std::string::npos) return false;

    *value = strtod(report.c_str() + position + label.size(), NULL);
    return true;
}


struct ProjectUnderTest
{
    ProjectUnderTest() : ph(NULL), closed(false)
    {
        EN_createproject(&ph);
    }

    ~ProjectUnderTest()
    {
        close();
        EN_deleteproject(ph);
    }

    int open(const char *inpPath, const char *rptPath)
    {
        return EN_open(ph, inpPath, rptPath, "");
    }

    // A single hydraulic solve executes the [SCRIPT] at least once
    int solveOneHydraulicStep()
    {
        long time;
        int error = EN_openH(ph);
        if (!error) error = EN_initH(ph, EN_NOSAVE);
        if (!error) error = EN_runH(ph, &time);
        if (!error) error = EN_closeH(ph);
        return error;
    }

    // Runs every time step of the simulation, reporting how many were solved
    int solveAllHydraulicSteps(int *steps)
    {
        long time, tstep = 0;
        int error = EN_openH(ph);
        if (!error) error = EN_initH(ph, EN_NOSAVE);

        *steps = 0;
        while (!error)
        {
            error = EN_runH(ph, &time);
            if (error) break;
            (*steps)++;

            error = EN_nextH(ph, &tstep);
            if (error || tstep == 0) break;
        }

        if (!error) error = EN_closeH(ph);
        return error;
    }

    int readValue(ElementKind kind, const char *elementId, int enProperty,
                  double *value)
    {
        int index;
        if (kind == OPTIONS) return EN_getoption(ph, enProperty, value);
        if (kind == TIMES)
        {
            long seconds = 0;
            int error = EN_gettimeparam(ph, enProperty, &seconds);
            *value = (double)seconds;
            return error;
        }
        if (kind == NODE)
        {
            EN_getnodeindex(ph, (char *)elementId, &index);
            return EN_getnodevalue(ph, index, enProperty, value);
        }
        EN_getlinkindex(ph, (char *)elementId, &index);
        return EN_getlinkvalue(ph, index, enProperty, value);
    }

    int writeValue(ElementKind kind, const char *elementId, int enProperty,
                   double value)
    {
        int index;
        if (kind == OPTIONS) return EN_setoption(ph, enProperty, value);
        if (kind == TIMES) return EN_settimeparam(ph, enProperty, (long)value);
        if (kind == NODE)
        {
            EN_getnodeindex(ph, (char *)elementId, &index);
            return EN_setnodevalue(ph, index, enProperty, value);
        }
        EN_getlinkindex(ph, (char *)elementId, &index);
        return EN_setlinkvalue(ph, index, enProperty, value);
    }

    // Closing flushes the report file so it can be read back
    void close()
    {
        if (!closed) EN_close(ph);
        closed = true;
    }

    EN_Project ph;
    bool closed;
};

#endif // LUA_TEST_UTILS_HPP
