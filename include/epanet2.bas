Attribute VB_Name = "Module1"

'EPANET2.BAS
'
'Declarations of functions in the EPANET PROGRAMMERs TOOLKIT
'(EPANET2.DLL)

'Last updated on 4/3/07

' These are codes used by the DLL functions
Global Const EN_ELEVATION = 0     ' Node parameters
Global Const EN_BASEDEMAND = 1
Global Const EN_PATTERN = 2
Global Const EN_EMITTER = 3
Global Const EN_INITQUAL = 4
Global Const EN_SOURCEQUAL = 5
Global Const EN_SOURCEPAT = 6
Global Const EN_SOURCETYPE = 7
Global Const EN_TANKLEVEL = 8
Global Const EN_DEMAND = 9
Global Const EN_HEAD = 10
Global Const EN_PRESSURE = 11
Global Const EN_QUALITY = 12
Global Const EN_SOURCEMASS = 13
Global Const EN_INITVOLUME = 14
Global Const EN_MIXMODEL = 15
Global Const EN_MIXZONEVOL = 16

Global Const EN_TANKDIAM = 17
Global Const EN_MINVOLUME = 18
Global Const EN_VOLCURVE = 19
Global Const EN_MINLEVEL = 20
Global Const EN_MAXLEVEL = 21
Global Const EN_MIXFRACTION = 22
Global Const EN_TANK_KBULK = 23
Global Const EN_TANKVOLUME = 24
Global Const EN_MAXVOLUME = 25

Global Const EN_DIAMETER = 0      ' Link parameters
Global Const EN_LENGTH = 1
Global Const EN_ROUGHNESS = 2
Global Const EN_MINORLOSS = 3
Global Const EN_INITSTATUS = 4
Global Const EN_INITSETTING = 5
Global Const EN_KBULK = 6
Global Const EN_KWALL = 7
Global Const EN_FLOW = 8
Global Const EN_VELOCITY = 9
Global Const EN_HEADLOSS = 10
Global Const EN_STATUS = 11
Global Const EN_SETTING = 12
Global Const EN_ENERGY = 13
Global Const EN_LINKQUAL = 14   'ES
Global Const EN_LINKPATTERN = 15

Global Const EN_DURATION = 0      ' Time parameters
Global Const EN_HYDSTEP = 1
Global Const EN_QUALSTEP = 2
Global Const EN_PATTERNSTEP = 3
Global Const EN_PATTERNSTART = 4
Global Const EN_REPORTSTEP = 5
Global Const EN_REPORTSTART = 6
Global Const EN_RULESTEP = 7
Global Const EN_STATISTIC = 8
Global Const EN_PERIODS = 9
Global Const EN_STARTTIME = 10
Global Const EN_HTIME = 11
Global Const EN_QTIME = 12
Global Const EN_HALTFLAG = 13
Global Const EN_NEXTEVENT = 14

Global Const EN_ITERATIONS = 0
Global Const EN_RELATIVEERROR = 1

Global Const EN_NODECOUNT = 0     'Component counts
Global Const EN_TANKCOUNT = 1
Global Const EN_LINKCOUNT = 2
Global Const EN_PATCOUNT = 3
Global Const EN_CURVECOUNT = 4
Global Const EN_CONTROLCOUNT = 5

Global Const EN_JUNCTION = 0      ' Node types
Global Const EN_RESERVOIR = 1
Global Const EN_TANK = 2

Global Const EN_CVPIPE = 0        ' Link types
Global Const EN_PIPE = 1
Global Const EN_PUMP = 2
Global Const EN_PRV = 3
Global Const EN_PSV = 4
Global Const EN_PBV = 5
Global Const EN_FCV = 6
Global Const EN_TCV = 7
Global Const EN_GPV = 8

Global Const EN_NONE = 0          ' Quality analysis types
Global Const EN_CHEM = 1
Global Const EN_AGE = 2
Global Const EN_TRACE = 3

Global Const EN_CONCEN = 0        ' Source quality types
Global Const EN_MASS = 1
Global Const EN_SETPOINT = 2
Global Const EN_FLOWPACED = 3

Global Const EN_CFS = 0           ' Flow units types
Global Const EN_GPM = 1
Global Const EN_MGD = 2
Global Const EN_IMGD = 3
Global Const EN_AFD = 4
Global Const EN_LPS = 5
Global Const EN_LPM = 6
Global Const EN_MLD = 7
Global Const EN_CMH = 8
Global Const EN_CMD = 9

Global Const EN_TRIALS = 0       ' Misc. options
Global Const EN_ACCURACY = 1
Global Const EN_TOLERANCE = 2
Global Const EN_EMITEXPON = 3
Global Const EN_DEMANDMULT = 4

