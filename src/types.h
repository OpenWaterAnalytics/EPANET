/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       types.h
 Description:  symbolic constants and data types used throughout EPANET
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 10/29/2019
 ******************************************************************************
*/

#ifndef TYPES_H
#define TYPES_H

#include <stdio.h>

#include "hash.h"

/*
-------------------------------------------
   Definition of 4-byte integers & reals
-------------------------------------------
*/
typedef  float        REAL4;
typedef  int          INT4;

/*
----------------------------------------------
   Various constants
----------------------------------------------
*/
#define   CODEVERSION        20200
#define   MAGICNUMBER        516114521
#define   ENGINE_VERSION     201   // Used for binary hydraulics file
#define   EOFMARK            0x1A  // Use 0x04 for UNIX systems
#define   MAXTITLE  3        // Max. # title lines
#define   TITLELEN  79       // Max. # characters in a title line
#define   MAXID     31       // Max. # characters in ID name
#define   MAXMSG    255      // Max. # characters in message text
#define   MAXLINE   1024     // Max. # characters read from input line
#define   MAXFNAME  259      // Max. # characters in file name
#define   MAXTOKS   40       // Max. items per line of input
#define   TRUE      1
#define   FALSE     0
#define   FULL      2
#define   BIG       1.E10
#define   TINY      1.E-6
#define   MISSING   -1.E10     // Missing value indicator
#define   DIFFUS    1.3E-8     // Diffusivity of chlorine
                               // @ 20 deg C (sq ft/sec)
#define   VISCOS    1.1E-5     // Kinematic viscosity of water
                               // @ 20 deg C (sq ft/sec)
#define   MINPDIFF  0.1        // PDA min. pressure difference (psi or m)
#define   SEPSTR    " \t\n\r"  // Token separator characters
#ifdef M_PI
  #define   PI        M_PI
#else
  #define   PI        3.141592654
#endif

/*
----------------------------------------------
   Flow units conversion factors
----------------------------------------------
*/
#define   GPMperCFS   448.831
#define   AFDperCFS   1.9837
#define   MGDperCFS   0.64632
#define   IMGDperCFS  0.5382
#define   LPSperCFS   28.317
#define   LPMperCFS   1699.0
#define   CMHperCFS   101.94
#define   CMDperCFS   2446.6
#define   MLDperCFS   2.4466
#define   M3perFT3    0.028317
#define   LperFT3     28.317
#define   MperFT      0.3048
#define   PSIperFT    0.4333
#define   KPAperPSI   6.895
#define   KWperHP     0.7457
#define   SECperDAY   86400

/*
---------------------------------------------------------------------
   Macros to test for successful allocation of memory and to free it
---------------------------------------------------------------------
*/
#define  MEMCHECK(x)  (((x) == NULL) ? 101 : 0 )
#define  FREE(x) do { free(x); (x) = NULL; } while(0)

/*
---------------------------------------------------------------------
   Conversion macros to be used in place of functions
---------------------------------------------------------------------
*/
#define INT(x)   ((int)(x))                   // integer portion of x
#define FRAC(x)  ((x)-(int)(x))               // fractional part of x
#define ABS(x)   (((x)<0) ? -(x) : (x))       // absolute value of x
#define MIN(x,y) (((x)<=(y)) ? (x) : (y))     // minimum of x and y
#define MAX(x,y) (((x)>=(y)) ? (x) : (y))     // maximum of x and y
#define ROUND(x) (((x)>=0) ? (int)((x)+.5) : (int)((x)-.5))
                                              // round-off of x
#define MOD(x,y) ((x)%(y))                    // x modulus y
#define SQR(x)   ((x)*(x))                    // x-squared
#define SGN(x)   (((x)<0) ? (-1) : (1))       // sign of x
#define UCHAR(x) (((x) >= 'a' && (x) <= 'z') ? ((x)&~32) : (x))
                                              // uppercase char of x
/*
------------------------------------------------------
   Macro to evaluate function x with error checking
   (Fatal errors are numbered higher than 100)
------------------------------------------------------
*/
#define ERRCODE(x) (errcode = ((errcode>100) ? (errcode) : (x)))

