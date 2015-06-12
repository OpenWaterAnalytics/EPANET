For compiling EPANET2.DLL :

Open the EPANET.C file and make sure that the line
       #define DLL
   is not commented out while the lines
       #define CLE
       #define SOL
   are commented out.

For compiling EPANET2.EXE :

Open the EPANET.C file and make sure that the line
       #define CLE
   is not commented out while the lines
       #define DLL
       #define SOL
   are commented out.
