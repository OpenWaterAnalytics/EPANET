//-----------------------------------------------------------------------------
//
//   outputapi.c -- API for reading results from EPANet binary output file
//
//   Version:    0.10
//   Date:       08/05/14
//   Date:       05/21/14
//
//   Author:     Michael E. Tryby
//               US EPA - NRMRL
//
//   Purpose: Output API provides an interface for retrieving results from
//   an EPANet binary output file.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "outputapi.h"

#define INT4  int
#define REAL4 float
#define RECORDSIZE    4          // number of bytes per file record

#define MEMCHECK(x)  (((x) == NULL) ? 411 : 0 )

#define MINNREC      14          // minimum allowable number of records
#define NNODERESULTS  4          // number of result fields for nodes
#define NLINKRESULTS  8          // number of result fields for links


struct ENResultsAPI {
    char name[MAXFNAME + 1];     // file path/name
    bool isOpened;               // current state (CLOSED = 0, OPEN = 1)
    FILE *file;                  // FILE structure pointer

    INT4 nodeCount, tankCount, linkCount, pumpCount, valveCount;
    INT4 reportStart, reportStep, simDuration, nPeriods;

    INT4 flowFlag, pressFlag;

    INT4 outputStartPos;         // starting file position of output data
    INT4 bytesPerPeriod;         // bytes saved per simulation time period
};

//-----------------------------------------------------------------------------
//   Local functions
//-----------------------------------------------------------------------------
float  getNodeValue(ENResultsAPI*, int, int, ENR_NodeAttribute);
float  getLinkValue(ENResultsAPI*, int, int, ENR_LinkAttribute);


ENResultsAPI* DLLEXPORT ENR_alloc(void)
{
	return malloc(sizeof(struct ENResultsAPI));
}

int DLLEXPORT ENR_open(ENResultsAPI* enrapi, const char* path)
//
//  Purpose: Open the output binary file and read epilogue
//
{
    int magic1, magic2, errCode, version;

    strncpy(enrapi->name, path, MAXFNAME);
    enrapi->isOpened = false;

    // Attempt to open binary output file for reading only
    if ((enrapi->file = fopen(path, "rb")) == NULL)
        return 434;
    else
        enrapi->isOpened = true;

    // Fast forward to end and check for minimum number of records
    fseek(enrapi->file, 0L, SEEK_END);
    if (ftell(enrapi->file) < MINNREC*RECORDSIZE) {
        fclose(enrapi->file);
        // Error run terminated no results in binary file
        return 435;
    }

    // Fast forward to end and read file epilogue
    fseek(enrapi->file, -3*RECORDSIZE, SEEK_END);
    fread(&(enrapi->nPeriods), RECORDSIZE, 1, enrapi->file);
    fread(&errCode, RECORDSIZE, 1, enrapi->file);
    fread(&magic2, RECORDSIZE, 1, enrapi->file);

    // Rewind and read magic number from beginning of file
    fseek(enrapi->file, 0L, SEEK_SET);
    fread(&magic1, RECORDSIZE, 1, enrapi->file);

    // Perform error checks
    if (magic1 != magic2 || errCode != 0 || enrapi->nPeriods == 0) {
        fclose(enrapi->file);
        // Error run terminated no results in binary file
        return 435;
    }

    // Otherwise read network size
    fread(&version, RECORDSIZE, 1, enrapi->file);
    fread(&(enrapi->nodeCount), RECORDSIZE, 1, enrapi->file);
    fread(&(enrapi->tankCount), RECORDSIZE, 1, enrapi->file);
    fread(&(enrapi->linkCount), RECORDSIZE, 1, enrapi->file);
    fread(&(enrapi->pumpCount), RECORDSIZE, 1, enrapi->file);

    // Jump ahead and read flow and pressure units
    fseek(enrapi->file, 3*RECORDSIZE, SEEK_CUR);
    fread(&(enrapi->flowFlag), RECORDSIZE, 1, enrapi->file);
    fread(&(enrapi->pressFlag), RECORDSIZE, 1, enrapi->file);

    // Jump ahead and read time information
    fseek(enrapi->file, RECORDSIZE, SEEK_CUR);
    fread(&(enrapi->reportStart), RECORDSIZE, 1, enrapi->file);
    fread(&(enrapi->reportStep), RECORDSIZE, 1, enrapi->file);
    fread(&(enrapi->simDuration), RECORDSIZE, 1, enrapi->file);

    // Compute positions and offsets for retrieving data
    enrapi->outputStartPos  = 884;
    enrapi->outputStartPos += 32*enrapi->nodeCount + 32*enrapi->linkCount;
    enrapi->outputStartPos += 12*enrapi->linkCount+ 8*enrapi->tankCount
    		+ 4*enrapi->nodeCount + 8*enrapi->linkCount;
    enrapi->outputStartPos += 28*enrapi->pumpCount + 4;

    enrapi->bytesPerPeriod = 16*enrapi->nodeCount + 32*enrapi->linkCount;

    return 0;
}

