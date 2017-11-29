//-----------------------------------------------------------------------------
//
//   epanet_output.c -- API for reading results from EPANET binary output file
//
//   Version:    0.30
//   Date        09/06/2017
//               06/17/2016
//               08/05/2014
//               05/21/2014
//
//   Author:     Michael E. Tryby
//               US EPA - ORD/NRMRL
//
//   Modified:   Maurizio Cingi
//               University of Modena
//
//   Purpose: Output API provides an interface for retrieving results from an
//   EPANET binary output file.
//
//   Output data in the binary file are aligned on a 4 byte word size.
//   Therefore all values both integers and reals are 32 bits in length.
//
//   All values returned by the output API are indexed from 0 to n-1. This
//   differs from how node and link elements are indexed by the binary file
//   writer found in EPANET. Times correspond to reporting periods are indexed
//   from 0 to number of reporting periods minus one. Node and link elements
//   are indexed from 0 to nodeCount minus one and 0 to linkCount minus one
//   respectively.
//
//   The Output API functions provide a convenient way to select "slices" of
//   data from the output file. As such they return arrays of data. The API
//   functions automatically allocate memory for the array to be returned. The
//   caller is responsible for deallocating memory. The function ENR_free() is
//   provided to deallocate memory.
//
//-----------------------------------------------------------------------------

#include "epanet_output.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "errormanager.h"
#include "messages.h"

// NOTE: These depend on machine data model and may change when porting
// F_OFF Must be a 8 byte / 64 bit integer for large file support
#ifdef _WIN32 // Windows (32-bit and 64-bit)
#define F_OFF __int64
#else         // Other platforms
#define F_OFF off_t
#endif
#define INT4  int          // Must be a 4 byte / 32 bit integer type
#define REAL4 float        // Must be a 4 byte / 32 bit real type
#define WORDSIZE       4   // Memory alignment 4 byte word size for both int and real

#define MINNREC       14   // Minimum allowable number of records
#define PROLOGUE     884   // Preliminary fixed length section of header
#define MAXID_P1      32   // Max. # characters in ID name

#define NELEMENTTYPES  5   // Number of element types
#define NENERGYRESULTS 6   // Number of energy results
#define NNODERESULTS   4   // number of result fields for nodes
#define NLINKRESULTS   8   // number of result fields for links
#define NREACTRESULTS  4   // number of net reaction results

#define MEMCHECK(x)  (((x) == NULL) ? 411 : 0 )

// Typedefs for opaque pointer
typedef struct data_s {
    char  name[MAXFNAME+1];  // file path/name
    FILE* file;            // FILE structure pointer
    INT4  nodeCount, tankCount, linkCount, pumpCount, valveCount, nPeriods;
    F_OFF outputStartPos;  // starting file position of output data
    F_OFF bytesPerPeriod;  // bytes saved per simulation time period

    error_handle_t* error_handle;
} data_t;

//-----------------------------------------------------------------------------
//   Local functions
//-----------------------------------------------------------------------------
void errorLookup(int errcode, char* errmsg, int length);
int    validateFile(ENR_Handle);
float  getNodeValue(ENR_Handle, int, int, int);
float  getLinkValue(ENR_Handle, int, int, int);

int _fopen(FILE **f, const char *name, const char *mode);
int _fseek(FILE* stream, F_OFF offset, int whence);
F_OFF _ftell(FILE* stream);

float* newFloatArray(int n);
int* newIntArray(int n);
char* newCharArray(int n);


int DLLEXPORT ENR_init(ENR_Handle* dp_handle)
//  Purpose: Initialized pointer for the opaque ENR_Handle.
//
//  Returns: Error code 0 on success, -1 on failure
//
//  Note: The existence of this function has been carefully considered.
//   Don't change it.
//
{
    int errorcode = 0;
    data_t* p_data;

    // Allocate memory for private data
    p_data = (data_t*)calloc(1, sizeof(data_t));

    if (p_data != NULL){
        p_data->error_handle = new_errormanager(&errorLookup);
        *dp_handle = p_data;
    }
    else
        errorcode = -1;

    // TODO: Need to handle errors during initialization better.
    return errorcode;
}

