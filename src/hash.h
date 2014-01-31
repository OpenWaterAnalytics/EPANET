/* HASH.H
**
** Header file for Hash Table module HASH.C
**
*/

#ifndef HASH_H
#define HASH_H

#define ENHASHTABLEMAXSIZE 128000
#define NOTFOUND  0

typedef struct HTentryStruct
{
	char 	*key;
	int 	data;
	struct	HTentryStruct *next;
} ENHashEntry;

typedef ENHashEntry *ENHashTable;

ENHashTable *ENHashTableCreate(void);
int     ENHashTableInsert(ENHashTable *, char *, int);
int     ENHashTableFind(ENHashTable *, char *);
char    *ENHashTableFindKey(ENHashTable *, char *);
void    ENHashTableFree(ENHashTable *);
	
#endif