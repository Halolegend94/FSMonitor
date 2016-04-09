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

	#define PROG_ERROR -1
	#define PROG_SUCCESS 0

	// ==========================================================================
	//
	// ll_create
	// Description: creates a new empty linked list
	// Params:
	// 	-	list: a pointer to a location where a pointer to a new empty linkedList will be stored.
	// Returns 0 in case of success, -1 in case of error.
	//
	// ==========================================================================
	int ll_create(linkedList **list);

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
	// Returns SUCCESS in case of success, ERROR in case of error.
	//
	// ==========================================================================
	int ll_remove_item(linkedList *list, linkedItem *Item);

	// ==========================================================================
	//
	// ll_free
	// Description: free all the memory occupied by the linkedItems (and the object they point at)
	// and the list itself.
	// Params:
	// 	-	list: the list to be freed
	//
	// ==========================================================================
	void ll_free(linkedList *list);
#endif
