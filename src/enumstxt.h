/*
 *****************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       enumstxt.h
 Description:  text strings for enumerated data types
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 06/20/2019
 ******************************************************************************
*/

#ifndef ENUMSTXT_H
#define ENUMSTXT_H
#include "text.h"

char *NodeTxt[]         = {t_JUNCTION,
                           t_RESERVOIR,
                           t_TANK}; 

char *LinkTxt[]         = {w_CV, 
                           w_PIPE,
                           w_PUMP,
                           w_PRV,
                           w_PSV,
                           w_PBV,
                           w_FCV,
                           w_TCV,
                           w_GPV};

char *StatTxt[]         = {t_XHEAD,
                           t_TEMPCLOSED,
                           t_CLOSED,
                           t_OPEN,
                           t_ACTIVE,
                           t_XFLOW,
                           t_XFCV,
                           t_XPRESSURE,
                           t_FILLING,
                           t_EMPTYING,
                           t_OVERFLOWING};

char *FormTxt[]         = {w_HW,
                           w_DW,
                           w_CM};

char *RptFormTxt[]      = {t_HW,
                           t_DW,
                           t_CM};

char *RptFlowUnitsTxt[] = {u_CFS,
                           u_GPM,
                           u_MGD,
                           u_IMGD,
                           u_AFD,
                           u_LPS,
                           u_LPM,
                           u_MLD,
                           u_CMH,
                           u_CMD};

char *FlowUnitsTxt[]    = {w_CFS,
                           w_GPM, 
                           w_MGD,
                           w_IMGD,
                           w_AFD,
                           w_LPS, 
                           w_LPM, 
                           w_MLD, 
                           w_CMH,
                           w_CMD};

char *PressUnitsTxt[]   = {w_PSI,
                           w_KPA,
                           w_METERS};

char *DemandModelTxt[] = { w_DDA,
                           w_PDA,
                           NULL };

char *QualTxt[]         = {w_NONE,
                           w_CHEM,
                           w_AGE,
                           w_TRACE};  


char *SourceTxt[]       = {w_CONCEN,
                           w_MASS,
                           w_SETPOINT,
                           w_FLOWPACED};

char *ControlTxt[]      = {w_BELOW,
                           w_ABOVE,
                           w_TIME,
                           w_CLOCKTIME};

char *TstatTxt[]        = {w_NONE,
                           w_AVG,
                           w_MIN,
                           w_MAX,
                           w_RANGE};

char *MixTxt[]          = {w_MIXED,
                           w_2COMP,
                           w_FIFO,
                           w_LIFO, 
                           NULL};

char *RptFlagTxt[]      = {w_NO,
                           w_YES,
                           w_FULL};

char *SectTxt[]         = {s_TITLE,     s_JUNCTIONS, s_RESERVOIRS,
                           s_TANKS,     s_PIPES,     s_PUMPS,
                           s_VALVES,    s_CONTROLS,  s_RULES,
                           s_DEMANDS,   s_SOURCES,   s_EMITTERS,
                           s_PATTERNS,  s_CURVES,    s_QUALITY,
                           s_STATUS,    s_ROUGHNESS, s_ENERGY,
                           s_REACTIONS, s_MIXING,    s_REPORT,
                           s_TIMES,     s_OPTIONS,   s_COORDS,
                           s_VERTICES,  s_LABELS,    s_BACKDROP,
                           s_TAGS,      s_END,
                           NULL};

char *Fldname[]         = {t_ELEV,      t_DEMAND,    t_HEAD,
                           t_PRESSURE,  t_QUALITY,   t_LENGTH,
                           t_DIAM,      t_FLOW,      t_VELOCITY,
                           t_HEADLOSS,  t_LINKQUAL,  t_LINKSTATUS,
                           t_SETTING,   t_REACTRATE, t_FRICTION,
                           "", "", "", "", "", "", NULL};


#endif
