rem : set path to Windows SDK
SET SDK_PATH="C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\"

rem : set path for source EPANET files
SET SRC_PATH="D:\Projects\EPANET\Open Source\Code\owa\EPANET\src"

CALL %SDK_PATH%SetEnv.cmd /x86 /release
CHDIR /D %SRC_PATH%

rem : do the magic ...
cl -o epanet2.exe ..\run\main.c /I ..\include /I ..\run /I ..\src /link