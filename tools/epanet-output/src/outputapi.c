//-----------------------------------------------------------------------------
//
//   outputapi.c -- API for reading results from EPANet binary output file
//
//   Version:    0.20
//   Data        06/17/2016
//               08/05/2014
//               05/21/2014
//
//   Author:     Michael E. Tryby
//               US EPA - NRMRL
//
//   Modified:   Maurizio Cingi
//               University of Modena
//
//   Purpose: Output API provides an interface for retrieving results from an
//   EPANet binary output file.
//
//   Output data in the binary file are aligned on a 4 byte word size.
//   Therefore all values both integers and reals are 32 bits in length.
//
//   The output API indexes reporting periods as well as node and link elements
//   identical to the binary file writer found in EPANET. Times correspond to
//   reporting periods and are indexed from zero to the number of reporting
//   periods. Node and link elements are indexed from one to nodeCount and one
//   to linkCount respectively.
//
//   The Output API functions provide a convenient way to select "slices" of
//   data from the output file. As such they return arrays of data. The caller
//   is responsible for allocating and deallocating memory. The functions
//   ENR_newOutValueSeries() and ENR_newOutValueArray() are provided to size
//   arrays properly. The function ENR_free() is provided to deallocate memory.
//
//-----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "outputapi.h"
#include "messages.h"

// NOTE: These depend on machine data model and may change when porting
#define F_OFF  long long  // Must be a 8 byte / 64 bit integer for large file support
#define INT4    int       // Must be a 4 byte / 32 bit integer type
#define REAL4 float       // Must be a 4 byte / 32 bit real type

#define WORDSIZE       4  // Memory alignment 4 byte word size for both int and real

#define MINNREC       14  // Minimum allowable number of records

#define PROLOGUE     884   // Preliminary fixed length section of header
#define MAXID_P1      32   // Max. # characters in ID name

#define NENERGYRESULTS 6   // Number of energy results
#define NNODERESULTS   4   // number of result fields for nodes
#define NLINKRESULTS   8   // number of result fields for links
#define NREACTRESULTS  4   // number of net reaction results


#define MEMCHECK(x)  (((x) == NULL) ? 411 : 0 )


struct ENResultsAPI {
    char  name[MAXFNAME];     // file path/name
    FILE  *file;                  // FILE structure pointer
    INT4  nodeCount, tankCount, linkCount, pumpCount, valveCount, nPeriods;
    F_OFF outputStartPos;         // starting file position of output data
    F_OFF bytesPerPeriod;         // bytes saved per simulation time period
};

//-----------------------------------------------------------------------------
//   Local functions
//-----------------------------------------------------------------------------
int    validateFile(ENResultsAPI*);
float  getNodeValue(ENResultsAPI*, int, int, int);
float  getLinkValue(ENResultsAPI*, int, int, int);


ENResultsAPI* DLLEXPORT ENR_init(void)
//  Purpose: Returns an initialized pointer for the opaque ENResultsAPI
//    structure.
//
//  FYI: The existence of this function has been carefully considered. I will
//  give you three good reasons not to change it. 1) Abstracting struct
//  initialization in its own function simplifies the API. The user can call
//  all API function with first degree pointer to the struct eliminating the
//  need for dealing with second degree pointers on the open and close calls.
//  2) It simplifies the code when wrapping the API in other languages. And,
//  3) Other libraries use this same pattern (see MPI Library C API).
//
{
	return malloc(sizeof(struct ENResultsAPI));
}


