rem : set path to Windows SDK
Set Reg.Key=HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows
Set Reg.Val=CurrentInstallFolder

For /F "Tokens=2*" %%A In ('Reg Query "%Reg.Key%" /v "%Reg.Val%" ^| Find /I "%Reg.Val%"' ) Do Call Set SDK_PATH=%%B
echo %SDK_PATH%


CALL "%SDK_PATH%bin\"SetEnv.cmd /x64 /release

rem : set path for source EPANET files
cd ..\..\src
set SRC_PATH=%CD%

del %SRC_PATH%\*.dll
del %SRC_PATH%\*.exe

rem : do the magic ...
  rem : create EPANET2.DLL
  cl -o epanet2.dll epanet.c hash.c hydraul.c inpfile.c input1.c input2.c input3.c mempool.c output.c quality.c report.c rules.c smatrix.c /I ..\include /I ..\run /link /DLL 
  rem : create EPANET2.EXE
  cl -o epanet2.exe epanet.c ..\run\main.c hash.c hydraul.c inpfile.c input1.c input2.c input3.c mempool.c output.c quality.c report.c rules.c smatrix.c /I ..\include /I ..\run /I ..\src /link

rem : a bit of housekeeping and cleaning
del %SRC_PATH%\*.obj
del %SRC_PATH%\*.exp
del %SRC_PATH%\*.lib
del %SRC_PATH%\*.map