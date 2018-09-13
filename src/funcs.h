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

#ifndef FUNCS_H
#define FUNCS_H

#include "types.h"

void    initpointers(EN_Project *pr);              /* Initializes pointers       */
int     allocdata(EN_Project *pr);                 /* Allocates memory           */
void    freeTmplist(STmplist *);                   /* Frees items in linked list */
void    freeFloatlist(SFloatlist *);               /* Frees list of floats       */
void    freedata(EN_Project *pr);                  /* Frees allocated memory     */
int     openfiles(EN_Project *pr, const char *, 
        const char *,const char *);                /* Opens input & report files */
int     openhydfile(EN_Project *pr);               /* Opens hydraulics file      */
int     openoutfile(EN_Project *pr);               /* Opens binary output file   */
int     strcomp(const char *, const char *);       /* Compares two strings       */
char*   getTmpName(EN_Project *p, char* fname);    /* Gets temporary file name   */     
double  interp(int n, double x[], double y[],
        double xx);                                /* Interpolates a data curve  */
               
int     findnode(EN_Network *n, char *);           /* Finds node's index from ID */
int     findlink(EN_Network *n, char *);           /* Finds link's index from ID */
int     findtank(EN_Network *n, int);              /* Find tank index from node index */  // (AH)
int     findvalve(EN_Network *n, int);             /* Find valve index from node index */ // (AH)
int     findpump(EN_Network *n, int);              /* Find pump index from node index */  // (AH)
char   *geterrmsg(int errcode, char *msg);         /* Gets text of error message */
void    errmsg(EN_Project *p, int);                /* Reports program error      */
void    writecon(const char *);                    /* Writes text to console     */
void    writewin(void (*vp)(char *), char *);      /* Passes text to calling app */

/* ------- INPUT1.C --------------------*/
int     getdata(EN_Project *pr);                    /* Gets network data          */
void    setdefaults(EN_Project *pr);                /* Sets default values        */
void    initreport(report_options_t *r);            /* Initializes report options */
void    adjustdata(EN_Project *pr);                 /* Adjusts input data         */
int     inittanks(EN_Project *pr);                  /* Initializes tank levels    */
void    initunits(EN_Project *pr);                  /* Determines reporting units */
void    convertunits(EN_Project *pr);               /* Converts data to std. units*/

/* -------- INPUT2.C -------------------*/
int     netsize(EN_Project *pr);                    /* Determines network size    */
int     readdata(EN_Project *pr);                   /* Reads in network data      */
int     newline(EN_Project *pr, int, char *);       /* Processes new line of data */
int     addnodeID(EN_Network *n, int, char *);      /* Adds node ID to data base  */
int     addlinkID(EN_Network *n, int, char *);      /* Adds link ID to data base  */
int     addpattern(parser_data_t *par, char *);     /* Adds pattern to data base  */
int     addcurve(parser_data_t *par, char *);       /* Adds curve to data base    */
STmplist *findID(char *, STmplist *);               /* Locates ID on linked list  */
int     unlinked(EN_Project *pr);                   /* Checks for unlinked nodes  */
int     getpumpparams(EN_Project *pr);              /* Computes pump curve coeffs.*/
int     getpatterns(EN_Project *pr);                /* Gets pattern data from list*/
int     getcurves(EN_Project *pr);                  /* Gets curve data from list  */
int     findmatch(char *, char *[]);                /* Finds keyword in line      */
int     match(const char *, const char *);          /* Checks for word match      */
int     gettokens(char *s, char** Tok, int maxToks,
                  char *comment);                   /* Tokenizes input line       */
int     getfloat(char *, double *);                 /* Converts string to double  */
double  hour(char *, char *);                       /* Converts time to hours     */
int     setreport(EN_Project *pr, char *);          /* Processes reporting command*/
void    inperrmsg(EN_Project *pr, int,int,char *);  /* Input error message        */

/* ---------- INPUT3.C -----------------*/
int     juncdata(EN_Project *pr);                   /* Processes junction data    */
int     tankdata(EN_Project *pr);                   /* Processes tank data        */
int     pipedata(EN_Project *pr);                   /* Processes pipe data        */
int     pumpdata(EN_Project *pr);                   /* Processes pump data        */
int     valvedata(EN_Project *pr);                  /* Processes valve data       */
int     patterndata(EN_Project *pr);                /* Processes pattern data     */
int     curvedata(EN_Project *pr);                  /* Processes curve data       */
int     coordata(EN_Project *pr);                   /* Processes coordinate data  */
int     demanddata(EN_Project *pr);                 /* Processes demand data      */
int     controldata(EN_Project *pr);                /* Processes simple controls  */
int     energydata(EN_Project *pr);                 /* Processes energy data      */
int     sourcedata(EN_Project *pr);                 /* Processes source data      */
int     emitterdata(EN_Project *pr);                /* Processes emitter data     */
int     qualdata(EN_Project *pr);                   /* Processes quality data     */
int     reactdata(EN_Project *pr);                  /* Processes reaction data    */
int     mixingdata(EN_Project *pr);                 /* Processes tank mixing data */
int     statusdata(EN_Project *pr);                 /* Processes link status data */
int     reportdata(EN_Project *pr);                 /* Processes report options   */
int     timedata(EN_Project *pr);                   /* Processes time options     */
int     optiondata(EN_Project *pr);                 /* Processes analysis options */
int     optionchoice(EN_Project *pr, int);          /* Processes option choices   */
int     optionvalue(EN_Project *pr, int);           /* Processes option values    */
int     getpumpcurve(EN_Project *pr, int);          /* Constructs a pump curve    */
int     powercurve(double, double, double,          /* Coeffs. of power pump curve*/
                   double, double, double *,
                   double *, double *);
