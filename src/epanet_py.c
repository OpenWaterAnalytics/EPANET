/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet_py.c
 Description:  EPANET API functions for Python SWIG wrap
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 02/08/2019
 ******************************************************************************
*/


#include <stdlib.h>
#include <string.h>

#include "epanet_py.h"
#include "util/errormanager.h"
#include "epanet2_2.h"
#include "text.h"
#include "types.h"


typedef struct {
    Project *project;
    error_handle_t *error;
}handle_t;
// Extern functions
extern char *geterrmsg(int, char *);
// Local functions
void error_lookup(int errcode, char *errmsg, int len);


int EXPORT_PY_API proj_create(Handle *ph)
{
    handle_t *handle = (handle_t *)calloc(1, sizeof(handle_t));

    if (handle != NULL)
    {
        EN_createproject(&handle->project);
        handle->error = error_new_manager(&error_lookup);
        *ph = handle;
        return 0;
    }
    return -1;
}

int EXPORT_PY_API proj_delete(Handle *ph)
{
    handle_t *handle = (handle_t *)*ph;

    if (handle == NULL)
        return -1;
    else
    {
        EN_deleteproject(&handle->project);
        error_dst_manager(handle->error);
    }
    free(handle);
    *ph = NULL;

    return 0;
}

int EXPORT_PY_API proj_run(Handle ph, const char *input_path,
    const char *report_path, const char *output_path)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_runproject(pr->project, input_path, report_path, output_path, NULL));
}

int EXPORT_PY_API proj_init(Handle ph, const char *rptFile, const char *outFile,
    EN_FlowUnits unitsType, EN_HeadLossType headLossType)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_init(pr->project, rptFile, outFile, unitsType, headLossType));
}

int EXPORT_PY_API proj_open(Handle ph, const char *inpFile, const char *rptFile,
        const char *binOutFile)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_open(pr->project, inpFile, rptFile, binOutFile));
}

int EXPORT_PY_API proj_gettitle(Handle ph, char *line1, char *line2, char *line3)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_gettitle(pr->project, line1, line2, line3));
}

int EXPORT_PY_API proj_settitle(Handle ph, const char *line1, const char *line2, const char *line3)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_settitle(pr->project, line1, line2, line3));
}

int EXPORT_PY_API proj_savefile(Handle ph, const char *filename)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_saveinpfile(pr->project, filename));
}

int EXPORT_PY_API proj_close(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_close(pr->project));
}




int EXPORT_PY_API hydr_solve(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_solveH(pr->project));
}

int EXPORT_PY_API hydr_save(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_saveH(pr->project));
}

int EXPORT_PY_API hydr_open(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_openH(pr->project));
}

int EXPORT_PY_API hydr_init(Handle ph, EN_InitHydOption saveFlag)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_initH(pr->project, saveFlag));
}

int EXPORT_PY_API hydr_run(Handle ph, long *currentTime)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_runH(pr->project, currentTime));
}

int EXPORT_PY_API hydr_next(Handle ph, long *tStep)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_nextH(pr->project, tStep));
}

int EXPORT_PY_API hydr_close(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_closeH(pr->project));
}

int EXPORT_PY_API hydr_savefile(Handle ph, char *filename)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_savehydfile(pr->project, filename));
}

int EXPORT_PY_API hydr_usefile(Handle ph, char *filename)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_usehydfile(pr->project, filename));
}




int EXPORT_PY_API qual_solve(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_solveQ(pr->project));
}

int EXPORT_PY_API qual_open(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_openQ(pr->project));
}

int EXPORT_PY_API qual_init(Handle ph, EN_InitHydOption saveFlag)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_initQ(pr->project, saveFlag));
}

int EXPORT_PY_API qual_run(Handle ph, long *currentTime)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_runQ(pr->project, currentTime));
}

int EXPORT_PY_API qual_next(Handle ph, long *tStep)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_nextQ(pr->project, tStep));
}

int EXPORT_PY_API qual_step(Handle ph, long *timeLeft)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_stepQ(pr->project, timeLeft));
}

int EXPORT_PY_API qual_close(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_closeQ(pr->project));
}




int EXPORT_PY_API rprt_writeline(Handle ph, char *line)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_writeline(pr->project, line));
}

int EXPORT_PY_API rprt_writeresults(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_report(pr->project));
}

int EXPORT_PY_API rprt_clear(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_clearreport(pr->project));
}

int EXPORT_PY_API rprt_reset(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_resetreport(pr->project));
}

int EXPORT_PY_API rprt_set(Handle ph, char *reportCommand)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setreport(pr->project, reportCommand));
}

int EXPORT_PY_API rprt_setlevel(Handle ph, EN_StatusReport code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setstatusreport(pr->project, code));
}

