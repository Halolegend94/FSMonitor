#include "include/notifications_bucket.h"

/*used in nb_read_notifications*/
#define MIN_NUMBER 40
#define NUM_INCREMENT 30

// ===========================================================================
// nb_create
// ===========================================================================
int nb_create(notificationsBucket **rootElement, int serverID, char *pathName){
   *rootElement = pmm_malloc(sizeof(notificationsBucket));
   if(!(*rootElement)){
      fprintf(stderr, "nb_create: error while allocating memory.\n");
      return PROG_ERROR;
   }
   (*rootElement)->serverID = -1; //it's a mark
   (*rootElement)->off_path = 0;
   (*rootElement)->off_list = 0;

   /*create the first server bucket*/
   notificationsBucket *firstElement = pmm_malloc(sizeof(notificationsBucket));
   if(!firstElement){
      fprintf(stderr, "nb_create: error while allocating memory.\n");
      return PROG_ERROR;
   }
   char *Pname =  concatenate_path(pathName, "");
   if(!Pname){
      fprintf(stderr, "Error while concatenating strings.\n");
      return PROG_ERROR;
   }
   char *mapName = pmm_malloc(sizeof(char) * (strlen(Pname) + 1));
   if(!mapName){
     fprintf(stderr, "nb_add_bucket: error while allocating memory.\n");
     return PROG_ERROR;
   }
   strcpy(mapName, Pname);
   free(Pname);
   firstElement->serverID = serverID;
   firstElement->off_path = pmm_pointer_to_offset(mapName);
   firstElement->off_next = 0;
   firstElement->off_list = 0;
   firstElement->deletionMark = 0;
   firstElement->off_last_notification = 0;

   (*rootElement)->off_next = pmm_pointer_to_offset(firstElement);
   return 0;
}

// ===========================================================================
// nb_add_bucket
// ===========================================================================
int nb_add_bucket(notificationsBucket *start, int serverID, char *pathName){
   notificationsBucket *lastBucket = start;
   //go to the last bucket
   while(lastBucket->off_next != 0){
     lastBucket = pmm_offset_to_pointer(lastBucket->off_next);
   }

   notificationsBucket *newBucket = pmm_malloc(sizeof(notificationsBucket));
   if(!newBucket){
      fprintf(stderr, "nb_add_bucket: error while allocating memory.\n");
      return PROG_ERROR;
   }
   char *Pname =  concatenate_path(pathName, "");
   if(!Pname){
      fprintf(stderr, "Error while concatenating strings.\n");
      return PROG_ERROR;
   }
   char *mapName = pmm_malloc(sizeof(char) * (strlen(Pname) + 1));
   if(!mapName){
     fprintf(stderr, "nb_add_bucket: error while allocating memory.\n");
     return PROG_ERROR;
   }
   strcpy(mapName, Pname);
   free(Pname);
   newBucket->serverID = serverID;
   newBucket->off_path = pmm_pointer_to_offset(mapName);
   newBucket->off_list = 0;
   newBucket->off_next = 0;
   newBucket->deletionMark = 0;
   newBucket->off_last_notification = 0;
   lastBucket->off_next = pmm_pointer_to_offset(newBucket);
   return 0;
}

