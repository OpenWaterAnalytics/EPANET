/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       types.h
 Description:  symbolic constants and data types used throughout EPANET
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/10/2018
 ******************************************************************************
*/

#ifndef TYPES_H
#define TYPES_H

#include "hash.h"
#include "util/errormanager.h"
#include <stdio.h>

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
#define   ENGINE_VERSION     201   /* Used for binary hydraulics file */
#define   EOFMARK            0x1A  /* Use 0x04 for UNIX systems */
#define   MAXTITLE  3        /* Max. # title lines                     */
#define   TITLELEN  79       // Max. # characters in a title line
#define   MAXID     31       /* Max. # characters in ID name           */      
#define   MAXMSG    255      /* Max. # characters in message text      */
#define   MAXLINE   1024     /* Max. # characters read from input line */
#define   MAXFNAME  259      /* Max. # characters in file name         */
#define   MAXTOKS   40       /* Max. items per line of input           */
#define   TZERO     1.E-4    /* Zero time tolerance                    */
#define   TRUE      1
#define   FALSE     0
#define   FULL      2
#define   BIG       1.E10
#define   TINY      1.E-6
#define   MISSING   -1.E10     /* Missing value indicator */
#define   DIFFUS    1.3E-8     /* Diffusivity of chlorine                */
                               /* @ 20 deg C (sq ft/sec)                 */
#define   VISCOS    1.1E-5     /* Kinematic viscosity of water           */
                               /* @ 20 deg C (sq ft/sec)                 */
#define   SEPSTR    " \t\n\r"  /* Token separator characters */
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
#define INT(x)   ((int)(x))                   /* integer portion of x  */
#define FRAC(x)  ((x)-(int)(x))               /* fractional part of x  */
#define ABS(x)   (((x)<0) ? -(x) : (x))       /* absolute value of x   */
#define MIN(x,y) (((x)<=(y)) ? (x) : (y))     /* minimum of x and y    */
#define MAX(x,y) (((x)>=(y)) ? (x) : (y))     /* maximum of x and y    */
#define ROUND(x) (((x)>=0) ? (int)((x)+.5) : (int)((x)-.5))
                                              /* round-off of x        */
#define MOD(x,y) ((x)%(y))                    /* x modulus y           */
#define SQR(x)   ((x)*(x))                    /* x-squared             */
#define SGN(x)   (((x)<0) ? (-1) : (1))       /* sign of x             */
#define UCHAR(x) (((x) >= 'a' && (x) <= 'z') ? ((x)&~32) : (x))
                                              /* uppercase char of x   */
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
  NODE    = 0,
  LINK    = 1
} ObjectType;

typedef enum {
  JUNCTION = 0,
  RESERVOIR = 1,
  TANK = 2
} NodeType;

typedef enum {
  CVPIPE = 0,
  PIPE   = 1,
  PUMP   = 2,
  PRV    = 3,
  PSV    = 4,
  PBV    = 5,
  FCV    = 6,
  TCV    = 7,
  GPV    = 8
} LinkType;

typedef enum {
  USE,           /*    use from previous run            */
  SAVE,          /*    save after current run           */
  SCRATCH        /*    use temporary file               */
} HydFiletype;

typedef enum {
  NONE,          /*    no quality analysis              */
  CHEM,          /*    analyze a chemical               */
  AGE,           /*    analyze water age                */
  TRACE          /*    trace % of flow from a source    */
} QualType;

typedef enum {
  V_CURVE,       /*    volume curve                      */
  P_CURVE,       /*    pump curve                        */
  E_CURVE,       /*    efficiency curve                  */
  H_CURVE,       /*    head loss curve                   */
  G_CURVE        /*    General\default curve             */
} CurveType;

typedef enum {
  CONST_HP,      /*    constant horsepower              */
  POWER_FUNC,    /*    power function                   */
  CUSTOM,        /*    user-defined custom curve        */
  NOCURVE
} PumpType;

typedef enum {
  CONCEN,        /*    inflow concentration             */
  MASS,          /*    mass inflow booster              */
  SETPOINT,      /*    setpoint booster                 */
  FLOWPACED      /*    flow paced booster               */
} SourceType;

typedef enum {
  LOWLEVEL,      /*    act when grade below set level   */
  HILEVEL,       /*    act when grade above set level   */
  TIMER,         /*    act when set time reached        */
  TIMEOFDAY      /*    act when time of day occurs      */
} ControlType;    

