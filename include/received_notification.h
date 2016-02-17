#ifndef __RECEIVEDNOTIFICATION__
   #define __RECEIVEDNOTIFICATION__

   /*the following structure represents a notification received by a server and saved in its
   local memory. It is similar to notification, but it uses pointer instead of offsets.*/
   typedef struct _receivedNotification{
      char *path;
      long long size;
      long long modTimestamp;
      char *perms;
      notificationType type;
   } receivedNotification;

#endif
