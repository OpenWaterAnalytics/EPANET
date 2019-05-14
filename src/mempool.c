/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       mempool.c
 Description:  a simple fast poooled memory allocation package
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 05/15/2019

 This module is based code by Steve Hill in Graphics Gems III,
 David Kirk (ed.), Academic Press, Boston, MA, 1992
 ******************************************************************************
*/

#include <stdlib.h>

#include "mempool.h"

/*
**  ALLOC_BLOCK_SIZE - adjust this size to suit your installation - it
**  should be reasonably large otherwise you will be mallocing a lot.
*/

#define ALLOC_BLOCK_SIZE   64000       /*(62*1024)*/

struct MemBlock
{
    struct MemBlock *next;   // Next block
    char            *block,  // Start of block
                    *free,   // Next free position in block
                    *end;    // block + block size
};

struct Mempool
{
    struct MemBlock *first;
    struct MemBlock *current;
};

static struct MemBlock* createMemBlock()
{
    struct MemBlock* memBlock = malloc(sizeof(struct MemBlock));
    if (memBlock)
    {
        memBlock->block = malloc(ALLOC_BLOCK_SIZE * sizeof(char));
        if (memBlock->block == NULL)
        {
            free(memBlock);
            return NULL;
        }
        memBlock->free = memBlock->block;
        memBlock->next = NULL;
        memBlock->end = memBlock->block + ALLOC_BLOCK_SIZE;
    }
    return memBlock;
}


static void deleteMemBlock(struct MemBlock* memBlock)
{
    free(memBlock->block);
    free(memBlock);
}


struct Mempool * mempool_create()
{
    struct Mempool *mempool;
    mempool = (struct Mempool *)malloc(sizeof(struct Mempool));
    if (mempool == NULL) return NULL;
    mempool->first = createMemBlock();
    mempool->current = mempool->first;
    if (mempool->first == NULL) return NULL;
    return mempool;
}

void mempool_delete(struct Mempool *mempool)
{
    if (mempool == NULL) return;
    while (mempool->first)
    {
        mempool->current = mempool->first->next;
        deleteMemBlock(mempool->first);
        mempool->first = mempool->current;
    }
    free(mempool);
    mempool = NULL;
}

void mempool_reset(struct Mempool *mempool)
{
    mempool->current = mempool->first;
    mempool->current->free = mempool->current->block;
}


char * mempool_alloc(struct Mempool *mempool, size_t size)
{
    char* ptr;

    /*
    **  Align to 4 byte boundary - should be ok for most machines.
    **  Change this if your machine has weird alignment requirements.
    */
    size = (size + 3) & 0xfffffffc;

    if (!mempool->current) return NULL;
    ptr = mempool->current->free;
    mempool->current->free += size;

    // Check if the current block is exhausted

    if (mempool->current->free >= mempool->current->end)
    {
        // Is the next block already allocated?

        if (mempool->current->next)
        {
            // re-use block
            mempool->current->next->free = mempool->current->next->block;
            mempool->current = mempool->current->next;
        }
        else
        {
            // extend the pool with a new block
            mempool->current->next = createMemBlock();
            if (!mempool->current->next) return NULL;
            mempool->current = mempool->current->next;
        }

        // set ptr to the first location in the next block

        ptr = mempool->current->free;
        mempool->current->free += size;
    }

    // Return pointer to allocated memory

    return ptr;
}
