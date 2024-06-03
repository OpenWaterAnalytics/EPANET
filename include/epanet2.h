/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2.h
 Description:  declarations of the legacy style EPANET 2 API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 09/28/2023
 ******************************************************************************
 */

/*
This module contains declarations of the legacy style EPANET API functions, with
version 2.2 updates, that apply only to single threaded applications. A second
set of thread safe API functions that allows one to run concurrent analyses on
multiple EPANET projects can be found in the epanet2_2.h header file. The two
APIs share the same function names and arguments with the difference being that
the thread safe functions use the prefix "EN_" and include an extra argument that
represents the EPANET project being analyzed. To avoid unneccesary repetition,
only the thread safe API functions have been documented. To see a description of
a legacy style API function declared here please refer to its complementary named
function in epanet2_2.h.
*/

#ifndef EPANET2_H
#define EPANET2_H

// The legacy style EPANET API can be compiled with support for either single
// precision or double precision floating point arguments, with the default
// being single precision. To compile for double precision one must #define
// EN_API_FLOAT_TYPE as double both here and in any client code that uses the
// API.
#ifndef EN_API_FLOAT_TYPE
  #define EN_API_FLOAT_TYPE float
#endif

#ifndef DLLEXPORT
  #ifdef _WIN32
    #ifdef epanet2_EXPORTS
      #define DLLEXPORT __declspec(dllexport) __stdcall
    #else
      #define DLLEXPORT __declspec(dllimport) __stdcall
    #endif
  #elif defined(__CYGWIN__)
    #define DLLEXPORT __stdcall
  #else
    #define DLLEXPORT
  #endif
#endif

#include "epanet2_enums.h"

