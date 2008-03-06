/*
**  mempool.h
**
**  Header for mempool.c
**
**  The type alloc_handle_t provides an opaque reference to the
**  alloc pool - only the alloc routines know its structure.
*/

typedef struct
{
   long  dummy;
}  alloc_handle_t;

alloc_handle_t *AllocInit(void);
char           *Alloc(long);
alloc_handle_t *AllocSetPool(alloc_handle_t *);
void            AllocReset(void);
void            AllocFreePool(void);
