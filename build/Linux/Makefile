# Linux Makefile for EPANET

# This will build EPANET as a shared object library
# (libepanet2.so) under Linux/Unix, and a standalone
# executable (epanet2).

# The following targets are defined:
# make
#     -Builds libepanet2.so, epanet2
# make install
#     -Creates shell wrapper runepanet2.sh that executes epanet2.
#      The runepanet2.sh wrapper simply exports
#      environment variables that help locate the runtime library,
#      allowing you to specify your own library locations.
#     -Installs epanet2 and runepanet2.sh
#      in <prefix>/bin, where <prefix> defaults to ~ (and can be set
#      below to something different - see "Install directories")
#     -Installs libepanet2.so in <prefix>/lib
#     -Installs epanet2.h in <prefix>/include.  This is the required
#      header file for the EPANET programmer's toolkit, and thus
#      <prefix>/include should be on your CPP include search path
#      for subsequent use of the toolkit and linking with the
#      library libepanet2.so
# make clean
#     -Removes object and library files, returning the build directory
#      to its pristine state.

# You may wish to change the install path 'prefix',
# or the compiler flags, but these defaults should be fine.

SHELL = /bin/sh

# Target filenames
epanetrootname := epanet2
libname := lib$(epanetrootname).so
exename := $(epanetrootname)
# Shell wrapper
runcmdtemplate = runepanet.sh.template
runcmdname = runepanet2.sh
# Location of EPANET toolkit includes
epanetincludedir = ../../include
# Search path for sources
epanetsrcdir = ../../src
VPATH = $(epanetsrcdir):$(epanetincludedir)

# Install directories
prefix = ~
exec_prefix = $(prefix)
libdir = $(exec_prefix)/lib
bindir = $(exec_prefix)/bin
includedir = $(prefix)/include
datarootdir = $(prefix)/share
docdir = $(datarootdir)/doc/epanet

# Compiler and flags
CC = gcc
CFLAGS = -g -O3 -fPIC -std=c99
CPPFLAGS = -I $(epanetincludedir)
LDFLAGS = -L . -Wl,-rpath,$(libdir) -lm

# Installer
INSTALL = install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA = $(INSTALL) -m 644

# Files for the shared object library
epanet_objs=hash.o hydraul.o inpfile.o input1.o input2.o \
	    input3.o mempool.o output.o quality.o report.o \
	    rules.o smatrix.o
# Epanet header files
epanet_heads=enumstxt.h funcs.h hash.h mempool.h text.h toolkit.h types.h vars.h
# Epanet main program
epanet_main=epanet
# Epanet main program header files
epanet_main_heads=epanet2.h

.PHONY: all
all: $(libname) $(exename)

$(libname): $(epanet_objs)
	$(CC) $(CFLAGS) $(CPPFLAGS) -D SOL -c $(epanetsrcdir)/$(epanet_main).c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $(epanet_main).o $^

$(exename): $(libname) $(epanet_main_heads)
	$(CC) $(CFLAGS) $(CPPFLAGS) -D CLE -c $(epanetsrcdir)/$(epanet_main).c
	$(CC) $(CFLAGS) -o $@ $(epanet_main).o -l$(epanetrootname) $(LDFLAGS)

$(epanet_objs): $(epanet_heads)

.PHONY: install
install:
	cat $(runcmdtemplate) | sed 's|libdir|$(libdir)|' \
                       | sed 's|exename|$(bindir)/$(exename)|' \
                       > $(runcmdname)
	$(INSTALL_PROGRAM) -D $(exename) $(bindir)/$(exename)
	$(INSTALL_PROGRAM) -D $(libname) $(libdir)/$(libname)
	$(INSTALL_DATA) -D $(epanetincludedir)/epanet2.h $(includedir)/epanet2.h
	$(INSTALL_PROGRAM) -D $(runcmdname) $(bindir)/$(runcmdname)

.PHONY: uninstall
uninstall:

.PHONY: check
check:

.PHONY: clean
clean:
	-/bin/rm *.o $(libname) $(exename) $(runcmdname)