/*
----------------------------------------------
   Enumerated Data Types
----------------------------------------------
*/

typedef enum {
  NODE,
  LINK,
  TIMEPAT,
  CURVE,
  CONTROL,
  RULE
} ObjectType;

typedef enum {
  JUNCTION,
  RESERVOIR,
  TANK
} NodeType;

typedef enum {
  CVPIPE,        // pipe with check valve
  PIPE,          // pipe
  PUMP,          // pump
  PRV,           // pressure reducing valve
  PSV,           // pressure sustaining valve
  PBV,           // pressure breaker valve
  FCV,           // flow control valve
  TCV,           // throttle control valve
  GPV            // general purpose valve
} LinkType;

typedef enum {
  USE,           // use hydraulics file from previous run
  SAVE,          // save hydraulics file after current run
  SCRATCH        // use temporary hydraulics file
} HydFiletype;

typedef enum {
  NONE,          // no quality analysis
  CHEM,          // analyze a chemical
  AGE,           // analyze water age
  TRACE          // trace % of flow from a source
} QualType;

typedef enum {
  VOLUME_CURVE,  // volume curve
  PUMP_CURVE,    // pump curve
  EFFIC_CURVE,   // efficiency curve
  HLOSS_CURVE,   // head loss curve
  GENERIC_CURVE  // generic curve
} CurveType;

typedef enum {
  CONST_HP,      // constant horsepower
  POWER_FUNC,    // power function
  CUSTOM,        // user-defined custom curve
  NOCURVE
} PumpType;

typedef enum {
  CONCEN,        // inflow concentration
  MASS,          // mass inflow booster
  SETPOINT,      // setpoint booster
  FLOWPACED      // flow paced booster
} SourceType;

typedef enum {
  LOWLEVEL,      // act when grade below set level
  HILEVEL,       // act when grade above set level
  TIMER,         // act when set time reached
  TIMEOFDAY      // act when time of day occurs
} ControlType;

typedef enum {
  XHEAD,         // pump cannot deliver head (closed)
  TEMPCLOSED,    // temporarily closed
  CLOSED,        // closed
  OPEN,          // open
  ACTIVE,        // valve active (partially open)
  XFLOW,         // pump exceeds maximum flow
  XFCV,          // FCV cannot supply flow
  XPRESSURE,     // valve cannot supply pressure
  FILLING,       // tank filling
  EMPTYING,      // tank emptying
  OVERFLOWING    // tank overflowing
} StatusType;

typedef enum {
  HW,            // Hazen-Williams
  DW,            // Darcy-Weisbach
  CM             // Chezy-Manning
} HeadLossType;

typedef enum {
  US,            // US
  SI             // SI (metric)
} UnitsType;

typedef enum {
  CFS,           // cubic feet per second
  GPM,           // gallons per minute
  MGD,           // million gallons per day
  IMGD,          // imperial million gal. per day
  AFD,           // acre-feet per day
  LPS,           // liters per second
  LPM,           // liters per minute
  MLD,           // megaliters per day
  CMH,           // cubic meters per hour
  CMD            // cubic meters per day
} FlowUnitsType;

typedef enum {
  PSI,           // pounds per square inch
  KPA,           // kiloPascals
  METERS         // meters
} PressureUnitsType;

typedef enum {
  LOW,           // lower limit
  HI,            // upper limit
  PREC           // precision
} RangeType;

typedef enum {
  MIX1,          // complete mix model
  MIX2,          // 2-compartment model
  FIFO,          // first in, first out model
  LIFO           // last in, first out model
} MixType;

typedef enum {
  SERIES,        // point time series
  AVG,           // time-averages
  MIN,           // minimum values
  MAX,           // maximum values
  RANGE          // max - min values
} StatisticType;

