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
static const char *RESOLVE_TEST_INP = "./lua-api-resolve.inp";
static const char *RESOLVE_TEST_RPT = "./lua-api-resolve.rpt";
static const char *RESOLVE_REFERENCE_RPT = "./lua-api-resolve-ref.rpt";
static const char *READ_ONLY_OPTION_INP = "./lua-api-readonly-option.inp";
static const char *READ_ONLY_OPTION_RPT = "./lua-api-readonly-option.rpt";
static const char *UNKNOWN_OPTION_INP = "./lua-api-unknown-option.inp";
static const char *UNKNOWN_OPTION_RPT = "./lua-api-unknown-option.rpt";

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

static const std::vector<PropertyWrite> WRITABLE_OPTION_WRITES = {
    { OPTIONS, "", "trials",               EN_TRIALS,        60           },
    { OPTIONS, "", "accuracy",             EN_ACCURACY,      0.005        },
    { OPTIONS, "", "tolerance",            EN_TOLERANCE,     0.02         },
    { OPTIONS, "", "emitter_exponent",     EN_EMITEXPON,     0.6          },
    { OPTIONS, "", "demand_multiplier",    EN_DEMANDMULT,    1.5          },
    { OPTIONS, "", "head_error",           EN_HEADERROR,     0.5          },
    { OPTIONS, "", "flow_change",          EN_FLOWCHANGE,    0.75         },
    { OPTIONS, "", "global_efficiency",    EN_GLOBALEFFIC,   80           },
    { OPTIONS, "", "global_price",         EN_GLOBALPRICE,   0.15         },
    { OPTIONS, "", "global_pattern",       EN_GLOBALPATTERN, 2            },
    { OPTIONS, "", "demand_charge",        EN_DEMANDCHARGE,  12.5         },
    { OPTIONS, "", "specific_gravity",     EN_SP_GRAVITY,    1.2          },
    { OPTIONS, "", "specific_viscosity",   EN_SP_VISCOS,     1.1          },
    { OPTIONS, "", "unbalanced",           EN_UNBALANCED,    15           },
    { OPTIONS, "", "check_frequency",      EN_CHECKFREQ,     3            },
    { OPTIONS, "", "max_check",            EN_MAXCHECK,      12           },
    { OPTIONS, "", "damp_limit",           EN_DAMPLIMIT,     0.05         },
    { OPTIONS, "", "specific_diffusivity", EN_SP_DIFFUS,     1.3          },
    { OPTIONS, "", "bulk_order",           EN_BULKORDER,     0.5          },
    { OPTIONS, "", "wall_order",           EN_WALLORDER,     0            },
    { OPTIONS, "", "tank_order",           EN_TANKORDER,     0.5          },
    { OPTIONS, "", "concentration_limit",  EN_CONCENLIMIT,   4            },
    { OPTIONS, "", "demand_pattern",       EN_DEMANDPATTERN, 2            },
    { OPTIONS, "", "emitter_backflow",     EN_EMITBACKFLOW,  0            },
    { OPTIONS, "", "pressure_units",       EN_PRESS_UNITS,   EN_METERS    },
    { OPTIONS, "", "status_report",        EN_STATUS_REPORT, EN_NO_REPORT },
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

static const std::vector<LuaProperty> OPTION_PROPERTIES = {
    { "trials",               EN_TRIALS         },
    { "accuracy",             EN_ACCURACY       },
    { "tolerance",            EN_TOLERANCE      },
    { "emitter_exponent",     EN_EMITEXPON      },
    { "demand_multiplier",    EN_DEMANDMULT     },
    { "head_error",           EN_HEADERROR      },
    { "flow_change",          EN_FLOWCHANGE     },
    { "headloss_form",        EN_HEADLOSSFORM   },
    { "global_efficiency",    EN_GLOBALEFFIC    },
    { "global_price",         EN_GLOBALPRICE    },
    { "global_pattern",       EN_GLOBALPATTERN  },
    { "demand_charge",        EN_DEMANDCHARGE   },
    { "specific_gravity",     EN_SP_GRAVITY     },
    { "specific_viscosity",   EN_SP_VISCOS      },
    { "unbalanced",           EN_UNBALANCED     },
    { "check_frequency",      EN_CHECKFREQ      },
    { "max_check",            EN_MAXCHECK       },
    { "damp_limit",           EN_DAMPLIMIT      },
    { "specific_diffusivity", EN_SP_DIFFUS      },
    { "bulk_order",           EN_BULKORDER      },
    { "wall_order",           EN_WALLORDER      },
    { "tank_order",           EN_TANKORDER      },
    { "concentration_limit",  EN_CONCENLIMIT    },
    { "demand_pattern",       EN_DEMANDPATTERN  },
    { "emitter_backflow",     EN_EMITBACKFLOW   },
    { "pressure_units",       EN_PRESS_UNITS    },
    { "status_report",        EN_STATUS_REPORT  },
};

static const std::vector<ElementToDump> DUMPED_ELEMENTS = {
    { NODE,    "11",   "n11",   &NODE_PROPERTIES   },
    { NODE,    "2",    "n2",    &NODE_PROPERTIES   },
    { LINK,    "110",  "l110",  &LINK_PROPERTIES   },
    { LINK,    "9",    "l9",    &LINK_PROPERTIES   },
    { LINK,    "VGPV", "lVGPV", &LINK_PROPERTIES   },
    { OPTIONS, "",     "opt",   &OPTION_PROPERTIES },
};

static std::vector<PropertyWrite> everyWritableProperty()
{
    std::vector<PropertyWrite> writes = WRITABLE_OPTION_WRITES;
    writes.insert(
        writes.end(),
        WRITABLE_PROPERTY_WRITES.begin(),
        WRITABLE_PROPERTY_WRITES.end()
    );
    
    return writes;
}

static std::string scriptWritingEveryWritableProperty()
{
    std::string script;
    for (const PropertyWrite &write : everyWritableProperty())
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

    for (const PropertyWrite &write : everyWritableProperty())
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
                                     luaDumpScript(DUMPED_ELEMENTS)));

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
        for (const LuaProperty &property : *element.properties)
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