int     valvecheck(EN_Project *pr, int, int, int);  /* Checks valve placement     */
void    changestatus(EN_Network *net, int, StatType,
                     double);                       /* Changes status of a link   */

/* -------------- RULES.C --------------*/
void    initrules(rules_t *rules);                  /* Initializes rule base      */
void    addrule(parser_data_t *par, char *);        /* Adds rule to rule base     */
int     allocrules(EN_Project *pr);                 /* Allocates memory for rule  */
int     ruledata(EN_Project *pr);                   /* Processes rule input data  */
int     checkrules(EN_Project *pr, long);           /* Checks all rules           */
void    freerules(EN_Project *pr);                  /* Frees rule base memory     */  
int     writeRuleinInp(EN_Project *pr, FILE *f,     /* Writes rule to an INP file */
                      int RuleIdx);

int     writeRuleinInp(EN_Project *pr, FILE *f, int RuleIdx);

/* ------------- REPORT.C --------------*/
int     writereport(EN_Project *pr);                /* Writes formatted report    */
void    writelogo(EN_Project *pr);                  /* Writes program logo        */
void    writesummary(EN_Project *pr);               /* Writes network summary     */
void    writehydstat(EN_Project *pr, int,double);   /* Writes hydraulic status    */
void    writeenergy(EN_Project *pr);                /* Writes energy usage        */
int     writeresults(EN_Project *pr);               /* Writes node/link results   */
void    writeheader(EN_Project *pr, int,int);       /* Writes heading on report   */
void    writeline(EN_Project *pr, char *);          /* Writes line to report file */
void    writerelerr(EN_Project *pr, int, double);   /* Writes convergence error   */
void    writestatchange(EN_Project *pr, int,char,char);   /* Writes link status change  */
void    writecontrolaction(EN_Project *pr, int, int);     /* Writes control action taken*/
void    writeruleaction(EN_Project *pr, int, char *);     /* Writes rule action taken   */
int     writehydwarn(EN_Project *pr, int,double);         /* Writes hydraulic warnings  */
void    writehyderr(EN_Project *pr, int);                 /* Writes hydraulic error msg.*/
int     disconnected(EN_Project *pr);                     /* Checks for disconnections  */
void    marknodes(EN_Project *pr, int, int *, char *);    /* Identifies connected nodes */
void    getclosedlink(EN_Project *pr, int, char *);       /* Finds a disconnecting link */
void    writelimits(EN_Project *pr, int,int);             /* Writes reporting limits    */
int     checklimits(report_options_t *rep, double *,
                    int,int);                             /* Checks variable limits     */
void    writetime(EN_Project *pr, char *);                /* Writes current clock time  */
char    *clocktime(char *, long);                         /* Converts time to hrs:min   */
char    *fillstr(char *, char, int);                      /* Fills string with character*/
int     getnodetype(EN_Network *net, int);                /* Determines node type       */

/* --------- HYDRAUL.C -----------------*/
int     openhyd(EN_Project *pr);                    /* Opens hydraulics solver    */
void    inithyd(EN_Project *pr, int initFlags);     /* Re-sets initial conditions */
int     runhyd(EN_Project *pr, long *);             /* Solves 1-period hydraulics */
int     nexthyd(EN_Project *pr, long *);            /* Moves to next time period  */
void    closehyd(EN_Project *pr);                   /* Closes hydraulics solver   */
void    setlinkstatus(EN_Project *pr, int, char,
                      StatType *, double *);        /* Sets link status           */
void    setlinksetting(EN_Project *pr, int, double,
                       StatType *, double *);       /* Sets pump/valve setting    */
int     tanktimestep(EN_Project *pr, long *);       /* Time till tanks fill/drain */
void    getenergy(EN_Project *pr, int, double *,
                  double *);                        /* Computes link energy use   */
double  tankvolume(EN_Project *pr, int,double);     /* Finds tank vol. from grade */
double  tankgrade(EN_Project *pr, int,double);      /* Finds tank grade from vol. */

/* ----------- HYDSOLVER.C -  ----------*/
int     hydsolve(EN_Project *pr, int *,double *);   /* Solves network equations   */