int DLLEXPORT ENR_getNetSize(ENResultsAPI* enrapi, ENR_ElementCount code, int* count)
//
//   Purpose: Returns network size
//
{
    *count = -1;
    if (enrapi->isOpened) {
        switch (code)
        {
        case ENR_nodeCount:    *count = enrapi->nodeCount;  break;
        case ENR_tankCount:    *count = enrapi->tankCount;  break;
        case ENR_linkCount:    *count = enrapi->linkCount;  break;
        case ENR_pumpCount:    *count = enrapi->pumpCount;  break;
		case ENR_valveCount:   *count = enrapi->valveCount;  break;
        default: return 421;
        }
        return 0;
    }
    return 412;
}

int DLLEXPORT ENR_getUnits(ENResultsAPI* enrapi, ENR_Unit code, int* unitFlag)
//
//   Purpose: Returns pressure and flow units
//
{
    *unitFlag = -1;
    if (enrapi->isOpened) {
        switch (code)
        {
        case ENR_flowUnits:   *unitFlag = enrapi->flowFlag;  break;
        case ENR_pressUnits:  *unitFlag = enrapi->pressFlag; break;
        default: return 421;
        }
        return 0;
    }
    return 412;
}

int DLLEXPORT ENR_getTimes(ENResultsAPI* enrapi, ENR_Time code, int* time)
//
//   Purpose: Returns report and simulation time related parameters.
//
{
    *time = -1;
    if (enrapi->isOpened) {
        switch (code)
        {
        case ENR_reportStart: *time = enrapi->reportStart;  break;
        case ENR_reportStep:  *time = enrapi->reportStep;   break;
        case ENR_simDuration: *time = enrapi->simDuration;  break;
        case ENR_numPeriods:  *time = enrapi->nPeriods;     break;
        default: return 421;
        }
        return 0;
    }
    return 412;
}


float* ENR_newOutValueSeries(ENResultsAPI* enrapi, int seriesStart,
        int seriesLength, int* length, int* errcode)
//
//  Purpose: Allocates memory for outValue Series.
//
//  Warning: Caller must free memory allocated by this function using ENR_free().
//
{
    int size;
    float* array;

    if (enrapi->isOpened) {

        size = seriesLength - seriesStart;
        if (size > enrapi->nPeriods)
            size = enrapi->nPeriods;

        // Allocate memory for outValues
        array = (float*) calloc(size + 1, sizeof(float));
        *errcode = (MEMCHECK(array));

        *length = size;
        return array;
    }
    *errcode = 412;
    return NULL;
}

float* ENR_newOutValueArray(ENResultsAPI* enrapi, ENR_ApiFunction func,
        ENR_ElementType type, int* length, int* errcode)
