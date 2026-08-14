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
static const char *RESOLVE_REFERENCE_RPT = "./lua-api-resolve-ref.rpt";
static const char *READ_ONLY_OPTION_INP = "./lua-api-readonly-option.inp";
static const char *READ_ONLY_OPTION_RPT = "./lua-api-readonly-option.rpt";
static const char *EVERY_OPTION_INP = "./lua-api-every-option.inp";
static const char *EVERY_OPTION_RPT = "./lua-api-every-option.rpt";
static const char *UNKNOWN_OPTION_INP = "./lua-api-unknown-option.inp";
static const char *UNKNOWN_OPTION_RPT = "./lua-api-unknown-option.rpt";
static const char *READ_ONLY_TIME_INP = "./lua-api-readonly-time.inp";
static const char *READ_ONLY_TIME_RPT = "./lua-api-readonly-time.rpt";
static const char *REPORT_EVENT_INP = "./lua-api-report-event.inp";
static const char *REPORT_EVENT_RPT = "./lua-api-report-event.rpt";
static const char *ITERATION_EVENT_INP = "./lua-api-iteration-event.inp";
static const char *ITERATION_EVENT_RPT = "./lua-api-iteration-event.rpt";
static const char *CLOSE_EVENT_INP = "./lua-api-close-event.inp";
static const char *CLOSE_EVENT_RPT = "./lua-api-close-event.rpt";
static const char *RUNAWAY_EVENT_INP = "./lua-api-runaway-event.inp";
static const char *RUNAWAY_EVENT_RPT = "./lua-api-runaway-event.rpt";
static const char *CURVE_INP = "./lua-api-curve.inp";
static const char *CURVE_RPT = "./lua-api-curve.rpt";
static const char *UNKNOWN_CURVE_INP = "./lua-api-unknown-curve.inp";
static const char *UNKNOWN_CURVE_RPT = "./lua-api-unknown-curve.rpt";
static const char *SYNTAX_ERROR_INP = "./lua-api-syntax-error.inp";
static const char *SYNTAX_ERROR_RPT = "./lua-api-syntax-error.rpt";
static const char *SYNTAX_ERROR_LINE_INP = "./lua-api-syntax-error-line.inp";
static const char *SYNTAX_ERROR_LINE_RPT = "./lua-api-syntax-error-line.rpt";
static const char *BRACKET_LINE_INP = "./lua-api-bracket-line.inp";
static const char *BRACKET_LINE_RPT = "./lua-api-bracket-line.rpt";
static const char *BAD_SECTION_INP = "./lua-api-bad-section.inp";
static const char *BAD_SECTION_RPT = "./lua-api-bad-section.rpt";
static const char *SECTION_AFTER_INP = "./lua-api-section-after.inp";
static const char *SECTION_AFTER_RPT = "./lua-api-section-after.rpt";
static const char *STEP_ERROR_INP = "./lua-api-step-error.inp";
static const char *STEP_ERROR_RPT = "./lua-api-step-error.rpt";
static const char *OPEN_ERROR_INP = "./lua-api-open-error.inp";
static const char *OPEN_ERROR_RPT = "./lua-api-open-error.rpt";
static const char *CLOSE_ERROR_INP = "./lua-api-close-error.inp";
static const char *CLOSE_ERROR_RPT = "./lua-api-close-error.rpt";
static const char *EVENT_BASELINE_RPT = "./lua-api-event-baseline.rpt";
static const char *ROUND_TRIP_INP = "./lua-api-round-trip.inp";
static const char *ROUND_TRIP_RPT = "./lua-api-round-trip.rpt";
static const char *ROUND_TRIP_SAVED_INP = "./lua-api-round-trip-saved.inp";
static const char *ROUND_TRIP_SAVED_RPT = "./lua-api-round-trip-saved.rpt";

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


static const std::vector<PropertyWrite> WRITABLE_TIME_WRITES = {
    { TIMES, "", "duration",        EN_DURATION,      43200 },
    { TIMES, "", "pattern_step",    EN_PATTERNSTEP,   5400  },
    { TIMES, "", "report_step",     EN_REPORTSTEP,    2700  },
    { TIMES, "", "hydraulic_step",  EN_HYDSTEP,       1800  },
    { TIMES, "", "quality_step",    EN_QUALSTEP,      900   },
    { TIMES, "", "rule_step",       EN_RULESTEP,      600   },
    { TIMES, "", "pattern_start",   EN_PATTERNSTART,  3600  },
    { TIMES, "", "report_start",    EN_REPORTSTART,   7200  },
    { TIMES, "", "start_time",      EN_STARTTIME,     21600 },
    { TIMES, "", "statistic",       EN_STATISTIC,     1     },
    { TIMES, "", "hydraulic_time",  EN_HTIME,         5400  },
    { TIMES, "", "quality_time",    EN_QTIME,         1200  },
};

