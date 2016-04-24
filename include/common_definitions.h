#ifndef __COMMONDEFINITIONS__
   #define __COMMONDEFINITIONS__

   /*the following enum type is used to specify the notification and receivedNotification type.*/
   typedef enum _notificationType {creation, deletion, perms, dimension, cambiocon} notificationType;

   typedef enum _registrationMode {RECURSIVE, NONRECURSIVE} registrationMode;

   #define TRUE 1
   #define FALSE 0
   #define PROG_SUCCESS 0
   #define PROG_ERROR -1
   #define PATH_NOT_ACCESSIBLE -2
   #define PATH_NOT_REGISTERED -3
   #define PATH_NOT_FOUND -4
   #define PATH_ALREADY_MONITORED -5
   #define PATH_UPDATED -6
#endif
