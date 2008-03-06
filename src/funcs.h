/*
**************************************************************************
                                                                   
FUNCS.H -- Function Prototypes for EPANET Program                       
                                                                   
VERSION:    2.00
DATE:       5/8/00
            9/25/00
            10/25/00
            12/29/00
            3/1/01
            2/14/08    (2.00.12)
AUTHOR:     L. Rossman
            US EPA - NRMRL
                                                                                
**************************************************************************
*/

/*****************************************************************/
/*   Most float arguments have been changed to double - 7/3/07   */
/*****************************************************************/

/* ------- EPANET.C --------------------*/
/*
**  NOTE: The exportable functions that can be called
**        via the DLL are prototyped in TOOLKIT.H.
*/
void    initpointers(void);               /* Initializes pointers       */
int     allocdata(void);                  /* Allocates memory           */
void    freeTmplist(STmplist *);          /* Frees items in linked list */
void    freeFloatlist(SFloatlist *);      /* Frees list of floats       */
void    freedata(void);                   /* Frees allocated memory     */
int     openfiles(char *,char *,char *);  /* Opens input & report files */
int     openhydfile(void);                /* Opens hydraulics file      */
int     openoutfile(void);                /* Opens binary output file   */
int     strcomp(char *, char *);          /* Compares two strings       */
char*   getTmpName(char* fname);          /* Gets temporary file name   */     //(2.00.12 - LR)
double  interp(int, double *,             /* Interpolates a data curve  */
               double *, double);
int     findnode(char *);                 /* Finds node's index from ID */
int     findlink(char *);                 /* Finds link's index from ID */
char*   geterrmsg(int);                   /* Gets text of error message */
void    errmsg(int);                      /* Reports program error      */
void    writecon(char *);                 /* Writes text to console     */
void    writewin(char *);                 /* Passes text to calling app */

/* ------- INPUT1.C --------------------*/
int     getdata(void);                    /* Gets network data          */
void    setdefaults(void);                /* Sets default values        */
void    initreport(void);                 /* Initializes report options */
void    adjustdata(void);                 /* Adjusts input data         */
int     inittanks(void);                  /* Initializes tank levels    */
void    initunits(void);                  /* Determines reporting units */
void    convertunits(void);               /* Converts data to std. units*/

/* -------- INPUT2.C -------------------*/
int     netsize(void);                    /* Determines network size    */
int     readdata(void);                   /* Reads in network data      */
int     newline(int, char *);             /* Processes new line of data */
int     addnodeID(int, char *);           /* Adds node ID to data base  */
int     addlinkID(int, char *);           /* Adds link ID to data base  */
int     addpattern(char *);               /* Adds pattern to data base  */
int     addcurve(char *);                 /* Adds curve to data base    */
STmplist *findID(char *, STmplist *);     /* Locates ID on linked list  */
int     unlinked(void);                   /* Checks for unlinked nodes  */
int     getpumpparams(void);              /* Computes pump curve coeffs.*/
int     getpatterns(void);                /* Gets pattern data from list*/
int     getcurves(void);                  /* Gets curve data from list  */
int     findmatch(char *,char *[]);       /* Finds keyword in line      */
int     match(char *, char *);            /* Checks for word match      */
int     gettokens(char *);                /* Tokenizes input line       */
int     getfloat(char *, double *);       /* Converts string to double   */
double  hour(char *, char *);             /* Converts time to hours     */
int     setreport(char *);                /* Processes reporting command*/
void    inperrmsg(int,int,char *);        /* Input error message        */

/* ---------- INPUT3.C -----------------*/
int     juncdata(void);                   /* Processes junction data    */
int     tankdata(void);                   /* Processes tank data        */
int     pipedata(void);                   /* Processes pipe data        */
int     pumpdata(void);                   /* Processes pump data        */
int     valvedata(void);                  /* Processes valve data       */
int     patterndata(void);                /* Processes pattern data     */
int     curvedata(void);                  /* Processes curve data       */
int     demanddata(void);                 /* Processes demand data      */
int     controldata(void);                /* Processes simple controls  */
int     energydata(void);                 /* Processes energy data      */
int     sourcedata(void);                 /* Processes source data      */
int     emitterdata(void);                /* Processes emitter data     */
int     qualdata(void);                   /* Processes quality data     */
int     reactdata(void);                  /* Processes reaction data    */
int     mixingdata(void);                 /* Processes tank mixing data */
int     statusdata(void);                 /* Processes link status data */
int     reportdata(void);                 /* Processes report options   */
int     timedata(void);                   /* Processes time options     */
int     optiondata(void);                 /* Processes analysis options */
int     optionchoice(int);                /* Processes option choices   */
int     optionvalue(int);                 /* Processes option values    */
int     getpumpcurve(int);                /* Constructs a pump curve    */
int     powercurve(double, double, double,/* Coeffs. of power pump curve*/
                   double, double, double *,
                   double *, double *);
int     valvecheck(int, int, int);        /* Checks valve placement     */
void    changestatus(int, char, double);  /* Changes status of a link   */