static const std::vector<LuaProperty> TIME_PROPERTIES = {
    { "duration",        EN_DURATION      },
    { "hydraulic_step",  EN_HYDSTEP       },
    { "quality_step",    EN_QUALSTEP      },
    { "pattern_step",    EN_PATTERNSTEP   },
    { "pattern_start",   EN_PATTERNSTART  },
    { "report_step",     EN_REPORTSTEP    },
    { "report_start",    EN_REPORTSTART   },
    { "rule_step",       EN_RULESTEP      },
    { "statistic",       EN_STATISTIC     },
    { "periods",         EN_PERIODS       },
    { "start_time",      EN_STARTTIME     },
    { "hydraulic_time",  EN_HTIME         },
    { "quality_time",    EN_QTIME         },
    { "halt_flag",       EN_HALTFLAG      },
    { "next_event",      EN_NEXTEVENT     },
    { "next_event_tank", EN_NEXTEVENTTANK },
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
    { TIMES,   "",     "time",  &TIME_PROPERTIES   },
};

static std::vector<PropertyWrite> everyWritableProperty()
{
    std::vector<PropertyWrite> writes = WRITABLE_TIME_WRITES;
    writes.insert(
        writes.end(),
        WRITABLE_PROPERTY_WRITES.begin(),
        WRITABLE_PROPERTY_WRITES.end()
    );

    return writes;
}

// Writes go in the iteration handler, the event meant for changing the
// network: it fires once the step has converged, and whatever it changes
// sends the solver round again before results are saved
static std::string scriptWritingEveryWritableProperty()
{
    std::string body;
    for (const PropertyWrite &write : everyWritableProperty())
    {
        body += "    " + luaAssignment(write);
    }
    return luaEventHandler("on_hydraulic_step", body);
}

static bool finalValueMatchesScriptValue(const ElementToDump &element,
                                         const LuaProperty &property)
{
    const std::string name = property.luaName;
    const std::string id = element.elementId;

    if (name == "demand") return false;

    // The clock is only advanced once the handler has run
    if (element.kind == TIMES) return name != "hydraulic_time";

    // Tank 2's level is advanced after the handler has run, which moves
    // its own results along with those of pipe 110, the pipe joined to it
    if (element.kind == NODE && id == "2")
    {
        return name != "head" && name != "pressure" && name != "tank_volume";
    }
    if (element.kind == LINK && id == "110")
    {
        return name != "headloss" && name != "energy";
    }

    return true;
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
    // Reads go in the report handler, which runs once the step has been
    // solved and its results saved
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, READ_TEST_INP,
                                     luaDumpScript(DUMPED_ELEMENTS,
                                                   "on_hydraulics_solved")));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(READ_TEST_INP, READ_TEST_RPT) == 0);
    BOOST_REQUIRE(project.solveAndAdvanceOneHydraulicStep() == 0);

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
            line.valueIsComparable = finalValueMatchesScriptValue(element,
                                                                  property);
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

BOOST_AUTO_TEST_CASE(script_cannot_write_an_option)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, READ_ONLY_OPTION_INP,
                                     "options().demand_multiplier = 1.5\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(READ_ONLY_OPTION_INP, READ_ONLY_OPTION_RPT) == 0);
    BOOST_REQUIRE_EQUAL(project.solveOneHydraulicStep(), 313);

    double multiplier;
    BOOST_CHECK(project.readValue(OPTIONS, "", EN_DEMANDMULT, &multiplier) == 0);
    BOOST_CHECK_NE(multiplier, 1.5);

    project.close();

    BOOST_CHECK(readWholeFile(READ_ONLY_OPTION_RPT).find(
        "options property is read only: demand_multiplier") != std::string::npos);
}