int DLLEXPORT ENR_close(ENR_Handle* p_handle)
/*------------------------------------------------------------------------
 **    Input:  *p_handle = pointer to ENR_Handle struct
 **
 **  Returns:  Error code 0 on success, -1 on failure
 **
 **  Purpose:  Close the output binary file, dellocate ENR_Handle struc
 **            and nullify pointer to ENR_Handle struct
 **
 **  NOTE: ENR_close must be called before program end
 **        after calling ENR_close data in  ENR_Handle struct are no more
 **        accessible
 **-------------------------------------------------------------------------
 */
{
    data_t* p_data;
    int errorcode = 0;

    p_data = (data_t*)(*p_handle);

    if (p_data == NULL || p_data->file == NULL)
        errorcode = -1;

    else
    {
        dst_errormanager(p_data->error_handle);
        fclose(p_data->file);
        free(p_data);

        *p_handle = NULL;
    }

    return errorcode;
}

int DLLEXPORT ENR_open(ENR_Handle p_handle, const char* path)
/*------------------------------------------------------------------------
 **   Input:   path
 **   Output:  p_handle = pointer to ENR_Handle struct
 **  Returns:  warning / error code
 **  Purpose:  Opens the output binary file and reads prologue and epilogue
 **
 **  NOTE: ENR_init must be called before anyother ENR_* functions
 **-------------------------------------------------------------------------
 */
{
    int err, errorcode = 0;
    F_OFF bytecount;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else
    {
        strncpy(p_data->name, path, MAXFNAME);
        // Attempt to open binary output file for reading only
        if ((_fopen(&(p_data->file), path, "rb")) != 0) errorcode = 434;

        // Perform checks to insure the file is valid
        else if ((err = validateFile(p_data)) != 0) errorcode = err;

        // If a warning is encountered read file header
        if (errorcode < 400 ) {

            // read network size
            fseek(p_data->file, 2*WORDSIZE, SEEK_SET);
            fread(&(p_data->nodeCount), WORDSIZE, 1, p_data->file);
            fread(&(p_data->tankCount), WORDSIZE, 1, p_data->file);
            fread(&(p_data->linkCount), WORDSIZE, 1, p_data->file);
            fread(&(p_data->pumpCount), WORDSIZE, 1, p_data->file);
            fread(&(p_data->valveCount), WORDSIZE, 1, p_data->file);

            // Compute positions and offsets for retrieving data
            // fixed portion of header + title section + filenames + chem names
            bytecount = PROLOGUE;
            // node names + link names
            bytecount += MAXID_P1*p_data->nodeCount + MAXID_P1*p_data->linkCount;
            // network connectivity + tank nodes + tank areas
            bytecount += 3*WORDSIZE*p_data->linkCount + 2*WORDSIZE*p_data->tankCount;
            // node elevations + link lengths and link diameters
            bytecount += WORDSIZE*p_data->nodeCount + 2*WORDSIZE*p_data->linkCount;
            // pump energy summary
            bytecount += 7*WORDSIZE*p_data->pumpCount + WORDSIZE;
            p_data->outputStartPos= bytecount;

            p_data->bytesPerPeriod = NNODERESULTS*WORDSIZE*p_data->nodeCount +
                    NLINKRESULTS*WORDSIZE*p_data->linkCount;
        }
    }
    // If error close the binary file
    if (errorcode > 400) {
        set_error(p_data->error_handle, errorcode);
        ENR_close(&p_handle);
    }

    return errorcode;
}

