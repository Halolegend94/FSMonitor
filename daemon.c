#include "include/daemon.h"

extern serverStructure server;

//function prototypes
void *do_work(void *arg);

// ===========================================================================
// create_daemon
// ===========================================================================
int create_daemon(){
   int returnValue = create_thread(do_work, NULL, &(server.thread));
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
   /*serverStructure *str = (serverStructure *) arg;*/
   int refreshTime = 0;

   //DO FOREVER
   while(1){
       syncmapping_acquire(server.mapLock);
       if(server.isActive == 0 || server.ID != (server.structure)->daemonServer){
           syncmapping_release(server.mapLock);
           break;
       }

       if(update(server.structure) == -1){
           fprintf(stderr, "daemon: error while updating the mapping. Terminate execution.\n");
           cs_terminate_server();
       }

       refreshTime = (server.structure)->refreshTime;
       // print_mappingstructure_state(server.structure);
       syncmapping_release(server.mapLock);
       if(thread_sleep(refreshTime) == -1)break; //signal received
   }
   return NULL;
}