/* -------------- RULES.C --------------*/
void    initrules(void);                  /* Initializes rule base      */
void    addrule(char *);                  /* Adds rule to rule base     */
int     allocrules(void);                 /* Allocates memory for rule  */
int     ruledata(void);                   /* Processes rule input data  */
int     checkrules(long);                 /* Checks all rules           */
void    freerules(void);                  /* Frees rule base memory     */  

/* ------------- REPORT.C --------------*/
int     writereport(void);                /* Writes formatted report    */
void    writelogo(void);                  /* Writes program logo        */
void    writesummary(void);               /* Writes network summary     */
void    writehydstat(int,double);          /* Writes hydraulic status    */
void    writeenergy(void);                /* Writes energy usage        */
int     writeresults(void);               /* Writes node/link results   */
void    writeheader(int,int);             /* Writes heading on report   */
void    writeline(char *);                /* Writes line to report file */
void    writerelerr(int, double);          /* Writes convergence error   */
void    writestatchange(int,char,char);   /* Writes link status change  */
void    writecontrolaction(int, int);     /* Writes control action taken*/
void    writeruleaction(int, char *);     /* Writes rule action taken   */
int     writehydwarn(int,double);          /* Writes hydraulic warnings  */
void    writehyderr(int);                 /* Writes hydraulic error msg.*/
int     disconnected(void);               /* Checks for disconnections  */
void    marknodes(int, int *, char *);    /* Identifies connected nodes */
void    getclosedlink(int, char *);       /* Finds a disconnecting link */
void    writelimits(int,int);             /* Writes reporting limits    */
int     checklimits(double *,int,int);     /* Checks variable limits     */
void    writetime(char *);                /* Writes current clock time  */
char    *clocktime(char *, long);         /* Converts time to hrs:min   */
char    *fillstr(char *, char, int);      /* Fills string with character*/
int     getnodetype(int);                 /* Determines node type       */

/* --------- HYDRAUL.C -----------------*/
int     openhyd(void);                    /* Opens hydraulics solver    */

/*** Updated 3/1/01 ***/
void    inithyd(int);                     /* Re-sets initial conditions */

int     runhyd(long *);                   /* Solves 1-period hydraulics */
int     nexthyd(long *);                  /* Moves to next time period  */
void    closehyd(void);                   /* Closes hydraulics solver   */
int     allocmatrix(void);                /* Allocates matrix coeffs.   */
void    freematrix(void);                 /* Frees matrix coeffs.       */
void    initlinkflow(int, char, double);  /* Initializes link flow      */
void    setlinkflow(int, double);         /* Sets link flow via headloss*/
void    setlinkstatus(int, char, char *,  /* Sets link status           */
                      double *);
void    setlinksetting(int, double,       /* Sets pump/valve setting    */
                       char *, double *);
void    resistance(int);                  /* Computes resistance coeff. */
void    demands(void);                    /* Computes current demands   */
int     controls(void);                   /* Controls link settings     */
long    timestep(void);                   /* Computes new time step     */
void    tanktimestep(long *);             /* Time till tanks fill/drain */
void    controltimestep(long *);          /* Time till control action   */
void    ruletimestep(long *);             /* Time till rule action      */
void    addenergy(long);                  /* Accumulates energy usage   */
void    getenergy(int, double *, double *); /* Computes link energy use   */
void    tanklevels(long);                 /* Computes new tank levels   */
double  tankvolume(int,double);           /* Finds tank vol. from grade */
double  tankgrade(int,double);            /* Finds tank grade from vol. */
int     netsolve(int *,double *);         /* Solves network equations   */
int     badvalve(int);                    /* Checks for bad valve       */
int     valvestatus(void);                /* Updates valve status       */
int     linkstatus(void);                 /* Updates link status        */
char    cvstatus(char,double,double);     /* Updates CV status          */
char    pumpstatus(int,double);           /* Updates pump status        */
char    prvstatus(int,char,double,        /* Updates PRV status         */
                  double,double);
char    psvstatus(int,char,double,        /* Updates PSV status         */
                  double,double);
char    fcvstatus(int,char,double,        /* Updates FCV status         */
                  double);
void    tankstatus(int,int,int);          /* Checks if tank full/empty  */
int     pswitch(void);                    /* Pressure switch controls   */
double  newflows(void);                   /* Updates link flows         */
void    newcoeffs(void);                  /* Computes matrix coeffs.    */
void    linkcoeffs(void);                 /* Computes link coeffs.      */
void    nodecoeffs(void);                 /* Computes node coeffs.      */
void    valvecoeffs(void);                /* Computes valve coeffs.     */
void    pipecoeff(int);                   /* Computes pipe coeff.       */
double  DWcoeff(int, double *);           /* Computes D-W coeff.        */
void    pumpcoeff(int);                   /* Computes pump coeff.       */

/*** Updated 10/25/00 ***/
/*** Updated 12/29/00 ***/
void    curvecoeff(int,double,double *,   /* Computes curve coeffs.     */
                             double *);

