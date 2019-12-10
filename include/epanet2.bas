Attribute VB_Name = "Module1"

'EPANET2.BAS
'
'Declarations of functions in the EPANET PROGRAMMERs TOOLKIT
'(EPANET2.DLL)

'Last updated on 11/04/2019

' These are codes used by the DLL functions
Public Const EN_ELEVATION = 0     ' Node parameters
Public Const EN_BASEDEMAND = 1
Public Const EN_PATTERN = 2
Public Const EN_EMITTER = 3
Public Const EN_INITQUAL = 4
Public Const EN_SOURCEQUAL = 5
Public Const EN_SOURCEPAT = 6
Public Const EN_SOURCETYPE = 7
Public Const EN_TANKLEVEL = 8
Public Const EN_DEMAND = 9
Public Const EN_HEAD = 10
Public Const EN_PRESSURE = 11
Public Const EN_QUALITY = 12
Public Const EN_SOURCEMASS = 13
Public Const EN_INITVOLUME = 14
Public Const EN_MIXMODEL = 15
Public Const EN_MIXZONEVOL = 16

Public Const EN_TANKDIAM = 17
Public Const EN_MINVOLUME = 18
Public Const EN_VOLCURVE = 19
Public Const EN_MINLEVEL = 20
Public Const EN_MAXLEVEL = 21
Public Const EN_MIXFRACTION = 22
Public Const EN_TANK_KBULK = 23
Public Const EN_TANKVOLUME = 24
Public Const EN_MAXVOLUME = 25
Public Const EN_CANOVERFLOW = 26
Public Const EN_DEMANDDEFICIT = 27 

Public Const EN_DIAMETER = 0      ' Link parameters
Public Const EN_LENGTH = 1
Public Const EN_ROUGHNESS = 2
Public Const EN_MINORLOSS = 3
Public Const EN_INITSTATUS = 4
Public Const EN_INITSETTING = 5
Public Const EN_KBULK = 6
Public Const EN_KWALL = 7
Public Const EN_FLOW = 8
Public Const EN_VELOCITY = 9
Public Const EN_HEADLOSS = 10
Public Const EN_STATUS = 11
Public Const EN_SETTING = 12
Public Const EN_ENERGY = 13
Public Const EN_LINKQUAL = 14
Public Const EN_LINKPATTERN = 15

Public Const EN_PUMP_STATE = 16
Public Const EN_PUMP_EFFIC = 17
Public Const EN_PUMP_POWER = 18
Public Const EN_PUMP_HCURVE = 19
Public Const EN_PUMP_ECURVE = 20
Public Const EN_PUMP_ECOST = 21
Public Const EN_PUMP_EPAT = 22

Public Const EN_DURATION = 0      ' Time parameters
Public Const EN_HYDSTEP = 1
Public Const EN_QUALSTEP = 2
Public Const EN_PATTERNSTEP = 3
Public Const EN_PATTERNSTART = 4
Public Const EN_REPORTSTEP = 5
Public Const EN_REPORTSTART = 6
Public Const EN_RULESTEP = 7
Public Const EN_STATISTIC = 8
Public Const EN_PERIODS = 9
Public Const EN_STARTTIME = 10
Public Const EN_HTIME = 11
Public Const EN_QTIME = 12
Public Const EN_HALTFLAG = 13
Public Const EN_NEXTEVENT = 14

Public Const EN_ITERATIONS = 0    ' Run statistics
Public Const EN_RELATIVEERROR = 1
Public Const EN_MAXHEADERROR = 2
Public Const EN_MAXFLOWCHANGE = 3
Public Const EN_MASSBALANCE = 4
Public Const EN_DEFICIENTNODES = 5
Public Const EN_DEMANDREDUCTION = 6

Public Const EN_NODE = 0          ' Component types
Public Const EN_LINK = 1
Public Const EN_TIMEPAT = 2
Public Const EN_CURVE = 3
Public Const EN_CONTROL = 4
Public Const EN_RULE = 5 

Public Const EN_NODECOUNT = 0     ' Component counts
Public Const EN_TANKCOUNT = 1
Public Const EN_LINKCOUNT = 2
Public Const EN_PATCOUNT = 3
Public Const EN_CURVECOUNT = 4
Public Const EN_CONTROLCOUNT = 5
Public Const EN_RULECOUNT = 6

Public Const EN_JUNCTION = 0      ' Node types
Public Const EN_RESERVOIR = 1
Public Const EN_TANK = 2

