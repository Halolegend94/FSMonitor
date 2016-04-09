#include "../include/notifications_bucket.h"
#include "../include/mem_management.h"
#include "../include/watched_list.h"
#define TOT 2048

int main(void){

   char *memory = malloc(sizeof(char) * TOT);
   pmm_initialize_management(memory, TOT * sizeof(char), NULL);

   notificationsBucket *start;

   nb_create(&start, 0, "/home/cristian");

   nb_add_bucket(start, 1, "/home");


   nb_add_bucket(start, 2, "/home/lucia");


   nb_add_bucket(start, 3, "/gianfranco");


   nb_add_bucket(start, 0, "/home/lucia");

   printf("print1\n");
   nb_print_notification_buckets(start);

   nb_push_notification(start, "permessi", "/ciaom", 45, 34,0, 0); //persa
   nb_push_notification(start, "permessi", "/home", 45, 34, 1, 0);
   nb_push_notification(start, "permessi", "/home/cristian/file", 45, 34, 0, dimension);
   nb_push_notification(start, "permessi", "/home/lucia/files", 45, 34, 1, 0);
   nb_push_notification(start, "permessi", "/gianfranco/filess", 45, 34,0, 0);
   nb_print_notification_buckets(start);

   receivedNotification *nots;
   int totNot;
   nb_read_notifications(start, &nots, &totNot, 0);
   int x;
   for(x = 0; x < totNot; x++){
      printf("%s, %s, %lld, %lld, %d\n", nots[x].path,nots[x].perms, nots[x].modTimestamp, nots[x].size, nots[x].type);
   }
   nb_print_notification_buckets(start);
   nb_remove_bucket(&start, 1, "/home");
   nb_print_notification_buckets(start);
   nb_remove_bucket(&start, 0, "/home/cristian");
   nb_print_notification_buckets(start);
   free(memory);
   return 0;
}
