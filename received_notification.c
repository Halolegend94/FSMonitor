#include "include/received_notification.h"

//private function prototypes
char *get_type_string(notificationType type);

// ===========================================================================
// get_string_representation
// ===========================================================================
char *get_string_representation(receivedNotification *not, unsigned long long serverStartupTime){

   long long modTime = get_relative_time(not->modTimestamp, serverStartupTime);
   if(modTime == -1){
       fprintf(stderr, "get_string_representation: error while getting the relative time.\n");
       printf("debug; %s %lli %s %lli", not->path, not->size, not->perms, modTime);
       return NULL;
   }
   int sizeLen = not->size < 10 ? 1 : ceil(log10(not->size));
   int modLen = modTime < 10 ? 1 :  ceil(log10(modTime));
   char *type = get_type_string(not->type);

   int len = strlen(not->path) + strlen(not->perms) + sizeLen + modLen  + strlen(type) + 9;

   char *tmp = malloc(sizeof(char) * (len + 1));
   if(!tmp){
      fprintf(stderr, "get_string_representation: error while allocating memory.\n");
      return NULL;
   }
   if(sprintf(tmp, "%s %lli %s %lli %s\r\n.\r\n", not->path, not->size, not->perms, modTime, type) < 0){
      fprintf(stderr, "get_string_representation: error while formatting the string in the buffer.\n");
      return NULL;
   }
   return tmp;
}

// ===========================================================================
// get_type_string
// ===========================================================================
char *get_type_string(notificationType type){
   switch (type) {
      case 0: return "creazione";
      case 1: return "cancellazione";
      case 2: return "permessi";
      case 3: return "dimensione";
      default: return "cambiocon";
   }
}
