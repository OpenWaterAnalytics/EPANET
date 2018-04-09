Attribute VB_Name = "basNetRes"
Public Type typNetRes
    nNodes As Long
    nLinks As Long
    
    NodesID() As String
    LinksID() As String
    
    NodesDemand() As Single
    NodesHead() As Single
    NodesPressure() As Single
    NodesQuality() As Single

    LinksFlow() As Single
    LinksHeadloss() As Single
    LinksQuality() As Single
    LinksSetting() As Single
    LinksStatus() As Single
    LinksVelocity() As Single

End Type
Public Function ReadOutputFile(OutFile As String, NetRes As typNetRes)
'---------------------------------------------------------------------------------------
Dim i As Long, F As Long, r As Long, nRep As Long, j As Long
Dim tmpArr() As Single
Dim pos As Long, N As Long
Dim tmpLong As Long, tmpSingle As Single
Dim ReportingTimeStep As Long, ReportingStartTime As Long
Dim SimulationDuration As Long
Dim ProblemTitletLine As String * 80
Dim NumberOfNodes As Long
Dim NumberOfLinks As Long
Dim NumberOfReservoirsAndTanks As Long
Dim NumberOfPumps As Long
Dim NumberOfValves As Long
Dim WaterQualityOption As Long
Dim FlowUnitsOption As Long, PressureUnitsOption As Long
Dim NameOfFile As String * 260, tmpString32 As String * 32
Dim ii As Long
Dim WarningFlag As Long

