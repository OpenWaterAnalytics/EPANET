//using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

//epanet2.cs[By Oscar Vegas]
//Last updated on 06/23/2024

//Declarations of functions in the EPANET PROGRAMMERs TOOLKIT
//(EPANET2.DLL) for use with C#


namespace EpanetCSharpLibrary
{  
    
    public static class Epanet
    {

        public const string EPANETDLL = "epanet2.dll";

        //These are codes used by the DLL functions
        public const int EN_ELEVATION = 0;    // Node parameters
        public const int EN_BASEDEMAND = 1;
        public const int EN_PATTERN = 2;
        public const int EN_EMITTER = 3;
        public const int EN_INITQUAL = 4;
        public const int EN_SOURCEQUAL = 5;
        public const int EN_SOURCEPAT = 6;
        public const int EN_SOURCETYPE = 7;
        public const int EN_TANKLEVEL = 8;
        public const int EN_DEMAND = 9;
        public const int EN_HEAD = 10;
        public const int EN_PRESSURE = 11;
        public const int EN_QUALITY = 12;
        public const int EN_SOURCEMASS = 13;
        public const int EN_INITVOLUME = 14;
        public const int EN_MIXMODEL = 15;
        public const int EN_MIXZONEVOL = 16;

        public const int EN_TANKDIAM = 17;
        public const int EN_MINVOLUME = 18;
        public const int EN_VOLCURVE = 19;
        public const int EN_MINLEVEL = 20;
        public const int EN_MAXLEVEL = 21;
        public const int EN_MIXFRACTION = 22;
        public const int EN_TANK_KBULK = 23;

        public const int EN_TANKVOLUME = 24;
        public const int EN_MAXVOLUME = 25;
        public const int EN_CANOVERFLOW = 26;
        public const int EN_DEMANDDEFICIT = 27;
        public const int EN_NODE_INCONTROL = 28;
        public const int EN_EMITTERFLOW = 29;
        public const int EN_LEAKAGEFLOW = 30;
        public const int EN_DEMANDFLOW = 31;
        public const int EN_FULLDEMAND = 32;

        public const int EN_DIAMETER = 0;      //Link parameters
        public const int EN_LENGTH = 1;
        public const int EN_ROUGHNESS = 2;
        public const int EN_MINORLOSS = 3;
        public const int EN_INITSTATUS = 4;
        public const int EN_INITSETTING = 5;
        public const int EN_KBULK = 6;
        public const int EN_KWALL = 7;
        public const int EN_FLOW = 8;
        public const int EN_VELOCITY = 9;
        public const int EN_HEADLOSS = 10;
        public const int EN_STATUS = 11;
        public const int EN_SETTING = 12;
        public const int EN_ENERGY = 13;
        public const int EN_LINKQUAL = 14;
        public const int EN_LINKPATTERN = 15;

        public const int EN_PUMP_STATE = 16;
        public const int EN_PUMP_EFFIC = 17;
        public const int EN_PUMP_POWER = 18;
        public const int EN_PUMP_HCURVE = 19;
        public const int EN_PUMP_ECURVE = 20;
        public const int EN_PUMP_ECOST = 21;
        public const int EN_PUMP_EPAT = 22;
        public const int EN_LINK_INCONTROL = 23;
        public const int EN_GPV_CURVE = 24;
        public const int EN_PCV_CURVE = 25;
        public const int EN_LEAK_AREA = 26;
        public const int EN_LEAK_EXPAN = 27;
        public const int EN_LINK_LEAKAGE = 28;

        public const int EN_DURATION = 0;      //Time parameters
        public const int EN_HYDSTEP = 1;
        public const int EN_QUALSTEP = 2;
        public const int EN_PATTERNSTEP = 3;
        public const int EN_PATTERNSTART = 4;
        public const int EN_REPORTSTEP = 5;
        public const int EN_REPORTSTART = 6;
        public const int EN_RULESTEP = 7;
        public const int EN_STATISTIC = 8;
        public const int EN_PERIODS = 9;
        public const int EN_STARTTIME = 10;
        public const int EN_HTIME = 11;
        public const int EN_QTIME = 12;
        public const int EN_HALTFLAG = 13;
        public const int EN_NEXTEVENT = 14;

