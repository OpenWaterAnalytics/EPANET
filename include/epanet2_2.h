/** @file epanet2.h
 @see http://github.com/openwateranalytics/epanet
 */

/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2.h
 Description:  symbolic constants and function declarations for the EPANET API
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 12/31/2018
 ******************************************************************************
 */

#ifndef EPANET2_2_H
#define EPANET2_2_H

// the toolkit can be compiled with support for double-precision as well.
// just make sure that you use the correct #define in your client code.
#ifndef EN_API_FLOAT_TYPE
  #define EN_API_FLOAT_TYPE float
#endif

#ifdef WITH_GENX
   #include "epanet2_export.h"
#else
  // --- define WINDOWS
  #undef WINDOWS
  #ifdef _WIN32
    #define WINDOWS
  #endif
  #ifdef __WIN32__
    #define WINDOWS
  #endif

  // --- define DLLEXPORT
  #ifndef DLLEXPORT
    #ifdef WINDOWS
      #ifdef __cplusplus
        #define DLLEXPORT __declspec(dllexport)
      #else
        #define DLLEXPORT __declspec(dllexport) __stdcall
      #endif // __cplusplus
    #elif defined(CYGWIN)
      #define DLLEXPORT __stdcall
    #else
      #define DLLEXPORT
    #endif
  #endif
#endif

#include "epanet2_enums.h"

