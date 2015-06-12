rem : set path to Windows SDK
SET SDK_PATH="C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\"

rem : set path for source EPANET files
SET SRC_PATH="D:\Projects\EPANET\Open Source\Code\owa\EPANET\src"

CALL %SDK_PATH%SetEnv.cmd /x86 /release
CHDIR /D %SRC_PATH%
del %SRC_PATH%\*.dll
del %SRC_PATH%\*.exe

rem : do the magic ...
  rem : creat EPANET2.DLL
  cl -o epanet2.dll epanet.c hash.c hydraul.c inpfile.c input1.c input2.c input3.c mempool.c output.c quality.c report.c rules.c smatrix.c /I ..\include /I ..\run /link /DLL /def:epanet2.def /MAP
  rem : creat EPANET2.EXE
  cl -o epanet2.exe epanet.c ..\run\main.c hash.c hydraul.c inpfile.c input1.c input2.c input3.c mempool.c output.c quality.c report.c rules.c smatrix.c /I ..\include /I ..\run /I ..\src /link

rem : a bit of housekeeping and cleaning
del %SRC_PATH%\*.obj
del %SRC_PATH%\*.exp
del %SRC_PATH%\*.lib
del %SRC_PATH%\*.map
