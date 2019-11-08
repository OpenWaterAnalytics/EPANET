/** @file epanet2_enums.h
*/
/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2_enums.h
 Description:  enumerations of symbolic constants used by the API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/06/2019
 ******************************************************************************
*/


#ifndef EPANET2_ENUMS_H
#define EPANET2_ENUMS_H


// --- Define the EPANET toolkit constants

/// Size Limts
/**
Limits on the size of character arrays used to store ID names
and text messages.
*/
typedef enum {
  EN_MAXID   = 31,     //!< Max. # characters in ID name
  EN_MAXMSG  = 255     //!< Max. # characters in message text
} EN_SizeLimits;

/// Node properties
/**
These node properties are used with @ref EN_getnodevalue and @ref EN_setnodevalue.
Those marked as read only are computed values that can only be retrieved.
*/
typedef enum {
  EN_ELEVATION    = 0, //!< Elevation
  EN_BASEDEMAND   = 1, //!< Primary demand baseline value
  EN_PATTERN      = 2, //!< Primary demand time pattern index
  EN_EMITTER      = 3, //!< Emitter flow coefficient
  EN_INITQUAL     = 4, //!< Initial quality
  EN_SOURCEQUAL   = 5, //!< Quality source strength
  EN_SOURCEPAT    = 6, //!< Quality source pattern index
  EN_SOURCETYPE   = 7, //!< Quality source type (see @ref EN_SourceType)
  EN_TANKLEVEL    = 8, //!< Current computed tank water level (read only)
  EN_DEMAND       = 9, //!< Current computed demand (read only)
  EN_HEAD         = 10, //!< Current computed hydraulic head (read only)
  EN_PRESSURE     = 11, //!< Current computed pressure (read only)
  EN_QUALITY      = 12, //!< Current computed quality (read only)
  EN_SOURCEMASS   = 13, //!< Current computed quality source mass inflow (read only)
  EN_INITVOLUME   = 14, //!< Tank initial volume (read only)
  EN_MIXMODEL     = 15, //!< Tank mixing model (see @ref EN_MixingModel)
  EN_MIXZONEVOL   = 16, //!< Tank mixing zone volume (read only)
  EN_TANKDIAM     = 17, //!< Tank diameter
  EN_MINVOLUME    = 18, //!< Tank minimum volume
  EN_VOLCURVE     = 19, //!< Tank volume curve index
  EN_MINLEVEL     = 20, //!< Tank minimum level
  EN_MAXLEVEL     = 21, //!< Tank maximum level
  EN_MIXFRACTION  = 22, //!< Tank mixing fraction
  EN_TANK_KBULK   = 23, //!< Tank bulk decay coefficient
  EN_TANKVOLUME   = 24, //!< Current computed tank volume (read only)
  EN_MAXVOLUME    = 25, //!< Tank maximum volume (read only)
  EN_CANOVERFLOW  = 26, //!< Tank can overflow (= 1) or not (= 0)
  EN_DEMANDDEFICIT = 27 //!< Amount that full demand is reduced under PDA (read only)
} EN_NodeProperty;

/// Link properties
/**
These link properties are used with @ref EN_getlinkvalue and @ref EN_setlinkvalue.
Those marked as read only are computed values that can only be retrieved.
*/
typedef enum {
  EN_DIAMETER     = 0,  //!< Pipe/valve diameter
  EN_LENGTH       = 1,  //!< Pipe length
  EN_ROUGHNESS    = 2,  //!< Pipe roughness coefficient
  EN_MINORLOSS    = 3,  //!< Pipe/valve minor loss coefficient
  EN_INITSTATUS   = 4,  //!< Initial status (see @ref EN_LinkStatusType)
  EN_INITSETTING  = 5,  //!< Initial pump speed or valve setting
  EN_KBULK        = 6,  //!< Bulk chemical reaction coefficient
  EN_KWALL        = 7,  //!< Pipe wall chemical reaction coefficient
  EN_FLOW         = 8,  //!< Current computed flow rate (read only)
  EN_VELOCITY     = 9,  //!< Current computed flow velocity (read only)
  EN_HEADLOSS     = 10, //!< Current computed head loss (read only)
  EN_STATUS       = 11, //!< Current link status (see @ref EN_LinkStatusType)
  EN_SETTING      = 12, //!< Current link setting
  EN_ENERGY       = 13, //!< Current computed pump energy usage (read only)
  EN_LINKQUAL     = 14, //!< Current computed link quality (read only)
  EN_LINKPATTERN  = 15, //!< Pump speed time pattern index
  EN_PUMP_STATE   = 16, //!< Current computed pump state (read only) (see @ref EN_PumpStateType)
  EN_PUMP_EFFIC   = 17, //!< Current computed pump efficiency (read only)
  EN_PUMP_POWER   = 18, //!< Pump constant power rating
  EN_PUMP_HCURVE  = 19, //!< Pump head v. flow curve index
  EN_PUMP_ECURVE  = 20, //!< Pump efficiency v. flow curve index
  EN_PUMP_ECOST   = 21, //!< Pump average energy price
  EN_PUMP_EPAT    = 22  //!< Pump energy price time pattern index
} EN_LinkProperty;