// Every option, not just a sample of them: the whole table is read only,
// so each assignment has to raise and leave the value where it was
BOOST_AUTO_TEST_CASE(script_cannot_write_any_option)
{
    std::string body;
    for (const LuaProperty &property : OPTION_PROPERTIES)
    {
        body += std::string("    refuse(\"") + property.luaName + "\")\n";
    }

    BOOST_REQUIRE(buildInpWithScript(BASE_INP, EVERY_OPTION_INP,
        "tried, refused = 0, 0\n"
        "local function refuse(name)\n"
        "    tried = tried + 1\n"
        "    local before = options()[name]\n"
        "    local ok = pcall(function() options()[name] = before + 1 end)\n"
        "    if not ok and options()[name] == before then\n"
        "        refused = refused + 1\n"
        "    end\n"
        "end\n"
      + luaEventHandler("on_hydraulic_step",
            body + "    print(\"tried=\" .. tried)\n"
                   "    print(\"refused=\" .. refused)\n")));

    ProjectUnderTest project;
    BOOST_REQUIRE_EQUAL(project.open(EVERY_OPTION_INP, EVERY_OPTION_RPT), 0);
    BOOST_REQUIRE_EQUAL(project.solveOneHydraulicStep(), 0);
    project.close();

    std::string report = readWholeFile(EVERY_OPTION_RPT);
    double tried = 0, refused = 0;
    BOOST_REQUIRE(findLastPrintedValue(report, "tried=", &tried));
    BOOST_REQUIRE(findLastPrintedValue(report, "refused=", &refused));

    BOOST_CHECK_EQUAL(tried, (double)OPTION_PROPERTIES.size());
    BOOST_CHECK_EQUAL(refused, (double)OPTION_PROPERTIES.size());
}

BOOST_AUTO_TEST_CASE(script_cannot_write_a_read_only_time_parameter)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, READ_ONLY_TIME_INP,
                                     "times().periods = 5\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(READ_ONLY_TIME_INP, READ_ONLY_TIME_RPT) == 0);
    BOOST_REQUIRE_EQUAL(project.solveOneHydraulicStep(), 313);

    double periods;
    BOOST_CHECK(project.readValue(TIMES, "", EN_PERIODS, &periods) == 0);
    BOOST_CHECK(periods != 5);

    project.close();

    BOOST_CHECK(readWholeFile(READ_ONLY_TIME_RPT).find(
        "times property is read only: periods") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(script_cannot_use_an_unknown_option)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, UNKNOWN_OPTION_INP,
                                     "options().not_an_option = 1\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(UNKNOWN_OPTION_INP, UNKNOWN_OPTION_RPT) == 0);
    BOOST_REQUIRE_EQUAL(project.solveOneHydraulicStep(), 313);
    project.close();

    BOOST_CHECK(readWholeFile(UNKNOWN_OPTION_RPT).find(
        "unknown options property: not_an_option") != std::string::npos);
}

// The [SCRIPT] chunk is evaluated once, when the project opens, and its
// only job is to define handlers: by the time it runs nothing has been
// solved yet, so a top-level read would see an empty network
BOOST_AUTO_TEST_CASE(report_event_runs_once_per_time_step)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, REPORT_EVENT_INP,
        "function on_hydraulics_solved()\n"
        "    print(\"on_hydraulics_solved p11=\" .. tostring(node(\"11\").pressure))\n"
        "end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(REPORT_EVENT_INP, REPORT_EVENT_RPT) == 0);

    int steps = 0;
    BOOST_REQUIRE(project.solveAllHydraulicSteps(&steps) == 0);
    BOOST_REQUIRE(steps > 1);

    double finalPressure;
    BOOST_REQUIRE(project.readValue(NODE, "11", EN_PRESSURE,
                                    &finalPressure) == 0);
    project.close();

    std::string report = readWholeFile(REPORT_EVENT_RPT);
    BOOST_CHECK(!reportMentionsLuaError(report));
    BOOST_CHECK_EQUAL(countOccurrences(report, "on_hydraulics_solved p11="), steps);

    // The handler runs after the step has converged, so the pressure it
    // sees is the one the step ended on
    double printed;
    BOOST_REQUIRE(findLastPrintedValue(report, "on_hydraulics_solved p11=", &printed));
    BOOST_CHECK_CLOSE(printed, finalPressure, 0.01);
}

