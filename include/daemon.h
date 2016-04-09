#ifndef __DAEMON__
   #define __DAEMON__
   #include "server_commons.h"

   // ==========================================================================
   //
   // create_daemon
   // Description: this function creates the daemon thread that is responsible of
   // periodically updating the mapping structure through scansions of the file system.
   // Params:
   //    -  str: a pointer to the serverStructure structure of the process that the thread belongs to.
   //    -  thr: a pointer to an object pThread where will be saved all the needed information to
   //       to interact with the daemon thread.
   // Returns -1 in case of error, 0 in case of success
   //
   // ==========================================================================
   int create_daemon();
#endif
