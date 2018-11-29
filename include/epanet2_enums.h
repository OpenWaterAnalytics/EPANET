/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2_enums.h
 Description:  enums shared between API versions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/29/2018
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
  EN_TANKLEVEL    = 8, //!< Tank water level
  EN_DEMAND       = 9, //!< Current simulated demand
  EN_HEAD         = 10, //!< Current hydraulic head
  EN_PRESSURE     = 11, //!< Current pressure
  EN_QUALITY      = 12, //!< Current quality
  EN_SOURCEMASS   = 13, //!< Current source mass inflow
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
  EN_TANKVOLUME   = 24, //!< Tank current volume
  EN_MAXVOLUME    = 25  //!< Tank maximum volume
} EN_NodeProperty;

/// Link property codes
typedef enum {
  EN_DIAMETER     = 0,  //!< Pipe/valve diameter
  EN_LENGTH       = 1,  //!> Pipe length
  EN_ROUGHNESS    = 2,  //!> Pipe roughness coefficient
  EN_MINORLOSS    = 3,  //!> Pipe/valve minor loss coefficient
  EN_INITSTATUS   = 4,  //!> Initial status (e.g., OPEN/CLOSED)
  EN_INITSETTING  = 5,  //!> Initial pump speed or valve setting
  EN_KBULK        = 6,  //!> Bulk chemical reaction coefficient
  EN_KWALL        = 7,  //!> Pipe wall chemical reaction coefficient
  EN_FLOW         = 8,  //!> Current link flow rate
  EN_VELOCITY     = 9,  //!> Current link flow velocity
  EN_HEADLOSS     = 10, //!> Current head loss across link
  EN_STATUS       = 11, //!> Current link status
  EN_SETTING      = 12, //!> Current link setting
  EN_ENERGY       = 13, //!> Current pump energy usage
  EN_LINKQUAL     = 14, //!> Current link quality
  EN_LINKPATTERN  = 15, //!> Pump speed time pattern
  EN_EFFICIENCY   = 16, //!> Current pump efficiency
  EN_HEADCURVE    = 17, //!> Pump head v. flow curve
  EN_EFFICIENCYCURVE = 18, //!> Pump efficiency v. flow curve
  EN_PRICEPATTERN = 19,    //!> Pump energy price time pattern
  EN_STATE        = 20,    //!> Current pump status
  EN_CONST_POWER  = 21,    //!> Horsepower of constant horsepower pump
  EN_SPEED        = 22     //!> Current pump speed setting
} EN_LinkProperty;

/// Time parameter codes
typedef enum {
  EN_DURATION     = 0,  //!> Total simulation duration
  EN_HYDSTEP      = 1,  //!> Hydraulic time step
  EN_QUALSTEP     = 2,  //!> Water quality time step
  EN_PATTERNSTEP  = 3,  //!> Time pattern period
  EN_PATTERNSTART = 4,  //!> Time when time patterns begin
  EN_REPORTSTEP   = 5,  //!> Reporting time step
  EN_REPORTSTART  = 6,  //!> Time when reporting starts
  EN_RULESTEP     = 7,  //!> Rule evaluation time step
  EN_STATISTIC    = 8,  //!> Reporting statistic code
  EN_PERIODS      = 9,  //!> Number of reporting time periods
  EN_STARTTIME    = 10, //!> Simulation starting time of day
  EN_HTIME        = 11, //!> Elapsed time of current hydraulic solution
  EN_QTIME        = 12, //!> Elapsed time of current quality solution
  EN_HALTFLAG     = 13, //!> Flag indicating if simulation halted
  EN_NEXTEVENT    = 14, //!> Next time until a tank becomes empty or full
  EN_NEXTEVENTIDX = 15  //!> Index of next tank that becomes empty or full
} EN_TimeProperty;

/// Analysis statistic codes
typedef enum {
  EN_ITERATIONS    = 0, //!< Number of hydraulic iterations
  EN_RELATIVEERROR = 1, //!< Sum of all flow changes / total flow
  EN_MAXHEADERROR  = 2, //!< Largest head loss error for links
  EN_MAXFLOWCHANGE = 3, //!< Largest flow change in links
  EN_MASSBALANCE   = 4  //!< Water quality mass balance ratio
} EN_AnalysisStatistic;

