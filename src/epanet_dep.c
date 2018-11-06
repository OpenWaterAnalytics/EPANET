/*
*******************************************************************************

EPANET.C -- Hydraulic & Water Quality Simulator for Water Distribution Networks

VERSION:    2.1
AUTHORS:     L. Rossman - US EPA - NRMRL
              OpenWaterAnalytics members: see git stats for contributors

*******************************************************************************
*/

#include <stdlib.h>

#include "epanet2.h"
#include "epanet2_dep.h"
#include "types.h"


// This single global variable is used only when the library is called
// in "legacy mode" with the 2.1-style API.
void *_defaultModel;


int EPANET_DEPRECATED_EXPORT ENepanet(const char *f1, const char *f2, const char *f3, void (*pviewprog)(char *))
{
  int errcode = 0;
  int warncode = 0;
  EN_Project *p = NULL;

  ERRCODE(EN_createproject(&_defaultModel));

  ERRCODE(EN_runproject(_defaultModel, f1, f2, f3, pviewprog));
  if (errcode < 100) warncode = errcode;

  ERRCODE(EN_deleteproject(&_defaultModel));

  if (warncode) errcode = MAX(errcode, warncode);
  return (errcode);
}

int EPANET_DEPRECATED_EXPORT ENinit(const char *f2, const char *f3, int UnitsType,
                     int HeadlossFormula) {
  int errcode = 0;
  ERRCODE(EN_createproject(&_defaultModel));
  ERRCODE(EN_init(_defaultModel, f2, f3, UnitsType, HeadlossFormula));
  return (errcode);
}

int EPANET_DEPRECATED_EXPORT ENopen(const char *f1, const char *f2, const char *f3) {
  int errcode = 0;
  ERRCODE(EN_createproject(&_defaultModel));
  EN_open(_defaultModel, f1, f2, f3);
  return (errcode);
}

int EPANET_DEPRECATED_EXPORT ENsaveinpfile(const char *filename) {
  return EN_saveinpfile(_defaultModel, filename);
}

