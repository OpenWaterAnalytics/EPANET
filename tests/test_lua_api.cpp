/*
******************************************************************************
Project:      OWA EPANET
Version:      2.4
Module:       test_lua_api.cpp
Description:  Acceptance test for the whole Lua scripting API surface
Authors:      see AUTHORS
Copyright:    see AUTHORS
License:      see LICENSE
******************************************************************************
*/

#define BOOST_TEST_MODULE lua_api

#include <cmath>

#include <boost/test/included/unit_test.hpp>

#include "lua_test_utils.hpp"

static const char *BASE_INP = "./net_lua_test.inp";
static const char *WRITE_TEST_INP = "./lua-api-write.inp";
static const char *WRITE_TEST_RPT = "./lua-api-write.rpt";
static const char *READ_TEST_INP = "./lua-api-read.inp";
static const char *READ_TEST_RPT = "./lua-api-read.rpt";

static const std::vector<PropertyWrite> WRITABLE_PROPERTY_WRITES = {
    { NODE, "10",   "elevation",      EN_ELEVATION,    712.5  },
    { NODE, "11",   "base_demand",    EN_BASEDEMAND,   155.5  },
    { NODE, "12",   "pattern",        EN_PATTERN,      2      },
    { NODE, "13",   "emitter",        EN_EMITTER,      0.75   },
    { NODE, "21",   "init_quality",   EN_INITQUAL,     0.8    },
    { NODE, "22",   "source_quality", EN_SOURCEQUAL,   1.25   },
    { NODE, "23",   "source_pattern", EN_SOURCEPAT,    2      },
    { NODE, "31",   "source_type",    EN_SOURCETYPE,   3      },
    { NODE, "2",    "min_level",      EN_MINLEVEL,     95     },
    { NODE, "2",    "max_level",      EN_MAXLEVEL,     155    },
    { NODE, "2",    "tank_level",     EN_TANKLEVEL,    125    },
    { NODE, "2",    "tank_diameter",  EN_TANKDIAM,     52     },
    { NODE, "2",    "min_volume",     EN_MINVOLUME,    12000  },
    { NODE, "2",    "mix_model",      EN_MIXMODEL,     2      },
    { NODE, "2",    "mix_fraction",   EN_MIXFRACTION,  0.5    },
    { NODE, "2",    "bulk_coeff",     EN_TANK_KBULK,   -0.4   },
    { NODE, "2",    "can_overflow",   EN_CANOVERFLOW,  1      },
    { NODE, "T2",   "volume_curve",   EN_VOLCURVE,     7      },
    { LINK, "10",   "diameter",       EN_DIAMETER,     20     },
    { LINK, "11",   "length",         EN_LENGTH,       5400   },
    { LINK, "12",   "roughness",      EN_ROUGHNESS,    110    },
    { LINK, "21",   "minor_loss",     EN_MINORLOSS,    0.5    },
    { LINK, "22",   "init_status",    EN_INITSTATUS,   0      },
    { LINK, "31",   "init_setting",   EN_INITSETTING,  105    },
    { LINK, "110",  "bulk_coeff",     EN_KBULK,        -0.3   },
    { LINK, "111",  "wall_coeff",     EN_KWALL,        -0.6   },
    { LINK, "112",  "setting",        EN_SETTING,      108    },
    { LINK, "113",  "status",         EN_STATUS,       0      },
    { LINK, "121",  "leak_area",      EN_LEAK_AREA,    10     },
    { LINK, "122",  "leak_expansion", EN_LEAK_EXPAN,   0.1    },
    { LINK, "9",    "pump_hcurve",    EN_PUMP_HCURVE,  2      },
    { LINK, "9",    "pump_ecurve",    EN_PUMP_ECURVE,  3      },
    { LINK, "9",    "pump_ecost",     EN_PUMP_ECOST,   0.12   },
    { LINK, "9",    "pump_epattern",  EN_PUMP_EPAT,    2      },
    { LINK, "9",    "pattern",        EN_LINKPATTERN,  2      },
    { LINK, "P9B",  "pump_power",     EN_PUMP_POWER,   50     },
    { LINK, "VGPV", "gpv_curve",      EN_GPV_CURVE,    5      },
    { LINK, "VPCV", "pcv_curve",      EN_PCV_CURVE,    6      },
};

