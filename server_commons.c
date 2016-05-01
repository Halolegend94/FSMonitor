#include "include/server_commons.h"
extern serverStructure server;

// ===========================================================================
// terminate_server
// NOTE: to use outside a critical section
// ===========================================================================
void terminate_server(){
   if(acquire_threadlock(server.activeLock) == PROG_ERROR){
      fprintf(stderr, "terminate_server: error while acquiring the activeLock. \n");
   }
    if(syncmapping_acquire(server.mapLock) == PROG_ERROR){
      fprintf(stderr, "terminate_server: error while acquiring the mapLock.\n");
   }
    cs_terminate_server();
}

// ===========================================================================
// cs_terminate_server
// NOTE: Assumption: this function is called inside a critical section
// ===========================================================================
void cs_terminate_server(){
   printf("\nTerminating server..");
   terminate_thread(server.tcpServer);
   (server.structure)->serverCounter--;
   if((server.structure)->daemonServer == server.ID) (server.structure)->daemonServer = -1;
   if(unregister_server(server.structure, server.ID, server.serverPaths, server.serverPathsCount) == PROG_ERROR){
      fprintf(stderr, "There have been errors while removing the server data from the mapping. "\
      "Probably it is corrupted.\n");
   }
   int numServers = (server.structure)->serverCounter;
   if(delete_mapping(server.mapping) == PROG_ERROR)
     fprintf(stderr, "terminate_server: error while unmapping the file.\n");

   if(numServers == 0) //if it is the last server, delete the mapping
      if(delete_file(server.mapName) == PROG_ERROR)
         fprintf(stderr, "terminate_server: error while deleting the mapping file. Delete it manually.\n");

   if(syncmapping_release(server.mapLock) == PROG_ERROR)
      fprintf(stderr, "terminate_server: error while releasing the lock.\n");
   if(syncmapping_closelock(server.mapLock) == PROG_ERROR)
      fprintf(stderr, "terminate_server: error while closing the lock handler.\n");

   //don't unlock activeLock to avoid data races
   free_sockets_library();
   printf("..done\n");
   exit(0);
}
