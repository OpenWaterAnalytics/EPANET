rem : set path to Windows SDK
Set Reg.Key=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows
Set Reg.Val=CurrentInstallFolder
Set Build_PATH=%CD%

rem : set path for source EPANET files
cd ..\..\include
Set H_PATH=%CD%
cd ..\src
Set SRC_PATH=%CD%

rem : set Windows SDK Path from registry
For /F "Tokens=2*" %%A In ('Reg Query "%Reg.Key%" /v "%Reg.Val%" ^| Find /I "%Reg.Val%"' ) Do Call Set SDK_PATH=%%B


rem : 64 bit
rem : check pc architecture
Set Reg.Qry=HKLM\Hardware\Description\System\CentralProcessor\0
REG.exe Query %Reg.Qry% > checkOS.tmp
Find /i "x86" < checkOS.tmp > StringCheck.tmp
If %ERRORLEVEL% == 1 (
	CALL "%SDK_PATH%bin\"SetEnv.cmd /x64 /release
	rem : create EPANET2.DLL
	cl -o epanet2.dll epanet.c epanet2.c hash.c hydraul.c hydcoeffs.c hydstatus.c hydsolver.c inpfile.c input1.c input2.c input3.c mempool.c output.c project.c quality.c qualroute.c qualreact.c report.c rules.c smatrix.c genmmd.c /Depanet2_EXPORTS /I ..\include /I ..\run /link /DLL
	rem : create EPANET2.EXE
	cl -o epanet2.exe epanet.c epanet2.c ..\run\main.c hash.c hydraul.c hydcoeffs.c hydstatus.c hydsolver.c inpfile.c input1.c input2.c input3.c mempool.c output.c project.c quality.c qualroute.c qualreact.c report.c rules.c smatrix.c genmmd.c /Depanet2_EXPORTS /I ..\include /I ..\run /I ..\src /link
	md "%Build_PATH%"\64bit
	move /y "%SRC_PATH%"\*.dll "%Build_PATH%"\64bit
	move /y "%SRC_PATH%"\*.exe "%Build_PATH%"\64bit
	rem copy "%H_PATH%"\*.h "%Build_PATH%"\64bit
)


rem : 32 bit with DEF
CALL "%SDK_PATH%bin\"SetEnv.cmd /x86 /release
echo "32 bit with epanet2.def mapping"
rem : create EPANET2.DLL
cl -o epanet2.dll epanet.c epanet2.c hash.c hydraul.c hydcoeffs.c hydstatus.c hydsolver.c inpfile.c input1.c input2.c input3.c mempool.c output.c project.c quality.c qualroute.c qualreact.c report.c rules.c smatrix.c genmmd.c /Depanet2_EXPORTS /I ..\include /I ..\run /link /DLL /def:..\win_build\WinSDK\epanet2.def /MAP
rem : create EPANET2.EXE
cl -o epanet2.exe epanet.c epanet2.c ..\run\main.c hash.c hydraul.c hydcoeffs.c hydstatus.c hydsolver.c inpfile.c input1.c input2.c input3.c mempool.c output.c project.c quality.c qualroute.c qualreact.c report.c rules.c smatrix.c genmmd.c /Depanet2_EXPORTS /I ..\include /I ..\run /I ..\src /link
md "%Build_PATH%"\32bit
move /y "%SRC_PATH%"\*.dll "%Build_PATH%"\32bit
move /y "%SRC_PATH%"\*.exe "%Build_PATH%"\32bit


rem : a bit of housekeeping and cleaning
del "%SRC_PATH%"\*.obj
del "%SRC_PATH%"\*.exp
del "%SRC_PATH%"\*.lib
del "%SRC_PATH%"\*.map
del "%SRC_PATH%"\*.tmp

cd "%Build_PATH%"