// --- Declare the EPANET toolkit functions
#if defined(__cplusplus)
extern "C" {
#endif


/********************************************************************

    Project Functions

********************************************************************/

  int  DLLEXPORT ENepanet(const char *inpFile, const char *rptFile,
                 const char *outFile, void (*pviewprog) (char *));

  int  DLLEXPORT ENinit(const char *rptFile, const char *outFile,
                 int unitsType, int headlossType);

  int  DLLEXPORT ENopen(const char *inpFile, const char *rptFile,
                 const char *outFile);

  int  DLLEXPORT ENopenX(const char *inpFile, const char *rptFile,
                 const char *outFile);

  int  DLLEXPORT ENgettitle(char *line1, char *line2, char *line3);

  int  DLLEXPORT ENsettitle(const char *line1, const char *line2, const char *line3);

  int  DLLEXPORT ENgetcomment(int object, int index, char *comment);

  int  DLLEXPORT ENsetcomment(int object, int index, const char *comment);

  int  DLLEXPORT ENgetcount(int object, int *count);

  int  DLLEXPORT ENsaveinpfile(const char *filename);

  int  DLLEXPORT ENclose();

/********************************************************************

    Hydraulic Analysis Functions

********************************************************************/

  int  DLLEXPORT ENsolveH();

  int  DLLEXPORT ENsaveH();

  int  DLLEXPORT ENopenH();

  int  DLLEXPORT ENinitH(int initFlag);

  int  DLLEXPORT ENrunH(long *currentTime);

  int  DLLEXPORT ENnextH(long *tStep);

  int  DLLEXPORT ENcloseH();

  int  DLLEXPORT ENsavehydfile(const char *filename);

  int  DLLEXPORT ENusehydfile(const char *filename);

/********************************************************************

    Water Quality Analysis Functions

********************************************************************/

  int  DLLEXPORT ENsolveQ();

  int  DLLEXPORT ENopenQ();

  int  DLLEXPORT ENinitQ(int saveFlag);

  int  DLLEXPORT ENrunQ(long *currentTime);

  int  DLLEXPORT ENnextQ(long *tStep);

  int  DLLEXPORT ENstepQ(long *timeLeft);

  int  DLLEXPORT ENcloseQ();

/********************************************************************

    Reporting Functions

********************************************************************/

  int  DLLEXPORT ENwriteline(const char *line);

  int  DLLEXPORT ENreport();

  int  DLLEXPORT ENcopyreport(const char *filename);

  int  DLLEXPORT ENclearreport();

  int  DLLEXPORT ENresetreport();

  int  DLLEXPORT ENsetreport(const char *format);

  int  DLLEXPORT ENsetstatusreport(int level);

  int  DLLEXPORT ENgetversion(int *version);

  int  DLLEXPORT ENgeterror(int errcode, char *errmsg, int maxLen);

  int  DLLEXPORT ENgetstatistic(int type, EN_API_FLOAT_TYPE* value);

  int  DLLEXPORT ENgetresultindex(int type, int index, int *value);

  int  DLLEXPORT ENtimetonextevent(int *eventType, long *duration, int *elementIndex);

  int DLLEXPORT ENsetreportcallback(void (*callback)(void *userData, void *EN_projectHandle, const char*));
  int DLLEXPORT ENsetreportcallbackuserdata(void *userData);


/********************************************************************

    Analysis Options Functions

********************************************************************/

  int  DLLEXPORT ENgetoption(int option, EN_API_FLOAT_TYPE *value);

  int  DLLEXPORT ENsetoption(int option, EN_API_FLOAT_TYPE value);

  int  DLLEXPORT ENgetflowunits(int *units);

  int  DLLEXPORT ENsetflowunits(int units);

  int  DLLEXPORT ENgettimeparam(int param, long *value);

  int  DLLEXPORT ENsettimeparam(int param, long value);

  int  DLLEXPORT ENgetqualinfo(int *qualType, char *chemName, char *chemUnits,
                 int *traceNode);

  int  DLLEXPORT ENgetqualtype(int *qualType, int *traceNode);

  int  DLLEXPORT ENsetqualtype(int qualType, const char *chemName,
                 const char *chemUnits, const char *traceNode);

/********************************************************************

    Node Functions

********************************************************************/

   int DLLEXPORT ENaddnode(const char *id, int nodeType, int *index);

   int DLLEXPORT ENdeletenode(int index, int actionCode);

   int DLLEXPORT ENgetnodeindex(const char *id, int *index);

   int DLLEXPORT ENgetnodeid(int index, char *id);

   int DLLEXPORT ENsetnodeid(int index, const char *newid);

   int DLLEXPORT ENgetnodetype(int index, int *nodeType);

   int DLLEXPORT ENgetnodevalue(int index, int property, EN_API_FLOAT_TYPE *value);

   int DLLEXPORT ENgetnodevalues(int property, EN_API_FLOAT_TYPE *value);

   int DLLEXPORT ENsetnodevalue(int index, int property, EN_API_FLOAT_TYPE value);

   int DLLEXPORT ENsetjuncdata(int index, EN_API_FLOAT_TYPE elev,
                 EN_API_FLOAT_TYPE dmnd, const char *dmndpat);

  int  DLLEXPORT ENsettankdata(int index, EN_API_FLOAT_TYPE elev,
                 EN_API_FLOAT_TYPE initlvl, EN_API_FLOAT_TYPE minlvl,
                 EN_API_FLOAT_TYPE maxlvl, EN_API_FLOAT_TYPE diam,
                 EN_API_FLOAT_TYPE minvol, const char *volcurve);

  int  DLLEXPORT ENgetcoord(int index, double *x, double *y);

  int  DLLEXPORT ENsetcoord(int index, double x, double y);

/********************************************************************

    Nodal Demand Functions

********************************************************************/

  int DLLEXPORT ENgetdemandmodel(int *model, EN_API_FLOAT_TYPE *pmin,
                EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp);

  int DLLEXPORT ENsetdemandmodel(int model, EN_API_FLOAT_TYPE pmin,
                EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp);

  int DLLEXPORT ENadddemand(int nodeIndex, EN_API_FLOAT_TYPE baseDemand,
                const char *demandPattern, const char *demandName);

  int DLLEXPORT ENdeletedemand(int nodeIndex, int demandIndex);

  int DLLEXPORT ENgetnumdemands(int nodeIndex, int *numDemands);

  int DLLEXPORT ENgetdemandindex(int nodeIndex, const char *demandName,
                int *demandIndex);

  int DLLEXPORT ENgetbasedemand(int nodeIndex, int demandIndex,
                EN_API_FLOAT_TYPE *baseDemand);

  int DLLEXPORT ENsetbasedemand(int nodeIndex, int demandIndex,
                EN_API_FLOAT_TYPE baseDemand);

  int DLLEXPORT ENgetdemandpattern(int nodeIndex, int demandIndex, int *patIndex);

  int DLLEXPORT ENsetdemandpattern(int nodeIndex, int demandIndex, int patIndex);

  int DLLEXPORT ENgetdemandname(int nodeIndex, int demandIndex, char *demandName);

  int DLLEXPORT ENsetdemandname(int nodeIndex, int demandIndex, const char *demandName);

/********************************************************************

    Link Functions

********************************************************************/

  int DLLEXPORT ENaddlink(const char *id, int linkType, const char *fromNode,
                const char *toNode, int *index);

  int DLLEXPORT ENdeletelink(int index, int actionCode);

  int DLLEXPORT ENgetlinkindex(const char *id, int *index);

  int DLLEXPORT ENgetlinkid(int index, char *id);

  int DLLEXPORT ENsetlinkid(int index, const char *newid);

  int DLLEXPORT ENgetlinktype(int index, int *linkType);

  int DLLEXPORT ENsetlinktype(int *index, int linkType, int actionCode);

  int DLLEXPORT ENgetlinknodes(int index, int *node1, int *node2);

  int DLLEXPORT ENsetlinknodes(int index, int node1, int node2);

  int DLLEXPORT ENgetlinkvalue(int index, int property, EN_API_FLOAT_TYPE *value);

  int DLLEXPORT ENgetlinkvalues(int property, EN_API_FLOAT_TYPE *value);

  int DLLEXPORT ENsetlinkvalue(int index, int property, EN_API_FLOAT_TYPE value);

  int DLLEXPORT ENsetpipedata(int index, EN_API_FLOAT_TYPE length,
                EN_API_FLOAT_TYPE diam, EN_API_FLOAT_TYPE rough,
                EN_API_FLOAT_TYPE mloss);

  int DLLEXPORT ENgetvertexcount(int index, int *count);

  int DLLEXPORT ENgetvertex(int index, int vertex, double *x, double *y);

  int DLLEXPORT ENsetvertex(int index, int vertex, double x, double y);

  int DLLEXPORT ENsetvertices(int index, double *x, double *y, int count);

/********************************************************************

    Pump Functions

********************************************************************/

  int DLLEXPORT ENgetpumptype(int linkIndex, int *pumpType);

  int DLLEXPORT ENgetheadcurveindex(int linkIndex, int *curveIndex);

  int DLLEXPORT ENsetheadcurveindex(int linkIndex, int curveIndex);

/********************************************************************

    Time Pattern Functions

********************************************************************/

  int DLLEXPORT ENaddpattern(const char *id);

  int DLLEXPORT ENdeletepattern(int index);

  int DLLEXPORT ENgetpatternindex(const char *id, int *index);

  int DLLEXPORT ENgetpatternid(int index, char *id);

  int DLLEXPORT ENsetpatternid(int index, const char *id);

  int DLLEXPORT ENgetpatternlen(int index, int *len);

  int DLLEXPORT ENgetpatternvalue(int index, int period, EN_API_FLOAT_TYPE *value);

  int DLLEXPORT ENsetpatternvalue(int index, int period, EN_API_FLOAT_TYPE value);

  int DLLEXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value);

  int DLLEXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *values, int len);