int DLLEXPORT ENR_open(ENResultsAPI* enrapi, const char* path)
/*------------------------------------------------------------------------
**   Input:   path
**   Output:  penrapi = pointer to ENResultsAPI struct
**  Returns:  error code
**  Purpose:  Allocate ENResultsAPI struc, open the output binary file and
**            read epilogue
**  NOTE: ENR_open must be called before any other ENR_* functions
**        no need to allocate ENResultsAPI struc before calling
**-------------------------------------------------------------------------
*/
{
    int err, errorcode = 0;
    F_OFF bytecount;

    if (enrapi == NULL) errorcode = 440;
    else
    {
		strncpy(enrapi->name, path, MAXFNAME+1);

		// Attempt to open binary output file for reading only
		if ((enrapi->file = fopen(path, "rb")) == NULL) errorcode = 434;
		// Perform checks to insure the file is valid
		else if ((err = validateFile(enrapi)) != 0) errorcode = err;

		else {
			// read network size
			fseek(enrapi->file, 2*WORDSIZE, SEEK_SET);
			fread(&(enrapi->nodeCount), WORDSIZE, 1, enrapi->file);
			fread(&(enrapi->tankCount), WORDSIZE, 1, enrapi->file);
			fread(&(enrapi->linkCount), WORDSIZE, 1, enrapi->file);
			fread(&(enrapi->pumpCount), WORDSIZE, 1, enrapi->file);
			fread(&(enrapi->valveCount), WORDSIZE, 1, enrapi->file);

			// Compute positions and offsets for retrieving data
			// fixed portion of header + title section + filenames + chem names
			bytecount = PROLOGUE;
			// node names + link names
			bytecount += MAXID_P1*enrapi->nodeCount + MAXID_P1*enrapi->linkCount;
			// network connectivity + tank nodes + tank areas
			bytecount += 3*WORDSIZE*enrapi->linkCount + 2*WORDSIZE*enrapi->tankCount;
			// node elevations + link lengths and link diameters
			bytecount += WORDSIZE*enrapi->nodeCount + 2*WORDSIZE*enrapi->linkCount;
			// pump energy summary
			bytecount += 7*WORDSIZE*enrapi->pumpCount + WORDSIZE;
			enrapi->outputStartPos= bytecount;

			enrapi->bytesPerPeriod = NNODERESULTS*WORDSIZE*enrapi->nodeCount +
					NLINKRESULTS*WORDSIZE*enrapi->linkCount;
		}
    }

    if (errorcode) ENR_close(enrapi);

    return errorcode;
}

int DLLEXPORT ENR_close(ENResultsAPI* enrapi)
/*------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
**  Returns:  error code
**  Purpose:  Close the output binary file, dellocate ENResultsAPI struc
**            and nullify pointer to ENResultsAPI struct
**  NOTE: ENR_close must be called before program end
**        after calling ENR_close data in  ENResultsAPI struct are no more 
**        accessible
**-------------------------------------------------------------------------
*/
{   int errcode = 0;

	if (enrapi == NULL) errcode = 440;
	else if (enrapi->file == NULL) errcode = 412;
	else
	{
        fclose( enrapi->file);

        free(enrapi);
        enrapi = NULL;
    }
    return errcode;
}

int DLLEXPORT ENR_getVersion(ENResultsAPI* enrapi, int* version)
/*------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
**   Output:  version  Epanet version
**  Returns: error code
**  Purpose: Returns Epanet version that wrote EBOFile
**
**--------------element codes-------------------------------------------
*/
{
	int errcode = 0;

	if (enrapi == NULL) errcode = 412;
	else
	{
		fseek(enrapi->file, 1*WORDSIZE, SEEK_SET);
		if (fread(version, WORDSIZE, 1, enrapi->file) != 1)
			errcode = 436;
	}
	return errcode;
}

int DLLEXPORT ENR_getNetSize(ENResultsAPI* enrapi, ENR_ElementCount code, int* count)
/*------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
**            code=  element code
**   Output:  count
**  Returns: error code                              
**  Purpose: Returns count of elements  of given kind
**-------------------------------------------------------------------------
*/
{
	int errcode = 0;

	*count = -1;

    if (enrapi == NULL) errcode = 412;
    else
    {
    	switch (code)
    	{
    	case ENR_nodeCount:
    		*count = enrapi->nodeCount;
    		break;
    	case ENR_tankCount:
    		*count = enrapi->tankCount;
    		break;
    	case ENR_linkCount:
    		*count = enrapi->linkCount;
    		break;
    	case ENR_pumpCount:
    		*count = enrapi->pumpCount;
    		break;
    	case ENR_valveCount:
    		*count = enrapi->valveCount;
    		break;
    	default:
    		errcode = 421;
    	}
    }
    return errcode;
}

