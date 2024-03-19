/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2.c
 Description:  implementation of the legacy EPANET API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 09/28/2023
 ******************************************************************************
*/

#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "funcs.h"

#include "epanet2.h"
#include "epanet2_2.h"


// This single global variable is used only when the library is called
// in "legacy mode" with the 2.1-style API.
Project __defaultProject;
Project *_defaultProject = &__defaultProject;

typedef struct Project* EN_Project;


// Functions for creating and removing default temporary files
void createtmpfiles()
{
    getTmpName(_defaultProject->TmpHydFname);
    getTmpName(_defaultProject->TmpOutFname);
    getTmpName(_defaultProject->TmpStatFname);
}

void removetmpfiles()
{
    remove(_defaultProject->TmpHydFname);
    remove(_defaultProject->TmpOutFname);
    remove(_defaultProject->TmpStatFname);
}

/*|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
||      FUNCIONES PARA DESARROLLO, INTERFAZ Y TESTS           ||
|||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
void appendToFile(const char* content) {
    // Open the file in append mode ("a+").
    FILE* file = fopen("MENSAJES.txt", "a+");

    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    // Write content to the end of the file.
    fprintf(file, "%s\n", content);

    // Close the file.
    fclose(file);
}

__declspec(dllexport) void* get_defaultProject() {
    /*------------------------------------------------------------------------
         **   Input:   none
         **   Output:  pointer _defaultproject
         **
         * *  This function returns the _defaultproject pointer so that i
         **   can use it from poutside C (from Julia being more precise).
         **-------------------------------------------------------------------------
         */
    const char *mensaje = "Hiii hayy hellooo :)";
    appendToFile(mensaje);

    return (void*)_defaultProject;
}

__declspec(dllexport) void consultar_campos(EN_Project p) {
    // Esta funci�n solo la quiero para consultar campos del projecto
    int nnodes = p->network.Nnodes;
    printf("Numero de nodos: %d \n", nnodes);
}

__declspec(dllexport) int numero_nodos(EN_Project p) {
    // Esta funci�n solo la quiero para consultar campos del projecto
    int nnodes = p->network.Nnodes;
    return nnodes;
}

__declspec(dllexport) int numero_sensores(EN_Project p) {
    // Esta funci�n solo la quiero para consultar campos del projecto
    int ns = p->network.Nflowmeters;
    return ns;
}
__declspec(dllexport) void print_sensordata_infile(EN_Project p) {
    // Function for checking if sensor data input has been successfull.
    // Prints sensor information in the MENSAJES.txt file
    Network* net = &p->network;
    Ssensor* sens;
    int n = net->Nsensors;
    int sIndex,
        elIndex;
    char tipo[20],
         sID[MAXID+1],
         elID[MAXID + 1];
    double error,
           defDemerr,
           defTankerr,
           defReserr;
    char buffer[100];

    sprintf(buffer,"Sensor\tSensor ID\tEl. Index\tEl. ID\tType\tError");
    const char* linea;
    linea = buffer;
    appendToFile(linea);

    for (int i = 1; i <= n; i++)
    {
        sens = &net->Sensor[i];
        strcpy(sID, sens->ID);
        strcpy(elID, sens->elID);
        sIndex = sens->sIndex;
        elIndex = sens->elIndex;
        error = sens->error;

        switch (sens->Type)
        {
        case PRESSUREMETER:     strcpy(tipo, "PMETER");    break;
        case FLOWMETER:         strcpy(tipo, "FMETER");    break;
        case WATERLEVELMETER:   strcpy(tipo, "WMETER");    break;
        default:
            strcpy(tipo, "error");
            break;
        }
        sprintf(buffer, "%d\t%s\t\t%d\t\t%s\t%s\t%f", sIndex, sID, elIndex, elID, tipo, error);
        linea = buffer;
        appendToFile(linea);

    }
    sprintf(buffer, "\nDefault pseudomeasurement uncertainties");
    linea = buffer;
    appendToFile(linea);

    defDemerr = net->defDemError;
    defTankerr = net->defTankLevError;
    defReserr = net->defResLevError;

    sprintf(buffer, "\nDemand:\t\t\tCV = %f LPS", defDemerr);
    linea = buffer;
    appendToFile(linea);
    sprintf(buffer, "\nTank water level:\tCV = %f m", defTankerr);
    linea = buffer;
    appendToFile(linea);
    sprintf(buffer, "\nTank water level:\tCV = %f m", defReserr);
    linea = buffer;
    appendToFile(linea);

    
}