typedef enum {
  ELEV = 0,      // nodal elevation
  DEMAND,        // nodal demand flow
  HEAD,          // nodal hydraulic head
  PRESSURE,      // nodal pressure
  QUALITY,       // nodal water quality

  LENGTH,        // link length
  DIAM,          // link diameter
  FLOW,          // link flow rate
  VELOCITY,      // link flow velocity
  HEADLOSS,      // link head loss
  LINKQUAL,      // avg. water quality in link
  STATUS,        // link status
  SETTING,       // pump/valve setting
  REACTRATE,     // avg. reaction rate in link
  FRICTION,      // link friction factor

  POWER,         // pump power output
  TIME,          // simulation time
  VOLUME,        // tank volume
  CLOCKTIME,     // simulation time of day
  FILLTIME,      // time to fill a tank
  DRAINTIME,     // time to drain a tank
  MAXVAR         // total number of variable fields
} FieldType;

typedef enum {
  _TITLE, _JUNCTIONS, _RESERVOIRS, _TANKS, _PIPES, _PUMPS,
  _VALVES, _CONTROLS, _RULES, _DEMANDS, _SOURCES, _EMITTERS,
  _PATTERNS, _CURVES, _QUALITY, _STATUS, _ROUGHNESS, _ENERGY,
  _REACTIONS, _MIXING, _REPORT, _TIMES, _OPTIONS,
    _COORDS, _VERTICES, _LABELS, _BACKDROP, _TAGS, _END
} SectionType;

typedef enum {
  STATHDR,       // hydraulic status header
  ENERHDR,       // energy usage header
  NODEHDR,       // node results header
  LINKHDR        // link results header
} HdrType;

typedef enum {
  NEGATIVE  = -1,  // flow in reverse of pre-assigned direction
  ZERO_FLOW = 0,   // zero flow
  POSITIVE  = 1    // flow in pre-assigned direction
} FlowDirection;

typedef enum {
  DDA,           // demand driven analysis
  PDA            // pressure driven analysis
} DemandModelType;

/*
------------------------------------------------------
   Fundamental Data Structures
------------------------------------------------------
*/

struct IDstring            // Holds component ID label
{
  char ID[MAXID+1];
};

typedef struct             // Time Pattern Object
{
  char   ID[MAXID+1];      // pattern ID
  char   *Comment;         // pattern comment
  int    Length;           // pattern length
  double *F;               // pattern factors
} Spattern;

typedef struct             // Curve Object
{
  char      ID[MAXID+1];   // curve ID
  char      *Comment;      // curve comment
  CurveType Type;          // curve type
  int       Npts;          // number of points
  int       Capacity;      // size of X & Y arrays
  double    *X;            // x-values
  double    *Y;            // y-values
} Scurve;

struct Sdemand             // Demand List Item
{
  double Base;             // baseline demand
  int    Pat;              // pattern index
  char   *Name;            // demand category name
  struct Sdemand *next;    // next demand list item
};
typedef struct Sdemand *Pdemand; // Pointer to demand list

typedef struct             // Energy Usage Object
{
  double TimeOnLine;       // hours pump is online
  double Efficiency;       // total time wtd. efficiency
  double KwHrsPerFlow;     // total kw-hrs per unit of flow
  double KwHrs;            // total kw-hrs consumed
  double MaxKwatts;        // max. kw consumed
  double TotalCost;        // total pumping cost
} Senergy;

struct Ssource             // Water Quality Source Object
{
    double     C0;         // base concentration/mass
    int        Pat;        // pattern index
    double     Smass;      // actual mass flow rate
    SourceType Type;       // type of source
};
typedef struct Ssource *Psource; // Pointer to source object

struct Svertices           // Coordinates of a link's vertices
{
  double *X;               // array of x-coordinates
  double *Y;               // array of y-coordinates
  int    Npts;             // number of vertex points
  int    Capacity;         // capacity of coordinate arrays
};
typedef struct Svertices *Pvertices; // Pointer to a link's vertices

typedef struct             // Node Object
{
  char     ID[MAXID+1];    // node ID
  double   X;              // x-coordinate
  double   Y;              // y-coordinate
  double   El;             // elevation
  Pdemand  D;              // demand pointer
  Psource  S;              // source pointer
  double   C0;             // initial quality
  double   Ke;             // emitter coeff.
  int      Rpt;            // reporting flag
  int      ResultIndex;    // saved result index
  NodeType Type;           // node type
  char     *Comment;       // node comment
} Snode;