Public Const EN_CVPIPE = 0        ' Link types
Public Const EN_PIPE = 1
Public Const EN_PUMP = 2
Public Const EN_PRV = 3
Public Const EN_PSV = 4
Public Const EN_PBV = 5
Public Const EN_FCV = 6
Public Const EN_TCV = 7
Public Const EN_GPV = 8

Public Const EN_NONE = 0          ' Quality analysis types
Public Const EN_CHEM = 1
Public Const EN_AGE = 2
Public Const EN_TRACE = 3

Public Const EN_CONCEN = 0        ' Source quality types
Public Const EN_MASS = 1
Public Const EN_SETPOINT = 2
Public Const EN_FLOWPACED = 3

Public Const EN_HW = 0            ' Head loss formulas
Public Const EN_DW = 1
Public Const EN_CM = 2

Public Const EN_CFS = 0           ' Flow units types
Public Const EN_GPM = 1
Public Const EN_MGD = 2
Public Const EN_IMGD = 3
Public Const EN_AFD = 4
Public Const EN_LPS = 5
Public Const EN_LPM = 6
Public Const EN_MLD = 7
Public Const EN_CMH = 8
Public Const EN_CMD = 9

Public Const EN_DDA = 0           ' Demand driven analysis
Public Const EN_PDA = 1           ' Pressure driven analysis

Public Const EN_TRIALS = 0        ' Simulation options
Public Const EN_ACCURACY = 1
Public Const EN_TOLERANCE = 2
Public Const EN_EMITEXPON = 3
Public Const EN_DEMANDMULT = 4
Public Const EN_HEADERROR = 5
Public Const EN_FLOWCHANGE = 6
Public Const EN_HEADLOSSFORM = 7
Public Const EN_GLOBALEFFIC = 8
Public Const EN_GLOBALPRICE = 9
Public Const EN_GLOBALPATTERN = 10
Public Const EN_DEMANDCHARGE = 11
Public Const EN_SP_GRAVITY = 12
Public Const EN_SP_VISCOS  = 13
Public Const EN_UNBALANCED = 14
Public Const EN_CHECKFREQ = 15
Public Const EN_MAXCHECK = 16
Public Const EN_DAMPLIMIT = 17
Public Const EN_SP_DIFFUS = 18
Public Const EN_BULKORDER = 19
Public Const EN_WALLORDER = 20
Public Const EN_TANKORDER = 21
Public Const EN_CONCENLIMIT = 22

Public Const EN_LOWLEVEL = 0      ' Control types
Public Const EN_HILEVEL = 1
Public Const EN_TIMER = 2
Public Const EN_TIMEOFDAY = 3

Public Const EN_AVERAGE = 1       ' Time statistic types
Public Const EN_MINIMUM = 2
Public Const EN_MAXIMUM = 3
Public Const EN_RANGE = 4

Public Const EN_MIX1 = 0          ' Tank mixing models
Public Const EN_MIX2 = 1
Public Const EN_FIFO = 2
Public Const EN_LIFO = 3

Public Const EN_NOSAVE = 0        ' Save-results-to-file flag
Public Const EN_SAVE = 1
Public Const EN_INITFLOW = 10     ' Re-initialize flow flag
Public Const EN_SAVE_AND_INIT = 11

Public Const EN_CONST_HP = 0      ' Constant horsepower pump curve
Public Const EN_POWER_FUNC = 1    ' Power function pump cuve
Public Const EN_CUSTOM = 2        ' User-defined custom pump curve
Public Const EN_NOCURVE = 3       ' No pump curve

Public Const EN_VOLUME_CURVE = 0  ' Volume curve
Public Const EN_PUMP_CURVE = 1    ' Pump curve
Public Const EN_EFFIC_CURVE = 2   ' Efficiency curve
Public Const EN_HLOSS_CURVE = 3   ' Head loss curve
Public Const EN_GENERIC_CURVE = 4 ' Generic curve

Public Const EN_UNCONDITIONAL = 0 ' Unconditional object deletion
Public Const EN_CONDITIONAL = 1   ' Conditional object deletion

Public Const EN_NO_REPORT = 0     ' No status report
Public Const EN_NORMAL_REPORT = 1 ' Normal status report
Public Const EN_FULL_REPORT = 2   ' Full status report

Public Const EN_R_NODE = 6        ' Rule objects
Public Const EN_R_LINK = 7
Public Const EN_R_SYSTEM = 8

