/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       hash.h
 Description:  header for a simple hash table
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 11/27/2018
 ******************************************************************************
*/
#ifndef HASH_H
#define HASH_H

#define NOTFOUND  0

typedef struct DataEntryStruct *HashTable;

HashTable *hashtable_create(void);
int       hashtable_insert(HashTable *, char *, int);
int       hashtable_find(HashTable *, char *);
char      *hashtable_findkey(HashTable *, char *);
void      hashtable_free(HashTable *);
int       hashtable_update(HashTable *ht, char *key, int new_data);
int       hashtable_delete(HashTable *ht, char *key);

#endif
