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

/*********************************************************/
/* All floats have been re-declared as doubles (7/3/07). */
/*********************************************************/ 
/*
-------------------------------------------
   Definition of 4-byte integers & reals
-------------------------------------------
*/
typedef  float        REAL4;                                                   //(2.00.11 - LR)
typedef  int          INT4;                                                    //(2.00.12 - LR)

/*
-----------------------------
   Global Constants
-----------------------------
*/
/*** Updated ***/
#define   CODEVERSION        20012                                             //(2.00.12 - LR)
#define   MAGICNUMBER        516114521
#define   VERSION            200
#define   EOFMARK            0x1A  /* Use 0x04 for UNIX systems */
#define   MAXTITLE  3        /* Max. # title lines                     */
#define   MAXID     31       /* Max. # characters in ID name           */      //(2.00.11 - LR)
#define   MAXMSG    79       /* Max. # characters in message text      */
#define   MAXLINE   255      /* Max. # characters read from input line */
#define   MAXFNAME  259      /* Max. # characters in file name         */
#define   MAXTOKS   40       /* Max. items per line of input           */
#define   TZERO     1.E-4    /* Zero time tolerance                    */
#define   TRUE      1
#define   FALSE     0
#define   FULL      2
#define   BIG       1.E10
#define   TINY      1.E-6
#define   MISSING   -1.E10
#define   PI        3.141592654

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
------------------------------------------------------
   Macro to find Pump index of Link[x]
   (Diameter = pump index for pump links)
------------------------------------------------------
*/
#define PUMPINDEX(x) (ROUND(Link[(x)].Diam))

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
   int    Type;        /* Curve type       */
   int    Npts;        /* Number of points */
   double *X;          /* X-values         */
   double *Y;          /* Y-values         */
}  Scurve;

struct Sdemand            /* DEMAND CATEGORY OBJECT */
{
   double Base;            /* Baseline demand  */
   int    Pat;             /* Pattern index    */
   struct Sdemand *next;   /* Next record      */
};
typedef struct Sdemand *Pdemand; /* Pointer to demand object */

struct Ssource     /* WQ SOURCE OBJECT */
{
 /*int   Node;*/     /* Node index of source     */
   double C0;       /* Base concentration/mass  */
   int    Pat;      /* Pattern index            */
   double Smass;    /* Actual mass flow rate    */
   char   Type;     /* SourceType (see below)   */
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
   char    Type;           /* Link type         */
   char    Stat;           /* Initial status    */
   char    Rpt;            /* Reporting flag    */
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
   char   MixModel; /* Type of mixing model     */
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
   double Energy[6];  /* Energy usage statistics:  */
                     /* 0 = pump utilization      */
                     /* 1 = avg. efficiency       */
                     /* 2 = avg. kW/flow          */
                     /* 3 = avg. kwatts           */
                     /* 4 = peak kwatts           */
                     /* 5 = cost/day              */
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
   char   Status;   /* New link status    */
   char   Type;     /* Control type       */
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


/*
----------------------------------------------
   Global Enumeration Variables
----------------------------------------------
*/
 enum Hydtype                   /* Hydraulics solution option:         */
                {USE,           /*    use from previous run            */
                 SAVE,          /*    save after current run           */
                 SCRATCH};      /*    use temporary file               */

 enum QualType                  /* Water quality analysis option:      */
                {NONE,          /*    no quality analysis              */
                 CHEM,          /*    analyze a chemical               */
                 AGE,           /*    analyze water age                */
                 TRACE};        /*    trace % of flow from a source    */

 enum NodeType                  /* Type of node:                       */
                {JUNC,          /*    junction                         */
                 RESERV,        /*    reservoir                        */
                 TANK};         /*    tank                             */

 enum LinkType                  /* Type of link:                       */
                 {CV,           /*    pipe with check valve            */
                  PIPE,         /*    regular pipe                     */
                  PUMP,         /*    pump                             */
                  PRV,          /*    pressure reducing valve          */
                  PSV,          /*    pressure sustaining valve        */
                  PBV,          /*    pressure breaker valve           */
                  FCV,          /*    flow control valve               */
                  TCV,          /*    throttle control valve           */
                  GPV};         /*    general purpose valve            */

 enum CurveType                /* Type of curve:                       */
                 {V_CURVE,     /*    volume curve                      */
                  P_CURVE,     /*    pump curve                        */
                  E_CURVE,     /*    efficiency curve                  */
                  H_CURVE};    /*    head loss curve                   */