typedef enum {
  XHEAD,        /*   pump cannot deliver head (closed) */
  TEMPCLOSED,   /*   temporarily closed                */
  CLOSED,       /*   closed                            */
  OPEN,         /*   open                              */
  ACTIVE,       /*   valve active (partially open)     */
  XFLOW,        /*   pump exceeds maximum flow         */
  XFCV,         /*   FCV cannot supply flow            */
  XPRESSURE,    /*   valve cannot supply pressure      */
  FILLING,      /*   tank filling                      */
  EMPTYING      /*   tank emptying                     */
} StatType;

typedef enum {
  HW,           /*   Hazen-Williams                    */
  DW,           /*   Darcy-Weisbach                    */
  CM            /*   Chezy-Manning                     */
} HeadLossType;    

typedef enum {
  US,           /*   US                                */
  SI            /*   SI (metric)                       */
} UnitsType;          

typedef enum {
  CFS,          /*   cubic feet per second             */
  GPM,          /*   gallons per minute                */
  MGD,          /*   million gallons per day           */
  IMGD,         /*   imperial million gal. per day     */
  AFD,          /*   acre-feet per day                 */
  LPS,          /*   liters per second                 */
  LPM,          /*   liters per minute                 */
  MLD,          /*   megaliters per day                */
  CMH,          /*   cubic meters per hour             */
  CMD           /*   cubic meters per day              */
} FlowUnitsType;         

typedef enum {
  PSI,          /*   pounds per square inch            */
  KPA,          /*   kiloPascals                       */
  METERS        /*   meters                            */
} PressUnitsType;

typedef enum {
  LOW,          /*   lower limit                       */
  HI,           /*   upper limit                       */
  PREC          /*   precision                         */
} RangeType;        

typedef enum {
  MIX1,         /*   1-compartment model               */
  MIX2,         /*   2-compartment model               */
  FIFO,         /*   First in, first out model         */
  LIFO          /*   Last in, first out model          */
} MixType;       

typedef enum {
  SERIES,       /*   none                              */
  AVG,          /*   time-averages                     */
  MIN,          /*   minimum values                    */
  MAX,          /*   maximum values                    */
  RANGE         /*   max - min values                  */
} StatisticType;


typedef enum {
  ELEV = 0,     /*   nodal elevation                   */
  DEMAND,       /*   nodal demand flow                 */
  HEAD,         /*   nodal hydraulic head              */
  PRESSURE,     /*   nodal pressure                    */
  QUALITY,      /*   nodal water quality               */
  
  LENGTH,       /*   link length                       */
  DIAM,         /*   link diameter                     */
  FLOW,         /*   link flow rate                    */
  VELOCITY,     /*   link flow velocity                */
  HEADLOSS,     /*   link head loss                    */
  LINKQUAL,     /*   avg. water quality in link        */
  STATUS,       /*   link status                       */
  SETTING,      /*   pump/valve setting                */
  REACTRATE,    /*   avg. reaction rate in link        */
  FRICTION,     /*   link friction factor              */
  
  POWER,        /*   pump power output                 */
  TIME,         /*   simulation time                   */
  VOLUME,       /*   tank volume                       */
  CLOCKTIME,    /*   simulation time of day            */
  FILLTIME,     /*   time to fill a tank               */
  DRAINTIME,    /*   time to drain a tank              */
  MAXVAR        //   total number of variable fields
} FieldType;

typedef enum {
  _TITLE,_JUNCTIONS,_RESERVOIRS,_TANKS,_PIPES,_PUMPS,
  _VALVES,_CONTROLS,_RULES,_DEMANDS,_SOURCES,_EMITTERS,
  _PATTERNS,_CURVES,_QUALITY,_STATUS,_ROUGHNESS,_ENERGY,
  _REACTIONS,_MIXING,_REPORT,_TIMES,_OPTIONS,
  _COORDS,_VERTICES,_LABELS,_BACKDROP,_TAGS,_END
} SectType;

typedef enum {
  STATHDR,      /*  Hydraulic Status header  */
  ENERHDR,      /*  Energy Usage header      */
  NODEHDR,      /*  Node Results header      */
  LINKHDR       /*  Link Results header      */
} HdrType;    

