/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       epanet2.c
 Description:  implementation of the legacy EPANET API functions
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/29/2018
 ******************************************************************************
*/


#include "types.h"
#include "funcs.h"

#include "epanet2.h"
#include "epanet2_2.h"


// This single global variable is used only when the library is called
// in "legacy mode" with the 2.1-style API.
Project __defaultProject;
Project *_defaultProject = &__defaultProject;

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


/********************************************************************

    System Functions

********************************************************************/

int DLLEXPORT ENepanet(const char *f1, const char *f2, const char *f3,
                       void (*pviewprog)(char *))
{
/*------------------------------------------------------------------------
 **   Input:   f1 = name of EPANET formatted input file
 **            f2 = name of report file
 **            f3 = name of binary output file
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
    errcode = EN_runproject(_defaultProject, f1, f2, f3, pviewprog);
    if (errcode < 100) warncode = errcode;
    removetmpfiles();

    // Return the warning code if the run had no errors
    if (warncode) errcode = MAX(errcode, warncode);
    return errcode;
}

int DLLEXPORT ENinit(const char *f2, const char *f3, int UnitsType,
                     int HeadlossFormula)
{
    int errcode = 0;
    createtmpfiles();
    errcode = EN_init(_defaultProject, f2, f3, UnitsType, HeadlossFormula);
    return errcode;
}

int DLLEXPORT ENopen(const char *f1, const char *f2, const char *f3)
{
    int errcode = 0;
    createtmpfiles();
    errcode = EN_open(_defaultProject, f1, f2, f3);
    return errcode;
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

int DLLEXPORT ENinitH(int flag) { return EN_initH(_defaultProject, flag); }

int DLLEXPORT ENrunH(long *t) { return EN_runH(_defaultProject, t); }

int DLLEXPORT ENnextH(long *tstep) { return EN_nextH(_defaultProject, tstep); }

int DLLEXPORT ENcloseH() { return EN_closeH(_defaultProject); }

int DLLEXPORT ENsavehydfile(char *filename)
{
    return EN_savehydfile(_defaultProject, filename);
}

int DLLEXPORT ENusehydfile(char *filename)
{
    return EN_usehydfile(_defaultProject, filename);
}

/********************************************************************

    Water Quality Analysis Functions

********************************************************************/

int DLLEXPORT ENsolveQ() { return EN_solveQ(_defaultProject); }

int DLLEXPORT ENopenQ() { return EN_openQ(_defaultProject); }

int DLLEXPORT ENinitQ(int saveflag) { return EN_initQ(_defaultProject, saveflag); }

int DLLEXPORT ENrunQ(long *t) { return EN_runQ(_defaultProject, t); }

int DLLEXPORT ENnextQ(long *tstep) { return EN_nextQ(_defaultProject, tstep); }

int DLLEXPORT ENstepQ(long *tleft) { return EN_stepQ(_defaultProject, tleft); }

int DLLEXPORT ENcloseQ() { return EN_closeQ(_defaultProject); }

/********************************************************************

    Reporting Functions

********************************************************************/

int DLLEXPORT ENwriteline(char *line) { return EN_writeline(_defaultProject, line); }

int DLLEXPORT ENreport() { return EN_report(_defaultProject); }

int DLLEXPORT ENresetreport() { return EN_resetreport(_defaultProject); }

int DLLEXPORT ENsetreport(char *s) { return EN_setreport(_defaultProject, s); }

int DLLEXPORT ENsetstatusreport(int code)
{
    return EN_setstatusreport(_defaultProject, code);
}

int DLLEXPORT ENgetversion(int *v) { return EN_getversion(v); }

int DLLEXPORT ENgetcount(int code, int *count)
{
    return EN_getcount(_defaultProject, (EN_CountType)code, count);
}

int DLLEXPORT ENgeterror(int errcode, char *errmsg, int n)
{
    return EN_geterror(errcode, errmsg, n);
}

int DLLEXPORT ENgetstatistic(int code, EN_API_FLOAT_TYPE *value)
{
    return EN_getstatistic(_defaultProject, code, value);
}

/********************************************************************

    Analysis Options Functions

********************************************************************/

int DLLEXPORT ENgetoption(int code, EN_API_FLOAT_TYPE *value)
{
    return EN_getoption(_defaultProject, (EN_Option)code, value);
}

int DLLEXPORT ENsetoption(int code, EN_API_FLOAT_TYPE v)
{
    return EN_setoption(_defaultProject, code, v);
}

int DLLEXPORT ENgetflowunits(int *code) { return EN_getflowunits(_defaultProject, code); }

int DLLEXPORT ENsetflowunits(int code) { return EN_setflowunits(_defaultProject, code); }

int DLLEXPORT ENgettimeparam(int code, long *value)
{
    return EN_gettimeparam(_defaultProject, code, value);
}

int DLLEXPORT ENsettimeparam(int code, long value)
{
    return EN_settimeparam(_defaultProject, code, value);
}

int DLLEXPORT ENgetqualinfo(int *qualcode, char *chemname, char *chemunits,
                            int *tracenode)
{
    return EN_getqualinfo(_defaultProject, qualcode, chemname, chemunits, tracenode);
}

int DLLEXPORT ENgetqualtype(int *qualcode, int *tracenode)
{
    return EN_getqualtype(_defaultProject, qualcode, tracenode);
}

int DLLEXPORT ENsetqualtype(int qualcode, char *chemname, char *chemunits,
                            char *tracenode)
{
    return EN_setqualtype(_defaultProject, qualcode, chemname, chemunits, tracenode);
}

/********************************************************************

    Node Functions

********************************************************************/

int DLLEXPORT ENaddnode(char *id, EN_NodeType nodeType)
{
    return EN_addnode(_defaultProject, id, nodeType);
}

int DLLEXPORT ENdeletenode(int index, int actionCode)
{
    return EN_deletenode(_defaultProject, index, actionCode);
}

int DLLEXPORT ENgetnodeindex(char *id, int *index)
{
    return EN_getnodeindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetnodeid(int index, char *id)
{
    return EN_getnodeid(_defaultProject, index, id);
}

int DLLEXPORT ENsetnodeid(int index, char *newid)
{
    return EN_setnodeid(_defaultProject, index, newid);
}

int DLLEXPORT ENgetnodetype(int index, int *code)
{
    return EN_getnodetype(_defaultProject, index, code);
}

int DLLEXPORT ENgetnodevalue(int index, int code, EN_API_FLOAT_TYPE *value)
{
    return EN_getnodevalue(_defaultProject, index, code, value);
}

int DLLEXPORT ENsetnodevalue(int index, int code, EN_API_FLOAT_TYPE v)
{
    return EN_setnodevalue(_defaultProject, index, code, v);
}

int DLLEXPORT ENgetcoord(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y)
{
    return EN_getcoord(_defaultProject, index, x, y);
}

int DLLEXPORT ENsetcoord(int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y)
{
    return EN_setcoord(_defaultProject, index, x, y);
}

/********************************************************************

    Nodal Demand Functions

********************************************************************/

int DLLEXPORT ENgetdemandmodel(int *type, EN_API_FLOAT_TYPE *pmin,
                               EN_API_FLOAT_TYPE *preq, EN_API_FLOAT_TYPE *pexp)
{
    return EN_getdemandmodel(_defaultProject, type, pmin, preq, pexp);
}

int DLLEXPORT ENsetdemandmodel(int type, EN_API_FLOAT_TYPE pmin,
                               EN_API_FLOAT_TYPE preq, EN_API_FLOAT_TYPE pexp)
{
    return EN_setdemandmodel(_defaultProject, type, pmin, preq, pexp);
}

int DLLEXPORT ENgetnumdemands(int nodeIndex, int *numDemands)
{
    return EN_getnumdemands(_defaultProject, nodeIndex, numDemands);
}

int DLLEXPORT ENgetbasedemand(int nodeIndex, int demandIdx, EN_API_FLOAT_TYPE *baseDemand)
{
    return EN_getbasedemand(_defaultProject, nodeIndex, demandIdx, baseDemand);
}

int DLLEXPORT ENsetbasedemand(int nodeIndex, int demandIdx, EN_API_FLOAT_TYPE baseDemand)
{
    return EN_setbasedemand(_defaultProject, nodeIndex, demandIdx, baseDemand);
}

int  DLLEXPORT ENsetdemandpattern(int nodeIndex, int demandIdx, int patIndex)
{
    return EN_setdemandpattern(_defaultProject, nodeIndex, demandIdx, patIndex);
}

int DLLEXPORT ENgetdemandpattern(int nodeIndex, int demandIdx, int *pattIdx)
{
    return EN_getdemandpattern(_defaultProject, nodeIndex, demandIdx, pattIdx);
}

int DLLEXPORT ENgetdemandname(int nodeIndex, int demandIdx, char *demandName)
{
    return EN_getdemandname(_defaultProject, nodeIndex, demandIdx, demandName);
}

int DLLEXPORT ENsetdemandname(int nodeIndex, int demandIdx, char *demandName)
{
    return EN_setdemandname(_defaultProject, nodeIndex, demandIdx, demandName);
}

/********************************************************************

    Link Functions

********************************************************************/

int DLLEXPORT ENaddlink(char *id, EN_LinkType linkType, char *fromNode, char *toNode)
{
    return EN_addlink(_defaultProject, id, linkType, fromNode, toNode);
}

int DLLEXPORT ENdeletelink(int index, int actionCode)
{
    return EN_deletelink(_defaultProject, index, actionCode);
}

int DLLEXPORT ENgetlinkindex(char *id, int *index)
{
    return EN_getlinkindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetlinkid(int index, char *id)
{
    return EN_getlinkid(_defaultProject, index, id);
}

int DLLEXPORT ENsetlinkid(int index, char *newid)
{
    return EN_setlinkid(_defaultProject, index, newid);
}

int DLLEXPORT ENgetlinktype(int index, EN_LinkType *code)
{
    return EN_getlinktype(_defaultProject, index, code);
}

int DLLEXPORT ENsetlinktype(int *index, EN_LinkType type, int actionCode)
{
    return EN_setlinktype(_defaultProject, index, type, actionCode);
}

int DLLEXPORT ENgetlinknodes(int index, int *node1, int *node2)
{
    return EN_getlinknodes(_defaultProject, index, node1, node2);
}

int DLLEXPORT ENsetlinknodes(int index, int node1, int node2)
{
    return EN_setlinknodes(_defaultProject, index, node1, node2);
}

int DLLEXPORT ENgetlinkvalue(int index, int code, EN_API_FLOAT_TYPE *value)
{
    return EN_getlinkvalue(_defaultProject, index, (EN_LinkProperty)code, value);
}

int DLLEXPORT ENsetlinkvalue(int index, int code, EN_API_FLOAT_TYPE v)
{
    return EN_setlinkvalue(_defaultProject, index, code, v);
}

/********************************************************************

    Pump Functions

********************************************************************/

int DLLEXPORT ENgetpumptype(int index, int *type)
{
    return EN_getpumptype(_defaultProject, index, type);
}

int DLLEXPORT ENgetheadcurveindex(int index, int *curveindex)
{
    return EN_getheadcurveindex(_defaultProject, index, curveindex);
}

int DLLEXPORT ENsetheadcurveindex(int index, int curveindex)
{
    return EN_setheadcurveindex(_defaultProject, index, curveindex);
}

/********************************************************************

    Time Pattern Functions

********************************************************************/

int DLLEXPORT ENaddpattern(char *id)
{
    return EN_addpattern(_defaultProject, id);
}

int DLLEXPORT ENgetpatternindex(char *id, int *index)
{
    return EN_getpatternindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetpatternid(int index, char *id)
{
    return EN_getpatternid(_defaultProject, index, id);
}

int DLLEXPORT ENgetpatternlen(int index, int *len)
{
    return EN_getpatternlen(_defaultProject, index, len);
}

int DLLEXPORT ENgetpatternvalue(int index, int period, EN_API_FLOAT_TYPE *value)
{
    return EN_getpatternvalue(_defaultProject, index, period, value);
}

int DLLEXPORT ENsetpatternvalue(int index, int period, EN_API_FLOAT_TYPE value)
{
    return EN_setpatternvalue(_defaultProject, index, period, value);
}

int DLLEXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value)
{
    return EN_getaveragepatternvalue(_defaultProject, index, value);
}

int DLLEXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *f, int n)
{
    return EN_setpattern(_defaultProject, index, f, n);
}

/********************************************************************

    Data Curve Functions

********************************************************************/

int DLLEXPORT ENaddcurve(char *id)
{
    return EN_addcurve(_defaultProject, id);
}

int DLLEXPORT ENgetcurveindex(char *id, int *index)
{
    return EN_getcurveindex(_defaultProject, id, index);
}

int DLLEXPORT ENgetcurveid(int index, char *id)
{
    return EN_getcurveid(_defaultProject, index, id);
}

int DLLEXPORT ENgetcurvelen(int index, int *len)
{
    return EN_getcurvelen(_defaultProject, index, len);
}

int DLLEXPORT ENgetcurvetype(int curveindex, int *type)
{
    return EN_getcurvetype(_defaultProject, curveindex, type);
}

int DLLEXPORT ENgetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE *x,
                              EN_API_FLOAT_TYPE *y)
{
    return EN_getcurvevalue(_defaultProject, index, pnt, x, y);
}

int DLLEXPORT ENsetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE x,
                              EN_API_FLOAT_TYPE y)
{
    return EN_setcurvevalue(_defaultProject, index, pnt, x, y);
}