typedef struct             // Link Object
{
  char     ID[MAXID+1];    // link ID
  int      N1;             // start node index
  int      N2;             // end node index
  double   Diam;           // diameter
  double   Len;            // length
  double   Kc;             // roughness
  double   Km;             // minor loss coeff.
  double   Kb;             // bulk react. coeff.
  double   Kw;             // wall react. coef.
  double   R;              // flow resistance
  double   Rc;             // reaction coeff.
  LinkType Type;           // link type
  StatusType Status;       // initial status
  Pvertices  Vertices;     // internal vertex coordinates
  int      Rpt;            // reporting flag
  int      ResultIndex;    // saved result index
  char     *Comment;       // link comment
} Slink;

typedef struct             // Tank Object
{
  int     Node;            // node index of tank
  double  A;               // tank area
  double  Hmin;            // minimum water elev
  double  Hmax;            // maximum water elev
  double  H0;              // initial water elev
  double  Vmin;            // minimum volume
  double  Vmax;            // maximum volume
  double  V0;              // initial volume
  double  Kb;              // bulk reaction coeff.
  double  V;               // tank volume
  double  C;               // concentration
  int     Pat;             // fixed grade time pattern
  int     Vcurve;          // volume v. elev. curve index
  MixType MixModel;        // type of mixing model
  double  V1max;           // mixing compartment size
  int     CanOverflow;     // tank can overflow or not
} Stank;

typedef struct             // Pump Object
{
  int     Link;            // link index of pump
  int     Ptype;           // pump curve type
  double  Q0;              // initial flow
  double  Qmax;            // maximum flow
  double  Hmax;            // maximum head
  double  H0;              // shutoff head
  double  R;               // flow coeffic.
  double  N;               // flow exponent
  int     Hcurve;          // head v. flow curve index
  int     Ecurve;          // effic. v. flow curve index
  int     Upat;            // utilization pattern index
  int     Epat;            // energy cost pattern index
  double  Ecost;           // unit energy cost
  Senergy Energy;          // energy usage statistics
} Spump;

typedef struct             // Valve Object
{
  int Link;                // link index of valve
} Svalve;

typedef struct             // Control Statement
{
    int         Link;      // link index
    int         Node;      // control node index
    long        Time;      // control time
    double      Grade;     // control grade
    double      Setting;   // new link setting
    StatusType  Status;    // new link status
    ControlType Type;      // control type
} Scontrol;

typedef struct             // Field Object of Report Table
{
    char   Name[MAXID+1];  // name of reported variable
    char   Units[MAXID+1]; // units of reported variable
    int    Enabled;        // enabled if in table
    int    Precision;      // number of decimal places
    double RptLim[2];      // lower/upper report limits
} SField;

struct Sadjlist            // Node Adjacency List Item
{
    int    node;           // index of connecting node
    int    link;           // index of connecting link
    struct Sadjlist *next; // next item in list
};
typedef struct Sadjlist *Padjlist; // Pointer to adjacency list

struct  Sseg               // Pipe Segment List Item
{
    double  v;             // segment volume
    double  c;             // segment water quality
    struct  Sseg *prev;    // previous segment in list
};
typedef struct Sseg *Pseg; // Pointer to pipe segment list

typedef struct s_Premise       // Rule Premise Clause
{
    int      logop;            // logical operator (IF, AND, OR)
    int      object;           // NODE or LINK
    int      index;            // object's index
    int      variable;         // pressure, flow, etc.
    int      relop;            // relational operator (=, >, <, etc.)
    int      status;           // variable's status (OPEN, CLOSED)
    double   value;            // variable's value
    struct   s_Premise *next;  // next premise clause
} Spremise;

typedef struct s_Action        // Rule Action Clause
{
    int     link;              // link index
    int     status;            // link's status
    double  setting;           // link's setting
    struct  s_Action *next;
} Saction;

typedef struct                 // Control Rule Structure
{
    char     label[MAXID+1];   // rule label
    double   priority;         // priority level
    Spremise *Premises;        // list of premises
    Saction  *ThenActions;     // list of THEN actions
    Saction  *ElseActions;     // list of ELSE actions
} Srule;

