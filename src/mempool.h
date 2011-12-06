/*
**  mempool.h
**
**  Header for mempool.c
**
**  The type alloc_handle_t provides an opaque reference to the
**  alloc pool - only the alloc routines know its structure.
*/
#ifndef DLLEXPORT
  #ifdef DLL
    #ifdef __cplusplus
    #define DLLEXPORT extern "C" __declspec(dllexport)
    #else
    #define DLLEXPORT __declspec(dllexport)
    #endif
  #elif defined(CYGWIN)
    #define DLLEXPORT __stdcall
  #else
    #ifdef __cplusplus
    #define DLLEXPORT
    #else
    #define DLLEXPORT
    #endif
  #endif
#endif

typedef struct
{
   long  dummy;
}  alloc_handle_t;

DLLEXPORT alloc_handle_t *AllocInit(void);
DLLEXPORT char           *Alloc(long);
DLLEXPORT alloc_handle_t *AllocSetPool(alloc_handle_t *);
DLLEXPORT void            AllocReset(void);
DLLEXPORT void            AllocFreePool(void);