Public Const EN_R_DEMAND = 0      ' Rule variables
Public Const EN_R_HEAD = 1
Public Const EN_R_GRADE = 2
Public Const EN_R_LEVEL = 3
Public Const EN_R_PRESSURE = 4
Public Const EN_R_FLOW = 5
Public Const EN_R_STATUS = 6
Public Const EN_R_SETTING = 7
Public Const EN_R_POWER = 8
Public Const EN_R_TIME = 9
Public Const EN_R_CLOCKTIME = 10
Public Const EN_R_FILLTIME = 11
Public Const EN_R_DRAINTIME = 12

Public Const EN_R_EQ = 0          ' Rule operators
Public Const EN_R_NE = 1
Public Const EN_R_LE = 2
Public Const EN_R_GE = 3
Public Const EN_R_LT = 4
Public Const EN_R_GT = 5
Public Const EN_R_IS = 6
Public Const EN_R_NOT = 7
Public Const EN_R_BELOW = 8
Public Const EN_R_ABOVE = 9

Public Const EN_R_IS_OPEN = 1     ' Rule status types
Public Const EN_R_IS_CLOSED = 2
Public Const EN_R_IS_ACTIVE = 3

Public Const EN_MISSING As Double = -1.0E10

'These are the external functions that comprise the DLL

'Project Functions
 Declare Function ENgetversion Lib "epanet2.dll" (value As Long) As Long
 Declare Function ENepanet Lib "epanet2.dll" (ByVal inpFile As String, ByVal rptFile As String, ByVal outFile As String, ByVal pviewprog As Any) As Long
 Declare Function ENinit Lib "epanet2.dll" (ByVal rptFile As String, ByVal outFile As String, ByVal unitsType As Long, ByVal headlossType As Long) As Long
 Declare Function ENopen Lib "epanet2.dll" (ByVal inpFile As String, ByVal rptFile As String, ByVal outFile As String) As Long
 Declare Function ENgettitle Lib "epanet2.dll" (ByVal line1 As String, ByVal line2 As String, ByVal line3 As String) As Long
 Declare Function ENsettitle Lib "epanet2.dll" (ByVal titleline1 As String, ByVal titleline2 As String, ByVal titleline3 As String) As Long
 Declare Function ENsaveinpfile Lib "epanet2.dll" (ByVal filename As String) As Long
 Declare Function ENclose Lib "epanet2.dll" () As Long

'Hydraulic Analysis Functions
 Declare Function ENsolveH Lib "epanet2.dll" () As Long
 Declare Function ENsaveH Lib "epanet2.dll" () As Long
 Declare Function ENopenH Lib "epanet2.dll" () As Long
 Declare Function ENinitH Lib "epanet2.dll" (ByVal initFlag As Long) As Long
 Declare Function ENrunH Lib "epanet2.dll" (currentTime As Long) As Long
 Declare Function ENnextH Lib "epanet2.dll" (tStep As Long) As Long
 Declare Function ENcloseH Lib "epanet2.dll" () As Long
 Declare Function ENsavehydfile Lib "epanet2.dll" (ByVal filename As String) As Long
 Declare Function ENusehydfile Lib "epanet2.dll" (ByVal filename As String) As Long

'Water Quality Analysis Functions
 Declare Function ENsolveQ Lib "epanet2.dll" () As Long
 Declare Function ENopenQ Lib "epanet2.dll" () As Long
 Declare Function ENinitQ Lib "epanet2.dll" (ByVal saveFlag As Long) As Long
 Declare Function ENrunQ Lib "epanet2.dll" (currentTime As Long) As Long
 Declare Function ENnextQ Lib "epanet2.dll" (tStep As Long) As Long
 Declare Function ENstepQ Lib "epanet2.dll" (timeLeft As Long) As Long
 Declare Function ENcloseQ Lib "epanet2.dll" () As Long

