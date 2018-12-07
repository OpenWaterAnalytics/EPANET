



#ifndef EPANET_PY_H
#define EPANET_PY_H


#ifndef EN_API_FLOAT_TYPE
  #define EN_API_FLOAT_TYPE float
#endif

// Opaque pointer to project
typedef void *Handle;


#include "epanet2_enums.h"

#include "epanet2_export.h"


#if defined(__cplusplus)
extern "C" {
#endif


int DLLEXPORT proj_create(Handle *ph_out);
int DLLEXPORT proj_delete(Handle *ph_inout);
int DLLEXPORT proj_run(Handle ph, const char *input_path, const char *report_path, const char *output_path);
int DLLEXPORT proj_init(Handle ph, const char *rptFile, const char *outFile, EN_FlowUnits unitsType, EN_HeadLossType headLossType);
int DLLEXPORT proj_open(Handle ph, const char *inpFile, const char *rptFile, const char *binOutFile);
int DLLEXPORT proj_savefile(Handle ph, const char *inpfilename);
int DLLEXPORT proj_close(Handle ph);


int DLLEXPORT hydr_solve(Handle ph);
int DLLEXPORT hydr_save(Handle ph);
int DLLEXPORT hydr_open(Handle ph);
int DLLEXPORT hydr_init(Handle ph, EN_SaveOption saveFlag);
int DLLEXPORT hydr_run(Handle ph, long *currentTime);
int DLLEXPORT hydr_next(Handle ph, long *tStep);
int DLLEXPORT hydr_close(Handle ph);
int DLLEXPORT hydr_savefile(Handle ph, char *filename);
int DLLEXPORT hydr_usefile(Handle ph, char *filename);


int DLLEXPORT qual_solve(Handle ph);
int DLLEXPORT qual_open(Handle ph);
int DLLEXPORT qual_init(Handle ph, EN_SaveOption saveFlag);
int DLLEXPORT qual_run(Handle ph, long *currentTime);
int DLLEXPORT qual_next(Handle ph, long *tStep);
int DLLEXPORT qual_step(Handle ph, long *timeLeft);
int DLLEXPORT qual_close(Handle ph);


int DLLEXPORT rprt_writeline(Handle ph, char *line);
int DLLEXPORT rprt_writeresults(Handle ph);
int DLLEXPORT rprt_reset(Handle ph);
int DLLEXPORT rprt_set(Handle ph, char *reportCommand);
int DLLEXPORT rprt_setlevel(Handle ph, EN_StatusReport code);
int DLLEXPORT rprt_getcount(Handle ph, EN_CountType code, int *count);
int DLLEXPORT rprt_anlysstats(Handle ph, EN_AnalysisStatistic code, EN_API_FLOAT_TYPE* value);


int DLLEXPORT anlys_getoption(Handle ph, EN_Option opt, EN_API_FLOAT_TYPE *value);
int DLLEXPORT anlys_setoption(Handle ph, int code, EN_API_FLOAT_TYPE value);
int DLLEXPORT anlys_getflowunits(Handle ph, int *code);
int DLLEXPORT anlys_setflowunits(Handle ph, EN_FlowUnits code);
int DLLEXPORT anlys_gettimeparam(Handle ph, EN_TimeProperty code, long *value);
int DLLEXPORT anlys_settimeparam(Handle ph, EN_TimeProperty code, long value);
int DLLEXPORT anlys_getqualinfo(Handle ph, EN_QualityType *qualcode, char *chemname, char *chemunits, int *tracenode);
int DLLEXPORT anlys_getqualtype(Handle ph, EN_QualityType *qualcode, int *tracenode);
int DLLEXPORT anlys_setqualtype(Handle ph, EN_QualityType qualcode, char *chemname, char *chemunits, char *tracenode);


int DLLEXPORT node_add(Handle ph, char *id, EN_NodeType nodeType);
int DLLEXPORT node_delete(Handle ph, int index, int actionCode);
int DLLEXPORT node_getindex(Handle ph, char *id, int *index);
int DLLEXPORT node_getid(Handle ph, int index, char *id);
int DLLEXPORT node_setid(Handle ph, int index, char *newid);
int DLLEXPORT node_gettype(Handle ph, int index, int *code);
int DLLEXPORT node_getvalue(Handle ph, int index, int code, EN_API_FLOAT_TYPE *value);
int DLLEXPORT node_setvalue(Handle ph, int index, int code, EN_API_FLOAT_TYPE value);
int DLLEXPORT node_getcoord(Handle ph, int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);
int DLLEXPORT node_setcoord(Handle ph, int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);


int DLLEXPORT dmnd_getmodel(Handle ph, int *type, EN_API_FLOAT_TYPE *pmin, EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp);
int DLLEXPORT dmnd_setmodel(Handle ph, int type, EN_API_FLOAT_TYPE pmin, EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp);
int DLLEXPORT dmnd_getcount(Handle ph, int nodeIndex, int *numDemands);
int DLLEXPORT dmnd_getbase(Handle ph, int nodeIndex, int demandIndex, EN_API_FLOAT_TYPE *baseDemand);
int DLLEXPORT dmnd_setbase(Handle ph, int nodeIndex, int demandIndex, EN_API_FLOAT_TYPE baseDemand);
int DLLEXPORT dmnd_getpattern(Handle ph, int nodeIndex, int demandIndex, int *pattIndex);
int DLLEXPORT dmnd_setpattern(Handle ph, int nodeIndex, int demandIndex, int patIndex);
int DLLEXPORT dmnd_getname(Handle ph, int nodeIndex, int demandIdx, char *demandName);
int DLLEXPORT dmnd_setname(Handle ph, int nodeIndex, int demandIdx, char *demandName);


int DLLEXPORT link_add(Handle ph, char *id, EN_LinkType linkType, char *fromNode, char *toNode);
int DLLEXPORT link_delete(Handle ph, int index, int actionCode);
int DLLEXPORT link_getindex(Handle ph, char *id, int *index);
int DLLEXPORT link_getid(Handle ph, int index, char *id);
int DLLEXPORT link_setid(Handle ph, int index, char *newid);
int DLLEXPORT link_gettype(Handle ph, int index, int *code);
int DLLEXPORT link_settype(Handle ph, int *index, EN_LinkType type, int actionCode);
int DLLEXPORT link_getnodes(Handle ph, int index, int *node1, int *node2);
int DLLEXPORT link_setnodes(Handle ph, int index, int node1, int node2);
int DLLEXPORT link_getvalue(Handle ph, int index, EN_LinkProperty code, EN_API_FLOAT_TYPE *value);
int DLLEXPORT link_setvalue(Handle ph, int index, int code, EN_API_FLOAT_TYPE v);


int DLLEXPORT pump_gettype(Handle ph, int linkIndex, int *outType);
int DLLEXPORT pump_getheadcurveindex(Handle ph, int pumpIndex, int *curveIndex);
int DLLEXPORT pump_setheadcurveindex(Handle ph, int pumpIndex, int curveIndex);


int DLLEXPORT ptrn_add(Handle ph, char *id);
int DLLEXPORT ptrn_getindex(Handle ph, char *id, int *index);
int DLLEXPORT ptrn_getid(Handle ph, int index, char *id);
int DLLEXPORT ptrn_getlength(Handle ph, int index, int *len);
int DLLEXPORT ptrn_getvalue(Handle ph, int index, int period, EN_API_FLOAT_TYPE *value);
int DLLEXPORT ptrn_setvalue(Handle ph, int index, int period, EN_API_FLOAT_TYPE value);
int DLLEXPORT ptrn_getavgvalue(Handle ph, int index, EN_API_FLOAT_TYPE *value);
int DLLEXPORT ptrn_set(Handle ph, int index, EN_API_FLOAT_TYPE *f, int len);


int DLLEXPORT curv_add(Handle ph, char *id);
int DLLEXPORT curv_getindex(Handle ph, char *id, int *index);
int DLLEXPORT curv_getid(Handle ph, int index, char *id);
int DLLEXPORT curv_getlength(Handle ph, int index, int *len);
int DLLEXPORT curv_gettype(Handle ph, int curveIndex, int *outType);
int DLLEXPORT curv_getvalue(Handle ph, int curveIndex, int pointIndex, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y);
int DLLEXPORT curv_setvalue(Handle ph, int curveIndex, int pointIndex, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y);
int DLLEXPORT curv_get(Handle ph, int curveIndex, char* id, int *nValues, EN_API_FLOAT_TYPE **xValues, EN_API_FLOAT_TYPE **yValues);
int DLLEXPORT curv_set(Handle ph, int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y, int len);


void DLLEXPORT err_clear(Handle ph);
int  DLLEXPORT err_check(Handle ph, char** msg_buffer);
void DLLEXPORT toolkit_free(void **memory);
int  DLLEXPORT toolkit_getversion(int *version);


#if defined(__cplusplus)
}
#endif


#endif //EPANET_PY_H