// on_open fires from EN_initH and on_close from EN_closeH, so they
// bracket the run: one open before every report, one close after them
// all, and the network is already in place when the first one runs
BOOST_AUTO_TEST_CASE(open_and_close_events_bracket_the_run)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, CLOSE_EVENT_INP,
        "function on_open()\n"
        "    print(\"event on_open n11elev=\" .. tostring(node(\"11\").elevation))\n"
        "end\n"
        "function on_hydraulics_solved() print(\"event on_hydraulics_solved\") end\n"
        "function on_close() print(\"event on_close\") end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(CLOSE_EVENT_INP, CLOSE_EVENT_RPT) == 0);

    double elevation;
    BOOST_REQUIRE(project.readValue(NODE, "11", EN_ELEVATION, &elevation) == 0);

    int steps = 0;
    BOOST_REQUIRE(project.solveAllHydraulicSteps(&steps) == 0);
    BOOST_REQUIRE(steps > 1);
    project.close();

    std::string report = readWholeFile(CLOSE_EVENT_RPT);
    BOOST_CHECK(!reportMentionsLuaError(report));

    BOOST_CHECK_EQUAL(countOccurrences(report, "event on_open n11elev="), 1);
    BOOST_CHECK_EQUAL(countOccurrences(report, "event on_close"), 1);
    BOOST_CHECK_EQUAL(countOccurrences(report, "event on_hydraulics_solved"), steps);

    // The parsed network is readable from on_open, which is what firing
    // it after the input has been read buys
    double printed;
    BOOST_REQUIRE(findLastPrintedValue(report, "event on_open n11elev=",
                                       &printed));
    BOOST_CHECK_CLOSE(printed, elevation, 0.01);

    BOOST_CHECK(report.find("event on_open") < report.find("event on_hydraulics_solved"));
    BOOST_CHECK(report.rfind("event on_close")
                > report.rfind("event on_hydraulics_solved"));
}

// The iteration event fires once the step has converged, and a change
// made there has to send the solver round again before results are
// saved. Halving the roughness of main pipe 10 is used because, unlike a
// valve change, it does not trip the solver's own status checks: only
// the change flag raised by the handler can trigger the re-solve
BOOST_AUTO_TEST_CASE(iteration_event_changes_take_effect_in_the_same_timestep)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, ITERATION_EVENT_INP,
        "function on_hydraulic_step() link(\"10\").roughness = 50 end\n"));

    ProjectUnderTest scripted;
    BOOST_REQUIRE(scripted.open(ITERATION_EVENT_INP, ITERATION_EVENT_RPT) == 0);
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

    // Without the re-solve the step would report the untouched network,
    // so the baseline is what a passing test must not look like
    ProjectUnderTest baseline;
    BOOST_REQUIRE(baseline.open(BASE_INP, EVENT_BASELINE_RPT) == 0);
    BOOST_REQUIRE(baseline.solveOneHydraulicStep() == 0);
    double baselinePressure;
    BOOST_REQUIRE(baseline.readValue(NODE, "11", EN_PRESSURE,
                                     &baselinePressure) == 0);

    BOOST_CHECK_CLOSE(scriptedPressure, referencePressure, 0.5);
    BOOST_CHECK(std::abs(scriptedPressure - baselinePressure) > 0.5);
}

// A handler that changes something on every call would re-solve forever,
// so runhyd caps the number of passes
BOOST_AUTO_TEST_CASE(iteration_event_re_solving_is_bounded)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, RUNAWAY_EVENT_INP,
        "function on_hydraulic_step()\n"
        "    print(\"iteration pass\")\n"
        "    link(\"10\").roughness = link(\"10\").roughness - 1\n"
        "end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(RUNAWAY_EVENT_INP, RUNAWAY_EVENT_RPT) == 0);
    BOOST_REQUIRE(project.solveOneHydraulicStep() == 0);
    project.close();

    std::string report = readWholeFile(RUNAWAY_EVENT_RPT);
    BOOST_CHECK(!reportMentionsLuaError(report));

    int passes = countOccurrences(report, "iteration pass");
    BOOST_CHECK_MESSAGE(passes > 1, "the handler ran " << passes << " times, "
                        << "so its change never triggered a re-solve");
    BOOST_CHECK_MESSAGE(passes <= 11, "the handler ran " << passes << " times, "
                        << "so re-solving is not capped");
}

// Curves of one, two and three points, so a single-point curve is not
// mistaken for a bare {x, y} pair and the array nesting is exercised
static const std::vector<const char *> DUMPED_CURVES = { "1", "4", "6" };

