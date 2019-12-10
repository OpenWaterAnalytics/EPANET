/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       funcs.h
 Description:  prototypes of external functions called by various modules
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/15/2019
 ******************************************************************************
*/
#ifndef FUNCS_H
#define FUNCS_H

// ------- PROJECT.C ------------

void    initpointers(Project *);
int     allocdata(Project *);
void    freedata(Project *);

int     openfiles(Project *, const char *, const char *,const char *);
int     openhydfile(Project *);
int     openoutfile(Project *);
void    closeoutfile(Project *);

int     buildadjlists(Network *);
void    freeadjlists(Network *);

int     incontrols(Project *, int, int);
int     valvecheck(Project *, int, int, int, int);
int     findnode(Network *, char *);
int     findlink(Network *, char *);
int     findtank(Network *, int);
int     findvalve(Network *, int);
int     findpump(Network *, int);
int     findpattern(Network *, char *);
int     findcurve(Network *, char *);

Pdemand finddemand(Pdemand, int);
int     adddemand(Snode *, double, int, char *);
void    freedemands(Snode *);

int     addlinkvertex(Slink *, double, double);
void    freelinkvertices(Slink *);

void    adjustpatterns(Network *, int);
void    adjustcurves(Network *, int);
int     adjustpumpparams(Project *, int);
int     resizecurve(Scurve *, int);

int     getcomment(Network *, int, int, char *);
int     setcomment(Network *, int, int, const char *);

int     namevalid(const char *);
void    getTmpName(char *);
char    *xstrcpy(char **, const char *, const size_t n);
int     strcomp(const char *, const char *);
double  interp(int, double [], double [], double);
char    *geterrmsg(int, char *);
void    errmsg(Project *, int);
void    writewin(void (*vp)(char *), char *);

// ------- INPUT1.C ----------------

int     getdata(Project *);
void    setdefaults(Project *);
void    initreport(Report *);
void    adjustdata(Project *);
int     inittanks(Project *);
void    initunits(Project *);
void    convertunits(Project *);

//-------- INPUT2.C -----------------

int     netsize(Project *);
int     readdata(Project *);
int     updatepumpparams(Project *, int);
int     findmatch(char *, char *[]);
int     match(const char *, const char *);
int     gettokens(char *, char **, int, char *);
int     getfloat(char *, double *);
double  hour(char *, char *);
int     setreport(Project *, char *);

// ------- INPUT3.C -----------------

int     juncdata(Project *);
int     tankdata(Project *);
int     pipedata(Project *);
int     pumpdata(Project *);
int     valvedata(Project *);
int     patterndata(Project *);
int     curvedata(Project *);
int     coordata(Project *);
int     demanddata(Project *);
int     controldata(Project *);
int     energydata(Project *);
int     sourcedata(Project *);
int     emitterdata(Project *);
int     qualdata(Project *);
int     reactdata(Project *);
int     mixingdata(Project *);
int     statusdata(Project *);
int     reportdata(Project *);
int     timedata(Project *);
int     optiondata(Project *);
int     vertexdata(Project *);

// ------- RULES.C ------------------

void    initrules(Project *);
void    addrule(Parser *, char *);
void    deleterule(Project *, int);
int     allocrules(Project *);
void    freerules(Project *);
int     ruledata(Project *);
void    ruleerrmsg(Project *);
void    adjustrules(Project *, int, int);
void    adjusttankrules(Project *);
Spremise *getpremise(Spremise *, int);
Saction  *getaction(Saction *, int);
int     writerule(Project *, FILE *, int);
int     checkrules(Project *, long);

// ------- REPORT.C -----------------

int     clearreport(Project *);
int     copyreport(Project *, char *);
int     writereport(Project *);
void    writelogo(Project *);
void    writesummary(Project *);
void    writehydstat(Project *, int, double);
void    writeheader(Project *, int,int);
void    writeline(Project *, char *);
void    writerelerr(Project *, int, double);
void    writestatchange(Project *, int,char,char);
void    writecontrolaction(Project *, int, int);
void    writeruleaction(Project *, int, char *);
int     writehydwarn(Project *, int,double);
void    writehyderr(Project *, int);
void    writemassbalance(Project *);
void    writetime(Project *, char *);
char    *clocktime(char *, long);

// ------- HYDRAUL.C -----------------

int     openhyd(Project *);
void    inithyd(Project *, int initFlags);
int     runhyd(Project *, long *);
int     nexthyd(Project *, long *);
void    closehyd(Project *);
void    setlinkstatus(Project *, int, char, StatusType *, double *);
void    setlinksetting(Project *, int, double, StatusType *, double *);
int     tanktimestep(Project *, long *);
void    getenergy(Project *, int, double *, double *);
double  tankvolume(Project *, int, double);
double  tankgrade(Project *, int, double);

// ------- HYDCOEFFS.C -----------------

void    resistcoeff(Project *, int);
void    headlosscoeffs(Project *);
void    matrixcoeffs(Project *);
void    emitterheadloss(Project *, int, double *, double *);           
void    demandheadloss(Project *, int, double, double, double *, double *);

// ------- QUALITY.C --------------------

int     openqual(Project *);
int     initqual(Project *);
int     runqual(Project *, long *);
int     nextqual(Project *, long *);
int     stepqual(Project *, long *);
int     closequal(Project *);
double  avgqual(Project *, int);

// ------- OUTPUT.C ---------------------

int     savenetdata(Project *);
int     savehyd(Project *, long *);
int     savehydstep(Project *, long *);
int     saveenergy(Project *);
int     readhyd(Project *, long *);
int     readhydstep(Project *, long *);
int     saveoutput(Project *);
int     savefinaloutput(Project *);

// ------- INPFILE.C --------------------

int     saveinpfile(Project *, const char *);

#endif