int DLLEXPORT ENR_getVersion(ENR_Handle p_handle, int* version)
/*------------------------------------------------------------------------
 **    Input: p_handle = pointer to ENR_Handle struct
 **   Output: version  Epanet version
 **  Returns: error code
 **
 **  Purpose: Returns Epanet version that wrote EBOFile
 **--------------element codes-------------------------------------------
 */
{
    int errorcode = 0;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else
    {
        fseek(p_data->file, 1*WORDSIZE, SEEK_SET);
        if (fread(version, WORDSIZE, 1, p_data->file) != 1)
            errorcode = 436;
    }

    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getNetSize(ENR_Handle p_handle, int** elementCount, int* length)
/*------------------------------------------------------------------------
 **    Input:   p_handle = pointer to ENR_Handle struct
 **   Output:  array of element counts (nodes, tanks, links, pumps, valves)
 **  Returns: error code
 **  Purpose: Returns an array of count values
 **-------------------------------------------------------------------------
 */
{
    int errorcode = 0;
    int* temp = newIntArray(NELEMENTTYPES);
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else
    {
        temp[0] = p_data->nodeCount;
        temp[1] = p_data->tankCount;
        temp[2] = p_data->linkCount;
        temp[3] = p_data->pumpCount;
        temp[4] = p_data->valveCount;

        *elementCount = temp;
        *length = NELEMENTTYPES;
    }

    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getUnits(ENR_Handle p_handle, ENR_Units code, int* unitFlag)
/*------------------------------------------------------------------------
 **   Input:   p_handle = pointer to ENR_Handle struct
 **            code
 **   Output:  count
 **  Returns: unitFlag
 **  Purpose: Returns pressure or flow unit flag
 **--------------pressure unit flags----------------------------------------
 **  0 = psi
 **  1 = meters
 **  2 = kPa
 **------------------flow unit flags----------------------------------------
 **  0 = cubic feet/second
 **  1 = gallons/minute
 **  2 = million gallons/day
 **  3 = Imperial million gallons/day
 **  4 = acre-ft/day
 **  5 = liters/second
 **  6 = liters/minute
 **  7 = megaliters/day
 **  8 = cubic meters/hour
 **  9 = cubic meters/day
 **-------------------------------------------------------------------------
 */
{
    int errorcode = 0;
    data_t* p_data;

    *unitFlag = -1;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else
    {
        switch (code)
        {
        case ENR_flowUnits:
            fseek(p_data->file, 9*WORDSIZE, SEEK_SET);
            fread(unitFlag, WORDSIZE, 1, p_data->file);
            break;

        case ENR_pressUnits:
            fseek(p_data->file, 10*WORDSIZE, SEEK_SET);
            fread(unitFlag, WORDSIZE, 1, p_data->file);
            break;

        default: errorcode = 421;
        }
    }
    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getTimes(ENR_Handle p_handle, ENR_Time code, int* time)
/*------------------------------------------------------------------------
 **   Input:   p_handle = pointer to ENR_Handle struct
 **            code = element code
 **   Output:  time
 **  Returns: error code
 **  Purpose: Returns report and simulation time related parameters.
 **-------------------------------------------------------------------------
 */
{
    int errorcode = 0;
    data_t* p_data;

    *time = -1;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else
    {
        switch (code)
        {
        case ENR_reportStart:
            fseek(p_data->file, 12*WORDSIZE, SEEK_SET);
            fread(time, WORDSIZE, 1, p_data->file);
            break;

        case ENR_reportStep:
            fseek(p_data->file, 13*WORDSIZE, SEEK_SET);
            fread(time, WORDSIZE, 1, p_data->file);
            break;

        case ENR_simDuration:
            fseek(p_data->file, 14*WORDSIZE, SEEK_SET);
            fread(time, WORDSIZE, 1, p_data->file);
            break;

        case ENR_numPeriods:
            *time = p_data->nPeriods;
            break;

        default:
            errorcode = 421;
        }
    }
    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getElementName(ENR_Handle p_handle, ENR_ElementType type,
        int elementIndex, char** name, int* length)
/*------------------------------------------------------------------------
 **   Input:   p_handle = pointer to ENR_Handle struct
 **            type = ENR_node or ENR_link
 **            elementIndex from 1 to nodeCount or 1 to linkCount
 **   Output:  name = elementName
 **  Returns: error code
 **  Purpose: Retrieves Name of a specified node or link element
 **  NOTE: 'name' must be able to hold MAXID characters
 **  TODO: Takes EPANET indexing from 1 to n not 0 to n-1
 **-------------------------------------------------------------------------
 */
{
    F_OFF offset;
    int errorcode = 0;
    char* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    /* Allocate memory for name */
    else if MEMCHECK(temp = newCharArray(MAXID_P1)) errorcode = 411;

    else
    {
        switch (type)
        {
        case ENR_node:
            if (elementIndex < 1 || elementIndex > p_data->nodeCount)
                errorcode = 423;
            else offset = PROLOGUE + (elementIndex - 1)*MAXID_P1;
            break;

        case ENR_link:
            if (elementIndex < 1 || elementIndex > p_data->linkCount)
                errorcode = 423;
            else
                offset = PROLOGUE + p_data->nodeCount*MAXID_P1 +
                (elementIndex - 1)*MAXID_P1;
            break;

        default:
            errorcode = 421;
        }

        if (!errorcode)
        {
            _fseek(p_data->file, offset, SEEK_SET);
            fread(temp, 1, MAXID_P1, p_data->file);

            *name = temp;
            *length = MAXID_P1;
        }
    }

    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getEnergyUsage(ENR_Handle p_handle, int pumpIndex,
        int* linkIndex, float** outValues, int* length)
/*
 * Purpose: Returns pump energy usage statistics.
 *
 * Energy usage statistics:
 *    0 = pump utilization
 *    1 = avg. efficiency
 *    2 = avg. kW/flow
 *    3 = avg. kwatts
 *    4 = peak kwatts
 *    5 = cost/day
 */
{
    F_OFF offset;
    int errorcode = 0;
    float* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    // Check for valid pump index
    else if (pumpIndex < 1 || pumpIndex > p_data->pumpCount) errorcode = 423;
    // Check memory for outValues
    else if MEMCHECK(temp = newFloatArray(NENERGYRESULTS)) errorcode = 411;

    else
    {
        // Position offset to start of pump energy summary
        offset = p_data->outputStartPos - (p_data->pumpCount*(WORDSIZE + 6*WORDSIZE) + WORDSIZE);
        // Adjust offset by pump index
        offset += (pumpIndex - 1)*(WORDSIZE + 6*WORDSIZE);

        // Power summary is 1 int and 6 floats for each pump
        _fseek(p_data->file, offset, SEEK_SET);
        fread(linkIndex, WORDSIZE, 1, p_data->file);
        fread(temp, WORDSIZE, 6, p_data->file);

        *outValues = temp;
        *length = NENERGYRESULTS;
    }
    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getNetReacts(ENR_Handle p_handle, float** outValues, int* length)
/*
 *  Purpose: Returns network wide average reaction rates and average
 *  source mass inflow:
 *     0 = bulk
 *     1 = wall
 *     2 = tank
 *     3 = source
 */
{
    F_OFF offset;
    int errorcode = 0;
    float* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    // Check memory for outValues
    else if MEMCHECK(temp = newFloatArray(NREACTRESULTS)) errorcode = 411;

    else
    {
        // Reaction summary is 4 floats located right before epilogue.
        // This offset is relative to the end of the file.
        offset = - 3*WORDSIZE - 4*WORDSIZE;
        _fseek(p_data->file, offset, SEEK_END);
        fread(temp, WORDSIZE, 4, p_data->file);

        *outValues = temp;
        *length = NREACTRESULTS;
    }
    return set_error(p_data->error_handle, errorcode);
}

void DLLEXPORT ENR_free(void** array)
//
//  Purpose: Frees memory allocated by API calls
//
{
    if (array != NULL) {
        free(*array);
        *array = NULL;
    }
}

int DLLEXPORT ENR_getNodeSeries(ENR_Handle p_handle, int nodeIndex, ENR_NodeAttribute attr,
        int startPeriod, int endPeriod, float** outValueSeries, int* dim)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using seriesStart and seriesLength respectively.
//
//  NOTE: The node index argument corresponds to the EPANET node index from 1 to
//     nnodes. The series returned is indexed from 0 to nperiods - 1.
//
{
    int k, length, errorcode = 0;
    float* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else if (nodeIndex < 1 || nodeIndex > p_data->nodeCount) errorcode = 423;
    else if (startPeriod < 0 || endPeriod >= p_data->nPeriods ||
            endPeriod <= startPeriod) errorcode = 422;
    // Check memory for outValues
    else if MEMCHECK(temp = newFloatArray(length = endPeriod - startPeriod)) errorcode = 411;
    else
    {
        // loop over and build time series
        for (k = 0; k < length; k++)
            temp[k] = getNodeValue(p_handle, startPeriod + k,
                    nodeIndex, attr);

        *outValueSeries = temp;
        *dim = length;
    }
    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getLinkSeries(ENR_Handle p_handle, int linkIndex, ENR_LinkAttribute attr,
        int startPeriod, int endPeriod, float** outValueSeries, int* dim)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using seriesStart and seriesLength respectively.
//
//  NOTE:
//     The link index argument corresponds to the EPANET link index from 1 to
//     nlinks. The series returned is indexed from 0 to nperiods - 1.
//
{
    int k, length, errorcode = 0;
    float* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else if (linkIndex < 1 || linkIndex > p_data->linkCount) errorcode = 423;
    else if (startPeriod < 0 || endPeriod >= p_data->nPeriods ||
            endPeriod <= startPeriod) errorcode = 422;
    // Check memory for outValues
    else if MEMCHECK(temp = newFloatArray(length = endPeriod - startPeriod)) errorcode = 411;
    else
    {
        // loop over and build time series
        for (k = 0; k < length; k++)
            temp[k] = getLinkValue(p_handle, startPeriod + k, linkIndex, attr);

        *outValueSeries = temp;
        *dim = length;
    }
    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getNodeAttribute(ENR_Handle p_handle, int periodIndex,
        ENR_NodeAttribute attr, float** outValueArray, int* length)
//
//   Purpose:
//        For all nodes at given time, get a particular attribute
//
//   Returns:
//        Error code
//        OutValueArray of results is indexed from 0 to nodeCount
//
//   Warning:
//        Caller must free memory allocated for outValueArray
//
//   NOTE:
//        The array returned is indexed from 0 to nnodes - 1. So to access
//        node values by their EPANET index, the index value must be
//        decremented by one.
//
{
    F_OFF offset;
    int errorcode = 0;
    float * temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    // if the time index is out of range return an error
    else if (periodIndex < 0 || periodIndex >= p_data->nPeriods) errorcode = 422;
    // Check memory for outValues
    else if MEMCHECK(temp = newFloatArray(p_data->nodeCount)) errorcode = 411;

    else
    {
        // calculate byte offset to start time for series
        offset = p_data->outputStartPos + (periodIndex)*p_data->bytesPerPeriod;
        // add offset for node and attribute
        offset += ((attr - 1)*p_data->nodeCount)*WORDSIZE;

        _fseek(p_data->file, offset, SEEK_SET);
        fread(temp, WORDSIZE, p_data->nodeCount, p_data->file);

        *outValueArray = temp;
        *length = p_data->nodeCount;
    }

    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getLinkAttribute(ENR_Handle p_handle, int periodIndex,
        ENR_LinkAttribute attr, float** outValueArray, int* length)
//
//   Purpose:
//        For all links at given time, get a particular attribute
//
//   Returns:
//        Error code
//        OutValueArray of results is indexed from 0 to linkCount
//
//   Warning:
//        Caller must free memory allocated for outValueArray
//
//   NOTE:
//        The array returned is indexed from 0 to nlinks - 1. So to access
//        link values by their EPANET index, the index value must be
//        decremented by one.
//
{
    F_OFF offset;
    int errorcode = 0;
    float* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    // if the time index is out of range return an error
    else if (periodIndex < 0 || periodIndex >= p_data->nPeriods) errorcode = 422;
    // Check memory for outValues
    else if MEMCHECK(temp = newFloatArray(p_data->linkCount)) errorcode = 411;

    else
    {
        // calculate byte offset to start time for series
        offset = p_data->outputStartPos + (periodIndex)*p_data->bytesPerPeriod
                + (NNODERESULTS*p_data->nodeCount)*WORDSIZE;
        // add offset for link and attribute
        offset += ((attr - 1)*p_data->linkCount)*WORDSIZE;

        _fseek(p_data->file, offset, SEEK_SET);
        fread(temp, WORDSIZE, p_data->linkCount, p_data->file);

        *outValueArray = temp;
        *length = p_data->linkCount;
    }

    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getNodeResult(ENR_Handle p_handle, int periodIndex,
        int nodeIndex, float** outValueArray, int* length)
//
//   Purpose: For a node at given time, get all attributes.
//
//   NOTE:
//
{
    int j, errorcode = 0;
    float* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else if (periodIndex < 0 || periodIndex >= p_data->nPeriods) errorcode = 422;
    else if (nodeIndex < 1 || nodeIndex > p_data->nodeCount) errorcode = 423;
    else if MEMCHECK(temp = newFloatArray(NNODERESULTS)) errorcode = 411;
    else
    {
        for (j = 0; j < NNODERESULTS; j++)
            temp[j] = getNodeValue(p_handle, periodIndex, nodeIndex, j);

        *outValueArray = temp;
        *length = NNODERESULTS;
    }

    return set_error(p_data->error_handle, errorcode);
}

int DLLEXPORT ENR_getLinkResult(ENR_Handle p_handle, int periodIndex,
        int linkIndex, float** outValueArray, int* length)
//
//   Purpose: For a link at given time, get all attributes
//
{
    int j, errorcode = 0;
    float* temp;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else if (periodIndex < 0 || periodIndex >= p_data->nPeriods) errorcode = 422;
    else if (linkIndex < 1 || linkIndex > p_data->linkCount) errorcode = 423;
    else if MEMCHECK(temp = newFloatArray(NLINKRESULTS)) errorcode = 411;
    else
    {
        for (j = 0; j < NLINKRESULTS; j++)
            temp[j] = getLinkValue(p_handle, periodIndex, linkIndex, j);

        *outValueArray = temp;
        *length = NLINKRESULTS;
    }
    return set_error(p_data->error_handle, errorcode);
}

void DLLEXPORT ENR_clearError(ENR_Handle p_handle)
{
    data_t* p_data;

    p_data = (data_t*)p_handle;
    clear_error(p_data->error_handle);
}

int DLLEXPORT ENR_checkError(ENR_Handle p_handle, char** msg_buffer)
{
    int errorcode = 0;
    char *temp = NULL;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    if (p_data == NULL) return -1;
    else
    {
        errorcode = p_data->error_handle->error_status;
        if (errorcode)
            temp = check_error(p_data->error_handle);

        *msg_buffer = temp;
    }

    return errorcode;
}


void errorLookup(int errcode, char* dest_msg, int dest_len)
//
//  Purpose: takes error code returns error message
//
{
    const char* msg;

    switch (errcode)
    {
    case 10:  msg = WARN10;
    break;
    case 411: msg = ERR411;
    break;
    case 412: msg = ERR412;
    break;
    case 421: msg = ERR421;
    break;
    case 422: msg = ERR422;
    break;
    case 423: msg = ERR423;
    break;
    case 434: msg = ERR434;
    break;
    case 435: msg = ERR435;
    break;
    case 436: msg = ERR436;
    break;
    default:  msg = ERRERR;
    }

    strncpy(dest_msg, msg, MAXMSG);
}

int validateFile(ENR_Handle p_handle)
// Returns:
// 	 Error code: 435, 436
//   Warning code: 10
{
    INT4 magic1, magic2, hydcode;
    int errorcode = 0;
    F_OFF filepos;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    // Read magic number from beginning of file
    fseek(p_data->file, 0L, SEEK_SET);
    fread(&magic1, WORDSIZE, 1, p_data->file);

    // Fast forward to end and read file epilogue
    fseek(p_data->file, -3*WORDSIZE, SEEK_END);
    fread(&(p_data->nPeriods), WORDSIZE, 1, p_data->file);
    fread(&hydcode, WORDSIZE, 1, p_data->file);
    fread(&magic2, WORDSIZE, 1, p_data->file);

    filepos = _ftell(p_data->file);

    // Is the file an EPANET binary file?
    if (magic1 != magic2) errorcode = 435;
    // Does the binary file contain results?
    else if (filepos < MINNREC*WORDSIZE || p_data->nPeriods == 0)
        errorcode = 436;
    // Issue warning if there were problems with the model run.
    else if (hydcode != 0) errorcode = 10;

    return errorcode;
}

float getNodeValue(ENR_Handle p_handle, int periodIndex, int nodeIndex,
        int attr)
//
//   Purpose: Retrieves an attribute value at a specified node and time
//
{
    F_OFF offset;
    REAL4 y;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    // calculate byte offset to start time for series
    offset = p_data->outputStartPos + periodIndex*p_data->bytesPerPeriod;
    // add byte position for attribute and node
    offset += ((attr - 1)*p_data->nodeCount + (nodeIndex - 1))*WORDSIZE;

    _fseek(p_data->file, offset, SEEK_SET);
    fread(&y, WORDSIZE, 1, p_data->file);

    return y;
}

float getLinkValue(ENR_Handle p_handle, int periodIndex, int linkIndex,
        int attr)
//
//   Purpose: Retrieves an attribute value at a specified link and time
//
{
    F_OFF offset;
    REAL4 y;
    data_t* p_data;

    p_data = (data_t*)p_handle;

    // Calculate byte offset to start time for series
    offset = p_data->outputStartPos + periodIndex*p_data->bytesPerPeriod
            + (NNODERESULTS*p_data->nodeCount)*WORDSIZE;
    // add byte position for attribute and link
    offset += ((attr - 1)*p_data->linkCount + (linkIndex - 1))*WORDSIZE;

    _fseek(p_data->file, offset, SEEK_SET);
    fread(&y, WORDSIZE, 1, p_data->file);

    return y;
}

int _fopen(FILE **f, const char *name, const char *mode) {
    //
    //  Purpose: Substitute for fopen_s on platforms where it doesn't exist
    //  Note: fopen_s is part of C++11 standard
    //
    int ret = 0;
#ifdef _WIN32
    ret = (int)fopen_s(f, name, mode);
#else
    *f = fopen(name, mode);
    if (!*f)
        ret = -1;
#endif
    return ret;
}

int _fseek(FILE* stream, F_OFF offset, int whence)
//
//  Purpose: Selects platform fseek() for large file support
//
{
#ifdef _WIN32 // Windows (32-bit and 64-bit)
#define FSEEK64 _fseeki64
#else         // Other platforms
#define FSEEK64 fseeko
#endif

    return FSEEK64(stream, offset, whence);
}

F_OFF _ftell(FILE* stream)
//
//  Purpose: Selects platform ftell() for large file support
//
{
#ifdef _WIN32 // Windows (32-bit and 64-bit)
#define FTELL64 _ftelli64
#else         // Other platforms
#define FTELL64 ftello
#endif

    return FTELL64(stream);
}

float* newFloatArray(int n)
//
//  Warning: Caller must free memory allocated by this function.
//
{
    return (float*) malloc((n)*sizeof(float));
}

int* newIntArray(int n)
//
//  Warning: Caller must free memory allocated by this function.
//
{
    return (int*) malloc((n)*sizeof(int));
}

char* newCharArray(int n)
//
//  Warning: Caller must free memory allocated by this function.
//
{
    return (char*) malloc((n)*sizeof(char));
}