/********************************************************************

    Data Curve Functions

********************************************************************/

  int DLLEXPORT ENaddcurve(const char *id);

  int DLLEXPORT ENdeletecurve(int index);

  int DLLEXPORT ENgetcurveindex(const char *id, int *index);

  int DLLEXPORT ENgetcurveid(int index, char *id);

  int DLLEXPORT ENsetcurveid(int index, const char *id);

  int DLLEXPORT ENgetcurvelen(int index, int *len);

  int DLLEXPORT ENgetcurvetype(int index, int *type);

  int DLLEXPORT ENsetcurvetype(int index, int type);

  int DLLEXPORT ENgetcurvevalue(int curveIndex, int pointIndex,
                EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);

  int DLLEXPORT ENsetcurvevalue(int curveIndex, int pointIndex,
                EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);

  int DLLEXPORT ENgetcurve(int index, char* id, int *nPoints,
                EN_API_FLOAT_TYPE *xValues, EN_API_FLOAT_TYPE *yValues);

  int DLLEXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *xValues,
                EN_API_FLOAT_TYPE *yValues, int nPoints);

/********************************************************************

    Simple Controls Functions

********************************************************************/

  int DLLEXPORT ENaddcontrol(int type, int linkIndex, EN_API_FLOAT_TYPE setting,
                int nodeIndex, EN_API_FLOAT_TYPE level, int *index);

  int DLLEXPORT ENdeletecontrol(int index);

  int DLLEXPORT ENgetcontrol(int index, int *type, int *linkIndex,
                EN_API_FLOAT_TYPE *setting, int *nodeIndex, EN_API_FLOAT_TYPE *level);

  int DLLEXPORT ENsetcontrol(int index, int type, int linkIndex,
                EN_API_FLOAT_TYPE setting, int nodeIndex, EN_API_FLOAT_TYPE level);

  int DLLEXPORT ENgetcontrolenabled(int index, int *out_enabled);

  int DLLEXPORT ENsetcontrolenabled(int index, int enabled);