 enum PumpType                  /* Type of pump curve:                 */
                {CONST_HP,      /*    constant horsepower              */
                 POWER_FUNC,    /*    power function                   */
                 CUSTOM,        /*    user-defined custom curve        */
                 NOCURVE};

 enum SourceType                /* Type of source quality input        */
                {CONCEN,        /*    inflow concentration             */
                 MASS,          /*    mass inflow booster              */
                 SETPOINT,      /*    setpoint booster                 */
                 FLOWPACED};    /*    flow paced booster               */

 enum ControlType               /* Control condition type:             */
                {LOWLEVEL,      /*    act when grade below set level   */
                 HILEVEL,       /*    act when grade above set level   */
                 TIMER,         /*    act when set time reached        */
                 TIMEOFDAY};    /*    act when time of day occurs      */

 enum StatType                  /* Link/Tank status:                   */
                 {XHEAD,        /*   pump cannot deliver head (closed) */
                  TEMPCLOSED,   /*   temporarily closed                */
                  CLOSED,       /*   closed                            */
                  OPEN,         /*   open                              */
                  ACTIVE,       /*   valve active (partially open)     */
                  XFLOW,        /*   pump exceeds maximum flow         */
                  XFCV,         /*   FCV cannot supply flow            */
                  XPRESSURE,    /*   valve cannot supply pressure      */
                  FILLING,      /*   tank filling                      */
                  EMPTYING};    /*   tank emptying                     */

 enum FormType                  /* Head loss formula:                  */
                 {HW,           /*   Hazen-Williams                    */
                  DW,           /*   Darcy-Weisbach                    */
                  CM};          /*   Chezy-Manning                     */

 enum UnitsType                 /* Unit system:                        */
                 {US,           /*   US                                */
                  SI};          /*   SI (metric)                       */

 enum FlowUnitsType             /* Flow units:                         */
                 {CFS,          /*   cubic feet per second             */
                  GPM,          /*   gallons per minute                */
                  MGD,          /*   million gallons per day           */
                  IMGD,         /*   imperial million gal. per day     */
                  AFD,          /*   acre-feet per day                 */
                  LPS,          /*   liters per second                 */
                  LPM,          /*   liters per minute                 */
                  MLD,          /*   megaliters per day                */
                  CMH,          /*   cubic meters per hour             */
                  CMD};         /*   cubic meters per day              */

 enum PressUnitsType            /* Pressure units:                     */
                 {PSI,          /*   pounds per square inch            */
                  KPA,          /*   kiloPascals                       */
                  METERS};      /*   meters                            */

 enum RangeType                 /* Range limits:                       */
                 {LOW,          /*   lower limit                       */
                  HI,           /*   upper limit                       */
                  PREC};        /*   precision                         */

 enum MixType                   /* Tank mixing regimes                 */
                 {MIX1,         /*   1-compartment model               */
                  MIX2,         /*   2-compartment model               */
                  FIFO,         /*   First in, first out model         */
                  LIFO};        /*   Last in, first out model          */ 

 enum TstatType                 /* Time series statistics              */
                 {SERIES,       /*   none                              */
                  AVG,          /*   time-averages                     */
                  MIN,          /*   minimum values                    */
                  MAX,          /*   maximum values                    */
                  RANGE};       /*   max - min values                  */

#define MAXVAR   21             /* Max. # types of network variables   */
                                /* (equals # items enumed below)       */
 enum FieldType                 /* Network variables:                  */
                 {ELEV,         /*   nodal elevation                   */
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
                  DRAINTIME};   /*   time to drain a tank              */

enum SectType    {_TITLE,_JUNCTIONS,_RESERVOIRS,_TANKS,_PIPES,_PUMPS,
                  _VALVES,_CONTROLS,_RULES,_DEMANDS,_SOURCES,_EMITTERS,
                  _PATTERNS,_CURVES,_QUALITY,_STATUS,_ROUGHNESS,_ENERGY,
                  _REACTIONS,_MIXING,_REPORT,_TIMES,_OPTIONS,
                  _COORDS,_VERTICES,_LABELS,_BACKDROP,_TAGS,_END};

enum HdrType                    /* Type of table heading   */
                 {STATHDR,      /*  Hydraulic Status       */
                  ENERHDR,      /*  Energy Usage           */
                  NODEHDR,      /*  Node Results           */
                  LINKHDR};     /*  Link Results           */

