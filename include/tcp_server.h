#ifndef __TCPSERVER__
   #define __TCPSERVER__

   #include "networking.h"
   #include "thread.h"
   #include "server_commons.h"
   #include "myfile.h"
   #include "networking.h"
   #include <stdio.h>
   #include <stdlib.h>

   #define INFO 0
   #define INNR 1
   #define ADDP 2
   #define ADDPNR 3
   #define DISC 4

   /*Struct used to pass arguments to the function create_client_request_handler, that is
   executed on a different thread.*/
   typedef struct _crhParams{
      clientData *data;
      int sock;
   } CRHParams;

   // ===========================================================================
   //
   // start_tcp_server
   // Description: start a tcp server to receive clients requests
   // Returns ERROR in case of error, SUCCESS on case of success
   //
   // ===========================================================================
   int start_tcp_server(void);

#endif
