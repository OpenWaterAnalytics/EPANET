
'EPANET2.VB
'
'Declarations of functions in the EPANET PROGRAMMERs TOOLKIT
'(EPANET2.DLL) for use with VB.Net.

'Last updated on 7/19/15 - LR

Imports System.Runtime.InteropServices
Imports System.Text

Module Epanet2

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

Public Const EN_TANKVOLUME = 24 'ES

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
Public Const EN_LINKQUAL = 14   'ES
Public Const EN_LINKPATTERN = 15

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
Public Const EN_STARTTIME = 10    'ES

Public Const EN_NODECOUNT = 0     'Component counts
Public Const EN_TANKCOUNT = 1
Public Const EN_LINKCOUNT = 2
Public Const EN_PATCOUNT = 3
Public Const EN_CURVECOUNT = 4
Public Const EN_CONTROLCOUNT = 5

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

Public Const EN_TRIALS = 0       ' Misc. options
Public Const EN_ACCURACY = 1
Public Const EN_TOLERANCE = 2
Public Const EN_EMITEXPON = 3
Public Const EN_DEMANDMULT = 4

Public Const EN_LOWLEVEL = 0     ' Control types
Public Const EN_HILEVEL = 1
Public Const EN_TIMER = 2
Public Const EN_TIMEOFDAY = 3

Public Const EN_AVERAGE = 1      'Time statistic types
Public Const EN_MINIMUM = 2
Public Const EN_MAXIMUM = 3
Public Const EN_RANGE = 4

Public Const EN_MIX1 = 0         'Tank mixing models
Public Const EN_MIX2 = 1
Public Const EN_FIFO = 2
Public Const EN_LIFO = 3

Public Const EN_NOSAVE = 0       ' Save-results-to-file flag
Public Const EN_SAVE = 1
Public Const EN_INITFLOW = 10    ' Re-initialize flow flag

Public Const EN_CONST_HP = 0      ' constant horsepower
Public Const EN_POWER_FUNC = 1    ' power function
Public Const EN_CUSTOM = 2        ' user-defined custom curve

