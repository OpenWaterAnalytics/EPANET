/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/list.h
 Description:  Generic list
               https://gist.github.com/pseudomuto/6334796#file-list-c
               Accessed: April 9, 2019
 Authors:      David Muto, Michael Tryby
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/09/2019
 ******************************************************************************
*/

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#else
#include <stdlib.h>
#endif

#include <string.h>
#include <assert.h>

#include "list.h"


typedef struct list_node_s {
    void *data;
    struct list_node_s *next;
} list_node_t;


typedef struct list_s {
    int logicalLength;
    size_t elementSize;
    list_node_t *head;
    list_node_t *tail;
    freeFunction freeFn;
} list_t;


list_t *create_list(size_t elementSize, freeFunction freeFn)
{
    list_t *list;
    list = (list_t *)calloc(1, sizeof(list_t));

    assert(elementSize > 0);
    list->logicalLength = 0;
    list->elementSize = elementSize;
    list->head = list->tail = NULL;
    list->freeFn = freeFn;
    return list;
}

void delete_list(list_t *list)
{
    list_node_t *current;

    while(list->head != NULL) {
        current = list->head;
        list->head = current->next;
        delete_node(list, current);
    }
    free(list);
}

void prepend_list(list_t *list, void *element)
{
    list_node_t *node = malloc(sizeof(list_node_t));
    node->data = malloc(list->elementSize);
    memcpy(node->data, element, list->elementSize);

    node->next = list->head;
    list->head = node;

    // first node?
    if(!list->tail) {
        list->tail = list->head;
    }

    list->logicalLength++;
}

void append_list(list_t *list, void *element)
{
	list_node_t *node = malloc(sizeof(list_node_t));
    node->data = malloc(list->elementSize);
    node->next = NULL;

    memcpy(node->data, element, list->elementSize);

    if(list->logicalLength == 0) {
        list->head = list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    list->logicalLength++;
}

void for_each_list(list_t *list, listIterator iterator)
{
    assert(iterator != NULL);

    list_node_t *node = list->head;
    bool result = true;

	while(node != NULL && result) {
		result = (iterator);
        node = node->next;
    }
}

list_node_t *head_list(list_t *list, bool removeFromList)
//
// Warning: When node is removed caller is responsible for freeing it.
//
{
    assert(list->head != NULL);
	
	if (list) {
		list_node_t *node = list->head;
		if (removeFromList) {
			// Disconnecting head node
			list->head = node->next;
			list->logicalLength--;
		}
		return node;
    }
    return NULL;
}

list_node_t *tail_list(list_t *list)
{
    assert(list->tail != NULL);
    return list->tail;
}

list_node_t *get_nth_list(list_t *list, int index)
{	
	int n;
	list_node_t *lnode;

	for (n = 1, lnode = first_list(list); n < index && done_list(lnode); n++, lnode = next_list(lnode));
	if (n != index)
		return NULL;
	else
		return lnode;
}

int size_list(list_t *list)
{
    return list->logicalLength;
}

void *get_data(list_node_t *lnode)
{
    return lnode->data;
}

list_node_t *get_next(list_node_t *lnode)
{
	return lnode->next;
}

void delete_node(list_t *list, list_node_t *lnode)
{
    if (list->freeFn)
        list->freeFn(lnode->data);

    free(lnode->data);
    free(lnode);
}
