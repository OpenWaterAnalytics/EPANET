/** @file epanet2_enums.h
 */
/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2_enums.h
 Description:  enumerations of symbolic constants used by the API
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 01/08/2019
 ******************************************************************************
*/


#ifndef EPANET2_ENUMS_H
#define EPANET2_ENUMS_H


// --- Define the EPANET toolkit constants

#define  EN_MAXID  31  //!< Max. # characters in ID name
#define  EN_MAXMSG 255 //!< Max. # characters in message text

/// Node property codes
typedef enum {
  EN_ELEVATION    = 0, //!< Elevation
  EN_BASEDEMAND   = 1, //!< Junction baseline demand, from last demand category
  EN_PATTERN      = 2, //!< Junction baseline demand pattern
  EN_EMITTER      = 3, //!< Junction emitter coefficient
  EN_INITQUAL     = 4, //!< Initial quality
  EN_SOURCEQUAL   = 5, //!< Quality source strength
  EN_SOURCEPAT    = 6, //!< Quality source pattern
  EN_SOURCETYPE   = 7, //!< Qualiy source type
  EN_TANKLEVEL    = 8, //!< Current computed tank water level
  EN_DEMAND       = 9, //!< Current computed demand
  EN_HEAD         = 10, //!< Current computed hydraulic head
  EN_PRESSURE     = 11, //!< Current computed pressure
  EN_QUALITY      = 12, //!< Current computed quality
  EN_SOURCEMASS   = 13, //!< Current computed quality source mass inflow
  EN_INITVOLUME   = 14, //!< Tank initial volume
  EN_MIXMODEL     = 15, //!< Tank mixing model
  EN_MIXZONEVOL   = 16, //!< Tank mixing zone volume
  EN_TANKDIAM     = 17, //!< Tank diameter
  EN_MINVOLUME    = 18, //!< Tank minimum volume
  EN_VOLCURVE     = 19, //!< Tank volume curve
  EN_MINLEVEL     = 20, //!< Tank minimum level
  EN_MAXLEVEL     = 21, //!< Tank maximum level
  EN_MIXFRACTION  = 22, //!< Tank mixing fraction
  EN_TANK_KBULK   = 23, //!< Tank bulk decay coefficient
  EN_TANKVOLUME   = 24, //!< Current computed tank volume
  EN_MAXVOLUME    = 25  //!< Tank maximum volume
} EN_NodeProperty;

/// Link property codes
typedef enum {
  EN_DIAMETER     = 0,  //!< Pipe/valve diameter
  EN_LENGTH       = 1,  //!< Pipe length
  EN_ROUGHNESS    = 2,  //!< Pipe roughness coefficient
  EN_MINORLOSS    = 3,  //!< Pipe/valve minor loss coefficient
  EN_INITSTATUS   = 4,  //!< Initial status (e.g., OPEN/CLOSED)
  EN_INITSETTING  = 5,  //!< Initial pump speed or valve setting
  EN_KBULK        = 6,  //!< Bulk chemical reaction coefficient
  EN_KWALL        = 7,  //!< Pipe wall chemical reaction coefficient
  EN_FLOW         = 8,  //!< Current computed flow rate
  EN_VELOCITY     = 9,  //!< Current computed flow velocity
  EN_HEADLOSS     = 10, //!< Current computed head loss
  EN_STATUS       = 11, //!< Current link status
  EN_SETTING      = 12, //!< Current link setting
  EN_ENERGY       = 13, //!< Current computed pump energy usage
  EN_LINKQUAL     = 14, //!< Current computed link quality
  EN_LINKPATTERN  = 15, //!< Pump speed time pattern

  EN_PUMP_STATE   = 16, //!< Current computed pump state
  EN_PUMP_EFFIC   = 17, //!< Current computed pump efficiency
  EN_PUMP_POWER   = 18, //!< Pump constant power rating
  EN_PUMP_HCURVE  = 19, //!< Pump head v. flow curve
  EN_PUMP_ECURVE  = 20, //!< Pump efficiency v. flow curve
  EN_PUMP_ECOST   = 21, //!< Pump average energy price
  EN_PUMP_EPAT    = 22  //!< Pump energy price time pattern
} EN_LinkProperty;

