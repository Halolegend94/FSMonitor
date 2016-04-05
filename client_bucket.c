#ifndef __CLIENTBUCKET__
   #define __CLIENTBUCKET__
   #include "networking.h"
   #include "linked_list.h"
   #include <stdio.h>
   #include <stdlib.h>

   typedef struct _clientBucket{
      clientData *networkData;
      linkedList *updates;
      int numRegisteredPaths;
      struct _clientBucket *next;
   } clientBucket;

#endif
//TODO
