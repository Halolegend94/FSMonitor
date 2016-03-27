#ifndef __RECEIVEDNOTIFICATION__
   #define __RECEIVEDNOTIFICATION__
   #include "common_definitions.h"
   #include "time_utilities.h"
   #include <stdio.h>
   #include <stdlib.h>
   #include <math.h>

   /*the following structure represents a notification received by a server and saved in its
   local memory. It is similar to notification, but it uses pointer instead of offsets.*/
   typedef struct _receivedNotification{
      char *path;
      long long size;
      long long modTimestamp;
      char *perms;
      notificationType type;
   } receivedNotification;

   // ===========================================================================
   //
   // get_string_representation
   // Description: this function takes as input a receivedNotification structure an
   // return its string representation.
   // Params:
   //    -  not: a pointer to the receivedNotification structure;
   //    -  serverStartupTime: the timestamp of when the server has started running
   // Returns: the string in case of success, NULL in case of error
   //
   // ===========================================================================
   char *get_string_representation(receivedNotification *not, long long serverStartupTime);

#endif
