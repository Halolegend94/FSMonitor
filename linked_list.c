#include "include/linked_list.h"

// ==========================================================================
// ll_create
// ==========================================================================
linkedList ll_create(){
	linkedList l;
	l.idCounter = 0;
	l.count = 0;
	l.first = NULL;
	l.last = NULL;
	return l;
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
	int len = list->count++;
	el->id = list->idCounter++;

	if(len == 0) {
		list->first = el;
	}else{
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
// ll_create_iterator
// ==========================================================================
linkedListIterator ll_create_iterator(linkedList *list){
	linkedListIterator i;
	i.current = list->first;
	return i;
}
// ==========================================================================
// ll_iter_next
// ==========================================================================
void *ll_iter_next(linkedListIterator *it){
	if(it->current == NULL) return NULL;
	void *val = (it->current)->item;
	it->current = (it->current)->next;
	return val;
}
