rem : set path to Windows SDK
Set Reg.Key=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows
Set Reg.Val=CurrentInstallFolder
Set Build_PATH=%CD%

For /F "Tokens=2*" %%A In ('Reg Query "%Reg.Key%" /v "%Reg.Val%" ^| Find /I "%Reg.Val%"' ) Do Call Set SDK_PATH=%%B

rem : check pc architecture (32 or 64-bit)
Set Reg.Qry=HKLM\Hardware\Description\System\CentralProcessor\0
REG.exe Query %Reg.Qry% > checkOS.tmp
Find /i "x86" < checkOS.tmp > StringCheck.tmp
 
If %ERRORLEVEL% == 0 (
	CALL "%SDK_PATH%bin\"SetEnv.cmd /x86 /release
) ELSE (
	CALL "%SDK_PATH%bin\"SetEnv.cmd /x64 /release
)

rem : set path for source EPANET files
cd ..\..\src
Set SRC_PATH=%CD%
del "%SRC_PATH%"\*.dll
del "%SRC_PATH%"\*.exe

rem : do the magic ...
  rem : create EPANET2.DLL
  cl -o epanet2.dll epanet.c hash.c hydraul.c inpfile.c input1.c input2.c input3.c mempool.c output.c quality.c report.c rules.c smatrix.c /I ..\include /I ..\run /link /DLL 
  rem : create EPANET2.EXE
  cl -o epanet2.exe epanet.c ..\run\main.c hash.c hydraul.c inpfile.c input1.c input2.c input3.c mempool.c output.c quality.c report.c rules.c smatrix.c /I ..\include /I ..\run /I ..\src /link

rem : a bit of housekeeping and cleaning
echo "%SRC_PATH%"
del "%SRC_PATH%"\*.obj
del "%SRC_PATH%"\*.exp
del "%SRC_PATH%"\*.lib
del "%SRC_PATH%"\*.map
del "%Build_PATH%"\*.tmp

C:
IF EXIST "%Build_PATH%"\bin GOTO BINEXIST
	md "%Build_PATH%"\bin
	echo "BIN folder created"
:BINEXIST

move /y "%SRC_PATH%"\*.dll "%Build_PATH%"\bin
move /y "%SRC_PATH%"\*.exe "%Build_PATH%"\bin
cd "%Build_PATH%"
