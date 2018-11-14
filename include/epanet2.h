/** @file epanet2.h
 @see http://github.com/openwateranalytics/epanet
 */

/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2.h
 Description:  symbolic constants and function declarations for the EPANET API 
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/10/2018
 ******************************************************************************
 */

#ifndef EPANET2_H
#define EPANET2_H

// the toolkit can be compiled with support for double-precision as well.
// just make sure that you use the correct #define in your client code.
#ifndef EN_API_FLOAT_TYPE
  #define EN_API_FLOAT_TYPE float
#endif

#ifdef WITH_GENX
   #include "epanet_export.h"
#else 
  // --- define WINDOWS
  #undef WINDOWS
  #ifdef _WIN32
    #define WINDOWS
  #endif
  #ifdef __WIN32__
    #define WINDOWS
  #endif

  // --- define DLLEXPORT
  #ifndef DLLEXPORT
    #ifdef WINDOWS
      #ifdef __cplusplus
        #define DLLEXPORT __declspec(dllexport)
      #else
        #define DLLEXPORT __declspec(dllexport) __stdcall
      #endif // __cplusplus
    #elif defined(CYGWIN)
      #define DLLEXPORT __stdcall
    #elif defined(__APPLE__)
      #ifdef __cplusplus
        #define DLLEXPORT
      #else
        #define DLLEXPORT
      #endif
    #else
      #define DLLEXPORT
    #endif
  #endif
#endif


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
  EN_CVPIPE       = 0,
  EN_PIPE         = 1,
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
  EN_CHEM        = 1,
  EN_AGE         = 2,
  EN_TRACE       = 3
} EN_QualityType;

typedef enum {
  EN_CONCEN      = 0,    /* Source quality types.      */
  EN_MASS        = 1,
  EN_SETPOINT    = 2,
  EN_FLOWPACED   = 3
} EN_SourceType;

typedef enum {           /* Head loss formula:                  */
  EN_HW          = 0,    /*   Hazen-Williams                    */
  EN_DW          = 1,    /*   Darcy-Weisbach                    */
  EN_CM          = 2     /*   Chezy-Manning                     */
} EN_HeadLossType;

typedef enum {
  EN_CFS         = 0,    /* Flow units types.   */
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
  EN_HILEVEL     = 1,
  EN_TIMER       = 2,
  EN_TIMEOFDAY   = 3
} EN_ControlType;

