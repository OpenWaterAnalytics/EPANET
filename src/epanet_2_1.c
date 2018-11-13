/*
*******************************************************************************

EPANET.C -- Hydraulic & Water Quality Simulator for Water Distribution Networks

VERSION:    2.1
AUTHORS:    OpenWaterAnalytics members: see git stats for contributors

*******************************************************************************
*/

#include <stdlib.h>

#include "epanet_2_1.h"
#include "types.h"


// This single global variable is used only when the library is called
// in "legacy mode" with the 2.1-style API.
Project *_defaultModel;


int DLLEXPORT ENepanet(const char *f1, const char *f2, const char *f3, void (*pviewprog)(char *))
{
  int errcode = 0;
  int warncode = 0;

  ERRCODE(EN_createproject(&_defaultModel));

  ERRCODE(EN_runproject(_defaultModel, f1, f2, f3, pviewprog));
  if (errcode < 100) warncode = errcode;

  ERRCODE(EN_deleteproject(&_defaultModel));

  if (warncode) errcode = MAX(errcode, warncode);
  
  return (errcode);
}

int DLLEXPORT ENinit(const char *f2, const char *f3, int UnitsType,
                     int HeadlossFormula) {
  int errcode = 0;
  
  ERRCODE(EN_createproject(&_defaultModel));
  ERRCODE(EN_init(_defaultModel, f2, f3, UnitsType, HeadlossFormula));
  
  return (errcode);
}

int DLLEXPORT ENopen(const char *f1, const char *f2, const char *f3) {
  int errcode = 0;
  
  ERRCODE(EN_createproject(&_defaultModel));
  EN_open(_defaultModel, f1, f2, f3);
  
  return (errcode);
}

int DLLEXPORT ENsaveinpfile(const char *filename) {
  return EN_saveinpfile(_defaultModel, filename);
}

int DLLEXPORT ENclose() {
    int errcode = 0;

    ERRCODE(EN_close(_defaultModel));
    ERRCODE(EN_deleteproject(&_defaultModel));
    
	return (errcode);
}

int DLLEXPORT ENsolveH() { return EN_solveH(_defaultModel); }

int DLLEXPORT ENsaveH() { return EN_saveH(_defaultModel); }

int DLLEXPORT ENopenH() { return EN_openH(_defaultModel); }

int DLLEXPORT ENinitH(int flag) { return EN_initH(_defaultModel, flag); }

int DLLEXPORT ENrunH(long *t) { return EN_runH(_defaultModel, t); }

int DLLEXPORT ENnextH(long *tstep) { return EN_nextH(_defaultModel, tstep); }

int DLLEXPORT ENcloseH() { return EN_closeH(_defaultModel); }

int DLLEXPORT ENsavehydfile(char *filename) {
  return EN_savehydfile(_defaultModel, filename);
}

int DLLEXPORT ENusehydfile(char *filename) {
  return EN_usehydfile(_defaultModel, filename);
}

int DLLEXPORT ENsolveQ() { return EN_solveQ(_defaultModel); }

int DLLEXPORT ENopenQ() { return EN_openQ(_defaultModel); }

int DLLEXPORT ENinitQ(int saveflag) {
  return EN_initQ(_defaultModel, saveflag);
}

int DLLEXPORT ENrunQ(long *t) { return EN_runQ(_defaultModel, t); }

int DLLEXPORT ENnextQ(long *tstep) { return EN_nextQ(_defaultModel, tstep); }

int DLLEXPORT ENstepQ(long *tleft) { return EN_stepQ(_defaultModel, tleft); }

int DLLEXPORT ENcloseQ() { return EN_closeQ(_defaultModel); }

int DLLEXPORT ENwriteline(char *line) {
  return EN_writeline(_defaultModel, line);
}

int DLLEXPORT ENreport() { return EN_report(_defaultModel); }

int DLLEXPORT ENresetreport() { return EN_resetreport(_defaultModel); }

int DLLEXPORT ENsetreport(char *s) { return EN_setreport(_defaultModel, s); }

int DLLEXPORT ENgetversion(int *v) { return EN_getversion(v); }

int DLLEXPORT ENgetcontrol(int cindex, int *ctype, int *lindex,
    EN_API_FLOAT_TYPE *setting, int *nindex, EN_API_FLOAT_TYPE *level) {
  return EN_getcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex, level);
}

int DLLEXPORT ENgetcount(int code, int *count) {
  return EN_getcount(_defaultModel, (EN_CountType)code, count);
}

