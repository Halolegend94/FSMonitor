#ifndef __CLIENT_REQUEST_HANDLER__
   #define __CLIENT_REQUEST_HANDLER__

   #include "server_commons.h"
   #include "networking.h"
   #include <stdio.h>
   #include <stdlib.h>

   /*Struct used to pass arguments to the function create_client_request_handler, that is
   executed on a different thread.*/
   typedef struct _crhParams{
      clientData data;
      int sock;
   } CRHParams;

   // ===========================================================================
   // create_client_request_handler
   // ===========================================================================
   void *create_client_request_handler(char *arg);
#endif