static std::string scriptDumpingCurves()
{
    std::string body =
        "    for _, id in ipairs(" + luaStringList(DUMPED_CURVES) + ") do\n"
        "        local points = curve(id)\n"
        "        print(\"curve.\" .. id .. \".count=\" .. tostring(#points))\n"
        "        for i, point in ipairs(points) do\n"
        "            print(\"curve.\" .. id .. \".\" .. i .. \".x=\"\n"
        "                  .. tostring(point[1]))\n"
        "            print(\"curve.\" .. id .. \".\" .. i .. \".y=\"\n"
        "                  .. tostring(point[2]))\n"
        "        end\n"
        "    end\n";
    return luaEventHandler("on_hydraulics_solved", body);
}

BOOST_AUTO_TEST_CASE(script_reads_every_point_of_a_curve)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, CURVE_INP, scriptDumpingCurves()));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(CURVE_INP, CURVE_RPT) == 0);
    BOOST_REQUIRE(project.solveAndAdvanceOneHydraulicStep() == 0);

    struct ExpectedCurve
    {
        std::string id;
        int length;
        std::vector<double> x, y;
    };
    std::vector<ExpectedCurve> expected;

    for (const char *id : DUMPED_CURVES)
    {
        ExpectedCurve curve;
        curve.id = id;

        int index = 0;
        BOOST_REQUIRE(EN_getcurveindex(project.ph, id, &index) == 0);
        BOOST_REQUIRE(EN_getcurvelen(project.ph, index, &curve.length) == 0);
        BOOST_REQUIRE(curve.length > 0);

        for (int point = 1; point <= curve.length; point++)
        {
            double x, y;
            BOOST_REQUIRE(EN_getcurvevalue(project.ph, index, point, &x, &y) == 0);
            curve.x.push_back(x);
            curve.y.push_back(y);
        }
        expected.push_back(curve);
    }

    project.close();

    std::string report = readWholeFile(CURVE_RPT);
    BOOST_REQUIRE(!report.empty());
    BOOST_CHECK(!reportMentionsLuaError(report));

    for (const ExpectedCurve &curve : expected)
    {
        std::string prefix = "curve." + curve.id + ".";

        double count;
        BOOST_REQUIRE(findLastPrintedValue(report, prefix + "count=", &count));
        BOOST_CHECK_EQUAL((int)count, curve.length);

        for (int point = 1; point <= curve.length; point++)
        {
            std::string at = prefix + std::to_string(point) + ".";
            double x, y;

            BOOST_REQUIRE(findLastPrintedValue(report, at + "x=", &x));
            BOOST_REQUIRE(findLastPrintedValue(report, at + "y=", &y));

            if (curve.x[point - 1] == 0.0) BOOST_CHECK_SMALL(x, 1e-6);
            else BOOST_CHECK_CLOSE(x, curve.x[point - 1], 0.01);

            if (curve.y[point - 1] == 0.0) BOOST_CHECK_SMALL(y, 1e-6);
            else BOOST_CHECK_CLOSE(y, curve.y[point - 1], 0.01);
        }
    }
}

BOOST_AUTO_TEST_CASE(script_cannot_read_an_unknown_curve)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, UNKNOWN_CURVE_INP,
        "function on_hydraulics_solved() local points = curve(\"not_a_curve\") end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE(project.open(UNKNOWN_CURVE_INP, UNKNOWN_CURVE_RPT) == 0);
    BOOST_REQUIRE_EQUAL(project.solveAndAdvanceOneHydraulicStep(), 313);
    project.close();

    BOOST_CHECK(readWholeFile(UNKNOWN_CURVE_RPT).find(
        "curve not found: not_a_curve") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(script_with_a_syntax_error_stops_the_project_opening)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, SYNTAX_ERROR_INP,
                                     "this is not ( valid lua\n"));

    ProjectUnderTest project;
    BOOST_CHECK_EQUAL(project.open(SYNTAX_ERROR_INP, SYNTAX_ERROR_RPT), 312);
    BOOST_CHECK_EQUAL(project.solveOneHydraulicStep(), 102);
    project.close();

    std::string report = readWholeFile(SYNTAX_ERROR_RPT);
    BOOST_CHECK(report.find("Lua script error while parsing")
                != std::string::npos);
    BOOST_CHECK(report.find("Error 312") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(syntax_error_is_reported_at_the_offending_line)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, SYNTAX_ERROR_LINE_INP,
        "function on_hydraulic_step()\n"           // 1
        "    local pressure = node(\"11\").pressure\n"
        "\n"                                  // 3
        "    if pressure > 10 then\n"
        "        node(\"11\" elevation = 5\n" // 5: missing ')' and ','
        "    end\n"
        "end\n"));

    ProjectUnderTest project;
    BOOST_CHECK_EQUAL(project.open(SYNTAX_ERROR_LINE_INP,
                                   SYNTAX_ERROR_LINE_RPT), 312);
    project.close();

    BOOST_CHECK(readWholeFile(SYNTAX_ERROR_LINE_RPT).find(":5:")
                != std::string::npos);
}