        public const int EN_ITERATIONS = 0;
        public const int EN_RELATIVEERROR = 1;
        public const int EN_MAXHEADERROR = 2;
        public const int EN_MAXFLOWCHANGE = 3;
        public const int EN_MASSBALANCE = 4;
        public const int EN_DEFICIENTNODES = 5;
        public const int EN_DEMANDREDUCTION = 6;
        public const int EN_LEAKAGELOSS = 7;

        public const int EN_NODE = 0;          //Component types
        public const int EN_LINK = 1;
        public const int EN_TIMEPAT = 2;
        public const int EN_CURVE = 3;
        public const int EN_CONTROL = 4;
        public const int EN_RULE = 5;

        public const int EN_NODECOUNT = 0;     //Component counts
        public const int EN_TANKCOUNT = 1;
        public const int EN_LINKCOUNT = 2;
        public const int EN_PATCOUNT = 3;
        public const int EN_CURVECOUNT = 4;
        public const int EN_CONTROLCOUNT = 5;
        public const int EN_RULECOUNT = 6;

        public const int EN_JUNCTION = 0;      //Node types
        public const int EN_RESERVOIR = 1;
        public const int EN_TANK = 2;

        public const int EN_CVPIPE = 0;        //Link types
        public const int EN_PIPE = 1;
        public const int EN_PUMP = 2;
        public const int EN_PRV = 3;
        public const int EN_PSV = 4;
        public const int EN_PBV = 5;
        public const int EN_FCV = 6;
        public const int EN_TCV = 7;
        public const int EN_GPV = 8;
        public const int EN_PCV = 9;

        public const int EN_NONE = 0;          //Quality analysis types
        public const int EN_CHEM = 1;
        public const int EN_AGE = 2;
        public const int EN_TRACE = 3;

        public const int EN_CONCEN = 0;        //Source quality types
        public const int EN_MASS = 1;
        public const int EN_SETPOINT = 2;
        public const int EN_FLOWPACED = 3;

        public const int EN_HW = 0;            //Head loss formulas
        public const int EN_DW = 1;
        public const int EN_CM = 2;

        public const int EN_CFS = 0;           //Flow units types
        public const int EN_GPM = 1;
        public const int EN_MGD = 2;
        public const int EN_IMGD = 3;
        public const int EN_AFD = 4;
        public const int EN_LPS = 5;
        public const int EN_LPM = 6;
        public const int EN_MLD = 7;
        public const int EN_CMH = 8;
        public const int EN_CMD = 9;
        public const int EN_CMS = 10;

        public const int EN_PSI = 0;           //Pressure units types
        public const int EN_KPA = 1;
        public const int EN_METERS = 2;

        public const int EN_DDA = 0;           //Demand driven analysis
        public const int EN_PDA = 1;           //Pressure driven analysis

        public const int EN_TRIALS = 0;        //Simulation options
        public const int EN_ACCURACY = 1;
        public const int EN_TOLERANCE = 2;
        public const int EN_EMITEXPON = 3;
        public const int EN_DEMANDMULT = 4;
        public const int EN_HEADERROR = 5;
        public const int EN_FLOWCHANGE = 6;
        public const int EN_HEADLOSSFORM = 7;
        public const int EN_GLOBALEFFIC = 8;
        public const int EN_GLOBALPRICE = 9;
        public const int EN_GLOBALPATTERN = 10;
        public const int EN_DEMANDCHARGE = 11;
        public const int EN_SP_GRAVITY = 12;
        public const int EN_SP_VISCOS  = 13;
        public const int EN_UNBALANCED = 14;
        public const int EN_CHECKFREQ = 15;
        public const int EN_MAXCHECK = 16;
        public const int EN_DAMPLIMIT = 17;
        public const int EN_SP_DIFFUS = 18;
        public const int EN_BULKORDER = 19;
        public const int EN_WALLORDER = 20;
        public const int EN_TANKORDER = 21;
        public const int EN_CONCENLIMIT = 22;
        public const int EN_DEMANDPATTERN = 23;
        public const int EN_EMITBACKFLOW = 24;
        public const int EN_PRESS_UNITS = 25;
        public const int EN_STATUS_REPORT = 26;

