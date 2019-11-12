unit epanet2;

{ Declarations of imported procedures from the EPANET PROGRAMMERs TOOLKIT }
{ (EPANET2.DLL) }

{Last updated on 11/12/19}

interface

const

{ These are codes used by the DLL functions }
 EN_MAXID = 31;        { Max. # characters in ID name }
 EN_MAXMSG = 255;      { Max. # characters in strings }
 EN_MISSING = -1.E10;

 EN_ELEVATION  = 0;    { Node parameters }
 EN_BASEDEMAND = 1;
 EN_PATTERN    = 2;
 EN_EMITTER    = 3;
 EN_INITQUAL   = 4;
 EN_SOURCEQUAL = 5;
 EN_SOURCEPAT  = 6;
 EN_SOURCETYPE = 7;
 EN_TANKLEVEL  = 8;
 EN_DEMAND     = 9;
 EN_HEAD       = 10;
 EN_PRESSURE   = 11;
 EN_QUALITY    = 12;
 EN_SOURCEMASS = 13;
 EN_INITVOLUME = 14;
 EN_MIXMODEL   = 15;
 EN_MIXZONEVOL = 16;

 EN_TANKDIAM    = 17;
 EN_MINVOLUME   = 18;
 EN_VOLCURVE    = 19;
 EN_MINLEVEL    = 20;
 EN_MAXLEVEL    = 21;
 EN_MIXFRACTION = 22;
 EN_TANK_KBULK  = 23;
 EN_TANKVOLUME  = 24;
 EN_MAXVOLUME   = 25;
 EN_CANOVERFLOW = 26;
 EN_DEMANDDEFICIT = 27;  

 EN_DIAMETER    = 0;    { Link parameters }
 EN_LENGTH      = 1;
 EN_ROUGHNESS   = 2;
 EN_MINORLOSS   = 3;
 EN_INITSTATUS  = 4;
 EN_INITSETTING = 5;
 EN_KBULK       = 6;
 EN_KWALL       = 7;
 EN_FLOW        = 8;
 EN_VELOCITY    = 9;
 EN_HEADLOSS    = 10;
 EN_STATUS      = 11;
 EN_SETTING     = 12;
 EN_ENERGY      = 13;
 EN_LINKQUAL    = 14;
 EN_LINKPATTERN = 15;
 EN_PUMP_STATE  = 16;
 EN_PUMP_EFFIC  = 17;
 EN_PUMP_POWER  = 18;
 EN_PUMP_HCURVE = 19;
 EN_PUMP_ECURVE = 20;
 EN_PUMP_ECOST  = 21;
 EN_PUMP_EPAT   = 22;
 
 EN_DURATION     = 0;  { Time parameters }
 EN_HYDSTEP      = 1;
 EN_QUALSTEP     = 2;
 EN_PATTERNSTEP  = 3;
 EN_PATTERNSTART = 4;
 EN_REPORTSTEP   = 5;
 EN_REPORTSTART  = 6;
 EN_RULESTEP     = 7;
 EN_STATISTIC    = 8;
 EN_PERIODS      = 9;
 EN_STARTTIME    = 10;
 EN_HTIME        = 11;
 EN_QTIME        = 12;
 EN_HALTFLAG     = 13;
 EN_NEXTEVENT    = 14;
 EN_NEXTEVENTTANK = 15;
 
 EN_ITERATIONS     = 0;  { Analysis statistics }
 EN_RELATIVEERROR  = 1;
 EN_MAXHEADERROR   = 2;
 EN_MAXFLOWCHANGE  = 3;
 EN_MASSBALANCE    = 4;
 EN_DEFICIENTNODES = 5;
 EN_DEMANDREDUCTION = 6;

 EN_NODE    = 0;        { Component Types }
 EN_LINK    = 1;
 EN_TIMEPAT = 2;
 EN_CURVE   = 3;
 EN_CONTROL = 4;
 EN_RULE    = 5;

 EN_NODECOUNT    = 0; { Component counts }
 EN_TANKCOUNT    = 1;
 EN_LINKCOUNT    = 2;
 EN_PATCOUNT     = 3;
 EN_CURVECOUNT   = 4;
 EN_CONTROLCOUNT = 5;
 EN_RULECOUNT    = 6;
  
 EN_JUNCTION   = 0;   { Node types }
 EN_RESERVOIR  = 1;
 EN_TANK       = 2;

 EN_CVPIPE     = 0;   { Link types }
 EN_PIPE       = 1;
 EN_PUMP       = 2;
 EN_PRV        = 3;
 EN_PSV        = 4;
 EN_PBV        = 5;
 EN_FCV        = 6;
 EN_TCV        = 7;
 EN_GPV        = 8;
 
 EN_CLOSED     = 0;   { Link status types }
 EN_OPEN       = 1;
 
 EN_PUMP_XHEAD  = 0;  { Pump state types }
 EN_PUMP_CLOSED = 2;
 EN_PUMP_OPEN   = 3;
 EN_PUMP_XFLOW  = 5;

 EN_NONE       = 0;   { Quality analysis types }
 EN_CHEM       = 1;
 EN_AGE        = 2;
 EN_TRACE      = 3;

 EN_CONCEN     = 0;   { Source quality types }
 EN_MASS       = 1;
 EN_SETPOINT   = 2;
 EN_FLOWPACED  = 3;

 EN_HW         = 0;   { Head loss formulas }
 EN_DW         = 1;
 EN_CM         = 2;
  
 EN_CFS        = 0;   { Flow units types }
 EN_GPM        = 1;
 EN_MGD        = 2;
 EN_IMGD       = 3;
 EN_AFD        = 4;
 EN_LPS        = 5;
 EN_LPM        = 6;
 EN_MLD        = 7;
 EN_CMH        = 8;
 EN_CMD        = 9;

 EN_DDA        = 0;   { Demand model types }
 EN_PDA        = 1;  
 
 EN_TRIALS     = 0;   { Option types }
 EN_ACCURACY   = 1;
 EN_TOLERANCE  = 2;
 EN_EMITEXPON  = 3;
 EN_DEMANDMULT = 4;
 EN_HEADERROR  = 5;
 EN_FLOWCHANGE = 6;
 EN_HEADLOSSFORM = 7;
 EN_GLOBALEFFIC  = 8;
 EN_GLOBALPRICE  = 9;
 EN_GLOBALPATTERN = 10;
 EN_DEMANDCHARGE  = 11;
 EN_SP_GRAVITY   = 12;
 EN_SP_VISCOS    = 13;
 EN_EXTRA_ITER   = 14;
 EN_CHECKFREQ    = 15;
 EN_MAXCHECK     = 16;
 EN_DAMPLIMIT    = 17;
 EN_SP_DIFFUS    = 18;
 EN_BULKORDER    = 19;
 EN_WALLORDER    = 20;
 EN_TANKORDER    = 21;
 EN_CONCENLIMIT  = 22;

 EN_LOWLEVEL   = 0;   { Control types }
 EN_HILEVEL    = 1;
 EN_TIMER      = 2;
 EN_TIMEOFDAY  = 3;

 EN_SERIES     = 0;   { Report statistic types }
 EN_AVERAGE    = 1;
 EN_MINIMUM    = 2; 
 EN_MAXIMUM    = 3;
 EN_RANGE      = 4;

 EN_MIX1       = 0;   { Tank mixing models }
 EN_MIX2       = 1;
 EN_FIFO       = 2;
 EN_LIFO       = 3;

 EN_NOSAVE     = 0;   { Hydraulics flags }
 EN_SAVE       = 1;
 EN_INITFLOW   = 10;
 EN_SAVE_AND_INIT = 11;
 
 EN_CONST_HP   = 0;   { Pump curve types }
 EN_POWER_FUNC = 1;
 EN_CUSTOM     = 2;
 EN_NOCURVE    = 3;  
 
 EN_VOLUME_CURVE  = 0; { Curve types }
 EN_PUMP_CURVE    = 1;
 EN_EFFIC_CURVE   = 2;
 EN_HLOSS_CURVE   = 3;
 EN_GENERIC_CURVE = 4; 
 
 EN_UNCONDITIONAL = 0; { Deletion action codes }
 EN_CONDITIONAL   = 1; 

 EN_NO_REPORT     = 0; { Status reporting levels }
 EN_NORMAL_REPORT = 1;
 EN_FULL_REPORT   = 2;
 
 EN_R_NODE     = 6;    { Rule-based control objects }
 EN_R_LINK     = 7;
 EN_R_SYSTEM   = 8;
 
 EN_R_DEMAND    = 0;   { Rule-based control variables }
 EN_R_HEAD      = 1;
 EN_R_GRADE     = 2;
 EN_R_LEVEL     = 3;
 EN_R_PRESSURE  = 4;
 EN_R_FLOW      = 5;
 EN_R_STATUS    = 6;
 EN_R_SETTING   = 7;
 EN_R_POWER     = 8;
 EN_R_TIME      = 9;
 EN_R_CLOCKTIME = 10;
 EN_R_FILLTIME  = 11;
 EN_R_DRAINTIME = 12;  
 
 EN_R_EQ        = 0;   { Rule-based control operators }
 EN_R_NE        = 1;
 EN_R_LE        = 2;
 EN_R_GE        = 3;
 EN_R_LT        = 4;
 EN_R_GT        = 5;
 EN_R_IS        = 6;
 EN_R_NOT       = 7;
 EN_R_BELOW     = 8;
 EN_R_ABOVE     = 9; 
 
 EN_R_IS_OPEN   = 1;   { Rule-based control link status }
 EN_R_IS_CLOSED = 2;
 EN_R_IS_ACTIVE = 3;
 
 EpanetLib = 'epanet2.dll';
  
{Project Functions}  
 function  ENepanet(F1: PAnsiChar; F2: PAnsiChar; F3: PAnsiChar; F4: Pointer): Integer; stdcall; external EpanetLib;
 function  ENinit(F2: PAnsiChar; F3: PAnsiChar; UnitsType: Integer; HeadlossType: Integer): Integer; stdcall; external EpanetLib;
 function  ENopen(F1: PAnsiChar; F2: PAnsiChar; F3: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENgetcount(Code: Integer; var Count: Integer): Integer; stdcall; external EpanetLib;
 function  ENgettitle(Line1: PAnsiChar; Line2: PAnsiChar; Line3: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsettitle(Line1: PAnsiChar; Line2: PAnsiChar; Line3: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENgetcomment(ObjType: Integer; Index: Integer; Comment: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsetcomment(ObjType: Integer; Index: Integer; Comment: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsaveinpfile(F: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENclose: Integer; stdcall; external EpanetLib;

{Hydraulic Functions} 
 function  ENsolveH: Integer; stdcall; external EpanetLib;
 function  ENsaveH: Integer; stdcall; external EpanetLib;
 function  ENopenH: Integer; stdcall; external EpanetLib;
 function  ENinitH(SaveFlag: Integer): Integer; stdcall; external EpanetLib;
 function  ENrunH(var T: LongInt): Integer; stdcall; external EpanetLib;
 function  ENnextH(var Tstep: LongInt): Integer; stdcall; external EpanetLib;
 function  ENcloseH: Integer; stdcall; external EpanetLib;
 function  ENsavehydfile(F: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENusehydfile(F: PAnsiChar): Integer; stdcall; external EpanetLib;

{Quality Functions}
 function  ENsolveQ: Integer; stdcall; external EpanetLib;
 function  ENopenQ: Integer; stdcall; external EpanetLib;
 function  ENinitQ(SaveFlag: Integer): Integer; stdcall; external EpanetLib;
 function  ENrunQ(var T: LongInt): Integer; stdcall; external EpanetLib;
 function  ENnextQ(var Tstep: LongInt): Integer; stdcall; external EpanetLib;
 function  ENstepQ(var Tleft: LongInt): Integer; stdcall; external EpanetLib;
 function  ENcloseQ: Integer; stdcall; external EpanetLib;

{Reporting Functions}
 function  ENwriteline(S: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENreport: Integer; stdcall; external EpanetLib;
 function  ENcopyreport(F: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENclearreport: Integer; stdcall; external EpanetLib;
 function  ENresetreport: Integer; stdcall; external EpanetLib;
 function  ENsetreport(S: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsetstatusreport(Code: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetversion(var Version: Integer): Integer; stdcall; external EpanetLib;
 function  ENgeterror(Errcode: Integer; Errmsg: PAnsiChar; MaxLen: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetstatistic(StatType: Integer; var Value: Single): Integer; stdcall; external EpanetLib;
 function  ENgetresultindex(Code: Integer; Index: Integer; var Value: Integer): Integer; stdcall; external EpanetLib; 

{Analysis Options Functions}
 function  ENgetoption(Code: Integer; var Value: Single): Integer; stdcall; external EpanetLib;
 function  ENsetoption(Code: Integer; Value: Single): Integer; stdcall; external EpanetLib;
 function  ENgetflowunits(var Code: Integer): Integer; stdcall; external EpanetLib;
 function  ENsetflowunits(Code: Integer): Integer; stdcall; external EpanetLib;
 function  ENgettimeparam(Code: Integer; var Value: LongInt): Integer; stdcall; external EpanetLib;
 function  ENsettimeparam(Code: Integer; Value: LongInt): Integer; stdcall; external EpanetLib;
 function  ENgetqualinfo(var QualType: Integer; ChemName: PAnsiChar; ChemUnits: PAnsiChar; var TraceNode: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetqualtype(var QualCode: Integer; var TraceNode: Integer): Integer; stdcall; external EpanetLib;
 function  ENsetqualtype(QualCode: Integer; ChemName: PAnsiChar; ChemUnits: PAnsiChar; TraceNodeID: PAnsiChar): Integer; stdcall; external EpanetLib;

{Node Functions} 
 function  ENaddnode(ID: PAnsiChar; NodeType: Integer; var Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENdeletenode(Index: Integer; ActionCode: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetnodeindex(ID: PAnsiChar; var Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetnodeid(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsetnodeid(Index: Integer; NewID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENgetnodetype(Index: Integer; var Code: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetnodevalue(Index: Integer; Code: Integer; var Value: Single): Integer; stdcall; external EpanetLib;
 function  ENsetnodevalue(Index: Integer; Code: Integer; Value: Single): Integer; stdcall; external EpanetLib;
 function  ENsetjuncdata(Index: Integer; Elev: Single; Dmnd: Single; DmndPat: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsettankdata(Index: Integer; Elev, InitLvl, MinLvl, MaxLvl, Diam, MinVol: Single; VolCurve: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENgetcoord(Index: Integer; var X: Double; var Y: Double): Integer; stdcall; external EpanetLib;
 function  ENsetcoord(Index: Integer; X: Double; Y: Double): Integer; stdcall; external EpanetLib;

{Demand Functions}
 function  ENgetdemandmodel(var Model: Integer; var Pmin: Single; var Preq: Single; var Pexp: Single): Integer; stdcall; external EpanetLib;
 function  ENsetdemandmodel(Model: Integer; Pmin: Single; Preq: Single; Pexp: Single): Integer; stdcall; external EpanetLib;
 function  ENgetnumdemands(NodeIndex: Integer; var NumDemands: Integer): Integer; stdcall; external EpanetLib;
 function  ENadddemand(NodeIndex: Integer; BaseDemand: Single; PatName: PAnsiChar; DemandName: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENdeletedemand(NodeIndex: Integer; DemandIndex: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetdemandindex(NodeIndex: Integer; DemandName: PAnsiChar; var DemandIndex: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetbasedemand(NodeIndex: Integer; DemandIndex: Integer; var BaseDemand: Single): Integer; stdcall; external EpanetLib;
 function  ENsetbasedemand(NodeIndex: Integer; DemandIndex: Integer; BaseDemand: Single): Integer; stdcall; external EpanetLib;
 function  ENgetdemandpattern(NodeIndex: Integer; DemandIndex: Integer; var PatIndex: Integer): Integer; stdcall; external EpanetLib;
 function  ENsetdemandpattern(NodeIndex: Integer; DemandIndex: Integer; PatIndex: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetdemandname(NodeIndex: Integer; DemandIndex: Integer; DemandName: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsetdemandname(NodeIndex: Integer; DemandIndex: Integer; DemandName: PAnsiChar): Integer; stdcall; external EpanetLib;

{Link Functions}
 function  ENaddlink(ID: PAnsiChar; LinkType: Integer; FromNode: PAnsiChar; ToNode: PAnsiChar; var Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENdeletelink(Index: Integer; ActionCode: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetlinkindex(ID: PAnsiChar; var Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetlinkid(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsetlinkid(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENgetlinktype(Index: Integer; var Code: Integer): Integer; stdcall; external EpanetLib;
 function  ENsetlinktype(var Index: Integer; LinkType: Integer; ActionCode: Integer): Integer; stdcall; external EpanetLib; 
 function  ENgetlinknodes(Index: Integer; var Node1: Integer; var Node2: Integer): Integer; stdcall; external EpanetLib;
 function  ENsetlinknodes(Index: Integer; Node1: Integer; Node2: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetlinkvalue(Index: Integer; Code: Integer; var Value: Single): Integer; stdcall; external EpanetLib;
 function  ENsetlinkvalue(Index: Integer; Code: Integer; Value: Single): Integer; stdcall; external EpanetLib;
 function  ENsetpipedata(Index: Integer; Length: Single; Diam: Single; Rough: Single; Mloss:Single): Integer; stdcall; external EpanetLib;

 function  ENgetvertexcount(Index: Integer; var Count: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetvertex(Index: Integer; Vertex: Integer; var X: Double; var Y: Double): Integer; stdcall; external EpanetLib;
 function  ENsetvertices(Index: Integer; var X: Double; var Y: Double; Count: Integer): Integer; stdcall; external EpanetLib;
 
{Pump Functions}
 function  ENgetpumptype(LinkIndex: Integer; var PumpType: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetheadcurveindex(LinkIndex: Integer; var CurveIndex: Integer): Integer; stdcall; external EpanetLib;
 function  ENsetheadcurveindex(LinkIndex: Integer; CurveIndex: Integer): Integer; stdcall; external EpanetLib; 
 
{Pattern Functions} 
 function  ENaddpattern(ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENdeletepattern(Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetpatternindex(ID: PAnsiChar; var Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetpatternid(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsetpatternid(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENgetpatternlen(Index: Integer; var Len: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetpatternvalue(Index: Integer; Period: Integer; var Value: Single): Integer; stdcall; external EpanetLib;
 function  ENsetpatternvalue(Index: Integer; Period: Integer; Value: Single): Integer; stdcall; external EpanetLib;
 function  ENgetaveragepatternvalue(Index: Integer; var Value: Single): Integer; stdcall; external EpanetLib;
 function  ENsetpattern(Index: Integer; var F: Single; N: Integer): Integer; stdcall; external EpanetLib;
           
{Curve Functions}
 function  ENaddcurve(ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENdeletecurve(Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetcurveindex(ID: PAnsiChar; var Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetcurveid(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENsetcurveid(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function  ENgetcurvelen(Index: Integer; var Len: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetcurvetype(Index: Integer; var CurveType: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetcurvevalue(CurveIndex: Integer; PointIndex: Integer; var X: Single; var Y: Single): Integer; stdcall; external EpanetLib;
 function  ENsetcurvevalue(CurveIndex: Integer; PointIndex: Integer; X: Single; Y: Single): Integer; stdcall; external EpanetLib;
 function  ENgetcurve(Index: Integer; ID: PAnsiChar; var N: Integer; var X: Single; var Y: Single): Integer; stdcall; external EpanetLib;
 function  ENsetcurve(Index: Integer; var X: Single; var Y: Single; N: Integer): Integer; stdcall; external EpanetLib;

{Control Functions}
 function  ENaddcontrol(Ctype: Integer; Link: Integer; Setting: Single; Node: Integer; Level: Single; var Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENdeletecontrol(Index: Integer): Integer; stdcall; external EpanetLib;
 function  ENgetcontrol(Index: Integer; var Ctype: Integer; var Link: Integer; var Setting: Single; var Node: Integer; var Level: Single): Integer; stdcall; external EpanetLib;
 function  ENsetcontrol(Index: Integer; Ctype: Integer; Link: Integer; Setting: Single; Node: Integer; Level: Single): Integer; stdcall; external EpanetLib;

 {Rule-Based Control Functions}
 function ENaddrule(Rule: PAnsiChar): Integer; stdcall; external EpanetLib;
 function ENdeleterule(Index: Integer): Integer; stdcall; external EpanetLib;
 function ENgetrule(Index: Integer; var Npremises: Integer; var NthenActions: Integer;
                    var NelseActions: Integer; var Priority: Single): Integer; stdcall; external EpanetLib;
 function ENgetruleID(Index: Integer; ID: PAnsiChar): Integer; stdcall; external EpanetLib;
 function ENsetrulepriority(Index: Integer; Priority: Single): Integer; stdcall; external EpanetLib;
 function ENgetpremise(RuleIndex: Integer; PremiseIndex: Integer; var LogOp: Integer;
          var ObjType: Integer; var ObjIndex: Integer; var Param: Integer; var RelOp: Integer;
          var Status: Integer; var Value: Single): Integer; stdcall; external EpanetLib;
 function ENsetpremise(RuleIndex: Integer; PremiseIndex: Integer; LogOp: Integer; ObjType: Integer;
          ObjIndex: Integer; Param: Integer; RelOp: Integer; Status: Integer; Value: Single): Integer; stdcall; external EpanetLib;
 function ENsetpremiseindex(RuleIndex: Integer; PremiseIndex: Integer; ObjIndex: Integer): Integer; stdcall; external EpanetLib;
 function ENsetpremisestatus(RuleIndex: Integer; PremiseIndex: Integer; Status: Integer): Integer; stdcall; external EpanetLib;
 function ENsetpremisevalue(RuleIndex: Integer; PremiseIndex: Integer; Value: Single): Integer; stdcall; external EpanetLib;
 function ENgetthenaction(RuleIndex: Integer; ActionIndex: Integer; var LinkIndex: Integer;
          var Status: Integer; var Setting: Single): Integer; stdcall; external EpanetLib;
 function ENsetthenaction(RuleIndex: Integer; ActionIndex: Integer; LinkIndex: Integer;
          Status: Integer; Setting: Single): Integer; stdcall; external EpanetLib;
 function ENgetelseaction(RuleIndex: Integer; ActionIndex: Integer; var LinkIndex: Integer;
          var Status: Integer; var Setting: Single): Integer; stdcall; external EpanetLib;
 function ENsetelseaction(RuleIndex: Integer; ActionIndex: Integer; LinkIndex: Integer;
          Status: Integer; Setting: Single): Integer; stdcall; external EpanetLib;

implementation

end.
