/* HASH.H
**
** Header file for Hash Table module HASH.C
**
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
