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
		linkedItem *first; //the first element of the list
		linkedItem *last;  //the last element of the list
		int count;	  //the number of items
		int idCounter;
	} linkedList;
	
	typedef struct _linkedListIterator{
		linkedItem *current;
	}linkedListIterator;
	
	#include "linked_list.h"

	linkedList ll_create();

	int ll_add_element(linkedList *list, void *pElement);

	int ll_remove_element(linkedList *list, linkedItem *item);

	linkedListIterator ll_create_iterator(linkedList *list);

	void *ll_iter_next(linkedListIterator *it);

#endif