//
//  Purpose: Allocates memory for outValue Array.
//
//  Warning: Caller must free memory allocated by this function using ENR_free().
//
{
    int size;
    float* array;

    if (enrapi->isOpened) {
        switch (func)
        {
        case ENR_getAttribute:
            if (type == ENR_node)
                size = enrapi->nodeCount;
            else
                size = enrapi->linkCount;
            break;
        case ENR_getResult:
            if (type == ENR_node)
                size = NNODERESULTS;
            else
                size = NLINKRESULTS;
            break;
        default: *errcode = 421;
                 return NULL;
        }

        // Allocate memory for outValues
        array = (float*) calloc(size, sizeof(float));
        *errcode = (MEMCHECK(array));

        *length = size;
        return array;
    }
    *errcode = 412;
    return NULL;
}


int DLLEXPORT ENR_getNodeSeries(ENResultsAPI* enrapi, int nodeIndex, ENR_NodeAttribute attr,
        int seriesStart, int seriesLength, float* outValueSeries, int* length)
//
//  What if timeIndex 0? length 0?
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using seriesStart and seriesLength respectively.
//
{
    int k;

    if (enrapi->isOpened) {

        // Check memory for outValues
        if (outValueSeries == NULL) return 411;

        // loop over and build time series
        for (k = 0; k <= seriesLength; k++)
            outValueSeries[k] = getNodeValue(enrapi, seriesStart + 1 + k,
            		nodeIndex, attr);

        return 0;
    }
    // Error no results to report on binary file not opened
    return 412;
}

int DLLEXPORT ENR_getLinkSeries(ENResultsAPI* enrapi, int linkIndex, ENR_LinkAttribute attr,
        int seriesStart, int seriesLength, float* outValueSeries)
//
//  What if timeIndex 0? length 0?
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using seriesStart and seriesLength respectively.
//
{
    int k;

    if (enrapi->isOpened) {
        // Check memory for outValues
        if (outValueSeries == NULL) return 411;

        // loop over and build time series
        for (k = 0; k <= seriesLength; k++)
            outValueSeries[k] = getLinkValue(enrapi, seriesStart +1 + k,
            		linkIndex, attr);

        return 0;
    }
    // Error no results to report on binary file not opened
    return 412;
}


int DLLEXPORT ENR_getNodeAttribute(ENResultsAPI* enrapi, int timeIndex,
        ENR_NodeAttribute attr, float* outValueArray)
//
//   Purpose: For all nodes at given time, get a particular attribute
//
{
    INT4 offset;

    if (enrapi->isOpened) {
        // Check memory for outValues
        if (outValueArray == NULL) return 411;

        // calculate byte offset to start time for series
        offset = enrapi->outputStartPos + (timeIndex)*enrapi->bytesPerPeriod;
        // add offset for node and attribute
        offset += (attr*enrapi->nodeCount)*RECORDSIZE;

        fseek(enrapi->file, offset, SEEK_SET);
        fread(outValueArray, RECORDSIZE, enrapi->nodeCount, enrapi->file);

        return 0;
    }
    // Error no results to report on binary file not opened
    return 412;
}

int DLLEXPORT ENR_getLinkAttribute(ENResultsAPI* enrapi, int timeIndex,
        ENR_LinkAttribute attr, float* outValueArray)
//
//   Purpose: For all links at given time, get a particular attribute
//
{
    INT4 offset;

    if (enrapi->isOpened) {
        // Check memory for outValues
        if (outValueArray == NULL) return 411;

        // calculate byte offset to start time for series
        offset = enrapi->outputStartPos + (timeIndex)*enrapi->bytesPerPeriod
                + (NNODERESULTS*enrapi->nodeCount)*RECORDSIZE;
        // add offset for link and attribute
        offset += (attr*enrapi->linkCount)*RECORDSIZE;

        fseek(enrapi->file, offset, SEEK_SET);
        fread(outValueArray, RECORDSIZE, enrapi->linkCount, enrapi->file);

        return 0;
    }
    // Error no results to report on binary file not opened
    return 412;
}

