/*
 *  epanet_output.h - EPANET Output API
 *
 *  Created on: Jun 4, 2014
 *
 *      Author: Michael E. Tryby
 *              US EPA - ORD/NRMRL
 */

#ifndef EPANET_OUTPUT_H_
#define EPANET_OUTPUT_H_
/* Epanet Results binary file API */

#define MAXFNAME     259   // Max characters in file name
#define MAXID         31   // Max characters in ID name

// This is an opaque pointer to struct. Do not access variables.
typedef void* ENR_Handle;

typedef enum {
    ENR_node        = 1,
    ENR_link        = 2
} ENR_ElementType;

typedef enum {
    ENR_flowUnits   = 1,
    ENR_pressUnits  = 2,
    ENR_qualUnits   = 3
} ENR_Units;

typedef enum {
    ENR_CFS         = 0,
    ENR_GPM         = 1,
    ENR_MGD         = 2,
    ENR_IMGD        = 3,
    ENR_AFD         = 4,
    ENR_LPS         = 5,
    ENR_LPM         = 6,
    ENR_MLD         = 7,
    ENR_CMH         = 8,
    ENR_CMD         = 9
} ENR_FlowUnits;

typedef enum {
	ENR_PSI         = 0,
	ENR_MTR         = 1,
	ENR_KPA         = 2
} ENR_PressUnits;

typedef enum {
    ENR_NONE        = 0,
    ENR_MGL         = 1,
    ENR_UGL         = 2,
    ENR_HOURS       = 3,
    ENR_PRCNT       = 4
} ENR_QualUnits;

typedef enum {
    ENR_reportStart = 1,
    ENR_reportStep  = 2,
    ENR_simDuration = 3,
    ENR_numPeriods  = 4
}ENR_Time;

typedef enum {
    ENR_demand      = 1,
    ENR_head        = 2,
    ENR_pressure    = 3,
    ENR_quality     = 4
} ENR_NodeAttribute;

typedef enum {
    ENR_flow        = 1,
    ENR_velocity    = 2,
    ENR_headloss    = 3,
    ENR_avgQuality  = 4,
    ENR_status      = 5,
    ENR_setting     = 6,
    ENR_rxRate      = 7,
    ENR_frctnFctr   = 8
} ENR_LinkAttribute;


// #ifdef WINDOWS
// #ifdef __cplusplus
// #define DLLEXPORT __declspec(dllexport) __cdecl
// #else
// #define DLLEXPORT __declspec(dllexport) __stdcall
// #endif
// #else
// #define DLLEXPORT
// #endif


#include "epanet_output_export.h"


#ifdef __cplusplus
extern "C" {
#endif

int DLLEXPORT ENR_init(ENR_Handle* p_handle_out);

int DLLEXPORT ENR_open(ENR_Handle p_handle_in, const char* path);

int DLLEXPORT ENR_getVersion(ENR_Handle p_handle_in, int* int_out);

int DLLEXPORT ENR_getNetSize(ENR_Handle p_handle_in, int** int_out, int* int_dim);

int DLLEXPORT ENR_getUnits(ENR_Handle p_handle_in, ENR_Units t_enum, int* int_out);

int DLLEXPORT ENR_getTimes(ENR_Handle p_handle_in, ENR_Time t_enum, int* int_out);

int DLLEXPORT ENR_getElementName(ENR_Handle p_handle_in, ENR_ElementType t_enum,
        int elementIndex, char** string_out, int* slen);

int DLLEXPORT ENR_getEnergyUsage(ENR_Handle p_handle_in, int pumpIndex,
        int* int_out, float** float_out, int* int_dim);

int DLLEXPORT ENR_getNetReacts(ENR_Handle p_handle_in, float** float_out, int* int_dim);


int DLLEXPORT ENR_getNodeSeries(ENR_Handle p_handle_in, int nodeIndex, ENR_NodeAttribute t_enum,
        int startPeriod, int endPeriod, float** outValueSeries, int* dim);

int DLLEXPORT ENR_getLinkSeries(ENR_Handle p_handle_in, int linkIndex, ENR_LinkAttribute t_enum,
        int startPeriod, int endPeriod, float** outValueSeries, int* dim);

int DLLEXPORT ENR_getNodeAttribute(ENR_Handle p_handle_in, int periodIndex,
        ENR_NodeAttribute t_enum, float** outValueArray, int* dim);

int DLLEXPORT ENR_getLinkAttribute(ENR_Handle p_handle_in, int periodIndex,
        ENR_LinkAttribute t_enum, float** outValueArray, int* dim);

int DLLEXPORT ENR_getNodeResult(ENR_Handle p_handle_in, int periodIndex, int nodeIndex,
        float** float_out, int* int_dim);

int DLLEXPORT ENR_getLinkResult(ENR_Handle p_handle_in, int periodIndex, int linkIndex,
        float** float_out, int* int_dim);

int DLLEXPORT ENR_close(ENR_Handle* p_handle_out);

void DLLEXPORT ENR_free(void** array);

void DLLEXPORT ENR_clearError(ENR_Handle p_handle_in);

int DLLEXPORT ENR_checkError(ENR_Handle p_handle_in, char** msg_buffer);

#ifdef __cplusplus
}
#endif

#endif /* EPANET_OUTPUT_H_ */
