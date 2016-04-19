#ifndef __CLIENT__
   #define __CLIENT__

   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "thread.h"
   #include "networking.h"
   #include "params_parser.h"
   #include "settings_parser.h"

   #define PROG_ERROR -1
   #define PROG_SUCCESS 0
   
   typedef struct _clientStruct {
      char *tcpPort;
      char *udpPort;
      char *serverAddress;

   } clientStruct;

   /*function prototypes*/
   // ===========================================================================
   // is_command
   // ===========================================================================
   int is_command(char *str);

   // ===========================================================================
   // execute_command
   // ===========================================================================
   int execute_command(int w, optToken *comm);


#endif
