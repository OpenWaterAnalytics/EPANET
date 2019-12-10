 /*
  ******************************************************************************
  Project:      OWA EPANET
  Version:      2.2
  Module:       hash.c
  Description:  implementation of a simple hash table
  Authors:      see AUTHORS
  Copyright:    see AUTHORS
  License:      see LICENSE
  Last Updated: 05/15/2019
 ******************************************************************************
 */

#include <stdlib.h>
#include <string.h>
#include "hash.h"

#define HASHTABLEMAXSIZE 128000

// An entry in the hash table
typedef struct DataEntryStruct
{
    char   *key;
    int    data;
    struct DataEntryStruct *next;
} DataEntry;

// Hash a string to an integer
unsigned int gethash(char *str)
{
    unsigned int hash = 5381;
    unsigned int retHash;
    int c;
    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    retHash = hash % HASHTABLEMAXSIZE;
    return retHash;
}

// Produce a duplicate string
char *dupstr(const char *s)
{
    size_t size = strlen(s) + 1;
    char *p = malloc(size);
    if (p) memcpy(p, s, size);
    return p;
}

// Create a hash table
HashTable *hashtable_create()
{
    int i;
    HashTable *ht = (HashTable *) calloc(HASHTABLEMAXSIZE, sizeof(HashTable));
    if (ht != NULL)
    {
        for (i = 0; i < HASHTABLEMAXSIZE; i++) ht[i] = NULL;
    }
    return ht;
}

// Insert an entry into the hash table
int hashtable_insert(HashTable *ht, char *key, int data)
{
    unsigned int i = gethash(key);
    DataEntry *entry;
    if ( i >= HASHTABLEMAXSIZE ) return 0;
    entry = (DataEntry *) malloc(sizeof(DataEntry));
    if (entry == NULL) return(0);
    entry->key = dupstr(key);
    entry->data = data;
    entry->next = ht[i];
    ht[i] = entry;
    return 1;
}

// Change the hash table's data entry for a particular key
int hashtable_update(HashTable *ht, char *key, int new_data)
{
    unsigned int i = gethash(key);
    DataEntry *entry;

    if ( i >= HASHTABLEMAXSIZE ) return NOTFOUND;
    entry = ht[i];
    while (entry != NULL)
    {
        if ( strcmp(entry->key, key) == 0 )
        {
            entry->data = new_data;
            return 1;
        }
        entry = entry->next;
    }
    return NOTFOUND;
}

// Delete an entry in the hash table
int hashtable_delete(HashTable *ht, char *key)
{
    unsigned int i = gethash(key);
    DataEntry *entry, *preventry;

    if ( i >= HASHTABLEMAXSIZE ) return NOTFOUND;

    preventry = NULL;
    entry = ht[i];
    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            if (preventry == NULL) ht[i] = entry->next;
            else preventry->next = entry->next;
            free(entry->key);
            free(entry);
            return 1;
        }
        preventry = entry;
        entry = entry->next;
    }
    return NOTFOUND;
}

// Find the data for a particular key
int hashtable_find(HashTable *ht, char *key)
{
    unsigned int i = gethash(key);
    DataEntry *entry;

    if ( i >= HASHTABLEMAXSIZE ) return NOTFOUND;
    entry = ht[i];
    while (entry != NULL)
    {
        if ( strcmp(entry->key, key) == 0 )
        {
            return entry->data;
        }
        entry = entry->next;
    }
    return NOTFOUND;
}

// Find a particular key in the hash table
char *hashtable_findkey(HashTable *ht, char *key)
{
    unsigned int i = gethash(key);
    DataEntry *entry;
    if ( i >= HASHTABLEMAXSIZE ) return NULL;
    entry = ht[i];
    while (entry != NULL)
    {
        if ( strcmp(entry->key, key) == 0 ) return entry->key;
        entry = entry->next;
    }
    return NULL;
}

// Delete a hash table and free all of its memory
void hashtable_free(HashTable *ht)
{
    DataEntry *entry, *nextentry;
    int i;

    for (i = 0; i < HASHTABLEMAXSIZE; i++)
    {
        entry = ht[i];
        while (entry != NULL)
        {
            nextentry = entry->next;
            free(entry->key);
            free(entry);
            entry = nextentry;
        }
        ht[i] = NULL;
    }
    free(ht);
}
