#include "include/daemon.h"

extern serverStructure server;

//function prototypes
void *do_work(void *arg);

// ===========================================================================
// create_daemon
// ===========================================================================
int create_daemon(){
   if(create_thread(do_work, NULL, &(server.thread)) == PROG_ERROR){
      fprintf(stderr, "create_daemon: error while creating the thread.\n");
      return PROG_ERROR;
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// do_work
// ===========================================================================
void *do_work(void *arg){
   /*serverStructure *str = (serverStructure *) arg;*/
   int refreshTime = 0;
   //DO FOREVER
   while(1){
       if(syncmapping_acquire(server.mapLock) == PROG_ERROR){
          fprintf(stderr, "daemon: error while acquiring the mapLock. Terminating execution.\n");
          exit(0);
       }
       if(server.isActive == 0 || server.ID != (server.structure)->daemonServer){
          syncmapping_release(server.mapLock);
          break;
       }
       if(update(server.structure) == -1){
           fprintf(stderr, "daemon: error while updating the mapping. Terminating execution.\n");
           cs_terminate_server();
       }

       refreshTime = (server.structure)->refreshTime - 1;
       // print_mappingstructure_state(server.structure);
       if(syncmapping_release(server.mapLock) == PROG_ERROR){
          fprintf(stderr, "daemon: error while releasing the mapLock. Terminating execution.\n");
          cs_terminate_server();
       }
       if(thread_sleep(refreshTime) == -1)break; //signal received
   }
   return NULL;
}