'Reporting Functions
 Declare Function ENwriteline Lib "epanet2.dll" (ByVal line As String) As Long
 Declare Function ENreport Lib "epanet2.dll" () As Long
 Declare Function ENcopyreport Lib "epanet2.dll" (ByVal filename As String) As Long
 Declare Function ENclearreport Lib "epanet2.dll" () As Long
 Declare Function ENresetreport Lib "epanet2.dll" () As Long
 Declare Function ENsetreport Lib "epanet2.dll" (ByVal format As String) As Long
 Declare Function ENsetstatusreport Lib "epanet2.dll" (ByVal level As Long) As Long
 Declare Function ENgetcount Lib "epanet2.dll" (ByVal object As Long, count As Long) As Long
 Declare Function ENgeterror Lib "epanet2.dll" (ByVal errcode As Long, ByVal errmsg As String, ByVal maxLen As Long) As Long
 Declare Function ENgetstatistic Lib "epanet2.dll" (ByVal type_ As Long, ByRef value As Single) As Long
 Declare Function ENgetresultindex Lib "epanet2.dll" (ByVal type_ As Long, ByVal index As Long, ByRef value As Long) As Long

'Analysis Options Functions
 Declare Function ENgetoption Lib "epanet2.dll" (ByVal option_ As Long, value As Single) As Long
 Declare Function ENsetoption Lib "epanet2.dll" (ByVal option_ As Long, ByVal value As Single) As Long
 Declare Function ENgetflowunits Lib "epanet2.dll" (units As Long) As Long
 Declare Function ENsetflowunits Lib "epanet2.dll" (ByVal units As Long) As Long
 Declare Function ENgettimeparam Lib "epanet2.dll" (ByVal param As Long, value As Long) As Long
 Declare Function ENsettimeparam Lib "epanet2.dll" (ByVal param As Long, ByVal value As Long) As Long
 Declare Function ENgetqualinfo Lib "epanet2.dll" (qualType As Long, ByVal chemName As String, ByVal chemUnits As String, traceNode As Long) As Long
 Declare Function ENgetqualtype Lib "epanet2.dll" (qualType As Long, traceNode As Long) As Long
 Declare Function ENsetqualtype Lib "epanet2.dll" (ByVal qualType As Long, ByVal chemName As String, ByVal chemUnits As String, ByVal traceNode As String) As Long

'Node Functions
 Declare Function ENaddnode Lib "epanet2.dll" (ByVal id As String, ByVal nodeType As Long, index As Long) As Long
 Declare Function ENdeletenode Lib "epanet2.dll" (ByVal index As Long, ByVal actionCode As Long) As Long
 Declare Function ENgetnodeindex Lib "epanet2.dll" (ByVal id As String, index As Long) As Long
 Declare Function ENgetnodeid Lib "epanet2.dll" (ByVal index As Long, ByVal id As String) As Long
 Declare Function ENsetnodeid Lib "epanet2.dll" (ByVal index As Long, ByVal newid As String) As Long
 Declare Function ENgetnodetype Lib "epanet2.dll" (ByVal index As Long, nodeType As Long) As Long
 Declare Function ENgetnodevalue Lib "epanet2.dll" (ByVal index As Long, ByVal property As Long, value As Single) As Long
 Declare Function ENsetnodevalue Lib "epanet2.dll" (ByVal index As Long, ByVal property As Long, ByVal value As Single) As Long
 Declare Function ENsetjuncdata Lib "epanet2.dll" (ByVal index As Long, ByVal elev As Single, ByVal dmnd As Single, ByVal dmndpat As String) As Long
 Declare Function ENsettankdata Lib "epanet2.dll" (ByVal index As Long, ByVal elev As Single, ByVal initlvl As Single, ByVal minlvl As Single, ByVal maxlvl As Single, ByVal diam As Single, ByVal minvol As Single, ByVal volcurve As String) As Long
 Declare Function ENgetcoord Lib "epanet2.dll" (ByVal index As Long, x As Double, y As Double) As Long
 Declare Function ENsetcoord Lib "epanet2.dll" (ByVal index As Long, ByVal x As Double, ByVal y As Double) As Long
 
'Nodal Demand Functions
 Declare Function ENgetdemandmodel Lib "epanet2.dll" (type_ As Long, pmin As Single, preq As Single, pexp As Single) As Long
 Declare Function ENsetdemandmodel Lib "epanet2.dll" (ByVal type_ As Long, ByVal pmin As Single, ByVal preq As Single, ByVal pexp As Single) As Long
 Declare Function ENadddemand Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal baseDemand As Single, ByVal patternName As String, ByVal demandName As String) As Long
 Declare Function ENdeletedemand Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandIndex As Long) As Long
 Declare Function ENgetdemandindex Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandName As String, demandIndex As Long) As Long
 Declare Function ENgetnumdemands Lib "epanet2.dll" (ByVal nodeIndex As Long, numDemands As Long) As Long
 Declare Function ENgetbasedemand Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandIndex As Long, value As Single) As Long
 Declare Function ENsetbasedemand Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandIndex As Long, ByVal BaseDemand As Single) As Long
 Declare Function ENgetdemandpattern Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandIndex As Long, patIndex As Long) As Long
 Declare Function ENsetdemandpattern Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandIndex As Long, ByVal patIndex As Long) As Long
 Declare Function ENgetdemandname Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandIndex As Long, ByVal demandName As String) As Long
 Declare Function ENsetdemandname Lib "epanet2.dll" (ByVal nodeIndex As Long, ByVal demandIndex As Long, ByVal demandName As String) As Long