/// Time parameters
/**
These time-related options are used with @ref EN_gettimeparam and@ref EN_settimeparam.
All times are expressed in seconds The parameters marked as read only are
computed values that can only be retrieved.
*/
typedef enum {
  EN_DURATION     = 0,  //!< Total simulation duration
  EN_HYDSTEP      = 1,  //!< Hydraulic time step
  EN_QUALSTEP     = 2,  //!< Water quality time step
  EN_PATTERNSTEP  = 3,  //!< Time pattern period
  EN_PATTERNSTART = 4,  //!< Time when time patterns begin
  EN_REPORTSTEP   = 5,  //!< Reporting time step
  EN_REPORTSTART  = 6,  //!< Time when reporting starts
  EN_RULESTEP     = 7,  //!< Rule-based control evaluation time step
  EN_STATISTIC    = 8,  //!< Reporting statistic code (see @ref EN_StatisticType)
  EN_PERIODS      = 9,  //!< Number of reporting time periods (read only)
  EN_STARTTIME    = 10, //!< Simulation starting time of day
  EN_HTIME        = 11, //!< Elapsed time of current hydraulic solution (read only)
  EN_QTIME        = 12, //!< Elapsed time of current quality solution (read only)
  EN_HALTFLAG     = 13, //!< Flag indicating if the simulation was halted (read only)
  EN_NEXTEVENT    = 14, //!< Shortest time until a tank becomes empty or full (read only)
  EN_NEXTEVENTTANK = 15  //!< Index of tank with shortest time to become empty or full (read only)
} EN_TimeParameter;

/// Analysis convergence statistics
/**
These statistics report the convergence criteria for the most current hydraulic analysis
and the cumulative water quality mass balance error at the current simulation time. They
can be retrieved with @ref EN_getstatistic.
*/
typedef enum {
  EN_ITERATIONS      = 0, //!< Number of hydraulic iterations taken
  EN_RELATIVEERROR   = 1, //!< Sum of link flow changes / sum of link flows
  EN_MAXHEADERROR    = 2, //!< Largest head loss error for links
  EN_MAXFLOWCHANGE   = 3, //!< Largest flow change in links
  EN_MASSBALANCE     = 4, //!< Cumulative water quality mass balance ratio
  EN_DEFICIENTNODES  = 5, //!< Number of pressure deficient nodes
  EN_DEMANDREDUCTION = 6  //!< % demand reduction at pressure deficient nodes
} EN_AnalysisStatistic;

/// Types of network objects
/**
The types of objects that comprise a network model.
*/
typedef enum {
    EN_NODE    = 0,     //!< Nodes
    EN_LINK    = 1,     //!< Links
    EN_TIMEPAT = 2,     //!< Time patterns
    EN_CURVE   = 3,     //!< Data curves
    EN_CONTROL = 4,     //!< Simple controls
    EN_RULE    = 5      //!< Control rules
} EN_ObjectType;

/// Types of objects to count
/**
These options tell @ref EN_getcount which type of object to count.
*/
typedef enum {
  EN_NODECOUNT    = 0,  //!< Number of nodes (junctions + tanks + reservoirs)
  EN_TANKCOUNT    = 1,  //!< Number of tanks and reservoirs
  EN_LINKCOUNT    = 2,  //!< Number of links (pipes + pumps + valves)
  EN_PATCOUNT     = 3,  //!< Number of time patterns
  EN_CURVECOUNT   = 4,  //!< Number of data curves
  EN_CONTROLCOUNT = 5,  //!< Number of simple controls
  EN_RULECOUNT    = 6   //!< Number of rule-based controls
} EN_CountType;

/// Node Types
/**
These are the different types of nodes that can be returned by calling @ref EN_getnodetype.
*/
typedef enum {
  EN_JUNCTION    = 0,   //!< Junction node
  EN_RESERVOIR   = 1,   //!< Reservoir node
  EN_TANK        = 2    //!< Storage tank node
} EN_NodeType;

