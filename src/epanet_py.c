

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


int DLLEXPORT proj_create(Handle *ph)
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

int DLLEXPORT proj_delete(Handle *ph)
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

int DLLEXPORT proj_run(Handle ph, const char *input_path,
    const char *report_path, const char *output_path)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_runproject(pr->project, input_path, report_path, output_path, NULL));
}

int DLLEXPORT proj_init(Handle ph, const char *rptFile, const char *outFile,
    EN_FlowUnits unitsType, EN_HeadLossType headLossType)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_init(pr->project, rptFile, outFile, unitsType, headLossType));
}

int DLLEXPORT proj_open(Handle ph, const char *inpFile, const char *rptFile,
        const char *binOutFile)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_open(pr->project, inpFile, rptFile, binOutFile));
}

int DLLEXPORT proj_savefile(Handle ph, const char *filename)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_saveinpfile(pr->project, filename));
}

int DLLEXPORT proj_close(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_close(pr->project));
}




int DLLEXPORT hydr_solve(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_solveH(pr->project));
}

int DLLEXPORT hydr_save(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_saveH(pr->project));
}

int DLLEXPORT hydr_open(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_openH(pr->project));
}

int DLLEXPORT hydr_init(Handle ph, EN_SaveOption saveFlag)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_initH(pr->project, saveFlag));
}

int DLLEXPORT hydr_run(Handle ph, long *currentTime)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_runH(pr->project, currentTime));
}

int DLLEXPORT hydr_next(Handle ph, long *tStep)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_nextH(pr->project, tStep));
}

int DLLEXPORT hydr_close(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_closeH(pr->project));
}

int DLLEXPORT hydr_savefile(Handle ph, char *filename)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_savehydfile(pr->project, filename));
}

int DLLEXPORT hydr_usefile(Handle ph, char *filename)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_usehydfile(pr->project, filename));
}




int DLLEXPORT qual_solve(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_solveQ(pr->project));
}

int DLLEXPORT qual_open(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_openQ(pr->project));
}

int DLLEXPORT qual_init(Handle ph, EN_SaveOption saveFlag)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_initQ(pr->project, saveFlag));
}

int DLLEXPORT qual_run(Handle ph, long *currentTime)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_runQ(pr->project, currentTime));
}

int DLLEXPORT qual_next(Handle ph, long *tStep)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_nextQ(pr->project, tStep));
}

int DLLEXPORT qual_step(Handle ph, long *timeLeft)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_stepQ(pr->project, timeLeft));
}

int DLLEXPORT qual_close(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_closeQ(pr->project));
}




int DLLEXPORT rprt_writeline(Handle ph, char *line)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_writeline(pr->project, line));
}

int DLLEXPORT rprt_writeresults(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_report(pr->project));
}

int DLLEXPORT rprt_reset(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_resetreport(pr->project));
}

int DLLEXPORT rprt_set(Handle ph, char *reportCommand)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setreport(pr->project, reportCommand));
}

int DLLEXPORT rprt_setlevel(Handle ph, EN_StatusReport code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setstatusreport(pr->project, code));
}

int DLLEXPORT rprt_getcount(Handle ph, EN_CountType code, int *count)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcount(pr->project, code, count));
}

int DLLEXPORT rprt_anlysstats(Handle ph, EN_AnalysisStatistic code, EN_API_FLOAT_TYPE* value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getstatistic(pr->project, code, value));
}




int DLLEXPORT anlys_getoption(Handle ph, EN_Option code, EN_API_FLOAT_TYPE *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getoption(pr->project, code, value));
}

int DLLEXPORT anlys_setoption(Handle ph, EN_Option code, EN_API_FLOAT_TYPE value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setoption(pr->project, code, value));
}

int DLLEXPORT anlys_getflowunits(Handle ph, EN_FlowUnits *code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getflowunits(pr->project, (int *)code));
}

int DLLEXPORT anlys_setflowunits(Handle ph, EN_FlowUnits code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setflowunits(pr->project, code));
}

int DLLEXPORT anlys_gettimeparam(Handle ph, EN_TimeProperty code, long *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_gettimeparam(pr->project, code, value));
}

int DLLEXPORT anlys_settimeparam(Handle ph, EN_TimeProperty code, long value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_settimeparam(pr->project, code, value));
}

