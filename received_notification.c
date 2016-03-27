#include "include/received_notification.h"

//private function prototypes
char *get_type_string(notificationType type);

// ===========================================================================
// get_string_representation
// ===========================================================================
char *get_string_representation(receivedNotification *not, long long serverStartupTime){
   long long modTime = get_relative_time(not->modTimestamp, serverStartupTime);
   char *type = get_type_string(not->type);
   int len = strlen(not->path) + strlen(not->perms) + ceil(log10(not->size)) +
               ceil(log10(modTime)) + strlen(type) + 9;
   char *tmp = malloc(sizeof(char) * (len + 1));
   if(!tmp){
      fprintf(stderr, "get_string_representation: error while allocating memory.\n");
      return NULL;
   }
   if(sprintf(tmp, "%s %lli %s %lli %s\r\n.\r\n", not->path, not->size, not->perms, modTime, type) < 0){
      fprintf(stderr, "get_string_representation: error while formatting the string in the buffer.\n");
      return -1;
   }
}

// ===========================================================================
// get_type_string
// ===========================================================================
char *get_type_string(notificationType type){
   switch (type) {
      case 0: return "creazione";
      case 1: return "cancellazione";
      case 2: return "permessi";
      case 3: return "dimensioni";
      default: return "cambiocon";
   }
}