int DLLEXPORT ENR_getUnits(ENResultsAPI* enrapi, ENR_Unit code, int* unitFlag)
/*------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
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
	int errcode = 0;

    *unitFlag = -1;

	if (enrapi == NULL) errcode = 412;
	else
	{
		switch (code)
        {
        case ENR_flowUnits:   
           fseek(enrapi->file, 9*WORDSIZE, SEEK_SET);
           fread(unitFlag, WORDSIZE, 1, enrapi->file);
           break;

        case ENR_pressUnits:  
           fseek(enrapi->file, 10*WORDSIZE, SEEK_SET);
           fread(unitFlag, WORDSIZE, 1, enrapi->file);
           break;

        default: errcode = 421;
        }
	}
	return errcode;
}

int DLLEXPORT ENR_getTimes(ENResultsAPI* enrapi, ENR_Time code, int* time)
/*------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
**            code=  element code
**   Output:  time
**  Returns: error code                              
**  Purpose: Returns report and simulation time related parameters.
**-------------------------------------------------------------------------
*/
{
	int errcode = 0;

	*time = -1;
    if (enrapi == NULL) errcode = 412;
    else
    {
        switch (code)
        {
        case ENR_reportStart:
            fseek(enrapi->file, 12*WORDSIZE, SEEK_SET);
            fread(time, WORDSIZE, 1, enrapi->file);
            break;

        case ENR_reportStep:
            fseek(enrapi->file, 13*WORDSIZE, SEEK_SET);
            fread(time, WORDSIZE, 1, enrapi->file);
            break;

        case ENR_simDuration: 
            fseek(enrapi->file, 14*WORDSIZE, SEEK_SET);
            fread(time, WORDSIZE, 1, enrapi->file);
            break;

        case ENR_numPeriods:  
        	*time = enrapi->nPeriods;
        	break;

        default:
        	errcode = 421;
        }
    }
    return errcode;
}

int DLLEXPORT ENR_getElementName(ENResultsAPI* enrapi, ENR_ElementType type,
		int elementIndex, char* name)
/*------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
**            type = ENR_node or ENR_link
**            elementIndex from 1 to nodeCount or 1 to linkCount
**   Output:  name = elementName
**  Returns: error code
**  Purpose: Retrieves Name of a specified node or link element
**  NOTE: 'name' must be able to hold MAXID characters
**-------------------------------------------------------------------------
*/
{
	F_OFF offset;
    int errcode = 0;

    if (enrapi == NULL) errcode = 412;
    else
    {
    	switch (type)
    	{
    	case ENR_node:
    		if (elementIndex < 1 || elementIndex > enrapi->nodeCount)
    			errcode = 423;
    		else offset = PROLOGUE + (elementIndex - 1)*MAXID_P1;
    		break;

    	case ENR_link:
    		if (elementIndex < 1 || elementIndex > enrapi->linkCount)
    			errcode = 423;
    		else
    			offset = PROLOGUE + enrapi->nodeCount*MAXID_P1 +
    					(elementIndex - 1)*MAXID_P1;
    		break;

    	default:
    		errcode = 421;
    	}

    	if (!errcode)
    	{
    		fseek(enrapi->file, offset, SEEK_SET);
    		fread(name, 1, MAXID_P1, enrapi->file);
    	}
    }
    return errcode;
}

int DLLEXPORT ENR_getEnergyUsage(ENResultsAPI* enrapi, int pumpIndex,
		int* linkIndex, float* outValues)
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
	int errcode = 0;

	if (enrapi == NULL) errcode = 412;
	// Check memory for outValues
	else if (outValues == NULL) errcode = 411;
	// Check for valid pump index
	else if (pumpIndex < 1 || pumpIndex > enrapi->pumpCount) errcode = 423;

	else
	{
	    // Position offset to start of pump energy summary
	    offset = enrapi->outputStartPos - (enrapi->pumpCount*(WORDSIZE + 6*WORDSIZE) + WORDSIZE);
	    // Adjust offset by pump index
	    offset += (pumpIndex - 1)*(WORDSIZE + 6*WORDSIZE);

	    // Power summary is 1 int and 6 floats for each pump
	    fseek(enrapi->file, offset, SEEK_SET);
	    fread(linkIndex, WORDSIZE, 1, enrapi->file);
	    fread(outValues+1, WORDSIZE, 6, enrapi->file);
	}
	return errcode;
}