int EXPORT_PY_API rprt_getcount(Handle ph, EN_CountType code, int *count)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcount(pr->project, code, count));
}

int EXPORT_PY_API rprt_anlysstats(Handle ph, EN_AnalysisStatistic code, double* value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getstatistic(pr->project, code, value));
}




int EXPORT_PY_API anlys_getoption(Handle ph, EN_Option code, double *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getoption(pr->project, (int)code, value));
}

int EXPORT_PY_API anlys_setoption(Handle ph, EN_Option code, double value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setoption(pr->project, (int)code, value));
}

int EXPORT_PY_API anlys_getflowunits(Handle ph, int *code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getflowunits(pr->project, code));
}

int EXPORT_PY_API anlys_setflowunits(Handle ph, EN_FlowUnits code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setflowunits(pr->project, code));
}

int EXPORT_PY_API anlys_gettimeparam(Handle ph, EN_TimeParameter code, long *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_gettimeparam(pr->project, code, value));
}

int EXPORT_PY_API anlys_settimeparam(Handle ph, EN_TimeParameter code, long value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_settimeparam(pr->project, code, value));
}

int EXPORT_PY_API anlys_getqualinfo(Handle ph, int *qualcode, char *chemname, char *chemunits, int *tracenode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getqualinfo(pr->project, qualcode, chemname, chemunits, tracenode));
}

int EXPORT_PY_API anlys_getqualtype(Handle ph, int *qualcode, int *tracenode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getqualtype(pr->project, qualcode, tracenode));
}

int EXPORT_PY_API anlys_setqualtype(Handle ph, EN_QualityType qualcode, char *chemname, char *chemunits, char *tracenode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setqualtype(pr->project, qualcode, chemname, chemunits, tracenode));
}




int EXPORT_PY_API node_add(Handle ph, char *id, EN_NodeType nodeType)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addnode(pr->project, id, nodeType));
}

int EXPORT_PY_API node_delete(Handle ph, int index, int actionCode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_deletenode(pr->project, index, actionCode));
}

int EXPORT_PY_API node_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodeindex(pr->project, id, index));
}

int EXPORT_PY_API node_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodeid(pr->project, index, id));
}

int EXPORT_PY_API node_setid(Handle ph, int index, char *newid)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodeid(pr->project, index, newid));
}

int EXPORT_PY_API node_gettype(Handle ph, int index, int *code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodetype(pr->project, index, code));
}

int EXPORT_PY_API node_getvalue(Handle ph, int index, EN_NodeProperty code, double *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodevalue(pr->project, index, (int)code, value));
}

int EXPORT_PY_API node_setvalue(Handle ph, int index, EN_NodeProperty code, double value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setnodevalue(pr->project, index, (int)code, value));
}

int EXPORT_PY_API node_getcoord(Handle ph, int index, double *x, double *y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcoord(pr->project, index, x, y));
}

int EXPORT_PY_API node_setcoord(Handle ph, int index, double x, double y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setcoord(pr->project, index, x, y));
}




int EXPORT_PY_API dmnd_getmodel(Handle ph, int *type, double *pmin, double *preq, double *pexp)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getdemandmodel(pr->project, type, pmin, preq, pexp));
}

int EXPORT_PY_API dmnd_setmodel(Handle ph, int type, double pmin, double preq, double pexp)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setdemandmodel(pr->project, type, pmin, preq, pexp));
}

int EXPORT_PY_API dmnd_getcount(Handle ph, int nodeIndex, int *numDemands)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnumdemands(pr->project, nodeIndex, numDemands));
}

int EXPORT_PY_API dmnd_getbase(Handle ph, int nodeIndex, int demandIndex, double *baseDemand)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getbasedemand(pr->project, nodeIndex, demandIndex, baseDemand));
}

int EXPORT_PY_API dmnd_setbase(Handle ph, int nodeIndex, int demandIndex, double baseDemand)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setbasedemand(pr->project, nodeIndex, demandIndex, baseDemand));
}

int EXPORT_PY_API dmnd_getpattern(Handle ph, int nodeIndex, int demandIndex, int *patIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getdemandpattern(pr->project, nodeIndex, demandIndex, patIndex));
}

int EXPORT_PY_API dmnd_setpattern(Handle ph, int nodeIndex, int demandIndex, int patIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setdemandpattern(pr->project, nodeIndex, demandIndex, patIndex));
}

int EXPORT_PY_API dmnd_getname(Handle ph, int nodeIndex, int demandIdx, char *demandName)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getdemandname(pr->project, nodeIndex, demandIdx, demandName));
}

