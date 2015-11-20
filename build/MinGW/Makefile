# MINGW32 Makefile for EPANET

# This will build EPANET as DLL
# (epanet2.dll) under MINGW32 GCC, and a standalone
# executable (epanet2d.exe).

# The following targets are defined:
# make
#     -Builds epanet2.dll, epanet2d.exe
# make install
#     -Copy epanet2.dll, epanet2d.exe, epanet2.def to $(prefix)
# make clean
#     -Removes object and library files, returning the build directory
#      to its pristine state.

# You may wish to change the install path 'prefix',
# or the compiler flags, but these defaults should be fine.


# Target filenames
epanetrootname := epanet2
libname := $(epanetrootname).dll
exename := $(epanetrootname)d.exe

# Location of EPANET toolkit includes
epanetincludedir = ../../include
# Search path for sources
epanetsrcdir = ../../src
# Search path for sources
epanetmainsrcdir = ../../run
VPATH = $(epanetsrcdir):$(epanetincludedir):$(epanetmainsrcdir)

# Install directories
prefix = C:\discoD\EPA\EPAnet_util
execdir = $(prefix)
libdir = $(prefix)


# Compiler and flags
CC = gcc
CFLAGS = -g -O3 -std=c99 -Wno-implicit-function-declaration -D DLL
CPPFLAGS = -I $(epanetincludedir) -I $(epanetsrcdir)
LDFLAGS = -L . -Wl,--kill-at,--enable-stdcall-fixup,-rpath,$(libdir) -lm


# Files for the shared object library
epanet_objs=hash.o hydraul.o inpfile.o input1.o input2.o \
	    input3.o mempool.o output.o quality.o report.o \
	    rules.o smatrix.o epanet.o
# Epanet header files
epanet_heads=enumstxt.h funcs.h hash.h mempool.h text.h types.h vars.h epanet2.h
# Epanet main program
epanet_main=main
# Epanet main program header files
epanet_main_heads=epanet2.h

.PHONY: all
all: $(libname) $(exename)

$(libname): $(epanet_objs)
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -o $@ $^ -Wl,--output-def,$(epanetrootname).def

$(exename): $(epanet_main_heads) $(libname) $(epanet_main).o
	$(CC) $(CFLAGS) -o $@ $(epanet_main).o  -l$(epanetrootname) $(LDFLAGS)

$(epanet_objs): $(epanet_heads)


.PHONY: clean
clean:
	del $(epanet_main).o $(epanet_objs) $(exename) $(libname) $(epanetrootname).def
	
.PHONY: install
install:
	copy $(exename) $(execdir)
	copy $(libname) $(libdir)
	copy $(epanetrootname).def $(libdir)