int DLLEXPORT ENR_getNetReacts(ENResultsAPI* enrapi, float* outValues)
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
	int errcode = 0;

    if (enrapi == NULL) errcode = 412;
    else if (outValues == NULL) errcode = 411;

    else
    {
        // Reaction summary is 4 floats located right before epilogue.
    	// This offset is relative to the end of the file.
    	offset = - 3*WORDSIZE - 4*WORDSIZE;
        fseek(enrapi->file, offset, SEEK_END);
        fread(outValues+1, WORDSIZE, 4, enrapi->file);
    }
    return errcode;
}

float* DLLEXPORT ENR_newOutValueSeries(ENResultsAPI* enrapi, int startPeriod,
        int endPeriod, int* length, int* errcode)
//
//  Purpose: Allocates memory for outValue Series.
//
//  Warning: Caller must free memory allocated by this function using ENR_free().
//
{
    int size;
    float* array;

    if (enrapi!=NULL)
    {
    	if (startPeriod < 0 || endPeriod >= enrapi->nPeriods ||
    			endPeriod <= startPeriod)
    	{
    		*errcode = 422; return NULL;
    	}

        size = endPeriod - startPeriod;
        if (size > enrapi->nPeriods) size = enrapi->nPeriods - 1;

        // Allocate memory for outValues
        array = (float*) calloc(size + 1, sizeof(float));
        *errcode = (MEMCHECK(array));

        *length = size + 1;
        return array;
    }
    *errcode = 412;
    return NULL;
}

float* DLLEXPORT ENR_newOutValueArray(ENResultsAPI* enrapi, ENR_ApiFunction func,
        ENR_ElementType type, int* length, int* errcode)
//
//  Purpose: Allocates memory for outValue Array.
//
//  Warning: Caller must free memory allocated by this function using ENR_free().
//
{
    int size;
    float* array;

    if (enrapi!=NULL) {
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

        case ENR_getReacts:
        	size = NREACTRESULTS;
        	break;

        case ENR_getEnergy:
        	size = NENERGYRESULTS;
        	break;

        default:
        	*errcode = 421;
        	return NULL;
        }

        // Allocate memory for outValues
        array = (float*) calloc(size + 1, sizeof(float));
        *errcode = (MEMCHECK(array));

        *length = size + 1;
        return array;
    }
    *errcode = 412;
    return NULL;
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

int DLLEXPORT ENR_getNodeSeries(ENResultsAPI* enrapi, int nodeIndex, ENR_NodeAttribute attr,
        int startPeriod, int length, float* outValueSeries)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using seriesStart and seriesLength respectively.
//
{
    int k, errcode = 0;

    if (enrapi == NULL) errcode = 412;
    // Check memory and node index
    else if (outValueSeries == NULL) errcode = 411;
    else if (nodeIndex < 1 || nodeIndex > enrapi->nodeCount) errcode = 423;
    else if (startPeriod < 0 || startPeriod >= enrapi->nPeriods ||
        		length > enrapi->nPeriods) errcode = 422;
    else
    {
        // loop over and build time series
        for (k = 0; k < length; k++)
            outValueSeries[k] = getNodeValue(enrapi, startPeriod + k,
            		nodeIndex, attr);
    }
    return errcode;
}

int DLLEXPORT ENR_getLinkSeries(ENResultsAPI* enrapi, int linkIndex,
		ENR_LinkAttribute attr, int startPeriod, int length,
		float* outValueSeries)
//
//  Purpose: Get time series results for particular attribute. Specify series
//  start and length using seriesStart and seriesLength respectively.
//
{
    int k, errcode = 0;

    if (enrapi == NULL) errcode = 412;
    // Check memory for outValues
    else if (outValueSeries == NULL) errcode = 411;
    else if (linkIndex < 1 || linkIndex > enrapi->linkCount) errcode = 423;
    else if (startPeriod < 0 || startPeriod >= enrapi->nPeriods ||
        		length > enrapi->nPeriods) errcode = 422;
    else
    {
        // loop over and build time series
        for (k = 0; k < length; k++)
            outValueSeries[k] = getLinkValue(enrapi, startPeriod + k,
            		linkIndex, attr);
    }
    return errcode;
}

int DLLEXPORT ENR_getNodeAttribute(ENResultsAPI* enrapi, int periodIndex,
        ENR_NodeAttribute attr, float* outValueArray, int* length)