typedef enum {
  EN_AVERAGE     = 1,   /* Time statistic types.    */
  EN_MINIMUM     = 2,
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

typedef enum {
  EN_UNCONDITIONAL = 0,
  EN_CONDITIONAL   = 1
} EN_ActionCodeType;

typedef enum {
    EN_NO_REPORT     = 0,
    EN_NORMAL_REPORT = 1,
    EN_FULL_REPORT   = 2
} EN_StatusReport;

// --- Declare the EPANET toolkit functions
#if defined(__cplusplus)
extern "C" {
#endif
  
  /**
   @brief The EPANET Project wrapper object
   */
//  typedef void *EN_ProjectHandle;
    typedef struct Project *EN_Project;
  
  /**
   @brief runs a complete EPANET simulation
   @param inpFile pointer to name of input file (must exist)
   @param rptFile pointer to name of report file (to be created)
   @param binOutFile pointer to name of binary output file (to be created)
   @param callback a callback function that takes a character string (char *) as its only parameter.
   @return error code
   
   The callback function should reside in and be used by the calling
   code to display the progress messages that EPANET generates
   as it carries out its computations. If this feature is not
   needed then the argument should be NULL.
   */
  int  DLLEXPORT ENepanet(const char *inpFile, const char *rptFile, 
                 const char *binOutFile, void (*callback) (char *));
  
  /**
   @brief Initializes an EPANET session
   @param rptFile pointer to name of report file (to be created)
   @param binOutFile pointer to name of binary output file (to be created)
   @param UnitsType flow units flag
   @param HeadlossFormula headloss formula flag
   @return error code
   */
  int  DLLEXPORT ENinit(const char *rptFile, const char *binOutFile, 
                 int UnitsType, int HeadlossFormula);
  
  /**
   @brief Opens EPANET input file & reads in network data
   @param inpFile pointer to name of input file (must exist)
   @param rptFile pointer to name of report file (to be created)
   @param binOutFile pointer to name of binary output file (to be created)
   @return error code
   */
  int  DLLEXPORT ENopen(const char *inpFile, const char *rptFile,
                 const char *binOutFile);
  
  /**
   @brief Saves current data to "INP" formatted text file.
   @param filename The file path to create
   @return Error code
   */
  int  DLLEXPORT ENsaveinpfile(const char *filename);
  
  /**
   @brief Frees all memory and files used by EPANET
   @return Error code
   */
  int  DLLEXPORT ENclose();
  
  /**
   @brief Solves the network hydraulics for all time periods
   @return Error code
   */
  int  DLLEXPORT ENsolveH();
  
  /**
   @brief Saves hydraulic results to binary file
   @return Error code
   
   Must be called before ENreport() if no WQ simulation has been made.
   Should not be called if ENsolveQ() will be used.
   */
  int  DLLEXPORT ENsaveH();
  
  /**
   @brief Sets up data structures for hydraulic analysis
   @return Error code
   */
  int  DLLEXPORT ENopenH();
  
  /**
   @brief Initializes hydraulic analysis
   @param initFlag 2-digit initialization flag
   @return Error code

   The initialization flag is a two digit number where the 1st (left) digit
   indicates if link flows should be re-initialized (1) or not (0), and the
   2nd digit indicates if hydraulic results should be saved to file (1) or not (0).
   */
  int  DLLEXPORT ENinitH(int initFlag);
  
  /**
   @brief Run a hydraulic solution period
   @param[out] currentTime The current simulation time in seconds
   @return Error or warning code
   @see ENsolveH
   
   This function is used in a loop with ENnextH() to run
   an extended period hydraulic simulation.
   See ENsolveH() for an example.
   */
  int  DLLEXPORT ENrunH(long *currentTime);
  
  /**
   @brief Determine time (in seconds) until next hydraulic event
   @param[out] tStep Time (seconds) until next hydraulic event. 0 marks end of simulation period.
   @return Error code
   
   This function is used in a loop with ENrunH() to run an extended period hydraulic simulation.
   See ENsolveH() for an example.
   */
  int  DLLEXPORT ENnextH(long *tStep);
  
  
  /**
   @brief Frees data allocated by hydraulics solver
   @return Error code
   */
  int  DLLEXPORT ENcloseH();
  
  /**
   @brief Copies binary hydraulics file to disk
   @param filename Name of file to be created
   @return Error code
   */
  int  DLLEXPORT ENsavehydfile(char *filename);
  
  /**
   @brief Opens previously saved binary hydraulics file
   @param filename Name of file to be used
   @return Error code
   */
  int  DLLEXPORT ENusehydfile(char *filename);
  
  /**
   @brief Solves for network water quality in all time periods
   @return Error code
   */
  int  DLLEXPORT ENsolveQ();
  
  /**
   @brief Sets up data structures for WQ analysis
   @return Error code
   */
  int  DLLEXPORT ENopenQ();
  
  /**
   @brief Initializes water quality analysis
   @param saveFlag EN_SAVE (1) if results saved to file, EN_NOSAVE (0) if not
   @return Error code
   */
  int  DLLEXPORT ENinitQ(int saveFlag);
  
  /**
   @brief Retrieves hydraulic & WQ results at time t.
   @param[out] currentTime Current simulation time, in seconds.
   @return Error code
   
   This function is used in a loop with ENnextQ() to run
   an extended period WQ simulation. See ENsolveQ() for
   an example.
   */
  int  DLLEXPORT ENrunQ(long *currentTime);
  
  /**
   @brief Advances WQ simulation to next hydraulic event.
   @param[out] tStep Time in seconds until next hydraulic event. 0 marks end of simulation period.
   @return Error code
   
   This function is used in a loop with ENrunQ() to run
   an extended period WQ simulation. See ENsolveQ() for
   an example.
   */
  int  DLLEXPORT ENnextQ(long *tStep);
  
  /**
   @brief Advances WQ simulation by a single WQ time step
   @param[out] timeLeft Time left in overall simulation (in seconds)
   @return Error code
   
   This function is used in a loop with ENrunQ() to run
   an extended period WQ simulation.
   */
  int  DLLEXPORT ENstepQ(long *timeLeft);
  
  /**
   @brief Frees data allocated by water quality solver.
   @return Error code.
   */
  int  DLLEXPORT ENcloseQ();
  
  /**
   @brief Writes line of text to the report file.
   @param line Text string to write
   @return Error code.
   */
  int  DLLEXPORT ENwriteline(char *line);
  
  /**
   @brief Writes simulation report to the report file
   @return Error code
   */
  int  DLLEXPORT ENreport();
  
  /**
   @brief Resets report options to default values
   @return Error code
   */
  int  DLLEXPORT ENresetreport();
  
  /**
   @brief Processes a reporting format command
   @return Error code
   */
  int  DLLEXPORT ENsetreport(char *reportFormat);
  
  /**
   @brief Retrieves parameters that define a simple control
   @param controlIndex Position of control in list of controls added to the project
   @param[out] controlType Control type code (see EN_ControlType enumeration)
   @param[out] linkIndex Index of controlled link
   @param[out] setting Control setting on link
   @param[out] nodeIndex Index of controlling node (0 for TIMER or TIMEOFDAY control)
   @param[out] level Control level (tank level, junction pressure, or time (seconds))
   @return Error code
   */
  int  DLLEXPORT ENgetcontrol(int controlIndex, int *controlType, int *linkIndex,
                 EN_API_FLOAT_TYPE *setting, int *nodeIndex, EN_API_FLOAT_TYPE *level);
  
  /**
   @brief Retrieves the number of components of a given type in the network.
   @param code Component code (see EPANET2.H)
   @param[out] count Number of components in network
   @return Error code
   */
  int  DLLEXPORT ENgetcount(int code, int *count);
  
  /**
   @brief Gets value for an analysis option
   @param code Option code (see EPANET2.H)
   @param[out] value Option value
   @return Error code
   */
  int  DLLEXPORT ENgetoption(int code, EN_API_FLOAT_TYPE *value);
  
  /**
   @brief Retrieves value of specific time parameter.
   @param code Time parameter code
   @param[out] value Value of time parameter.
   @return Error code
   */
  int  DLLEXPORT ENgettimeparam(int code, long *value);
  
  /**
   @brief Retrieves the flow units code
   @param[out] code Code of flow units in use
   @return Error code
   */
  int  DLLEXPORT ENgetflowunits(int *code);
  
  /**
   @brief Sets the flow units
   @param code Code of flow units to use
   @return Error code
   */
  int  DLLEXPORT ENsetflowunits(int code);

  /**
   @brief Retrieves the type of demand model in use and its parameters
   @param[out] type  Type of demand model (EN_DDA or EN_PDA)
   @param[out] pmin  Pressure below which there is no demand
   @param[out] preq  Pressure required to deliver full demand
   @param[out] pexp  Pressure exponent in demand function
   @return Error code
  */
  int DLLEXPORT ENgetdemandmodel(int *type, EN_API_FLOAT_TYPE *pmin,
                EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp);

  /**
  @brief Sets the type of demand model to use and its parameters
  @param type  Type of demand model (EN_DDA or EN_PDA)
  @param pmin  Pressure below which there is no demand
  @param preq  Pressure required to deliver full demand
  @param pexp  Pressure exponent in demand function
  @return Error code
  */
  int DLLEXPORT ENsetdemandmodel(int type, EN_API_FLOAT_TYPE pmin,
                EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp);

  /**
   @brief Retrieves the index of the time pattern with specified ID
   @param id String ID of the time pattern
   @param[out] index Index of the specified time pattern
   @return Error code
   @see ENgetpatternid
   */
  int  DLLEXPORT ENgetpatternindex(char *id, int *index);
  
  /**
   @brief Retrieves ID of a time pattern with specific index.
   @param index The index of a time pattern.
   @param[out] id The string ID of the time pattern.
   @return Error code
   @see ENgetpatternindex
   */
  int  DLLEXPORT ENgetpatternid(int index, char *id);

  /**
   @brief Retrieves the number of multipliers in a time pattern.
   @param index The index of a time pattern.
   @param[out] len The length of the time pattern.
   @return Error code
   */
  int  DLLEXPORT ENgetpatternlen(int index, int *len);
  
  /**
   @brief Retrive a multiplier from a pattern for a specific time period.
   @param index The index of a time pattern
   @param period The pattern time period. First time period is 1.
   @param[out] value Pattern multiplier
   @return Error code
   */
  int  DLLEXPORT ENgetpatternvalue(int index, int period, EN_API_FLOAT_TYPE *value);
  
  /**
   @brief Retrieve the average multiplier value in a time pattern
   @param index The index of a time pattern
   @param[out] value The average of all of this time pattern's values
   @return Error code
   */
  int  DLLEXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value);
  
  /**
   @brief Retrieve the type of quality analytis to be run.
   @param[out] qualcode The quality analysis code number.
   @param[out] tracenode The index of node being traced, if qualcode == trace
   @return Error code
   @see ENsetqualtype
   */
  int  DLLEXPORT ENgetqualtype(int *qualcode, int *tracenode);
  
  /**
   @brief Get the text of an error code.
   @param errcode The error code
   @param[out] errmsg The error string represented by the code
   @param maxLen The maximum number of characters to copy into the char pointer errmsg
   @return Error code
   */
  int  DLLEXPORT ENgeterror(int errcode, char *errmsg, int maxLen);
  
  /**
   @brief Get hydraulic simulation statistic
   @param code The type of statistic to get
   @param[out] value The value of the statistic
   @return Error code
   */
  int  DLLEXPORT ENgetstatistic(int code, EN_API_FLOAT_TYPE* value);
  
  /**
   @brief Get index of node with specified ID
   @param id The string ID of the node
   @param[out] index The node's index (first node is index 1)
   @return Error code
   @see ENgetnodeid
   */
  int  DLLEXPORT ENgetnodeindex(char *id, int *index);
  
  /**
   @brief Get the string ID of the specified node.
   @param index The index of the node (first node is index 1)
   @param[out] id The string ID of the specified node.
   @return Error code
   @see ENgetnodeindex

   The ID string must be sized to hold at least MAXID characters.
   */
  int  DLLEXPORT ENgetnodeid(int index, char *id);
  
  /**
   @brief Get the type of node with specified index.
   @param index The index of a node (first node is index 1)
   @param[out] code The type code for the node (see the EN_NodeType enumeration)
   @return Error code
   */
  int  DLLEXPORT ENgetnodetype(int index, int *code);
  
  /**
   @brief Get a property value for specified node
   @param index The index of a node (first node is index 1).
   @param code The property type code
   @param[out] value The value of the node's property.
   @return Error code
   @see EN_NodeProperty
   */
  int  DLLEXPORT ENgetnodevalue(int index, int code, EN_API_FLOAT_TYPE *value);
  
  /**
   @brief Get coordinates (x,y) for a node.
   @param index The index of a node (first node is index 1).
   @param[out] x X-value of node's coordinate
   @param[out] y Y-value of node's coordinate
   @return Error code
   @see ENsetcoord
   */
  int  DLLEXPORT ENgetcoord(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);
  
  /**
   @brief Set coordinates (x,y) for a node.
   @param index The index of a node (first node is index 1)
   @param x X-value of node's coordinate
   @param y Y-value of node's coordinate
   @return Error code
   @see ENgetcoord
   */
  int  DLLEXPORT ENsetcoord(int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);
  
  /**
   @brief Get the number of demand categories for a node.
   @param nodeIndex The index of a node (first node is index 1)
   @param[out] numDemands The number of demand categories
   @return Error code
   */
  int  DLLEXPORT ENgetnumdemands(int nodeIndex, int *numDemands);
  
  /**
   @brief Get a node's base demand for a specified category.
   @param nodeIndex The index of a node (first node is index 1)
   @param demandIndex The index of the demand category (starting at 1)
   @return Error code
   */
  int  DLLEXPORT ENgetbasedemand(int nodeIndex, int demandIndex,
                 EN_API_FLOAT_TYPE *baseDemand);
  
  /**
   @brief Get the index of the demand pattern assigned to a node for a category index.
   @param nodeIndex The index of a node (first node is index 1).
   @param demandIndex The index of a category (first category is index 1).
   @param[out] pattIndex The index of the pattern for this node and category.
   @return Error code
   */
  int  DLLEXPORT ENgetdemandpattern(int nodeIndex, int demandIndex, int *pattIndex);
  
  /**
   @brief Get the index of a Link with specified ID.
   @param id The string ID of a link.
   @param[out] index The index of the named link (first link is index 1)
   @return Error code
   @see ENgetlinkid
   */
  int  DLLEXPORT ENgetlinkindex(char *id, int *index);
  
  /**
   @brief Get the string ID of a link with specified index
   @param index The index of a link (first link is index 1)
   @param[out] id The ID of the link.
   @return Error code
   @see ENgetlinkindex

   The ID string must be sized to hold at least MAXID characters.
   */
  int  DLLEXPORT ENgetlinkid(int index, char *id);
  
  /**
   @brief Get the link type code for a specified link
   @param index The index of a link (first link is index 1)
   @param[out] code The type code of the link (see the EN_LinkType enumeration)
   @return Error code
   @see EN_LinkType
   */
  int  DLLEXPORT ENgetlinktype(int index, EN_LinkType *code);

  /**
   @brief Set the link type code for a specified link
   @param[in,out] index The index of a link before [in] and after [out] the type change
   @param code The new type code of the link (see EN_LinkType).
   @param actionCode Action taken if any controls contain the link.
   @return Error code
   @see the EN_LinkType enumeration

   If 'actionCode' is EN_UNCONDITIONAL then all simple and rule-based controls that
   contain the link are deleted when the link's type is changed. If set to
   EN_CONDITIONAL then the type change is cancelled if the link appears in any
   control and an error code is returned. 
   */
  int  DLLEXPORT ENsetlinktype(int *index, EN_LinkType Code, int actionCode);
  
  /**
   @brief Get the indexes of a link's start- and end-nodes.
   @param index The index of a link (first link is index 1)
   @param[out] node1 The index of the link's start node (first node is index 1).
   @param[out] node2 The index of the link's end node (first node is index 1).
   @return Error code
   @see ENgetnodeid, ENgetlinkid
   */
  int  DLLEXPORT ENgetlinknodes(int index, int *node1, int *node2);
  
  /**
   @brief Get a property value for specified link.
   @param index The index of a node (first node is index 1).
   @param code The parameter desired.
   @param[out] value The value of the link's specified property.
   @return Error code
   @see ENgetnodevalue, EN_LinkProperty
   */
  int  DLLEXPORT ENgetlinkvalue(int index, int code, EN_API_FLOAT_TYPE *value);
  
  /**
   @brief Get a curve's properties.
   @param curveIndex The index of a curve (first curve is index 1).
   @param[out] id The curve's string ID. Client code must preallocate at least MAXID characters.
   @param[out] nValues The number of values in the curve's (x,y) list.
   @param[out] xValues The curve's x-values. Pointer must be freed by client.
   @param[out] yValues The curve's y-values. Pointer must be freed by client.
   @return Error code.
   */
  int  DLLEXPORT ENgetcurve(int curveIndex, char* id, int *nValues,
                 EN_API_FLOAT_TYPE **xValues, EN_API_FLOAT_TYPE **yValues);
  
  /**
   @brief Retrieves the curve index for a specified pump index.
   @param pumpIndex The index of a pump
   @param[out] curveIndex The index of the curve used by the pump.
   @return Error code.
   */
  int  DLLEXPORT ENgetheadcurveindex(int pumpIndex, int *curveIndex);
  
  /**
   @brief Sets the curve id for a specified pump index.
   @param pumpIndex The index of the pump
   @param curveIndex The index of the curve used by the pump
   @return Error code.
   */
  int  DLLEXPORT ENsetheadcurveindex(int pumpIndex, int curveIndex);
  
  /**
   @brief Get the type of pump
   @param linkIndex The index of the pump element
   @param[out] outType The integer-typed pump curve type signifier (output parameter)
   @return Error code
   @see EN_PumpType
   */
  int  DLLEXPORT ENgetpumptype(int linkIndex, int *outType);

  /**
   @brief Get the type of a curve
   @param curveIndex The index of the curve element
   @param[out] outType The integer-typed curve curve type signifier (output parameter)
   @return Error code
   @see EN_CurveType
   */
  int  DLLEXPORT ENgetcurvetype(int curveIndex, int *outType);
    
  /**
   @brief Get the version number. 
   @param[out] version The version of EPANET
   @return Error code.

   The version number is to be interpreted with implied decimals, i.e.,
   "20100" == "2(.)01(.)00"
   */
  int  DLLEXPORT ENgetversion(int *version);

  /**
   @brief Add a new simple control to the project.
   @param[out] cindex The index of the new control. First control is index 1.
   @param ctype The type of control to add (see EN_ControlType).
   @param lindex The index of a link to control.
   @param setting The control setting applied to the link.
   @param nindex The index of a node used to control the link, or 0 for TIMER / TIMEOFDAY control.
   @param level control point (tank level, junction pressure, or time in seconds).
   @return Error code.
   */
  int  DLLEXPORT ENaddcontrol(int *cindex, int ctype, int lindex,
                 EN_API_FLOAT_TYPE setting, int nindex, EN_API_FLOAT_TYPE level);
  
  /**
   @brief Delete an existing simple control
   @param cindex The index of the control. First control is index 1.
   @return Error code.
  */
  int  DLLEXPORT ENdeletecontrol(int cindex);

  /**
   @brief Set the parameters of an existing simple control.
   @param cindex The index of the control. First control is index 1.
   @param ctype The type of control to use (see EN_ControlType).
   @param lindex The index of a link to control.
   @param setting The control setting applied to the link.
   @param nindex The index of a node used to control the link, or 0 for TIMER / TIMEOFDAY control.
   @param level control point (tank level, junction pressure, or time in seconds).
   @return Error code.
   */
  int  DLLEXPORT ENsetcontrol(int cindex, int ctype, int lindex,
                 EN_API_FLOAT_TYPE setting, int nindex, EN_API_FLOAT_TYPE level);

  /**
  @brief Change the ID name for a node.
  @param index The index of a node. First node is index 1.
  @param newid A string containing the node's new ID name.
  @return Error code.
  */
  int DLLEXPORT ENsetnodeid(int index, char *newid);
  
  /**
   @brief Set a property value for a node.
   @param index The index of a node. First node is index 1.
   @param code The code for the proprty to set.
   @param v The value to set for this node and property.
   @return Error code.
   @see EN_NodeProperty
   */
  int  DLLEXPORT ENsetnodevalue(int index, int code, EN_API_FLOAT_TYPE v);

  /**
  @brief Change the ID name for a link.
  @param index The index of a link. First link is index 1.
  @param newid A string containing the link's new ID name.
  @return Error code.
  */
  int DLLEXPORT ENsetlinkid(int index, char *newid);
  
  /**
   @brief Set the indexes of a link's start- and end-nodes.
   @param index The index of a link (first link is index 1)
   @param node1 The index of the link's start node (first node is index 1).
   @param node2 The index of the link's end node (first node is index 1).
   @return Error code
   @see ENsetnodeid, ENsetlinkid
   */
  int  DLLEXPORT ENsetlinknodes(int index, int node1, int node2);
  
  /**
   @brief Set a property value for a link.
   @param index The index of a link. First link is index 1.
   @param code The code for the property to set.
   @param v The value to set for this link and property.
   @return Error code.
   @see EN_LinkProperty
   */
  int  DLLEXPORT ENsetlinkvalue(int index, int code, EN_API_FLOAT_TYPE v);
  
  /**
   @brief Add a new time pattern.
   @param id The string ID of the pattern to add.
   @return Error code.
   @see ENgetpatternindex
   */
  int  DLLEXPORT ENaddpattern(char *id);
  
  /**
   @brief Set multipliers for a specific pattern
   @param index The index of a pattern. First pattern is index 1.
   @param f An array of multipliers
   @param len The length of array f.
   @return Error code.
   @see ENgetpatternindex
   */
  int  DLLEXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *f, int len);
  
  /**
   @brief Set the multiplier for a specific pattern at a specific period.
   @param index The index of a pattern. First pattern is index 1.
   @param period The period of the pattern to set.
   @param value The value of the multiplier to set.
   @return Error code.
   */
  int  DLLEXPORT ENsetpatternvalue(int index, int period, EN_API_FLOAT_TYPE value);
  
  /**
   @brief Set the value for a time parameter.
   @param code The code for the parameter to set.
   @param value The desired value of the parameter.
   @return Error code.
   @see EN_TimeProperty
   */
  int  DLLEXPORT ENsettimeparam(int code, long value);
  
  /**
   @brief Set a value for an anlysis option.
   @param code The code for the desired option.
   @param v The desired value for the option specified.
   @return Error code.
   @see EN_Option
   */
  int  DLLEXPORT ENsetoption(int code, EN_API_FLOAT_TYPE v);
  
  /**
   @brief Set the level of hydraulic status reporting.
   @param code Status reporting code (see EN_StatusReport).
   @return Error code.
   */
  int  DLLEXPORT ENsetstatusreport(int code);
  
  /**
   @brief Set the type of quality analysis called for.
   @param qualcode Type of analysis to be made (see EN_QualityType).
   @param chemname Name of the quality constituent.
   @param chemunits Concentration units of the constituent.
   @param tracenode ID of node being traced (if applicable).
   @return Error code.
   
   Note: "chemname" and "chemunits" only apply when "qualcode" is EN_CHEM.
   "tracenode" only applies when 'qualcode" is EN_TRACE.
   */
  int  DLLEXPORT ENsetqualtype(int qualcode, char *chemname, char *chemunits,
                 char *tracenode);
  
  /**
   @brief Get information about the type of water quality analysis requested.
   @param[out] qualcode Type of analysis to be made (see EN_QualityType).
   @param[out] chemname Name of the quality constituent.
   @param[out] chemunits Concentration units of the constituent.
   @param[out] tracenode ID of node being traced (if applicable). 
   @return Error code.
   */
  int  DLLEXPORT ENgetqualinfo(int *qualcode, char *chemname, char *chemunits,
                 int *tracenode);

  /**
   @brief Set the name of a node's demand category.
   @param nodeIndex The node's index.
   @param demandIdx Index of the node's demand.
   @param demandName Name for the category the demand belongs to.
   @return Error code.
   @see ENgetdemandname
   */
  int DLLEXPORT ENsetdemandname(int nodeIndex, int demandIdx, char *demandName);

  /**
   @brief Retrieve the name of a node's demand category.
   @param nodeIndex The node's index.
   @param demandIdx Index of the node's demand.
   @param demandName[out] Name of the category the demand belongs to.
   @return Error code.
   @see ENsetdemandname
   */
  int DLLEXPORT ENgetdemandname(int nodeIndex, int demandIdx, char *demandName);
  
  /**
   @brief Set a node's base demand for a demand category.
   @param nodeIndex The node's index.
   @param demandIndex The index of one of the node's demand categories.
   @param baseDemand The base demand for the selected category.
   @return Error code.
   @see ENgetbasedemand
   */
  int  DLLEXPORT ENsetbasedemand(int nodeIndex, int demandIndex,
                 EN_API_FLOAT_TYPE baseDemand);
  
   /**  
   @brief Set the time pattern assigned to a node's demand category. 
   @param nodeIndex The node's index.  
   @param demandIndex The index of one of the node's demand categories.  
   @param pattIndex The index of a time pattern applied to this demand category.  
   @return Error code 
   */ 
  int  DLLEXPORT ENsetdemandpattern(int nodeIndex, int demandIndex, int patIndex);
  
  /**
   @brief Retrieve the index of a curve given its name.
   @param id The ID name of a curve.
   @param[out] index The index of the named curve.
   @return Error code.
   @see ENgetcurveid
   */
  int  DLLEXPORT ENgetcurveindex(char *id, int *index);
  
  /**
   @brief Retrieve the ID name of a curve given its index.
   @param index The index of a curve.
   @param[out] id The ID of the specified curve.
   @return Error code.
   @see ENsetcurveindex
   
   NOTE: 'id' must be sized to hold MAXID characters.
   */
  int  DLLEXPORT ENgetcurveid(int index, char *id);
  
  /**
   @brief Retrieve the number of points in a curve.
   @param index The index of a curve.
   @param[out] len The number of data points assigned to the curve.
   @return Error code.
   @see ENgetcurvevalue
   */
  int  DLLEXPORT ENgetcurvelen(int index, int *len);
  
  /**
   @brief Retrieve an x,y data point for a curve.
   @param curveIndex The index of a curve.
   @param pointIndex The index of a point in the curve.
   @param[out] x The x-value of the specified point.
   @param[out] y The y-value of the specified point.
   @return Error code.
   @see ENgetcurvelen ENsetcurvevalue
   */
  int  DLLEXPORT ENgetcurvevalue(int curveIndex, int pointIndex,
                 EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);
  
  /**
   @brief Set the x and y values for a curve's data point.
   @param curveIndex The index of a curve.
   @param pointIndex The index of a point in the curve.
   @param x The x-value of the point.
   @param y The y-value of the point.
   @return Error code.
   */
  int  DLLEXPORT ENsetcurvevalue(int curveIndex, int pointIndex,
                 EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);
  
  /**
   @brief Set the x,y values for all points on a curve.
   @param index The index of a curve.
   @param x An array of x-values for the curve.
   @param y An array of y-values for the curve.
   @param len The length of the arrays for x and y.
   @return Error code.
   */
  int  DLLEXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y,
                 int len);
  
  /**
   @brief Add a new curve to the project.
   @param id The name of the curve to be added.
   @return Error code.
   @see ENgetcurveindex ENsetcurve
   */
  int  DLLEXPORT ENaddcurve(char *id);

  /**
   @brief Get summary information for a rule-based control.
   @param index The control's index.
   @param[out] nPremises Number of premises in the IF section of the control.
   @param[out] nThenActions Number of THEN actions in the control.
   @param nElseActions[out] Number of ELSE actions in the control.
   @param priority[out] Rule's priority.
   @return Error code.
   */
  int  DLLEXPORT ENgetrule(int index, int *nPremises, int *nTrueActions,
                 int *nFalseActions, EN_API_FLOAT_TYPE *priority);

  /**
   @brief Set the priority of a rule-based control.
   @param index The control's index.
   @param priority The priority assigned to the control.
   @return Error code.
   */
  int  DLLEXPORT ENsetrulepriority(int index, EN_API_FLOAT_TYPE priority);

  /**
   @brief Get the components of a premise in a rule-based control.
   @param ruleIndex The control's index.
   @param premiseIndex The premise's index.
   @param logop[out] Logical operator (IF/AND/OR) of the premise
   @param object[out] Type of object (e.g. TANK) the premise is looking at.
   @param objIndex[out] Index of the object (e.g. the index of the tank).
   @param variable[out] Index of the variable to be checked (e.g. LEVEL).
   @param relop[out] Relationship operator (e.g. ABOVE) in the premise.
   @param status[out] Status of the object being checked (e.g. CLOSED)
   @param value[out] Setting of the variable being checked (e.g. 5.5) 
   @return Error code.
   */
  int  DLLEXPORT ENgetpremise(int ruleIndex, int premiseIndex, int *logop,
                 int *object, int *objIndex, int *variable,
                 int *relop, int *status, EN_API_FLOAT_TYPE *value);

  /**
   @brief Set the components of a premise in a rule-based control.
   @param ruleIndex The control's index.
   @param premiseIndex The premise's index.
   @param logop Logical operator (IF/AND/OR) of the premise
   @param object Type of object (e.g. TANK) the premise is looking at.
   @param objIndex Index of the object (e.g. the index of the tank).
   @param variable Index of the variable to be checked (e.g. LEVEL).
   @param relop Relationship operator (e.g. ABOVE) in the premise.
   @param status Status of the object being checked (e.g. CLOSED)
   @param value Setting of the variable being checked (e.g. 5.5) 
   @return Error code.
   */
  int  DLLEXPORT ENsetpremise(int ruleIndex, int premiseIndex, int logop,
                 int object, int objIndex, int variable, int relop,
                 int status, EN_API_FLOAT_TYPE value);

  /**
   @brief Set the index of an object in a premise of a rule-based control.
   @param ruleIndex The control's index.
   @param premiseIndex The premise's index.
   @param objIndex The index of the premise's object (e.g. the index of the tank).
   @return Error code.
   */
  int  DLLEXPORT ENsetpremiseindex(int ruleIndex, int premiseIndex, int objIndex);

  /**
   @brief Set the status in a premise of a rule-based control.
   @param ruleIndex The control's index.
   @param premiseIndex The premise's index.
   @param status The status of the object being checked (e.g. CLOSED)
   @return Error code.
   */
  int  DLLEXPORT ENsetpremisestatus(int ruleIndex, int premiseIndex, int status);

  /**
   @brief Set the value in a premise of a rule-based control.
   @param ruleIndex The control's index.
   @param premiseIndex The premise's index.
   @param value The value of the premise's variable being checked (e.g. 5.5) 
   @return Error code.
   */
  int  DLLEXPORT ENsetpremisevalue(int ruleIndex, int premiseIndex,
                 EN_API_FLOAT_TYPE value);
  
  /**
   @brief Get the components of a THEN action in a rule-based control.
   @param ruleIndex The control's index.
   @param actionIndex Index of the THEN action to retrieve.
   @param linkIndex[out] Index of the link in the action (e.g. index of Pump 1)
   @param status[out] Status of the link (e.g. CLOSED)
   @param setting[out] Value of the link's setting (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENgetthenaction(int ruleIndex, int actionIndex, int *linkIndex,
                 int *status, EN_API_FLOAT_TYPE *setting);

  /**
   @brief Set the components of a THEN action in a rule-based control.
   @param ruleIndex The control's index.
   @param actionIndex Index of the THEN action to modify.
   @param linkIndex Index of the link in the action (e.g. index of Pump 1)
   @param status Status assigned to the link (e.g. CLOSED)
   @param setting Setting value assigned to the link (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENsetthenaction(int ruleIndex, int actionIndex, int linkIndex,
                 int status, EN_API_FLOAT_TYPE setting);
  
  /**
   @brief Get the components of an ELSE action in a rule-based control.
   @param ruleIndex The control's index.
   @param actionIndex Index of the ELSE action to retrieve.
   @param linkIndex[out] Index of the link in the action (e.g. index of Pump 1).
   @param status[out] Status of the link (e.g. CLOSED).
   @param setting[out] Value of the link's setting (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENgetelseaction(int ruleIndex, int actionIndex, int *linkIndex,
                 int *status, EN_API_FLOAT_TYPE *setting);

  /**
   @brief Set the components of an ELSE action in a rule-based control.
   @param ruleIndex The control's index.
   @param actionIndex Index of the ELSE action being modified.
   @param linkIndex Index of the link in the action (e.g. index of Pump 1)
   @param status Status assigned to the link (e.g. CLOSED)
   @param setting Setting value assigned to the link (e.g. pump speed 0.9)
   @return Error code.
   */
  int  DLLEXPORT ENsetelseaction(int ruleIndex, int actionIndex, int linkIndex,
                 int status, EN_API_FLOAT_TYPE setting);

  /**
   @brief Get the ID name of a rule-based control.
   @param index The rule's index.
   @param id[out] The rule's ID name.
   @return Error code.
   */
  int  DLLEXPORT ENgetruleID(int index, char* id);

  /**
   @brief Delete a rule-based control.
   @param index The rule's index.
   @return Error code.
   */
  int  DLLEXPORT ENdeleterule(int index);

  /**
   @brief Add a new node to the project.
   @param id The name of the node to be added.
   @param nodeType The type of node being added (see EN_NodeType)
   @return Error code.
   */
  int DLLEXPORT ENaddnode(char *id, EN_NodeType nodeType);
  
  /**
   @brief Add a new link to the project.
   @param id The name of the link to be added.
   @param linkType The type of link being added (see EN_LinkType)
   @param fromNode The id of the link's starting node
   @param toNode The id of the link's ending node
   @return Error code.
   */
  int DLLEXPORT ENaddlink(char *id, EN_LinkType linkType, char *fromNode, char *toNode);

  /**
   @brief Add a new control rule to the project.
   @param rule Text of the rule following the format used in an INP file.
   @return Error code.
   */
  int DLLEXPORT ENaddrule(char *rule);
  
  /**
   @brief Delete a node from the project.
   @param index The index of the node to be deleted.
   @param actionCode The action taken if any control contains the node and its links.
   @return Error code.

   If 'actionCode' is EN_UNCONDITIONAL then the node, its incident links and all
   simple and rule-based controls that contain them are deleted. If set to
   EN_CONDITIONAL then the node is not deleted if it or its incident links appear
   in any controls and an error code is returned.

   */
  int DLLEXPORT ENdeletenode(int index, int actionCode);
  
  /**
   @brief Delete a link from the project.
   @param index The index of the link to be deleted.
   @param ctrlsCode The action taken if any control contains the link.
   @return Error code.

   If 'actionCode' is EN_UNCONDITIONAL then the link an all simple and rule-based
   controls that contain it are deleted. If set to EN_CONDITIONAL then the link
   is not deleted if it appears in any control and an error code is returned.

   */
  int DLLEXPORT ENdeletelink(int index, int actionCode);
  
  
  /***************************************************
   
   Threadsafe versions of all epanet functions
   
   ***************************************************/
  int DLLEXPORT EN_createproject(EN_Project *ph);
  int DLLEXPORT EN_deleteproject(EN_Project *ph);

  int DLLEXPORT EN_runproject(EN_Project ph, const char *f1, 
                const char *f2, const char *f3, void (*pviewprog)(char *));

  void DLLEXPORT EN_clearError(EN_Project ph);
  int DLLEXPORT EN_checkError(EN_Project ph, char** msg_buffer);

  int DLLEXPORT EN_init(EN_Project ph, const char *rptFile,
                const char *binOutFile, EN_FlowUnits unitsType,
                EN_HeadLossType headLossType);

  int DLLEXPORT EN_open(EN_Project ph, const char *inpFile,
                const char *rptFile, const char *binOutFile);

  int DLLEXPORT EN_saveinpfile(EN_Project ph, const char *filename);

  int DLLEXPORT EN_close(EN_Project ph);
  int DLLEXPORT EN_solveH(EN_Project ph);

  int DLLEXPORT EN_saveH(EN_Project ph);
  int DLLEXPORT EN_openH(EN_Project ph);
  int DLLEXPORT EN_initH(EN_Project ph, int saveFlag);
  int DLLEXPORT EN_runH(EN_Project ph, long *currentTime);
  int DLLEXPORT EN_nextH(EN_Project ph, long *tStep);
  int DLLEXPORT EN_closeH(EN_Project ph);
  int DLLEXPORT EN_savehydfile(EN_Project ph, char *filename);
  int DLLEXPORT EN_usehydfile(EN_Project ph, char *filename);

  int DLLEXPORT EN_solveQ(EN_Project ph);
  int DLLEXPORT EN_openQ(EN_Project ph);
  int DLLEXPORT EN_initQ(EN_Project ph, int saveFlag);
  int DLLEXPORT EN_runQ(EN_Project ph, long *currentTime);
  int DLLEXPORT EN_nextQ(EN_Project ph, long *tStep);
  int DLLEXPORT EN_stepQ(EN_Project ph, long *timeLeft);
  int DLLEXPORT EN_closeQ(EN_Project ph);
  int DLLEXPORT EN_writeline(EN_Project ph, char *line);

  int DLLEXPORT EN_report(EN_Project ph);
  int DLLEXPORT EN_resetreport(EN_Project ph);
  int DLLEXPORT EN_setreport(EN_Project ph, char *reportCommand);

  int DLLEXPORT EN_getcontrol(EN_Project ph, int controlIndex,
                int *controlType, int *linkIndex, EN_API_FLOAT_TYPE *setting,
                int *nodeIndex, EN_API_FLOAT_TYPE *level);
  int DLLEXPORT EN_getcount(EN_Project ph, EN_CountType code, int *count);
  int DLLEXPORT EN_getoption(EN_Project ph, EN_Option opt,
                EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_gettimeparam(EN_Project ph, int code, long *value);
  int DLLEXPORT EN_getflowunits(EN_Project ph, int *code);
  int DLLEXPORT EN_setflowunits(EN_Project ph, int code);
  int DLLEXPORT EN_getpatternindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getpatternid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_getpatternlen(EN_Project ph, int index, int *len);
  int DLLEXPORT EN_getpatternvalue(EN_Project ph, int index, int period,
                EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_getaveragepatternvalue(EN_Project ph, int index,
                EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_getqualtype(EN_Project ph, int *qualcode, int *tracenode);
  int DLLEXPORT EN_geterror(int errcode, char *errmsg, int maxLen);

  int DLLEXPORT EN_getstatistic(EN_Project ph, int code,
                EN_API_FLOAT_TYPE* value);
  int DLLEXPORT EN_getnodeindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getnodeid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_getnodetype(EN_Project ph, int index, int *code);
  int DLLEXPORT EN_getnodevalue(EN_Project ph, int index, int code,
                EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_getcoord(EN_Project ph, int index, EN_API_FLOAT_TYPE *x,
                EN_API_FLOAT_TYPE *y);
  int DLLEXPORT EN_setcoord(EN_Project ph, int index, EN_API_FLOAT_TYPE x,
                EN_API_FLOAT_TYPE y);
  int DLLEXPORT EN_getnumdemands(EN_Project ph, int nodeIndex,
                int *numDemands);
  int DLLEXPORT EN_getbasedemand(EN_Project ph, int nodeIndex,
                int demandIndex, EN_API_FLOAT_TYPE *baseDemand);
  int DLLEXPORT EN_getdemandpattern(EN_Project ph, int nodeIndex,
                int demandIndex, int *pattIndex);
  int DLLEXPORT EN_getlinkindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getlinkid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_getlinktype(EN_Project ph, int index, EN_LinkType *code);
  int DLLEXPORT EN_getlinknodes(EN_Project ph, int index, int *node1,
                int *node2);
  int DLLEXPORT EN_getlinkvalue(EN_Project ph, int index,
                EN_LinkProperty code, EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_getcurve(EN_Project ph, int curveIndex, char* id,
                int *nValues, EN_API_FLOAT_TYPE **xValues,
                EN_API_FLOAT_TYPE **yValues);
  int DLLEXPORT EN_getheadcurveindex(EN_Project ph, int pumpIndex,
                int *curveIndex);
  int DLLEXPORT EN_setheadcurveindex(EN_Project ph, int pumpIndex,
                int curveIndex);
  int DLLEXPORT EN_getpumptype(EN_Project ph, int linkIndex, int *outType);
  int DLLEXPORT EN_getcurvetype(EN_Project ph, int curveIndex, int *outType);

  int DLLEXPORT EN_getversion(int *version);

  int DLLEXPORT EN_addcontrol(EN_Project ph, int *cindex, int ctype,
                int lindex, EN_API_FLOAT_TYPE setting, int nindex,
                EN_API_FLOAT_TYPE level);
  int DLLEXPORT EN_setcontrol(EN_Project ph, int cindex, int ctype,
                int lindex, EN_API_FLOAT_TYPE setting, int nindex,
                EN_API_FLOAT_TYPE level);
  int DLLEXPORT EN_setnodeid(EN_Project ph, int index, char *newid);
  int DLLEXPORT EN_setnodevalue(EN_Project ph, int index, int code,
                EN_API_FLOAT_TYPE v);
  int DLLEXPORT EN_setlinkid(EN_Project ph, int index, char *newid);
  int DLLEXPORT EN_setlinknodes(EN_Project ph, int index, int node1,
                int node2);
  int DLLEXPORT EN_setlinktype(EN_Project ph, int *index, EN_LinkType type,
                int actionCode);
  int DLLEXPORT EN_setlinkvalue(EN_Project ph, int index, int code,
                EN_API_FLOAT_TYPE v);
  int DLLEXPORT EN_addpattern(EN_Project ph, char *id);
  int DLLEXPORT EN_setpattern(EN_Project ph, int index,
                EN_API_FLOAT_TYPE *f, int len);
  int DLLEXPORT EN_setpatternvalue(EN_Project ph, int index, int period,
                EN_API_FLOAT_TYPE value);
  int DLLEXPORT EN_settimeparam(EN_Project ph, int code, long value);
  int DLLEXPORT EN_setoption(EN_Project ph, int code, EN_API_FLOAT_TYPE v);
  int DLLEXPORT EN_setstatusreport(EN_Project ph, int code);
  int DLLEXPORT EN_setqualtype(EN_Project ph, int qualcode, char *chemname,
                char *chemunits, char *tracenode);

  int DLLEXPORT EN_getdemandmodel(EN_Project ph, int *type, EN_API_FLOAT_TYPE *pmin,
                EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp);
  int DLLEXPORT EN_setdemandmodel(EN_Project ph, int type, EN_API_FLOAT_TYPE pmin,
                EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp);

  int DLLEXPORT EN_setdemandname(EN_Project ph, int nodeIndex,
                int demandIdx, char *demandName);
  int DLLEXPORT EN_getdemandname(EN_Project ph, int nodeIndex,
                int demandIdx, char *demandName);
  int DLLEXPORT EN_getqualinfo(EN_Project ph, int *qualcode,
                char *chemname, char *chemunits, int *tracenode);
  int DLLEXPORT EN_setbasedemand(EN_Project ph, int nodeIndex,
                int demandIndex, EN_API_FLOAT_TYPE baseDemand);
  int DLLEXPORT EN_setdemandpattern(EN_Project ph, int nodeIndex,
                int demandIndex, int patIndex);
  int DLLEXPORT EN_getcurveindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getcurveid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_getcurvelen(EN_Project ph, int index, int *len);
  int DLLEXPORT EN_getcurvevalue(EN_Project ph, int curveIndex,
                int pointIndex, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);
  int DLLEXPORT EN_setcurvevalue(EN_Project ph, int curveIndex,
                int pointIndex, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);
  int DLLEXPORT EN_setcurve(EN_Project ph, int index, EN_API_FLOAT_TYPE *x,
                EN_API_FLOAT_TYPE *y, int len);
  int DLLEXPORT EN_addcurve(EN_Project ph, char *id);
  int DLLEXPORT EN_getrule(EN_Project ph, int index, int *nPremises,
                int *nThenActions, int *nElseActions, EN_API_FLOAT_TYPE *priority);
  int DLLEXPORT EN_setrulepriority(EN_Project ph, int index,
                EN_API_FLOAT_TYPE priority);
  int DLLEXPORT EN_getpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                int *logop, int *object, int *objIndex, int *variable, int *relop,
                int *status, EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_setpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                int logop, int object, int objIndex, int variable, int relop,
                int status, EN_API_FLOAT_TYPE value);
  int DLLEXPORT EN_setpremiseindex(EN_Project ph, int ruleIndex,
                int premiseIndex, int objIndex);
  int DLLEXPORT EN_setpremisestatus(EN_Project ph, int ruleIndex,
                int premiseIndex, int status);
  int DLLEXPORT EN_setpremisevalue(EN_Project ph, int ruleIndex,
                int premiseIndex, EN_API_FLOAT_TYPE value);
  int DLLEXPORT EN_getthenaction(EN_Project ph, int ruleIndex,
                int actionIndex, int *linkIndex, int *status,
                EN_API_FLOAT_TYPE *setting);
  int DLLEXPORT EN_setthenaction(EN_Project ph, int ruleIndex,
                int actionIndex, int linkIndex, int status,
                EN_API_FLOAT_TYPE setting);
  int DLLEXPORT EN_getelseaction(EN_Project ph, int ruleIndex,
                int actionIndex, int *linkIndex, int *status,
                EN_API_FLOAT_TYPE *setting);
  int DLLEXPORT EN_setelseaction(EN_Project ph, int ruleIndex,
                int actionIndex, int linkIndex, int status,
                EN_API_FLOAT_TYPE setting);
  int DLLEXPORT EN_getruleID(EN_Project ph, int index, char* id);
  int DLLEXPORT EN_addnode(EN_Project ph, char *id, EN_NodeType nodeType);
  int DLLEXPORT EN_addlink(EN_Project ph, char *id, EN_LinkType linkType,
                char *fromNode, char *toNode);
  int DLLEXPORT EN_addrule(EN_Project ph, char *rule);
  int DLLEXPORT EN_deletenode(EN_Project ph, int index, int actionCode);
  int DLLEXPORT EN_deletelink(EN_Project ph, int index, int actionCode);
  int DLLEXPORT EN_deletecontrol(EN_Project ph, int index);
  int DLLEXPORT EN_deleterule(EN_Project ph, int index);
  
#if defined(__cplusplus)
}
#endif

#endif //EPANET2_H
