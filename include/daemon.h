#ifndef __DAEMON__
   #define __DAEMON__
   #include <stdio.h>
   #include <stdlib.h>
   #include "thread.h"
   #include "mapping_structure.h"
   #include "server_monitor.h"
   #include "syncmapping.h"

   // ==========================================================================
   //
   // create_daemon
   // Description: this function creates the daemon thread that is responsible of
   // periodically updating the mapping structure through scansions of the file system.
   // Params:
   //    -  str: a pointer to the serverMonitor structure of the process that the thread belongs to.
   //    -  thr: a pointer to an object pThread where will be saved all the needed information to
   //       to interact with the daemon thread.
   // Returns -1 in case of error, 0 in case of success
   //
   // ==========================================================================
   int create_daemon(serverMonitor *str, pToThread *thr);
#endif
