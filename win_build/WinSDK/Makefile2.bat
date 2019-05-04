rem : set the path to CMAKE
SET CMAKE_PATH=cmake.exe 

SET Build_PATH=%CD%

rem : set genarator
SET GENERATOR=Visual Studio 15 2017

rem : the directory where the program will be compiled to
SET COMPILE_PATH_WIN64TMP=%Build_PATH%\tmp64\
SET COMPILE_PATH_WIN64=%Build_PATH%\64bit\

rem : the directory where the program will be compiled to
SET COMPILE_PATH_WIN32TMP=%Build_PATH%\tmp32\
SET COMPILE_PATH_WIN32=%Build_PATH%\32bit\

rem : CMAKE the root directory of the EPANET project 
rem : 64 bit
MKDIR "%COMPILE_PATH_WIN64TMP%"
CD "%COMPILE_PATH_WIN64TMP%"
MKDIR "%COMPILE_PATH_WIN64%"
%CMAKE_PATH% -G "%GENERATOR% Win64" ../../../
rem : %CMAKE_PATH% --build . --config Debug
%CMAKE_PATH% --build . --config Release

XCOPY "%COMPILE_PATH_WIN64TMP%bin\Release\epanet2.dll" "%COMPILE_PATH_WIN64%epanet2.dll*" /y
XCOPY "%COMPILE_PATH_WIN64TMP%bin\Release\runepanet.exe" "%COMPILE_PATH_WIN64%epanet2.exe*" /y

rem : CMAKE the root directory of the EPANET project 
rem : 32 bit
MKDIR "%COMPILE_PATH_WIN32TMP%"
CD "%COMPILE_PATH_WIN32TMP%"
MKDIR "%COMPILE_PATH_WIN32%"
%CMAKE_PATH% -G "%GENERATOR%" ../../../
rem : %CMAKE_PATH% --build . --config Debug
%CMAKE_PATH% --build . --config Release

XCOPY "%COMPILE_PATH_WIN32TMP%bin\Release\epanet2.dll" "%COMPILE_PATH_WIN32%epanet2.dll*" /y
XCOPY "%COMPILE_PATH_WIN32TMP%bin\Release\runepanet.exe" "%COMPILE_PATH_WIN32%epanet2.exe*" /y

CD "%Build_PATH%"

rem : cleaning
RMDIR /s /q "%COMPILE_PATH_WIN64TMP%"
RMDIR /s /q "%COMPILE_PATH_WIN32TMP%"