'Link Functions
 Declare Function ENaddlink Lib "epanet2.dll" (ByVal id As String, ByVal linkType As Long, ByVal fromNode As String, ByVal toNode As String, index As Long) As Long
 Declare Function ENdeletelink Lib "epanet2.dll" (ByVal index As Long, ByVal actionCode As Long) As Long
 Declare Function ENgetlinkindex Lib "epanet2.dll" (ByVal id As String, index As Long) As Long
 Declare Function ENgetlinkid Lib "epanet2.dll" (ByVal index As Long, ByVal id As String) As Long
 Declare Function ENsetlinkid Lib "epanet2.dll" (ByVal index As Long, ByVal newid As String) As Long
 Declare Function ENgetlinktype Lib "epanet2.dll" (ByVal index As Long, linkType As Long) As Long
 Declare Function ENsetlinktype Lib "epanet2.dll" (index As Long, ByVal linkType As Long, ByVal actionCode As Long) As Long
 Declare Function ENgetlinknodes Lib "epanet2.dll" (ByVal index As Long, node1 As Long, node2 As Long) As Long
 Declare Function ENsetlinknodes Lib "epanet2.dll" (ByVal index As Long, ByVal node1 As Long, ByVal node2 As Long) As Long
 Declare Function ENgetlinkvalue Lib "epanet2.dll" (ByVal index As Long, ByVal property As Long, value As Single) As Long
 Declare Function ENsetlinkvalue Lib "epanet2.dll" (ByVal index As Long, ByVal property As Long, ByVal value As Single) As Long
 Declare Function ENsetpipedata Lib "epanet2.dll" (ByVal index As Long, ByVal length As Single, ByVal diam As Single, ByVal rough As Single, ByVal mloss As Single) As Long
 Declare Function ENgetvertexcount Lib "epanet2.dll" (ByVal index As Long, count As Long) As Long
 Declare Function ENgetvertex Lib "epanet2.dll" (ByVal index As Long, ByVal vertex As Long, x As Double, y As Double) As Long
 Declare Function ENsetvertices Lib "epanet2.dll" (ByVal index As Long, xCoords As Any, yCoords As Any, ByVal count As Long) As Long

'Pump Functions
 Declare Function ENgetheadcurveindex Lib "epanet2.dll" (ByVal linkIndex As Long, curveIndex As Long) As Long
 Declare Function ENsetheadcurveindex Lib "epanet2.dll" (ByVal linkIndex As Long, ByVal curveIndex As Long) As Long
 Declare Function ENgetpumptype Lib "epanet2.dll" (ByVal linkIndex As Long, pumpType As Long) As Long

'Time Pattern Functions
 Declare Function ENaddpattern Lib "epanet2.dll" (ByVal id As String) As Long
 Declare Function ENdeletepattern Lib "epanet2.dll" (ByVal index As Long) As Long
 Declare Function ENgetpatternindex Lib "epanet2.dll" (ByVal id As String, index As Long) As Long
 Declare Function ENgetpatternid Lib "epanet2.dll" (ByVal index As Long, ByVal id As String) As Long
 Declare Function ENsetpatternid Lib "epanet2.dll" (ByVal index As Long, ByVal newid As String) As Long
 Declare Function ENgetpatternlen Lib "epanet2.dll" (ByVal index As Long, len_ As Long) As Long
 Declare Function ENgetpatternvalue Lib "epanet2.dll" (ByVal index As Long, ByVal period As Long, value As Single) As Long
 Declare Function ENsetpatternvalue Lib "epanet2.dll" (ByVal index As Long, ByVal period As Long, ByVal value As Single) As Long
 Declare Function ENgetaveragepatternvalue Lib "epanet2.dll" (ByVal index As Long, value As Single) As Long
 Declare Function ENsetpattern Lib "epanet2.dll" (ByVal index As Long, values As Any, ByVal len_ As Long) As Long