/// Link types
/**
These are the different types of links that can be returned by calling @ref EN_getlinktype.
*/
typedef enum {
  EN_CVPIPE       = 0,  //!< Pipe with check valve
  EN_PIPE         = 1,  //!< Pipe
  EN_PUMP         = 2,  //!< Pump
  EN_PRV          = 3,  //!< Pressure reducing valve
  EN_PSV          = 4,  //!< Pressure sustaining valve
  EN_PBV          = 5,  //!< Pressure breaker valve
  EN_FCV          = 6,  //!< Flow control valve
  EN_TCV          = 7,  //!< Throttle control valve
  EN_GPV          = 8   //!< General purpose valve
} EN_LinkType;

/// Link status
/**
One of these values is returned when @ref EN_getlinkvalue is used to retrieve a link's
initial status ( \b EN_INITSTATUS ) or its current status ( \b EN_STATUS ). These options are
also used with @ref EN_setlinkvalue to set values for these same properties.
*/
typedef enum {
  EN_CLOSED       = 0,
  EN_OPEN         = 1
} EN_LinkStatusType;

/// Pump states
/**
One of these codes is returned when @ref EN_getlinkvalue is used to retrieve a pump's
current operating state ( \b EN_PUMP_STATE ). \b EN_PUMP_XHEAD indicates that the pump has been
shut down because it is being asked to deliver more than its shutoff head. \b EN_PUMP_XFLOW
indicates that the pump is being asked to deliver more than its maximum flow.
*/
typedef enum {
  EN_PUMP_XHEAD   = 0,  //!< Pump closed - cannot supply head
  EN_PUMP_CLOSED  = 2,  //!< Pump closed
  EN_PUMP_OPEN    = 3,  //!< Pump open
  EN_PUMP_XFLOW   = 5   //!< Pump open - cannot supply flow
} EN_PumpStateType;

/// Types of water quality analyses
/**
These are the different types of water quality analyses that EPANET can run. They
are used with @ref EN_getqualinfo, @ref EN_getqualtype, and @ref EN_setqualtype.
*/
typedef enum {
  EN_NONE        = 0,   //!< No quality analysis
  EN_CHEM        = 1,   //!< Chemical fate and transport
  EN_AGE         = 2,   //!< Water age analysis
  EN_TRACE       = 3    //!< Source tracing analysis
} EN_QualityType;

/// Water quality source types
/**
These are the different types of external water quality sources that can be assigned
to a node's \b EN_SOURCETYPE property as used by @ref EN_getnodevalue and @ref EN_setnodevalue.
*/
typedef enum {
  EN_CONCEN      = 0,   //!< Sets the concentration of external inflow entering a node
  EN_MASS        = 1,   //!< Injects a given mass/minute into a node
  EN_SETPOINT    = 2,   //!< Sets the concentration leaving a node to a given value
  EN_FLOWPACED   = 3    //!< Adds a given value to the concentration leaving a node
} EN_SourceType;

/// Head loss formulas
/**
The available choices for the \b EN_HEADLOSSFORM option in @ref EN_getoption and
@ref EN_setoption. They are also used for the head loss type argument in @ref EN_init.
Each head loss formula uses a different type of roughness coefficient ( \b EN_ROUGHNESS )
that can be set with @ref EN_setlinkvalue.
*/
typedef enum {
  EN_HW          = 0,   //!< Hazen-Williams
  EN_DW          = 1,   //!< Darcy-Weisbach
  EN_CM          = 2    //!< Chezy-Manning
} EN_HeadLossType;

/// Flow units
/**
These choices for flow units are used with @ref EN_getflowunits and @ref EN_setflowunits.
They are also used for the flow units type argument in @ref EN_init. If flow units are
expressed in US Customary units ( \b EN_CFS through \b EN_AFD ) then all other quantities are
in US Customary units. Otherwise they are in metric units.
*/
typedef enum {
  EN_CFS         = 0,   //!< Cubic feet per second
  EN_GPM         = 1,   //!< Gallons per minute
  EN_MGD         = 2,   //!< Million gallons per day
  EN_IMGD        = 3,   //!< Imperial million gallons per day
  EN_AFD         = 4,   //!< Acre-feet per day
  EN_LPS         = 5,   //!< Liters per second
  EN_LPM         = 6,   //!< Liters per minute
  EN_MLD         = 7,   //!< Million liters per day
  EN_CMH         = 8,   //!< Cubic meters per hour
  EN_CMD         = 9    //!< Cubic meters per day
} EN_FlowUnits;