typedef struct s_ActionItem    // Action List Item
{
    int     ruleIndex;           // index of rule action belongs to
    Saction *action;             // an action clause
    struct  s_ActionItem *next;  // next action on the list
} SactionList;

typedef struct                 // Mass Balance Components
{
    double    initial;         // initial mass in system
    double    inflow;          // mass inflow to system
    double    outflow;         // mass outflow from system
    double    reacted;         // mass reacted in system
    double    final;           // final mass in system
    double    ratio;           // ratio of mass added to mass lost
} SmassBalance;

/*
------------------------------------------------------
  Wrapper Data Structures
------------------------------------------------------
*/

// Input File Parser Wrapper
typedef struct {
  FILE *InFile;            // Input file handle

  char
      DefPatID[MAXID + 1],     // Default demand pattern ID
      InpFname[MAXFNAME + 1],  // Input file name
      *Tok[MAXTOKS],           // Array of token strings
      Comment[MAXMSG + 1],     // Comment text
      LineComment[MAXMSG + 1]; // Full line comment

  int
    MaxNodes,              // Node count   from input file */
    MaxLinks,              // Link count    "    "    "
    MaxJuncs,              // Junction count "   "    "
    MaxPipes,              // Pipe count    "    "    "
    MaxTanks,              // Tank count    "    "    "
    MaxPumps,              // Pump count    "    "    "
    MaxValves,             // Valve count   "    "    "
    MaxControls,           // Control count "   "     "
    MaxRules,              // Rule count    "   "     "
    MaxPats,               // Pattern count "   "     "
    MaxCurves,             // Curve count   "   "     "
    Ntokens,               // Number of tokens in line of input
    Ntitle,                // Number of title lines
    ErrTok,                // Index of error-producing token
    Unitsflag,             // Unit system flag
    Flowflag,              // Flow units flag
    Pressflag,             // Pressure units flag
    DefPat;                // Default demand pattern

  Spattern *PrevPat;       // Previous pattern processed
  Scurve   *PrevCurve;     // Previous curve processed
  double *X;               // Temporary array for curve data

} Parser;

// Time Step Wrapper
typedef struct {

  long
    Tstart,                // Starting time of day
    Hstep,                 // Nominal hyd. time step
    Pstep,                 // Time pattern time step
    Pstart,                // Starting pattern time
    Rstep,                 // Reporting time step
    Rstart,                // Time when reporting starts
    Rtime,                 // Next reporting time
    Htime,                 // Current hyd. time
    Hydstep,               // Actual hydraulic time step
    Qstep,                 // Quality time step
    Qtime,                 // Current quality time
    Rulestep,              // Rule evaluation time step
    Dur;                   // Duration of simulation

} Times;

// Reporting Wrapper
typedef struct {

  FILE *RptFile;           // Report file handle

  int
    Nperiods,              // Number of reporting periods
    PageSize,              // Lines/page in output report/
    Rptflag,               // Report flag
    Tstatflag,             // Report time series statistic flag
    Summaryflag,           // Report summary flag
    Messageflag,           // Error/warning message flag
    Statflag,              // Status report flag
    Energyflag,            // Energy report flag
    Nodeflag,              // Node report flag
    Linkflag,              // Link report flag
    Fprinterr;             // File write error flag

  long
    LineNum,               // Current line number
    PageNum;               // Current page number

  char
    Atime[13],             // Clock time (hrs:min:sec)
    Rpt1Fname[MAXFNAME+1], // Primary report file name
    Rpt2Fname[MAXFNAME+1], // Secondary report file name
    DateStamp[26];         // Current date & time

  SField   Field[MAXVAR];  // Output reporting fields

} Report;

// Output File Wrapper
typedef struct {

  char
    HydFname[MAXFNAME+1],  // Hydraulics file name
    OutFname[MAXFNAME+1];  // Binary output file name

  int
    Outflag,               // Output file flag
    Hydflag,               // Hydraulics flag
    SaveHflag,             // Hydraulic results saved flag
    SaveQflag,             // Quality results saved flag
    Saveflag;              // General purpose save flag

  long
    HydOffset,             // Hydraulics file byte offset
    OutOffset1,            // 1st output file byte offset
    OutOffset2;            // 2nd output file byte offset

  FILE
    *OutFile,              // Output file handle
    *HydFile,              // Hydraulics file handle
    *TmpOutFile;           // Temporary file handle

} Outfile;

