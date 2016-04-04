#ifndef __TCPSERVER__
   #define __TCPSERVER__

   #include "networking.h"
   #include "thread.h"
   #include "server_commons.h"
   #include "networking.h"
   #include <stdio.h>
   #include <stdlib.h>

   // ===========================================================================
   //
   // start_tcp_server
   // Description: start a tcp server to receive clients requests
   // Returns ERROR in case of error, SUCCESS on case of success
   //
   // ===========================================================================
   int start_tcp_server(void);

#endif
