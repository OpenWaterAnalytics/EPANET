//
// Author: David Muto
// https://gist.github.com/pseudomuto/6334796#file-list-c
//
//  Modified by:
//         Michael E. Tryby
//         US EPA ORD
//
// Accessed on: April 9, 2019
//


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
    listNode *current;

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
    listNode *node = malloc(sizeof(listNode));
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
    listNode *node = malloc(sizeof(listNode));
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

    listNode *node = list->head;
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

    listNode *node = list->head;
    void *element = (void *)malloc(list->elementSize);
	// Copying pointer to node->data
    memcpy(element, node->data, list->elementSize);

    if(removeFromList) {
		// Disconnecting head node
        list->head = node->next;
        list->logicalLength--;

		// Freeing pointer to node->data and node
        free(node->data);
        free(node);
    }
	// Now element points to data formerly pointed to by node
	return element;
}

void *tail_list(list_t *list)
{
    assert(list->tail != NULL);

    listNode *node = list->tail;
    void *element = (void *)malloc(list->elementSize);
    memcpy(element, node->data, list->elementSize);

	return element;
}

int size_list(list_t *list)
{
    return list->logicalLength;
}
