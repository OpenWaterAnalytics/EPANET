/*-----------------------------------------------------------------------------
 **   hash.c
 **
 **   Implementation of a simple Hash Table that uses a string pointer
 **   as a key and an associated integer as data.
 **
 **   Written by L. Rossman
 **   Last Updated on 10/17/18
 **
 **   Interface Functions:
 **      hashtable_create  - creates a hash table
 **      hashtable_insert  - inserts a string & its data value into a table
 **      hashtable_find    - retrieves the data value associated with a string
 **      hashtable_findkey - retrieves the key associated with a data value
 **      hashtable_delete  - deletes an entry from a table
 **      hashtable_free    - frees a hash table
 **
 */

#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
#include "hash.h"

#define HASHTABLEMAXSIZE 128000

typedef struct DataEntryStruct
{
    char   *key;
    int    data;
    struct DataEntryStruct *next;
} DataEntry;

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

int hashtable_insert(HashTable *ht, char *key, int data)
{
    unsigned int i = gethash(key);
    DataEntry *entry;
    if ( i >= HASHTABLEMAXSIZE ) return 0;
    entry = (DataEntry *) malloc(sizeof(DataEntry));
    if (entry == NULL) return(0);
    entry->key = key;
    entry->data = data;
    entry->next = ht[i];
    ht[i] = entry;
    return 1;
}

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

int hashtable_delete(HashTable *ht, char *key)
{
    unsigned int i = gethash(key);
    DataEntry *entry, *preventry;
    
    if ( i >= HASHTABLEMAXSIZE ) return NOTFOUND;

    // Check if first entry in bucket i to be deleted
    entry = ht[i];
    if (strcmp(entry->key, key) == 0)
    {
        ht[i] = entry->next;
        free(entry);
        return 1;
    }

    // Check remaining entries in bucket i
    preventry = ht[i];
    entry = ht[i]->next;
    while (entry != NULL)
    {
        if (strcmp(entry->key, key) == 0)
        {
            preventry->next = entry->next;
            free(entry);
            return 1;
        }
        preventry = entry;
        entry = entry->next;
    }
    return NOTFOUND;
}

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
          free(entry);
          entry = nextentry;
        }
    }
    free(ht);
}