int DLLEXPORT ENgetoption(int code, EN_API_FLOAT_TYPE *value) {
  return EN_getoption(_defaultModel, (EN_Option)code, value);
}

int DLLEXPORT ENgettimeparam(int code, long *value) {
  return EN_gettimeparam(_defaultModel, code, value);
}

int DLLEXPORT ENgetflowunits(int *code) {
  return EN_getflowunits(_defaultModel, code);
}

int DLLEXPORT ENsetflowunits(int code) {
  return EN_setflowunits(_defaultModel, code);
}

int DLLEXPORT ENgetdemandmodel(int *type, EN_API_FLOAT_TYPE *pmin, EN_API_FLOAT_TYPE *preq,
                               EN_API_FLOAT_TYPE *pexp) {
    return EN_getdemandmodel(_defaultModel, type, pmin, preq, pexp);
}

int DLLEXPORT ENsetdemandmodel(int type, EN_API_FLOAT_TYPE pmin, EN_API_FLOAT_TYPE preq,
                               EN_API_FLOAT_TYPE pexp) {
    return EN_setdemandmodel(_defaultModel, type, pmin, preq, pexp);
}

int DLLEXPORT ENgetpatternindex(char *id, int *index) {
  return EN_getpatternindex(_defaultModel, id, index);
}

int DLLEXPORT ENgetpatternid(int index, char *id) {
  return EN_getpatternid(_defaultModel, index, id);
}

int DLLEXPORT ENgetpatternlen(int index, int *len) {
  return EN_getpatternlen(_defaultModel, index, len);
}

int DLLEXPORT ENgetpatternvalue(int index, int period,
                                EN_API_FLOAT_TYPE *value) {
  return EN_getpatternvalue(_defaultModel, index, period, value);
}

int DLLEXPORT ENgetcurveindex(char *id, int *index) {
  return EN_getcurveindex(_defaultModel, id, index);
}

int DLLEXPORT ENgetcurveid(int index, char *id) {
  return EN_getcurveid(_defaultModel, index, id);
}

int DLLEXPORT ENgetcurvelen(int index, int *len) {
  return EN_getcurvelen(_defaultModel, index, len);
}

int DLLEXPORT ENgetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE *x,
                              EN_API_FLOAT_TYPE *y) {
  return EN_getcurvevalue(_defaultModel, index, pnt, x, y);
}

int DLLEXPORT ENgetqualtype(int *qualcode, int *tracenode) {
  return EN_getqualtype(_defaultModel, qualcode, tracenode);
}

int DLLEXPORT ENgetqualinfo(int *qualcode, char *chemname, char *chemunits,
                            int *tracenode) {
  return EN_getqualinfo(_defaultModel, qualcode, chemname, chemunits,
                        tracenode);
}

int DLLEXPORT ENgeterror(int errcode, char *errmsg, int n) {
  return EN_geterror(errcode, errmsg, n);
}

int DLLEXPORT ENgetstatistic(int code, EN_API_FLOAT_TYPE *value) {
  return EN_getstatistic(_defaultModel, code, value);
}

int DLLEXPORT ENgetnodeindex(char *id, int *index) {
  return EN_getnodeindex(_defaultModel, id, index);
}

int DLLEXPORT ENgetnodeid(int index, char *id) {
  return EN_getnodeid(_defaultModel, index, id);
}

int DLLEXPORT ENgetnodetype(int index, int *code) {
  return EN_getnodetype(_defaultModel, index, code);
}

int DLLEXPORT ENgetcoord(int index, EN_API_FLOAT_TYPE *x,
                         EN_API_FLOAT_TYPE *y) {
  return EN_getcoord(_defaultModel, index, x, y);
}

int DLLEXPORT ENsetcoord(int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y) {
  return EN_setcoord(_defaultModel, index, x, y);
}

int DLLEXPORT ENgetnodevalue(int index, int code, EN_API_FLOAT_TYPE *value) {
  return EN_getnodevalue(_defaultModel, index, code, value);
}

int DLLEXPORT ENgetlinkindex(char *id, int *index) {
  return EN_getlinkindex(_defaultModel, id, index);
}

int DLLEXPORT ENgetlinkid(int index, char *id) {
  return EN_getlinkid(_defaultModel, index, id);
}

int DLLEXPORT ENgetlinktype(int index, int *code) {
  return EN_getlinktype(_defaultModel, index, (EN_LinkType *)code);
}

int DLLEXPORT ENgetlinknodes(int index, int *node1, int *node2) {
  return EN_getlinknodes(_defaultModel, index, node1, node2);
}

