/*
***********************************************************************
                                                                     
TYPES.H -- Global constants and data types for EPANET program  
                                                                     
VERSION:    2.00                                               
DATE:       5/8/00
            9/7/00
            10/25/00
            3/1/01
            12/6/01
            6/24/02
            8/15/07    (2.00.11)
            2/14/08    (2.00.12)
AUTHOR:     L. Rossman                                         
            US EPA - NRMRL
                                                                     
**********************************************************************
*/
#ifndef TYPES_H
#define TYPES_H

#include "epanet2.h"
#include "hash.h"
#include "mempool.h"

/*********************************************************/
/* All floats have been re-declared as doubles (7/3/07). */
/*********************************************************/ 
/*
-------------------------------------------
   Definition of 4-byte integers & reals
-------------------------------------------
*/
typedef  float        REAL4;                                                   
typedef  int          INT4;                                                    

/*
-----------------------------
   Global Constants
-----------------------------
*/
/*** Updated ***/
#define   CODEVERSION        20200
#define   MAGICNUMBER        516114521
#define   ENGINE_VERSION     201
#define   EOFMARK            0x1A  /* Use 0x04 for UNIX systems */
#define   MAXTITLE  3        /* Max. # title lines                     */
#define   MAXID     31       /* Max. # characters in ID name           */      
#define   MAXMSG    79       /* Max. # characters in message text      */
#define   MAXLINE   255      /* Max. # characters read from input line */
#define   MAXFNAME  259      /* Max. # characters in file name         */
//>>>>>>> f11308fc72eccc8b74b18d5ddab0eb2ae0d36587
#define   MAXTOKS   40       /* Max. items per line of input           */
#define   TZERO     1.E-4    /* Zero time tolerance                    */
#define   TRUE      1
#define   FALSE     0
#define   FULL      2
#define   BIG       1.E10
#define   TINY      1.E-6
#define   MISSING   -1.E10

#define   CBIG      1.e8     /* Big coefficient         */
#define   CSMALL    1.e-6    /* Small coefficient       */

#ifdef M_PI
  #define   PI        M_PI
#else
  #define   PI        3.141592654
#endif

/*** Updated 9/7/00 ***/
/* Various conversion factors */
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

#define   DIFFUS    1.3E-8   /* Diffusivity of chlorine                */
                             /* @ 20 deg C (sq ft/sec)                 */
#define   VISCOS    1.1E-5   /* Kinematic viscosity of water           */
                             /* @ 20 deg C (sq ft/sec)                 */

#define   SEPSTR    " \t\n\r"  /* Token separator characters */

/*
---------------------------------------------------------------------
   Macro to test for successful allocation of memory            
---------------------------------------------------------------------
*/
#define  MEMCHECK(x)  (((x) == NULL) ? 101 : 0 )
#define  FREE(x)      (free((x)))

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
 Global Enumeration Types
 ----------------------------------------------
 */
typedef enum {
  USE,           /*    use from previous run            */
  SAVE,          /*    save after current run           */
  SCRATCH
} Hydtype;       /*    use temporary file               */

typedef enum {
  NONE,          /*    no quality analysis              */
  CHEM,          /*    analyze a chemical               */
  AGE,           /*    analyze water age                */
  TRACE
} QualType;      /*    trace % of flow from a source    */

typedef enum {
  V_CURVE,       /*    volume curve                      */
  P_CURVE,       /*    pump curve                        */
  E_CURVE,       /*    efficiency curve                  */
  H_CURVE
} CurveType;     /*    head loss curve                   */

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
  FLOWPACED
} SourceType;    /*    flow paced booster               */

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
  EMPTYING
} StatType;     /*   tank emptying                     */

typedef enum {
  HW,           /*   Hazen-Williams                    */
  DW,           /*   Darcy-Weisbach                    */
  CM            /*   Chezy-Manning                     */
} FormType;    

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
} TstatType;


