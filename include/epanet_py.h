/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet_py.h
 Description:  EPANET API functions for Python SWIG wrap
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 02/08/2019
 ******************************************************************************
*/


#ifndef EPANET_PY_H
#define EPANET_PY_H


// Opaque pointer to project
typedef void *Handle;


#include "epanet2_enums.h"
#include "epanet_py_export.h"


#if defined(__cplusplus)
extern "C" {
#endif


int EXPORT_PY_API proj_create(Handle *ph_out);
int EXPORT_PY_API proj_delete(Handle *ph_inout);
int EXPORT_PY_API proj_run(Handle ph, const char *input_path, const char *report_path, const char *output_path);
int EXPORT_PY_API proj_init(Handle ph, const char *rptFile, const char *outFile, EN_FlowUnits unitsType, EN_HeadLossType headLossType);
int EXPORT_PY_API proj_open(Handle ph, const char *inpFile, const char *rptFile, const char *binOutFile);
int EXPORT_PY_API proj_gettitle(Handle ph, char *line1, char *line2, char *line3);
int EXPORT_PY_API proj_settitle(Handle ph, const char *line1, const char *line2, const char *line3);
int EXPORT_PY_API proj_savefile(Handle ph, const char *inpfilename);
int EXPORT_PY_API proj_close(Handle ph);


int EXPORT_PY_API hydr_solve(Handle ph);
int EXPORT_PY_API hydr_save(Handle ph);
int EXPORT_PY_API hydr_open(Handle ph);
int EXPORT_PY_API hydr_init(Handle ph, EN_InitHydOption saveFlag);
int EXPORT_PY_API hydr_run(Handle ph, long *currentTime);
int EXPORT_PY_API hydr_next(Handle ph, long *tStep);
int EXPORT_PY_API hydr_close(Handle ph);
int EXPORT_PY_API hydr_savefile(Handle ph, char *filename);
int EXPORT_PY_API hydr_usefile(Handle ph, char *filename);


int EXPORT_PY_API qual_solve(Handle ph);
int EXPORT_PY_API qual_open(Handle ph);
int EXPORT_PY_API qual_init(Handle ph, EN_InitHydOption saveFlag);
int EXPORT_PY_API qual_run(Handle ph, long *currentTime);
int EXPORT_PY_API qual_next(Handle ph, long *tStep);
int EXPORT_PY_API qual_step(Handle ph, long *timeLeft);
int EXPORT_PY_API qual_close(Handle ph);


int EXPORT_PY_API rprt_writeline(Handle ph, char *line);
int EXPORT_PY_API rprt_writeresults(Handle ph);
int EXPORT_PY_API rprt_clear(Handle ph);
int EXPORT_PY_API rprt_reset(Handle ph);
int EXPORT_PY_API rprt_set(Handle ph, char *reportCommand);
int EXPORT_PY_API rprt_setlevel(Handle ph, EN_StatusReport code);
int EXPORT_PY_API rprt_getcount(Handle ph, EN_CountType code, int *count);
int EXPORT_PY_API rprt_anlysstats(Handle ph, EN_AnalysisStatistic code, double* value);


int EXPORT_PY_API anlys_getoption(Handle ph, EN_Option code, double *value);
int EXPORT_PY_API anlys_setoption(Handle ph, EN_Option code, double value);
int EXPORT_PY_API anlys_getflowunits(Handle ph, int *code);
int EXPORT_PY_API anlys_setflowunits(Handle ph, EN_FlowUnits code);
int EXPORT_PY_API anlys_gettimeparam(Handle ph, EN_TimeParameter code, long *value);
int EXPORT_PY_API anlys_settimeparam(Handle ph, EN_TimeParameter code, long value);
int EXPORT_PY_API anlys_getqualinfo(Handle ph, int *qualcode, char *chemname, char *chemunits, int *tracenode);
int EXPORT_PY_API anlys_getqualtype(Handle ph, int *qualcode, int *tracenode);
int EXPORT_PY_API anlys_setqualtype(Handle ph, EN_QualityType qualcode, char *chemname, char *chemunits, char *tracenode);


int EXPORT_PY_API node_add(Handle ph, char *id, EN_NodeType nodeType);
int EXPORT_PY_API node_delete(Handle ph, int index, int actionCode);
int EXPORT_PY_API node_getindex(Handle ph, char *id, int *index);
int EXPORT_PY_API node_getid(Handle ph, int index, char *id);
int EXPORT_PY_API node_setid(Handle ph, int index, char *newid);
int EXPORT_PY_API node_gettype(Handle ph, int index, int *code);
int EXPORT_PY_API node_getvalue(Handle ph, int index, EN_NodeProperty code, double *value);
int EXPORT_PY_API node_setvalue(Handle ph, int index, EN_NodeProperty code, double value);
int EXPORT_PY_API node_getcoord(Handle ph, int index, double *x, double *y);
int EXPORT_PY_API node_setcoord(Handle ph, int index, double x, double y);


int EXPORT_PY_API dmnd_getmodel(Handle ph, int *type, double *pmin, double *preq, double *pexp);
int EXPORT_PY_API dmnd_setmodel(Handle ph, int type, double pmin, double preq, double pexp);
int EXPORT_PY_API dmnd_getcount(Handle ph, int nodeIndex, int *numDemands);
int EXPORT_PY_API dmnd_getbase(Handle ph, int nodeIndex, int demandIndex, double *baseDemand);
int EXPORT_PY_API dmnd_setbase(Handle ph, int nodeIndex, int demandIndex, double baseDemand);
int EXPORT_PY_API dmnd_getpattern(Handle ph, int nodeIndex, int demandIndex, int *pattIndex);
int EXPORT_PY_API dmnd_setpattern(Handle ph, int nodeIndex, int demandIndex, int patIndex);
int EXPORT_PY_API dmnd_getname(Handle ph, int nodeIndex, int demandIdx, char *demandName);
int EXPORT_PY_API dmnd_setname(Handle ph, int nodeIndex, int demandIdx, char *demandName);


int EXPORT_PY_API link_add(Handle ph, char *id, EN_LinkType linkType, char *fromNode, char *toNode);
int EXPORT_PY_API link_delete(Handle ph, int index, int actionCode);
int EXPORT_PY_API link_getindex(Handle ph, char *id, int *index);
int EXPORT_PY_API link_getid(Handle ph, int index, char *id);
int EXPORT_PY_API link_setid(Handle ph, int index, char *newid);
int EXPORT_PY_API link_gettype(Handle ph, int index, int *code);
int EXPORT_PY_API link_settype(Handle ph, int *index, EN_LinkType type, int actionCode);
int EXPORT_PY_API link_getnodes(Handle ph, int index, int *node1, int *node2);
int EXPORT_PY_API link_setnodes(Handle ph, int index, int node1, int node2);
int EXPORT_PY_API link_getvalue(Handle ph, int index, EN_LinkProperty code, double *value);
int EXPORT_PY_API link_setvalue(Handle ph, int index, int code, double v);


int EXPORT_PY_API pump_gettype(Handle ph, int linkIndex, int *outType);
int EXPORT_PY_API pump_getheadcurveindex(Handle ph, int pumpIndex, int *curveIndex);
int EXPORT_PY_API pump_setheadcurveindex(Handle ph, int pumpIndex, int curveIndex);


int EXPORT_PY_API ptrn_add(Handle ph, char *id);
int EXPORT_PY_API ptrn_getindex(Handle ph, char *id, int *index);
int EXPORT_PY_API ptrn_getid(Handle ph, int index, char *id);
int EXPORT_PY_API ptrn_getlength(Handle ph, int index, int *len);
int EXPORT_PY_API ptrn_getvalue(Handle ph, int index, int period, double *value);
int EXPORT_PY_API ptrn_setvalue(Handle ph, int index, int period, double value);
int EXPORT_PY_API ptrn_getavgvalue(Handle ph, int index, double *value);
int EXPORT_PY_API ptrn_set(Handle ph, int index, double *f, int len);


int EXPORT_PY_API curv_add(Handle ph, char *id);
int EXPORT_PY_API curv_getindex(Handle ph, char *id, int *index);
int EXPORT_PY_API curv_getid(Handle ph, int index, char *id);
int EXPORT_PY_API curv_getlength(Handle ph, int index, int *len);
int EXPORT_PY_API curv_gettype(Handle ph, int curveIndex, int *outType);
int EXPORT_PY_API curv_getvalue(Handle ph, int curveIndex, int pointIndex, double *x, double *y);
int EXPORT_PY_API curv_setvalue(Handle ph, int curveIndex, int pointIndex, double x, double y);
int EXPORT_PY_API curv_get(Handle ph, int curveIndex, char* id, int *nValues, double **xValues, double **yValues);
int EXPORT_PY_API curv_set(Handle ph, int index, double *x, double *y, int len);

int EXPORT_PY_API scntl_add(Handle ph, int type, int linkIndex, double setting, int nodeIndex, double level, int *index);
int EXPORT_PY_API scntl_delete(Handle ph, int index);
int EXPORT_PY_API scntl_get(Handle ph, int controlIndex, int *controlType, int *linkIndex, double *setting, int *nodeIndex, double *level);
int EXPORT_PY_API scntl_set(Handle ph, int cindex, int ctype, int lindex, double setting, int nindex, double level);


int EXPORT_PY_API rcntl_add(Handle ph, char *rule);
int EXPORT_PY_API rcntl_delete(Handle ph, int index);
int EXPORT_PY_API rcntl_get(Handle ph, int index, int *nPremises, int *nThenActions, int *nElseActions, double *priority);
int EXPORT_PY_API rcntl_getid(Handle ph, int index, char* id);
int EXPORT_PY_API rcntl_getpremise(Handle ph, int ruleIndex, int premiseIndex, int *logop, int *object, int *objIndex, int *variable, int *relop, int *status, double *value);
int EXPORT_PY_API rcntl_setpremise(Handle ph, int ruleIndex, int premiseIndex, int logop, int object, int objIndex, int variable, int relop, int status, double value);
int EXPORT_PY_API rcntl_setpremiseindex(Handle ph, int ruleIndex, int premiseIndex, int objIndex);
int EXPORT_PY_API rcntl_setpremisestatus(Handle ph, int ruleIndex, int premiseIndex, int status);
int EXPORT_PY_API rcntl_setpremisevalue(Handle ph, int ruleIndex, int premiseIndex, double value);
int EXPORT_PY_API rcntl_getthenaction(Handle ph, int ruleIndex, int actionIndex, int *linkIndex, int *status, double *setting);
int EXPORT_PY_API rcntl_setthenaction(Handle ph, int ruleIndex, int actionIndex, int linkIndex, int status, double setting);
int EXPORT_PY_API rcntl_getelseaction(Handle ph, int ruleIndex, int actionIndex, int *linkIndex, int *status, double *setting);
int EXPORT_PY_API rcntl_setelseaction(Handle ph, int ruleIndex, int actionIndex, int linkIndex, int status, double setting);
int EXPORT_PY_API rcntl_setrulepriority(Handle ph, int index, double priority);


void EXPORT_PY_API err_clear(Handle ph);
int  EXPORT_PY_API err_check(Handle ph, char** msg_buffer);
void EXPORT_PY_API toolkit_free(void **memory);
int  EXPORT_PY_API toolkit_getversion(int *version);


#if defined(__cplusplus)
}
#endif


#endif //EPANET_PY_H