typedef enum {
  NEGATIVE  = -1,    // Flow in reverse of pre-assigned direction
  ZERO_FLOW = 0,     // Zero flow
  POSITIVE  = 1      // Flow in pre-assigned direction
} FlowDirection;

typedef enum {
  PCNT_ONLINE,
  PCNT_EFFIC,
  KWH_PER_FLOW,
  TOTAL_KWH,
  MAX_KW,
  TOTAL_COST,
  MAX_ENERGY_STATS
} EnergyStats;

typedef enum {
    DDA,        // Demand Driven Analysis    
    PDA         // Pressure Driven Analysis
} DemandModelType;

/*
------------------------------------------------------
   Fundamental Data Structures                             
------------------------------------------------------
*/

struct IDstring    /* Holds component ID labels */
{
   char ID[MAXID+1];
};

struct  Floatlist  /* Element of list of floats */
{
   double  value;
   struct  Floatlist *next;
};
typedef struct Floatlist SFloatlist;

struct  Tmplist    /* Element of temp list for Pattern & Curve data */
{
   int        i;
   char       ID[MAXID+1];
   SFloatlist *x;
   SFloatlist *y;
   struct     Tmplist  *next;
};
typedef struct Tmplist STmplist;

typedef struct        /* TIME PATTERN OBJECT */
{
   char   ID[MAXID+1]; /* Pattern ID       */
   int    Length;      /* Pattern length   */
   double *F;          /* Pattern factors  */
}  Spattern;

typedef struct        /* CURVE OBJECT */
{
   char   ID[MAXID+1]; /* Curve ID         */
   CurveType Type;     /* Curve type       */
   int    Npts;        /* Number of points */
   double *X;          /* X-values         */
   double *Y;          /* Y-values         */
}  Scurve;

struct Sdemand            /* DEMAND CATEGORY OBJECT */
{
   double Base;            /* Baseline demand      */
   int    Pat;             /* Pattern index        */
   char   Name[MAXMSG+1];  /* Demand category name */
   struct Sdemand *next;   /* Next record          */
};
typedef struct Sdemand *Pdemand; /* Pointer to demand object */

typedef struct
{
    double hrsOnLine;        // hours pump is online
    double efficiency;       // total time wtd. efficiency
    double kwHrsPerCFS;      // total kw-hrs per cfs of flow
    double kwHrs;            // total kw-hrs consumed
    double maxKwatts;        // max. kw consumed
    double totalCost;        // total pumping cost
} Senergy;

struct Ssource     /* WQ SOURCE OBJECT */
{
 /*int   Node;*/     /* Node index of source     */
   double C0;       /* Base concentration/mass  */
   int    Pat;      /* Pattern index            */
   double Smass;    /* Actual mass flow rate    */
   SourceType Type;  /* SourceType (see below)   */
};
typedef struct Ssource *Psource; /* Pointer to WQ source object */

typedef struct            /* NODE OBJECT */
{
   char    ID[MAXID+1];    /* Node ID          */
   double  X;              /* X-coordinate     */
   double  Y;              /* Y-coordinate     */
   double  El;             /* Elevation        */
   Pdemand D;              /* Demand pointer   */
   Psource S;              /* Source pointer   */
   double  C0;             /* Initial quality  */
   double  Ke;             /* Emitter coeff.   */
   char    Rpt;            /* Reporting flag   */
   NodeType Type;          /* Node Type */
   char Comment[MAXMSG+1]; /* Node Comment */
}  Snode;

typedef struct            /* LINK OBJECT */
{
   char    ID[MAXID+1];    /* Link ID           */
   int     N1;             /* Start node index  */
   int     N2;             /* End node index    */
   double  Diam;           /* Diameter          */
   double  Len;            /* Length            */
   double  Kc;             /* Roughness         */
   double  Km;             /* Minor loss coeff. */
   double  Kb;             /* Bulk react. coeff */
   double  Kw;             /* Wall react. coeff */
   double  R;              /* Flow resistance   */
   double  Rc;             /* Reaction coeff.   */
   double  Qa;             // Low flow limit
   LinkType Type;          // Link type         */
   StatType Stat;          /* Initial status    */
   char Rpt;            /* Reporting flag    */
   char Comment[MAXMSG+1]; /* Link Comment */
}  Slink;

