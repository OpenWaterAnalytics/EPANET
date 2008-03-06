# Microsoft Developer Studio Generated NMAKE File, Based on epanet2.dsp
!IF "$(CFG)" == ""
CFG=epanet2 - Win32 Release
!MESSAGE No configuration specified. Defaulting to epanet2 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "epanet2 - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "epanet2.mak" CFG="epanet2 - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "epanet2 - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\epanet2.dll"


CLEAN :
	-@erase "$(INTDIR)\epanet.obj"
	-@erase "$(INTDIR)\hash.obj"
	-@erase "$(INTDIR)\hydraul.obj"
	-@erase "$(INTDIR)\inpfile.obj"
	-@erase "$(INTDIR)\input1.obj"
	-@erase "$(INTDIR)\input2.obj"
	-@erase "$(INTDIR)\input3.obj"
	-@erase "$(INTDIR)\mempool.obj"
	-@erase "$(INTDIR)\output.obj"
	-@erase "$(INTDIR)\quality.obj"
	-@erase "$(INTDIR)\report.obj"
	-@erase "$(INTDIR)\rules.obj"
	-@erase "$(INTDIR)\smatrix.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\epanet2.dll"
	-@erase "$(OUTDIR)\epanet2.exp"
	-@erase "$(OUTDIR)\epanet2.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MT /W4 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "EPANET2_EXPORTS" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\epanet2.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /incremental:no /pdb:"$(OUTDIR)\epanet2.pdb" /machine:I386 /def:"..\epanet2.def" /out:"$(OUTDIR)\epanet2.dll" /implib:"$(OUTDIR)\epanet2.lib" 
DEF_FILE= \
	"..\epanet2.def"
LINK32_OBJS= \
	"$(INTDIR)\epanet.obj" \
	"$(INTDIR)\hash.obj" \
	"$(INTDIR)\hydraul.obj" \
	"$(INTDIR)\inpfile.obj" \
	"$(INTDIR)\input1.obj" \
	"$(INTDIR)\input2.obj" \
	"$(INTDIR)\input3.obj" \
	"$(INTDIR)\mempool.obj" \
	"$(INTDIR)\output.obj" \
	"$(INTDIR)\quality.obj" \
	"$(INTDIR)\report.obj" \
	"$(INTDIR)\rules.obj" \
	"$(INTDIR)\smatrix.obj"

"$(OUTDIR)\epanet2.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("epanet2.dep")
!INCLUDE "epanet2.dep"
!ELSE 
!MESSAGE Warning: cannot find "epanet2.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "epanet2 - Win32 Release"
SOURCE=..\epanet.c

"$(INTDIR)\epanet.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\hash.c

"$(INTDIR)\hash.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\hydraul.c

"$(INTDIR)\hydraul.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\inpfile.c

"$(INTDIR)\inpfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\input1.c

"$(INTDIR)\input1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\input2.c

"$(INTDIR)\input2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\input3.c

"$(INTDIR)\input3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\mempool.c

"$(INTDIR)\mempool.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\output.c

"$(INTDIR)\output.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\quality.c

"$(INTDIR)\quality.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\report.c

"$(INTDIR)\report.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\rules.c

"$(INTDIR)\rules.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=..\smatrix.c

"$(INTDIR)\smatrix.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 