// A '[' opening a line means a new section everywhere else in the input
// file, but in a script it is ordinary Lua: a table key or a long string
BOOST_AUTO_TEST_CASE(script_lines_may_open_with_a_bracket)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, BRACKET_LINE_INP,
        "local monitored = {\n"
        "    [\"11\"] = true,\n"
        "}\n"
        "local j = 1\n"
        "local indexed = {\n"
        "    [j] = \"11\",\n"
        "}\n"
        "local banner = [[\n"
        "a long string, [PIPES] and all\n"
        "]]\n"
        "function on_hydraulic_step()\n"
        "    for id in pairs(monitored) do\n"
        "        print(\"keyed=\" .. node(id).pressure)\n"
        "    end\n"
        "    print(\"indexed=\" .. node(indexed[1]).pressure)\n"
        "    print(\"banner=\" .. #banner)\n"
        "end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE_EQUAL(project.open(BRACKET_LINE_INP, BRACKET_LINE_RPT), 0);
    BOOST_REQUIRE_EQUAL(project.solveAndAdvanceOneHydraulicStep(), 0);
    project.close();

    std::string report = readWholeFile(BRACKET_LINE_RPT);
    BOOST_CHECK(!reportMentionsLuaError(report));
    BOOST_CHECK(report.find("keyed=") != std::string::npos);
    BOOST_CHECK(report.find("indexed=") != std::string::npos);
    BOOST_CHECK(report.find("banner=") != std::string::npos);
}

// The keywords are what a script gives way to, so the section that
// follows one still has to be read
BOOST_AUTO_TEST_CASE(a_section_keyword_ends_the_script)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, SECTION_AFTER_INP,
        "function on_hydraulic_step() end\n"
        "\n"
        "[TIMES]\n"
        "Duration\t12:00\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE_EQUAL(project.open(SECTION_AFTER_INP, SECTION_AFTER_RPT), 0);

    long duration = 0;
    BOOST_CHECK_EQUAL(EN_gettimeparam(project.ph, EN_DURATION, &duration), 0);
    BOOST_CHECK_EQUAL(duration, 12 * 3600);
    project.close();

    BOOST_CHECK(!reportMentionsLuaError(readWholeFile(SECTION_AFTER_RPT)));
}

// A runtime error is fatal: the step it happened in fails with 313 and
// the run stops there rather than carrying on with a script that raised
BOOST_AUTO_TEST_CASE(a_failing_step_handler_stops_the_simulation)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, STEP_ERROR_INP,
        "solved = 0\n"
        "function on_hydraulic_step()\n"
        "    solved = solved + 1\n"
        "    if solved > 2 then error(\"stop here\") end\n"
        "end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE_EQUAL(project.open(STEP_ERROR_INP, STEP_ERROR_RPT), 0);

    int steps = 0;
    BOOST_CHECK_EQUAL(project.solveAllHydraulicSteps(&steps), 313);
    BOOST_CHECK_EQUAL(steps, 2);
    project.close();

    std::string report = readWholeFile(STEP_ERROR_RPT);
    BOOST_CHECK(report.find("Lua script error in on_hydraulic_step")
                != std::string::npos);
    BOOST_CHECK(report.find("stop here") != std::string::npos);
    BOOST_CHECK(report.find("Error 313") != std::string::npos);
}