typedef struct     /* TANK OBJECT */
{
   int    Node;     /* Node index of tank       */
   double A;        /* Tank area                */
   double Hmin;     /* Minimum water elev       */
   double Hmax;     /* Maximum water elev       */
   double H0;       /* Initial water elev       */
   double Vmin;     /* Minimum volume           */
   double Vmax;     /* Maximum volume           */
   double V0;       /* Initial volume           */
   double Kb;       /* Reaction coeff. (1/days) */
   double V;        /* Tank volume              */
   double C;        /* Concentration            */
   int    Pat;      /* Fixed grade time pattern */
   int    Vcurve;   /* Vol.- elev. curve index  */
   MixType MixModel;/* Type of mixing model     */
                    /* (see MixType below)      */
   double V1max;    /* Mixing compartment size  */
}  Stank;

typedef struct     /* PUMP OBJECT */
{
   int    Link;     /* Link index of pump          */
   int    Ptype;    /* Pump curve type             */
                    /* (see PumpType below)        */
   double Q0;       /* Initial flow                */
   double Qmax;     /* Maximum flow                */
   double Hmax;     /* Maximum head                */
   double H0;       /* Shutoff head                */
   double R;        /* Flow coeffic.               */
   double N;        /* Flow exponent               */
   int    Hcurve;   /* Head v. flow curve index    */
   int    Ecurve;   /* Effic. v. flow curve index  */
   int    Upat;     /* Utilization pattern index   */
   int    Epat;     /* Energy cost pattern index   */
   double Ecost;    /* Unit energy cost            */
   double Energy[MAX_ENERGY_STATS];  /* Energy usage statistics     */
}  Spump;

typedef struct     /* VALVE OBJECT */
{
   int   Link;     /* Link index of valve */
}  Svalve;

typedef struct     /* CONTROL STATEMENT */
{
   int    Link;     /* Link index         */
   int    Node;     /* Control node index */
   long   Time;     /* Control time       */
   double Grade;    /* Control grade      */
   double Setting;  /* New link setting   */
   StatType Status; /* New link status    */
   ControlType Type;/* Control type       */
                   /* (see ControlType below) */
}  Scontrol;

struct   Sadjlist         /* NODE ADJACENCY LIST ITEM */
{
   int    node;            /* Index of connecting node */
   int    link;            /* Index of connecting link */
   struct Sadjlist *next;  /* Next item in list        */
};
/* Pointer to adjacency list item */
typedef struct Sadjlist *Padjlist; 

struct  Sseg               /* PIPE SEGMENT record used */
{                          /*   for WQ routing         */
   double  v;              /* Segment volume      */
   double  c;              /* Water quality value */
   struct  Sseg *prev;     /* Record for previous segment */
};
typedef struct Sseg *Pseg;    /* Pointer to pipe segment */

typedef struct            /* FIELD OBJECT of report table */
{
   char   Name[MAXID+1];   /* Name of reported variable  */
   char   Units[MAXID+1];  /* Units of reported variable */
   char   Enabled;         /* Enabled if in table        */
   int    Precision;       /* Number of decimal places   */
   double RptLim[2];       /* Lower/upper report limits  */
} SField;

typedef struct s_Premise    /* Rule Premise Clause */
{
   int      logop;          /* Logical operator */
   int      object;         /* Node or link */
   int      index;          /* Object's index */
   int      variable;       /* Pressure, flow, etc. */
   int      relop;          /* Relational operator */
   int      status;         /* Variable's status */
   double   value;          /* Variable's value */
   struct   s_Premise *next;
} Spremise;

typedef struct s_Action     /* Rule Action Clause */
{
   int     link;            /* Link index */
   int     status;          /* Link's status */
   double  setting;         /* Link's setting */
   struct  s_Action *next;
} Saction;

typedef struct              /* Control Rule Structure */
{
   char     label[MAXID+1]; /* Rule label */
   double   priority;       /* Priority level */
   Spremise *Premises;      /* Linked list of premises */
   Saction  *ThenActions;   /* Linked list of THEN actions */
   Saction  *ElseActions;   /* Linked list of ELSE actions */
} Srule;

typedef struct s_ActionItem /* Action list item */
{
   int      ruleIndex;      /* Index of rule action belongs to */
   Saction  *action;        /* An action structure */
   struct   s_ActionItem *next;     
} SactionList;