/********************************************************************

    Project Functions

********************************************************************/

int DLLEXPORT 
ENepanet(const char *inpFile, const char *rptFile,
              const char *outFile,  void (*pviewprog)(char *))
{
/*------------------------------------------------------------------------
 **   Input:   inpFile = name of EPANET formatted input file
 **            rptFile = name of report file
 **            outFile = name of binary output file
 **            pviewprog = see note below
 **   Output:  none
 **   Returns: error code
 **   Purpose: runs a complete EPANET simulation
 **
 **  The pviewprog() argument is a pointer to a callback function
 **  that takes a character string (char *) as its only parameter.
 **  The function would reside in and be used by the calling
 **  program to display the progress messages that EPANET generates
 **  as it carries out its computations. If this feature is not
 **  needed then the argument should be NULL.
 **-------------------------------------------------------------------------
 */
    int errcode = 0;
    int warncode = 0;

    // Run the project and record any warning
    createtmpfiles();
    errcode = EN_runproject(_defaultProject, inpFile, rptFile, outFile, pviewprog);

    if (errcode < 100) warncode = errcode;
    removetmpfiles();

    // Return the warning code if the run had no errors
    if (warncode) errcode = MAX(errcode, warncode);
    return errcode;
}

int DLLEXPORT ENinit(const char *rptFile, const char *outFile, int unitsType,
                     int headlossType)
{
    int errcode = 0;
    createtmpfiles();
    errcode = EN_init(_defaultProject, rptFile, outFile, unitsType, headlossType);
    return errcode;
}

int DLLEXPORT ENopen(const char *inpFile, const char *rptFile, const char *outFile)
{
    int errcode = 0;
    createtmpfiles();
    errcode = EN_open(_defaultProject, inpFile, rptFile, outFile);
    return errcode;
}

int DLLEXPORT ENopenX(const char *inpFile, const char *rptFile, const char *outFile)
{
    int errcode = 0;
    createtmpfiles();
    errcode = EN_openX(_defaultProject, inpFile, rptFile, outFile);
    return errcode;
}

int DLLEXPORT ENgettitle(char *line1, char *line2, char *line3)
{
    return EN_gettitle(_defaultProject, line1, line2, line3) ;
}

int DLLEXPORT ENsettitle(const char *line1, const char *line2, const char *line3)
{
    return EN_settitle(_defaultProject, line1, line2, line3) ;
}

int DLLEXPORT ENgetcomment(int object, int index, char *comment)
{
    return EN_getcomment(_defaultProject, object, index, comment);
}

int  DLLEXPORT ENsetcomment(int object, int index, const char *comment)
{
    return EN_setcomment(_defaultProject, object, index, comment);
}

int DLLEXPORT ENgetcount(int object, int *count)
{
    return EN_getcount(_defaultProject, object, count);
}

int DLLEXPORT ENsaveinpfile(const char *filename)
{
    return EN_saveinpfile(_defaultProject, filename);
}

int DLLEXPORT ENclose()
{
    EN_close(_defaultProject);
    removetmpfiles();
    return 0;
}

/********************************************************************

    Hydraulic Analysis Functions

********************************************************************/

int DLLEXPORT ENsolveH() { return EN_solveH(_defaultProject); }

int DLLEXPORT ENsaveH() { return EN_saveH(_defaultProject); }

int DLLEXPORT ENopenH() { return EN_openH(_defaultProject); }

int DLLEXPORT ENinitH(int initFlag) { return EN_initH(_defaultProject, initFlag); }

int DLLEXPORT ENrunH(long *currentTime) { return EN_runH(_defaultProject, currentTime); }

int DLLEXPORT ENnextH(long *tStep) { return EN_nextH(_defaultProject, tStep); }

int DLLEXPORT ENcloseH() { return EN_closeH(_defaultProject); }