        public const int EN_LOWLEVEL = 0;      //Control types
        public const int EN_HILEVEL = 1;
        public const int EN_TIMER = 2;
        public const int EN_TIMEOFDAY = 3;

        public const int EN_AVERAGE = 1;       //Time statistic types
        public const int EN_MINIMUM = 2;
        public const int EN_MAXIMUM = 3;
        public const int EN_RANGE = 4;

        public const int EN_MIX1 = 0;          //Tank mixing models
        public const int EN_MIX2 = 1;
        public const int EN_FIFO = 2;
        public const int EN_LIFO = 3;

        public const int EN_NOSAVE = 0;        //Save-results-to-file flag
        public const int EN_SAVE = 1;
        public const int EN_INITFLOW = 10;     //Re-initialize flow flag
        public const int EN_SAVE_AND_INIT = 11;

        public const int EN_CONST_HP = 0;      //Constant horsepower pump curve
        public const int EN_POWER_FUNC = 1;    //Power function pump curve
        public const int EN_CUSTOM = 2;        //User-defined custom pump curve
        public const int EN_NOCURVE = 3;       //No pump curve

        public const int EN_VOLUME_CURVE = 0;  //Volume curve
        public const int EN_PUMP_CURVE = 1;    //Pump curve
        public const int EN_EFFIC_CURVE = 2;   //Efficiency curve
        public const int EN_HLOSS_CURVE = 3;   //Head loss curve
        public const int EN_GENERIC_CURVE = 4; //Generic curve
        public const int EN_VALVE_CURVE = 5;   //Valve position curve

        public const int EN_UNCONDITIONAL = 0; //Unconditional object deletion
        public const int EN_CONDITIONAL   = 1; //Conditional object deletion

        public const int EN_NO_REPORT = 0;     //No status report
        public const int EN_NORMAL_REPORT = 1; //Normal status report
        public const int EN_FULL_REPORT = 2;   //Full status report

        public const int EN_R_NODE   = 6;      //Rule objects
        public const int EN_R_LINK   = 7;
        public const int EN_R_SYSTEM = 8;

        public const int EN_R_DEMAND    = 0;   //Rule variables
        public const int EN_R_HEAD      = 1;
        public const int EN_R_GRADE     = 2;
        public const int EN_R_LEVEL     = 3;
        public const int EN_R_PRESSURE  = 4;
        public const int EN_R_FLOW      = 5;
        public const int EN_R_STATUS    = 6;
        public const int EN_R_SETTING   = 7;
        public const int EN_R_POWER     = 8;
        public const int EN_R_TIME      = 9;
        public const int EN_R_CLOCKTIME = 10;
        public const int EN_R_FILLTIME  = 11;
        public const int EN_R_DRAINTIME = 12;

        public const int EN_R_EQ    = 0;       //Rule operators
        public const int EN_R_NE    = 1;
        public const int EN_R_LE    = 2;
        public const int EN_R_GE    = 3;
        public const int EN_R_LT    = 4;
        public const int EN_R_GT    = 5;
        public const int EN_R_IS    = 6;
        public const int EN_R_NOT   = 7;
        public const int EN_R_BELOW = 8;
        public const int EN_R_ABOVE = 9;

        public const int EN_R_IS_OPEN   = 1;   //Rule status types
        public const int EN_R_IS_CLOSED = 2;
        public const int EN_R_IS_ACTIVE = 3;

