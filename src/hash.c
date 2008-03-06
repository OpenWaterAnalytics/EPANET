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

#include <malloc.h>
#include <string.h>
#include "hash.h"

/* Use Fletcher's checksum to compute 2-byte hash of string */
unsigned int hash(char *str)
{
    unsigned int sum1= 0, check1;
    unsigned long sum2= 0L;
	while(  '\0' != *str  )
    {
        sum1 += (*str);
        str++;
        if (  255 <= sum1  ) sum1 -= 255;
        sum2 += sum1;
    }
    check1= sum2;
    check1 %= 255;
    check1= 255 - (sum1+check1) % 255;
    sum1= 255 - (sum1+check1) % 255;
    return( ( ( check1 << 8 )  |  sum1  ) % HTMAXSIZE);
}

HTtable *HTcreate()
{
        int i;
        HTtable *ht = (HTtable *) calloc(HTMAXSIZE, sizeof(HTtable));
        if (ht != NULL) for (i=0; i<HTMAXSIZE; i++) ht[i] = NULL;
        return(ht);
}

int     HTinsert(HTtable *ht, char *key, int data)
{
        unsigned int i = hash(key);
        struct HTentry *entry;
        if ( i >= HTMAXSIZE ) return(0);
        entry = (struct HTentry *) malloc(sizeof(struct HTentry));
        if (entry == NULL) return(0);
        entry->key = key;
        entry->data = data;
        entry->next = ht[i];
        ht[i] = entry;
        return(1);
}

int     HTfind(HTtable *ht, char *key)
{
        unsigned int i = hash(key);
        struct HTentry *entry;
        if ( i >= HTMAXSIZE ) return(NOTFOUND);
        entry = ht[i];
        while (entry != NULL)
        {
            if ( strcmp(entry->key,key) == 0 ) return(entry->data);
            entry = entry->next;
        }
        return(NOTFOUND);
}

char    *HTfindKey(HTtable *ht, char *key)
{
        unsigned int i = hash(key);
        struct HTentry *entry;
        if ( i >= HTMAXSIZE ) return(NULL);
        entry = ht[i];
        while (entry != NULL)
        {
            if ( strcmp(entry->key,key) == 0 ) return(entry->key);
            entry = entry->next;
        }
        return(NULL);
}

void    HTfree(HTtable *ht)
{
        struct HTentry *entry,
                       *nextentry;
        int i;
        for (i=0; i<HTMAXSIZE; i++)
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
