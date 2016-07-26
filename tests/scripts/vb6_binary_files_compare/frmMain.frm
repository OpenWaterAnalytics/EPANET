VERSION 5.00
Begin VB.Form frmMain 
   Caption         =   "Form1"
   ClientHeight    =   5304
   ClientLeft      =   48
   ClientTop       =   396
   ClientWidth     =   9648
   LinkTopic       =   "Form1"
   ScaleHeight     =   5304
   ScaleWidth      =   9648
   StartUpPosition =   3  'Windows Default
   Begin VB.ListBox List1 
      Height          =   4272
      Left            =   1680
      TabIndex        =   1
      Top             =   360
      Width           =   7092
   End
   Begin VB.CommandButton cmdRun 
      Caption         =   "RUN"
      Height          =   372
      Left            =   360
      TabIndex        =   0
      Top             =   360
      Width           =   972
   End
End
Attribute VB_Name = "frmMain"
Attribute VB_GlobalNameSpace = False
Attribute VB_Creatable = False
Attribute VB_PredeclaredId = True
Attribute VB_Exposed = False
Option Explicit

Dim NetName As String

Dim net1 As typNetRes
Dim net2 As typNetRes

Private Sub CompareVersions(NetName As String)
'-----------------------------------------------
Dim i As Long
Dim v1 As Single, v2 As Single
Dim T As Long
Dim L As Long
Dim maxDiff As Single
Dim diff As Single
Dim nRep As Long
Dim F As Long

i = ShellSync("epanet2d.exe nets\" & NetName & ".inp nets\" & NetName & "_1.rep nets\" & NetName & "_1.out", vbNormalFocus)
i = ReadOutputFile("nets\" & NetName & "_1.out", net1)

i = ShellSync("epanet2.exe nets\" & NetName & ".inp nets\" & NetName & "_2.rep nets\" & NetName & "_2.out", vbNormalFocus)
i = ReadOutputFile("nets\" & NetName & "_2.out", net2)

F = FreeFile
Open App.Path & "\Nets\" & NetName & ".dif" For Output As #F

nRep = UBound(net1.LinksFlow, 2)

maxDiff = 100: diff = 101
For L = 1 To net1.nLinks
    For T = 1 To nRep
        v1 = net1.LinksFlow(L, T)
        v2 = net2.LinksFlow(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2)) / Log(10))
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Links Flow max diff = " & Format(maxDiff, "0")

maxDiff = 100: diff = 101
For L = 1 To net1.nLinks
    For T = 1 To nRep
        v1 = net1.LinksHeadloss(L, T)
        v2 = net2.LinksHeadloss(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2)) / Log(10))
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Links Headloss max diff = " & Format(maxDiff, "0")

maxDiff = 100: diff = 101
For L = 1 To net1.nLinks
    For T = 1 To nRep
        v1 = net1.LinksQuality(L, T)
        v2 = net2.LinksQuality(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2)) / Log(10))
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Links Quality max diff = " & Format(maxDiff, "0")

maxDiff = 100: diff = 101
For L = 1 To net1.nLinks
    For T = 1 To nRep
        v1 = net1.LinksVelocity(L, T)
        v2 = net2.LinksVelocity(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2)) / Log(10))
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Links Velocity max diff = " & Format(maxDiff, "0")

maxDiff = 100: diff = 101
For L = 1 To net1.nNodes
    For T = 1 To nRep
        v1 = net1.NodesDemand(L, T)
        v2 = net2.NodesDemand(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2)) / Log(10))
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Nodes Demand max diff = " & Format(maxDiff, "0")

maxDiff = 100: diff = 101
For L = 1 To net1.nNodes
    For T = 1 To nRep
        v1 = net1.NodesHead(L, T)
        v2 = net2.NodesHead(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2)) / Log(10))
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Nodes Head max diff = " & Format(maxDiff, "0")

maxDiff = 100: diff = 101
For L = 1 To net1.nNodes
    For T = 1 To nRep
        v1 = net1.NodesPressure(L, T)
        v2 = net2.NodesPressure(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2)) / Log(10))
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Nodes Pressure max diff = " & Format(maxDiff, "0")

maxDiff = 100: diff = 101
For L = 1 To net1.nNodes
    For T = 1 To nRep
        v1 = net1.NodesQuality(L, T)
        v2 = net2.NodesQuality(L, T)
        If Abs(v1 - v2) > 0 Then diff = -Round(Log(Abs(v1 - v2))) / Log(10)
        If diff < maxDiff Then maxDiff = diff
    Next T
Next L
Print #F, "Nodes Quality max diff = " & Format(maxDiff, "0")

Close #F

End Sub

Private Sub cmdRun_Click()
'----------------------------------------------
Dim fName As String

fName = Dir(App.Path & "\Nets\*.inp")
If fName <> "" Then
    Do
        List1.AddItem fName: DoEvents
        fName = StrReverse(fName)
        fName = Mid(fName, 5)
        fName = StrReverse(fName)
        Call CompareVersions(fName)
        fName = Dir
    Loop Until fName = ""
End If

End Sub


Private Sub Form_Load()

List1.Clear

End Sub