static const std::vector<LuaProperty> NODE_PROPERTIES = {
    { "elevation",       EN_ELEVATION      },
    { "base_demand",     EN_BASEDEMAND     },
    { "pattern",         EN_PATTERN        },
    { "emitter",         EN_EMITTER        },
    { "init_quality",    EN_INITQUAL       },
    { "source_quality",  EN_SOURCEQUAL     },
    { "source_pattern",  EN_SOURCEPAT      },
    { "source_type",     EN_SOURCETYPE     },
    { "tank_level",      EN_TANKLEVEL      },
    { "demand",          EN_DEMAND         },
    { "head",            EN_HEAD           },
    { "pressure",        EN_PRESSURE       },
    { "quality",         EN_QUALITY        },
    { "source_mass",     EN_SOURCEMASS     },
    { "init_volume",     EN_INITVOLUME     },
    { "mix_model",       EN_MIXMODEL       },
    { "mix_zone_volume", EN_MIXZONEVOL     },
    { "tank_diameter",   EN_TANKDIAM       },
    { "min_volume",      EN_MINVOLUME      },
    { "volume_curve",    EN_VOLCURVE       },
    { "min_level",       EN_MINLEVEL       },
    { "max_level",       EN_MAXLEVEL       },
    { "mix_fraction",    EN_MIXFRACTION    },
    { "bulk_coeff",      EN_TANK_KBULK     },
    { "tank_volume",     EN_TANKVOLUME     },
    { "max_volume",      EN_MAXVOLUME      },
    { "can_overflow",    EN_CANOVERFLOW    },
    { "demand_deficit",  EN_DEMANDDEFICIT  },
    { "in_control",      EN_NODE_INCONTROL },
    { "emitter_flow",    EN_EMITTERFLOW    },
    { "leakage_flow",    EN_LEAKAGEFLOW    },
    { "demand_flow",     EN_DEMANDFLOW     },
    { "full_demand",     EN_FULLDEMAND     },
};

static const std::vector<LuaProperty> LINK_PROPERTIES = {
    { "diameter",        EN_DIAMETER       },
    { "length",          EN_LENGTH         },
    { "roughness",       EN_ROUGHNESS      },
    { "minor_loss",      EN_MINORLOSS      },
    { "init_status",     EN_INITSTATUS     },
    { "init_setting",    EN_INITSETTING    },
    { "bulk_coeff",      EN_KBULK          },
    { "wall_coeff",      EN_KWALL          },
    { "flow",            EN_FLOW           },
    { "velocity",        EN_VELOCITY       },
    { "headloss",        EN_HEADLOSS       },
    { "status",          EN_STATUS         },
    { "setting",         EN_SETTING        },
    { "energy",          EN_ENERGY         },
    { "quality",         EN_LINKQUAL       },
    { "pattern",         EN_LINKPATTERN    },
    { "pump_state",      EN_PUMP_STATE     },
    { "pump_efficiency", EN_PUMP_EFFIC     },
    { "pump_power",      EN_PUMP_POWER     },
    { "pump_hcurve",     EN_PUMP_HCURVE    },
    { "pump_ecurve",     EN_PUMP_ECURVE    },
    { "pump_ecost",      EN_PUMP_ECOST     },
    { "pump_epattern",   EN_PUMP_EPAT      },
    { "in_control",      EN_LINK_INCONTROL },
    { "gpv_curve",       EN_GPV_CURVE      },
    { "pcv_curve",       EN_PCV_CURVE      },
    { "leak_area",       EN_LEAK_AREA      },
    { "leak_expansion",  EN_LEAK_EXPAN     },
    { "leakage",         EN_LINK_LEAKAGE   },
    { "valve_type",      EN_VALVE_TYPE     },
};

