Attribute VB_Name = "basShellSync"
Option Explicit

Private Const INFINITE = &HFFFFFFFF
Private Const SYNCHRONIZE = &H100000
Private Const PROCESS_QUERY_INFORMATION = &H400&

Private Declare Function CloseHandle Lib "kernel32" (ByVal hObject As Long) As Long
Private Declare Function GetExitCodeProcess Lib "kernel32" (ByVal hProcess As Long, lpExitCode As Long) As Long
Private Declare Function OpenProcess Lib "kernel32" (ByVal dwDesiredAccess As Long, ByVal bInheritHandle As Long, ByVal dwProcessId As Long) As Long
Private Declare Function WaitForSingleObject Lib "kernel32" (ByVal hHandle As Long, ByVal dwMilliseconds As Long) As Long
Public Function ShellSync(ByVal PathName As String, ByVal WindowStyle As VbAppWinStyle) As Long
'-----------------------------------------------------------------------------------------------
'Shell and wait.  Return exit code result, raise an
'exception on any error.
Dim lngPid As Long
Dim lngHandle As Long
Dim lngExitCode As Long

lngPid = Shell(PathName, WindowStyle)
If lngPid <> 0 Then
    lngHandle = OpenProcess(SYNCHRONIZE Or PROCESS_QUERY_INFORMATION, 0, lngPid)
    If lngHandle <> 0 Then
        WaitForSingleObject lngHandle, INFINITE
        If GetExitCodeProcess(lngHandle, lngExitCode) <> 0 Then
            ShellSync = lngExitCode
            CloseHandle lngHandle
        Else
            CloseHandle lngHandle
            Err.Raise &H8004AA00, "ShellSync", "Failed to retrieve exit code, error " & CStr(Err.LastDllError)
        End If
    Else
        Err.Raise &H8004AA01, "ShellSync", "Failed to open child process"
    End If
Else
    Err.Raise &H8004AA02, "ShellSync", "Failed to Shell child process"
End If

End Function