/// Demand models
/**
These choices for modeling consumer demands are used with @ref EN_getdemandmodel
and @ref EN_setdemandmodel.

A demand driven analysis requires that a junction's full demand be supplied
in each time period independent of how much pressure is available. A pressure
driven analysis makes demand be a power function of pressure, up to the point
where the full demand is met.
*/
typedef enum {
  EN_DDA         = 0,   //!< Demand driven analysis
  EN_PDA         = 1    //!< Pressure driven analysis
} EN_DemandModel;

/// Simulation options
/**
These constants identify the hydraulic and water quality simulation options
that are applied on a network-wide basis. They are accessed using the
@ref EN_getoption and @ref EN_setoption functions.
*/
typedef enum {
  EN_TRIALS         = 0,  //!< Maximum trials allowed for hydraulic convergence
  EN_ACCURACY       = 1,  //!< Total normalized flow change for hydraulic convergence
  EN_TOLERANCE      = 2,  //!< Water quality tolerance
  EN_EMITEXPON      = 3,  //!< Exponent in emitter discharge formula
  EN_DEMANDMULT     = 4,  //!< Global demand multiplier
  EN_HEADERROR      = 5,  //!< Maximum head loss error for hydraulic convergence
  EN_FLOWCHANGE     = 6,  //!< Maximum flow change for hydraulic convergence
  EN_HEADLOSSFORM   = 7,  //!< Head loss formula (see @ref EN_HeadLossType)
  EN_GLOBALEFFIC    = 8,  //!< Global pump efficiency (percent)
  EN_GLOBALPRICE    = 9,  //!< Global energy price per KWH
  EN_GLOBALPATTERN  = 10, //!< Index of a global energy price pattern
  EN_DEMANDCHARGE   = 11,  //!< Energy charge per max. KW usage
  EN_SP_GRAVITY     = 12, //!< Specific gravity
  EN_SP_VISCOS      = 13, //!< Specific viscosity (relative to water at 20 deg C)
  EN_UNBALANCED     = 14, //!< Extra trials allowed if hydraulics don't converge
  EN_CHECKFREQ      = 15, //!< Frequency of hydraulic status checks
  EN_MAXCHECK       = 16, //!< Maximum trials for status checking
  EN_DAMPLIMIT      = 17, //!< Accuracy level where solution damping begins
  EN_SP_DIFFUS      = 18, //!< Specific diffusivity (relative to chlorine at 20 deg C)
  EN_BULKORDER      = 19, //!< Bulk water reaction order for pipes
  EN_WALLORDER      = 20, //!< Wall reaction order for pipes (either 0 or 1)
  EN_TANKORDER      = 21, //!< Bulk water reaction order for tanks
  EN_CONCENLIMIT    = 22  //!< Limiting concentration for growth reactions
} EN_Option;

/// Simple control types
/**
These are the different types of simple (single statement) controls that can be applied
to network links. They are used as an argument to @ref EN_addcontrol,@ref EN_getcontrol,
and @ref EN_setcontrol.
*/
typedef enum {
  EN_LOWLEVEL    = 0,   //!< Act when pressure or tank level drops below a setpoint
  EN_HILEVEL     = 1,   //!< Act when pressure or tank level rises above a setpoint
  EN_TIMER       = 2,   //!< Act at a prescribed elapsed amount of time
  EN_TIMEOFDAY   = 3    //!< Act at a particular time of day
} EN_ControlType;

/// Reporting statistic choices
/**
These options determine what kind of statistical post-processing should be done on
the time series of simulation results generated before they are reported using
@ref EN_report. An option can be chosen by using \b STATISTIC _option_ as the argument
to @ref EN_setreport.
*/
typedef enum {
  EN_SERIES      = 0,   //!< Report all time series points
  EN_AVERAGE     = 1,   //!< Report average value over simulation period
  EN_MINIMUM     = 2,   //!< Report minimum value over simulation period
  EN_MAXIMUM     = 3,   //!< Report maximum value over simulation period
  EN_RANGE       = 4    //!< Report maximum - minimum over simulation period
} EN_StatisticType;

/// Tank mixing models
/**
These are the different types of models that describe water quality mixing in storage tanks.
The choice of model is accessed with the \b EN_MIXMODEL property of a Tank node using
@ref EN_getnodevalue and @ref EN_setnodevalue.
*/
typedef enum {
  EN_MIX1        = 0,   //!< Complete mix model
  EN_MIX2        = 1,   //!< 2-compartment model
  EN_FIFO        = 2,   //!< First in, first out model
  EN_LIFO        = 3    //!< Last in, first out model
} EN_MixingModel;