int DLLEXPORT ENsavehydfile(const char *filename)
{
    return EN_savehydfile(_defaultProject, filename);
}

int DLLEXPORT ENusehydfile(const char *filename)
{
    return EN_usehydfile(_defaultProject, filename);
}

/********************************************************************

    Water Quality Analysis Functions

********************************************************************/

int DLLEXPORT ENsolveQ() { return EN_solveQ(_defaultProject); }

int DLLEXPORT ENopenQ() { return EN_openQ(_defaultProject); }

int DLLEXPORT ENinitQ(int saveFlag) { return EN_initQ(_defaultProject, saveFlag); }

int DLLEXPORT ENrunQ(long *currentTime) { return EN_runQ(_defaultProject, currentTime); }

int DLLEXPORT ENnextQ(long *tStep) { return EN_nextQ(_defaultProject, tStep); }

int DLLEXPORT ENstepQ(long *timeLeft) { return EN_stepQ(_defaultProject, timeLeft); }

int DLLEXPORT ENcloseQ() { return EN_closeQ(_defaultProject); }

/********************************************************************

    Reporting Functions

********************************************************************/

int DLLEXPORT ENwriteline(const char *line) { return EN_writeline(_defaultProject, line); }

int DLLEXPORT ENreport() { return EN_report(_defaultProject); }

int DLLEXPORT ENcopyreport(const char *filename)
{
    return EN_copyreport(_defaultProject, filename);
}

int DLLEXPORT ENclearreport() { return EN_clearreport(_defaultProject); }

int DLLEXPORT ENresetreport() { return EN_resetreport(_defaultProject); }

int DLLEXPORT ENsetreport(const char *format) { return EN_setreport(_defaultProject, format); }

int DLLEXPORT ENsetstatusreport(int level)
{
    return EN_setstatusreport(_defaultProject, level);
}

int DLLEXPORT ENsetreportcallback(void (*callback)(void *userData, void *EN_projectHandle, const char*))
{
  return EN_setreportcallback(_defaultProject, callback);
}

int DLLEXPORT ENsetreportcallbackuserdata(void *userData)
{
  return EN_setreportcallbackuserdata(_defaultProject, userData);
}

int DLLEXPORT ENgetversion(int *version) { return EN_getversion(version); }

int DLLEXPORT ENgeterror(int errcode, char *errmsg, int maxLen)
{
    return EN_geterror(errcode, errmsg, maxLen);
}

int DLLEXPORT ENgetstatistic(int type, EN_API_FLOAT_TYPE *value)
{
    double v = 0.0;
    int errcode = EN_getstatistic(_defaultProject, type, &v);
    *value = (EN_API_FLOAT_TYPE)v;
    return errcode;
}

int DLLEXPORT ENgetresultindex(int type, int index, int *value)
{
    return EN_getresultindex(_defaultProject, type, index, value);
}

int  DLLEXPORT ENtimetonextevent(int *eventType, long *duration, int *elementIndex)
{
    return EN_timetonextevent(_defaultProject, eventType, duration, elementIndex);
}

/********************************************************************

    Analysis Options Functions

********************************************************************/

int DLLEXPORT ENgetoption(int option, EN_API_FLOAT_TYPE *value)
{
    double v = 0.0;
    int errcode = EN_getoption(_defaultProject, option, &v);
    *value = (EN_API_FLOAT_TYPE)v;
    return errcode;
}

int DLLEXPORT ENsetoption(int option, EN_API_FLOAT_TYPE value)
{
    return EN_setoption(_defaultProject, option, value);
}

int DLLEXPORT ENgetflowunits(int *units)
{
    return EN_getflowunits(_defaultProject, units);
}

int DLLEXPORT ENsetflowunits(int units)
{
    return EN_setflowunits(_defaultProject, units);
}

int DLLEXPORT ENgettimeparam(int param, long *value)
{
    return EN_gettimeparam(_defaultProject, param, value);
}

int DLLEXPORT ENsettimeparam(int param, long value)
{
    return EN_settimeparam(_defaultProject, param, value);
}

int DLLEXPORT ENgetqualinfo(int *qualType, char *chemName, char *chemUnits,
              int *traceNode)
{
    return EN_getqualinfo(_defaultProject, qualType, chemName, chemUnits, traceNode);
}