#define MAXVAR   21 /* Max. # types of network variables */        
                    /* (equals # items enumed below)   */
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
  DRAINTIME     /*   time to drain a tank              */
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
  POSITIVE,
  NEGATIVE
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

/*
------------------------------------------------------
   Global Data Structures                             
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

typedef struct        /* Coord OBJECT */
{
	char   ID[MAXID+1];  /* Coord ID         */
	double X;            /* X-value          */
	double Y;            /* Y-value          */
	char   HaveCoords;   /* Coordinates flag */
}  Scoord;

struct Sdemand            /* DEMAND CATEGORY OBJECT */
{
   double Base;            /* Baseline demand  */
   int    Pat;             /* Pattern index    */
   struct Sdemand *next;   /* Next record      */
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
   double  El;             /* Elevation        */
   Pdemand D;              /* Demand pointer   */
   Psource S;              /* Source pointer   */
   double  C0;             /* Initial quality  */
   double  Ke;             /* Emitter coeff.   */
   char    Rpt;            /* Reporting flag   */
   EN_NodeType Type;       /* Node Type */
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
   double  Rc;             /* Reaction cal      */
   EN_LinkType Type;       /* Link type         */
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
} Premise;

typedef struct s_Action     /* Rule Action Clause */
{
   int     link;            /* Link index */
   int     status;          /* Link's status */
   double  setting;         /* Link's setting */
   struct  s_Action *next;
} Action;

typedef struct s_aRule      /* Control Rule Structure */
{
   char     label[MAXID+1]; /* Rule character label */
   double   priority;       /* Priority level */
   Premise  *Pchain;        /* Linked list of premises */
   Action   *Tchain;        /* Linked list of actions if true */
   Action   *Fchain;        /* Linked list of actions if false */
   struct   s_aRule    *next;
} aRule;

typedef struct s_ActItem        /* Action list item */
{
   int      ruleindex;          /* Index of rule action belongs to */
   struct   s_Action   *action; /* An action structure */
   struct   s_ActItem  *next;     
} ActItem;



typedef struct {
  char
  Qualflag,  /// Water quality flag
  OpenQflag, /// Quality system opened flag
  Reactflag; /// Reaction indicator 
  
  char
  ChemName[MAXID+1],     /* Name of chemical             */
  ChemUnits[MAXID+1];    /* Units of chemical            */
  
  int
  TraceNode; /// Source node for flow tracing
  
  double 
  Ctol,      /// Water quality tolerance
  Diffus,    /// Diffusivity (sq ft/sec)
  Wbulk,     /// Avg. bulk reaction rate
  Wwall,     /// Avg. wall reaction rate
  Wtank,     /// Avg. tank reaction rate
  Wsource,   /// Avg. mass inflow
  Rfactor,   /// Roughness-reaction factor
  BulkOrder, /// Bulk flow reaction order
  WallOrder, /// Pipe wall reaction order     
  TankOrder, /// Tank reaction order          
  Kbulk,     /// Global bulk reaction coeff.  
  Kwall,     /// Global wall reaction coeff.  
  Climit,    /// Limiting potential quality   
  *NodeQual, /// Node quality state
  *TempQual, /// General purpose array for water quality
  *QTempVolumes,
  *QTankVolumes,
  *QLinkFlow,
  *PipeRateCoeff;
  
  long
  Qstep, /// Quality time step (sec)
  Qtime; /// Current quality time (sec)
  
  char      OutOfMemory;          /* Out of memory indicator                 */
  alloc_handle_t *SegPool;        // Memory pool for water quality segments   
  
  Pseg      FreeSeg;              /* Pointer to unused segment               */
  Pseg      *FirstSeg,            /* First (downstream) segment in each pipe */
            *LastSeg;             /* Last (upstream) segment in each pipe    */
  FlowDirection *FlowDir;         /* Flow direction for each pipe            */
  double    *VolIn;               /* Total volume inflow to node             */
  double    *MassIn;              /* Total mass inflow to node               */
  double    Sc;                   /* Schmidt Number                          */
  double    Bucf;                 /* Bulk reaction units conversion factor   */
  double    Tucf;                 /* Tank reaction units conversion factor   */
  
} quality_t;

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