typedef struct
{
    double    initial;
    double    inflow;
    double    outflow;
    double    reacted;
    double    final;
    double    ratio;
} MassBalance;

/*
------------------------------------------------------
  Wrapper Data Structures
------------------------------------------------------
*/

// Input File Parser Wrapper
typedef struct {
  FILE *InFile;          /* Input file pointer           */
  
  char
    Unitsflag,             /* Unit system flag             */
    Flowflag,              /* Flow units flag              */
    Pressflag,             /* Pressure units flag          */
    DefPatID[MAXID+1],     /* Default demand pattern ID    */
    InpFname[MAXFNAME+1],  /* Input file name              */
    *Tok[MAXTOKS],         /* Array of token strings       */
    Comment[MAXMSG+1];     /* Comment text                 */
  
  int      
    MaxNodes,              /* Node count from input file   */
    MaxLinks,              /* Link count from input file   */
    MaxJuncs,              /* Junction count               */
    MaxPipes,              /* Pipe count                   */
    MaxTanks,              /* Tank count                   */
    MaxPumps,              /* Pump count                   */
    MaxValves,             /* Valve count                  */
    MaxControls,           /* Control count                */
    MaxRules,              /* Rule count                   */
    MaxPats,               /* Pattern count                */
    MaxCurves,             /* Curve count                  */
    Ntokens,               /* # tokens a line of input     */
    Ntitle;                /* Number of title lines        */
  
  STmplist 
    *Patlist,              /* Temporary time pattern list  */ 
    *PrevPat,              /* Ptr to pattern list element  */
    *Curvelist,            /* Temporary list of curves     */
    *PrevCurve;            /* Ptr to curve list element    */

  double *X;               // temporary array for curve data

} parser_data_t;

// Time Step Options Wrapper
typedef struct {

  long
    Tstart,                /* Starting time of day (sec)   */
    Hstep,                 /* Nominal hyd. time step (sec) */
    Pstep,                 /* Time pattern time step (sec) */
    Pstart,                /* Starting pattern time (sec)  */
    Rstep,                 /* Reporting time step (sec)    */
    Rstart,                /* Time when reporting starts   */
    Rtime,                 /* Next reporting time          */
    Htime,                 /* Current hyd. time (sec)      */
    Hydstep,               /* Actual hydraulic time step   */
    Rulestep,              /* Rule evaluation time step    */
    Dur;                   /* Duration of simulation (sec) */

} time_options_t;

// Reporting Options Wrapper
typedef struct {

  FILE *RptFile;           /* Report file pointer          */
  
  int
    Nperiods,              /* Number of reporting periods  */
    PageSize;              /* Lines/page in output report  */

  long
    LineNum,               /* Current line number          */
    PageNum;               /* Current page number          */
  
  char
    Rptflag,               /* Report flag                  */
    Tstatflag,             /* Time statistics flag         */
    Summaryflag,           /* Report summary flag          */
    Messageflag,           /* Error/warning message flag   */
    Statflag,              /* Status report flag           */
    Energyflag,            /* Energy report flag           */
    Nodeflag,              /* Node report flag             */
    Linkflag,              /* Link report flag             */
    Atime[13],             /* Clock time (hrs:min:sec)     */
    Rpt1Fname[MAXFNAME+1], /* Primary report file name     */
    Rpt2Fname[MAXFNAME+1], /* Secondary report file name   */
    DateStamp[26],         /* Current date & time          */
    Fprinterr;             /* File write error flag        */
  
  SField   Field[MAXVAR];  /* Output reporting fields      */

} report_options_t;

// Output File Wrapper
typedef struct {

  char
    HydFname[MAXFNAME+1],  /* Hydraulics file name         */
    OutFname[MAXFNAME+1],  /* Binary output file name      */
    Outflag,               /* Output file flag             */
    Hydflag;               /* Hydraulics flag              */
  
  long     
    HydOffset,             /* Hydraulics file byte offset  */
    OutOffset1,            /* 1st output file byte offset  */
    OutOffset2;            /* 2nd output file byte offset  */
  
  FILE
    *OutFile,              /* Output file pointer          */
    *HydFile,              /* Hydraulics file pointer      */
    *TmpOutFile;           /* Temporary file handle        */

} out_file_t;