int DLLEXPORT ENgetqualtype(int *qualType, int *traceNode)
{
    return EN_getqualtype(_defaultProject, qualType, traceNode);
}

int DLLEXPORT ENsetqualtype(int qualType, const char *chemName,
              const char *chemUnits, const char *traceNode)
{
    return EN_setqualtype(_defaultProject, qualType, chemName, chemUnits, traceNode);
}

/********************************************************************

    Node Functions

********************************************************************/

int DLLEXPORT ENaddnode(const char *id, int nodeType, int *index)
{
    return EN_addnode(_defaultProject, id, nodeType, index);
}

int DLLEXPORT ENdeletenode(int index, int actionCode)
{
    return EN_deletenode(_defaultProject, index, actionCode);
}

int DLLEXPORT ENgetnodeindex(const char *id, int *index)
{
    return EN_getnodeindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetnodeid(int index, char *id)
{
    return EN_getnodeid(_defaultProject, index, id);
}

int DLLEXPORT ENsetnodeid(int index, const char *newid)
{
    return EN_setnodeid(_defaultProject, index, newid);
}

int DLLEXPORT ENgetnodetype(int index, int *nodeType)
{
    return EN_getnodetype(_defaultProject, index, nodeType);
}

int DLLEXPORT ENgetnodevalue(int index, int property, EN_API_FLOAT_TYPE *value)
{
    double v = 0.0;
    int errcode = EN_getnodevalue(_defaultProject, index, property, &v);
    *value = (EN_API_FLOAT_TYPE)v;
    return errcode;
}

int DLLEXPORT ENsetnodevalue(int index, int property, EN_API_FLOAT_TYPE value)
{
    return EN_setnodevalue(_defaultProject, index, property, value);
}

int DLLEXPORT ENsetjuncdata(int index, EN_API_FLOAT_TYPE elev, EN_API_FLOAT_TYPE dmnd,
              const char *dmndpat)
{
    return EN_setjuncdata(_defaultProject, index, elev, dmnd, dmndpat);
}

int  DLLEXPORT ENsettankdata(int index, EN_API_FLOAT_TYPE elev,
               EN_API_FLOAT_TYPE initlvl, EN_API_FLOAT_TYPE minlvl,
               EN_API_FLOAT_TYPE maxlvl, EN_API_FLOAT_TYPE diam,
               EN_API_FLOAT_TYPE minvol, const char *volcurve)
{
    return EN_settankdata(_defaultProject, index, elev, initlvl, minlvl, maxlvl,
                          diam, minvol, volcurve);
}

int DLLEXPORT ENgetcoord(int index, double *x, double *y)
{
    return EN_getcoord(_defaultProject, index, x, y);
}

int DLLEXPORT ENsetcoord(int index, double x, double y)
{
    return EN_setcoord(_defaultProject, index, x, y);
}

/********************************************************************

    Nodal Demand Functions

********************************************************************/

int DLLEXPORT ENgetdemandmodel(int *model, EN_API_FLOAT_TYPE *pmin,
              EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp)
{
    double pmin2 = 0.0, preq2 = 0.0, pexp2 = 0.0;
    int errcode = EN_getdemandmodel(_defaultProject, model, &pmin2, &preq2, &pexp2);
    *pmin = (EN_API_FLOAT_TYPE)pmin2;
    *preq = (EN_API_FLOAT_TYPE)preq2;
    *pexp = (EN_API_FLOAT_TYPE)pexp2;
    return errcode;
}

int DLLEXPORT ENsetdemandmodel(int model, EN_API_FLOAT_TYPE pmin,
              EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp)
{
    return EN_setdemandmodel(_defaultProject, model, pmin, preq, pexp);
}

int DLLEXPORT ENadddemand(int nodeIndex, EN_API_FLOAT_TYPE baseDemand,
              const char *demandPattern, const char *demandName)
{
    return EN_adddemand(_defaultProject, nodeIndex, baseDemand, demandPattern, demandName);
}

int DLLEXPORT ENdeletedemand(int nodeIndex, int demandIndex)
{
    return EN_deletedemand(_defaultProject, nodeIndex, demandIndex);
}

int DLLEXPORT ENgetdemandindex(int nodeIndex, const char *demandName, int *demandIndex)
{
    return EN_getdemandindex(_defaultProject, nodeIndex, demandName, demandIndex);
}

int DLLEXPORT ENgetnumdemands(int nodeIndex, int *numDemands)
{
    return EN_getnumdemands(_defaultProject, nodeIndex, numDemands);
}

int DLLEXPORT ENgetbasedemand(int nodeIndex, int demandIndex,
              EN_API_FLOAT_TYPE *baseDemand)
{
    double bd2 = 0.0;
    int errcode = EN_getbasedemand(_defaultProject, nodeIndex, demandIndex, &bd2);
    *baseDemand = (EN_API_FLOAT_TYPE)bd2;
    return errcode;
}

int DLLEXPORT ENsetbasedemand(int nodeIndex, int demandIndex,
              EN_API_FLOAT_TYPE baseDemand)
{
    return EN_setbasedemand(_defaultProject, nodeIndex, demandIndex, baseDemand);
}

int  DLLEXPORT ENsetdemandpattern(int nodeIndex, int demandIndex, int patIndex)
{
    return EN_setdemandpattern(_defaultProject, nodeIndex, demandIndex, patIndex);
}

int DLLEXPORT ENgetdemandpattern(int nodeIndex, int demandIndex, int *pattIdx)
{
    return EN_getdemandpattern(_defaultProject, nodeIndex, demandIndex, pattIdx);
}

int DLLEXPORT ENgetdemandname(int nodeIndex, int demandIndex, char *demandName)
{
    return EN_getdemandname(_defaultProject, nodeIndex, demandIndex, demandName);
}

int DLLEXPORT ENsetdemandname(int nodeIndex, int demandIndex, const char *demandName)
{
    return EN_setdemandname(_defaultProject, nodeIndex, demandIndex, demandName);
}

/********************************************************************

    Link Functions

********************************************************************/

int DLLEXPORT ENaddlink(const char *id, int linkType, const char *fromNode,
              const char *toNode, int *index)
{
    return EN_addlink(_defaultProject, id, linkType, fromNode, toNode, index);
}

int DLLEXPORT ENdeletelink(int index, int actionCode)
{
    return EN_deletelink(_defaultProject, index, actionCode);
}

int DLLEXPORT ENgetlinkindex(const char *id, int *index)
{
    return EN_getlinkindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetlinkid(int index, char *id)
{
    return EN_getlinkid(_defaultProject, index, id);
}

int DLLEXPORT ENsetlinkid(int index, const char *newid)
{
    return EN_setlinkid(_defaultProject, index, newid);
}

int DLLEXPORT ENgetlinktype(int index, int *linkType)
{
    return EN_getlinktype(_defaultProject, index, linkType);
}

int DLLEXPORT ENsetlinktype(int *index, int linkType, int actionCode)
{
    return EN_setlinktype(_defaultProject, index, linkType, actionCode);
}

int DLLEXPORT ENgetlinknodes(int index, int *node1, int *node2)
{
    return EN_getlinknodes(_defaultProject, index, node1, node2);
}

int DLLEXPORT ENsetlinknodes(int index, int node1, int node2)
{
    return EN_setlinknodes(_defaultProject, index, node1, node2);
}

int DLLEXPORT ENgetlinkvalue(int index, int property, EN_API_FLOAT_TYPE *value)
{
    double v = 0.0;
    int errcode = EN_getlinkvalue(_defaultProject, index, property, &v);
    *value = (EN_API_FLOAT_TYPE)v;
    return errcode;
}

int DLLEXPORT ENsetlinkvalue(int index, int property, EN_API_FLOAT_TYPE value)
{
    return EN_setlinkvalue(_defaultProject, index, property, value);
}

int DLLEXPORT ENsetpipedata(int index, EN_API_FLOAT_TYPE length,
              EN_API_FLOAT_TYPE diam, EN_API_FLOAT_TYPE rough, EN_API_FLOAT_TYPE mloss)
{
    return EN_setpipedata(_defaultProject, index, length, diam, rough, mloss);
}

int DLLEXPORT ENgetvertexcount(int index, int *count)
{
    return EN_getvertexcount(_defaultProject, index, count);
}

int DLLEXPORT ENgetvertex(int index, int vertex, double *x, double *y)
{
    return EN_getvertex(_defaultProject, index, vertex, x, y);
}

int DLLEXPORT ENsetvertex(int index, int vertex, double x, double y)
{
    return EN_setvertex(_defaultProject, index, vertex, x, y);
}

int DLLEXPORT ENsetvertices(int index, double *x, double *y, int count)
{
    return EN_setvertices(_defaultProject, index, x, y, count);
}

/********************************************************************

    Pump Functions

********************************************************************/

int DLLEXPORT ENgetpumptype(int linkIndex, int *pumpType)
{
    return EN_getpumptype(_defaultProject, linkIndex, pumpType);
}

int DLLEXPORT ENgetheadcurveindex(int linkIndex, int *curveIndex)
{
    return EN_getheadcurveindex(_defaultProject, linkIndex, curveIndex);
}

int DLLEXPORT ENsetheadcurveindex(int linkIndex, int curveIndex)
{
    return EN_setheadcurveindex(_defaultProject, linkIndex, curveIndex);
}

/********************************************************************

    Time Pattern Functions

********************************************************************/

int DLLEXPORT ENaddpattern(const char *id)
{
    return EN_addpattern(_defaultProject, id);
}

int DLLEXPORT ENdeletepattern(int index)
{
    return EN_deletepattern(_defaultProject, index);
}

int DLLEXPORT ENgetpatternindex(const char *id, int *index)
{
    return EN_getpatternindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetpatternid(int index, char *id)
{
    return EN_getpatternid(_defaultProject, index, id);
}

int DLLEXPORT ENsetpatternid(int index, const char *id)
{
    return EN_setpatternid(_defaultProject, index, id);
}

int DLLEXPORT ENgetpatternlen(int index, int *len)
{
    return EN_getpatternlen(_defaultProject, index, len);
}

int DLLEXPORT ENgetpatternvalue(int index, int period, EN_API_FLOAT_TYPE *value)
{
    double v = 0.0;
    int errcode = EN_getpatternvalue(_defaultProject, index, period, &v);
    *value = (EN_API_FLOAT_TYPE)v;
    return errcode;
}

int DLLEXPORT ENsetpatternvalue(int index, int period, EN_API_FLOAT_TYPE value)
{
    return EN_setpatternvalue(_defaultProject, index, period, value);
}

int DLLEXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value)
{
    double v;
    int errcode = EN_getaveragepatternvalue(_defaultProject, index, &v);
    *value = (EN_API_FLOAT_TYPE)v;
    return errcode;
}

int DLLEXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *values, int len)
{
    double *v = NULL;
    int i, errcode;
    if (values == NULL) return 206;
    v = (double *)calloc(len, sizeof(double));
    if (v)
    {
        for (i = 0; i < len; i++) v[i] = values[i];
        errcode = EN_setpattern(_defaultProject, index, v, len);
    }
    else errcode = 101;
    free(v);
    return errcode;
}

/********************************************************************

    Data Curve Functions

********************************************************************/

int DLLEXPORT ENaddcurve(const char *id)
{
    return EN_addcurve(_defaultProject, id);
}

int DLLEXPORT ENdeletecurve(int index)
{
    return EN_deletecurve(_defaultProject, index);
}

int DLLEXPORT ENgetcurveindex(const char *id, int *index)
{
    return EN_getcurveindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetcurveid(int index, char *id)
{
    return EN_getcurveid(_defaultProject, index, id);
}

int DLLEXPORT ENsetcurveid(int index, const char *id)
{
    return EN_setcurveid(_defaultProject, index, id);
}

int DLLEXPORT ENgetcurvelen(int index, int *len)
{
    return EN_getcurvelen(_defaultProject, index, len);
}

int DLLEXPORT ENgetcurvetype(int index, int *type)
{
    return EN_getcurvetype(_defaultProject, index, type);
}

int DLLEXPORT ENsetcurvetype(int index, int type)
{
    return EN_setcurvetype(_defaultProject, index, type);
}

int DLLEXPORT ENgetcurvevalue(int curveIndex, int pointIndex, EN_API_FLOAT_TYPE *x,
              EN_API_FLOAT_TYPE *y)
{
    double xx = 0.0, yy = 0.0;
    int errcode = EN_getcurvevalue(_defaultProject, curveIndex, pointIndex, &xx, &yy);
    *x = (EN_API_FLOAT_TYPE)xx;
    *y = (EN_API_FLOAT_TYPE)yy;
    return errcode;
}

int DLLEXPORT ENsetcurvevalue(int curveIndex, int pointIndex, EN_API_FLOAT_TYPE x,
              EN_API_FLOAT_TYPE y)
{
    return EN_setcurvevalue(_defaultProject, curveIndex, pointIndex, x, y);
}

int DLLEXPORT ENgetcurve(int index, char *id, int *nPoints,
              EN_API_FLOAT_TYPE *xValues, EN_API_FLOAT_TYPE *yValues)
{
    int i;
    Network *net = &_defaultProject->network;
    Scurve *curve;

    if (index <= 0 || index > net->Ncurves) return 206;
    if (xValues == NULL || yValues == NULL) return 206;
    curve = &net->Curve[index];
    strncpy(id, curve->ID, MAXID);
    *nPoints = curve->Npts;
    for (i = 0; i < curve->Npts; i++)
    {
        xValues[i] = (EN_API_FLOAT_TYPE)curve->X[i];
        yValues[i] = (EN_API_FLOAT_TYPE)curve->Y[i];
    }
    return 0;
}

int DLLEXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *xValues,
              EN_API_FLOAT_TYPE *yValues, int nPoints)
{
    double *xx = NULL;
    double *yy = NULL;
    int i, errcode = 0;

    if (xValues == NULL || yValues == NULL) return 206;
    if (nPoints < 1) return 202;

    xx = (double *)calloc(nPoints, sizeof(double));
    yy = (double *)calloc(nPoints, sizeof(double));
    if (xx && yy)
    {
        for (i = 0; i < nPoints; i++)
        {
            xx[i] = xValues[i];
            yy[i] = yValues[i];
        }
        errcode = EN_setcurve(_defaultProject, index, xx, yy, nPoints);
    }
    else errcode = 101;
    free(xx);
    free(yy);
    return errcode;
}

/********************************************************************

    Simple Controls Functions

********************************************************************/

int DLLEXPORT ENaddcontrol(int type, int linkIndex, EN_API_FLOAT_TYPE setting,
              int nodeIndex, EN_API_FLOAT_TYPE level, int *index)
{
    return EN_addcontrol(_defaultProject, type, linkIndex, setting, nodeIndex,
                         level, index);
}

int  DLLEXPORT ENdeletecontrol(int index)
{
    return EN_deletecontrol(_defaultProject, index);
}

int DLLEXPORT ENgetcontrol(int index, int *type, int *linkIndex,
              EN_API_FLOAT_TYPE *setting, int *nodeIndex, EN_API_FLOAT_TYPE *level)
{
    double setting2 = 0.0, level2 = 0.0;
    int errcode = EN_getcontrol(_defaultProject, index, type, linkIndex, &setting2,
                                nodeIndex, &level2);
    *setting = (EN_API_FLOAT_TYPE)setting2;
    *level = (EN_API_FLOAT_TYPE)level2;
    return errcode;
}

int DLLEXPORT ENsetcontrol(int index, int type, int linkIndex,
              EN_API_FLOAT_TYPE setting, int nodeIndex, EN_API_FLOAT_TYPE level)
{
    return EN_setcontrol(_defaultProject, index, type, linkIndex, setting,
                         nodeIndex, level);
}


int DLLEXPORT ENgetcontrolenabled(int index, int *out_enabled)
{
    return EN_getcontrolenabled(_defaultProject, index, out_enabled);
}

int DLLEXPORT ENsetcontrolenabled(int index, int enabled)
{
    return EN_setcontrolenabled(_defaultProject, index, enabled);
}

/********************************************************************

    Rule-Based Controls Functions

********************************************************************/

int DLLEXPORT ENaddrule(char *rule)
{
    return EN_addrule(_defaultProject, rule);
}

int DLLEXPORT ENdeleterule(int index)
{
    return EN_deleterule(_defaultProject, index);
}

int DLLEXPORT ENgetrule(int index, int *nPremises, int *nThenActions,
              int *nElseActions, EN_API_FLOAT_TYPE *priority)
{
    double priority2 = 0.0;
    int errcode = EN_getrule(_defaultProject, index, nPremises, nThenActions,
                             nElseActions, &priority2);
    *priority = (EN_API_FLOAT_TYPE)priority2;
    return errcode;
}

int DLLEXPORT ENgetruleID(int index, char* id)
{
    return EN_getruleID(_defaultProject, index, id);
}

int DLLEXPORT ENgetpremise(int ruleIndex, int premiseIndex, int *logop, int *object,
              int *objIndex, int *variable, int *relop, int *status,
              EN_API_FLOAT_TYPE *value)
{
    double v = 0.0;
    int errcode = EN_getpremise(_defaultProject, ruleIndex, premiseIndex, logop,
                                object, objIndex, variable, relop, status, &v);
    *value = (EN_API_FLOAT_TYPE)v;
    return errcode;
}

int DLLEXPORT ENsetpremise(int ruleIndex, int premiseIndex, int logop, int object,
              int objIndex, int variable, int relop, int status, EN_API_FLOAT_TYPE value)
{
    return EN_setpremise(_defaultProject, ruleIndex, premiseIndex, logop, object,
                         objIndex, variable, relop, status, value);
}

int DLLEXPORT ENsetpremiseindex(int ruleIndex, int premiseIndex, int objIndex)
{
    return EN_setpremiseindex(_defaultProject, ruleIndex, premiseIndex, objIndex);
}

int DLLEXPORT ENsetpremisestatus(int ruleIndex, int premiseIndex, int status)
{
    return EN_setpremisestatus(_defaultProject, ruleIndex, premiseIndex, status);
}

int DLLEXPORT ENsetpremisevalue(int ruleIndex, int premiseIndex, EN_API_FLOAT_TYPE value)
{
    return EN_setpremisevalue(_defaultProject, ruleIndex, premiseIndex, value);
}

int DLLEXPORT ENgetthenaction(int ruleIndex, int actionIndex, int *linkIndex,
              int *status, EN_API_FLOAT_TYPE *setting)
{
    double setting2 = 0.0;
    int errcode = EN_getthenaction(_defaultProject, ruleIndex, actionIndex, linkIndex,
                                   status, &setting2);
    *setting = (EN_API_FLOAT_TYPE)setting2;
    return errcode;
}

int DLLEXPORT ENsetthenaction(int ruleIndex, int actionIndex, int linkIndex,
              int status, EN_API_FLOAT_TYPE setting)
{
    return EN_setthenaction(_defaultProject, ruleIndex, actionIndex, linkIndex,
                            status, setting);
}

int DLLEXPORT ENgetelseaction(int ruleIndex, int actionIndex, int *linkIndex,
              int *status, EN_API_FLOAT_TYPE *setting)
{
    double setting2 = 0.0;
    int errcode = EN_getelseaction(_defaultProject, ruleIndex, actionIndex, linkIndex,
                                   status, &setting2);
    *setting = (EN_API_FLOAT_TYPE)setting2;
    return errcode;
}

int DLLEXPORT ENsetelseaction(int ruleIndex, int actionIndex, int linkIndex,
              int status, EN_API_FLOAT_TYPE setting)
{
    return EN_setelseaction(_defaultProject, ruleIndex, actionIndex, linkIndex,
                            status, setting);
}

int DLLEXPORT ENsetrulepriority(int index, EN_API_FLOAT_TYPE priority)
{
    return EN_setrulepriority(_defaultProject, index, priority);
}


int DLLEXPORT ENgetruleenabled(int index, int *out_enabled)
{
    return EN_getruleenabled(_defaultProject, index, out_enabled);
}

int DLLEXPORT ENsetruleenabled(int index, int enabled)
{
    return EN_setruleenabled(_defaultProject, index, enabled);
}
