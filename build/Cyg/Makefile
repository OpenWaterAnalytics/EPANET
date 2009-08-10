# Cygwin Makefile for EPANET

# This will build EPANET as a cygwin DLL and import
# library (cygepanet_gcc_<Rev>.dll/libcygepanet_gcc_<Rev>.dll.a) under Cygwin/gcc,
# and a standalone executable (cygepanet_gcc_<Rev>.exe).
# <Rev> is the atomic revision number of the EPANET SVN repo,
# so the results of each build can be unambiguously tracked to a repo Rev.
# The current release of the Cygwin environment can be
# obtained from http://www.cygwin.com

# The following targets are defined (for execution in
# the build directory under the Cygwin environment):
# make
#     -Builds cygepanet_gcc_<Rev>.dll, cygepanet_gcc_<Rev>.def, libcygepanet_gcc_<Rev>.dll.a,
#      cygepanet_gcc_<Rev>.exe
# make install
#     -Creates shell wrapper runcygepanet_<Rev>.sh that
#      executes cygepanet_gcc_<Rev>.exe.
#      The runcygepanet_<Rev>.sh wrapper simply exports
#      environment variables so that the DLL is found at runtime,
#      allowing you to specify your own locations for installing
#      the DLL.
#     -Installs cygepanet_gcc_<Rev>.dll, cygepanet_gcc_<Rev>.exe, and runcygepanet_<Rev>.sh
#      in <prefix>/bin, where <prefix> defaults to ~ (and can be set
#      below to something different - see "Install directories")
#     -Installs libcygepanet_gcc_<Rev>.dll.a (import library), and cygepanet_gcc_<Rev>.def,
#      in <prefix>/lib
#     -Installs epanet2.h in <prefix>/include.  This is the required
#      header file for the EPANET programmer's toolkit, and thus
#      <prefix>/include should be on your CPP include search path
#      for subsequent use of the toolkit and linking with the import
#      library libcygepanet_gcc_<Rev>.dll.a.
# make clean
#     -Removes object and library files, returning the build directory
#      to its pristine state.

SHELL = /bin/sh

# C H A N G E  H E R E  A S  N E E D E D
# You may wish to change the install path 'prefix',
# or the compiler flags, but these defaults should be fine.

# Target filenames
# svnname.sh constructs a name: <prefix><root><Rev><postfix>
# where <Rev> is the atomic revision number of the svn repo.
epanetsvnpath = ../../..
epanetrootname := $(shell ../svnname.sh $(epanetsvnpath) "" cygepanet_gcc_ "")
dllname := $(epanetrootname).dll
defname := $(epanetrootname).def
implibname = lib$(dllname).a
exename := $(epanetrootname)
# Shell wrapper
runcmdtemplate = runepanet.sh.template
runcmdname = $(shell ../svnname.sh $(epanetsvnpath) "" runcygepanet_ .sh)
# Location of EPANET toolkit includes
epanetincludedir = ../../include
# Search path for sources
epanetsrcdir = ../../src
VPATH = $(epanetsrcdir):$(epanetincludedir)

# Install directories
prefix = ~
exec_prefix = $(prefix)
srcdir = .
libdir = $(exec_prefix)/lib
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
datarootdir = $(prefix)/share
docdir = $(datarootdir)/doc/epanet

# Compiler and flags
CC = /bin/gcc
dlltool = /bin/dlltool
CFLAGS = -g -O3
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
	$(CC) $(CFLAGS) $(CPPFLAGS) -D SOL -c $(epanetsrcdir)/$(epanet_main).c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $(epanet_main).o $^ -Wl,--output-def,$(defname)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $(epanet_main).o $^ -Wl,--kill-at
	$(dlltool) -d $(defname) --dllname $@ --output-lib $(implibname) --kill-at

$(exename): $(epanet_objs)
	$(CC) $(CFLAGS) $(CPPFLAGS) -D CLE -c $(epanetsrcdir)/$(epanet_main).c
	$(CC) $(CFLAGS) -o $@ $(epanet_main).o $^ $(LDFLAGS)

$(epanet_objs): $(epanet_heads)

.PHONY: install
install:
	cat $(runcmdtemplate) | sed 's|libdir|$(bindir)|' \
                                | sed 's|exename|$(bindir)/$(exename)|' \
                                > $(runcmdname)
	$(INSTALL_PROGRAM) -D $(exename) $(bindir)/$(exename)
	$(INSTALL_PROGRAM) -D $(dllname) $(bindir)/$(dllname)
	$(INSTALL_PROGRAM) -D $(defname) $(libdir)/$(defname)
	$(INSTALL_PROGRAM) -D $(implibname) $(libdir)/$(implibname)
	$(INSTALL_DATA) -D $(epanetincludedir)/epanet2.h $(includedir)/epanet2.h
	$(INSTALL_PROGRAM) -D $(runcmdname) $(bindir)/$(runcmdname)

.PHONY: uninstall
uninstall:

.PHONY: check
check:

.PHONY: clean
clean:
	-/bin/rm *.o $(dllname) $(defname) $(implibname) $(exename).exe $(runcmdname)