        public const double EN_MISSING = -1.0E10;
        public const double EN_SET_CLOSED = -1.0E10
        public const double EN_SET_OPEN = 1.0E10
        
        public const int EN_FALSE = 0   // boolean false
        public const int EN_TRUE  = 1   // boolean true

        #region Epanet Imports

        public delegate void UserSuppliedFunction(string param0);


        //Project Functions
        [DllImport(EPANETDLL, EntryPoint = "ENgetversion")]
        public static extern int ENgetversion(ref int version);

        [DllImport(EPANETDLL, EntryPoint = "ENepanet")]
        public static extern int ENepanet(string inpFile, string rptFile, string outFile, UserSuppliedFunction vfunc);

        [DllImport(EPANETDLL, EntryPoint = "ENopen")]
        public static extern int ENopen(string inpFile, string rptFile, string outFile);

        [DllImport(EPANETDLL, EntryPoint = "ENopenX")]
        public static extern int ENopenX(string inpFile, string rptFile, string outFile);

        [DllImport(EPANETDLL, EntryPoint = "ENgettitle")]
        public static extern int ENgettitle(string titleline1, string titleline2, string titleline3);

        [DllImport(EPANETDLL, EntryPoint = "ENsettitle")]
        public static extern int ENsettitle(string titleline1, string titleline2, string titleline3);

        [DllImport(EPANETDLL, EntryPoint = "ENsaveinpfile")]
        public static extern int ENsaveinpfile(string filename);

        [DllImport(EPANETDLL, EntryPoint = "ENclose")]
        public static extern int ENclose();


        //Hydraulic Analysis Functions
        [DllImport(EPANETDLL, EntryPoint = "ENsolveH")]
        public static extern int ENsolveH();

        [DllImport(EPANETDLL, EntryPoint = "ENsaveH")]
        public static extern int ENsaveH();

        [DllImport(EPANETDLL, EntryPoint = "ENopenH")]
        public static extern int ENopenH();

        [DllImport(EPANETDLL, EntryPoint = "ENinitH")]
        public static extern int ENinitH(int initFlag);

        [DllImport(EPANETDLL, EntryPoint = "ENrunH")]
        public static extern int ENrunH(ref long currentTime);

        [DllImport(EPANETDLL, EntryPoint = "ENnextH")]
        public static extern int ENnextH(ref long tStep);

        [DllImport(EPANETDLL, EntryPoint = "ENcloseH")]
        public static extern int ENcloseH();

        [DllImport(EPANETDLL, EntryPoint = "ENsavehydfile")]
        public static extern int ENsavehydfile(string filename);

        [DllImport(EPANETDLL, EntryPoint = "ENusehydfile")]
        public static extern int ENusehydfile(string filename);


        //Water Quality Analysis Functions
        [DllImport(EPANETDLL, EntryPoint = "ENsolveQ")]
        public static extern int ENsolveQ();

        [DllImport(EPANETDLL, EntryPoint = "ENopenQ")]
        public static extern int ENopenQ();

        [DllImport(EPANETDLL, EntryPoint = "ENinitQ")]
        public static extern int ENinitQ(int saveFlag);

        [DllImport(EPANETDLL, EntryPoint = "ENrunQ")]
        public static extern int ENrunQ(ref long currentTime);

        [DllImport(EPANETDLL, EntryPoint = "ENnextQ")]
        public static extern int ENnextQ(ref long tStep);

        [DllImport(EPANETDLL, EntryPoint = "ENstepQ")]
        public static extern int ENstepQ(ref long timeLeft);

        [DllImport(EPANETDLL, EntryPoint = "ENcloseQ")]
        public static extern int ENcloseQ();


        //Reporting Functions
        [DllImport(EPANETDLL, EntryPoint = "ENwriteline")]
        public static extern int ENwriteline(string line);

        [DllImport(EPANETDLL, EntryPoint = "ENreport")]
        public static extern int ENreport();

        [DllImport(EPANETDLL, EntryPoint = "ENcopyreport")]
        public static extern int ENcopyreport(string filename);

