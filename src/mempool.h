/*
**  mempool.h
**
**  Header for mempool.c
**
**  The type alloc_handle_t provides an opaque reference to the
**  alloc pool - only the alloc routines know its structure.
*/

#ifndef MEMPOOL_H
#define MEMPOOL_H

#ifndef DLLEXPORT
#ifdef DLL
#ifdef __cplusplus
#define DLLEXPORT extern "C" __declspec(dllexport)
#else
#define DLLEXPORT __declspec(dllexport) __stdcall
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

typedef struct { long dummy; } alloc_handle_t;

alloc_handle_t *AllocInit(void);
char *Alloc(long);
alloc_handle_t *AllocSetPool(alloc_handle_t *);
void AllocReset(void);
void AllocFreePool(void);

#endif