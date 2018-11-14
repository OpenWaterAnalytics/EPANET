/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       funcs.h
 Description:  prototypes of external functions called by various modules
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/10/2018
 ******************************************************************************
*/
#ifndef FUNCS_H
#define FUNCS_H

/* ------- PROJECT.C -------------------*/
void    initpointers(EN_Project pr);              /* Initializes pointers       */
int     allocdata(EN_Project pr);                 /* Allocates memory           */
void    freeTmplist(STmplist *);                   /* Frees items in linked list */
void    freeFloatlist(SFloatlist *);               /* Frees list of floats       */
void    freedata(EN_Project pr);                  /* Frees allocated memory     */
int     openfiles(EN_Project pr, const char *, 
        const char *,const char *);                /* Opens input & report files */
int     openhydfile(EN_Project pr);               /* Opens hydraulics file      */
int     openoutfile(EN_Project pr);               /* Opens binary output file   */
int     strcomp(const char *, const char *);       /* Compares two strings       */
char*   getTmpName(char* fname);                   /* Gets temporary file name   */     
double  interp(int n, double x[], double y[],
        double xx);                                /* Interpolates a data curve  */
               
int     findnode(network_t *n, char *);           /* Finds node's index from ID */
int     findlink(network_t *n, char *);           /* Finds link's index from ID */
int     findtank(network_t *n, int);              /* Find tank index from node index */
int     findvalve(network_t *n, int);             /* Find valve index from node index */
int     findpump(network_t *n, int);              /* Find pump index from node index */
char   *geterrmsg(int errcode, char *msg);         /* Gets text of error message */
void    errmsg(EN_Project p, int);                /* Reports program error      */
void    writewin(void (*vp)(char *), char *);      /* Passes text to calling app */

/* ------- INPUT1.C --------------------*/
int     getdata(EN_Project pr);                    /* Gets network data          */
void    setdefaults(EN_Project pr);                /* Sets default values        */
void    initreport(report_options_t *r);            /* Initializes report options */
void    adjustdata(EN_Project pr);                 /* Adjusts input data         */
int     inittanks(EN_Project pr);                  /* Initializes tank levels    */
void    initunits(EN_Project pr);                  /* Determines reporting units */
void    convertunits(EN_Project pr);               /* Converts data to std. units*/

/* -------- INPUT2.C -------------------*/
int     netsize(EN_Project pr);                    /* Determines network size    */
int     readdata(EN_Project pr);                   /* Reads in network data      */
int     updatepumpparams(EN_Project pr, int);      // Updates pump curve coeffs.
int     getpatterns(EN_Project pr);                /* Gets pattern data from list*/
int     getcurves(EN_Project pr);                  /* Gets curve data from list  */
int     findmatch(char *, char *[]);                /* Finds keyword in line      */
int     match(const char *, const char *);          /* Checks for word match      */
int     gettokens(char *s, char** Tok, int maxToks,
                  char *comment);                   /* Tokenizes input line       */
int     getfloat(char *, double *);                 /* Converts string to double  */
double  hour(char *, char *);                       /* Converts time to hours     */
int     setreport(EN_Project pr, char *);          /* Processes reporting command*/

/* ---------- INPUT3.C -----------------*/
int     juncdata(EN_Project pr);                   /* Processes junction data    */
int     tankdata(EN_Project pr);                   /* Processes tank data        */
int     pipedata(EN_Project pr);                   /* Processes pipe data        */
int     pumpdata(EN_Project pr);                   /* Processes pump data        */
int     valvedata(EN_Project pr);                  /* Processes valve data       */
int     patterndata(EN_Project pr);                /* Processes pattern data     */
int     curvedata(EN_Project pr);                  /* Processes curve data       */
int     coordata(EN_Project pr);                   /* Processes coordinate data  */
int     demanddata(EN_Project pr);                 /* Processes demand data      */
int     controldata(EN_Project pr);                /* Processes simple controls  */
int     energydata(EN_Project pr);                 /* Processes energy data      */
int     sourcedata(EN_Project pr);                 /* Processes source data      */
int     emitterdata(EN_Project pr);                /* Processes emitter data     */
int     qualdata(EN_Project pr);                   /* Processes quality data     */
int     reactdata(EN_Project pr);                  /* Processes reaction data    */
int     mixingdata(EN_Project pr);                 /* Processes tank mixing data */
int     statusdata(EN_Project pr);                 /* Processes link status data */
int     reportdata(EN_Project pr);                 /* Processes report options   */
int     timedata(EN_Project pr);                   /* Processes time options     */
int     optiondata(EN_Project pr);                 /* Processes analysis options */
int     valvecheck(EN_Project pr, int, int, int);  /* Checks valve placement     */

/* -------------- RULES.C --------------*/
void    initrules(EN_Project pr);                  /* Initializes rule base      */
void    addrule(parser_data_t *par, char *);        /* Adds rule to rule base     */
int     allocrules(EN_Project pr);                 /* Allocates memory for rule  */
void    adjustrules(EN_Project pr, int, int);      // Shifts object indices down
void    adjusttankrules(EN_Project pr);            // Shifts tank indices up
int     ruledata(EN_Project pr);                   /* Processes rule input data  */
int     checkrules(EN_Project pr, long);           /* Checks all rules           */
void    deleterule(EN_Project pr, int);            // Deletes a rule
void    freerules(EN_Project pr);                  /* Frees rule base memory     */
int     writerule(EN_Project pr, FILE *, int);     /* Writes rule to an INP file */
void    ruleerrmsg(EN_Project pr);                 /* Reports rule parser error  */
Spremise *getpremise(Spremise *, int);              // Retrieves a rule's premise
Saction  *getaction(Saction *, int);                // Retrieves a rule's action