int EXPORT_PY_API dmnd_setname(Handle ph, int nodeIndex, int demandIdx, char *demandName)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setdemandname(pr->project, nodeIndex, demandIdx, demandName));
}




int EXPORT_PY_API link_add(Handle ph, char *id, EN_LinkType linkType, char *fromNode, char *toNode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addlink(pr->project, id, linkType, fromNode, toNode));
}

int EXPORT_PY_API link_delete(Handle ph, int index, int actionCode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_deletelink(pr->project, index, actionCode));
}

int EXPORT_PY_API link_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinkindex(pr->project, id, index));
}

int EXPORT_PY_API link_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinkid(pr->project, index, id));
}

int EXPORT_PY_API link_setid(Handle ph, int index, char *newid)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinkid(pr->project, index, newid));
}

int EXPORT_PY_API link_gettype(Handle ph, int index, int *code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinktype(pr->project, index, code));
}

int EXPORT_PY_API link_settype(Handle ph, int *index, EN_LinkType type, int actionCode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinktype(pr->project, index, type, actionCode));
}

int EXPORT_PY_API link_getnodes(Handle ph, int index, int *node1, int *node2)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinknodes(pr->project, index, node1, node2));
}

int EXPORT_PY_API link_setnodes(Handle ph, int index, int node1, int node2)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinknodes(pr->project, index, node1, node2));
}

int EXPORT_PY_API link_getvalue(Handle ph, int index, EN_LinkProperty code, double *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinkvalue(pr->project, index, code, value));
}

int EXPORT_PY_API link_setvalue(Handle ph, int index, int code, double value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinkvalue(pr->project, index, code, value));
}




int EXPORT_PY_API pump_gettype(Handle ph, int linkIndex, int *outType)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpumptype(pr->project, linkIndex, outType));
}

int EXPORT_PY_API pump_getheadcurveindex(Handle ph, int pumpIndex, int *curveIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getheadcurveindex(pr->project, pumpIndex, curveIndex));
}

int EXPORT_PY_API pump_setheadcurveindex(Handle ph, int pumpIndex, int curveIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setheadcurveindex(pr->project, pumpIndex, curveIndex));
}




int EXPORT_PY_API ptrn_add(Handle ph, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addpattern(pr->project, id));
}

int EXPORT_PY_API ptrn_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternindex(pr->project, id, index));
}

int EXPORT_PY_API ptrn_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternid(pr->project, index, id));
}

int EXPORT_PY_API ptrn_getlength(Handle ph, int index, int *len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternlen(pr->project, index, len));
}

int EXPORT_PY_API ptrn_getvalue(Handle ph, int index, int period, double *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternvalue(pr->project, index, period, value));
}

int EXPORT_PY_API ptrn_setvalue(Handle ph, int index, int period, double value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpatternvalue(pr->project, index, period, value));
}

int EXPORT_PY_API ptrn_getavgvalue(Handle ph, int index, double *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getaveragepatternvalue(pr->project, index, value));
}

int EXPORT_PY_API ptrn_set(Handle ph, int index, double *values, int len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpattern(pr->project, index, values, len));
}




int EXPORT_PY_API curv_add(Handle ph, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addcurve(pr->project, id));
}

int EXPORT_PY_API curv_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurveindex(pr->project, id, index));
}

int EXPORT_PY_API curv_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurveid(pr->project, index, id));
}

int EXPORT_PY_API curv_getlength(Handle ph, int index, int *len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurvelen(pr->project, index, len));
}

int EXPORT_PY_API curv_gettype(Handle ph, int index, int *type)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurvetype(pr->project, index, type));
}

int EXPORT_PY_API curv_getvalue(Handle ph, int curveIndex, int pointIndex, double *x, double *y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurvevalue(pr->project, curveIndex, pointIndex, x, y));
}

int EXPORT_PY_API curv_setvalue(Handle ph, int curveIndex, int pointIndex, double x, double y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setcurvevalue(pr->project, curveIndex, pointIndex, x, y));
}

int EXPORT_PY_API curv_get(Handle ph, int curveIndex, char* id, int *nValues, double **xValues, double **yValues)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurve(pr->project, curveIndex, id, nValues, xValues, yValues));
}

int EXPORT_PY_API curv_set(Handle ph, int index, double *x, double *y, int len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setcurve(pr->project, index, x, y, len));
}




int EXPORT_PY_API scntl_add(Handle ph, int type, int linkIndex, double setting, int nodeIndex, double level, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addcontrol(pr->project, type, linkIndex, setting, nodeIndex, level, index));
}

int EXPORT_PY_API scntl_delete(Handle ph, int index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_deletecontrol(pr->project, index));
}