/* ----------- HYDCOEFFS.C --------------*/
void    resistcoeff(EN_Project *pr, int k);         /* Finds pipe flow resistance */
void    headlosscoeffs(EN_Project *pr);             // Finds link head loss coeffs.
void    matrixcoeffs(EN_Project *pr);               /* Finds hyd. matrix coeffs.  */
double  emitflowchange(EN_Project *pr, int i);      /* Change in emitter outflow  */
double  demandflowchange(EN_Project *pr, int i,     // Change in demand outflow
                         double dp, double n);
void    demandparams(EN_Project *pr, double *dp,    // PDA function parameters
                     double *n); 

/* ----------- SMATRIX.C ---------------*/
int     createsparse(EN_Project *pr);               /* Creates sparse matrix      */
void    freesparse(EN_Project *pr);                 /* Frees matrix memory        */
int     linsolve(EN_Project *pr, int);              /* Solves set of linear eqns. */

/* ----------- QUALITY.C ---------------*/
int     openqual(EN_Project *pr);                   /* Opens WQ solver system     */
void    initqual(EN_Project *pr);                   /* Initializes WQ solver      */
int     runqual(EN_Project *pr, long *);            /* Gets current WQ results    */
int     nextqual(EN_Project *pr, long *);           /* Updates WQ by hyd.timestep */
int     stepqual(EN_Project *pr, long *);           /* Updates WQ by WQ time step */
int     closequal(EN_Project *pr);                  /* Closes WQ solver system    */
int     gethyd(EN_Project *pr, long *, long *);     /* Gets next hyd. results     */
char    setReactflag(EN_Project *pr);               /* Checks for reactive chem.  */
void    transport(EN_Project *pr, long);            /* Transports mass in network */
void    initsegs(EN_Project *pr);                   /* Initializes WQ segments    */
void    reorientsegs(EN_Project *pr);               /* Re-orients WQ segments     */
void    updatesegs(EN_Project *pr, long);           /* Updates quality in segments*/
void    removesegs(EN_Project *pr, int);            /* Removes a WQ segment       */
void    addseg(EN_Project *pr, int,double,double);  /* Adds a WQ segment to pipe  */
void    accumulate(EN_Project *pr, long);           /* Sums mass flow into node   */
void    updatenodes(EN_Project *pr, long);          /* Updates WQ at nodes        */
void    sourceinput(EN_Project *pr, long);          /* Computes source inputs     */
void    release(EN_Project *pr, long);              /* Releases mass from nodes   */
void    updatetanks(EN_Project *pr, long);          /* Updates WQ in tanks        */
void    updatesourcenodes(EN_Project *pr, long);    /* Updates WQ at source nodes */
void    tankmix1(EN_Project *pr, int, long);        /* Complete mix tank model    */
void    tankmix2(EN_Project *pr, int, long);        /* 2-compartment tank model   */
void    tankmix3(EN_Project *pr, int, long);        /* FIFO tank model            */
void    tankmix4(EN_Project *pr, int, long);        /* LIFO tank model            */
double  sourcequal(EN_Project *pr, Psource);        /* Finds WQ input from source */
double  avgqual(EN_Project *pr, int);               /* Finds avg. quality in pipe */
void    ratecoeffs(EN_Project *pr);                 /* Finds wall react. coeffs.  */
double  piperate(EN_Project *pr, int);              /* Finds wall react. coeff.   */
double  pipereact(EN_Project *pr, int,double,
                  double,long);                     /* Reacts water in a pipe     */
double  tankreact(EN_Project *pr, double,double,
                  double,long);                     /* Reacts water in a tank     */
double  bulkrate(EN_Project *pr, double,double,
                double);                            /* Finds bulk reaction rate   */
double  wallrate(EN_Project *pr, double,double,
                 double,double);                    /* Finds wall reaction rate   */

/* ------------ OUTPUT.C ---------------*/
int     savenetdata(EN_Project *pr);                /* Saves basic data to file   */
int     savehyd(EN_Project *pr, long *);            /* Saves hydraulic solution   */
int     savehydstep(EN_Project *pr, long *);        /* Saves hydraulic timestep   */
int     saveenergy(EN_Project *pr);                 /* Saves energy usage         */
int     readhyd(EN_Project *pr, long *);            /* Reads hydraulics from file */
int     readhydstep(FILE *hydFile, long *);         /* Reads time step from file  */
int     saveoutput(EN_Project *pr);                 /* Saves results to file      */
int     nodeoutput(EN_Project *pr, int, REAL4 *,
                   double);                         /* Saves node results to file */
int     linkoutput(EN_Project *pr, int, REAL4 *,
                   double);                         /* Saves link results to file */
int     savefinaloutput(EN_Project *pr);            /* Finishes saving output     */
int     savetimestat(EN_Project *pr, REAL4 *,
                     HdrType);                      /* Saves time stats to file   */
int     savenetreacts(EN_Project *pr, double,
                      double,double, double);       /* Saves react. rates to file */
int     saveepilog(EN_Project *pr);                 /* Saves output file epilog   */


/* ------------ INPFILE.C --------------*/
int     saveinpfile(EN_Project *pr, const char *);  /* Saves network to text file  */

#endif