typedef struct {

  char
    SaveHflag,             /* Hydraul. results saved flag  */
    SaveQflag,             /* Quality results saved flag   */
    Saveflag;              /* General purpose save flag    */

} save_options_t;

// Rule-Based Controls Wrapper
typedef struct {

    SactionList *ActionList;     /* Linked list of action items */
    int         RuleState;       /* State of rule interpreter */
    int         Errcode;         // Rule parser error code
    long        Time1;           /* Start of rule evaluation time interval (sec) */
    Spremise    *LastPremise;    /* Previous premise clause */
    Saction     *LastThenAction; /* Previous THEN action */
    Saction     *LastElseAction; /* Previous ELSE action */

} rules_t;

// Hydraulic Solution Wrapper
typedef struct {

  double
  *Aii,        /* Diagonal coeffs. of A               */
  *Aij,        /* Non-zero, off-diagonal coeffs. of A */
  *F,          /* Right hand side coeffs.             */
  *P,          /* Inverse headloss derivatives        */
  *Y;          /* Flow correction factors             */
  
  int          /* Ordered sparse matrix storage       */
  *Order,      /* Node-to-row of A                    */
  *Row,        /* Row-to-node of A                    */
  *Ndx,        /* Index of link's coeff. in Aij       */
  *XLNZ,       /* Start position of each column in NZSUB  */
  *NZSUB,      /* Row index of each coeff. in each column */
  *LNZ,        /* Position of each coeff. in Aij array    */
  *Degree;     /* Number of links adjacent to each node  */

} solver_t;

// Hydraulic Engine Wrapper
typedef struct {

  double  
    *NodeDemand,           // Node actual total outflow
    *DemandFlows,          // Demand outflows
    *EmitterFlows,         /* Emitter flows                */
    *LinkSetting,          /* Link settings                */
    *LinkFlows,            /* Link flows                   */
    *NodeHead,
    Htol,                  /* Hydraulic head tolerance     */
    Qtol,                  /* Flow rate tolerance          */
    RQtol,                 /* Flow resistance tolerance    */
    Hexp,                  /* Exponent in headloss formula */
    Qexp,                  /* Exponent in emitter formula  */
    Pexp,                  // Exponent in demand formula
    Pmin,                  // Pressure needed for any demand
    Preq,                  // Pressure needed for full demand
    Dmult,                 /* Demand multiplier            */
    Hacc,                  /* Hydraulics solution accuracy */
    FlowChangeLimit,       /* Hydraulics flow change limit */
    HeadErrorLimit,        /* Hydraulics head error limit  */
    DampLimit,             /* Solution damping threshold   */
    Viscos,                /* Kin. viscosity (sq ft/sec)   */
    SpGrav,                /* Specific gravity             */
    Epump,                 /* Global pump efficiency       */  
    Dsystem,               /* Total system demand          */
    Ecost,                 /* Base energy cost per kwh     */
    Dcost,                 /* Energy demand charge/kw/day  */
    Emax,                  /* Peak energy usage            */
    RelativeError,         // Total flow change / total flow
    MaxHeadError,          // Max. error for link head loss 
    MaxFlowChange,         // Max. change in link flow
    *X_tmp;                // Scratch array
  
  int
    DefPat,                /* Default demand pattern       */
    Epat,                  /* Energy cost time pattern     */
    DemandModel,           // Fixed or pressure dependent
    Iterations,            // # hydraulic trials taken
    MaxIter,               /* Max. hydraulic trials        */
    ExtraIter,             /* Extra hydraulic trials       */
    Ncoeffs,               /* Number of non-0 matrix coeffs*/
    CheckFreq,             /* Hydraulics solver parameter  */
    MaxCheck;              /* Hydraulics solver parameter  */

  StatType  
    *LinkStatus,           /* Link status                  */
    *OldStat;              /* Previous link/tank status    */
  
  char
    OpenHflag,             /* Hydraul. system opened flag  */
    Formflag;              /* Hydraulic formula flag       */
  
  
  /* Flag used to halt taking further time steps */
  int Haltflag;
  /* Relaxation factor used for updating flow changes */                         
  double RelaxFactor;
  solver_t solver;

} hydraulics_t;

// Forward declaration of the Mempool structure defined in mempool.h
struct Mempool;

