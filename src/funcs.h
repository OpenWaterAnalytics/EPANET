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

void    initpointers(Project *pr);              /* Initializes pointers       */
int     allocdata(Project *pr);                 /* Allocates memory           */
void    freeTmplist(STmplist *);                   /* Frees items in linked list */
void    freeFloatlist(SFloatlist *);               /* Frees list of floats       */
void    freedata(Project *pr);                  /* Frees allocated memory     */
int     openfiles(Project *pr, const char *, 
        const char *,const char *);                /* Opens input & report files */
int     openhydfile(Project *pr);               /* Opens hydraulics file      */
int     openoutfile(Project *pr);               /* Opens binary output file   */
int     strcomp(const char *, const char *);       /* Compares two strings       */
char*   getTmpName(Project *p, char* fname);    /* Gets temporary file name   */     
double  interp(int n, double x[], double y[],
        double xx);                                /* Interpolates a data curve  */
               
int     findnode(EN_Network *n, char *);           /* Finds node's index from ID */
int     findlink(EN_Network *n, char *);           /* Finds link's index from ID */
int     findtank(EN_Network *n, int);              /* Find tank index from node index */  // (AH)
int     findvalve(EN_Network *n, int);             /* Find valve index from node index */ // (AH)
int     findpump(EN_Network *n, int);              /* Find pump index from node index */  // (AH)
char   *geterrmsg(int errcode, char *msg);         /* Gets text of error message */
void    errmsg(Project *p, int);                /* Reports program error      */
void    writewin(void (*vp)(char *), char *);      /* Passes text to calling app */

/* ------- INPUT1.C --------------------*/
int     getdata(Project *pr);                    /* Gets network data          */
void    setdefaults(Project *pr);                /* Sets default values        */
void    initreport(report_options_t *r);            /* Initializes report options */
void    adjustdata(Project *pr);                 /* Adjusts input data         */
int     inittanks(Project *pr);                  /* Initializes tank levels    */
void    initunits(Project *pr);                  /* Determines reporting units */
void    convertunits(Project *pr);               /* Converts data to std. units*/

/* -------- INPUT2.C -------------------*/
int     netsize(Project *pr);                    /* Determines network size    */
int     readdata(Project *pr);                   /* Reads in network data      */
int     newline(Project *pr, int, char *);       /* Processes new line of data */
int     addnodeID(EN_Network *n, int, char *);      /* Adds node ID to data base  */
int     addlinkID(EN_Network *n, int, char *);      /* Adds link ID to data base  */
int     addpattern(parser_data_t *par, char *);     /* Adds pattern to data base  */
int     addcurve(parser_data_t *par, char *);       /* Adds curve to data base    */
STmplist *findID(char *, STmplist *);               /* Locates ID on linked list  */
int     unlinked(Project *pr);                   /* Checks for unlinked nodes  */
int     getpumpparams(Project *pr);              /* Computes pump curve coeffs.*/
int     updatepumpparams(Project *pr, int);      // Updates pump curve coeffs.
int     getpatterns(Project *pr);                /* Gets pattern data from list*/
int     getcurves(Project *pr);                  /* Gets curve data from list  */
int     findmatch(char *, char *[]);                /* Finds keyword in line      */
int     match(const char *, const char *);          /* Checks for word match      */
int     gettokens(char *s, char** Tok, int maxToks,
                  char *comment);                   /* Tokenizes input line       */
int     getfloat(char *, double *);                 /* Converts string to double  */
double  hour(char *, char *);                       /* Converts time to hours     */
int     setreport(Project *pr, char *);          /* Processes reporting command*/
void    inperrmsg(Project *pr, int,int,char *);  /* Input error message        */