// --- Declare the EPANET toolkit functions
#if defined(__cplusplus)
extern "C" {
#endif

/**
 @brief The EPANET Project wrapper object
*/
typedef struct Project *EN_Project;

/********************************************************************

    Threadsafe versions of all EPANET functions

********************************************************************/

  int DLLEXPORT EN_createproject(EN_Project *ph);
  int DLLEXPORT EN_deleteproject(EN_Project *ph);
  int DLLEXPORT EN_runproject(EN_Project ph, const char *f1, const char *f2, const char *f3,
                void (*pviewprog)(char *));
  int DLLEXPORT EN_init(EN_Project ph, const char *rptFile, const char *outFile,
                EN_FlowUnits unitsType, EN_HeadLossType headLossType);
  int DLLEXPORT EN_open(EN_Project ph, const char *inpFile,
                const char *rptFile, const char *binOutFile);
  int DLLEXPORT EN_saveinpfile(EN_Project ph, const char *filename);
  int DLLEXPORT EN_close(EN_Project ph);

  int DLLEXPORT EN_solveH(EN_Project ph);
  int DLLEXPORT EN_saveH(EN_Project ph);
  int DLLEXPORT EN_openH(EN_Project ph);
  int DLLEXPORT EN_initH(EN_Project ph, int saveFlag);
  int DLLEXPORT EN_runH(EN_Project ph, long *currentTime);
  int DLLEXPORT EN_nextH(EN_Project ph, long *tStep);
  int DLLEXPORT EN_closeH(EN_Project ph);
  int DLLEXPORT EN_savehydfile(EN_Project ph, char *filename);
  int DLLEXPORT EN_usehydfile(EN_Project ph, char *filename);

  int DLLEXPORT EN_solveQ(EN_Project ph);
  int DLLEXPORT EN_openQ(EN_Project ph);
  int DLLEXPORT EN_initQ(EN_Project ph, int saveFlag);
  int DLLEXPORT EN_runQ(EN_Project ph, long *currentTime);
  int DLLEXPORT EN_nextQ(EN_Project ph, long *tStep);
  int DLLEXPORT EN_stepQ(EN_Project ph, long *timeLeft);
  int DLLEXPORT EN_closeQ(EN_Project ph);

  int DLLEXPORT EN_writeline(EN_Project ph, char *line);
  int DLLEXPORT EN_report(EN_Project ph);
  int DLLEXPORT EN_resetreport(EN_Project ph);
  int DLLEXPORT EN_setreport(EN_Project ph, char *reportCommand);
  int DLLEXPORT EN_setstatusreport(EN_Project ph, int code);
  int DLLEXPORT EN_getversion(int *version);
  int DLLEXPORT EN_getcount(EN_Project ph, EN_CountType code, int *count);
  int DLLEXPORT EN_geterror(int errcode, char *errmsg, int maxLen);
  int DLLEXPORT EN_getstatistic(EN_Project ph, int code, EN_API_FLOAT_TYPE* value);

  int DLLEXPORT EN_getoption(EN_Project ph, EN_Option opt, EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_setoption(EN_Project ph, int code, EN_API_FLOAT_TYPE v);
  int DLLEXPORT EN_getflowunits(EN_Project ph, int *code);
  int DLLEXPORT EN_setflowunits(EN_Project ph, int code);
  int DLLEXPORT EN_gettimeparam(EN_Project ph, int code, long *value);
  int DLLEXPORT EN_settimeparam(EN_Project ph, int code, long value);
  int DLLEXPORT EN_getqualinfo(EN_Project ph, int *qualcode, char *chemname,
                char *chemunits, int *tracenode);
  int DLLEXPORT EN_getqualtype(EN_Project ph, int *qualcode, int *tracenode);
  int DLLEXPORT EN_setqualtype(EN_Project ph, int qualcode, char *chemname,
                char *chemunits, char *tracenode);

  int DLLEXPORT EN_addnode(EN_Project ph, char *id, EN_NodeType nodeType);
  int DLLEXPORT EN_deletenode(EN_Project ph, int index, int actionCode);
  int DLLEXPORT EN_getnodeindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getnodeid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_setnodeid(EN_Project ph, int index, char *newid);
  int DLLEXPORT EN_getnodetype(EN_Project ph, int index, int *code);
  int DLLEXPORT EN_getnodevalue(EN_Project ph, int index, int code, EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_setnodevalue(EN_Project ph, int index, int code, EN_API_FLOAT_TYPE v);
  int DLLEXPORT EN_setjuncdata(EN_Project ph, int index, EN_API_FLOAT_TYPE elev,
                EN_API_FLOAT_TYPE dmnd, char *dmndpat);
  int DLLEXPORT EN_settankdata(EN_Project ph, int index, EN_API_FLOAT_TYPE elev,
                EN_API_FLOAT_TYPE initlvl, EN_API_FLOAT_TYPE minlvl,
                EN_API_FLOAT_TYPE maxlvl,  EN_API_FLOAT_TYPE diam,
                EN_API_FLOAT_TYPE minvol, char *volcurve);
  int DLLEXPORT EN_getcoord(EN_Project ph, int index, EN_API_FLOAT_TYPE *x,
                EN_API_FLOAT_TYPE *y);
  int DLLEXPORT EN_setcoord(EN_Project ph, int index, EN_API_FLOAT_TYPE x,
                EN_API_FLOAT_TYPE y);

  int DLLEXPORT EN_getdemandmodel(EN_Project ph, int *type, EN_API_FLOAT_TYPE *pmin,
                EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp);
  int DLLEXPORT EN_setdemandmodel(EN_Project ph, int type, EN_API_FLOAT_TYPE pmin,
                EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp);
  int DLLEXPORT EN_getnumdemands(EN_Project ph, int nodeIndex, int *numDemands);
  int DLLEXPORT EN_getbasedemand(EN_Project ph, int nodeIndex,
                int demandIndex, EN_API_FLOAT_TYPE *baseDemand);
  int DLLEXPORT EN_setbasedemand(EN_Project ph, int nodeIndex,
                int demandIndex, EN_API_FLOAT_TYPE baseDemand);
  int DLLEXPORT EN_getdemandpattern(EN_Project ph, int nodeIndex, int demandIndex,
                int *pattIndex);
  int DLLEXPORT EN_setdemandpattern(EN_Project ph, int nodeIndex, int demandIndex,
                int patIndex);
  int DLLEXPORT EN_getdemandname(EN_Project ph, int nodeIndex, int demandIdx,
                char *demandName);
  int DLLEXPORT EN_setdemandname(EN_Project ph, int nodeIndex, int demandIdx,
                char *demandName);

  int DLLEXPORT EN_addlink(EN_Project ph, char *id, EN_LinkType linkType,
                char *fromNode, char *toNode);
  int DLLEXPORT EN_deletelink(EN_Project ph, int index, int actionCode);
  int DLLEXPORT EN_getlinkindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getlinkid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_setlinkid(EN_Project ph, int index, char *newid);
  int DLLEXPORT EN_getlinktype(EN_Project ph, int index, EN_LinkType *code);
  int DLLEXPORT EN_setlinktype(EN_Project ph, int *index, EN_LinkType type, int actionCode);
  int DLLEXPORT EN_getlinknodes(EN_Project ph, int index, int *node1, int *node2);
  int DLLEXPORT EN_setlinknodes(EN_Project ph, int index, int node1, int node2);
  int DLLEXPORT EN_getlinkvalue(EN_Project ph, int index, EN_LinkProperty code,
                EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_setlinkvalue(EN_Project ph, int index, int code, EN_API_FLOAT_TYPE v);
  int DLLEXPORT EN_setpipedata(EN_Project ph, int index, EN_API_FLOAT_TYPE length,
                EN_API_FLOAT_TYPE diam, EN_API_FLOAT_TYPE rough, EN_API_FLOAT_TYPE mloss);


  int DLLEXPORT EN_getpumptype(EN_Project ph, int linkIndex, int *outType);
  int DLLEXPORT EN_getheadcurveindex(EN_Project ph, int pumpIndex, int *curveIndex);
  int DLLEXPORT EN_setheadcurveindex(EN_Project ph, int pumpIndex, int curveIndex);

  int DLLEXPORT EN_addpattern(EN_Project ph, char *id);
  int DLLEXPORT EN_getpatternindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getpatternid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_getpatternlen(EN_Project ph, int index, int *len);
  int DLLEXPORT EN_getpatternvalue(EN_Project ph, int index, int period, EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_setpatternvalue(EN_Project ph, int index, int period, EN_API_FLOAT_TYPE value);
  int DLLEXPORT EN_getaveragepatternvalue(EN_Project ph, int index, EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_setpattern(EN_Project ph, int index, EN_API_FLOAT_TYPE *f, int len);

  int DLLEXPORT EN_addcurve(EN_Project ph, char *id);
  int DLLEXPORT EN_getcurveindex(EN_Project ph, char *id, int *index);
  int DLLEXPORT EN_getcurveid(EN_Project ph, int index, char *id);
  int DLLEXPORT EN_getcurvelen(EN_Project ph, int index, int *len);
  int DLLEXPORT EN_getcurvetype(EN_Project ph, int curveIndex, int *outType);
  int DLLEXPORT EN_getcurvevalue(EN_Project ph, int curveIndex, int pointIndex,
                EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);
  int DLLEXPORT EN_setcurvevalue(EN_Project ph, int curveIndex, int pointIndex,
                EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);
  int DLLEXPORT EN_getcurve(EN_Project ph, int curveIndex, char* id,
                int *nValues, EN_API_FLOAT_TYPE **xValues,
                EN_API_FLOAT_TYPE **yValues);
  int DLLEXPORT EN_setcurve(EN_Project ph, int index, EN_API_FLOAT_TYPE *x,
                EN_API_FLOAT_TYPE *y, int len);

  int DLLEXPORT EN_addcontrol(EN_Project ph, int *cindex, int ctype, int lindex,
                EN_API_FLOAT_TYPE setting, int nindex, EN_API_FLOAT_TYPE level);
  int DLLEXPORT EN_deletecontrol(EN_Project ph, int index);
  int DLLEXPORT EN_getcontrol(EN_Project ph, int controlIndex,
                int *controlType, int *linkIndex, EN_API_FLOAT_TYPE *setting,
                int *nodeIndex, EN_API_FLOAT_TYPE *level);
  int DLLEXPORT EN_setcontrol(EN_Project ph, int cindex, int ctype, int lindex,
                EN_API_FLOAT_TYPE setting, int nindex, EN_API_FLOAT_TYPE level);

  int DLLEXPORT EN_addrule(EN_Project ph, char *rule);
  int DLLEXPORT EN_deleterule(EN_Project ph, int index);
  int DLLEXPORT EN_getrule(EN_Project ph, int index, int *nPremises,
                int *nThenActions, int *nElseActions, EN_API_FLOAT_TYPE *priority);
  int DLLEXPORT EN_getruleID(EN_Project ph, int index, char* id);
  int DLLEXPORT EN_getpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                int *logop, int *object, int *objIndex, int *variable, int *relop,
                int *status, EN_API_FLOAT_TYPE *value);
  int DLLEXPORT EN_setpremise(EN_Project ph, int ruleIndex, int premiseIndex,
                int logop, int object, int objIndex, int variable, int relop,
                int status, EN_API_FLOAT_TYPE value);
  int DLLEXPORT EN_setpremiseindex(EN_Project ph, int ruleIndex,
                int premiseIndex, int objIndex);
  int DLLEXPORT EN_setpremisestatus(EN_Project ph, int ruleIndex,
                int premiseIndex, int status);
  int DLLEXPORT EN_setpremisevalue(EN_Project ph, int ruleIndex,
                int premiseIndex, EN_API_FLOAT_TYPE value);
  int DLLEXPORT EN_getthenaction(EN_Project ph, int ruleIndex, int actionIndex,
                int *linkIndex, int *status, EN_API_FLOAT_TYPE *setting);
  int DLLEXPORT EN_setthenaction(EN_Project ph, int ruleIndex, int actionIndex,
                int linkIndex, int status, EN_API_FLOAT_TYPE setting);
  int DLLEXPORT EN_getelseaction(EN_Project ph, int ruleIndex, int actionIndex,
                int *linkIndex, int *status, EN_API_FLOAT_TYPE *setting);
  int DLLEXPORT EN_setelseaction(EN_Project ph, int ruleIndex, int actionIndex,
                int linkIndex, int status, EN_API_FLOAT_TYPE setting);
  int DLLEXPORT EN_setrulepriority(EN_Project ph, int index, EN_API_FLOAT_TYPE priority);


#if defined(__cplusplus)
}
#endif

#endif //EPANET2_2_H