int DLLEXPORT anlys_getqualinfo(Handle ph, EN_QualityType *qualcode, char *chemname, char *chemunits, int *tracenode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getqualinfo(pr->project, (int *)qualcode, chemname, chemunits, tracenode));
}

int DLLEXPORT anlys_getqualtype(Handle ph, EN_QualityType *qualcode, int *tracenode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getqualtype(pr->project, (int *)qualcode, tracenode));
}

int DLLEXPORT anlys_setqualtype(Handle ph, EN_QualityType qualcode, char *chemname, char *chemunits, char *tracenode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setqualtype(pr->project, qualcode, chemname, chemunits, tracenode));
}




int DLLEXPORT node_add(Handle ph, char *id, EN_NodeType nodeType)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addnode(pr->project, id, nodeType));
}

int DLLEXPORT node_delete(Handle ph, int index, int actionCode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_deletenode(pr->project, index, actionCode));
}

int DLLEXPORT node_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodeindex(pr->project, id, index));
}

int DLLEXPORT node_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodeid(pr->project, index, id));
}

int DLLEXPORT node_setid(Handle ph, int index, char *newid)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodeid(pr->project, index, newid));
}

int DLLEXPORT node_gettype(Handle ph, int index, int *code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodetype(pr->project, index, code));
}

int DLLEXPORT node_getvalue(Handle ph, int index, EN_NodeProperty code, EN_API_FLOAT_TYPE *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnodevalue(pr->project, index, code, value));
}

int DLLEXPORT node_setvalue(Handle ph, int index, EN_NodeProperty code, EN_API_FLOAT_TYPE value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setnodevalue(pr->project, index, code, value));
}

int DLLEXPORT node_getcoord(Handle ph, int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcoord(pr->project, index, x, y));
}

int DLLEXPORT node_setcoord(Handle ph, int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setcoord(pr->project, index, x, y));
}




int DLLEXPORT dmnd_getmodel(Handle ph, int *type, EN_API_FLOAT_TYPE *pmin, EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getdemandmodel(pr->project, type, pmin, preq, pexp));
}

int DLLEXPORT dmnd_setmodel(Handle ph, int type, EN_API_FLOAT_TYPE pmin, EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setdemandmodel(pr->project, type, pmin, preq, pexp));
}

int DLLEXPORT dmnd_getcount(Handle ph, int nodeIndex, int *numDemands)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getnumdemands(pr->project, nodeIndex, numDemands));
}

int DLLEXPORT dmnd_getbase(Handle ph, int nodeIndex, int demandIndex, EN_API_FLOAT_TYPE *baseDemand)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getbasedemand(pr->project, nodeIndex, demandIndex, baseDemand));
}

int DLLEXPORT dmnd_setbase(Handle ph, int nodeIndex, int demandIndex, EN_API_FLOAT_TYPE baseDemand)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setbasedemand(pr->project, nodeIndex, demandIndex, baseDemand));
}

int DLLEXPORT dmnd_getpattern(Handle ph, int nodeIndex, int demandIndex, int *patIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getdemandpattern(pr->project, nodeIndex, demandIndex, patIndex));
}

int DLLEXPORT dmnd_setpattern(Handle ph, int nodeIndex, int demandIndex, int patIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setdemandpattern(pr->project, nodeIndex, demandIndex, patIndex));
}

int DLLEXPORT dmnd_getname(Handle ph, int nodeIndex, int demandIdx, char *demandName)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getdemandname(pr->project, nodeIndex, demandIdx, demandName));
}

int DLLEXPORT dmnd_setname(Handle ph, int nodeIndex, int demandIdx, char *demandName)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setdemandname(pr->project, nodeIndex, demandIdx, demandName));
}




int DLLEXPORT link_add(Handle ph, char *id, EN_LinkType linkType, char *fromNode, char *toNode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addlink(pr->project, id, linkType, fromNode, toNode));
}

int DLLEXPORT link_delete(Handle ph, int index, int actionCode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_deletelink(pr->project, index, actionCode));
}

int DLLEXPORT link_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinkindex(pr->project, id, index));
}

int DLLEXPORT link_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinkid(pr->project, index, id));
}

