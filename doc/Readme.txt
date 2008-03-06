Contents of EPANET2.ZIP
=======================
This archive contains the source code for the EPANET 2
network hydraulic and water quality solver. The solver
provides functions for simulating the extended period
hydraulic and water quality behavior of water distribution
system pipe networks. It is written in ANSI-compatible C
and can be compiled into either a Windows Dynamic Link
Library of functions or into a command-line executable.

The archived code is set up for compilation as a DLL.
To compile it as a command line (or console) application
simply comment out the "#define DLL" macro statement at
the top of EPANET.C and un-comment the "#define CLE" macro.

The DLL version of the solver (epanet2.dll) is used with
the EPANET 2 user interface executable (epanet2w.exe) to
form a complete Windows modeling package. It also serves
as the function library for the EPANET Programmer's Toolkit,
allowing developers to construct their own customized pipe
network analysis applications.

The following C-code files are included in this archive:
    EPANET.C  -- main module providing supervisory control
    INPUT1.C  -- controls processing of input data
    INPUT2.C  -- reads data from input file
    INPUT3.C  -- parses individual lines of input data
    INPFILE.C -- saves modified input data to a text file
    RULES.C   -- implements rule-based control of piping system
    HYDRAUL.C -- computes extended period hydraulic behavior
    QUALITY.C -- tracks transport & fate of water quality
    OUTPUT.C  -- handles transfer of data to and from binary files
    REPORT.C  -- handles reporting of results to text file
    SMATRIX.C -- sparse matrix linear equation solver routines
    MEMPOOL.C -- memory pool management routines
    HASH.C    -- hash table routines

Also included are the following header files:
    TOOLKIT.H  -- function prototypes of exported DLL functions
    FUNCS.H    -- prototypes of all other functions
    TYPES.H    -- declaration of global constants and data structures
    VARS.H     -- declaration of global variables
    HASH.H     -- header file for hash table routines
    MEMPOOL.H  -- header file for memory pool routines
    ENUMSTXT.H -- string constants for enumerated types
    TEXT.H     -- declaration of all other string constants

The comments at the top of each file lists the date when the latest
update was made, and these updates can be located in the code by
searching for comments with the phrase "/*** Updated" or with the
release number (e.g., 2.00.12) in them.

Other useful documentation that can be consulted includes the EPANET
Programmers Toolkit Help file and the EPANET Version 2 Users Manual.