/// Time parameter codes (all in seconds)
typedef enum {
  EN_DURATION     = 0,  //!< Total simulation duration
  EN_HYDSTEP      = 1,  //!< Hydraulic time step
  EN_QUALSTEP     = 2,  //!< Water quality time step
  EN_PATTERNSTEP  = 3,  //!< Time pattern period
  EN_PATTERNSTART = 4,  //!< Time when time patterns begin
  EN_REPORTSTEP   = 5,  //!< Reporting time step
  EN_REPORTSTART  = 6,  //!< Time when reporting starts
  EN_RULESTEP     = 7,  //!< Rule evaluation time step
  EN_STATISTIC    = 8,  //!< Reporting statistic code
  EN_PERIODS      = 9,  //!< Number of reporting time periods
  EN_STARTTIME    = 10, //!< Simulation starting time of day
  EN_HTIME        = 11, //!< Elapsed time of current hydraulic solution
  EN_QTIME        = 12, //!< Elapsed time of current quality solution
  EN_HALTFLAG     = 13, //!< Flag indicating if the simulation was halted
  EN_NEXTEVENT    = 14, //!< Next time until a tank becomes empty or full
  EN_NEXTEVENTIDX = 15  //!< Index of next tank that becomes empty or full
} EN_TimeProperty;

/// Statistics for the most current hydraulic/quality analysis made
typedef enum {
  EN_ITERATIONS    = 0, //!< Number of hydraulic iterations taken
  EN_RELATIVEERROR = 1, //!< Sum of link flow changes / sum of link flows
  EN_MAXHEADERROR  = 2, //!< Largest head loss error for links
  EN_MAXFLOWCHANGE = 3, //!< Largest flow change in links
  EN_MASSBALANCE   = 4  //!< Cumulative water quality mass balance ratio
} EN_AnalysisStatistic;

/// Object count codes
typedef enum {
  EN_NODECOUNT    = 0,  //!< Number of nodes (junctions + tanks + reservoirs)
  EN_TANKCOUNT    = 1,  //!< Number of tanks and reservoirs
  EN_LINKCOUNT    = 2,  //!< Number of links (pipes + pumps + valves)
  EN_PATCOUNT     = 3,  //!< Number of time patterns
  EN_CURVECOUNT   = 4,  //!< Number of data curves
  EN_CONTROLCOUNT = 5,  //!< Number of simple controls
  EN_RULECOUNT	  = 6   //!< Number of rule-based controls
} EN_CountType;

/// Node type codes
typedef enum {
  EN_JUNCTION    = 0,   //!< Junction node
  EN_RESERVOIR   = 1,   //!< Reservoir node
  EN_TANK        = 2    //!< Storage tank node
} EN_NodeType;

/// Link type codes
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

/// Water quality analysis types
typedef enum {
  EN_NONE        = 0,   //!< No quality analysis
  EN_CHEM        = 1,   //!< Chemical fate and transport
  EN_AGE         = 2,   //!< Water age analysis
  EN_TRACE       = 3    //!< Source tracing analysis
} EN_QualityType;

/// Water quality source types
typedef enum {
  EN_CONCEN      = 0,   //!< Concentration inflow source
  EN_MASS        = 1,   //!< Mass inflow source
  EN_SETPOINT    = 2,   //!< Concentration setpoint source
  EN_FLOWPACED   = 3    //!< Concentration flow paced source
} EN_SourceType;

/// Head loss formulas
typedef enum {
  EN_HW          = 0,   //!< Hazen-Williams
  EN_DW          = 1,   //!< Darcy-Weisbach
  EN_CM          = 2    //!< Chezy-Manning
} EN_HeadLossType;

/// Flow units types
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

/// Demand model types
typedef enum {
  EN_DDA         = 0,   //!< Demand driven analysis
  EN_PDA         = 1    //!< Pressure driven analysis
} EN_DemandModel;

