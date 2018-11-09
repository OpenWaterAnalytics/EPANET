

#ifndef EPANET2_ENUMS_H
#define EPANET2_ENUMS_H


// --- Define the EPANET toolkit constants

#define  EN_MAXID  31  /**< Max. # characters in ID name */
#define  EN_MAXMSG 255 /**< Max. # characters in message text */

/// Node property codes
typedef enum {
  EN_ELEVATION    = 0, /**< Node Elevation */
  EN_BASEDEMAND   = 1, /**< Node Base Demand, from last demand category */
  EN_PATTERN      = 2, /**< Node Demand Pattern */
  EN_EMITTER      = 3, /**< Node Emitter Coefficient */
  EN_INITQUAL     = 4, /**< Node initial quality */
  EN_SOURCEQUAL   = 5, /**< Node source quality */
  EN_SOURCEPAT    = 6, /**< Node source pattern index */
  EN_SOURCETYPE   = 7, /**< Node source type */
  EN_TANKLEVEL    = 8, /**< Tank Level */
  EN_DEMAND       = 9, /**< Node current simulated demand */
  EN_HEAD         = 10, /**< Node Head value */
  EN_PRESSURE     = 11, /**< Node pressure value */
  EN_QUALITY      = 12, /**< Node quality value */
  EN_SOURCEMASS   = 13, /**< Node source mass value */
  EN_INITVOLUME   = 14, /**< Tank or Reservoir initial volume */
  EN_MIXMODEL     = 15, /**< Tank mixing model */
  EN_MIXZONEVOL   = 16, /**< Tank mixing zone volume  */
  EN_TANKDIAM     = 17, /**< Tank diameter  */
  EN_MINVOLUME    = 18, /**< Tank minimum volume  */
  EN_VOLCURVE     = 19, /**< Tank volume curve index  */
  EN_MINLEVEL     = 20, /**< Tank minimum level  */
  EN_MAXLEVEL     = 21, /**< Tank maximum level  */
  EN_MIXFRACTION  = 22, /**< Tank mixing fraction  */
  EN_TANK_KBULK   = 23, /**< Tank bulk decay coefficient  */
  EN_TANKVOLUME   = 24, /**< Tank current volume  */
  EN_MAXVOLUME    = 25  /**< Tank maximum volume  */
} EN_NodeProperty;

/// Link property codes
typedef enum {
  EN_DIAMETER     = 0,
  EN_LENGTH       = 1,
  EN_ROUGHNESS    = 2,
  EN_MINORLOSS    = 3,
  EN_INITSTATUS   = 4,
  EN_INITSETTING  = 5,
  EN_KBULK        = 6,
  EN_KWALL        = 7,
  EN_FLOW         = 8,
  EN_VELOCITY     = 9,
  EN_HEADLOSS     = 10,
  EN_STATUS       = 11,
  EN_SETTING      = 12,
  EN_ENERGY       = 13,
  EN_LINKQUAL     = 14,
  EN_LINKPATTERN  = 15,
  EN_EFFICIENCY   = 16,
  EN_HEADCURVE    = 17,
  EN_EFFICIENCYCURVE = 18,
  EN_PRICEPATTERN = 19,
  EN_STATE        = 20,
  EN_CONST_POWER  = 21,
  EN_SPEED        = 22
} EN_LinkProperty;

/// Time parameter codes
typedef enum {
  EN_DURATION     = 0,
  EN_HYDSTEP      = 1,
  EN_QUALSTEP     = 2,
  EN_PATTERNSTEP  = 3,
  EN_PATTERNSTART = 4,
  EN_REPORTSTEP   = 5,
  EN_REPORTSTART  = 6,
  EN_RULESTEP     = 7,
  EN_STATISTIC    = 8,
  EN_PERIODS      = 9,
  EN_STARTTIME    = 10,
  EN_HTIME        = 11,
  EN_QTIME        = 12,
  EN_HALTFLAG     = 13,
  EN_NEXTEVENT    = 14,
  EN_NEXTEVENTIDX = 15
} EN_TimeProperty;

typedef enum {
  EN_ITERATIONS    = 0,
  EN_RELATIVEERROR = 1,
  EN_MAXHEADERROR  = 2,
  EN_MAXFLOWCHANGE = 3,
  EN_MASSBALANCE   = 4
} EN_AnalysisStatistic;