typedef struct {
  
  FILE *InFile; /// Input file pointer
  
  char
  Coordflag,             /* Load coordinates flag        */
  Unitsflag,             /* Unit system flag             */
  Flowflag,              /* Flow units flag              */
  Pressflag;             /* Pressure units flag          */
  
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
  MaxCurves;             /* Curve count                  */
  
  char
  DefPatID[MAXID+1],     /* Default demand pattern ID    */
  InpFname[MAXFNAME+1];  /* Input file name              */
  
  STmplist 
  *Patlist,              /* Temporary time pattern list  */ 
  *Curvelist;            /* Temporary list of curves     */
  
  double *X;             // temporary array for curve data
  int 
  Ntokens,               /* Number of tokens in input line */
  Ntitle;                /* Number of title lines          */
  
  char *Tok[MAXTOKS];    /* Array of token strings         */
  char Comment[MAXMSG+1];
  STmplist *PrevPat;     /* Pointer to pattern list element */
  STmplist *PrevCurve;   /* Pointer to curve list element   */
  
} parser_data_t;

typedef struct {
  
  FILE *RptFile;         /* Report file pointer          */
  
  int
  Nperiods,              /* Number of reporting periods  */
  PageSize;              /* Lines/page in output report  */
  
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
  Rpt2Fname[MAXFNAME+1]; /* Secondary report file name   */
  
  SField   Field[MAXVAR];   /* Output reporting fields      */
  
  long      LineNum;        /* Current line number     */
  long      PageNum;        /* Current page number     */
  char      DateStamp[26];  /* Current date & time     */
  char      Fprinterr;      /* File write error flag   */
  
} report_options_t;