/// Simulation option codes
typedef enum {
  EN_TRIALS         = 0,  //!< Maximum hydraulic trials allowed
  EN_ACCURACY       = 1,  //!< Hydraulic convergence accuracy
  EN_TOLERANCE      = 2,  //!< Water quality tolerance
  EN_EMITEXPON      = 3,  //!< Exponent for emitter head loss formula
  EN_DEMANDMULT     = 4,  //!< Global demand multiplier
  EN_HEADERROR      = 5,  //!< Maximum allowable head loss error
  EN_FLOWCHANGE     = 6,  //!< Maximum allowable flow change
  EN_DEFDEMANDPAT   = 7,  //!< Default demand time pattern
  EN_HEADLOSSFORM   = 8,  //!< Head loss formula
  EN_GLOBALEFFIC    = 9,  //!< Global pump efficiency
  EN_GLOBALPRICE    = 10, //!< Global energy price per KWH
  EN_GLOBALPATTERN  = 11, //!< Global energy price pattern
  EN_DEMANDCHARGE   = 12  //!< Energy charge per max. KW usage
} EN_Option;

/// Simple control types
typedef enum {
  EN_LOWLEVEL    = 0,   //!< Act when level drops below a setpoint
  EN_HILEVEL     = 1,   //!< Act when level rises above a setpoint
  EN_TIMER       = 2,   //!< Act at a prescribed elapsed amount of time
  EN_TIMEOFDAY   = 3    //!< Act at a particular time of day
} EN_ControlType;

/// Reporting statistic types
typedef enum {
  EN_AVERAGE     = 1,   //!< Report average value over simulation period
  EN_MINIMUM     = 2,   //!< Report minimum value over simulation period
  EN_MAXIMUM     = 3,   //!< Report maximum value over simulation period
  EN_RANGE       = 4    //!< Report maximum - minimum over simulation period
} EN_StatisticType;

/// Tank mixing models
typedef enum {
  EN_MIX1        = 0,   //!< Complete mix model
  EN_MIX2        = 1,   //!< 2-compartment model
  EN_FIFO        = 2,   //!< First in, first out model
  EN_LIFO        = 3    //!< Last in, first out model
} EN_MixingModel;

/// Hydraulic initialization options
typedef enum {
  EN_NOSAVE        = 0,  //!< Don't save hydraulics; don't re-initialize flows
  EN_SAVE          = 1,  //!< Save hydraulics to file, don't re-initialize flows
  EN_INITFLOW      = 10, //!< Don't save hydraulics; re-initialize flows
  EN_SAVE_AND_INIT = 11  //!< Save hydraulics; re-initialize flows
} EN_SaveOption;

/// Pump curve types
typedef enum {
  EN_CONST_HP    = 0,   //!< Constant horsepower
  EN_POWER_FUNC  = 1,   //!< Power function
  EN_CUSTOM      = 2,   //!< User-defined custom curve
  EN_NOCURVE     = 3    //!< No curve
} EN_PumpType;

/// Data curve types
typedef enum {
  EN_VOLUME_CURVE  = 0,   //!< Tank volume curve
  EN_PUMP_CURVE    = 1,   //!< Pump head curve
  EN_EFFIC_CURVE   = 2,   //!< Pump efficiency curve
  EN_HLOSS_CURVE   = 3,   //!< Valve head loss curve
  EN_GENERIC_CURVE = 4    //!< Generic curve
} EN_CurveType;

/// Deletion action codes
typedef enum {
  EN_UNCONDITIONAL = 0, //!< Delete all controls that contain object
  EN_CONDITIONAL   = 1  //!< Cancel object deletion if contained in controls
} EN_ActionCodeType;

/// Options for reporting on the status of the hydraulic solver at each time period
typedef enum {
  EN_NO_REPORT = 0,     //!< No status reporting
  EN_NORMAL_REPORT = 1, //!< Normal level of status reporting
  EN_FULL_REPORT = 2    //!< Full level of status reporting
} EN_StatusReport;

/// Codes for objects referred to in the clauses of rule-based controls
typedef enum {
  EN_R_NODE      = 6,   //!< Clause refers to a node
  EN_R_LINK      = 7,   //!< Clause refers to a link
  EN_R_SYSTEM    = 8    //!< Clause refers to a system parameter (e.g., time)
} EN_RuleObject;

/// Codes for variables used in the clauses of rule-based controls
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

/// Comparison operators used in the premises of rule-based controls
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

/// Status codes used in the clauses of rule-based controls
typedef enum {
  EN_R_IS_OPEN   = 1,   //!< Link is open
  EN_R_IS_CLOSED = 2,   //!< Link is closed
  EN_R_IS_ACTIVE = 3    //!< Control valve is active
} EN_RuleStatus;


#endif //EPANET2_ENUMS_H
