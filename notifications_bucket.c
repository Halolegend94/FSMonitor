#include "include/notifications_bucket.h"

/*used in nb_get_notifications*/
#define MIN_NUMBER 40
#define NUM_INCREMENT 30

// ===========================================================================
// nb_create
// NOTE: an offset to the path name is passed because it is already on the mapping
// for the watched list.
// ===========================================================================
int nb_create(notificationsBucket **firstElement, unsigned int serverID, unsigned long off_pathName){
   *firstElement = pmm_malloc(sizeof(notificationsBucket));
   if(!(*firstElement)){
      fprintf(stderr, "nb_create: error while allocating memory.\n");
      return -1;
   }
   (*firstElement)->serverID = serverID;
   (*firstElement)->off_path = off_pathName;
   (*firstElement)->off_next = 0;
   (*firstElement)->off_list = 0;
   (*firstElement)->off_last_notification = 0;
   return 0;
}

// ===========================================================================
// nb_add_bucket
// ===========================================================================
int nb_add_bucket(notificationsBucket *start, unsigned int serverID, unsigned long off_pathName){
   notificationsBucket *lastBucket = start;
   //go to the last bucket
   while(lastBucket->off_next != 0){
      lastBucket = pmm_offset_to_pointer(lastBucket->off_next);
   }

   notificationsBucket *newBucket = pmm_malloc(sizeof(notificationsBucket));
   if(!newBucket){
      fprintf(stderr, "nb_add_bucket: error while allocating memory.\n");
      return -1;
   }
   newBucket->serverID = serverID;
   newBucket->off_path = off_pathName;
   newBucket->off_list = 0;
   newBucket->off_next = 0;
   newBucket->off_last_notification = 0;
   lastBucket->off_next = pmm_pointer_to_offset(newBucket);
   return 0;
}

// ===========================================================================
// nb_remove_bucket
// NOTE: the function modifies the start pointer, so it is necessary to update the data structure
// after the function call, setting the right offset.
// ===========================================================================
int nb_remove_bucket(notificationsBucket **start, unsigned int serverID, unsigned long off_pathName){
   notificationsBucket *previous = NULL;
   notificationsBucket *currentBucket = *start;

   do{
      if(currentBucket->serverID == serverID && currentBucket->off_path == off_pathName){
         //bucket found. We have to delete it. First we update the linked list pointers
         if(previous != NULL){
            previous->off_next = currentBucket->off_next;
         }else if(currentBucket->off_next == 0){
            //the bucket to be deleted is the last one. We set start to point to NULL
            *start = NULL;
         }else{
            *start = pmm_offset_to_pointer(currentBucket->off_next);
         }
         //delete the bucket and related notifications
         if(currentBucket->off_list != 0){
            //there are notifications, delete them
            do{
               notification *not = pmm_offset_to_pointer(currentBucket->off_list);
               pmm_free(pmm_offset_to_pointer(not->off_perms));
               pmm_free(pmm_offset_to_pointer(not->off_path));
               if(not->off_next != 0){
                  notification *next = pmm_offset_to_pointer(not->off_next);
                  pmm_free(not);
                  not = next;
               }else{
                  pmm_free(not);
                  break;
               }
            }while(1);
         }
         //delete the bucket structure
         pmm_free(currentBucket);
         return 0;
      }else if(currentBucket->off_next != 0){
         previous = currentBucket;
         currentBucket = pmm_offset_to_pointer(currentBucket->off_next);
      }else{
         fprintf(stderr, "nb_remove_bucket: error, no bucket found.\n");
         return -1;
      }
   }while(1);
}

// ===========================================================================
// nb_push_notification
// ===========================================================================
int nb_push_notification(notificationsBucket *start, char *perms, char *path, long long size,
                           long long mod, notificationType type){
   notificationsBucket *current = start;
   do{
      char *monitoredPath = pmm_offset_to_pointer(current->off_path);
      if(is_prefix(path, monitoredPath)){ //we work with absolute paths
         //the server need to receive this notification
         notification *newNotification = pmm_malloc(sizeof(notification));
         unsigned long newOffset = pmm_pointer_to_offset(newNotification);

         if(!newNotification){
            fprintf(stderr, "nb_push_notification: error while allocating memory.\n");
            return -1;
         }

         //get the last notification
         if(current->off_last_notification != 0){
            notification *last = pmm_offset_to_pointer(current->off_last_notification);
            last->off_next = newOffset;
         }else{
            //set the list pointer if it is the first notification
            current->off_list = newOffset;
         }
         //either way, we need to udate the pointer to the last notification
         current->off_last_notification = newOffset;

         //now we set the structure values
         int pathLen = strlen(path);
         int permsLen = strlen(perms);
         char *tPath = pmm_malloc(sizeof(char) * (pathLen + 1));
         if(!tPath){
            fprintf(stderr, "nb_push_notification: error while allocating memory.\n");
            return -1;
         }
         strcpy(tPath, path);
         newNotification->off_path = pmm_pointer_to_offset(tPath);

         char *tPerms = pmm_malloc(sizeof(char) * (permsLen + 1));
         if(!tPerms){
            fprintf(stderr, "nb_push_notification: error while allocating memory.\n");
            return -1;
         }
         strcpy(tPerms, perms);
         newNotification->off_perms = pmm_pointer_to_offset(tPerms);

         newNotification->size = size;
         newNotification->modTimestamp = mod;
         newNotification->type = type;
      }
      if(current->off_next == 0){
         break;
      }else{//go to the next bucket
         current = pmm_offset_to_pointer(current->off_next);
      }

   }while(1);
   return 0;
}