BOOST_AUTO_TEST_CASE(script_cannot_write_a_read_only_option)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, READ_ONLY_OPTION_INP,
                                     "options().headloss_form = 1\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(READ_ONLY_OPTION_INP, READ_ONLY_OPTION_RPT) == 0);
    BOOST_REQUIRE(project.solveOneHydraulicStep() == 0);

    double headlossForm;
    BOOST_CHECK(project.readValue(OPTIONS, "", EN_HEADLOSSFORM,
                                  &headlossForm) == 0);
    BOOST_CHECK_EQUAL(headlossForm, EN_HW);

    project.close();

    BOOST_CHECK(readWholeFile(READ_ONLY_OPTION_RPT).find(
        "options property is read only: headloss_form") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(script_cannot_use_an_unknown_option)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, UNKNOWN_OPTION_INP,
                                     "options().not_an_option = 1\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(UNKNOWN_OPTION_INP, UNKNOWN_OPTION_RPT) == 0);
    BOOST_REQUIRE(project.solveOneHydraulicStep() == 0);
    project.close();

    BOOST_CHECK(readWholeFile(UNKNOWN_OPTION_RPT).find(
        "unknown options property: not_an_option") != std::string::npos);
}

// A script write invalidates the solution the solver just converged on,
// so the solver must re-converge before reporting results.
//
// Halving the roughness of main pipe 10 is used because, unlike a valve
// change, it does not trip the solver's own status checks: only the change 
// flag raised by the script bindings can trigger the re-solve.
//
// The solver's periodic status checks are disabled because they also run
// the script mid-solve, which would let the change slip in before convergence 
// and mask a broken change flag. MAXCHECK 1 disables them: the parser
// rejects 0, and the first check would come at iteration CHECKFREQ (2),
// already past it.
static const char *PERIODIC_STATUS_CHECKS_OFF =
    "[OPTIONS]\n"
    " MAXCHECK  1\n"
    "\n";

BOOST_AUTO_TEST_CASE(script_writes_take_effect_in_the_same_timestep)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, RESOLVE_TEST_INP,
                                     "link(\"10\").roughness = 50\n",
                                     PERIODIC_STATUS_CHECKS_OFF));

    ProjectUnderTest scripted;
    BOOST_REQUIRE(scripted.open(RESOLVE_TEST_INP, RESOLVE_TEST_RPT) == 0);
    BOOST_REQUIRE(scripted.solveOneHydraulicStep() == 0);
    double scriptedPressure;
    BOOST_REQUIRE(scripted.readValue(NODE, "11", EN_PRESSURE,
                                     &scriptedPressure) == 0);

    ProjectUnderTest reference;
    BOOST_REQUIRE(reference.open(BASE_INP, RESOLVE_REFERENCE_RPT) == 0);
    BOOST_REQUIRE(reference.writeValue(LINK, "10", EN_ROUGHNESS, 50) == 0);
    BOOST_REQUIRE(reference.solveOneHydraulicStep() == 0);
    double referencePressure;
    BOOST_REQUIRE(reference.readValue(NODE, "11", EN_PRESSURE,
                                      &referencePressure) == 0);

    BOOST_CHECK_CLOSE(scriptedPressure, referencePressure, 0.5);
}

BOOST_AUTO_TEST_SUITE_END()
