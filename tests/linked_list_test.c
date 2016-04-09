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

   printf("delete middle.\n");
   ll_remove_item(&l, l.first->next);

   linkedListIterator i4 = ll_create_iterator(&l);

	while((p = ll_iter_next(&i4)) != NULL) printf("%s\n", (char *) p);

   printf("Delete first\n");

   ll_remove_item(&l, l.first);

   linkedListIterator i2 = ll_create_iterator(&l);
	while((p = ll_iter_next(&i2)) != NULL) printf("%s\n", (char *) p);

   ll_remove_item(&l, l.last);
   printf("remodev last\n");
   linkedListIterator i3 = ll_create_iterator(&l);
	while((p = ll_iter_next(&i3)) != NULL) printf("%s\n", (char *) p);



	return 0;

 }