//
//   Purpose: For all nodes at given time, get a particular attribute
//
{
    int offset, errcode = 0;

    if (enrapi == NULL) errcode = 412;
    // Check memory for outValues
    else if (outValueArray == NULL) errcode = 411;
    // if the time index is out of range return an error
    else if (periodIndex < 0 || periodIndex >= enrapi->nPeriods) errcode = 422;

    else
    {
        // calculate byte offset to start time for series
        offset = enrapi->outputStartPos + (periodIndex)*enrapi->bytesPerPeriod;
        // add offset for node and attribute
        offset += ((attr - 1)*enrapi->nodeCount)*WORDSIZE;

        fseek(enrapi->file, offset, SEEK_SET);
        fread(outValueArray+1, WORDSIZE, enrapi->nodeCount, enrapi->file);
        *length = enrapi->nodeCount;
    }
    return errcode;
}

int DLLEXPORT ENR_getLinkAttribute(ENResultsAPI* enrapi, int periodIndex,
        ENR_LinkAttribute attr, float* outValueArray, int* length)
//
//   Purpose: For all links at given time, get a particular attribute
//
{
	F_OFF offset;
    int errcode = 0;

    if (enrapi == NULL) errcode = 412;
    // Check memory for outValues
    else if (outValueArray == NULL) errcode = 411;
    // if the time index is out of range return an error
    else if (periodIndex < 0 || periodIndex >= enrapi->nPeriods) errcode = 422;

    else
    {
        // calculate byte offset to start time for series
        offset = enrapi->outputStartPos + (periodIndex)*enrapi->bytesPerPeriod
                + (NNODERESULTS*enrapi->nodeCount)*WORDSIZE;
        // add offset for link and attribute
        offset += ((attr - 1)*enrapi->linkCount)*WORDSIZE;

        fseek(enrapi->file, offset, SEEK_SET);
        fread(outValueArray+1, WORDSIZE, enrapi->linkCount, enrapi->file);
        *length = enrapi->linkCount;
    }
    return errcode;
}

int DLLEXPORT ENR_getNodeResult(ENResultsAPI* enrapi, int periodIndex, int nodeIndex,
        float* outValueArray)
//
//   Purpose: For a node at given time, get all attributes
//
{
    int j, errcode = 0;

    if (enrapi == NULL) errcode = 412;
    else if (outValueArray == NULL) errcode = 411;
    else if (periodIndex < 0 || periodIndex >= enrapi->nPeriods) errcode = 422;
    else if (nodeIndex < 1 || nodeIndex > enrapi->nodeCount) errcode = 423;
    else
    {
        for (j = 1; j <= NNODERESULTS; j++)
            outValueArray[j] = getNodeValue(enrapi, periodIndex, nodeIndex, j);
    }
    return errcode;
}

int DLLEXPORT ENR_getLinkResult(ENResultsAPI* enrapi, int periodIndex, int linkIndex,
        float* outValueArray)
//
//   Purpose: For a link at given time, get all attributes
//
{
    int j, errcode = 0;

    if (enrapi == NULL) errcode = 412;
    else if (outValueArray == NULL) errcode = 411;
    else if (periodIndex < 0 || periodIndex >= enrapi->nPeriods) errcode = 422;
    else if (linkIndex < 1 || linkIndex > enrapi->linkCount) errcode = 423;
    else
    {
        for (j = 1; j <= NLINKRESULTS; j++)
            outValueArray[j] = getLinkValue(enrapi, periodIndex, linkIndex, j);
    }
    return errcode;
}

int DLLEXPORT ENR_errMessage(int errcode, char* errmsg, int n)
//
//  Purpose: takes error code returns error message
//
{
    switch (errcode)
    {
    case 411: strncpy(errmsg, ERR411, n);
    	break;
    case 412: strncpy(errmsg, ERR412, n);
    	break;
    case 421: strncpy(errmsg, ERR421, n);
    	break;
    case 422: strncpy(errmsg, ERR422, n);
    	break;
    case 423: strncpy(errmsg, ERR423, n);
    	break;
    case 434: strncpy(errmsg, ERR434, n);
    	break;
    case 435: strncpy(errmsg, ERR435, n);
    	break;
    case 436: strncpy(errmsg, ERR436, n);
    	break;
    case 437: strncpy(errmsg, ERR437, n);
    	break;
    default: return -1;
    }
    return 0;
}