int DLLEXPORT link_setid(Handle ph, int index, char *newid)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinkid(pr->project, index, newid));
}

int DLLEXPORT link_gettype(Handle ph, int index, int *code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinktype(pr->project, index, (EN_LinkType *)code));
}

int DLLEXPORT link_settype(Handle ph, int *index, EN_LinkType type, int actionCode)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinktype(pr->project, index, type, actionCode));
}

int DLLEXPORT link_getnodes(Handle ph, int index, int *node1, int *node2)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinknodes(pr->project, index, node1, node2));
}

int DLLEXPORT link_setnodes(Handle ph, int index, int node1, int node2)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinknodes(pr->project, index, node1, node2));
}

int DLLEXPORT link_getvalue(Handle ph, int index, EN_LinkProperty code, EN_API_FLOAT_TYPE *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getlinkvalue(pr->project, index, code, value));
}

int DLLEXPORT link_setvalue(Handle ph, int index, int code, EN_API_FLOAT_TYPE value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setlinkvalue(pr->project, index, code, value));
}




int DLLEXPORT pump_gettype(Handle ph, int linkIndex, int *outType)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpumptype(pr->project, linkIndex, outType));
}

int DLLEXPORT pump_getheadcurveindex(Handle ph, int pumpIndex, int *curveIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getheadcurveindex(pr->project, pumpIndex, curveIndex));
}

int DLLEXPORT pump_setheadcurveindex(Handle ph, int pumpIndex, int curveIndex)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setheadcurveindex(pr->project, pumpIndex, curveIndex));
}




int DLLEXPORT ptrn_add(Handle ph, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addpattern(pr->project, id));
}

int DLLEXPORT ptrn_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternindex(pr->project, id, index));
}

int DLLEXPORT ptrn_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternid(pr->project, index, id));
}

int DLLEXPORT ptrn_getlength(Handle ph, int index, int *len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternlen(pr->project, index, len));
}

int DLLEXPORT ptrn_getvalue(Handle ph, int index, int period, EN_API_FLOAT_TYPE *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getpatternvalue(pr->project, index, period, value));
}

int DLLEXPORT ptrn_setvalue(Handle ph, int index, int period, EN_API_FLOAT_TYPE value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpatternvalue(pr->project, index, period, value));
}

int DLLEXPORT ptrn_getavgvalue(Handle ph, int index, EN_API_FLOAT_TYPE *value)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getaveragepatternvalue(pr->project, index, value));
}

int DLLEXPORT ptrn_set(Handle ph, int index, EN_API_FLOAT_TYPE *values, int len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setpattern(pr->project, index, values, len));
}




int DLLEXPORT curv_add(Handle ph, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_addcurve(pr->project, id));
}

int DLLEXPORT curv_getindex(Handle ph, char *id, int *index)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurveindex(pr->project, id, index));
}

int DLLEXPORT curv_getid(Handle ph, int index, char *id)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurveid(pr->project, index, id));
}

int DLLEXPORT curv_getlength(Handle ph, int index, int *len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurvelen(pr->project, index, len));
}

int DLLEXPORT curv_gettype(Handle ph, int index, int *type)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurvetype(pr->project, index, type));
}

int DLLEXPORT curv_getvalue(Handle ph, int curveIndex, int pointIndex, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurvevalue(pr->project, curveIndex, pointIndex, x, y));
}

int DLLEXPORT curv_setvalue(Handle ph, int curveIndex, int pointIndex, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setcurvevalue(pr->project, curveIndex, pointIndex, x, y));
}

int DLLEXPORT curv_get(Handle ph, int curveIndex, char* id, int *nValues, EN_API_FLOAT_TYPE **xValues, EN_API_FLOAT_TYPE **yValues)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcurve(pr->project, curveIndex, id, nValues, xValues, yValues));
}

int DLLEXPORT curv_set(Handle ph, int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y, int len)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setcurve(pr->project, index, x, y, len));
}




void DLLEXPORT err_clear(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    error_clear(pr->error);
}


int DLLEXPORT err_check(Handle ph, char** msg_buffer)
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

int DLLEXPORT toolkit_getversion(int *version)
{
    return EN_getversion(version);
}

void DLLEXPORT toolkit_free(void **memory)
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