Global Const EN_LOWLEVEL = 0     ' Control types
Global Const EN_HILEVEL = 1
Global Const EN_TIMER = 2
Global Const EN_TIMEOFDAY = 3

Global Const EN_AVERAGE = 1      'Time statistic types
Global Const EN_MINIMUM = 2
Global Const EN_MAXIMUM = 3
Global Const EN_RANGE = 4

Global Const EN_MIX1 = 0         'Tank mixing models
Global Const EN_MIX2 = 1
Global Const EN_FIFO = 2
Global Const EN_LIFO = 3

Global Const EN_NOSAVE = 0       ' Save-results-to-file flag
Global Const EN_SAVE = 1

Global Const EN_INITFLOW = 10    ' Re-initialize flow flag

Global Const EN_CONST_HP = 0      ' constant horsepower
Global Const EN_POWER_FUNC = 1    ' power function
Global Const EN_CUSTOM = 2        ' user-defined custom curve

'These are the external functions that comprise the DLL

 Declare Function ENepanet Lib "epanet2.dll" (ByVal F1 As String, ByVal F2 As String, ByVal F3 As String, ByVal F4 As Any) As Long
 Declare Function ENopen Lib "epanet2.dll" (ByVal F1 As String, ByVal F2 As String, ByVal F3 As String) As Long
 Declare Function ENsaveinpfile Lib "epanet2.dll" (ByVal F As String) As Long
 Declare Function ENclose Lib "epanet2.dll" () As Long

 Declare Function ENsolveH Lib "epanet2.dll" () As Long
 Declare Function ENsaveH Lib "epanet2.dll" () As Long
 Declare Function ENopenH Lib "epanet2.dll" () As Long
 Declare Function ENinitH Lib "epanet2.dll" (ByVal SaveFlag As Long) As Long
 Declare Function ENrunH Lib "epanet2.dll" (T As Long) As Long
 Declare Function ENnextH Lib "epanet2.dll" (Tstep As Long) As Long
 Declare Function ENcloseH Lib "epanet2.dll" () As Long
 Declare Function ENsavehydfile Lib "epanet2.dll" (ByVal F As String) As Long
 Declare Function ENusehydfile Lib "epanet2.dll" (ByVal F As String) As Long

 Declare Function ENsolveQ Lib "epanet2.dll" () As Long
 Declare Function ENopenQ Lib "epanet2.dll" () As Long
 Declare Function ENinitQ Lib "epanet2.dll" (ByVal SaveFlag As Long) As Long
 Declare Function ENrunQ Lib "epanet2.dll" (T As Long) As Long
 Declare Function ENnextQ Lib "epanet2.dll" (Tstep As Long) As Long
 Declare Function ENstepQ Lib "epanet2.dll" (Tleft As Long) As Long
 Declare Function ENcloseQ Lib "epanet2.dll" () As Long

 Declare Function ENwriteline Lib "epanet2.dll" (ByVal S As String) As Long
 Declare Function ENreport Lib "epanet2.dll" () As Long
 Declare Function ENresetreport Lib "epanet2.dll" () As Long
 Declare Function ENsetreport Lib "epanet2.dll" (ByVal S As String) As Long

 Declare Function ENgetcontrol Lib "epanet2.dll" (ByVal Cindex As Long, Ctype As Long, Lindex As Long, Setting As Single, Nindex As Long, Level As Single) As Long
 Declare Function ENgetcount Lib "epanet2.dll" (ByVal Code As Long, Value As Long) As Long
 Declare Function ENgetoption Lib "epanet2.dll" (ByVal Code As Long, Value As Single) As Long
 Declare Function ENgettimeparam Lib "epanet2.dll" (ByVal Code As Long, Value As Long) As Long
 Declare Function ENgetflowunits Lib "epanet2.dll" (Code As Long) As Long
 Declare Function ENgetpatternindex Lib "epanet2.dll" (ByVal ID As String, Index As Long) As Long
 Declare Function ENgetpatternid Lib "epanet2.dll" (ByVal Index As Long, ByVal ID As String) As Long
 Declare Function ENgetpatternlen Lib "epanet2.dll" (ByVal Index As Long, L As Long) As Long
 Declare Function ENgetpatternvalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Period As Long, Value As Single) As Long
 Declare Function ENgetaveragepatternvalue Lib "epanet2.dll" (ByVal Index As Long, Value As Single) As Long
 Declare Function ENgetqualtype Lib "epanet2.dll" (QualCode As Long, TraceNode As Long) As Long
 Declare Function ENgeterror Lib "epanet2.dll" (ByVal ErrCode As Long, ByVal ErrMsg As String, ByVal N As Long) As Long
 Declare Function ENgetstatistic Lib "epanet2.dll" (ByVal Code As Long, Value As Single) As Long

 Declare Function ENgetnodeindex Lib "epanet2.dll" (ByVal ID As String, Index As Long) As Long
 Declare Function ENgetnodeid Lib "epanet2.dll" (ByVal Index As Long, ByVal ID As String) As Long
 Declare Function ENgetnodetype Lib "epanet2.dll" (ByVal Index As Long, Code As Long) As Long
 Declare Function ENgetnodevalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Code As Long, Value As Single) As Long
 Declare Function ENgetcoord Lib "epanet2.dll" (ByVal Index As Long, X As Single, Y As Single) As Long
 Declare Function ENsetcoord Lib "epanet2.dll" (ByVal Index As Long, ByVal X As Single, ByVal Y As Single) As Long

 Declare Function ENgetnumdemands Lib "epanet2.dll" (ByVal Index As Long, numDemands As Long) As Long
 Declare Function ENgetbasedemand Lib "epanet2.dll" (ByVal Index As Long, ByVal DemandIndex As Long, Value As Single) As Long
 Declare Function ENgetdemandpattern Lib "epanet2.dll" (ByVal Index As Long, ByVal DemandIndex As Long, PatIndex As Long) As Long

 Declare Function ENgetlinkindex Lib "epanet2.dll" (ByVal ID As String, Index As Long) As Long
 Declare Function ENgetlinkid Lib "epanet2.dll" (ByVal Index As Long, ByVal ID As String) As Long
 Declare Function ENgetlinktype Lib "epanet2.dll" (ByVal Index As Long, Code As Long) As Long
 Declare Function ENgetlinknodes Lib "epanet2.dll" (ByVal Index As Long, Node1 As Long, Node2 As Long) As Long
 Declare Function ENgetlinkvalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Code As Long, Value As Single) As Long

 Declare Function ENgetcurve Lib "epanet2.dll" (ByVal CurveIndex As Long, ByVal CurveID As String, nValues As Long, xValues As Any, yValues As Any) As Long
 Declare Function ENgetheadcurveindex Lib "epanet2.dll" (ByVal PumpIndex As Long, CurveIndex As Long) As Long
 Declare Function ENgetpumptype Lib "epanet2.dll" (ByVal Index As Long, PumpType As Long) As Long

 Declare Function ENgetversion Lib "epanet2.dll" (Value As Long) As Long

 Declare Function ENsetcontrol Lib "epanet2.dll" (ByVal Cindex As Long, ByVal Ctype As Long, ByVal Lindex As Long, ByVal Setting As Single, ByVal Nindex As Long, ByVal Level As Single) As Long
 Declare Function ENsetnodevalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Code As Long, ByVal Value As Single) As Long
 Declare Function ENsetlinkvalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Code As Long, ByVal Value As Single) As Long
 Declare Function ENaddpattern Lib "epanet2.dll" (ByVal ID As String) As Long
 Declare Function ENsetpattern Lib "epanet2.dll" (ByVal Index As Long, F As Any, ByVal N As Long) As Long
 Declare Function ENsetpatternvalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Period As Long, ByVal Value As Single) As Long
 Declare Function ENsettimeparam Lib "epanet2.dll" (ByVal Code As Long, ByVal Value As Long) As Long
 Declare Function ENsetoption Lib "epanet2.dll" (ByVal Code As Long, ByVal Value As Single) As Long
 Declare Function ENsetstatusreport Lib "epanet2.dll" (ByVal Code As Long) As Long
 Declare Function ENsetqualtype Lib "epanet2.dll" (ByVal QualCode As Long, ByVal ChemName As String, ByVal ChemUnits As String, ByVal TraceNode As String) As Long
 Declare Function ENgetqualinfo Lib "epanet2.dll" (QualCode As Long, ByVal ChemName As String, ByVal ChemUnits As String, TraceNode As Long) As Long
 Declare Function ENsetbasedemand Lib "epanet2.dll" (ByVal NodeIndex As Long, ByVal DemandIndex As Long, ByVal BaseDemand As Single) As Long

 Declare Function ENgetcurveindex Lib "epanet2.dll" (ByVal ID As String, Index As Long) As Long
 Declare Function ENgetcurveid Lib "epanet2.dll" (ByVal Index As Long, ByVal ID As String) As Long
 Declare Function ENgetcurvelen Lib "epanet2.dll" (ByVal Index As Long, L As Long) As Long
 Declare Function ENgetcurvevalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Ptn As Long, X As Single, Y As Single) As Long
 Declare Function ENsetcurvevalue Lib "epanet2.dll" (ByVal Index As Long, ByVal Ptn As Long, ByVal X As Single, ByVal Y As Single) As Long
 Declare Function ENsetcurve Lib "epanet2.dll" (ByVal Index As Long, X As Any, Y As Any, ByVal N As Long) As Long
 Declare Function ENaddcurve Lib "epanet2.dll" (ByVal ID As String) As Long