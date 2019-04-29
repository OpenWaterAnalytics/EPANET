/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       util/list.h
 Description:  Generic list
               https://gist.github.com/pseudomuto/6334796#file-list-h
               Accessed: April 9, 2019
 Authors:      David Muto, Michael Tryby
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 04/09/2019
 ******************************************************************************
*/

#ifndef LIST_H
#define LIST_H

#if defined(__cplusplus)
extern "C" {
#endif

// Forward declarations
typedef struct list_node_s list_node_t;
typedef struct list_s list_t;

typedef enum { FALSE, TRUE } BOOL;
typedef void (*freeFunction) (void *);
typedef BOOL (*listIterator) (list_node_t *);

//-----------------------------------------------------//
//  List creators, destructors, inserters & iterators  //
//-----------------------------------------------------//

/**
@brief Initializes a linked list to store elements of elementSize and to call
freeFunction for each element when destroying a list.
*/
list_t *create_list(size_t elementSize, freeFunction freeFn);

/**
@brief Frees dynamically allocated nodes and optionally calls freeFunction
with each node’s data pointer.
*/
void delete_list(list_t *list);

/**
@brief Adds a node to the head of the list and returns its key.
*/
int prepend_list(list_t *list, void *element);

/**
@brief Adds a node to the tail of the list and returns its key.
*/
int append_list(list_t *list, void *element);

/**
@brief Removes the n-th node of the list.
*/
void remove_nth_node(list_t *list, int n);

/**
@brief Calls the supplied iterator function with the data element of each
node (iterates over the list's data elements).
*/
void for_each_list(list_t *list, listIterator iterator);

/**
@brief Returns the number of items in the list.
*/
int size_list(list_t *list);

/**
@brief Returns the index of item with given key.
*/
int get_index(list_t *list, int key);


//----------------------------------------//
// List operations using node references  //
//----------------------------------------//

/**
@brief Returns pointer to list node's data.
*/
void *get_data(list_node_t *lnode);

/**
@brief Returns list node's key value.
*/
int get_key(list_node_t *lnode);

/**
@brief Returns next list node.
*/
list_node_t *get_next(list_node_t *lnode);

/**
@brief Frees memory associated with a list node.
*/
void delete_node(list_t *list,  list_node_t *lnode);

/**
@brief Returns the head of the list (optionally removing it at the same time).
*/
list_node_t *head_list(list_t *list, BOOL removeFromList);

/**
@brief Returns the tail of the list.
*/
list_node_t *tail_list(list_t *list);

/**
@brief Returns nth node of the list or NULL.
*/
list_node_t *get_nth_node(list_t *list, int index);

/**
@brief Returns the list node with the given key or NULL.
*/
list_node_t *search_list(list_t *list, int key);

/**
@brief Removes the list node with the given key from the list. 
*/
void remove_node(list_t *list, int key);

//---------------------------//
//  Data element retrievers  //
//---------------------------//

/**
@brief Returns pointer to data element of first list item.
*/
void *get_first_data(list_t *list);

/**
@brief Returns pointer to data element of next list item.
*/
void *get_next_data(list_t *list);

/**
@brief Returns pointer to data element of nth list item.
*/
void *get_nth_data(list_t *list, int n);

//
// Iterator first/done/next operations
// http://www.cs.yale.edu/homes/aspnes/pinewiki/C(2f)Iterators.html
// Accessed on April 11, 2019
//

/**
@brief Returns list head node.
*/
static inline list_node_t *first_list(list_t *list) { return head_list(list, FALSE); }

/**
@brief Returns true if end of list false otherwise.
*/
static inline int done_list(list_node_t *lnode) { return lnode != NULL; }

/**
@brief Returns next node in the list.
*/
static inline list_node_t *next_list(list_node_t *lnode) { return get_next(lnode); }


#if defined(__cplusplus)
}
#endif

#endif /* LIST_H */