        [DllImport(EPANETDLL, EntryPoint = "ENclearreport")]
        public static extern int ENclearreport();

        [DllImport(EPANETDLL, EntryPoint = "ENresetreport")]
        public static extern int ENresetreport();

        [DllImport(EPANETDLL, EntryPoint = "ENsetreport")]
        public static extern int ENsetreport(string format);

        [DllImport(EPANETDLL, EntryPoint = "ENsetstatusreport")]
        public static extern int ENsetstatusreport(int level);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcount")]
        public static extern int ENgetcount(int code, ref int count);

        [DllImport(EPANETDLL, EntryPoint = "ENgeterror")]
        public static extern int ENgeterror(int errcode, string errmsg, int maxLen);

        [DllImport(EPANETDLL, EntryPoint = "ENgetstatistic")]
        public static extern int ENgetstatistic(int type, ref int value);

        [DllImport(EPANETDLL, EntryPoint = "ENgetresultindex")]
        public static extern int ENgetresultindex(int type, int index, ref int value);

        [DllImport(EPANETDLL, EntryPoint = "ENtimetonextevent")]
        public static extern int ENtimetonextevent(ref int eventType, ref long duration, ref int elementIndex);


        //Analysis Options Functions
        [DllImport(EPANETDLL, EntryPoint = "ENgetoption")]
        public static extern int ENgetoption(int option, ref float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetoption")]
        public static extern int ENsetoption(int option, ref float value);

        [DllImport(EPANETDLL, EntryPoint = "ENgetflowunits")]
        public static extern int ENgetflowunits(ref int units);

        [DllImport(EPANETDLL, EntryPoint = "ENsetflowunits")]
        public static extern int ENsetflowunits(int units);

        [DllImport(EPANETDLL, EntryPoint = "ENgettimeparam")]
        public static extern int ENgettimeparam(int param, ref int value);

        [DllImport(EPANETDLL, EntryPoint = "ENsettimeparam")]
        public static extern int ENsettimeparam(int optioncode, long value);

        [DllImport(EPANETDLL, EntryPoint = "ENgetqualinfo")]
        public static extern int ENgetqualinfo(ref int qualType, string chemName, string chemUnits, ref int traceNode);

        [DllImport(EPANETDLL, EntryPoint = "ENgetqualtype")]
        public static extern int ENgetqualtype(ref int qualType, ref int traceNode);

        [DllImport(EPANETDLL, EntryPoint = "ENsetqualtype")]
        public static extern int ENsetqualtype(int qualType, string chemName, string chemUnits, string traceNode);


        //Node Functions
        [DllImport(EPANETDLL, EntryPoint = "ENaddnode")]
        public static extern int ENaddnode(string id, int nodeType, ref int index);

        [DllImport(EPANETDLL, EntryPoint = "ENdeletenode")]
        public static extern int ENdeletenode(int index, int actionCode);

        [DllImport(EPANETDLL, EntryPoint = "ENgetnodeindex")]
        public static extern int ENgetnodeindex(string id, ref int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetnodeid")]
        public static extern int ENgetnodeid(int index, string id);

        [DllImport(EPANETDLL, EntryPoint = "ENsetnodeid")]
        public static extern int ENsetnodeid(int index, string newid);

        [DllImport(EPANETDLL, EntryPoint = "ENgetnodetype")]
        public static extern int ENgetnodetype(int index, ref int nodeType);

        [DllImport(EPANETDLL, EntryPoint = "ENgetnodevalue")]
        public static extern int ENgetnodevalue(int index, int param, ref float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetnodevalue")]
        public static extern int ENsetnodevalue(int index, int param, float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetjuncdata")]
        public static extern int ENsetjuncdata(int index, float elev, float dmnd, string dmndpat);

        [DllImport(EPANETDLL, EntryPoint = "ENsettankdata")]
        public static extern int ENsettankdata(int index, float elev, float initlvl, float minlvl, float maxlvl, float diam, float minvol, string volcurve);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcoord")]
        public static extern int ENgetcoord(int index, ref double x, ref double y);

        [DllImport(EPANETDLL, EntryPoint = "ENsetcoord")]
        public static extern int ENsetcoord(int index, double x, double y);

        [DllImport(EPANETDLL, EntryPoint = "ENgetnodevalues")]
        public static extern int ENgetnodevalues(int param, ref float values);

        //Nodal Demand Functions
        [DllImport(EPANETDLL, EntryPoint = "ENgetdemandmodel")]
        public static extern int ENgetdemandmodel(ref int model, ref float pmin, ref float preq, ref float pexp);

        [DllImport(EPANETDLL, EntryPoint = "ENsetdemandmodel")]
        public static extern int ENsetdemandmodel(int model, float pmin, float preq, float pexp);

        [DllImport(EPANETDLL, EntryPoint = "ENadddemand")]
        public static extern int ENadddemand(int nodeIndex, float baseDemand, string demandPattern, string demandName);

        [DllImport(EPANETDLL, EntryPoint = "ENdeletedemand")]
        public static extern int ENdeletedemand(int nodeIndex, int demandIndex);

        [DllImport(EPANETDLL, EntryPoint = "ENgetdemandindex")]
        public static extern int ENgetdemandindex(int nodeIndex, string demandName, ref int demandIndex);

        [DllImport(EPANETDLL, EntryPoint = "ENgetnumdemands")]
        public static extern int ENgetnumdemands(int nodeIndex, ref int numDemands);

        [DllImport(EPANETDLL, EntryPoint = "ENgetbasedemand")]
        public static extern int ENgetbasedemand(int nodeIndex, int demandIndex, ref float baseDemand);

        [DllImport(EPANETDLL, EntryPoint = "ENsetbasedemand")]
        public static extern int ENsetbasedemand(int nodeIndex, int demandIndex, float baseDemand);

        [DllImport(EPANETDLL, EntryPoint = "ENgetdemandpattern")]
        public static extern int ENgetdemandpattern(int nodeIndex, int demandIndex, ref int patIndex);

        [DllImport(EPANETDLL, EntryPoint = "ENsetdemandpattern")]
        public static extern int ENsetdemandpattern(int nodeIndex, int demandIndex, int patIndex);

        [DllImport(EPANETDLL, EntryPoint = "ENgetdemandname")]
        public static extern int ENgetdemandname(int nodeIndex, int demandIndex, string demandName);

        [DllImport(EPANETDLL, EntryPoint = "ENsetdemandname")]
        public static extern int ENsetdemandname(int nodeIndex, int demandIndex, string demandName);


        //Link Functions
        [DllImport(EPANETDLL, EntryPoint = "ENaddlink")]
        public static extern int ENaddlink(string id, int linkType, string fromNode, string toNode, ref int index);

        [DllImport(EPANETDLL, EntryPoint = "ENdeletelink")]
        public static extern int ENdeletelink(int index, int actionCode);

        [DllImport(EPANETDLL, EntryPoint = "ENgetlinkindex")]
        public static extern int ENgetlinkindex(string id, ref int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetlinkid")]
        public static extern int ENgetlinkid(int index, string id);

        [DllImport(EPANETDLL, EntryPoint = "ENsetlinkid")]
        public static extern int ENsetlinkid(int index, string newid);

        [DllImport(EPANETDLL, EntryPoint = "ENgetlinktype")]
        public static extern int ENgetlinktype(int index, ref int linkType);

        [DllImport(EPANETDLL, EntryPoint = "ENsetlinktype")]
        public static extern int ENsetlinktype(ref int index, int linkType, int actionCode);

        [DllImport(EPANETDLL, EntryPoint = "ENgetlinknodes")]
        public static extern int ENgetlinknodes(int index, ref int node1, ref int node2);

        [DllImport(EPANETDLL, EntryPoint = "ENsetlinknodes")]
        public static extern int ENsetlinknodes(int index, int node1, int node2);

        [DllImport(EPANETDLL, EntryPoint = "ENgetlinkvalue")]
        public static extern int ENgetlinkvalue(int index, int param, ref float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetlinkvalue")]
        public static extern int ENsetlinkvalue(int index, int param, float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpipedata")]
        public static extern int ENsetpipedata(int index, float length, float diam, float rough, float mloss);

        [DllImport(EPANETDLL, EntryPoint = "ENgetvertexcount")]
        public static extern int ENgetvertexcount(int index, ref int count);

        [DllImport(EPANETDLL, EntryPoint = "ENgetvertex")]
        public static extern int ENgetvertex(int index, int vertex, ref double x, ref double y);

        [DllImport(EPANETDLL, EntryPoint = "ENsetvertices")]
        public static extern int ENsetvertices(int index, ref double[] x, ref double[] y, int count);

        [DllImport(EPANETDLL, EntryPoint = "ENgetlinkvalues")]
        public static extern int ENgetlinkvalues(int param, ref float values);


        //Pump Functions
        [DllImport(EPANETDLL, EntryPoint = "ENgetheadcurveindex")]
        public static extern int ENgetheadcurveindex(int linkIndex, ref int curveIndex);

        [DllImport(EPANETDLL, EntryPoint = "ENsetheadcurveindex")]
        public static extern int ENsetheadcurveindex(int linkIndex, int curveIndex);

        [DllImport(EPANETDLL, EntryPoint = "ENgetpumptype")]
        public static extern int ENgetpumptype(int linkIndex, ref int pumpType);


        //Time Pattern Functions
        [DllImport(EPANETDLL, EntryPoint = "ENaddpattern")]
        public static extern int ENaddpattern(string id);

        [DllImport(EPANETDLL, EntryPoint = "ENdeletepattern")]
        public static extern int ENdeletepattern(int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetpatternindex")]
        public static extern int ENgetpatternindex(string id, ref int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetpatternid")]
        public static extern int ENgetpatternid(int index, string id);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpatternid")]
        public static extern int ENsetpatternid(int index, string id);

        [DllImport(EPANETDLL, EntryPoint = "ENgetpatternlen")]
        public static extern int ENgetpatternlen(int index, ref int len);

        [DllImport(EPANETDLL, EntryPoint = "ENgetpatternvalue")]
        public static extern int ENgetpatternvalue(int index, int period, ref float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpatternvalue")]
        public static extern int ENsetpatternvalue(int index, int period, float value);

        [DllImport(EPANETDLL, EntryPoint = "ENgetaveragepatternvalue")]
        public static extern int ENgetaveragepatternvalue(int index, ref float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpattern")]
        public static extern int ENsetpattern(int index, ref float[] values, int len);


        //Data Curve Functions
        [DllImport(EPANETDLL, EntryPoint = "ENaddcurve")]
        public static extern int ENaddcurve(string id);

        [DllImport(EPANETDLL, EntryPoint = "ENdeletecurve")]
        public static extern int ENdeletecurve(int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcurveindex")]
        public static extern int ENgetcurveindex(string id, ref int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcurveid")]
        public static extern int ENgetcurveid(int index, string id);

        [DllImport(EPANETDLL, EntryPoint = "ENsetcurveid")]
        public static extern int ENsetcurveid(int index, string id);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcurvelen")]
        public static extern int ENgetcurvelen(int index, ref int len);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcurvetype")]
        public static extern int ENgetcurvetype(int index, ref int type);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcurvevalue")]
        public static extern int ENgetcurvevalue(int curveIndex, int pointIndex, ref float x, ref float y);

        [DllImport(EPANETDLL, EntryPoint = "ENsetcurvevalue")]
        public static extern int ENsetcurvevalue(int curveIndex, int pointIndex, float x, float y);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcurve")]
        public static extern int ENgetcurve(int index, string id, ref int nPoints, ref float xValues, ref float yValues);

        [DllImport(EPANETDLL, EntryPoint = "ENsetcurve")]
        public static extern int ENsetcurve(int index, ref float[] xValues, ref float[] yValues, int nPoints);


        //Simple Control Functions
        [DllImport(EPANETDLL, EntryPoint = "ENaddcontrol")]
        public static extern int ENaddcontrol(int type, int linkIndex, float setting, int nodeIndex, float level, ref int index);

        [DllImport(EPANETDLL, EntryPoint = "ENdeletecontrol")]
        public static extern int ENdeletecontrol(int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcontrol")]
        public static extern int ENgetcontrol(int index, ref int type, ref int linkIndex, ref float setting, ref int nodeIndex, ref float level);

        [DllImport(EPANETDLL, EntryPoint = "ENsetcontrol")]
        public static extern int ENsetcontrol(int index, int type, int linkIndex, float setting, int nodeIndex, float level);

        [DllImport(EPANETDLL, EntryPoint = "ENgetcontrolenabled")]
        public static extern int ENgetcontrolenabled(int index, int out_enabled);

        [DllImport(EPANETDLL, EntryPoint = "ENsetcontrolenabled")]
        public static extern int ENsetcontrolenabled(int index, int enabled);


        //Rule-Based Control Functions
        [DllImport(EPANETDLL, EntryPoint = "ENaddrule")]
        public static extern int ENaddrule(string rule);

        [DllImport(EPANETDLL, EntryPoint = "ENdeleterule")]
        public static extern int ENdeleterule(int index);

        [DllImport(EPANETDLL, EntryPoint = "ENgetrule")]
        public static extern int ENgetrule(int index, ref int nPremises, ref int nThenActions, ref int nElseActions, ref float priority);

        [DllImport(EPANETDLL, EntryPoint = "ENgetruleID")]
        public static extern int ENgetruleID(int index, string id);

        [DllImport(EPANETDLL, EntryPoint = "ENsetrulepriority")]
        public static extern int ENsetcurvENsetrulepriorityeid(int index, float priority);

        [DllImport(EPANETDLL, EntryPoint = "ENgetpremise")]
        public static extern int ENgetpremise(int ruleIndex, int premiseIndex, ref int logop, ref int objectt, ref int objIndex, ref int variable, ref int relop, ref int status, ref float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpremise")]
        public static extern int ENsetpremise(int ruleIndex, int premiseIndex, int logop, int objectt, int objIndex, int variable, int relop, int status, float value);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpremiseindex")]
        public static extern int ENsetpremiseindex(int ruleIndex, int premiseIndex, int objIndex);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpremisestatus")]
        public static extern int ENsetpremisestatus(int ruleIndex, int premiseIndex, int status);

        [DllImport(EPANETDLL, EntryPoint = "ENsetpremisevalue")]
        public static extern int ENsetpremisevalue(int ruleIndex, int premiseIndex, float value);

        [DllImport(EPANETDLL, EntryPoint = "ENgetthenaction")]
        public static extern int ENgetthenaction(int ruleIndex, int actionIndex, ref int linkIndex, ref int status, ref float setting);

        [DllImport(EPANETDLL, EntryPoint = "ENsetthenaction")]
        public static extern int ENsetthenaction(int ruleIndex, int actionIndex, int linkIndex, int status, float setting);

        [DllImport(EPANETDLL, EntryPoint = "ENgetelseaction")]
        public static extern int ENgetelseaction(int ruleIndex, int actionIndex, ref int linkIndex, ref int status, ref float setting);

        [DllImport(EPANETDLL, EntryPoint = "ENsetelseaction")]
        public static extern int ENsetelseaction(int ruleIndex, int actionIndex, int linkIndex, int status, float setting);

        [DllImport(EPANETDLL, EntryPoint = "ENgetruleenabled")]
        public static extern int ENgetruleenabled(int index, int out_enabled);

        [DllImport(EPANETDLL, EntryPoint = "ENsetruleenabled")]
        public static extern int ENsetruleenabled(int index, int enabled);

        #endregion
    }
    
}
