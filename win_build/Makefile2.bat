rem : rem : Compilation script (Visual Studio 2017/2019 with CMAKE)

rem : set the path to CMAKE
SET CMAKE_PATH=cmake.exe 

SET Build_PATH=%CD%

rem : the directory where the program will be compiled to
SET COMPILE_PATH_TMP=%Build_PATH%\tmp\
SET COMPILE_PATH_WIN64=%Build_PATH%\64bit\
SET COMPILE_PATH_WIN32=%Build_PATH%\32bit\

rem : CMAKE the root directory of the EPANET project 
rem : 32 bit
MKDIR "%COMPILE_PATH_TMP%"
CD "%COMPILE_PATH_TMP%"
%CMAKE_PATH% ../../ -A Win32
%CMAKE_PATH% --build . --config Release

MKDIR "%COMPILE_PATH_WIN32%"
XCOPY "%COMPILE_PATH_TMP%bin\Release\epanet2.dll" "%COMPILE_PATH_WIN32%epanet2.dll*" /y
XCOPY "%COMPILE_PATH_TMP%bin\Release\runepanet.exe" "%COMPILE_PATH_WIN32%runepanet.exe*" /y
XCOPY "%COMPILE_PATH_TMP%lib\Release\epanet2.lib" "%COMPILE_PATH_WIN32%epanet2.lib*" /y

CD "%Build_PATH%"
RMDIR /s /q "%COMPILE_PATH_TMP%"

rem : CMAKE the root directory of the EPANET project 
rem : 64 bit
MKDIR "%COMPILE_PATH_TMP%"
CD "%COMPILE_PATH_TMP%"
%CMAKE_PATH% ../../ -A x64
%CMAKE_PATH% --build . --config Release

MKDIR "%COMPILE_PATH_WIN64%"
XCOPY "%COMPILE_PATH_TMP%bin\Release\epanet2.dll" "%COMPILE_PATH_WIN64%epanet2.dll*" /y
XCOPY "%COMPILE_PATH_TMP%bin\Release\runepanet.exe" "%COMPILE_PATH_WIN64%runepanet.exe*" /y
XCOPY "%COMPILE_PATH_TMP%lib\Release\epanet2.lib" "%COMPILE_PATH_WIN64%epanet2.lib*" /y

CD "%Build_PATH%"
RMDIR /s /q "%COMPILE_PATH_TMP%"

