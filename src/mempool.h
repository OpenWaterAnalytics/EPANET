/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       mempool.h
 Description:  header for a simple pooled memory allocator
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/27/2018
 ******************************************************************************
*/

#ifndef MEMPOOL_H
#define MEMPOOL_H

struct Mempool;

struct Mempool * mempool_create();
void   mempool_delete(struct Mempool *mempool);
void   mempool_reset(struct Mempool *mempool);
char * mempool_alloc(struct Mempool *mempool, size_t size);

#endif