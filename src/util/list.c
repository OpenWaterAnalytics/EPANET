/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/list.h
 Description:  Generic list
               https://gist.github.com/pseudomuto/6334796#file-list-c
               Accessed: April 9, 2019
 Authors:      David Muto, Modified by Michael E. Tryby
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
	list_node_t *next;
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

        if (list->freeFn)
            list->freeFn(current->data);

        free(current->data);
        free(current);
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
        result = iterator(node->data);
        node = node->next;
    }
}

void *head_list(list_t *list, bool removeFromList)
//
// Warning: Caller is responsible for freeing the node->data returned.
//
{
    assert(list->head != NULL);

	list_node_t *node = list->head;
    // Allocating and copying pointer to node data
    void *element = (void *)malloc(list->elementSize);
    memcpy(element, node->data, list->elementSize);

    if(removeFromList) {
        // Disconnecting head node
        list->head = node->next;
        list->logicalLength--;

        // Freeing pointer to node->data and node
        free(node->data);
        free(node);
    }
    // Now element points to data formerly pointed to by node. Caller
    // is responsible for freeing both pointer to data and data.
	return element;
}

void *tail_list(list_t *list)
//
// Warning: Caller is responsible for freeing the node->data returned.
//
{
    assert(list->tail != NULL);

	list_node_t *node = list->tail;
    // Allocating and copying pointer to node data
    void *element = (void *)malloc(list->elementSize);
    memcpy(element, node->data, list->elementSize);

    // Pointer to element data gets returned. Caller is responsible
    // for freeing pointer to data.
	return element;
}

int size_list(list_t *list)
{
    return list->logicalLength;
}