// Water Quality Engine Wrapper
typedef struct {

  char
    Qualflag,        // Water quality flag
    OpenQflag,       // Quality system opened flag
    Reactflag,       // Reaction indicator 
    OutOfMemory;     // Out of memory indicator

  char
    ChemName[MAXID + 1],    // Name of chemical
    ChemUnits[MAXID + 1];   // Units of chemical

  int
    TraceNode,       // Source node for flow tracing
    *SortedNodes,    // Topologically sorted node indexes
    *Ilist,          // Link incidence lists for all nodes
    *IlistPtr;       // Start index of each node in Ilist

  double
    Ctol,            // Water quality tolerance
    Diffus,          // Diffusivity (sq ft/sec)
    Wbulk,           // Avg. bulk reaction rate
    Wwall,           // Avg. wall reaction rate
    Wtank,           // Avg. tank reaction rate
    Wsource,         // Avg. mass inflow
    Rfactor,         // Roughness-reaction factor
    Sc,              // Schmidt Number
    Bucf,            // Bulk reaction units conversion factor
    Tucf,            // Tank reaction units conversion factor
    BulkOrder,       // Bulk flow reaction order
    WallOrder,       // Pipe wall reaction order     
    TankOrder,       // Tank reaction order          
    Kbulk,           // Global bulk reaction coeff.  
    Kwall,           // Global wall reaction coeff.  
    Climit,          // Limiting potential quality
    SourceQual,      // External source quality
    *NodeQual,       // Reported node quality state
    *PipeRateCoeff;  // Pipe reaction rate coeffs.

  long
    Qstep,           // Quality time step (sec)
    Qtime;           // Current quality time (sec)

  struct Mempool
    *SegPool;        // Memory pool for water quality segments   

  Pseg
    FreeSeg,         // Pointer to unused segment
    *FirstSeg,       // First (downstream) segment in each pipe
    *LastSeg;        // Last (upstream) segment in each pipe

  FlowDirection
    *FlowDir;        // Flow direction for each pipe

  MassBalance
    massbalance;     // Mass balance components

} quality_t;

// Pipe Network Wrapper
typedef struct {

  int
    Nnodes,            /* Number of network nodes      */
    Ntanks,            /* Number of tanks              */
    Njuncs,            /* Number of junction nodes     */
    Nlinks,            /* Number of network links      */
    Npipes,            /* Number of pipes              */
    Npumps,            /* Number of pumps              */
    Nvalves,           /* Number of valves             */
    Ncontrols,         /* Number of simple controls    */
    Nrules,            /* Number of control rules      */
    Npats,             /* Number of time patterns      */
    Ncurves;           /* Number of data curves        */
  
  Snode    *Node;        /* Node array                   */
  Slink    *Link;        /* Link array                   */
  Stank    *Tank;        /* Tank array                   */
  Spump    *Pump;        /* Pump array                   */
  Svalve   *Valve;       /* Valve array                  */
  Spattern *Pattern;     /* Time pattern array           */
  Scurve   *Curve;       /* Data curve array             */
  Scontrol *Control;     /* Simple controls array        */
  Srule    *Rule;        /* Rule-based controls array    */
  HashTable
    *NodeHashTable,
    *LinkHashTable;      /* Hash tables for ID labels    */
  Padjlist *Adjlist;     /* Node adjacency lists         */

} network_t;

/* Overall Project Wrapper */
typedef struct Project {

  parser_data_t    parser;
  time_options_t   time_options;
  report_options_t report;
  out_file_t       out_files;
  save_options_t   save_options;
  rules_t          rules;
  hydraulics_t     hydraulics;
  quality_t        quality;
  network_t        network;
  
  double
    Ucf[MAXVAR];                 // Unit conversion factors
  
  char
    Openflag,                    // Toolkit open flag
    Warnflag,                    // Warning flag
    Msg[MAXMSG+1],               // General-purpose string: errors, messages
    Title[MAXTITLE][TITLELEN+1], // Project title
    MapFname[MAXFNAME+1],        // Map file name
    TmpHydFname[MAXFNAME+1],     // Temporary hydraulics file name
    TmpOutFname[MAXFNAME+1],     // Temporary output file name
    TmpStatFname[MAXFNAME+1];    // Temporary statistic file name
  
  error_handle_t* error_handle;  // Simple error manager
  void (* viewprog) (char *);    // Pointer to progress viewing function   
  
} Project, *EN_Project;

#endif