int DLLEXPORT ENgetlinkvalue(int index, int code, EN_API_FLOAT_TYPE *value) {
  return EN_getlinkvalue(_defaultModel, index, (EN_LinkProperty)code, value);
}

int DLLEXPORT ENgetcurve(int curveIndex, char *id, int *nValues,
                         EN_API_FLOAT_TYPE **xValues,
                         EN_API_FLOAT_TYPE **yValues) {
  return EN_getcurve(_defaultModel, curveIndex, id, nValues, xValues, yValues);
}

int DLLEXPORT ENsetcontrol(int cindex, int ctype, int lindex,
                           EN_API_FLOAT_TYPE setting, int nindex,
                           EN_API_FLOAT_TYPE level) {
  return EN_setcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex,
                       level);
}

int DLLEXPORT ENaddcontrol(int *cindex, int ctype, int lindex,
                           EN_API_FLOAT_TYPE setting, int nindex,
                           EN_API_FLOAT_TYPE level) {
  return EN_addcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex,
                       level);
}

int  DLLEXPORT ENdeletecontrol(int cindex) {
    return EN_deletecontrol(_defaultModel, cindex);
}


int DLLEXPORT ENsetnodeid(int index, char *newid) {
    return EN_setnodeid(_defaultModel, index, newid);
}

int DLLEXPORT ENsetnodevalue(int index, int code, EN_API_FLOAT_TYPE v) {
  return EN_setnodevalue(_defaultModel, index, code, v);
}

int DLLEXPORT ENsetlinkid(int index, char *newid) {
    return EN_setlinkid(_defaultModel, index, newid);
}

int DLLEXPORT ENsetlinknodes(int index, int node1, int node2) {
  return EN_setlinknodes(_defaultModel, index, node1, node2);
}

int DLLEXPORT ENsetlinktype(int *index, int type) {
  return EN_setlinktype(_defaultModel, index, (EN_LinkType)type);
}

int DLLEXPORT ENsetlinkvalue(int index, int code, EN_API_FLOAT_TYPE v) {
  return EN_setlinkvalue(_defaultModel, index, code, v);
}

int DLLEXPORT ENaddpattern(char *id) {
  return EN_addpattern(_defaultModel, id);
}

int DLLEXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *f, int n) {
  return EN_setpattern(_defaultModel, index, f, n);
}

int DLLEXPORT ENsetpatternvalue(int index, int period,
                                EN_API_FLOAT_TYPE value) {
  return EN_setpatternvalue(_defaultModel, index, period, value);
}

int DLLEXPORT ENaddcurve(char *id) { return EN_addcurve(_defaultModel, id); }

int DLLEXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y,
                         int n) {
  return EN_setcurve(_defaultModel, index, x, y, n);
}

int DLLEXPORT ENsetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE x,
                              EN_API_FLOAT_TYPE y) {
  return EN_setcurvevalue(_defaultModel, index, pnt, x, y);
}

int DLLEXPORT ENsettimeparam(int code, long value) {
  return EN_settimeparam(_defaultModel, code, value);
}

int DLLEXPORT ENsetoption(int code, EN_API_FLOAT_TYPE v) {
  return EN_setoption(_defaultModel, code, v);
}

int DLLEXPORT ENsetstatusreport(int code) {
  return EN_setstatusreport(_defaultModel, code);
}

int DLLEXPORT ENsetqualtype(int qualcode, char *chemname, char *chemunits,
                            char *tracenode) {
  return EN_setqualtype(_defaultModel, qualcode, chemname, chemunits,
                        tracenode);
}

int DLLEXPORT ENgetheadcurveindex(int index, int *curveindex) {
  return EN_getheadcurveindex(_defaultModel, index, curveindex);
}

int DLLEXPORT ENsetheadcurveindex(int index, int curveindex) {
  return EN_setheadcurveindex(_defaultModel, index, curveindex);
}

int DLLEXPORT ENgetpumptype(int index, int *type) {
  return EN_getpumptype(_defaultModel, index, type);
}

int DLLEXPORT ENgetcurvetype(int curveindex, int *type) {
  return EN_getcurvetype(_defaultModel, curveindex, type);
}

int DLLEXPORT ENgetnumdemands(int nodeIndex, int *numDemands) {
  return EN_getnumdemands(_defaultModel, nodeIndex, numDemands);
}

