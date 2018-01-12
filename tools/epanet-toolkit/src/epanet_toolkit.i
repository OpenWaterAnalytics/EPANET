/*
 *  epanet_toolkit.i - SWIG interface description file for EPANET toolkit
 * 
 *  Created:    11/27/2017
 *  Author:     Michael E. Tryby
 *              US EPA - ORD/NRMRL
 *  
 *  Build command: 
 *    $ swig -I../../../include -python epanet_toolkit.i
 *
*/ 

%module epanet_toolkit
%{
#include "epanet2.h"

#define SWIG_FILE_WITH_INIT
%}


/* DEFINE AND TYPEDEF MUST BE INCLUDED */
typedef void* EN_ProjectHandle;


#ifndef DLLEXPORT
  #ifdef WINDOWS
    #ifdef __cplusplus
      #define DLLEXPORT extern "C" __declspec(dllexport)
    #else
      #define DLLEXPORT __declspec(dllexport) __stdcall
    #endif // __cplusplus
  #elif defined(CYGWIN)
    #define DLLEXPORT __stdcall
  #elif defined(__APPLE__)
    #ifdef __cplusplus
      #define DLLEXPORT
    #else
      #define DLLEXPORT
    #endif
  #else
    #define DLLEXPORT
  #endif
#endif


/* TYPEMAPS FOR OPAQUE POINTER */
/* Used for functions that output a new opaque pointer */
%typemap(in, numinputs=0) EN_ProjectHandle* ph (EN_ProjectHandle retval)
{
 /* OUTPUT in */
    retval = NULL;
    $1 = &retval;
}
/* used for functions that take in an opaque pointer (or NULL)
and return a (possibly) different pointer */
%typemap(argout) EN_PorjectHandle* ph
{
 /* OUTPUT argout */
    %append_output(SWIG_NewPointerObj(SWIG_as_voidptr(retval$argnum), $1_descriptor, 0));
} 
/* No need for special IN typemap for opaque pointers, it works anyway */


/* NO EXCEPTION HANDLING FOR THESE FUNCTIONS */    
int DLLEXPORT EN_alloc(EN_ProjectHandle* ph);
int DLLEXPORT EN_open(EN_ProjectHandle ph, char *inpFile, char *rptFile, char *binOutFile);
int DLLEXPORT EN_solveH(EN_ProjectHandle ph);
int DLLEXPORT EN_solveQ(EN_ProjectHandle ph);
int DLLEXPORT EN_report(EN_ProjectHandle ph);
int DLLEXPORT EN_close(EN_ProjectHandle ph);
int DLLEXPORT EN_free(EN_ProjectHandle ph);
