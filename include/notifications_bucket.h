#ifndef __NOTIFICATIONSBUCKET__
   #define __NOTIFICATIONSBUCKET__
   #include "mem_management.h"
   #include "myfile.h"
   #include "received_notification.h"
   #include "common_definitions.h"
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>

   /*the following structure defines a linked list of notifications to be used inside the
   file mapping*/
   typedef struct _notification{
         unsigned long off_path;
         long long size;
         char isDir;
         long long modTimestamp;
         unsigned long off_perms;
         notificationType type; //common_definitions.h
         unsigned long off_next; //offset to the next notification
   } notification;

   typedef struct _notificationsBucket{
      int serverID;
      char deletionMark;
      unsigned long off_path;
      unsigned long off_next; //offset to the next bucket
      unsigned long off_list; //offset to the notifications linked list
      unsigned long off_last_notification; //offset to the last notification in the linked list
   } notificationsBucket;

   // ===========================================================================
   //
   // nb_create
   // Description: creates the data structure with a first bucket for the server that calls
   // this function.
   // Params:
   //    -  firstElement: a pointer that will contain a pointer to the new data structure
   //    -  serverID: server ID
   //    -  pathName: the path ot a directory that the calling server must monitor
   // Returns 0 in case of success, -1 otherwise
   //
   // ===========================================================================
   int nb_create(notificationsBucket **firstElement, int serverID, char *pathName);

   // ===========================================================================
   //
   // nb_add_bucket
   // Description: adds a bucket to the data structure. There is a bucket for each path
   // monitored by a server.
   // Params:
   //    -  start: the pointer to the first bucket of the linked list of buckets
   //    -  serverID: the ID of the server that will be associated to this new bucket
   //    -  pathName: the path of a directory that must be associated with the bucket
   // Returns 0 in case of success, -1 otherwise
   //
   // ===========================================================================
   int nb_add_bucket(notificationsBucket *start, int serverID, char *pathName);

   // ===========================================================================
   //
   // nb_remove_bucket
   // Description: removes a bucket (and all the notifications in it) from the linked list.
   // NOTE: the function modifies the start pointer, so it is necessary to update the data structure
   // after the function call, setting the right offset.
   // Params:
   //    -  start: a pointer to a pointer to the first element of the buckets linked list
   //    -  serverID: the ID of the server associated with the bucket to be removed
   //    -  pathName: the path of a directory associated with the bucket to be removed
   // Returns 0 in case of success, -1 otherwise.
   //
   // ===========================================================================
   int nb_remove_bucket(notificationsBucket *start, int serverID, char *pathName);

   // ===========================================================================
   //
   // nb_push_notification
   // Description: add a notification to every bucket which associated path contains the file
   // the notifications refers to.
   // Params:
   //    -  start: the pointer to the first element of the buckets linked list
   //    -  perms: the permissions string of the file the notification refers to
   //    -  path: the absolute path of the file the notification refers to
   //    -  size: the size of the file the notification refers to
   //    -  mod: the last modification time of the file the notification refers to
   //    -  isDir: tells if the file is a directory
   //    -  type: the notification type (creation, deletion, permissions, dimension)
   // Returns 0 in case of success, -1 otherwise
   //
   // ===========================================================================
   int nb_push_notification(notificationsBucket *start, char *perms, char *path, long long size,
                              long long mod, char isDir, notificationType type);

   // ===========================================================================
   //
   // nb_read_notifications
   // Description: gets all the pending notification for a server from all its associated buckets.
   // The notifications will be then deleted from the buckets.
   // Params:
   //    -  start: the first bucket of the linked list
   //    -  list: a pointer that will contain a pointer to an array of receivedNotifications
   //    -  count: a pointer to an integer that will contain the number of the list elements
   //    -  serverID: the ID of the server that queries for the associated notifications
   //    -  deletedPaths: a pointer that will point to a list of deleted paths monitored by the server
   //    -  numDeletedPaths: the number of elements in deletedPaths
   // Returns 0 in case of success, -1 otherwise
   //
   // ===========================================================================
   int nb_read_notifications(notificationsBucket *start, receivedNotification ***list, int *count,
      int serverID, char ***deletedPaths, int *numDeletedPaths);
   // ===========================================================================
   //
   // nb_exists_bucket
   // Description: this function is used to tells if the server already monitors the path (there is a
   // bucket whose path is a prefix of the given path) or if the server monitors a path that is contained
   // in the given path.
   // Params:
   //    -  start: the first bucket of the linked list
   //    -  serverID: the ID of the server
   //    -  path: the path string
   //    -  buck: a pointer where will be stored a pointer to the bucket whose associated path must be modified
   // Returns 1 and sets buck to NULL if the server already monitors the path, 1 if the server already monitors
   // a subfolder of the given directory and sets buck to point at the bucket of that subfolder. Returns 0
   // and sets buck to NULL if the server does not already monitor the path.
   //
   // ===========================================================================
   int nb_exists_bucket(notificationsBucket *start, int serverID, char *path, notificationsBucket **buck);

   // ===========================================================================
   //
   // nb_update_bucket_path
   // Description: updates the path associated with a bucket
   // Params:
   //    -  bucket: a pointer to a bucket
   //    -  path: the new path to be associated with the bucket
   // Returns 0 in case of success, -1 otherwise
   //
   // ===========================================================================
   int nb_update_bucket_path(notificationsBucket *bucket, char *path);

   // ===========================================================================
   //
   // nb_print_notification_buckets
   // Description: prints to stdout the content of the buckets array list. Used for debug purposes
   // Params:
   //    -  start: a pointer to the first element of the buckets linked listW
   //
   // ===========================================================================
   void nb_print_notification_buckets(notificationsBucket *start);
#endif
