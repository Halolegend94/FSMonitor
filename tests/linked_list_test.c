 #include "../include/linked_list.h"

int main(void){
	linkedList l = ll_create();
	ll_add_item(&l, "Primo");
	ll_add_item(&l, "Secondo");
	ll_add_item(&l, "Terzo");
	ll_add_item(&l, "Quarto");

	linkedListIterator i = ll_create_iterator(&l);
	void *p;
	while((p = ll_iter_next(&i)) != NULL) printf("%s\n", (char *) p);
	return 0;

 }
