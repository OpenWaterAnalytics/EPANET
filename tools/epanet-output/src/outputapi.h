/*
 * outputapi.h
 *
 *  Created on: Jun 4, 2014
 *      Author: mtryby
 */

#ifndef OUTPUTAPI_H_
#define OUTPUTAPI_H_
/* Epanet Results binary file API */

#define MAXFNAME     259   // Max characters in file name
#define MAXID         31   // Max characters in ID name

// This is an opaque struct. Do not access variables.
typedef struct ENResultsAPI ENResultsAPI;

typedef enum {
	ENR_node = 1,
    ENR_link = 2
} ENR_ElementType;

typedef enum {
    ENR_getSeries    = 1,
    ENR_getAttribute = 2,
    ENR_getResult    = 3,
	ENR_getReacts    = 4,
	ENR_getEnergy    = 5
} ENR_ApiFunction;

typedef enum {
    ENR_nodeCount  = 1,
    ENR_tankCount  = 2,
    ENR_linkCount  = 3,
    ENR_pumpCount  = 4,
    ENR_valveCount = 5
} ENR_ElementCount;

typedef enum {
    ENR_flowUnits   = 1,
    ENR_pressUnits  = 2
} ENR_Unit;

typedef enum {
	ENR_reportStart = 1,
	ENR_reportStep  = 2,
	ENR_simDuration = 3,
	ENR_numPeriods  = 4
}ENR_Time;

typedef enum {
    ENR_demand   = 1,
    ENR_head     = 2,
    ENR_pressure = 3,
    ENR_quality  = 4
} ENR_NodeAttribute;

typedef enum {
    ENR_flow         = 1,
    ENR_velocity     = 2,
    ENR_headloss     = 3,
    ENR_avgQuality   = 4,
    ENR_status       = 5,
    ENR_setting      = 6,
    ENR_rxRate       = 7,
    ENR_frctnFctr    = 8
} ENR_LinkAttribute;


#ifdef WINDOWS
  #ifdef __cplusplus
  #define DLLEXPORT extern "C" __declspec(dllexport) __stdcall
  #else
  #define DLLEXPORT __declspec(dllexport) __stdcall
  #endif
#else
  #ifdef __cplusplus
  #define DLLEXPORT extern "C"
  #else
  #define DLLEXPORT
  #endif
#endif


ENResultsAPI* DLLEXPORT ENR_init(void);

int DLLEXPORT ENR_open(ENResultsAPI* enrapi, const char* path);

int DLLEXPORT ENR_getVersion(ENResultsAPI* enrapi, int* version);

int DLLEXPORT ENR_getNetSize(ENResultsAPI* enrapi, ENR_ElementCount code, int* count);

int DLLEXPORT ENR_getUnits(ENResultsAPI* enrapi, ENR_Unit code, int* unitFlag);

int DLLEXPORT ENR_getTimes(ENResultsAPI* enrapi, ENR_Time code, int* time);

int DLLEXPORT ENR_getElementName(ENResultsAPI* enrapi, ENR_ElementType type,
		int elementIndex, char* name);

int DLLEXPORT ENR_getEnergyUsage(ENResultsAPI* enrapi, int pumpIndex,
		int* linkIndex, float* values);
int DLLEXPORT ENR_getNetReacts(ENResultsAPI* enrapi, float* values);

float* DLLEXPORT ENR_newOutValueSeries(ENResultsAPI* enrapi, int startPeriod,
        int endPeriod, int* length, int* errcode);
float* DLLEXPORT ENR_newOutValueArray(ENResultsAPI* enrapi, ENR_ApiFunction func,
        ENR_ElementType type, int* length, int* errcode);

int DLLEXPORT ENR_getNodeSeries(ENResultsAPI* enrapi, int nodeIndex, ENR_NodeAttribute attr,
        int startPeriod, int length, float* outValueSeries);
int DLLEXPORT ENR_getLinkSeries(ENResultsAPI* enrapi, int linkIndex, ENR_LinkAttribute attr,
        int startPeriod, int length, float* outValueSeries);

int DLLEXPORT ENR_getNodeAttribute(ENResultsAPI* enrapi, int periodIndex,
        ENR_NodeAttribute attr, float* outValueArray, int* length);
int DLLEXPORT ENR_getLinkAttribute(ENResultsAPI* enrapi, int periodIndex,
        ENR_LinkAttribute attr, float* outValueArray, int* length);

int DLLEXPORT ENR_getNodeResult(ENResultsAPI* enrapi, int periodIndex, int nodeIndex,
        float* outValueArray);
int DLLEXPORT ENR_getLinkResult(ENResultsAPI* enrapi, int periodIndex, int linkIndex,
        float* outValueArray);

int DLLEXPORT ENR_free(float *array);

int DLLEXPORT ENR_close(ENResultsAPI* enrapi);

int DLLEXPORT ENR_errMessage(int errcode, char* errmsg, int n);


#endif /* OUTPUTAPI_H_ */