/* ------------- REPORT.C --------------*/
int     writereport(EN_Project pr);                /* Writes formatted report    */
void    writelogo(EN_Project pr);                  /* Writes program logo        */
void    writesummary(EN_Project pr);               /* Writes network summary     */
void    writehydstat(EN_Project pr, int,double);   /* Writes hydraulic status    */
void    writeheader(EN_Project pr, int,int);       /* Writes heading on report   */
void    writeline(EN_Project pr, char *);          /* Writes line to report file */
void    writerelerr(EN_Project pr, int, double);   /* Writes convergence error   */
void    writestatchange(EN_Project pr, int,char,char);   /* Writes link status change  */
void    writecontrolaction(EN_Project pr, int, int);     /* Writes control action taken*/
void    writeruleaction(EN_Project pr, int, char *);     /* Writes rule action taken   */
int     writehydwarn(EN_Project pr, int,double);         /* Writes hydraulic warnings  */
void    writehyderr(EN_Project pr, int);                 /* Writes hydraulic error msg.*/
void    writemassbalance(EN_Project pr);                 // Writes mass balance ratio
void    writetime(EN_Project pr, char *);                /* Writes current clock time  */
char    *clocktime(char *, long);                         /* Converts time to hrs:min   */

/* --------- HYDRAUL.C -----------------*/
int     openhyd(EN_Project pr);                    /* Opens hydraulics solver    */
void    inithyd(EN_Project pr, int initFlags);     /* Re-sets initial conditions */
int     runhyd(EN_Project pr, long *);             /* Solves 1-period hydraulics */
int     nexthyd(EN_Project pr, long *);            /* Moves to next time period  */
void    closehyd(EN_Project pr);                   /* Closes hydraulics solver   */
void    setlinkstatus(EN_Project pr, int, char,
                      StatType *, double *);        /* Sets link status           */
void    setlinksetting(EN_Project pr, int, double,
                       StatType *, double *);       /* Sets pump/valve setting    */
int     tanktimestep(EN_Project pr, long *);       /* Time till tanks fill/drain */
void    getenergy(EN_Project pr, int, double *,
                  double *);                        /* Computes link energy use   */
double  tankvolume(EN_Project pr, int, double);    /* Finds tank vol. from grade */
double  tankgrade(EN_Project pr, int, double);     /* Finds tank grade from vol. */

/* ----------- HYDSOLVER.C -  ----------*/
int     hydsolve(EN_Project pr, int *,double *);   /* Solves network equations   */

/* ----------- HYDCOEFFS.C --------------*/
void    resistcoeff(EN_Project pr, int k);         /* Finds pipe flow resistance */
void    headlosscoeffs(EN_Project pr);             // Finds link head loss coeffs.
void    matrixcoeffs(EN_Project pr);               /* Finds hyd. matrix coeffs.  */
void    emitheadloss(EN_Project pr, int,           // Finds emitter head loss
                     double *, double *);           
double  demandflowchange(EN_Project pr, int,       // Change in demand outflow
                         double, double);
void    demandparams(EN_Project pr, double *,      // PDA function parameters
                     double *); 

/* ----------- SMATRIX.C ---------------*/
int     createsparse(EN_Project pr);               /* Creates sparse matrix      */
void    freesparse(EN_Project pr);                 /* Frees matrix memory        */
int     linsolve(EN_Project pr, int);              /* Solves set of linear eqns. */

/* ----------- QUALITY.C ---------------*/
int     openqual(EN_Project pr);                   /* Opens WQ solver system     */
int     initqual(EN_Project pr);                   /* Initializes WQ solver      */
int     runqual(EN_Project pr, long *);            /* Gets current WQ results    */
int     nextqual(EN_Project pr, long *);           /* Updates WQ by hyd.timestep */
int     stepqual(EN_Project pr, long *);           /* Updates WQ by WQ time step */
int     closequal(EN_Project pr);                  /* Closes WQ solver system    */
double  avgqual(EN_Project pr, int);               /* Finds avg. quality in pipe */

/* ------------ OUTPUT.C ---------------*/
int     savenetdata(EN_Project pr);                /* Saves basic data to file   */
int     savehyd(EN_Project pr, long *);            /* Saves hydraulic solution   */
int     savehydstep(EN_Project pr, long *);        /* Saves hydraulic timestep   */
int     saveenergy(EN_Project pr);                 /* Saves energy usage         */
int     readhyd(EN_Project pr, long *);            /* Reads hydraulics from file */
int     readhydstep(EN_Project pr, long *);        /* Reads time step from file  */
int     saveoutput(EN_Project pr);                 /* Saves results to file      */
int     savefinaloutput(EN_Project pr);            /* Finishes saving output     */

/* ------------ INPFILE.C --------------*/
int     saveinpfile(EN_Project pr, const char *);  /* Saves network to text file  */

#endif
