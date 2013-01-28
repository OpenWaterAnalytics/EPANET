#ifndef LEMONTIGER_H
#define LEMONTIGER_H

/*The header file to be included for USING Epanet LemonTiger DLL on WINDOWS platforms,
all original Epanet functions remain intact, and the new LT_functions are added. */

/*Note that this file is not used by the functions in the toolkit itself.
Refer to toolkit.h for the internally used function declarations. */

#define DLLIMPORT __declspec(dllimport) 

// --- Define the EPANET toolkit constants

#define EN_ELEVATION    0    /* Node parameters */
#define EN_BASEDEMAND   1
#define EN_PATTERN      2
#define EN_EMITTER      3
#define EN_INITQUAL     4
#define EN_SOURCEQUAL   5
#define EN_SOURCEPAT    6
#define EN_SOURCETYPE   7
#define EN_TANKLEVEL    8
#define EN_DEMAND       9
#define EN_HEAD         10
#define EN_PRESSURE     11
#define EN_QUALITY      12
#define EN_SOURCEMASS   13
#define EN_INITVOLUME   14
#define EN_MIXMODEL     15
#define EN_MIXZONEVOL   16

#define EN_TANKDIAM     17
#define EN_MINVOLUME    18
#define EN_VOLCURVE     19
#define EN_MINLEVEL     20
#define EN_MAXLEVEL     21
#define EN_MIXFRACTION  22
#define EN_TANK_KBULK   23

#define EN_TANKVOLUME   24     /* TNT */

#define EN_DIAMETER     0    /* Link parameters */
#define EN_LENGTH       1
#define EN_ROUGHNESS    2
#define EN_MINORLOSS    3
#define EN_INITSTATUS   4
#define EN_INITSETTING  5
#define EN_KBULK        6
#define EN_KWALL        7
#define EN_FLOW         8
#define EN_VELOCITY     9
#define EN_HEADLOSS     10
#define EN_STATUS       11
#define EN_SETTING      12
#define EN_ENERGY       13
#define EN_LINKQUAL     14     /* TNT */

#define EN_DURATION     0    /* Time parameters */
#define EN_HYDSTEP      1
#define EN_QUALSTEP     2
#define EN_PATTERNSTEP  3
#define EN_PATTERNSTART 4
#define EN_REPORTSTEP   5
#define EN_REPORTSTART  6
#define EN_RULESTEP     7
#define EN_STATISTIC    8
#define EN_PERIODS      9
#define EN_STARTTIME    10   /* Added TNT 10/2/2009 */

#define EN_NODECOUNT    0    /* Component counts */
#define EN_TANKCOUNT    1
#define EN_LINKCOUNT    2
#define EN_PATCOUNT     3
#define EN_CURVECOUNT   4
#define EN_CONTROLCOUNT 5

#define EN_JUNCTION     0    /* Node types */
#define EN_RESERVOIR    1
#define EN_TANK         2

#define EN_CVPIPE       0    /* Link types */
#define EN_PIPE         1
#define EN_PUMP         2
#define EN_PRV          3
#define EN_PSV          4
#define EN_PBV          5
#define EN_FCV          6
#define EN_TCV          7
#define EN_GPV          8

#define EN_NONE         0    /* Quality analysis types */
#define EN_CHEM         1
#define EN_AGE          2
#define EN_TRACE        3

#define EN_CONCEN       0    /* Source quality types */
#define EN_MASS         1
#define EN_SETPOINT     2
#define EN_FLOWPACED    3

#define EN_CFS          0    /* Flow units types */
#define EN_GPM          1
#define EN_MGD          2
#define EN_IMGD         3
#define EN_AFD          4
#define EN_LPS          5
#define EN_LPM          6
#define EN_MLD          7
#define EN_CMH          8
#define EN_CMD          9

#define EN_TRIALS       0   /* Misc. options */
#define EN_ACCURACY     1
#define EN_TOLERANCE    2
#define EN_EMITEXPON    3
#define EN_DEMANDMULT   4

#define EN_LOWLEVEL     0   /* Control types */
#define EN_HILEVEL      1
#define EN_TIMER        2
#define EN_TIMEOFDAY    3

#define EN_AVERAGE      1   /* Time statistic types.    */
#define EN_MINIMUM      2 
#define EN_MAXIMUM      3
#define EN_RANGE        4

#define EN_MIX1         0   /* Tank mixing models */
#define EN_MIX2         1
#define EN_FIFO         2
#define EN_LIFO         3

#define EN_NOSAVE       0   /* Save-results-to-file flag */
#define EN_SAVE         1
#define EN_INITFLOW     10  /* Re-initialize flow flag   */

