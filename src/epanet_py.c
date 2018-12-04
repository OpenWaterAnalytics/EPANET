

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




int DLLEXPORT hyd_solve(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_solveH(pr->project));
}

int DLLEXPORT hyd_save(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_saveH(pr->project));
}

int DLLEXPORT hyd_open(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_openH(pr->project));
}

int DLLEXPORT hyd_init(Handle ph, EN_SaveOption saveFlag)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_initH(pr->project, saveFlag));
}

int DLLEXPORT hyd_run(Handle ph, long *currentTime)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_runH(pr->project, currentTime));
}

int DLLEXPORT hyd_next(Handle ph, long *tStep)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_nextH(pr->project, tStep));
}

int DLLEXPORT hyd_close(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_closeH(pr->project));
}

int DLLEXPORT hyd_savefile(Handle ph, char *filename)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_savehydfile(pr->project, filename));
}

int DLLEXPORT hyd_usefile(Handle ph, char *filename)
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




int DLLEXPORT rpt_writeline(Handle ph, char *line)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_writeline(pr->project, line));
}

int DLLEXPORT rpt_writeresults(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_report(pr->project));
}

int DLLEXPORT rpt_reset(Handle ph)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_resetreport(pr->project));
}

int DLLEXPORT rpt_set(Handle ph, char *reportCommand)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setreport(pr->project, reportCommand));
}

int DLLEXPORT rpt_setlevel(Handle ph, EN_StatusReport code)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_setstatusreport(pr->project, code));
}

int DLLEXPORT rpt_getcount(Handle ph, EN_CountType code, int *count)
{
    handle_t *pr = (handle_t *)ph;
    return error_set(pr->error, EN_getcount(pr->project, code, count));
}

int DLLEXPORT rpt_anlysstats(Handle ph, EN_AnalysisStatistic code, EN_API_FLOAT_TYPE* value)
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


