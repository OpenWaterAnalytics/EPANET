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


#include "epanet_output_enums.h"
#include "epanet_output_export.h"


#ifdef __cplusplus
extern "C" {
#endif

int EXPORT_OUT_API ENR_init(ENR_Handle* p_handle_out);

int EXPORT_OUT_API ENR_open(ENR_Handle p_handle_in, const char* path);

int EXPORT_OUT_API ENR_getVersion(ENR_Handle p_handle_in, int* int_out);

int EXPORT_OUT_API ENR_getNetSize(ENR_Handle p_handle_in, int** int_out, int* int_dim);

int EXPORT_OUT_API ENR_getUnits(ENR_Handle p_handle_in, ENR_Units t_enum, int* int_out);

int EXPORT_OUT_API ENR_getTimes(ENR_Handle p_handle_in, ENR_Time t_enum, int* int_out);

int EXPORT_OUT_API ENR_getElementName(ENR_Handle p_handle_in, ENR_ElementType t_enum,
        int elementIndex, char** string_out, int* slen);

int EXPORT_OUT_API ENR_getEnergyUsage(ENR_Handle p_handle_in, int pumpIndex,
        int* int_out, float** float_out, int* int_dim);

int EXPORT_OUT_API ENR_getNetReacts(ENR_Handle p_handle_in, float** float_out, int* int_dim);


int EXPORT_OUT_API ENR_getNodeSeries(ENR_Handle p_handle_in, int nodeIndex, ENR_NodeAttribute t_enum,
        int startPeriod, int endPeriod, float** outValueSeries, int* dim);

int EXPORT_OUT_API ENR_getLinkSeries(ENR_Handle p_handle_in, int linkIndex, ENR_LinkAttribute t_enum,
        int startPeriod, int endPeriod, float** outValueSeries, int* dim);

int EXPORT_OUT_API ENR_getNodeAttribute(ENR_Handle p_handle_in, int periodIndex,
        ENR_NodeAttribute t_enum, float** outValueArray, int* dim);

int EXPORT_OUT_API ENR_getLinkAttribute(ENR_Handle p_handle_in, int periodIndex,
        ENR_LinkAttribute t_enum, float** outValueArray, int* dim);

int EXPORT_OUT_API ENR_getNodeResult(ENR_Handle p_handle_in, int periodIndex, int nodeIndex,
        float** float_out, int* int_dim);

int EXPORT_OUT_API ENR_getLinkResult(ENR_Handle p_handle_in, int periodIndex, int linkIndex,
        float** float_out, int* int_dim);

int EXPORT_OUT_API ENR_close(ENR_Handle* p_handle_out);

void EXPORT_OUT_API ENR_free(void** array);

void EXPORT_OUT_API ENR_clearError(ENR_Handle p_handle_in);

int EXPORT_OUT_API ENR_checkError(ENR_Handle p_handle_in, char** msg_buffer);

#ifdef __cplusplus
}
#endif

#endif /* EPANET_OUTPUT_H_ */