static const std::vector<ElementToDump> DUMPED_ELEMENTS = {
    { NODE, "11",   "n11"   },
    { NODE, "2",    "n2"    },
    { LINK, "110",  "l110"  },
    { LINK, "9",    "l9"    },
    { LINK, "VGPV", "lVGPV" },
};

static std::string scriptWritingEveryWritableProperty()
{
    std::string script;
    for (const PropertyWrite &write : WRITABLE_PROPERTY_WRITES)
    {
        script += luaAssignment(write);
    }
    return script;
}

static bool finalValueMatchesScriptValue(const LuaProperty &property)
{
    return std::string(property.luaName) != "demand";
}


BOOST_AUTO_TEST_SUITE(test_lua_api)

BOOST_AUTO_TEST_CASE(script_writes_all_writable_properties)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, WRITE_TEST_INP,
                                     scriptWritingEveryWritableProperty()));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(WRITE_TEST_INP, WRITE_TEST_RPT) == 0);
    BOOST_REQUIRE(project.solveOneHydraulicStep() == 0);

    for (const PropertyWrite &write : WRITABLE_PROPERTY_WRITES)
    {
        double value;
        int error = project.readValue(write.kind, write.elementId,
                                      write.enProperty, &value);
        BOOST_CHECK_MESSAGE(error == 0, "error " << error << " reading "
            << write.elementId << "." << write.luaProperty);
        if (error != 0) continue;

        if (write.value == 0.0) BOOST_CHECK_SMALL(value, 1e-6);
        else BOOST_CHECK_CLOSE(value, write.value, 0.01);
    }

    project.close();
    BOOST_CHECK(!reportMentionsLuaError(readWholeFile(WRITE_TEST_RPT)));
}

BOOST_AUTO_TEST_CASE(script_reads_all_properties_into_report)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, READ_TEST_INP,
        luaDumpScript(NODE_PROPERTIES, LINK_PROPERTIES, DUMPED_ELEMENTS)));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(READ_TEST_INP, READ_TEST_RPT) == 0);
    BOOST_REQUIRE(project.solveOneHydraulicStep() == 0);

    struct ExpectedLine
    {
        std::string label;
        int readError;
        double value;
        bool valueIsComparable;
    };
    std::vector<ExpectedLine> expectedLines;

    for (const ElementToDump &element : DUMPED_ELEMENTS)
    {
        const std::vector<LuaProperty> &properties =
            element.kind == NODE ? NODE_PROPERTIES : LINK_PROPERTIES;

        for (const LuaProperty &property : properties)
        {
            ExpectedLine line;
            line.label = std::string(element.reportTag) + "."
                       + property.luaName + "=";
            line.readError = project.readValue(element.kind, element.elementId,
                                               property.enProperty, &line.value);
            line.valueIsComparable = finalValueMatchesScriptValue(property);
            expectedLines.push_back(line);
        }
    }

    project.close();

    std::string report = readWholeFile(READ_TEST_RPT);
    BOOST_REQUIRE(!report.empty());
    BOOST_CHECK(!reportMentionsLuaError(report));

    for (const ExpectedLine &line : expectedLines)
    {
        double printed;
        bool found = findLastPrintedValue(report, line.label, &printed);

        if (line.readError != 0)
        {
            BOOST_CHECK_MESSAGE(!found, line.label << " printed but the API "
                << "fails with error " << line.readError);
            continue;
        }

        BOOST_CHECK_MESSAGE(found, line.label << " missing from report");
        if (!found || !line.valueIsComparable) continue;

        if (line.value == 0.0) BOOST_CHECK_SMALL(printed, 1e-6);
        else BOOST_CHECK_MESSAGE(
            std::abs(printed - line.value) <= 0.001 * std::abs(line.value),
            line.label << printed << " but the API returns " << line.value);
    }
}

BOOST_AUTO_TEST_SUITE_END()
