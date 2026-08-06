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


enum ElementKind { NODE, LINK };

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
};


inline std::string readWholeFile(const char *path)
{
    std::ifstream in(path);
    std::stringstream contents;
    contents << in.rdbuf();
    return contents.str();
}

// Builds an INP file by inserting a [SCRIPT] section right before the
// [END] of a base INP file
inline bool buildInpWithScript(const char *basePath, const char *outPath,
                               const std::string &script)
{
    std::string inp = readWholeFile(basePath);
    size_t endSection = inp.rfind("[END]");
    if (inp.empty() || endSection == std::string::npos) return false;

    inp.insert(endSection, "[SCRIPT]\n" + script + "\n");

    std::ofstream out(outPath);
    out << inp;
    return out.good();
}

inline std::string luaAssignment(const PropertyWrite &write)
{
    std::ostringstream statement;
    statement << (write.kind == NODE ? "node" : "link")
              << "(\"" << write.elementId << "\")."
              << write.luaProperty << " = " << write.value << "\n";
    return statement.str();
}

inline std::string luaStringList(const std::vector<LuaProperty> &properties)
{
    std::string list = "{";
    for (size_t i = 0; i < properties.size(); i++)
    {
        if (i > 0) list += ",";
        list += "\"" + std::string(properties[i].luaName) + "\"";
    }
    return list + "}";
}

// Builds a script that prints one "tag.property=value" line per readable
// property of each element; properties whose read raises an error are
// skipped
inline std::string luaDumpScript(const std::vector<LuaProperty> &nodeProperties,
                                 const std::vector<LuaProperty> &linkProperties,
                                 const std::vector<ElementToDump> &elements)
{
    std::string script =
        "local nodeprops = " + luaStringList(nodeProperties) + "\n"
        "local linkprops = " + luaStringList(linkProperties) + "\n"
        "local function dump(tag, element, propertyNames)\n"
        "    for _, name in ipairs(propertyNames) do\n"
        "        local ok, value = pcall(function() return element[name] end)\n"
        "        if ok then print(tag .. \".\" .. name .. \"=\" .. tostring(value)) end\n"
        "    end\n"
        "end\n";

    for (const ElementToDump &element : elements)
    {
        script += std::string("dump(\"") + element.reportTag + "\", "
                + (element.kind == NODE ? "node" : "link")
                + "(\"" + element.elementId + "\"), "
                + (element.kind == NODE ? "nodeprops" : "linkprops") + ")\n";
    }
    return script;
}

inline bool reportMentionsLuaError(const std::string &report)
{
    return report.find("Lua script error") != std::string::npos;
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

    int readValue(ElementKind kind, const char *elementId, int enProperty,
                  double *value)
    {
        int index;
        if (kind == NODE)
        {
            EN_getnodeindex(ph, (char *)elementId, &index);
            return EN_getnodevalue(ph, index, enProperty, value);
        }
        EN_getlinkindex(ph, (char *)elementId, &index);
        return EN_getlinkvalue(ph, index, enProperty, value);
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