void    gpvcoeff(int);                    /* Computes GPV coeff.        */
void    pbvcoeff(int);                    /* Computes PBV coeff.        */
void    tcvcoeff(int);                    /* Computes TCV coeff.        */
void    prvcoeff(int,int,int);            /* Computes PRV coeff.        */
void    psvcoeff(int,int,int);            /* Computes PSV coeff.        */
void    fcvcoeff(int,int,int);            /* Computes FCV coeff.        */
void    emittercoeffs(void);              /* Computes emitter coeffs.   */
double  emitflowchange(int);              /* Computes new emitter flow  */

/* ----------- SMATRIX.C ---------------*/
int     createsparse(void);               /* Creates sparse matrix      */
int     allocsparse(void);                /* Allocates matrix memory    */
void    freesparse(void);                 /* Frees matrix memory        */
int     buildlists(int);                  /* Builds adjacency lists     */
int     paralink(int, int, int);          /* Checks for parallel links  */
void    xparalinks(void);                 /* Removes parallel links     */
void    freelists(void);                  /* Frees adjacency lists      */
void    countdegree(void);                /* Counts links at each node  */
int     reordernodes(void);               /* Finds a node re-ordering   */
int     mindegree(int, int);              /* Finds min. degree node     */
int     growlist(int);                    /* Augments adjacency list    */
int     newlink(Padjlist);                /* Adds fill-ins for a node   */
int     linked(int, int);                 /* Checks if 2 nodes linked   */
int     addlink(int, int, int);           /* Creates new fill-in        */
int     storesparse(int);                 /* Stores sparse matrix       */
int     ordersparse(int);                 /* Orders matrix storage      */
void    transpose(int,int *,int *,        /* Transposes sparse matrix   */
        int *,int *,int *,int *,int *);
int     linsolve(int, double *, double *, /* Solution of linear eqns.   */
                 double *);               /* via Cholesky factorization */

/* ----------- QUALITY.C ---------------*/
int     openqual(void);                   /* Opens WQ solver system     */
void    initqual(void);                   /* Initializes WQ solver      */
int     runqual(long *);                  /* Gets current WQ results    */
int     nextqual(long *);                 /* Updates WQ by hyd.timestep */
int     stepqual(long *);                 /* Updates WQ by WQ time step */
int     closequal(void);                  /* Closes WQ solver system    */
int     gethyd(long *, long *);           /* Gets next hyd. results     */
char    setReactflag(void);               /* Checks for reactive chem.  */
void    transport(long);                  /* Transports mass in network */
void    initsegs(void);                   /* Initializes WQ segments    */
void    reorientsegs(void);               /* Re-orients WQ segments     */
void    updatesegs(long);                 /* Updates quality in segments*/
void    removesegs(int);                  /* Removes a WQ segment       */
void    addseg(int,double,double);        /* Adds a WQ segment to pipe  */
void    accumulate(long);                 /* Sums mass flow into node   */
void    updatenodes(long);                /* Updates WQ at nodes        */
void    sourceinput(long);                /* Computes source inputs     */
void    release(long);                    /* Releases mass from nodes   */
void    updatetanks(long);                /* Updates WQ in tanks        */
void    updatesourcenodes(long);          /* Updates WQ at source nodes */
void    tankmix1(int, long);              /* Complete mix tank model    */
void    tankmix2(int, long);              /* 2-compartment tank model   */
void    tankmix3(int, long);              /* FIFO tank model            */
void    tankmix4(int, long);              /* LIFO tank model            */
double  sourcequal(Psource);              /* Finds WQ input from source */
double  avgqual(int);                     /* Finds avg. quality in pipe */
void    ratecoeffs(void);                 /* Finds wall react. coeffs.  */
double  piperate(int);                    /* Finds wall react. coeff.   */
double  pipereact(int,double,double,long);/* Reacts water in a pipe     */
double  tankreact(double,double,double,
                  long);                  /* Reacts water in a tank     */
double  bulkrate(double,double,double);   /* Finds bulk reaction rate   */
double  wallrate(double,double,double,double);/* Finds wall reaction rate   */


/* ------------ OUTPUT.C ---------------*/
int     savenetdata(void);                /* Saves basic data to file   */
int     savehyd(long *);                  /* Saves hydraulic solution   */
int     savehydstep(long *);              /* Saves hydraulic timestep   */
int     saveenergy(void);                 /* Saves energy usage         */
int     readhyd(long *);                  /* Reads hydraulics from file */
int     readhydstep(long *);              /* Reads time step from file  */
int     saveoutput(void);                 /* Saves results to file      */
int     nodeoutput(int, REAL4 *, double); /* Saves node results to file */
int     linkoutput(int, REAL4 *, double); /* Saves link results to file */
int     savefinaloutput(void);            /* Finishes saving output     */
int     savetimestat(REAL4 *, char);      /* Saves time stats to file   */
int     savenetreacts(double, double,
                      double, double);    /* Saves react. rates to file */
int     saveepilog(void);                 /* Saves output file epilog   */


/* ------------ INPFILE.C --------------*/
int     saveinpfile(char *);              /* Saves network to text file  */