'Data Curve Functions
 Declare Function ENaddcurve Lib "epanet2.dll" (ByVal id As String) As Long
 Declare Function ENdeletecurve Lib "epanet2.dll" (ByVal index As Long) As Long
 Declare Function ENgetcurveindex Lib "epanet2.dll" (ByVal id As String, index As Long) As Long
 Declare Function ENgetcurveid Lib "epanet2.dll" (ByVal index As Long, ByVal id As String) As Long
 Declare Function ENsetcurveid Lib "epanet2.dll" (ByVal index As Long, ByVal newid As String) As Long
 Declare Function ENgetcurvelen Lib "epanet2.dll" (ByVal index As Long, len_ As Long) As Long
 Declare Function ENgetcurvetype Lib "epanet2.dll" (ByVal index As Long, type_ As Long) As Long
 Declare Function ENgetcurvevalue Lib "epanet2.dll" (ByVal curveIndex As Long, ByVal pointIndex As Long, x As Single, y As Single) As Long
 Declare Function ENsetcurvevalue Lib "epanet2.dll" (ByVal curveIndex As Long, ByVal pointIndex As Long, ByVal x As Single, ByVal y As Single) As Long
 Declare Function ENgetcurve Lib "epanet2.dll" (ByVal index As Long, ByVal id As String, nPoints As Long, xValues As Any, yValues As Any) As Long
 Declare Function ENsetcurve Lib "epanet2.dll" (ByVal index As Long, xValues As Any, yValues As Any, ByVal nPoints As Long) As Long

'Simple Control Functions
 Declare Function ENaddcontrol Lib "epanet2.dll" (ByVal type_ As Long, ByVal linkIndex As Long, ByVal setting As Single, ByVal nodeIndex As Long, ByVal level As Single, index As Long) As Long
 Declare Function ENdeletecontrol Lib "epanet2.dll" (ByVal index As Long) As Long
 Declare Function ENgetcontrol Lib "epanet2.dll" (ByVal index As Long, type_ As Long, linkIndex As Long, setting As Single, nodeIndex As Long, level As Single) As Long
 Declare Function ENsetcontrol Lib "epanet2.dll" (ByVal index As Long, ByVal type_ As Long, ByVal linkIndex As Long, ByVal setting As Single, ByVal nodeIndex As Long, ByVal level As Single) As Long

'Rule-Based Control Functions
 Declare Function ENaddrule Lib "epanet2.dll" (ByVal rule As String) As Long
 Declare Function ENdeleterule Lib "epanet2.dll" (ByVal index As Long) As Long
 Declare Function ENgetrule Lib "epanet2.dll" (ByVal index As Long, nPremises As Long, nThenActions As Long, nElseActions As Long, priority As Single) As Long
 Declare Function ENgetruleID Lib "epanet2.dll" (ByVal index As Long, ByVal id As String) As Long
 Declare Function ENsetrulepriority Lib "epanet2.dll" (ByVal index As Long, ByVal priority As Single) As Long
 Declare Function ENgetpremise Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal premiseIndex As Long, logop As Long, object As Long, objIndex As Long, variable As Long, relop As Long, status As Long, value As Single) As Long
 Declare Function ENsetpremise Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal premiseIndex As Long, ByVal logop As Long, ByVal object As Long, ByVal objIndex As Long, ByVal variable As Long, ByVal relop As Long, ByVal status As Long, ByVal value As Single) As Long
 Declare Function ENsetpremiseindex Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal premiseIndex As Long, ByVal objIndex As Long) As Long
 Declare Function ENsetpremisestatus Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal premiseIndex As Long, ByVal status As Long) As Long
 Declare Function ENsetpremisevalue Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal premiseIndex As Long, ByVal value As Single) As Long
 Declare Function ENgetthenaction Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal actionIndex As Long, linkIndex As Long, status As Long, setting As Single) As Long
 Declare Function ENsetthenaction Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal actionIndex As Long, ByVal linkIndex As Long, ByVal status As Long, ByVal setting As Single) As Long
 Declare Function ENgetelseaction Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal actionIndex As Long, linkIndex As Long, status As Long, setting As Single) As Long
 Declare Function ENsetelseaction Lib "epanet2.dll" (ByVal ruleIndex As Long, ByVal actionIndex As Long, ByVal linkIndex As Long, ByVal status As Long, ByVal setting As Single) As Long
