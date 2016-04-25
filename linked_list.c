#include "include/linked_list.h"

// ==========================================================================
// ll_create
// ==========================================================================
int ll_create(linkedList **list){
	*list = malloc(sizeof(linkedList));
	if(!(*list)){
		fprintf(stderr, "ll_create: error while creating a linkedList.\n");
		return PROG_ERROR;
	}
	(*list)->idCounter = 0;
	(*list)->count = 0;
	(*list)->first = NULL;
	(*list)->last = NULL;
	return PROG_SUCCESS;
}

// ==========================================================================
// ll_add_item
// ==========================================================================
int ll_add_item(linkedList *list, void *pItem){
	linkedItem *el = malloc(sizeof(linkedItem));
	if(!el){
		fprintf(stderr, "ll_add_item: error while allocating memory.\n");
		return -1;
	}
	el->item = pItem;
	el->next = NULL;
	list->count++;
	el->id = list->idCounter++;

	if(list->first == NULL) {
		el->prev = NULL;
		list->first = el;
	}else{
		el->prev = list->last;
		(list->last)->next = el;
	}
	list->last = el;
	return 0;
}

// ==========================================================================
// ll_remove_item
// ==========================================================================
int ll_remove_item(linkedList *list, linkedItem *item){
	linkedItem *current = list->first;
	if(current == NULL){
		fprintf(stderr, "ll_remove_elment: the list is empty.\n");
		return -1;
	}

	while(item->id != current->id){
		current = current->next;
		if(current == NULL) break;
	}
	if(current == NULL){
		fprintf(stderr, "ll_remove_item: item not found.\n");
		return-1;
	}

	if(current->id == (list->first)->id){
		list->first = current->next;
		free(current);
		return 0;

	}else if(current->id == (list->last)->id){
		linkedItem *newLast = (list->last)->prev;
		newLast->next = NULL;
		free(list->last);
		list->last = newLast;
	}else{
		(current->prev)->next = current->next;
		free(current);
	}
	list->count--;
	return 0;
}

// ==========================================================================
// ll_free
// ==========================================================================
void ll_free(linkedList *list){
	linkedItem *item = list->first;
	while(item){
		free(item->item);
		linkedItem *next = item->next;
		free(item);
		item = next;
	}
	free(list);
}