int DLLEXPORT ENgetcurve(int curveIndex, char *id, int *nValues,
                         EN_API_FLOAT_TYPE **xValues, EN_API_FLOAT_TYPE **yValues)
{
    return EN_getcurve(_defaultProject, curveIndex, id, nValues, xValues, yValues);
}

int DLLEXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y, int n)
{
    return EN_setcurve(_defaultProject, index, x, y, n);
}

/********************************************************************

    Simple Controls Functions

********************************************************************/

int DLLEXPORT ENaddcontrol(int *cindex, int ctype, int lindex, EN_API_FLOAT_TYPE setting,
                           int nindex, EN_API_FLOAT_TYPE level)
{
    return EN_addcontrol(_defaultProject, cindex, ctype, lindex, setting, nindex, level);
}

int  DLLEXPORT ENdeletecontrol(int cindex)
{
    return EN_deletecontrol(_defaultProject, cindex);
}

int DLLEXPORT ENgetcontrol(int cindex, int *ctype, int *lindex, EN_API_FLOAT_TYPE *setting,
                           int *nindex, EN_API_FLOAT_TYPE *level)
{
    return EN_getcontrol(_defaultProject, cindex, ctype, lindex, setting, nindex, level);
}

int DLLEXPORT ENsetcontrol(int cindex, int ctype, int lindex, EN_API_FLOAT_TYPE setting,
                           int nindex, EN_API_FLOAT_TYPE level)
{
    return EN_setcontrol(_defaultProject, cindex, ctype, lindex, setting, nindex, level);
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

int DLLEXPORT ENgetrule(int index, int *nPremises, int *nThenActions, int *nElseActions,
                        EN_API_FLOAT_TYPE *priority)
{
    return EN_getrule(_defaultProject, index, nPremises, nThenActions, nElseActions, priority);
}

int DLLEXPORT ENgetruleID(int index, char* id)
{
    return EN_getruleID(_defaultProject, index, id);
}

int DLLEXPORT ENgetpremise(int ruleIndex, int premiseIndex, int *logop,
                           int *object, int *objIndex, int *variable,
                           int *relop, int *status, EN_API_FLOAT_TYPE *value)
{
    return EN_getpremise(_defaultProject, ruleIndex, premiseIndex, logop, object,
                         objIndex, variable, relop, status, value);
}

int DLLEXPORT ENsetpremise(int ruleIndex, int premiseIndex, int logop,
                           int object, int objIndex, int variable, int relop,
                           int status, EN_API_FLOAT_TYPE value)
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
    return EN_getthenaction(_defaultProject, ruleIndex, actionIndex, linkIndex,
                            status, setting);
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
    return EN_getelseaction(_defaultProject, ruleIndex, actionIndex, linkIndex,
                            status, setting);
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