int EPANET_DEPRECATED_EXPORT ENclose() { return EN_close(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENsolveH() { return EN_solveH(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENsaveH() { return EN_saveH(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENopenH() { return EN_openH(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENinitH(int flag) { return EN_initH(_defaultModel, flag); }

int EPANET_DEPRECATED_EXPORT ENrunH(long *t) { return EN_runH(_defaultModel, t); }

int EPANET_DEPRECATED_EXPORT ENnextH(long *tstep) { return EN_nextH(_defaultModel, tstep); }

int EPANET_DEPRECATED_EXPORT ENcloseH() { return EN_closeH(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENsavehydfile(char *filename) {
  return EN_savehydfile(_defaultModel, filename);
}

int EPANET_DEPRECATED_EXPORT ENusehydfile(char *filename) {
  return EN_usehydfile(_defaultModel, filename);
}

int EPANET_DEPRECATED_EXPORT ENsolveQ() { return EN_solveQ(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENopenQ() { return EN_openQ(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENinitQ(int saveflag) {
  return EN_initQ(_defaultModel, saveflag);
}

int EPANET_DEPRECATED_EXPORT ENrunQ(long *t) { return EN_runQ(_defaultModel, t); }

int EPANET_DEPRECATED_EXPORT ENnextQ(long *tstep) { return EN_nextQ(_defaultModel, tstep); }

int EPANET_DEPRECATED_EXPORT ENstepQ(long *tleft) { return EN_stepQ(_defaultModel, tleft); }

int EPANET_DEPRECATED_EXPORT ENcloseQ() { return EN_closeQ(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENwriteline(char *line) {
  return EN_writeline(_defaultModel, line);
}

int EPANET_DEPRECATED_EXPORT ENreport() { return EN_report(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENresetreport() { return EN_resetreport(_defaultModel); }

int EPANET_DEPRECATED_EXPORT ENsetreport(char *s) { return EN_setreport(_defaultModel, s); }

int EPANET_DEPRECATED_EXPORT ENgetversion(int *v) { return EN_getversion(v); }

int EPANET_DEPRECATED_EXPORT ENgetcontrol(int cindex, int *ctype, int *lindex,
                           EN_API_FLOAT_TYPE *setting, int *nindex,
                           EN_API_FLOAT_TYPE *level) {
  return EN_getcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex,
                       level);
}

int EPANET_DEPRECATED_EXPORT ENgetcount(int code, int *count) {
  return EN_getcount(_defaultModel, (EN_CountType)code, count);
}

int EPANET_DEPRECATED_EXPORT ENgetoption(int code, EN_API_FLOAT_TYPE *value) {
  return EN_getoption(_defaultModel, (EN_Option)code, value);
}

int EPANET_DEPRECATED_EXPORT ENgettimeparam(int code, long *value) {
  return EN_gettimeparam(_defaultModel, code, value);
}

int EPANET_DEPRECATED_EXPORT ENgetflowunits(int *code) {
  return EN_getflowunits(_defaultModel, code);
}

int EPANET_DEPRECATED_EXPORT ENsetflowunits(int code) {
  return EN_setflowunits(_defaultModel, code);
}

int EPANET_DEPRECATED_EXPORT ENgetdemandmodel(int *type, EN_API_FLOAT_TYPE *pmin, EN_API_FLOAT_TYPE *preq,
                               EN_API_FLOAT_TYPE *pexp) {
    return EN_getdemandmodel(_defaultModel, type, pmin, preq, pexp);
}

int EPANET_DEPRECATED_EXPORT ENsetdemandmodel(int type, EN_API_FLOAT_TYPE pmin, EN_API_FLOAT_TYPE preq,
                               EN_API_FLOAT_TYPE pexp) {
    return EN_setdemandmodel(_defaultModel, type, pmin, preq, pexp);
}

int EPANET_DEPRECATED_EXPORT ENgetpatternindex(char *id, int *index) {
  return EN_getpatternindex(_defaultModel, id, index);
}

int EPANET_DEPRECATED_EXPORT ENgetpatternid(int index, char *id) {
  return EN_getpatternid(_defaultModel, index, id);
}

int EPANET_DEPRECATED_EXPORT ENgetpatternlen(int index, int *len) {
  return EN_getpatternlen(_defaultModel, index, len);
}

int EPANET_DEPRECATED_EXPORT ENgetpatternvalue(int index, int period,
                                EN_API_FLOAT_TYPE *value) {
  return EN_getpatternvalue(_defaultModel, index, period, value);
}

int EPANET_DEPRECATED_EXPORT ENgetcurveindex(char *id, int *index) {
  return EN_getcurveindex(_defaultModel, id, index);
}

int EPANET_DEPRECATED_EXPORT ENgetcurveid(int index, char *id) {
  return EN_getcurveid(_defaultModel, index, id);
}

int EPANET_DEPRECATED_EXPORT ENgetcurvelen(int index, int *len) {
  return EN_getcurvelen(_defaultModel, index, len);
}

int EPANET_DEPRECATED_EXPORT ENgetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE *x,
                              EN_API_FLOAT_TYPE *y) {
  return EN_getcurvevalue(_defaultModel, index, pnt, x, y);
}

int EPANET_DEPRECATED_EXPORT ENgetqualtype(int *qualcode, int *tracenode) {
  return EN_getqualtype(_defaultModel, qualcode, tracenode);
}

int EPANET_DEPRECATED_EXPORT ENgetqualinfo(int *qualcode, char *chemname, char *chemunits,
                            int *tracenode) {
  return EN_getqualinfo(_defaultModel, qualcode, chemname, chemunits,
                        tracenode);
}

int EPANET_DEPRECATED_EXPORT ENgeterror(int errcode, char *errmsg, int n) {
  return EN_geterror(errcode, errmsg, n);
}

int EPANET_DEPRECATED_EXPORT ENgetstatistic(int code, EN_API_FLOAT_TYPE *value) {
  return EN_getstatistic(_defaultModel, code, value);
}

int EPANET_DEPRECATED_EXPORT ENgetnodeindex(char *id, int *index) {
  return EN_getnodeindex(_defaultModel, id, index);
}

int EPANET_DEPRECATED_EXPORT ENgetnodeid(int index, char *id) {
  return EN_getnodeid(_defaultModel, index, id);
}

int EPANET_DEPRECATED_EXPORT ENgetnodetype(int index, int *code) {
  return EN_getnodetype(_defaultModel, index, code);
}

int EPANET_DEPRECATED_EXPORT ENgetcoord(int index, EN_API_FLOAT_TYPE *x,
                         EN_API_FLOAT_TYPE *y) {
  return EN_getcoord(_defaultModel, index, x, y);
}

int EPANET_DEPRECATED_EXPORT ENsetcoord(int index, EN_API_FLOAT_TYPE x, EN_API_FLOAT_TYPE y) {
  return EN_setcoord(_defaultModel, index, x, y);
}

int EPANET_DEPRECATED_EXPORT ENgetnodevalue(int index, int code, EN_API_FLOAT_TYPE *value) {
  return EN_getnodevalue(_defaultModel, index, code, value);
}

int EPANET_DEPRECATED_EXPORT ENgetlinkindex(char *id, int *index) {
  return EN_getlinkindex(_defaultModel, id, index);
}

int EPANET_DEPRECATED_EXPORT ENgetlinkid(int index, char *id) {
  return EN_getlinkid(_defaultModel, index, id);
}

int EPANET_DEPRECATED_EXPORT ENgetlinktype(int index, EN_LinkType *code) {
  return EN_getlinktype(_defaultModel, index, code);
}

int EPANET_DEPRECATED_EXPORT ENgetlinknodes(int index, int *node1, int *node2) {
  return EN_getlinknodes(_defaultModel, index, node1, node2);
}

int EPANET_DEPRECATED_EXPORT ENgetlinkvalue(int index, int code, EN_API_FLOAT_TYPE *value) {
  return EN_getlinkvalue(_defaultModel, index, (EN_LinkProperty)code, value);
}

int EPANET_DEPRECATED_EXPORT ENgetcurve(int curveIndex, char *id, int *nValues,
                         EN_API_FLOAT_TYPE **xValues,
                         EN_API_FLOAT_TYPE **yValues) {
  return EN_getcurve(_defaultModel, curveIndex, id, nValues, xValues, yValues);
}

int EPANET_DEPRECATED_EXPORT ENsetcontrol(int cindex, int ctype, int lindex,
                           EN_API_FLOAT_TYPE setting, int nindex,
                           EN_API_FLOAT_TYPE level) {
  return EN_setcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex,
                       level);
}

int EPANET_DEPRECATED_EXPORT ENaddcontrol(int *cindex, int ctype, int lindex,
                           EN_API_FLOAT_TYPE setting, int nindex,
                           EN_API_FLOAT_TYPE level) {
  return EN_addcontrol(_defaultModel, cindex, ctype, lindex, setting, nindex,
                       level);
}

int  EPANET_DEPRECATED_EXPORT ENdeletecontrol(int cindex) {
    return EN_deletecontrol(_defaultModel, cindex);
}


int EPANET_DEPRECATED_EXPORT ENsetnodeid(int index, char *newid) {
    return EN_setnodeid(_defaultModel, index, newid);
}

int EPANET_DEPRECATED_EXPORT ENsetnodevalue(int index, int code, EN_API_FLOAT_TYPE v) {
  return EN_setnodevalue(_defaultModel, index, code, v);
}

int EPANET_DEPRECATED_EXPORT ENsetlinkid(int index, char *newid) {
    return EN_setlinkid(_defaultModel, index, newid);
}

int EPANET_DEPRECATED_EXPORT ENsetlinknodes(int index, int node1, int node2) {
  return EN_setlinknodes(_defaultModel, index, node1, node2);
}

int EPANET_DEPRECATED_EXPORT ENsetlinktype(int *index, EN_LinkType type) {
  return EN_setlinktype(_defaultModel, index, type);
}

int EPANET_DEPRECATED_EXPORT ENsetlinkvalue(int index, int code, EN_API_FLOAT_TYPE v) {
  return EN_setlinkvalue(_defaultModel, index, code, v);
}

int EPANET_DEPRECATED_EXPORT ENaddpattern(char *id) {
  return EN_addpattern(_defaultModel, id);
}

int EPANET_DEPRECATED_EXPORT ENsetpattern(int index, EN_API_FLOAT_TYPE *f, int n) {
  return EN_setpattern(_defaultModel, index, f, n);
}

int EPANET_DEPRECATED_EXPORT ENsetpatternvalue(int index, int period,
                                EN_API_FLOAT_TYPE value) {
  return EN_setpatternvalue(_defaultModel, index, period, value);
}

int EPANET_DEPRECATED_EXPORT ENaddcurve(char *id) { return EN_addcurve(_defaultModel, id); }

int EPANET_DEPRECATED_EXPORT ENsetcurve(int index, EN_API_FLOAT_TYPE *x, EN_API_FLOAT_TYPE *y,
                         int n) {
  return EN_setcurve(_defaultModel, index, x, y, n);
}

int EPANET_DEPRECATED_EXPORT ENsetcurvevalue(int index, int pnt, EN_API_FLOAT_TYPE x,
                              EN_API_FLOAT_TYPE y) {
  return EN_setcurvevalue(_defaultModel, index, pnt, x, y);
}

int EPANET_DEPRECATED_EXPORT ENsettimeparam(int code, long value) {
  return EN_settimeparam(_defaultModel, code, value);
}

int EPANET_DEPRECATED_EXPORT ENsetoption(int code, EN_API_FLOAT_TYPE v) {
  return EN_setoption(_defaultModel, code, v);
}

int EPANET_DEPRECATED_EXPORT ENsetstatusreport(int code) {
  return EN_setstatusreport(_defaultModel, code);
}

int EPANET_DEPRECATED_EXPORT ENsetqualtype(int qualcode, char *chemname, char *chemunits,
                            char *tracenode) {
  return EN_setqualtype(_defaultModel, qualcode, chemname, chemunits,
                        tracenode);
}

int EPANET_DEPRECATED_EXPORT ENgetheadcurveindex(int index, int *curveindex) {
  return EN_getheadcurveindex(_defaultModel, index, curveindex);
}

int EPANET_DEPRECATED_EXPORT ENsetheadcurveindex(int index, int curveindex) {
  return EN_setheadcurveindex(_defaultModel, index, curveindex);
}

int EPANET_DEPRECATED_EXPORT ENgetpumptype(int index, int *type) {
  return EN_getpumptype(_defaultModel, index, type);
}

int EPANET_DEPRECATED_EXPORT ENgetcurvetype(int curveindex, int *type) {
  return EN_getcurvetype(_defaultModel, curveindex, type);
}

int EPANET_DEPRECATED_EXPORT ENgetnumdemands(int nodeIndex, int *numDemands) {
  return EN_getnumdemands(_defaultModel, nodeIndex, numDemands);
}

int EPANET_DEPRECATED_EXPORT ENgetbasedemand(int nodeIndex, int demandIdx,
                              EN_API_FLOAT_TYPE *baseDemand) {
  return EN_getbasedemand(_defaultModel, nodeIndex, demandIdx, baseDemand);
}

int EPANET_DEPRECATED_EXPORT ENsetbasedemand(int nodeIndex, int demandIdx,
                              EN_API_FLOAT_TYPE baseDemand) {
  return EN_setbasedemand(_defaultModel, nodeIndex, demandIdx, baseDemand);
}

int  EPANET_DEPRECATED_EXPORT ENsetdemandpattern(int nodeIndex, int demandIdx, int patIndex) {
  return EN_setdemandpattern(_defaultModel, nodeIndex, demandIdx, patIndex);
}

int EPANET_DEPRECATED_EXPORT ENgetdemandpattern(int nodeIndex, int demandIdx, int *pattIdx) {
  return EN_getdemandpattern(_defaultModel, nodeIndex, demandIdx, pattIdx);
}

int EPANET_DEPRECATED_EXPORT ENgetaveragepatternvalue(int index, EN_API_FLOAT_TYPE *value) {
  return EN_getaveragepatternvalue(_defaultModel, index, value);
}

int EPANET_DEPRECATED_EXPORT ENgetdemandname(int nodeIndex, int demandIdx,
                               char *demandName) {
  return EN_getdemandname(_defaultModel, nodeIndex, demandIdx, demandName);
}

int EPANET_DEPRECATED_EXPORT ENsetdemandname(int nodeIndex, int demandIdx,
                              char *demandName) {
  return EN_setdemandname(_defaultModel, nodeIndex, demandIdx, demandName);
}

int EPANET_DEPRECATED_EXPORT ENgetrule(int index, int *nPremises, int *nTrueActions,
                        int *nFalseActions, EN_API_FLOAT_TYPE *priority) {
  return EN_getrule(_defaultModel, index, nPremises, nTrueActions, nFalseActions, priority);
}

int EPANET_DEPRECATED_EXPORT ENsetrulepriority(int index, EN_API_FLOAT_TYPE priority){
  return EN_setrulepriority(_defaultModel, index, priority);
}

int EPANET_DEPRECATED_EXPORT ENgetpremise(int indexRule, int indexPremise, int *logop, int *object, int *indexObj, int *variable, int *relop, int *status, EN_API_FLOAT_TYPE *value){
  return EN_getpremise(_defaultModel, indexRule, indexPremise, logop, object, indexObj, variable, relop, status, value);
}

int EPANET_DEPRECATED_EXPORT ENsetpremise(int indexRule, int indexPremise, int logop, int object, int indexObj, int variable, int relop, int status, EN_API_FLOAT_TYPE value){
  return EN_setpremise(_defaultModel, indexRule, indexPremise, logop, object, indexObj, variable, relop, status, value);
}

int EPANET_DEPRECATED_EXPORT ENsetpremiseindex(int indexRule, int indexPremise, int indexObj){
  return EN_setpremiseindex(_defaultModel, indexRule, indexPremise, indexObj);
}

int EPANET_DEPRECATED_EXPORT ENsetpremisestatus(int indexRule, int indexPremise, int status){
  return EN_setpremisestatus(_defaultModel, indexRule, indexPremise, status);
}

int EPANET_DEPRECATED_EXPORT ENsetpremisevalue(int indexRule, int indexPremise, EN_API_FLOAT_TYPE value){
  return EN_setpremisevalue(_defaultModel, indexRule, indexPremise, value);
}

int EPANET_DEPRECATED_EXPORT ENgettrueaction(int indexRule, int indexAction, int *indexLink,
                              int *status, EN_API_FLOAT_TYPE *setting){
  return EN_gettrueaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int EPANET_DEPRECATED_EXPORT ENsettrueaction(int indexRule, int indexAction, int indexLink,
                              int status, EN_API_FLOAT_TYPE setting){
  return EN_settrueaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int EPANET_DEPRECATED_EXPORT ENgetfalseaction(int indexRule, int indexAction, int *indexLink,
                               int *status, EN_API_FLOAT_TYPE *setting){
  return EN_getfalseaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int EPANET_DEPRECATED_EXPORT ENsetfalseaction(int indexRule, int indexAction, int indexLink,
                               int status, EN_API_FLOAT_TYPE setting){
  return EN_setfalseaction(_defaultModel, indexRule, indexAction, indexLink, status, setting);
}

int EPANET_DEPRECATED_EXPORT ENgetruleID(int indexRule, char* id){
  return EN_getruleID(_defaultModel, indexRule, id);
}

int EPANET_DEPRECATED_EXPORT ENaddnode(char *id, EN_NodeType nodeType) {
  return EN_addnode(_defaultModel, id, nodeType);
}

int EPANET_DEPRECATED_EXPORT ENaddlink(char *id, EN_LinkType linkType, char *fromNode,
                        char *toNode) {
  return EN_addlink(_defaultModel, id, linkType, fromNode, toNode);
}

int EPANET_DEPRECATED_EXPORT ENdeletelink(int index) {
  return EN_deletelink(_defaultModel, index);
}

int EPANET_DEPRECATED_EXPORT ENdeletenode(int index) {
  return EN_deletenode(_defaultModel, index);
}