/// Hydraulic initialization options
/**
These options are used to initialize a new hydraulic analysis when @ref EN_initH is called.
*/
typedef enum {
  EN_NOSAVE        = 0,  //!< Don't save hydraulics; don't re-initialize flows
  EN_SAVE          = 1,  //!< Save hydraulics to file, don't re-initialize flows
  EN_INITFLOW      = 10, //!< Don't save hydraulics; re-initialize flows
  EN_SAVE_AND_INIT = 11  //!< Save hydraulics; re-initialize flows
} EN_InitHydOption;

/// Types of pump curves
/**
@ref EN_getpumptype returns one of these values when it is called.
*/
typedef enum {
  EN_CONST_HP    = 0,   //!< Constant horsepower
  EN_POWER_FUNC  = 1,   //!< Power function
  EN_CUSTOM      = 2,   //!< User-defined custom curve
  EN_NOCURVE     = 3    //!< No curve
} EN_PumpType;

/// Types of data curves
/**
These are the different types of physical relationships that a data curve can
represent as returned by calling @ref EN_getcurvetype.
*/
typedef enum {
  EN_VOLUME_CURVE  = 0,   //!< Tank volume v. depth curve
  EN_PUMP_CURVE    = 1,   //!< Pump head v. flow curve
  EN_EFFIC_CURVE   = 2,   //!< Pump efficiency v. flow curve
  EN_HLOSS_CURVE   = 3,   //!< Valve head loss v. flow curve
  EN_GENERIC_CURVE = 4    //!< Generic curve
} EN_CurveType;

/// Deletion action codes
/**
These codes are used in @ref EN_deletenode and @ref EN_deletelink to indicate what action
should be taken if the node or link being deleted appears in any simple or rule-based
controls or if a deleted node has any links connected to it.
*/
typedef enum {
  EN_UNCONDITIONAL = 0, //!< Delete all controls and connecing links
  EN_CONDITIONAL   = 1  //!< Cancel object deletion if it appears in controls or has connecting links
} EN_ActionCodeType;

/// Status reporting levels
/**
These choices specify the level of status reporting written to a project's report
file during a hydraulic analysis. The level is set using the @ref EN_setstatusreport function.
*/
typedef enum {
  EN_NO_REPORT = 0,     //!< No status reporting
  EN_NORMAL_REPORT = 1, //!< Normal level of status reporting
  EN_FULL_REPORT = 2    //!< Full level of status reporting
} EN_StatusReport;

/// Network objects used in rule-based controls
typedef enum {
  EN_R_NODE      = 6,   //!< Clause refers to a node
  EN_R_LINK      = 7,   //!< Clause refers to a link
  EN_R_SYSTEM    = 8    //!< Clause refers to a system parameter (e.g., time)
} EN_RuleObject;

/// Object variables used in rule-based controls
typedef enum {
  EN_R_DEMAND    = 0,   //!< Nodal demand
  EN_R_HEAD      = 1,   //!< Nodal hydraulic head
  EN_R_GRADE     = 2,   //!< Nodal hydraulic grade
  EN_R_LEVEL     = 3,   //!< Tank water level
  EN_R_PRESSURE  = 4,   //!< Nodal pressure
  EN_R_FLOW      = 5,   //!< Link flow rate
  EN_R_STATUS    = 6,   //!< Link status
  EN_R_SETTING   = 7,   //!< Link setting
  EN_R_POWER     = 8,   //!< Pump power output
  EN_R_TIME      = 9,   //!< Elapsed simulation time
  EN_R_CLOCKTIME = 10,  //!< Time of day
  EN_R_FILLTIME  = 11,  //!< Time to fill a tank
  EN_R_DRAINTIME = 12   //!< Time to drain a tank
} EN_RuleVariable;

/// Comparison operators used in rule-based controls
typedef enum {
  EN_R_EQ        = 0,   //!< Equal to
  EN_R_NE        = 1,   //!< Not equal
  EN_R_LE        = 2,   //!< Less than or equal to
  EN_R_GE        = 3,   //!< Greater than or equal to
  EN_R_LT        = 4,   //!< Less than
  EN_R_GT        = 5,   //!< Greater than
  EN_R_IS        = 6,   //!< Is equal to
  EN_R_NOT       = 7,   //!< Is not equal to
  EN_R_BELOW     = 8,   //!< Is below
  EN_R_ABOVE     = 9    //!< Is above
} EN_RuleOperator;

/// Link status codes used in rule-based controls
typedef enum {
  EN_R_IS_OPEN   = 1,   //!< Link is open
  EN_R_IS_CLOSED = 2,   //!< Link is closed
  EN_R_IS_ACTIVE = 3    //!< Control valve is active
} EN_RuleStatus;

#define EN_MISSING -1.E10  //!< Missing value indicator

#endif //EPANET2_ENUMS_H
