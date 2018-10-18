/*-----------------------------------------------------------------------------
 **   hash.c
 **
 **   Implementation of a simple Hash Table for string storage & retrieval
 **
 **   Written by L. Rossman
 **   Last Updated on 6/19/03
 **
 **   The hash table data structure (HTable) is defined in "hash.h".
 **   Interface Functions:
 **      HTcreate() - creates a hash table
 **      HTinsert() - inserts a string & its index value into a hash table
 **      HTfind()   - retrieves the index value of a string from a table
 **      HTfree()   - frees a hash table
 **
 *********************************************************************
 **   NOTE:  This is a modified version of the original HASH.C module.
 *********************************************************************
 */

#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <string.h>
#include "hash.h"

unsigned int _enHash(char *str);
unsigned int _enHash(char *str)
{
  unsigned int hash = 5381;
  unsigned int retHash;
  int c;
  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }
  retHash = hash % ENHASHTABLEMAXSIZE;
  
  return retHash;
}

ENHashTable *ENHashTableCreate()
{
  int i;
  ENHashTable *ht = (ENHashTable *) calloc(ENHASHTABLEMAXSIZE, sizeof(ENHashTable));
  if (ht != NULL) {
    for (i=0; i<ENHASHTABLEMAXSIZE; i++) {
      ht[i] = NULL;
    }
  }
  return(ht);
}

int ENHashTableInsert(ENHashTable *ht, char *key, int data)
{
  size_t len;
  unsigned int i = _enHash(key);
  ENHashEntry *entry;
  if ( i >= ENHASHTABLEMAXSIZE ) {
    return(0);
  }
  entry = (ENHashEntry *) malloc(sizeof(ENHashEntry));
  if (entry == NULL) {
    return(0);
  }
  len = strlen(key) + 1;
  entry->key = calloc(len, sizeof(char));
  strncpy(entry->key, key, len);
  entry->data = data;
  entry->next = ht[i];
  ht[i] = entry;
  return(1);
}

/* Abel Heinsbroek: Added function to update the hash table value for a given key */
int ENHashTableUpdate(ENHashTable *ht, char *key, int new_data)
{
  unsigned int i = _enHash(key);
  ENHashEntry *entry;
  if ( i >= ENHASHTABLEMAXSIZE ) {
    return(NOTFOUND);
  }
  entry = ht[i];
  while (entry != NULL)
  {
    if ( strcmp(entry->key,key) == 0 ) {
      entry->data = new_data;
      return(1);
    }
    entry = entry->next;
  }
  return(NOTFOUND);
}

int ENHashTableDelete(ENHashTable *ht, char *key) {
  unsigned int i = _enHash(key);
  ENHashEntry *entry;
  if ( i >= ENHASHTABLEMAXSIZE ) {
    return(NOTFOUND);
  }
  entry = ht[i];
  while (entry != NULL)
  {
    if (strcmp(entry->key, key) == 0) {
      entry->key = "";
      return(1);
    }
    entry = entry->next;
  }

  return(NOTFOUND);
}

int     ENHashTableFind(ENHashTable *ht, char *key)
{
  unsigned int i = _enHash(key);

  ENHashEntry *entry;
  if ( i >= ENHASHTABLEMAXSIZE ) {
    return(NOTFOUND);
  }

  entry = ht[i];
  while (entry != NULL)
  {
    if ( strcmp(entry->key,key) == 0 ) {
      return(entry->data);
    }
    entry = entry->next;
  }
  return(NOTFOUND);
}

char    *ENHashTableFindKey(ENHashTable *ht, char *key)
{
  unsigned int i = _enHash(key);
  ENHashEntry *entry;
  if ( i >= ENHASHTABLEMAXSIZE ) {
    return(NULL);
  }
  entry = ht[i];
  while (entry != NULL)
  {
    if ( strcmp(entry->key,key) == 0 ) {
      return(entry->key);
    }
    entry = entry->next;
  }
  return(NULL);
}

void    ENHashTableFree(ENHashTable *ht)
{
  ENHashEntry *entry, *nextentry;
  int i;
  for (i=0; i<ENHASHTABLEMAXSIZE; i++)
  {
    entry = ht[i];
    while (entry != NULL)
    {
      nextentry = entry->next;
      free(entry->key);
      free(entry);
      entry = nextentry;
    }
  }
  free(ht);
}