// Rule-Based Controls Wrapper
typedef struct {

    SactionList *ActionList;     // Linked list of action items
    int         RuleState;       // State of rule interpreter
    int         Errcode;         // Rule parser error code
    long        Time1;           // Start of rule evaluation time interval
    Spremise    *LastPremise;    // Previous premise clause
    Saction     *LastThenAction; // Previous THEN action
    Saction     *LastElseAction; // Previous ELSE action

} Rules;

// Sparse Matrix Wrapper
typedef struct {

  double
    *Aii,        // Diagonal matrix coeffs.
    *Aij,        // Non-zero, off-diagonal matrix coeffs.
    *F,          // Right hand side vector
    *temp;       // Array used by linear eqn. solver

  int
    Ncoeffs,     // Number of non-zero matrix coeffs
    *Order,      // Node-to-row of re-ordered matrix
    *Row,        // Row-to-node of re-ordered matrix
    *Ndx,        // Index of link's coeff. in Aij
    *XLNZ,       // Start position of each column in NZSUB
    *NZSUB,      // Row index of each coeff. in each column
    *LNZ,        // Position of each coeff. in Aij array
    *Degree,     // Number of links adjacent to each node
    *link,       // Array used by linear eqn. solver
    *first;      // Array used by linear eqn. solver

} Smatrix;

// Hydraulics Solver Wrapper
typedef struct {

  double
    *NodeHead,             // Node hydraulic heads
    *NodeDemand,           // Node demand + emitter flows
    *DemandFlow,           // Work array of demand flows
    *EmitterFlow,          // Emitter outflows
    *LinkFlow,             // Link flows
    *LinkSetting,          // Link settings
    Htol,                  // Hydraulic head tolerance
    Qtol,                  // Flow rate tolerance
    RQtol,                 // Flow resistance tolerance
    Hexp,                  // Exponent in headloss formula
    Qexp,                  // Exponent in emitter formula
    Pexp,                  // Exponent in demand formula
    Pmin,                  // Pressure needed for any demand
    Preq,                  // Pressure needed for full demand
    Dmult,                 // Demand multiplier
    Hacc,                  // Relative flow change limit
    FlowChangeLimit,       // Absolute flow change limit
    HeadErrorLimit,        // Hydraulic head error limit
    DampLimit,             // Solution damping threshold
    Viscos,                // Kin. viscosity (sq ft/sec)
    SpGrav,                // Specific gravity
    Epump,                 // Global pump efficiency
    Dsystem,               // Total system demand
    Ecost,                 // Base energy cost per kwh
    Dcost,                 // Energy demand charge/kw/day
    Emax,                  // Peak energy usage
    RelativeError,         // Total flow change / total flow
    MaxHeadError,          // Max. error for link head loss
    MaxFlowChange,         // Max. change in link flow
    DemandReduction,       // % demand reduction at pressure deficient nodes
    RelaxFactor,           // Relaxation factor for flow updating
    *P,                    // Inverse of head loss derivatives
    *Y,                    // Flow correction factors
    *Xflow;                // Inflow - outflow at each node

  int
    Epat,                  // Energy cost time pattern
    DemandModel,           // Fixed or pressure dependent
    Formflag,              // Head loss formula flag
    Iterations,            // Number of hydraulic trials taken
    MaxIter,               // Max. hydraulic trials allowed
    ExtraIter,             // Extra hydraulic trials
    CheckFreq,             // Hydraulic trials between status checks
    MaxCheck,              // Hydraulic trials limit on status checks
    OpenHflag,             // Hydraulic system opened flag
    Haltflag,              // Flag to halt simulation
    DeficientNodes;        // Number of pressure deficient nodes

  StatusType
    *LinkStatus,           // Link status
    *OldStatus;            // Previous link/tank status

  Smatrix smatrix;         // Sparse matrix storage

} Hydraul;

// Forward declaration of the Mempool structure defined in mempool.h
struct Mempool;

