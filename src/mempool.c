/*  mempool.c
**
**  A simple fast memory allocation package.
**
**  By Steve Hill in Graphics Gems III, David Kirk (ed.),
**    Academic Press, Boston, MA, 1992
**
**  Modified by Lew Rossman, 8/13/94.
**
**  AllocInit()     - create an alloc pool, returns the old pool handle
**  Alloc()         - allocate memory
**  AllocReset()    - reset the current pool
**  AllocSetPool()  - set the current pool
**  AllocFree()     - free the memory used by the current pool.
**
*/

#include <stdlib.h>
#include <malloc.h>
#include "mempool.h"

/*
**  ALLOC_BLOCK_SIZE - adjust this size to suit your installation - it
**  should be reasonably large otherwise you will be mallocing a lot.
*/

#define ALLOC_BLOCK_SIZE   64000       /*(62*1024)*/

/*
**  alloc_hdr_t - Header for each block of memory.
*/

typedef struct alloc_hdr_s
{
    struct alloc_hdr_s *next;   /* Next Block          */
    char               *block,  /* Start of block      */
                       *free,   /* Next free in block  */
                       *end;    /* block + block size  */
}  alloc_hdr_t;

/*
**  alloc_root_t - Header for the whole pool.
*/

typedef struct alloc_root_s
{
    alloc_hdr_t *first,    /* First header in pool */
                *current;  /* Current header       */
}  alloc_root_t;

/*
**  root - Pointer to the current pool.
*/

static alloc_root_t *root;


/*
**  AllocHdr()
**
**  Private routine to allocate a header and memory block.
*/

static alloc_hdr_t *AllocHdr(void);
                
static alloc_hdr_t * AllocHdr()
{
    alloc_hdr_t     *hdr;
    char            *block;

    block = (char *) malloc(ALLOC_BLOCK_SIZE);
    hdr   = (alloc_hdr_t *) malloc(sizeof(alloc_hdr_t));

    if (hdr == NULL || block == NULL) return(NULL);
    hdr->block = block;
    hdr->free  = block;
    hdr->next  = NULL;
    hdr->end   = block + ALLOC_BLOCK_SIZE;

    return(hdr);
}


/*
**  AllocInit()
**
**  Create a new memory pool with one block.
**  Returns pointer to the new pool.
*/

alloc_handle_t * AllocInit()
{
    alloc_handle_t *newpool;

    root = (alloc_root_t *) malloc(sizeof(alloc_root_t));
    if (root == NULL) return(NULL);
    if ( (root->first = AllocHdr()) == NULL) return(NULL);
    root->current = root->first;
    newpool = (alloc_handle_t *) root;
    return(newpool);
}


/*
**  Alloc()
**
**  Use as a direct replacement for malloc().  Allocates
**  memory from the current pool.
*/

char * Alloc(long size)
{
    alloc_hdr_t  *hdr = root->current;
    char         *ptr;

    /*
    **  Align to 4 byte boundary - should be ok for most machines.
    **  Change this if your machine has weird alignment requirements.
    */
    size = (size + 3) & 0xfffffffc;

    ptr = hdr->free;
    hdr->free += size;

    /* Check if the current block is exhausted. */

    if (hdr->free >= hdr->end)
    {
        /* Is the next block already allocated? */

        if (hdr->next != NULL)
        {
            /* re-use block */
            hdr->next->free = hdr->next->block;
            root->current = hdr->next;
        }
        else
        {
            /* extend the pool with a new block */
            if ( (hdr->next = AllocHdr()) == NULL) return(NULL);
            root->current = hdr->next;
        }

        /* set ptr to the first location in the next block */
        ptr = root->current->free;
        root->current->free += size;
    }

    /* Return pointer to allocated memory. */

    return(ptr);
}


/*
**  AllocSetPool()
**
**  Change the current pool.  Return the old pool.
*/

alloc_handle_t * AllocSetPool(alloc_handle_t *newpool)
{
    alloc_handle_t *old = (alloc_handle_t *) root;
    root = (alloc_root_t *) newpool;
    return(old);
}


/*
**  AllocReset()
**
**  Reset the current pool for re-use.  No memory is freed,
**  so this is very fast.
*/

void  AllocReset()
{
    root->current = root->first;
    root->current->free = root->current->block;
}


/*
**  AllocFreePool()
**
**  Free the memory used by the current pool.
**  Don't use where AllocReset() could be used.
*/

void  AllocFreePool()
{
    alloc_hdr_t  *tmp,
                 *hdr = root->first;

    while (hdr != NULL)
    {
        tmp = hdr->next;
        free((char *) hdr->block);
        free((char *) hdr);
        hdr = tmp;
    }
    free((char *) root);
    root = NULL;
}