typedef enum {
  EN_NODECOUNT    = 0,   /**< Number of Nodes (Juntions + Tanks + Reservoirs) */
  EN_TANKCOUNT    = 1,   /**< Number of Tanks and Reservoirs */
  EN_LINKCOUNT    = 2,   /**< Number of Links (Pipes + Pumps + Valves) */
  EN_PATCOUNT     = 3,   /**< Number of Time Patterns */
  EN_CURVECOUNT   = 4,   /**< Number of Curves */
  EN_CONTROLCOUNT = 5,   /**< Number of Control Statements */
  EN_RULECOUNT	  = 6    /**< Number of Rule-based Control Statements */
} EN_CountType;

typedef enum {
  EN_JUNCTION    = 0,
  EN_RESERVOIR   = 1,
  EN_TANK        = 2
} EN_NodeType;

typedef enum {
  EN_CVPIPE       = 0,   /* Link types. */
  EN_PIPE         = 1,   /* See LinkType in TYPES.H */
  EN_PUMP         = 2,
  EN_PRV          = 3,
  EN_PSV          = 4,
  EN_PBV          = 5,
  EN_FCV          = 6,
  EN_TCV          = 7,
  EN_GPV          = 8
} EN_LinkType;

typedef enum {
  EN_NONE        = 0,    /* Quality analysis types. */
  EN_CHEM        = 1,    /* See QualType in TYPES.H */
  EN_AGE         = 2,
  EN_TRACE       = 3
} EN_QualityType;

typedef enum {
  EN_CONCEN      = 0,    /* Source quality types.      */
  EN_MASS        = 1,    /* See SourceType in TYPES.H. */
  EN_SETPOINT    = 2,
  EN_FLOWPACED   = 3
} EN_SourceType;

typedef enum {          /* Head loss formula:                  */
  EN_HW          = 0,    /*   Hazen-Williams                    */
  EN_DW          = 1,    /*   Darcy-Weisbach                    */
  EN_CM          = 2     /*   Chezy-Manning                     */
} EN_FormType;           /* See FormType in TYPES.H             */

typedef enum {
  EN_CFS         = 0,    /* Flow units types.   */
  EN_GPM         = 1,    /* See FlowUnitsType   */
  EN_MGD         = 2,    /* in TYPES.H.         */
  EN_IMGD        = 3,
  EN_AFD         = 4,
  EN_LPS         = 5,
  EN_LPM         = 6,
  EN_MLD         = 7,
  EN_CMH         = 8,
  EN_CMD         = 9
} EN_FlowUnits;

typedef enum {           /* Demand model types. */
  EN_DDA         = 0,   /**< Demand driven analysis */
  EN_PDA         = 1    /**< Pressure driven analysis */
} EN_DemandModel;

/// Simulation Option codes
typedef enum {
  EN_TRIALS         = 0,
  EN_ACCURACY       = 1,
  EN_TOLERANCE      = 2,
  EN_EMITEXPON      = 3,
  EN_DEMANDMULT     = 4,
  EN_HEADERROR      = 5,
  EN_FLOWCHANGE     = 6,
  EN_DEMANDDEFPAT   = 7,
  EN_HEADLOSSFORM 	= 8
} EN_Option;

typedef enum {
  EN_LOWLEVEL    = 0,   /* Control types.  */
  EN_HILEVEL     = 1,   /* See ControlType */
  EN_TIMER       = 2,   /* in TYPES.H.     */
  EN_TIMEOFDAY   = 3
} EN_ControlType;

typedef enum {
  EN_AVERAGE     = 1,   /* Time statistic types.    */
  EN_MINIMUM     = 2,   /* See TstatType in TYPES.H */
  EN_MAXIMUM     = 3,
  EN_RANGE       = 4
} EN_StatisticType;

typedef enum {
  EN_MIX1        = 0,   /* Tank mixing models */
  EN_MIX2        = 1,
  EN_FIFO        = 2,
  EN_LIFO        = 3
} EN_MixingModel;

typedef enum {
  EN_NOSAVE        = 0,
  EN_SAVE          = 1,
  EN_INITFLOW      = 10,
  EN_SAVE_AND_INIT = 11
} EN_SaveOption;

typedef enum {
  EN_CONST_HP    = 0,   /* constant horsepower       */
  EN_POWER_FUNC  = 1,   /* power function            */
  EN_CUSTOM      = 2,   /* user-defined custom curve */
  EN_NOCURVE     = 3    /* no curve                  */
} EN_PumpType;

typedef enum {
  EN_V_CURVE     = 0,   /*    volume curve                      */
  EN_P_CURVE     = 1,   /*    pump curve                        */
  EN_E_CURVE     = 2,   /*    efficiency curve                  */
  EN_H_CURVE     = 3,   /*    head loss curve                   */
  EN_G_CURVE     = 4    /*    General\default curve             */
} EN_CurveType;


#endif //EPANET_2_2_H