/********************************************************************

    Rule-Based Controls Functions

********************************************************************/

  int DLLEXPORT ENaddrule(char *rule);

  int DLLEXPORT ENdeleterule(int index);

  int DLLEXPORT ENgetrule(int index, int *nPremises, int *nThenActions,
                int *nElseActions, EN_API_FLOAT_TYPE *priority);

  int DLLEXPORT ENgetruleID(int index, char* id);

  int DLLEXPORT ENgetpremise(int ruleIndex, int premiseIndex, int *logop,
                int *object, int *objIndex, int *variable,
                int *relop, int *status, EN_API_FLOAT_TYPE *value);

  int DLLEXPORT ENsetpremise(int ruleIndex, int premiseIndex, int logop,
                int object, int objIndex, int variable, int relop,
                int status, EN_API_FLOAT_TYPE value);

  int DLLEXPORT ENsetpremiseindex(int ruleIndex, int premiseIndex, int objIndex);

  int DLLEXPORT ENsetpremisestatus(int ruleIndex, int premiseIndex, int status);

  int DLLEXPORT ENsetpremisevalue(int ruleIndex, int premiseIndex,
                EN_API_FLOAT_TYPE value);

  int DLLEXPORT ENgetthenaction(int ruleIndex, int actionIndex, int *linkIndex,
                int *status, EN_API_FLOAT_TYPE *setting);

  int DLLEXPORT ENsetthenaction(int ruleIndex, int actionIndex, int linkIndex,
                int status, EN_API_FLOAT_TYPE setting);

  int DLLEXPORT ENgetelseaction(int ruleIndex, int actionIndex, int *linkIndex,
                int *status, EN_API_FLOAT_TYPE *setting);

  int DLLEXPORT ENsetelseaction(int ruleIndex, int actionIndex, int linkIndex,
                int status, EN_API_FLOAT_TYPE setting);

  int DLLEXPORT ENsetrulepriority(int index, EN_API_FLOAT_TYPE priority);

  int DLLEXPORT ENgetruleenabled(int index, int *out_enabled);

  int DLLEXPORT ENsetruleenabled(int index, int enabled);

  #if defined(__cplusplus)
  }
  #endif

#endif //EPANET2_H