int DLLEXPORT ENgetbasedemand(int nodeIndex, int demandIdx,
                              EN_API_FLOAT_TYPE *baseDemand) {
  return EN_getbasedemand(_defaultModel, nodeIndex, demandIdx, baseDemand);
}

int DLLEXPORT ENsetbasedemand(int nodeIndex, int demandIdx,
                              EN_API_FLOAT_TYPE baseDemand) {
  return EN_setbasedemand(_defaultModel, nodeIndex, demandIdx, baseDemand);
}

int  DLLEXPORT ENsetdemandpattern(int nodeIndex, int demandIdx, int patIndex) {
  return EN_setdemandpattern(_defaultModel, nodeIndex, demandIdx, patIndex);
}

int DLLEXPORT ENgetdemandpattern(int nodeIndex, int demandIdx, int *pattIdx) {
  return EN_getdemandpattern(_defaultModel, nodeIndex, demandIdx, pattIdx);
}

int DLLEXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value) {
  return EN_getaveragepatternvalue(_defaultModel, index, value);
}

int DLLEXPORT ENgetdemandname(int nodeIndex, int demandIdx,
                               char *demandName) {
  return EN_getdemandname(_defaultModel, nodeIndex, demandIdx, demandName);
}

int DLLEXPORT ENsetdemandname(int nodeIndex, int demandIdx,
                              char *demandName) {
  return EN_setdemandname(_defaultModel, nodeIndex, demandIdx, demandName);
}

int DLLEXPORT ENgetrule(int index, int *nPremises, int *nTrueActions,
                        int *nFalseActions, EN_API_FLOAT_TYPE *priority) {
  return EN_getrule(_defaultModel, index, nPremises, nTrueActions, nFalseActions, priority);
}

int DLLEXPORT ENsetrulepriority(int index, EN_API_FLOAT_TYPE priority){
  return EN_setrulepriority(_defaultModel, index, priority);
}

int DLLEXPORT ENgetpremise(int indexRule, int indexPremise, int *logop, int *object, int *indexObj, int *variable, int *relop, int *status, EN_API_FLOAT_TYPE *value){
  return EN_getpremise(_defaultModel, indexRule, indexPremise, logop, object, indexObj, variable, relop, status, value);
}

int DLLEXPORT ENsetpremise(int indexRule, int indexPremise, int logop, int object, int indexObj, int variable, int relop, int status, EN_API_FLOAT_TYPE value){
  return EN_setpremise(_defaultModel, indexRule, indexPremise, logop, object, indexObj, variable, relop, status, value);
}

int DLLEXPORT ENsetpremiseindex(int indexRule, int indexPremise, int indexObj){
  return EN_setpremiseindex(_defaultModel, indexRule, indexPremise, indexObj);
}

int DLLEXPORT ENsetpremisestatus(int indexRule, int indexPremise, int status){
  return EN_setpremisestatus(_defaultModel, indexRule, indexPremise, status);
}

int DLLEXPORT ENsetpremisevalue(int indexRule, int indexPremise, EN_API_FLOAT_TYPE value){
  return EN_setpremisevalue(_defaultModel, indexRule, indexPremise, value);
}

int DLLEXPORT ENgettrueaction(int indexRule, int indexAction, int *indexLink,
                              int *status, EN_API_FLOAT_TYPE *setting){
  return EN_gettrueaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int DLLEXPORT ENsettrueaction(int indexRule, int indexAction, int indexLink,
                              int status, EN_API_FLOAT_TYPE setting){
  return EN_settrueaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int DLLEXPORT ENgetfalseaction(int indexRule, int indexAction, int *indexLink,
                               int *status, EN_API_FLOAT_TYPE *setting){
  return EN_getfalseaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int DLLEXPORT ENsetfalseaction(int indexRule, int indexAction, int indexLink,
                               int status, EN_API_FLOAT_TYPE setting){
  return EN_setfalseaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int DLLEXPORT ENgetruleID(int indexRule, char* id){
  return EN_getruleID(_defaultModel, indexRule, id);
}

int DLLEXPORT ENaddnode(char *id, int nodeType) {
  return EN_addnode(_defaultModel, id, (EN_NodeType)nodeType);
}

int DLLEXPORT ENaddlink(char *id, int linkType, char *fromNode, char *toNode) {
  return EN_addlink(_defaultModel, id, (EN_LinkType)linkType, fromNode, toNode);
}

int DLLEXPORT ENdeletelink(int index) {
  return EN_deletelink(_defaultModel, index);
}

int DLLEXPORT ENdeletenode(int index) {
  return EN_deletenode(_defaultModel, index);
}
