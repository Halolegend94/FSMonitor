#ifndef __LINKEDLIST__
	#define __LINKEDLIST__
	#include <stdio.h>
	#include <stdlib.h>

	typedef struct _linkedItem{
		void *item;
		int id;
		struct _linkedItem *next;
		struct _linkedItem *prev;
	}linkedItem;

	typedef struct _linkedList{
		linkedItem *first; //the first item of the list
		linkedItem *last;  //the last item of the list
		int count;	  //the number of Items
		int idCounter;
	} linkedList;

	typedef struct _linkedListIterator{
		linkedItem *current;
	}linkedListIterator;

	// ==========================================================================
	//
	// ll_create
	// Description: creates a new empty linked list
	// Returns the linked list structure
	//
	// ==========================================================================
	linkedList ll_create();

	// ==========================================================================
	//
	// ll_add_item
	// Description: add a new element to a linked list.
	// Params:
	// 	-	list: a pointer to the linked list where to add the new element
	// 	-	pItem: a pointer to the element that must be added to the linked list
	//
	// ==========================================================================
	int ll_add_item(linkedList *list, void *pItem);

	// ==========================================================================
	//
	// ll_remove_item
	// Description: remove an item from the linked list
	// Params:
	// 	-	list: a pointer to the linked list where to remove a particular Item
	// 	-	item: the pointer to a LinkedItem element that must be removed from
	// 		the linked list.
	// Returns 0 in case of success, -1 in case of error.
	//
	// ==========================================================================
	int ll_remove_item(linkedList *list, linkedItem *Item);

	// ==========================================================================
	//
	// ll_create_iterator
	// Description: creates an iterator to list all the elemnts in a linked list.
	// Params:
	// 	-	list: the pointer to the linked list to iterate on
	// Retuns 0 in case of success, -1 otherwise
	//
	// ==========================================================================
	linkedListIterator ll_create_iterator(linkedList *list);

	// ==========================================================================
	//
	// ll_iter_next
	// Description: get the next element of a linkedlist through an iterator.
	// Returns the pointer to the element in case of success, NULL in case of error.
	//
	// ==========================================================================
	void *ll_iter_next(linkedListIterator *it);
#endif