int validateFile(ENResultsAPI* enrapi)
//
{
	INT4 magic1, magic2, hydcode;
	int errorcode = 0;
    F_OFF filepos;

	// Read magic number from beginning of file
	fseek(enrapi->file, 0L, SEEK_SET);
	fread(&magic1, WORDSIZE, 1, enrapi->file);

	// Fast forward to end and read file epilogue
	fseek(enrapi->file, -3*WORDSIZE, SEEK_END);
	fread(&(enrapi->nPeriods), WORDSIZE, 1, enrapi->file);
	fread(&hydcode, WORDSIZE, 1, enrapi->file);
	fread(&magic2, WORDSIZE, 1, enrapi->file);

	filepos = ftell(enrapi->file);

	// Is the file an EPANET binary file?
	if (magic1 != magic2) errorcode = 435;
	// Does the binary file contain results?
	else if (filepos < MINNREC*WORDSIZE || enrapi->nPeriods == 0)
		errorcode = 436;
	// Were there any problems with the model run?
	else if (hydcode != 0) errorcode = 437;

	return errorcode;
}

float getNodeValue(ENResultsAPI* enrapi, int periodIndex, int nodeIndex,
		int attr)
//
//   Purpose: Retrieves an attribute value at a specified node and time
//
{
	F_OFF offset;
    REAL4 y;

    // calculate byte offset to start time for series
    offset = enrapi->outputStartPos + periodIndex*enrapi->bytesPerPeriod;
    // add byte position for attribute and node
    offset += ((attr - 1)*enrapi->nodeCount + (nodeIndex - 1))*WORDSIZE;

    fseek(enrapi->file, offset, SEEK_SET);
    fread(&y, WORDSIZE, 1, enrapi->file);

    return y;
}

float getLinkValue(ENResultsAPI* enrapi, int periodIndex, int linkIndex,
        int attr)
//
//   Purpose: Retrieves an attribute value at a specified link and time
//
{
	F_OFF offset;
    REAL4 y;

    // Calculate byte offset to start time for series
    offset = enrapi->outputStartPos + periodIndex*enrapi->bytesPerPeriod
            + (NNODERESULTS*enrapi->nodeCount)*WORDSIZE;
    // add byte position for attribute and link
    offset += ((attr - 1)*enrapi->linkCount + (linkIndex - 1))*WORDSIZE;

    fseek(enrapi->file, offset, SEEK_SET);
    fread(&y, WORDSIZE, 1, enrapi->file);

    return y;
}


/*
int DLLEXPORT ENR_getNodeValue(ENResultsAPI* enrapi, int periodIndex, int nodeIndex,
		ENR_NodeAttribute attr, float *value)
------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
**            nodeIndex from 1 to nodeCount
**            timeIndex from 0 to nPeriods
**            attr = attribute code
**   Output:  value=
**  Returns: error code
**  Purpose: Returns attribute for a node at given time
**-------------------------------------------------------------------------

{   int errcode = 0;

    if (enrapi==NULL) errcode =  412;
    else if ( periodIndex < 0 || periodIndex >=enrapi->nPeriods ) errcode = 422;
    else if ( nodeIndex < 1 || nodeIndex > enrapi->nodeCount ) errcode = 423;
    else
    	*value = getNodeValue(enrapi, periodIndex, nodeIndex, attr);

    return errcode;
}

int DLLEXPORT ENR_getLinkValue(ENResultsAPI* enrapi, int periodIndex, int linkIndex,
		ENR_LinkAttribute attr, float *value)
------------------------------------------------------------------------
**   Input:   enrapi = pointer to ENResultsAPI struct
**            linkIndex from 1 to linkCount
**            timeIndex from 0 to nPeriods
**
**            attr = attribute code
**   Output:  value=
**  Returns: error code
**  Purpose: Returns attribute for a link at given time
**-------------------------------------------------------------------------

{
    int errcode = 0;

    if (enrapi==NULL) errcode = 412;
    else if ( periodIndex < 0 || periodIndex >=enrapi->nPeriods ) errcode = 422;
    else if ( linkIndex < 1 || linkIndex > enrapi->linkCount ) errcode = 423;
    else
    	*value = getLinkValue(enrapi, periodIndex, linkIndex, attr);

    return errcode;
}
*/