/* ---------- INPUT3.C -----------------*/
int     juncdata(Project *pr);                   /* Processes junction data    */
int     tankdata(Project *pr);                   /* Processes tank data        */
int     pipedata(Project *pr);                   /* Processes pipe data        */
int     pumpdata(Project *pr);                   /* Processes pump data        */
int     valvedata(Project *pr);                  /* Processes valve data       */
int     patterndata(Project *pr);                /* Processes pattern data     */
int     curvedata(Project *pr);                  /* Processes curve data       */
int     coordata(Project *pr);                   /* Processes coordinate data  */
int     demanddata(Project *pr);                 /* Processes demand data      */
int     controldata(Project *pr);                /* Processes simple controls  */
int     energydata(Project *pr);                 /* Processes energy data      */
int     sourcedata(Project *pr);                 /* Processes source data      */
int     emitterdata(Project *pr);                /* Processes emitter data     */
int     qualdata(Project *pr);                   /* Processes quality data     */
int     reactdata(Project *pr);                  /* Processes reaction data    */
int     mixingdata(Project *pr);                 /* Processes tank mixing data */
int     statusdata(Project *pr);                 /* Processes link status data */
int     reportdata(Project *pr);                 /* Processes report options   */
int     timedata(Project *pr);                   /* Processes time options     */
int     optiondata(Project *pr);                 /* Processes analysis options */
int     optionchoice(Project *pr, int);          /* Processes option choices   */
int     optionvalue(Project *pr, int);           /* Processes option values    */
int     getpumpcurve(Project *pr, int);          /* Constructs a pump curve    */
int     powercurve(double, double, double,          /* Coeffs. of power pump curve*/
                   double, double, double *,
                   double *, double *);
int     valvecheck(Project *pr, int, int, int);  /* Checks valve placement     */
void    changestatus(EN_Network *net, int, StatType,
                     double);                       /* Changes status of a link   */

/* -------------- RULES.C --------------*/
void    initrules(rules_t *rules);                  /* Initializes rule base      */
void    addrule(parser_data_t *par, char *);        /* Adds rule to rule base     */
int     allocrules(Project *pr);                 /* Allocates memory for rule  */
void    adjustrules(Project *pr, int, int);      // Shifts object indices down
void    adjusttankrules(Project *pr);            // Shifts tank indices up
int     ruledata(Project *pr);                   /* Processes rule input data  */
int     checkrules(Project *pr, long);           /* Checks all rules           */
void    freerules(Project *pr);                  /* Frees rule base memory     */  
int     writeRuleinInp(Project *pr, FILE *f,     /* Writes rule to an INP file */
                      int RuleIdx);

/* ------------- REPORT.C --------------*/
int     writereport(Project *pr);                /* Writes formatted report    */
void    writelogo(Project *pr);                  /* Writes program logo        */
void    writesummary(Project *pr);               /* Writes network summary     */
void    writehydstat(Project *pr, int,double);   /* Writes hydraulic status    */
void    writeenergy(Project *pr);                /* Writes energy usage        */
int     writeresults(Project *pr);               /* Writes node/link results   */
void    writeheader(Project *pr, int,int);       /* Writes heading on report   */
void    writeline(Project *pr, char *);          /* Writes line to report file */
void    writerelerr(Project *pr, int, double);   /* Writes convergence error   */
void    writestatchange(Project *pr, int,char,char);   /* Writes link status change  */
void    writecontrolaction(Project *pr, int, int);     /* Writes control action taken*/
void    writeruleaction(Project *pr, int, char *);     /* Writes rule action taken   */
int     writehydwarn(Project *pr, int,double);         /* Writes hydraulic warnings  */
void    writehyderr(Project *pr, int);                 /* Writes hydraulic error msg.*/
void    writemassbalance(Project *pr);                 // Writes mass balance ratio
int     disconnected(Project *pr);                     /* Checks for disconnections  */
void    marknodes(Project *pr, int, int *, char *);    /* Identifies connected nodes */
void    getclosedlink(Project *pr, int, char *);       /* Finds a disconnecting link */
void    writelimits(Project *pr, int,int);             /* Writes reporting limits    */
int     checklimits(report_options_t *rep, double *,
                    int,int);                             /* Checks variable limits     */
void    writetime(Project *pr, char *);                /* Writes current clock time  */
char    *clocktime(char *, long);                         /* Converts time to hrs:min   */
char    *fillstr(char *, char, int);                      /* Fills string with character*/
int     getnodetype(EN_Network *net, int);                /* Determines node type       */