BOOST_AUTO_TEST_CASE(a_failing_open_handler_fails_the_solver_init)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, OPEN_ERROR_INP,
        "function on_open() error(\"stop here\") end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE_EQUAL(project.open(OPEN_ERROR_INP, OPEN_ERROR_RPT), 0);
    BOOST_REQUIRE_EQUAL(EN_openH(project.ph), 0);
    BOOST_CHECK_EQUAL(EN_initH(project.ph, EN_NOSAVE), 313);
    project.close();

    std::string report = readWholeFile(OPEN_ERROR_RPT);
    BOOST_CHECK(report.find("Lua script error in on_open") != std::string::npos);
    BOOST_CHECK(report.find("Error 313") != std::string::npos);
}

// on_close runs when the analysis is already over, so there is nothing
// left to abort: the error is reported and the close still succeeds
BOOST_AUTO_TEST_CASE(a_failing_close_handler_is_only_reported)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, CLOSE_ERROR_INP,
        "function on_close() error(\"stop here\") end\n"));

    ProjectUnderTest project;
    BOOST_REQUIRE_EQUAL(project.open(CLOSE_ERROR_INP, CLOSE_ERROR_RPT), 0);
    BOOST_REQUIRE_EQUAL(EN_openH(project.ph), 0);
    BOOST_REQUIRE_EQUAL(EN_initH(project.ph, EN_NOSAVE), 0);
    BOOST_CHECK_EQUAL(EN_closeH(project.ph), 0);
    project.close();

    std::string report = readWholeFile(CLOSE_ERROR_RPT);
    BOOST_CHECK(report.find("Lua script error in on_close") != std::string::npos);
    BOOST_CHECK(report.find("stop here") != std::string::npos);
}

// Outside a script every '[' still heads a section, so a misspelled
// keyword is reported rather than read as data
BOOST_AUTO_TEST_CASE(unknown_section_outside_a_script_is_still_reported)
{
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, BAD_SECTION_INP,
        "function on_hydraulic_step() end\n",
        "[NOSUCHSECTION]\nsome junk\n\n"));

    ProjectUnderTest project;
    BOOST_CHECK_EQUAL(project.open(BAD_SECTION_INP, BAD_SECTION_RPT), 200);
    project.close();

    BOOST_CHECK(readWholeFile(BAD_SECTION_RPT).find("Error 299")
                != std::string::npos);
}

// EN_saveinpfile has to write the section back out, or a project that is
// opened and saved silently loses its script. The text is reproduced as it
// was read, so that the line numbers Lua reports do not shift either.
BOOST_AUTO_TEST_CASE(a_saved_project_keeps_its_script)
{
    // A comment carrying ';', a blank line and a long string opening with
    // '[': the three things a section written back as plain text could
    // mangle or be cut short by
    const char *script =
        "-- a comment with a ; semicolon and [brackets]\n"
        "\n"
        "local banner = [[\n"
        "a long string, [PIPES] and all\n"
        "]]\n"
        "function on_hydraulic_step()\n"
        "    print(\"saved script ran, banner=\" .. #banner)\n"
        "end\n";
    BOOST_REQUIRE(buildInpWithScript(BASE_INP, ROUND_TRIP_INP, script));

    ProjectUnderTest original;
    BOOST_REQUIRE_EQUAL(original.open(ROUND_TRIP_INP, ROUND_TRIP_RPT), 0);
    BOOST_REQUIRE_EQUAL(EN_saveinpfile(original.ph, ROUND_TRIP_SAVED_INP), 0);
    original.close();

    // The saved section holds exactly what was read
    std::string saved = readWholeFile(ROUND_TRIP_SAVED_INP);
    BOOST_REQUIRE(saved.find("[SCRIPT]") != std::string::npos);
    BOOST_CHECK(saved.find(script) != std::string::npos);

    // And the saved file still behaves like the original
    ProjectUnderTest reopened;
    BOOST_REQUIRE_EQUAL(reopened.open(ROUND_TRIP_SAVED_INP,
                                      ROUND_TRIP_SAVED_RPT), 0);
    BOOST_REQUIRE_EQUAL(reopened.solveOneHydraulicStep(), 0);
    reopened.close();

    // The banner's length is checked as well as its presence: it is the
    // 30 characters of the long string plus its closing newline, so a
    // section written back with lines dropped or added would not match
    std::string report = readWholeFile(ROUND_TRIP_SAVED_RPT);
    BOOST_CHECK(!reportMentionsLuaError(report));
    BOOST_CHECK(report.find("saved script ran, banner=31") != std::string::npos);
}

BOOST_AUTO_TEST_SUITE_END()