typedef struct {
  
  char
  HydFname[MAXFNAME+1],  /* Hydraulics file name         */
  OutFname[MAXFNAME+1],  /* Binary output file name      */
  TmpFname[MAXFNAME+1],  /* Temporary file name          */
  TmpDir[MAXFNAME+1],    /* Temporary directory name     */
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


typedef struct {
  
  aRule *Rule;        /* Array of rules */
  ActItem *ActList;   /* Linked list of action items */
  int RuleState;      /* State of rule interpreter */
  long Time1;         /* Start of rule evaluation time interval (sec) */
  Premise *Plast;     /* Previous premise clause */
  Action *Tlast;      /* Previous true action */
  Action *Flast;      /* Previous false action */
} rules_t;

/*
 ** NOTE: Hydraulic analysis of the pipe network at a given point in time
 **       is done by repeatedly solving a linearized version of the 
 **       equations for conservation of flow & energy:
 **
 **           A*H = F
 **
 **       where H = vector of heads (unknowns) at each node,
 **             F = vector of right-hand side coeffs.
 **             A = square matrix of coeffs.
 **       and both A and F are updated at each iteration until there is
 **       negligible change in pipe flows.
 **
 **       Each row (or column) of A corresponds to a junction in the pipe
 **       network. Each link (pipe, pump or valve) in the network has a
 **       non-zero entry in the row-column of A that corresponds to its
 **       end points. This results in A being symmetric and very sparse.
 **       The following arrays are used to efficiently manage this sparsity:
 */
typedef struct {
  // hydraulic solution vars
  double  
  *Aii,        /* Diagonal coeffs. of A               */
  *Aij,        /* Non-zero, off-diagonal coeffs. of A */
  *F,          /* Right hand side coeffs.             */
  *P,          /* Inverse headloss derivatives        */
  *Y;          /* Flow correction factors             */
  
  int     
  *Order,      /* Node-to-row of A                    */
  *Row,        /* Row-to-node of A                    */
  *Ndx,        /* Index of link's coeff. in Aij       */
  *XLNZ,       /* Start position of each column in NZSUB  */
  *NZSUB,      /* Row index of each coeff. in each column */
  *LNZ,        /* Position of each coeff. in Aij array    */
  *Degree;     /* Number of links adjacent to each node  */
} solver_t;

typedef struct {
  double  
  *NodeDemand,           /* Node actual demand           */
  *EmitterFlows,         /* Emitter flows                */
  *LinkSetting,          /* Link settings                */
  *LinkFlows,            /* Link flows                   */
  *NodeHead,
  Htol,                  /* Hydraulic head tolerance     */
  Qtol,                  /* Flow rate tolerance          */
  RQtol,                 /* Flow resistance tolerance    */
  Hexp,                  /* Exponent in headloss formula */
  Qexp,                  /* Exponent in orifice formula  */
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
  *X_tmp;
  
  int
  DefPat,                /* Default demand pattern       */
  Epat;                  /* Energy cost time pattern     */

  StatType  
  *LinkStatus,           /* Link status                  */
  *OldStat;              /* Previous link/tank status    */
  
  int
  MaxIter,               /* Max. hydraulic trials        */
  ExtraIter,             /* Extra hydraulic trials       */
  Ncoeffs,               /* Number of non-0 matrix coeffs*/
  CheckFreq,             /* Hydraulics solver parameter  */
  MaxCheck;              /* Hydraulics solver parameter  */
  
  char
  OpenHflag,             /* Hydraul. system opened flag  */
  Formflag;              /* Hydraulic formula flag       */
  
  /* Info about hydraulic solution */
  double relativeError;
  int iterations;
  
  /* Flag used to halt taking further time steps */
  int Haltflag;
  /* Relaxation factor used for updating flow changes */                         
  double RelaxFactor;                                                            
  
  solver_t solver;
  
} hydraulics_t;

typedef struct {
  int Nnodes,            /* Number of network nodes      */
  Ntanks,                /* Number of tanks              */
  Njuncs,                /* Number of junction nodes     */
  Nlinks,                /* Number of network links      */
  Npipes,                /* Number of pipes              */
  Npumps,                /* Number of pumps              */
  Nvalves,               /* Number of valves             */
  Ncontrols,             /* Number of simple controls    */
  Nrules,                /* Number of control rules      */
  Npats,                 /* Number of time patterns      */
  Ncurves,               /* Number of data curves        */
  Ncoords;               /* Number of Coords             */
  
  Snode    *Node;        /* Node data                    */
  Slink    *Link;        /* Link data                    */
  Stank    *Tank;        /* Tank data                    */
  Spump    *Pump;        /* Pump data                    */
  Svalve   *Valve;       /* Valve data                   */
  Spattern *Pattern;     /* Time patterns                */
  Scurve   *Curve;       /* Curve data                   */
  Scoord   *Coord;       /* Coordinate data              */
  Scontrol *Control;     /* Control data                 */
  ENHashTable *NodeHashTable,
              *LinkHashTable; /* Hash tables for ID labels    */
  Padjlist *Adjlist;          /* Node adjacency lists         */
  
} EN_Network;


/* project wrapper */
struct EN_Project {
  
  EN_Network network; /// the network description struct
  hydraulics_t hydraulics;
  rules_t rules;
  quality_t quality;
  time_options_t time_options;
  
  parser_data_t parser;
  report_options_t report;
  out_file_t out_files;
  save_options_t save_options;
  
  double Ucf[MAXVAR];
  
  char
  Openflag,                   /// Toolkit open flag
  Warnflag,                   /// Warning flag
  Msg[MAXMSG+1],              /// General-purpose string: errors, messages
  Title[MAXTITLE][MAXMSG+1],  /// Problem title
  MapFname[MAXFNAME+1];       /// Map file name
  
  void (* viewprog) (char *);     /* Pointer to progress viewing function */   
  
};

#endif
