/*
**  mempool.h
**
**  Header for mempool.c
**
**  A simple pooled memory allocator
*/

#ifndef MEMPOOL_H
#define MEMPOOL_H

struct Mempool;

struct Mempool * mempool_create();
void   mempool_delete(struct Mempool *mempool);
void   mempool_reset(struct Mempool *mempool);
char * mempool_alloc(struct Mempool *mempool, size_t size);

#endif