With NetRes
    pos = 1: N = 0
    F = FreeFile
    Open OutFile For Binary As #F
    Get #F, pos, tmpLong: pos = pos + 4
    Get #F, pos, tmpLong: pos = pos + 4
    Get #F, pos, NumberOfNodes: pos = pos + 4
    Get #F, pos, NumberOfReservoirsAndTanks: pos = pos + 4
    Get #F, pos, NumberOfLinks: pos = pos + 4
    Get #F, pos, NumberOfPumps: pos = pos + 4
    Get #F, pos, NumberOfValves: pos = pos + 4
    Get #F, pos, WaterQualityOption: pos = pos + 4
    Get #F, pos, tmpLong: pos = pos + 4
    Get #F, pos, FlowUnitsOption: pos = pos + 4
    Get #F, pos, PressureUnitsOption: pos = pos + 4
    Get #F, pos, tmpLong: pos = pos + 4
    Get #F, pos, ReportingStartTime: pos = pos + 4
    Get #F, pos, ReportingTimeStep: pos = pos + 4
    Get #F, pos, SimulationDuration: pos = pos + 4
    Get #F, pos, ProblemTitletLine: pos = pos + 80
    Get #F, pos, ProblemTitletLine: pos = pos + 80
    Get #F, pos, ProblemTitletLine: pos = pos + 80
    Get #F, pos, NameOfFile: pos = pos + 260
    Get #F, pos, NameOfFile: pos = pos + 260
    Get #F, pos, tmpString32: pos = pos + 32
    Get #F, pos, tmpString32: pos = pos + 32
    .nNodes = NumberOfNodes
    .nLinks = NumberOfLinks
    ReDim .NodesID(.nNodes)
    For j = 1 To NumberOfNodes
        Get #F, pos, tmpString32: pos = pos + 32
        i = InStr(1, tmpString32, Chr(0))
        .NodesID(j) = Mid(tmpString32, 1, i - 1)
    Next j
    ReDim .LinksID(.nLinks)
    For j = 1 To NumberOfLinks
        Get #F, pos, tmpString32: pos = pos + 32
        i = InStr(1, tmpString32, Chr(0))
        .LinksID(j) = Mid(tmpString32, 1, i - 1)
    Next j
    For j = 1 To NumberOfLinks * 3 'Index of Start Node of Each Link + Index of End Node of Each Link + Type Code of Each Link
        Get #F, pos, tmpLong: pos = pos + 4
    Next j
    For j = 1 To NumberOfReservoirsAndTanks 'Node Index of Each Tank
        Get #F, pos, tmpLong: pos = pos + 4
    Next j
    For j = 1 To NumberOfReservoirsAndTanks 'Cross-Sectional Area of Each Tank
        Get #F, pos, tmpSingle: pos = pos + 4
    Next j
    For j = 1 To NumberOfNodes 'Elevation of Each Node
        Get #F, pos, tmpSingle: pos = pos + 4
    Next j
    For j = 1 To NumberOfLinks 'Length of Each Link
        Get #F, pos, tmpSingle: pos = pos + 4
    Next j
    For j = 1 To NumberOfLinks 'Diameter of Each Link
        Get #F, pos, tmpSingle: pos = pos + 4
    Next j
    For j = 1 To NumberOfPumps 'Energy Use Section
        Get #F, pos, tmpLong: pos = pos + 4 'Pump Index in List of Links
        Get #F, pos, tmpSingle: pos = pos + 4 'Pump Utilization (%)
        Get #F, pos, tmpSingle: pos = pos + 4 'Average Efficiency (%)
        Get #F, pos, tmpSingle: pos = pos + 4 'Average Kwatts/Million Gallons (/Meter3)
        Get #F, pos, tmpSingle: pos = pos + 4 'Average Kwatts
        Get #F, pos, tmpSingle: pos = pos + 4 'Peak Kwatts
        Get #F, pos, tmpSingle: pos = pos + 4 'Average Cost Per Day
    Next j
    Get #F, pos, tmpSingle: pos = pos + 4 'Overall Peak Energy Usage
    
    'reporting
    nRep = SimulationDuration / ReportingTimeStep + 1
    ReDim tmpArr(nRep)
    ReDim .NodesDemand(.nNodes, nRep)
    ReDim .NodesHead(.nNodes, nRep)
    ReDim .NodesPressure(.nNodes, nRep)
    ReDim .NodesQuality(.nNodes, nRep)

    ReDim .LinksFlow(.nLinks, nRep)
    ReDim .LinksHeadloss(.nLinks, nRep)
    ReDim .LinksQuality(.nLinks, nRep)
    ReDim .LinksSetting(.nLinks, nRep)
    ReDim .LinksStatus(.nLinks, nRep)
    ReDim .LinksVelocity(.nLinks, nRep)

    For r = 1 To nRep
        For j = 1 To NumberOfNodes 'Demand at Each Node
            Get #F, pos, tmpSingle: pos = pos + 4
            .NodesDemand(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfNodes 'Hydraulic Head at Each Node
            Get #F, pos, tmpSingle: pos = pos + 4
            .NodesHead(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfNodes 'Pressure at Each Node
            Get #F, pos, tmpSingle: pos = pos + 4
            .NodesPressure(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfNodes 'Water Quality at Each Node
            Get #F, pos, tmpSingle: pos = pos + 4
            .NodesPressure(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfLinks 'Flow in Each Link
            Get #F, pos, tmpSingle: pos = pos + 4
            .LinksFlow(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfLinks 'Velocity in Each Link
            Get #F, pos, tmpSingle: pos = pos + 4
            .LinksVelocity(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfLinks 'Headloss per 1000 Units of Length for Each Link
            Get #F, pos, tmpSingle: pos = pos + 4
            .LinksHeadloss(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfLinks 'Average Water Quality in Each Link
            Get #F, pos, tmpSingle: pos = pos + 4
            .LinksQuality(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfLinks 'Status Code for Each Link
            Get #F, pos, tmpSingle: pos = pos + 4
            .LinksStatus(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfLinks 'Setting for Each Link
            Get #F, pos, tmpSingle: pos = pos + 4
            .LinksSetting(j, r) = tmpSingle
        Next j
        For j = 1 To NumberOfLinks 'Reaction Rate for Each Link (mass/L/day)
            Get #F, pos, tmpSingle: pos = pos + 4
        Next j
        For j = 1 To NumberOfLinks 'Friction Factor for Each Link
            Get #F, pos, tmpSingle: pos = pos + 4
        Next j
    Next r
    
    'Epilog Section
    Get #F, pos, tmpSingle: pos = pos + 4 'Average bulk reaction rate (mass/hr)
    Get #F, pos, tmpSingle: pos = pos + 4 'Average wall reaction rate (mass/hr)
    Get #F, pos, tmpSingle: pos = pos + 4 'Average tank reaction rate (mass/hr)
    Get #F, pos, tmpSingle: pos = pos + 4 'Average source inflow rate (mass/hr)
    Get #F, pos, tmpLong: pos = pos + 4 'Number of Reporting Periods
    Get #F, pos, WarningFlag: pos = pos + 4 'Warning Flag
    Get #F, pos, tmpLong: pos = pos + 4 'Magic Number ( = 516114521)
    
    Close #F
End With

End Function