// --- declare the EPANET toolkit functions

#ifdef __cplusplus
extern "C" {
#endif

 int   DLLIMPORT ENepanet(char *, char *, char *, void (*) (char *));
 int   DLLIMPORT ENopen(char *, char *, char *);
 int   DLLIMPORT ENsaveinpfile(char *);
 int   DLLIMPORT ENclose(void);

 int   DLLIMPORT ENsolveH(void);
 int   DLLIMPORT ENsaveH(void);
 int   DLLIMPORT ENopenH(void);
 int   DLLIMPORT ENinitH(int);
 int   DLLIMPORT ENrunH(long *);
 int   DLLIMPORT ENnextH(long *);
 int   DLLIMPORT ENcloseH(void);
 int   DLLIMPORT ENsavehydfile(char *);
 int   DLLIMPORT ENusehydfile(char *);

 int   DLLIMPORT ENsolveQ(void);
 int   DLLIMPORT ENopenQ(void);
 int   DLLIMPORT ENinitQ(int);
 int   DLLIMPORT ENrunQ(long *);
 int   DLLIMPORT ENnextQ(long *);
 int   DLLIMPORT ENstepQ(long *);
 int   DLLIMPORT ENcloseQ(void);

 int   DLLIMPORT ENwriteline(char *);
 int   DLLIMPORT ENreport(void);
 int   DLLIMPORT ENresetreport(void);
 int   DLLIMPORT ENsetreport(char *);

 int   DLLIMPORT ENgetcontrol(int, int *, int *, float *,
                      int *, float *);
 int   DLLIMPORT ENgetcount(int, int *);
 int   DLLIMPORT ENgetoption(int, float *);
 int   DLLIMPORT ENgettimeparam(int, long *);
 int   DLLIMPORT ENgetflowunits(int *);
 int   DLLIMPORT ENgetpatternindex(char *, int *);
 int   DLLIMPORT ENgetpatternid(int, char *);
 int   DLLIMPORT ENgetpatternlen(int, int *);
 int   DLLIMPORT ENgetpatternvalue(int, int, float *);
 int   DLLIMPORT ENgetqualtype(int *, int *);
 int   DLLIMPORT ENgeterror(int, char *, int);

 int   DLLIMPORT ENgetnodeindex(char *, int *);
 int   DLLIMPORT ENgetnodeid(int, char *);
 int   DLLIMPORT ENgetnodetype(int, int *);
 int   DLLIMPORT ENgetnodevalue(int, int, float *);

 int   DLLIMPORT ENgetnumdemands(int, int *);
 int   DLLIMPORT ENgetbasedemand(int, int, float *);
 int   DLLIMPORT ENgetdemandpattern(int, int, int *);

 int   DLLIMPORT ENgetlinkindex(char *, int *);
 int   DLLIMPORT ENgetlinkid(int, char *);
 int   DLLIMPORT ENgetlinktype(int, int *);
 int   DLLIMPORT ENgetlinknodes(int, int *, int *);
 int   DLLIMPORT ENgetlinkvalue(int, int, float *);
  
 int   DLLIMPORT ENgetcurve(int curveIndex, int *nValues, float **xValues, float **yValues);
  
 int   DLLIMPORT ENgetversion(int *);

 int   DLLIMPORT ENsetcontrol(int, int, int, float, int, float);
 int   DLLIMPORT ENsetnodevalue(int, int, float);
 int   DLLIMPORT ENsetlinkvalue(int, int, float);
 int   DLLIMPORT ENaddpattern(char *);
 int   DLLIMPORT ENsetpattern(int, float *, int);
 int   DLLIMPORT ENsetpatternvalue(int, int, float);
 int   DLLIMPORT ENsettimeparam(int, long);
 int   DLLIMPORT ENsetoption(int, float);
 int   DLLIMPORT ENsetstatusreport(int);
 int   DLLIMPORT ENsetqualtype(int, char *, char *, char *);

 //LemonTiger functions
 /* See testLT.c for a LemonTiger test */

	//LT equivalent to ENopenH() + ENopenQ() + ENinitH() + ENinitQ()
	int DLLIMPORT ENopeninitHQ();

	//LT equivalent to ENrunQ() + ENnextQ();
	int DLLIMPORT ENrunnextHQ(long*, long*);

	//LT equivalent to ENrunQ() + ENstepQ();
	int DLLIMPORT ENrunstepHQ(long*, long*);

	//LT equivalent to ENcloseH() + ENcloseQ();
	int DLLIMPORT ENcloseHQ();


#ifdef __cplusplus
};
#endif



#endif  //LEMONTIGER_H