/// Object count codes
typedef enum {
  EN_NODECOUNT    = 0,  //!< Number of nodes (Juntions + Tanks + Reservoirs)
  EN_TANKCOUNT    = 1,  //!< Number of tanks and Reservoirs
  EN_LINKCOUNT    = 2,  //!< Number of links (Pipes + Pumps + Valves)
  EN_PATCOUNT     = 3,  //!< Number of time patterns
  EN_CURVECOUNT   = 4,  //!< Number of curves
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
  EN_CFS         = 0,
  EN_GPM         = 1,
  EN_MGD         = 2,
  EN_IMGD        = 3,
  EN_AFD         = 4,
  EN_LPS         = 5,
  EN_LPM         = 6,
  EN_MLD         = 7,
  EN_CMH         = 8,
  EN_CMD         = 9
} EN_FlowUnits;

/// Demand model types
typedef enum {
  EN_DDA         = 0,   //!< Demand driven analysis
  EN_PDA         = 1    //!< Pressure driven analysis
} EN_DemandModel;

/// Simulation Option codes
typedef enum {
  EN_TRIALS       = 0,  //!> Maximum hydraulic trials allowed
  EN_ACCURACY     = 1,  //!> Hydraulic convergence accuracy
  EN_TOLERANCE    = 2,  //!> Water quality tolerance
  EN_EMITEXPON    = 3,  //!> Exponent for emitter head loss formula
  EN_DEMANDMULT   = 4,  //!> Global demand multiplier
  EN_HEADERROR    = 5,  //!> Maximum allowable head loss error
  EN_FLOWCHANGE   = 6,  //!> Maximum allowable flow change
  EN_DEMANDDEFPAT = 7,  //!> Default demand time pattern
  EN_HEADLOSSFORM = 8   //!> Head loss formula code
} EN_Option;

/// Simple control types
typedef enum {
  EN_LOWLEVEL    = 0,
  EN_HILEVEL     = 1,
  EN_TIMER       = 2,
  EN_TIMEOFDAY   = 3
} EN_ControlType;

/// Reporting statistic types
typedef enum {
  EN_AVERAGE     = 1,   //!> Report average value over simulation period
  EN_MINIMUM     = 2,   //!> Report minimum value over simulation period
  EN_MAXIMUM     = 3,   //!> Report maximum value over simulation period
  EN_RANGE       = 4    //!> Report maximum - minimum over simulation period
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
  EN_NOSAVE        = 0,  //!> Don't save hydraulics; don't re-initialize flows
  EN_SAVE          = 1,  //!> Save hydraulics to file, don't re-initialize flows
  EN_INITFLOW      = 10, //!> Don't save hydraulics; re-initialize flows
  EN_SAVE_AND_INIT = 11  //!> Save hydraulics; re-initialize flows
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
  EN_V_CURVE     = 0,   //!< Tank volume curve
  EN_P_CURVE     = 1,   //!< Pump characteristic curve
  EN_E_CURVE     = 2,   //!< Pump efficiency curve
  EN_H_CURVE     = 3,   //!< Valve head loss curve
  EN_G_CURVE     = 4    //!< General\default curve
} EN_CurveType;

/// Deletion action types
typedef enum {
  EN_UNCONDITIONAL = 0, //!> Delete all controls that contain object
  EN_CONDITIONAL   = 1  //!> Cancel object deletion if contained in controls
} EN_ActionCodeType;

/// Rule object codes
typedef enum {
  EN_R_NODE      = 6,
  EN_R_LINK      = 7,
  EN_R_SYSTEM    = 8
} EN_RuleObject;

/// Rule variable codes
typedef enum {
  EN_R_DEMAND    = 0,
  EN_R_HEAD      = 1,
  EN_R_GRADE     = 2,
  EN_R_LEVEL     = 3,
  EN_R_PRESSURE  = 4,
  EN_R_FLOW      = 5,
  EN_R_STATUS    = 6,
  EN_R_SETTING   = 7,
  EN_R_POWER     = 8,
  EN_R_TIME      = 9,
  EN_R_CLOCKTIME = 10,
  EN_R_FILLTIME  = 11,
  EN_R_DRAINTIME = 12
} EN_RuleVariable;

/// Rule operator types
typedef enum {
  EN_R_EQ        = 0,
  EN_R_NE        = 1,
  EN_R_LE        = 2,
  EN_R_GE        = 3,
  EN_R_LT        = 4,
  EN_R_GT        = 5,
  EN_R_IS        = 6,
  EN_R_NOT       = 7,
  EN_R_BELOW     = 8,
  EN_R_ABOVE     = 9
} EN_RuleOperator;

/// Rule status types
typedef enum {
  EN_R_IS_OPEN   = 1,
  EN_R_IS_CLOSED = 2,
  EN_R_IS_ACTIVE = 3
} EN_RuleStatus;

/// Status report types
typedef enum {
  EN_NO_REPORT     = 0,
  EN_NORMAL_REPORT = 1,
  EN_FULL_REPORT   = 2
} EN_StatusReport;


#endif //EPANET2_ENUMS_H