// ===========================================================================
// nb_read_notifications
// NOTE: list will contain a pointer to a memory area in the process memory
// and not in the mapping.
// ===========================================================================
int nb_read_notifications(notificationsBucket *start, receivedNotification **list, int *count, unsigned int serverID){
      *count = 0;
      *list = NULL;
      int currentCapacity = MIN_NUMBER;
      notificationsBucket *current = start;
      do{
         if(current->serverID == serverID && current->off_list != 0){ //we have found a bucket to read
            if(*list == NULL){
               *list = malloc(sizeof(notification) * currentCapacity);
               if(!(*list)){
                  fprintf(stderr, "nb_read_notifications: error while allocating memory.\n");
                  return -1;
               }
            }
            //get all the notification
            notification *currentNot = pmm_offset_to_pointer(current->off_list);
            current->off_list = 0;
            current->off_last_notification = 0;
            do{
               if((*count) >= currentCapacity){ //check if there is enough space
                  currentCapacity += NUM_INCREMENT;
                  *list = realloc(*list, currentCapacity * sizeof(notification));
                  if(!(*list)){
                     fprintf(stderr, "nb_read_notifications: error while reallocating memory.\n");
                     return -1;
                  }
               }
               (*list)[*count].size = currentNot->size;
               (*list)[*count].modTimestamp = currentNot->modTimestamp;
               char *mPath = pmm_offset_to_pointer(currentNot->off_path);
               char *mPerms = pmm_offset_to_pointer(currentNot->off_perms);
               char *tpath = malloc(sizeof(char) * (strlen(mPath) + 1));
               if(!tpath){
                  fprintf(stderr, "%s\n", "nb_read_notifications: error while allocating memory.");
                  return -1;
               }
               strcpy(tpath, mPath);
               (*list)[*count].path = tpath;
               char *tperms = malloc(sizeof(char) * (strlen(mPerms) + 1));
               if(!tperms){
                  fprintf(stderr, "%s\n", "nb_read_notifications: error while allocating memory.");
                  return -1;
               }
               strcpy(tperms, mPerms);
               (*list)[*count].perms = tperms;
               (*list)[*count].type = currentNot->type;
               *count = *count + 1;

               //delete the notification from the linked list
               pmm_free(mPath);
               pmm_free(mPerms);

               //go to the next notification, if exists
               if(currentNot->off_next != 0){
                  notification *prev = currentNot;
                  currentNot = pmm_offset_to_pointer(currentNot->off_next);
                  pmm_free(prev);
               }else{
                  pmm_free(currentNot);
                  break;
               }
            }while(1);
         }
         if(current->off_next == 0) break;
         current = pmm_offset_to_pointer(current->off_next);
      }while(1);
      return 0;
}

// ===========================================================================
// nb_print_notification_buckets
// ===========================================================================
void nb_print_notification_buckets(notificationsBucket *start){
   if(!start){
      fprintf(stderr, "nb_print_notification_buckets: param is null.\n");
      return;
   }
   printf("------------------------------------------------- buckets list\n");
   notificationsBucket *current = start;
   do{
         printf("===> server ID: %u, path: %s\n", current->serverID, pmm_offset_to_pointer(current->off_path));
         //get all the notification
         notification *currentNot = pmm_offset_to_pointer(current->off_list);
         do{
            char *mPath = pmm_offset_to_pointer(currentNot->off_path);
            printf("------ path: %s\n", mPath);
            //go to the next notification, if exists
            if(currentNot->off_next != 0){
               currentNot = pmm_offset_to_pointer(currentNot->off_next);
               }else{
               break;
            }
         }while(1);
      }
      if(current->off_next == 0) break;
      current = pmm_offset_to_pointer(current->off_next);
   }while(1);

   printf("------------------------------------------------------------\n");
}