'These are the external functions that comprise the DLL

 Declare Function ENepanet Lib "epanet2.dll" (ByVal F1 As String, ByVal F2 As String, ByVal F3 As String, ByVal F4 As String) As Int32
 Declare Function ENopen Lib "epanet2.dll" (ByVal F1 As String, ByVal F2 As String, ByVal F3 As String) As Int32
 Declare Function ENsaveinpfile Lib "epanet2.dll" (ByVal F As String) As Int32
 Declare Function ENclose Lib "epanet2.dll" () As Int32

 Declare Function ENsolveH Lib "epanet2.dll" () As Int32
 Declare Function ENsaveH Lib "epanet2.dll" () As Int32
 Declare Function ENopenH Lib "epanet2.dll" () As Int32
 Declare Function ENinitH Lib "epanet2.dll" (ByVal SaveFlag As Int32) As Int32
 Declare Function ENrunH Lib "epanet2.dll" (ByRef T As Int32) As Int32
 Declare Function ENnextH Lib "epanet2.dll" (ByRef Tstep As Int32) As Int32
 Declare Function ENcloseH Lib "epanet2.dll" () As Int32
 Declare Function ENsavehydfile Lib "epanet2.dll" (ByVal F As String) As Int32
 Declare Function ENusehydfile Lib "epanet2.dll" (ByVal F As String) As Int32

 Declare Function ENsolveQ Lib "epanet2.dll" () As Int32
 Declare Function ENopenQ Lib "epanet2.dll" () As Int32
 Declare Function ENinitQ Lib "epanet2.dll" (ByVal SaveFlag As Int32) As Int32
 Declare Function ENrunQ Lib "epanet2.dll" (ByRef T As Int32) As Int32
 Declare Function ENnextQ Lib "epanet2.dll" (ByRef Tstep As Int32) As Int32
 Declare Function ENstepQ Lib "epanet2.dll" (ByRef Tleft As Int32) As Int32
 Declare Function ENcloseQ Lib "epanet2.dll" () As Int32

 Declare Function ENwriteline Lib "epanet2.dll" (ByVal S As String) As Int32
 Declare Function ENreport Lib "epanet2.dll" () As Int32
 Declare Function ENresetreport Lib "epanet2.dll" () As Int32
 Declare Function ENsetreport Lib "epanet2.dll" (ByVal S As String) As Int32
 
 Declare Function ENgetcontrol Lib "epanet2.dll" (ByVal Cindex As Int32, ByRef CtlType As Int32, ByRef Lindex As Int32, ByRef Setting As Single, ByRef Nindex As Int32, ByRef Level As Single) As Int32
 Declare Function ENgetcount Lib "epanet2.dll" (ByVal Code As Int32, ByRef Value As Int32) As Int32
 Declare Function ENgetoption Lib "epanet2.dll" (ByVal Code As Int32, ByRef Value As Single) As Int32
 Declare Function ENgettimeparam Lib "epanet2.dll" (ByVal Code As Int32, ByRef Value As Int32) As Int32
 Declare Function ENgetflowunits Lib "epanet2.dll" (ByRef Code As Int32) As Int32
 Declare Function ENgetpatternindex Lib "epanet2.dll" (ByVal ID As String, ByRef Index As Int32) As Int32
 Declare Function ENgetpatternid Lib "epanet2.dll" (ByVal Index As Int32, ByVal ID As StringBuilder) As Int32
 Declare Function ENgetpatternlen Lib "epanet2.dll" (ByVal Index As Int32, ByRef L As Int32) As Int32
 Declare Function ENgetpatternvalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Period As Int32, ByRef Value As Single) As Int32
 Declare Function ENgetqualtype Lib "epanet2.dll" (ByRef QualCode As Int32, ByRef TraceNode As Int32) As Int32
 Declare Function ENgeterror Lib "epanet2.dll" (ByVal ErrCode As Int32, ByVal ErrMsg As StringBuilder, ByVal N As Int32)

 Declare Function ENgetnodeindex Lib "epanet2.dll" (ByVal ID As String, ByRef Index As Int32) As Int32
 Declare Function ENgetnodeid Lib "epanet2.dll" (ByVal Index As Int32, ByVal ID As StringBuilder) As Int32
 Declare Function ENgetnodetype Lib "epanet2.dll" (ByVal Index As Int32, ByRef Code As Int32) As Int32
 Declare Function ENgetnodevalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Code As Int32, ByRef Value As Single) As Int32
 Declare Function ENgetcoord Lib "epanet2.dll" (ByVal Index As Int32, ByRef X As Single, ByRef Y As Single) As Int32
 Declare Function ENsetcoord Lib "epanet2.dll" (ByVal Index As Int32, ByVal X As Single, ByVal Y As Single) As Int32

 Declare Function ENgetnumdemands Lib "epanet2.dll" (ByVal Index As Int32, ByRef numDemands As Int32) As Int32   'ES
 Declare Function ENgetbasedemand Lib "epanet2.dll" (ByVal Index As Int32, ByVal DemandIndex As Int32, ByRef Value As Single) As Int32   'ES
 Declare Function ENgetdemandpattern Lib "epanet2.dll" (ByVal Index As Int32, ByVal DemandIndex As Int32, ByRef PatIndex As Int32) As Int32   'ES

 Declare Function ENgetlinkindex Lib "epanet2.dll" (ByVal ID As String, ByRef Index As Int32) As Int32
 Declare Function ENgetlinkid Lib "epanet2.dll" (ByVal Index As Int32, ByVal ID As StringBuilder) As Int32
 Declare Function ENgetlinktype Lib "epanet2.dll" (ByVal Index As Int32, ByRef Code As Int32) As Int32
 Declare Function ENgetlinknodes Lib "epanet2.dll" (ByVal Index As Int32, ByRef Node1 As Int32, ByRef Node2 As Int32) As Int32
 Declare Function ENgetlinkvalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Code As Int32, ByRef Value As Single) As Int32

 Declare Function ENgetcurve Lib "epanet2.dll" (ByVal CurveIndex As Int32, ByRef nValues As Int32, ByRef xValues As Single, ByRef yValues As Single) As Int32 'ES
 Declare Function ENgetheadcurveindex Lib "epanet2.dll" (ByVal Index As Int32, ByVal CurveIndex As int32) As Int32  'ES
 Declare Function ENgetpumptype Lib "epanet2.dll" (ByVal Index As Int32, ByRef PumpType As Int32) As Int32  'ES

 Declare Function ENgetversion Lib "epanet2.dll" (ByRef Value As Int32) As Int32

 Declare Function ENsetcontrol Lib "epanet2.dll" (ByVal Cindex As Int32, ByVal CtlType As Int32, ByVal Lindex As Int32, ByVal Setting As Single, ByVal Nindex As Int32, ByVal Level As Single) As Int32
 Declare Function ENsetnodevalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Code As Int32, ByVal Value As Single) As Int32
 Declare Function ENsetlinkvalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Code As Int32, ByVal Value As Single) As Int32
 Declare Function ENsetpattern Lib "epanet2.dll" (ByVal Index as Int32, ByRef F as Single, ByVal N as Int32) as Int32
 Declare Function ENsetpatternvalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Period As Int32, ByVal Value As Single) As Int32
 Declare Function ENsettimeparam Lib "epanet2.dll" (ByVal Code As Int32, ByVal Value As Int32) As Int32
 Declare Function ENsetoption Lib "epanet2.dll" (ByVal Code As Int32, ByVal Value As Single) As Int32
 Declare Function ENsetstatusreport Lib "epanet2.dll" (ByVal Code As Int32) As Int32
 Declare Function ENsetqualtype Lib "epanet2.dll" (ByVal QualCode As Int32, ByVal ChemName As String, ByVal ChemUnits As String, ByVal TraceNode As String) As Int32
 
 Declare Function ENaddpattern Lib "epanet2.dll" (ByVal ID As String) As Int32

 Declare Function ENgetcurveindex Lib "epanet2.dll" (ByVal ID As String, ByRef Index As Int32) As Int32
 Declare Function ENgetcurveid Lib "epanet2.dll" (ByVal Index As Int32, ByVal ID As StringBuilder) As Int32
 Declare Function ENgetcurvelen Lib "epanet2.dll" (ByVal Index As Int32, ByRef L As Int32) As Int32
 Declare Function ENgetcurvevalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Pnt As Int32, ByRef X As Single, ByRef Y As Single) As Int32 
 Declare Function ENsetcurvevalue Lib "epanet2.dll" (ByVal Index As Int32, ByVal Pnt As Int32, ByVal X As Single, ByVal Y As Single) As Int32
 Declare Function ENsetcurve Lib "epanet2.dll" (ByVal Index as Int32, ByRef X as Single, ByRef Y as Single, ByVal N as Int32) as Int32 
 Declare Function ENaddcurve Lib "epanet2.dll" (ByVal ID As String) As Int32
 
End Module