int EXPORT_PY_API scntl_get(Handle ph, int controlIndex, int *controlType, int *linkIndex, double *setting, int *nodeIndex, double *level)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcontrol(pr->project, controlIndex, controlType, linkIndex, setting, nodeIndex, level));
}

int EXPORT_PY_API scntl_set(Handle ph, int cindex, int ctype, int lindex, double setting, int nindex, double level)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setcontrol(pr->project, cindex, ctype, lindex, setting, nindex, level));
}




int EXPORT_PY_API rcntl_add(Handle ph, char *rule)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addrule(pr->project, rule));
}

int EXPORT_PY_API rcntl_delete(Handle ph, int index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_deleterule(pr->project, index));
}

int EXPORT_PY_API rcntl_get(Handle ph, int index, int *nPremises, int *nThenActions, int *nElseActions, double *priority)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getrule(pr->project, index, nPremises, nThenActions, nElseActions, priority));
}

int EXPORT_PY_API rcntl_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getruleID(pr->project, index, id));
}

int EXPORT_PY_API rcntl_getpremise(Handle ph, int ruleIndex, int premiseIndex, int *logop, int *object, int *objIndex, int *variable, int *relop, int *status, double *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpremise(pr->project, ruleIndex, premiseIndex, logop, object, objIndex, variable, relop, status, value));
}

int EXPORT_PY_API rcntl_setpremise(Handle ph, int ruleIndex, int premiseIndex, int logop, int object, int objIndex, int variable, int relop, int status, double value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpremise(pr->project, ruleIndex, premiseIndex, logop, object, objIndex, variable, relop, status, value));
}

int EXPORT_PY_API rcntl_setpremiseindex(Handle ph, int ruleIndex, int premiseIndex, int objIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpremiseindex(pr->project, ruleIndex, premiseIndex, objIndex));
}

int EXPORT_PY_API rcntl_setpremisestatus(Handle ph, int ruleIndex, int premiseIndex, int status)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpremisestatus(pr->project, ruleIndex, premiseIndex, status));
}

int EXPORT_PY_API rcntl_setpremisevalue(Handle ph, int ruleIndex, int premiseIndex, double value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpremisevalue(pr->project, ruleIndex, premiseIndex, value));
}

int EXPORT_PY_API rcntl_getthenaction(Handle ph, int ruleIndex, int actionIndex, int *linkIndex, int *status, double *setting)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getthenaction(pr->project, ruleIndex, actionIndex, linkIndex, status, setting));
}

int EXPORT_PY_API rcntl_setthenaction(Handle ph, int ruleIndex, int actionIndex, int linkIndex, int status, double setting)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setthenaction(pr->project, ruleIndex, actionIndex, linkIndex, status, setting));
}

int EXPORT_PY_API rcntl_getelseaction(Handle ph, int ruleIndex, int actionIndex, int *linkIndex, int *status, double *setting)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getelseaction(pr->project, ruleIndex, actionIndex, linkIndex, status, setting));
}

int EXPORT_PY_API rcntl_setelseaction(Handle ph, int ruleIndex, int actionIndex, int linkIndex, int status, double setting)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setelseaction(pr->project, ruleIndex, actionIndex, linkIndex, status, setting));
}

int EXPORT_PY_API rcntl_setrulepriority(Handle ph, int index, double priority)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setrulepriority(pr->project, index, priority));
}




void EXPORT_PY_API err_clear(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    error_clear(pr->error);
}

int EXPORT_PY_API err_check(Handle ph, char** msg_buffer)
//
// Purpose: Returns the error message or NULL.
//
// Note: Caller must free memory allocated by EN_check_error
//
{
    int errorcode = 0;
    char *temp = NULL;

    handle_t *pr = (handle_t *)ph;


    if (pr == NULL) return -1;
    else
    {
        errorcode = pr->error->error_status;
        if (errorcode)
            temp = error_check(pr->error);

        *msg_buffer = temp;
    }

    return errorcode;
}

int EXPORT_PY_API toolkit_getversion(int *version)
{
    return EN_getversion(version);
}

void EXPORT_PY_API toolkit_free(void **memory)
{
	free(*memory);
    *memory = NULL;
}

void error_lookup(int errcode, char *dest_msg, int dest_len)
// Purpose: takes error code returns error message
{
    char *msg = NULL;

    switch (errcode)
    {
    case 1: msg = WARN1;
    break;
    case 2: msg = WARN2;
    break;
    case 3: msg = WARN3;
    break;
    case 4: msg = WARN4;
    break;
    case 5: msg = WARN5;
    break;
    case 6: msg = WARN6;
    break;
    default:
	{
		char new_msg[MAXMSG + 1];
		msg = geterrmsg(errcode, new_msg);
	}
    }
    strncpy(dest_msg, msg, dest_len);
}
