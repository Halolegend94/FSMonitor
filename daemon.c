#include "include/daemon.h"

//function prototypes
void *do_work(void *arg);

// ===========================================================================
// create_daemon
// ===========================================================================
int create_daemon(serverMonitor *str){
   int returnValue = create_thread(do_work, (void *) str, str->thread);
   if(returnValue == -1){
      fprintf(stderr, "create_daemon: error while creating the thread.\n");
      return -1;
   }
   return 0;
}

// ===========================================================================
// do_work
// ===========================================================================
void *do_work(void *arg){
   serverMonitor *str = (serverMonitor *) arg;
   int refreshTime = 0;

   //DO FOREVER
   while(1){
      syncmapping_acquire(str->mapLock);
      update(str->structure);
      refreshTime = (str->structure)->refreshTime;
      syncmapping_release(str->mapLock);
      thread_sleep(refreshTime);
   }
}
