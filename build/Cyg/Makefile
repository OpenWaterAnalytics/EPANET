# Cygwin/MinGW Makefile for EPANET

# This will build EPANET as a native windows DLL and import
# library (epanet_gcc_<Rev>.dll/libepanet_gcc_<Rev>.dll.a) under Cygwin/MinGW,
# and a standalone executable (epanet_gcc_<Rev>.exe).
# <Rev> is the atomic revision number of the EPANET SVN repo,
# so the results of each build can be unambiguously tracked to a repo Rev.
# This makefile also creates a windows batch file
# (see MScmdname below) that will produce an import
# library for linking the DLL using MSVC.
# This makefile assumes a Cygwin environment with minimal
# MinGW installation, and uses the MinGW gcc compiler from
# the MinGW install directory.
# The current release of MinGW can be obtained from
# http://sourceforge.net/project/showfiles.php?group_id=2435
# The current release of the Cygwin environment can be
# obtained from http://www.cygwin.com

# The following targets are defined (for execution in
# the build directory under the Cygwin environment):
# make
#     -Builds epanet_gcc_<Rev>.dll, epanet_gcc_<Rev>.def, libepanet_gcc_<Rev>.dll.a,
#      epanet_gcc_<Rev>.exe
# make install
#     -Creates shell wrappers runepanet_<Rev>.sh and CreateMSLib_<Rev>.bat that
#      execute epanet_gcc_<Rev>.exe, and MSVC 'lib' (to create an MSVC
#      import library).  The runepanet_<Rev>.sh wrapper simply exports
#      environment variables so that the DLL is found at runtime,
#      allowing you to specify your own locations for installing
#      the DLL.
#     -Installs epanet_gcc_<Rev>.dll, epanet_gcc_<Rev>.exe, and runepanet_<Rev>.sh
#      in <prefix>/bin, where <prefix> defaults to ~ (and can be set
#      below to something different - see "Install directories")
#     -Installs libepanet_gcc_<Rev>.dll.a (import library), epanet_gcc_<Rev>.def,
#      and CreateMSLib-<Rev>.bat in <prefix>/lib
#     -Installs epanet2.h in <prefix>/include.  This is the required
#      header file for the EPANET programmer's toolkit, and thus
#      <prefix>/include should be on your CPP include search path
#      for subsequent use of the toolkit and linking with the import
#      library libepanet_gcc_<Rev>.dll.a.
# make clean
#     -Removes object and library files, returning the build directory
#      to its pristine state.

SHELL = /bin/sh

# C H A N G E  H E R E  A S  N E E D E D
# Change (as needed) the MinGW install directory below.
# Change (as needed) the location off the MS LIB tool.
# You may also wish to change the install path 'prefix',
# or the compiler flags, but these defaults should be fine.

# MinGW top level install directory, accessible from build environment
MinGWdir = /cygdrive/c/MinGW
# Microsoft lib tool directory (for creating an MS import library)
MSVClibexe = c:\\Program Files\\Microsoft Visual Studio .NET 2003\\Vc7\\bin\\lib

# Target filenames
# svnname.sh constructs a name: <prefix><root><Rev><postfix>
# where <Rev> is the atomic revision number of the svn repo.
epanetsvnpath = ../../..
epanetrootname := $(shell ../svnname.sh $(epanetsvnpath) "" epanet_gcc_ "")
dllname := $(epanetrootname).dll
defname := $(epanetrootname).def
implibname = lib$(dllname).a
exename := $(epanetrootname)
# Shell wrapper
runcmdtemplate = runepanet.sh.template
runcmdname = $(shell ../svnname.sh $(epanetsvnpath) "" runepanet_ .sh)
# MSVC import lib batch file
MScmdtemplate = CreateMSLib.bat.template
MScmdname = $(shell ../svnname.sh $(epanetsvnpath) "" CreateMSLib_ .bat)
# Location of EPANET toolkit includes
epanetincludedir = ../../include
# Search path for sources
epanetsrcdir = ../../src
VPATH = $(epanetsrcdir):$(epanetincludedir)

# Install directories
winprefix = $(shell cygpath -w $$HOME)
prefix = ~
exec_prefix = $(prefix)
srcdir = .
libdir = $(exec_prefix)/lib
winlibdir = "$(winprefix)\lib"
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
winincludedir = "$(winprefix)\include"
datarootdir = $(prefix)/share
docdir = $(datarootdir)/doc/epanet

# Compiler and flags
# MinGW gcc
CC = $(MinGWdir)/bin/gcc
dlltool = $(MinGWdir)/bin/dlltool
CFLAGS = -g -O
CPPFLAGS = -I $(srcdir) -I $(epanetincludedir)
LDFLAGS = -L . -W1,-rpath,$(libdir) -lm

# Installer
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# EPANET object files
epanet_objs=hash.o hydraul.o inpfile.o input1.o input2.o \
	    input3.o mempool.o output.o quality.o report.o \
	    rules.o smatrix.o
# Epanet header files
epanet_heads=enumstxt.h funcs.h hash.h mempool.h text.h toolkit.h types.h vars.h
epanet_main_heads=epanet2.h
# Epanet main program
epanet_main=epanet

.PHONY: all
all: $(dllname) $(exename)

$(dllname): $(epanet_objs)
#	$(dlltool) -z $(defname) --dllname $(dllname) --output-lib $(implibname) $^
#	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $(defname) $^
	$(CC) $(CFLAGS) $(CPPFLAGS) -D DLL -c $(epanetsrcdir)/epanet.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ epanet.o $^ -Wl,--output-def,$(defname)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ epanet.o $^ -Wl,--kill-at
	$(dlltool) -d $(defname) --dllname $@ --output-lib $(implibname) --kill-at

$(exename): $(dllname) $(epanet_main_heads)
	$(CC) $(CFLAGS) $(CPPFLAGS) -D CLE -c $(epanetsrcdir)/$(epanet_main).c
	$(CC) $(CFLAGS) -o $@ $(epanet_main).o -l$(exename) $(LDFLAGS)

$(epanet_objs): $(epanet_heads)

.PHONY: install
install:
	cat $(runcmdtemplate) | sed 's|libdir|$(bindir)|' \
                                | sed 's|exename|$(bindir)/$(exename)|' \
                                > $(runcmdname)
	cat $(MScmdtemplate) | sed 's|MSVClibexe|$(MSVClibexe)|' \
                                | sed 's|defname|$(defname)|' \
                                | sed 's|libdir|$(libdir)|' \
                                > $(MScmdname)
	$(INSTALL_PROGRAM) -D $(exename) $(bindir)/$(exename)
	$(INSTALL_PROGRAM) -D $(dllname) $(bindir)/$(dllname)
	$(INSTALL_PROGRAM) -D $(defname) $(libdir)/$(defname)
	$(INSTALL_PROGRAM) -D $(implibname) $(libdir)/$(implibname)
	$(INSTALL_DATA) -D $(epanetincludedir)/epanet2.h $(includedir)/epanet2.h
	$(INSTALL_PROGRAM) -D $(runcmdname) $(bindir)/$(runcmdname)
	$(INSTALL_PROGRAM) -D $(MScmdname) $(libdir)/$(MScmdname)

.PHONY: uninstall
uninstall:

.PHONY: check
check:

.PHONY: clean
clean:
	-/bin/rm *.o $(dllname) $(defname) $(implibname) $(exename).exe $(runcmdname) $(MScmdname)
