//
// Author: David Muto
// https://gist.github.com/pseudomuto/6334796#file-list-h
//
//  Modified by:
//         Michael E. Tryby
//         US EPA ORD
//
// Accessed on: April 9, 2019
//


#ifndef LIST_H
#define LIST_H


#include <stdbool.h>

#if defined(__cplusplus)
extern "C" {
#endif

// a common function used to free malloc'd objects
typedef void(*freeFunction)(void *);
typedef bool (*listIterator)(void *);


typedef struct _listNode {
  void *data;
  struct _listNode *next;
} listNode;


typedef struct {
  int logicalLength;
  int elementSize;
  listNode *head;
  listNode *tail;
  freeFunction freeFn;
} list_t;


list_t *create_list(int elementSize, freeFunction freeFn);
void delete_list(list_t *list);

void prepend_list(list_t *list, void *element);
void append_list(list_t *list, void *element);
int size_list(list_t *list);

void for_each_list(list_t *list, listIterator iterator);
void head_list(list_t *list, void *element, bool removeFromList);
void tail_list(list_t *list, void *element);


#if defined(__cplusplus)
}
#endif

#endif /* LIST_H */