// Water Quality Solver Wrapper
typedef struct {

  int
    Qualflag,              // Water quality analysis flag
    OpenQflag,             // Quality system opened flag
    Reactflag,             // Reaction indicator
    OutOfMemory,           // Out of memory indicator
    TraceNode,             // Source node for flow tracing
    *SortedNodes;          // Topologically sorted node indexes

  char
    ChemName[MAXID + 1],   // Name of chemical
    ChemUnits[MAXID + 1];  // Units of chemical

  double
    Ctol,                  // Water quality tolerance
    Diffus,                // Diffusivity (sq ft/sec)
    Wbulk,                 // Avg. bulk reaction rate
    Wwall,                 // Avg. wall reaction rate
    Wtank,                 // Avg. tank reaction rate
    Wsource,               // Avg. mass inflow
    Rfactor,               // Roughness-reaction factor
    Sc,                    // Schmidt Number
    Bucf,                  // Bulk reaction units conversion factor
    Tucf,                  // Tank reaction units conversion factor
    BulkOrder,             // Bulk flow reaction order
    WallOrder,             // Pipe wall reaction order
    TankOrder,             // Tank reaction order
    Kbulk,                 // Global bulk reaction coeff.
    Kwall,                 // Global wall reaction coeff.
    Climit,                // Limiting potential quality
    SourceQual,            // External source quality
    *NodeQual,             // Reported node quality state
    *PipeRateCoeff;        // Pipe reaction rate coeffs.

  struct Mempool
    *SegPool;              // Memory pool for water quality segments

  Pseg
    FreeSeg,               // Pointer to unused segment
    *FirstSeg,             // First (downstream) segment in each pipe
    *LastSeg;              // Last (upstream) segment in each pipe

  FlowDirection
    *FlowDir;              // Flow direction for each pipe

  SmassBalance
    MassBalance;           // Mass balance components

} Quality;

// Pipe Network Wrapper
typedef struct {

  int
    Nnodes,                // Number of network nodes
    Ntanks,                // Number of tanks
    Njuncs,                // Number of junction nodes
    Nlinks,                // Number of network links
    Npipes,                // Number of pipes
    Npumps,                // Number of pumps
    Nvalves,               // Number of valves
    Ncontrols,             // Number of simple controls
    Nrules,                // Number of control rules
    Npats,                 // Number of time patterns
    Ncurves;               // Number of data curves

  Snode    *Node;          // Node array
  Slink    *Link;          // Link array
  Stank    *Tank;          // Tank array
  Spump    *Pump;          // Pump array
  Svalve   *Valve;         // Valve array
  Spattern *Pattern;       // Time pattern array
  Scurve   *Curve;         // Data curve array
  Scontrol *Control;       // Simple controls array
  Srule    *Rule;          // Rule-based controls array
  HashTable
    *NodeHashTable,        // Hash table for Node ID names
    *LinkHashTable;        // Hash table for Link ID names
  Padjlist *Adjlist;       // Node adjacency lists

} Network;

// Overall Project Wrapper
typedef struct Project {

  Network    network;            // Pipe network wrapper
  Parser     parser;             // Input file parser wrapper
  Times      times;              // Time step wrapper
  Report     report;             // Reporting wrapper
  Outfile    outfile;            // Output file wrapper
  Rules      rules;              // Rule-based controls wrapper
  Hydraul    hydraul;            // Hydraulics solver wrapper
  Quality    quality;            // Water quality solver wrapper

  double Ucf[MAXVAR];            // Unit conversion factors

  int
    Openflag,                    // Project open flag
    Warnflag;                    // Warning flag

  char
    Msg[MAXMSG+1],               // General-purpose string: errors, messages
    Title[MAXTITLE][TITLELEN+1], // Project title
    MapFname[MAXFNAME+1],        // Map file name
    TmpHydFname[MAXFNAME+1],     // Temporary hydraulics file name
    TmpOutFname[MAXFNAME+1],     // Temporary output file name
    TmpStatFname[MAXFNAME+1];    // Temporary statistic file name

  void (* viewprog) (char *);    // Pointer to progress viewing function

} Project, *EN_Project;

#endif