/* --------- HYDRAUL.C -----------------*/
int     openhyd(Project *pr);                    /* Opens hydraulics solver    */
void    inithyd(Project *pr, int initFlags);     /* Re-sets initial conditions */
int     runhyd(Project *pr, long *);             /* Solves 1-period hydraulics */
int     nexthyd(Project *pr, long *);            /* Moves to next time period  */
void    closehyd(Project *pr);                   /* Closes hydraulics solver   */
void    setlinkstatus(Project *pr, int, char,
                      StatType *, double *);        /* Sets link status           */
void    setlinksetting(Project *pr, int, double,
                       StatType *, double *);       /* Sets pump/valve setting    */
int     tanktimestep(Project *pr, long *);       /* Time till tanks fill/drain */
void    getenergy(Project *pr, int, double *,
                  double *);                        /* Computes link energy use   */
double  tankvolume(Project *pr, int,double);     /* Finds tank vol. from grade */
double  tankgrade(Project *pr, int,double);      /* Finds tank grade from vol. */

/* ----------- HYDSOLVER.C -  ----------*/
int     hydsolve(Project *pr, int *,double *);   /* Solves network equations   */

/* ----------- HYDCOEFFS.C --------------*/
void    resistcoeff(Project *pr, int k);         /* Finds pipe flow resistance */
void    headlosscoeffs(Project *pr);             // Finds link head loss coeffs.
void    matrixcoeffs(Project *pr);               /* Finds hyd. matrix coeffs.  */
void    emitheadloss(Project *pr, int,           // Finds emitter head loss
                     double *, double *);           
double  demandflowchange(Project *pr, int,       // Change in demand outflow
                         double, double);
void    demandparams(Project *pr, double *,      // PDA function parameters
                     double *); 

/* ----------- SMATRIX.C ---------------*/
int     createsparse(Project *pr);               /* Creates sparse matrix      */
void    freesparse(Project *pr);                 /* Frees matrix memory        */
int     linsolve(Project *pr, int);              /* Solves set of linear eqns. */

/* ----------- QUALITY.C ---------------*/
int     openqual(Project *pr);                   /* Opens WQ solver system     */
int     initqual(Project *pr);                   /* Initializes WQ solver      */
int     runqual(Project *pr, long *);            /* Gets current WQ results    */
int     nextqual(Project *pr, long *);           /* Updates WQ by hyd.timestep */
int     stepqual(Project *pr, long *);           /* Updates WQ by WQ time step */
int     closequal(Project *pr);                  /* Closes WQ solver system    */
double  avgqual(Project *pr, int);               /* Finds avg. quality in pipe */

/* ------------ OUTPUT.C ---------------*/
int     savenetdata(Project *pr);                /* Saves basic data to file   */
int     savehyd(Project *pr, long *);            /* Saves hydraulic solution   */
int     savehydstep(Project *pr, long *);        /* Saves hydraulic timestep   */
int     saveenergy(Project *pr);                 /* Saves energy usage         */
int     readhyd(Project *pr, long *);            /* Reads hydraulics from file */
int     readhydstep(Project *pr, long *);        /* Reads time step from file  */
int     saveoutput(Project *pr);                 /* Saves results to file      */
int     nodeoutput(Project *pr, int, REAL4 *,
                   double);                         /* Saves node results to file */
int     linkoutput(Project *pr, int, REAL4 *,
                   double);                         /* Saves link results to file */
int     savefinaloutput(Project *pr);            /* Finishes saving output     */
int     savetimestat(Project *pr, REAL4 *,
                     HdrType);                      /* Saves time stats to file   */
int     savenetreacts(Project *pr, double,
                      double,double, double);       /* Saves react. rates to file */
int     saveepilog(Project *pr);                 /* Saves output file epilog   */


/* ------------ INPFILE.C --------------*/
int     saveinpfile(Project *pr, const char *);  /* Saves network to text file  */

#endif
