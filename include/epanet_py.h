



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

int DLLEXPORT hyd_solve(Handle ph);
int DLLEXPORT hyd_save(Handle ph);
int DLLEXPORT hyd_open(Handle ph);
int DLLEXPORT hyd_init(Handle ph, EN_SaveOption saveFlag);
int DLLEXPORT hyd_run(Handle ph, long *currentTime);
int DLLEXPORT hyd_next(Handle ph, long *tStep);
int DLLEXPORT hyd_close(Handle ph);
int DLLEXPORT hyd_savefile(Handle ph, char *filename);
int DLLEXPORT hyd_usefile(Handle ph, char *filename);

int DLLEXPORT qual_solve(Handle ph);
int DLLEXPORT qual_open(Handle ph);
int DLLEXPORT qual_init(Handle ph, EN_SaveOption saveFlag);
int DLLEXPORT qual_run(Handle ph, long *currentTime);
int DLLEXPORT qual_next(Handle ph, long *tStep);
int DLLEXPORT qual_step(Handle ph, long *timeLeft);
int DLLEXPORT qual_close(Handle ph);

int DLLEXPORT rpt_writeline(Handle ph, char *line);
int DLLEXPORT rpt_writeresults(Handle ph);
int DLLEXPORT rpt_reset(Handle ph);
int DLLEXPORT rpt_set(Handle ph, char *reportCommand);
int DLLEXPORT rpt_setlevel(Handle ph, EN_StatusReport code);
int DLLEXPORT rpt_getcount(Handle ph, EN_CountType code, int *count);
int DLLEXPORT rpt_anlysstats(Handle ph, EN_AnalysisStatistic code, EN_API_FLOAT_TYPE* value);

int DLLEXPORT anlys_getoption(Handle ph, EN_Option opt, EN_API_FLOAT_TYPE *value);
int DLLEXPORT anlys_setoption(Handle ph, int code, EN_API_FLOAT_TYPE value);
int DLLEXPORT anlys_getflowunits(Handle ph, EN_FlowUnits *code);
int DLLEXPORT anlys_setflowunits(Handle ph, EN_FlowUnits code);
int DLLEXPORT anlys_gettimeparam(Handle ph, EN_TimeProperty code, long *value);
int DLLEXPORT anlys_settimeparam(Handle ph, EN_TimeProperty code, long value);
int DLLEXPORT anlys_getqualinfo(Handle ph, EN_QualityType *qualcode, char *chemname, char *chemunits, int *tracenode);
int DLLEXPORT anlys_getqualtype(Handle ph, EN_QualityType *qualcode, int *tracenode);
int DLLEXPORT anlys_setqualtype(Handle ph, EN_QualityType qualcode, char *chemname, char *chemunits, char *tracenode);





void DLLEXPORT err_clear(Handle ph);
int  DLLEXPORT err_check(Handle ph, char** msg_buffer);
void DLLEXPORT toolkit_free(void **memory);
int  DLLEXPORT toolkit_getversion(int *version);


#if defined(__cplusplus)
}
#endif


#endif //EPANET_PY_H