int DLLEXPORT ENR_getNodeResult(ENResultsAPI* enrapi, int timeIndex, int nodeIndex,
        float* outValueArray)
//
//   Purpose: For a node at given time, get all attributes
//
{
    int j;

    if (enrapi->isOpened) {
        // Check memory for outValues
        if (outValueArray == NULL) return 411;

        for (j = 0; j < NNODERESULTS; j++)
            outValueArray[j] = getNodeValue(enrapi, timeIndex + 1, nodeIndex, j);

        return 0;
    }
    // Error no results to report on binary file not opened
    return 412;
}

int DLLEXPORT ENR_getLinkResult(ENResultsAPI* enrapi, int timeIndex, int linkIndex,
        float* outValueArray)
//
//   Purpose: For a link at given time, get all attributes
//
{
    int j;

    if (enrapi->isOpened) {
        // Check memory for outValues
        if (outValueArray == NULL) return 411;

        for (j = 0; j < NLINKRESULTS; j++)
            outValueArray[j] = getLinkValue(enrapi, timeIndex + 1, linkIndex, j);

        return 0;
    }
    // Error no results to report on binary file not opened
    return 412;
}

int DLLEXPORT ENR_free(float* array)
//
//  Purpose: frees memory allocated using ENR_newOutValueSeries() or
//  ENR_newOutValueArray()
//
{
	if (array != NULL)
		free(array);

	return 0;
}


int DLLEXPORT ENR_close(ENResultsAPI* enrapi)
//
//   Purpose: Clean up after and close Output API
//
{
    if (enrapi->isOpened) {
        fclose(enrapi->file);
        free(enrapi);
    }
    // Error binary file not opened
    else return 412;

    return 0;
}


int DLLEXPORT ENR_errMessage(int errcode, char* errmsg, int n)
//
//  Purpose: takes error code returns error message
//
//  Input Error 411: no memory allocated for results
//  Input Error 412: no results binary file hasn't been opened
//  Input Error 421: invalid parameter code
//  File Error  434: unable to open binary output file
//  File Error  435: run terminated no results in binary file
{
    switch (errcode)
    {
    case 411: strncpy(errmsg, ERR411, n); break;
    case 412: strncpy(errmsg, ERR412, n); break;
    case 421: strncpy(errmsg, ERR421, n); break;
    case 434: strncpy(errmsg, ERR434, n); break;
    case 435: strncpy(errmsg, ERR435, n); break;
    default: return 421;
    }

    return 0;
}


float getNodeValue(ENResultsAPI* enrapi, int timeIndex, int nodeIndex,
		ENR_NodeAttribute attr)
//
//   Purpose: Retrieves an attribute value at a specified node and time
//
{
    REAL4 y;
    INT4  offset;

    // calculate byte offset to start time for series
    offset = enrapi->outputStartPos + (timeIndex - 1)*enrapi->bytesPerPeriod;
    // add bytepos for node and attribute
    offset += (nodeIndex + attr*enrapi->nodeCount)*RECORDSIZE;

    fseek(enrapi->file, offset, SEEK_SET);
    fread(&y, RECORDSIZE, 1, enrapi->file);

    return y;
}

float getLinkValue(ENResultsAPI* enrapi, int timeIndex, int linkIndex,
        ENR_LinkAttribute attr)
//
//   Purpose: Retrieves an attribute value at a specified link and time
//
{
    REAL4 y;
    INT4  offset;

    // Calculate byte offset to start time for series
    offset = enrapi->outputStartPos + (timeIndex - 1)*enrapi->bytesPerPeriod
            + (NNODERESULTS*enrapi->nodeCount)*RECORDSIZE;
    // add bytepos for link and attribute
    offset += (linkIndex + attr*enrapi->linkCount)*RECORDSIZE;

    fseek(enrapi->file, offset, SEEK_SET);
    fread(&y, RECORDSIZE, 1, enrapi->file);

    return y;
}
