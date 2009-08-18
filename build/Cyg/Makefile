# Cygwin Makefile for EPANET

# This will build EPANET as a cygwin DLL and import
# library (cygepanet.dll/libcygepanet.dll.a) under Cygwin/gcc,
# and a standalone executable (cygepanet.exe).
# The current release of the Cygwin environment can be
# obtained from http://www.cygwin.com

# The following targets are defined (for execution in
# the build directory under the Cygwin environment):
# make
#     -Builds cygepanet2.dll, cygepanet2.def, libcygepanet2.dll.a,
#      cygepanet2.exe
# make install
#     -Creates shell wrapper runcygepanet2.sh that
#      executes cygepanet2.exe.
#      The runcygepanet2.sh wrapper simply exports
#      environment variables so that the DLL is found at runtime,
#      allowing you to specify your own locations for installing
#      the DLL.
#     -Installs cygepanet2.dll, cygepanet2.exe, and runcygepanet2.sh
#      in <prefix>/bin, where <prefix> defaults to ~ (and can be set
#      below to something different - see "Install directories")
#     -Installs libcygepanet2.dll.a (import library), and cygepanet2.def,
#      in <prefix>/lib
#     -Installs epanet2.h in <prefix>/include.  This is the required
#      header file for the EPANET programmer's toolkit, and thus
#      <prefix>/include should be on your CPP include search path
#      for subsequent use of the toolkit and linking with the import
#      library libcygepanet2.dll.a.
# make clean
#     -Removes object and library files, returning the build directory
#      to its pristine state.

SHELL = /bin/sh

# C H A N G E  H E R E  A S  N E E D E D
# You may wish to change the install path 'prefix',
# or the compiler flags, but these defaults should be fine.

# Target filenames
epanetrootname := cygepanet2
dllname := $(epanetrootname).dll
defname := $(epanetrootname).def
implibname = lib$(dllname).a
exename := $(epanetrootname)
# Shell wrapper
runcmdtemplate = runepanet.sh.template
runcmdname = runcygepanet2.sh
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
