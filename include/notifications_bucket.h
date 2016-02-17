#ifndef __notificationsBucket__
   #define __notificationsBucket__
   #include "mem_management.h"
   #include "common_utilities.h"
   #include "received_notification.h"
   #include "common_definitions.h"
   #include <stdio.h>
   #include <stdlib.h>

   /*the following structure defines a linked list of notifications to be used inside the
   file mapping*/
   typedef struct _notification{
         unsigned long off_path;
         long long size;
         long long modTimestamp;
         unsigned long off_perms;
         notificationType type; //common_definitions.h
         unsigned long off_next; //offset to the next notification
   } notification;

   typedef struct _notificationsBucket{
      unsigned int serverID;
      unsigned long off_path;
      unsigned long off_next; //offset to the next bucket
      unsigned long off_list; //offset to the notifications linked list
      unsigned long off_last_notification; //offset to the last notification in the linked list
   } notificationsBucket;

   // ===========================================================================
   // nb_create
   // ===========================================================================
   int nb_create(notificationsBucket **firstElement, unsigned int serverID, unsigned long off_pathName);

   // ===========================================================================
   // nb_add_bucket
   // ===========================================================================
   int nb_add_bucket(notificationsBucket *start, unsigned int serverID, unsigned long off_pathName);

   // ===========================================================================
   // nb_remove_bucket
   // ===========================================================================
   int nb_remove_bucket(notificationsBucket *start, unsigned int serverID, unsigned long off_pathName);

   // ===========================================================================
   // nb_push_notification
   // ===========================================================================
   int nb_push_notification(notificationsBucket *start, notification *notf);

   // ===========================================================================
   // nb_read_notifications
   // ===========================================================================
   int nb_read_notifications(notificationsBucket *start, unsigned int serverID, unsigned long off_pathName);
#endif