// ===========================================================================
// nb_remove_bucket
// ===========================================================================
int nb_remove_bucket(notificationsBucket *start, int serverID, char *pathName){
   notificationsBucket *previous = start;
   notificationsBucket *currentBucket = start;
   char *mPath =  concatenate_path(pathName, "");
   if(!mPath){
      fprintf(stderr, "Error while concatenating strings.\n");
      return PROG_ERROR;
   }
   do{
      if(currentBucket->serverID == serverID && //the first bucket can never match this condition
          fname_compare(pmm_offset_to_pointer(currentBucket->off_path), mPath) == 0){

         //bucket found. We have to delete it. First we update the linked list pointers
         previous->off_next = currentBucket->off_next;
         //delete the bucket and related notifications
         if(currentBucket->off_list != 0){
            //there are notifications, delete them
            notification *not = pmm_offset_to_pointer(currentBucket->off_list);
            do{
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
         pmm_free(pmm_offset_to_pointer(currentBucket->off_path));
         pmm_free(currentBucket);
         free(mPath);
         return 0;
      }else if(currentBucket->off_next != 0){
         previous = currentBucket;
         currentBucket = pmm_offset_to_pointer(currentBucket->off_next);
      }else{
         fprintf(stderr, "nb_remove_bucket: error, no bucket found.\n");
         free(mPath);
         return PROG_ERROR;
      }
   }while(1);
}

// ===========================================================================
// nb_push_notification
// ===========================================================================
int nb_push_notification(notificationsBucket *start, char *perms, char *path,
    long long size, long long mod, char isDir, notificationType type){
   notificationsBucket *current = NULL;
   /*the following code is used to correctly verify if the path file is in the
    subtree of the current bucket*/
    if(start->off_next == 0)
        return PROG_SUCCESS; //no one is listening
    else
        current = pmm_offset_to_pointer(start->off_next);

   char *tempName = concatenate_path(path, "");
   if(!tempName){
      fprintf(stderr, "Error while concatenating strings.\n");
      return PROG_ERROR;
   }
   do{
      if(current->deletionMark == 1){
         current = pmm_offset_to_pointer(current->off_next);
         continue;
      }
      char *monitoredPath = pmm_offset_to_pointer(current->off_path);
      if(is_prefix(monitoredPath, tempName) == 1) current->deletionMark = 1;

      if(is_prefix(tempName, monitoredPath) == 1 || current->deletionMark){
         //the server need to receive this notification
         notification *newNotification = pmm_malloc(sizeof(notification));
         unsigned long newOffset = pmm_pointer_to_offset(newNotification);

         if(!newNotification){
            fprintf(stderr, "nb_push_notification: error while allocating memory.\n");
            return PROG_ERROR;
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
         int pathLen = current->deletionMark ? strlen(monitoredPath) : strlen(tempName);
         int permsLen = strlen(perms);
         char *tPath = pmm_malloc(sizeof(char) * (pathLen + 1));
         if(!tPath){
            fprintf(stderr, "nb_push_notification: error while allocating memory.\n");
            return PROG_ERROR;
         }
         strcpy(tPath, current->deletionMark ? monitoredPath : tempName);
         tPath[pathLen - 1] = '\0';
         newNotification->off_path = pmm_pointer_to_offset(tPath);

         char *tPerms = pmm_malloc(sizeof(char) * (permsLen + 1));
         if(!tPerms){
            fprintf(stderr, "nb_push_notification: error while allocating memory.\n");
            return PROG_ERROR;
         }
         strcpy(tPerms, perms);
         newNotification->off_perms = pmm_pointer_to_offset(tPerms);

         newNotification->size = size;
         newNotification->modTimestamp = mod;
         newNotification->type = type;
         newNotification->isDir = isDir;
         newNotification->off_next = 0;
      }
      if(current->off_next == 0){
         break;
      }else{//go to the next bucket
         current = pmm_offset_to_pointer(current->off_next);
      }

   }while(1);
   free(tempName);
   return PROG_SUCCESS;
}

// ===========================================================================
// nb_read_notifications
// NOTE: list will contain a pointer to a memory area in the process memory
// and not in the mapping.
// ===========================================================================
int nb_read_notifications(notificationsBucket *start, receivedNotification ***list, int *count,
   int serverID, char ***deletedPaths, int *numDeletedPaths){
      *numDeletedPaths = 0;
      notificationsBucket *previous = start;
      *count = 0;
      *list = NULL;
      int currentCapacity = MIN_NUMBER;
      notificationsBucket *current = start;
      do{
         if(current->serverID == serverID && current->off_list != 0){ //we have found a bucket to read
            if(*list == NULL){
               *list = malloc(sizeof(notification *) * currentCapacity);
               if(!(*list)){
                  fprintf(stderr, "nb_read_notifications: error while allocating memory.\n");
                  return PROG_ERROR;
               }
            }
            //get all the notification
            notification *currentNot = pmm_offset_to_pointer(current->off_list);
            current->off_list = 0;
            current->off_last_notification = 0;
            do{
               if((*count) >= currentCapacity){ //check if there is enough space
                  currentCapacity += NUM_INCREMENT;
                  *list = realloc(*list, currentCapacity * sizeof(notification *));
                  if(!(*list)){
                     fprintf(stderr, "nb_read_notifications: error while reallocating memory.\n");
                     return PROG_ERROR;
                  }
               }
               receivedNotification *localNot = malloc(sizeof(notification));
               if(!localNot){
                  fprintf(stderr, "nb_read_notifications: error while allocating memory.\n");
                  return PROG_ERROR;
               }
               (*list)[*count] = localNot;
               localNot->size = currentNot->size;
               localNot->modTimestamp = currentNot->modTimestamp;
               char *mPath = pmm_offset_to_pointer(currentNot->off_path);
               char *mPerms = pmm_offset_to_pointer(currentNot->off_perms);
               char *tpath = malloc(sizeof(char) * (strlen(mPath) + 1));
               if(!tpath){
                  fprintf(stderr, "%s\n", "nb_read_notifications: error while allocating memory.");
                  return PROG_ERROR;
               }
               strcpy(tpath, mPath);
               localNot->path = tpath;
               char *tperms = malloc(sizeof(char) * (strlen(mPerms) + 1));
               if(!tperms){
                  fprintf(stderr, "%s\n", "nb_read_notifications: error while allocating memory.");
                  return PROG_ERROR;
               }
               strcpy(tperms, mPerms);
               localNot->perms = tperms;
               localNot->isDir = currentNot->isDir;
               localNot->type = currentNot->type;
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
            /*delete if deletion mark is present.*/
            if(current->deletionMark){
               char *mPath = pmm_offset_to_pointer(current->off_path);
               if(*numDeletedPaths == 0){
                  *deletedPaths = malloc(sizeof(char *));
                  if(!(*deletedPaths)){
                     fprintf(stderr, "nb_read_notifications: error while allocating memory.\n");
                     return PROG_ERROR;
                  }
               }else{
                  *deletedPaths = realloc(*deletedPaths, sizeof(char *) * (*numDeletedPaths + 1));
                  if(!(*deletedPaths)){
                     fprintf(stderr, "nb_read_notifications: error while allocating memory.\n");
                     return PROG_ERROR;
                  }
               }
               int len = strlen(mPath);
               int pos = (*numDeletedPaths)++;
               (*deletedPaths)[pos] = malloc(sizeof(char) * (len + 1 ));
               if(!((*deletedPaths)[pos])){
                  fprintf(stderr, "nb_read_notifications: error while allocating memory.\n");
                  return PROG_ERROR;
               }
               strcpy((*deletedPaths)[pos], mPath);
               (*deletedPaths)[pos][len - 1] = '\0';
               notificationsBucket *b = current;
               current = pmm_offset_to_pointer(current->off_next);
               previous->off_next = b->off_next; //delete from the chain
               pmm_free(mPath);
               pmm_free(b);
               continue;
            }
         }
         if(current->off_next == 0) break;
         previous = current;
         current = pmm_offset_to_pointer(current->off_next);
      }while(1);
      return PROG_SUCCESS;
}

// ===========================================================================
// nb_exists_bucket
// ===========================================================================
int nb_exists_bucket(notificationsBucket *start, int serverID, char *path, notificationsBucket **buck){
   /*build the internal representation of the path*/
   char *tpath = concatenate_path(path, "");
   if(!tpath){
     fprintf(stderr, "nb_exists_bucket: error while concatenating a path.\n");
     return PROG_ERROR;
   }
   notificationsBucket *current = start;
	do{
      char *mpath = pmm_offset_to_pointer(current->off_path);
      if(current->serverID == serverID){
         if(fname_compare(mpath, tpath) == 0){
         	//we have found the bucket
            *buck = NULL;
            free(tpath);
         	return 1;
         }else if(is_prefix(tpath, mpath)){ //we already monitor it
            *buck = NULL;
            free(tpath);
            return 1;

         }else if(is_prefix(mpath, tpath)){ // we already monitor a subfolder
            *buck = current;
            free(tpath);
            return 1;
         }
      }
      if(current->off_next == 0) {
         free(tpath);
         return 0;
      }
      current = pmm_offset_to_pointer(current->off_next);
   }while(1);
}

// ===========================================================================
// nb_update_bucket_path
// ===========================================================================
int nb_update_bucket_path(notificationsBucket *bucket, char *path){
/*build the internal representation of the path*/
char *tpath = concatenate_path(path, "");
if(!tpath){
   fprintf(stderr, "nb_update_bucket_path: error while concatenating a path.\n");
   return PROG_ERROR;
}
char *mapPath = pmm_malloc(sizeof(char) * (strlen(tpath) + 1));
if(!mapPath){
   fprintf(stderr, "nb_update_bucket_path: error while allocating memory.\n");
   return PROG_ERROR;
}
strcpy(mapPath, tpath);
free(tpath);
bucket->off_path = pmm_pointer_to_offset(mapPath);
return 0;
}

// ===========================================================================
// nb_print_notification_buckets [DEBUG]
// ===========================================================================
void nb_print_notification_buckets(notificationsBucket *start){
   if(start->off_next == 0){
      fprintf(stderr, "nb_print_notification_buckets: there is no bucket to show.\n");
      return;
   }
   printf("---------------------------------------- buckets list\n");
   notificationsBucket *current = pmm_offset_to_pointer(start->off_next);
   do{
      printf("===> server ID: %u, path: %s\n", current->serverID, (char *) pmm_offset_to_pointer(current->off_path));
      if(current->off_list != 0){

         //get all the notification
         notification *currentNot = pmm_offset_to_pointer(current->off_list);
         do{
            char *mPath = pmm_offset_to_pointer(currentNot->off_path);
            printf("------ path: %s, type: %d\n", mPath, currentNot->type);
            //go to the next notification, if exists
            if(currentNot->off_next != 0){
               currentNot = pmm_offset_to_pointer(currentNot->off_next);
            }else{
               break;
            }
         } while(1);
      }
      if(current->off_next == 0) break;
      current = pmm_offset_to_pointer(current->off_next);
   } while(1);

   printf("------------------------------------------------------------\n");
}
