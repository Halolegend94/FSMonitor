#include <stdio.h>
#include <stdlib.h>
#include "include/settings_parser.h"
#include "include/mapping_structure.h"
#include "include/mem_management.h"
#include "include/mapping.h"
#include "include/syncmapping.h"
#include "include/myfile.h"

typedef struct _serverMonitor{
   int ID;
   int timeout;                  //frequence by which the monitor must check for updates
   char **serverPaths;           //paths monitored by the server
   int serverPathsCount;         //number of server paths
   char *mapName;                //the name of the mapping
   char *mapLockName;            //the name of the file that will be used as lock
   pSyncMapping mapLock;         //the map Lock structure
   pMapping mapping;             //the mapping structure
   long long mapSize;            //the size of the mapping
   mappingStructure *structure;  //the logic structure of the mapping
   //TODO: Clients list

} serverMonitor;

/*function prototypes*/
void check_params(int argc, char **argv, serverMonitor *server);
void load_settings(serverMonitor *server);
void initialize_mapping(serverMonitor *server);

// ===========================================================================
// MAIN
// ===========================================================================
int main(int argc, char **argv){

   serverMonitor server; //all the information about the server will be stored here
   check_params(argc, argv, &server);

   fprintf(stdout, "Loading settings..\n");
   //load settings
   load_settings(&server);
   //TODO: avvia server tcp

   fprintf(stdout, "Creating mapping and initialize memory management..\n");
   initialize_mapping(&server);

   //======================================================================

   printf("myid: %d\n", server.ID);
   //start daemon
   while(getchar() != 'q'){
      getchar();
      syncmapping_acquire(server.mapLock);
      update(server.structure);
      printf("J\n");
      print_mappingstructure_state(server.structure);
      syncmapping_release(server.mapLock);
   }

   //terminating cleanup
   delete_mapping(server.mapping);
   if(delete_file(server.mapName) == -1)
      printf("Error deleting the file associated with the mapping!");
   //TODO: nsacco de roba
}

// ===========================================================================
// check_params
// ===========================================================================
void check_params(int argc, char **argv, serverMonitor *server){
   if(argc != 2){
      fprintf(stderr, "serverMonitor: you need to pass the initial path to monitor.\n");
      exit(0);
   }

   if(is_directory(argv[1]) == 0){
      fprintf(stderr, "severMonitor: the specified path is not a valid directory.\n");
      exit(0);
   }
   int len = strlen(argv[1]);
   if(argv[1][len-1] == '\\' || argv[1][len-1] == '/') argv[1][len-1] = '\0';

   server->serverPaths = malloc(sizeof(char *));
   if(!(server->serverPaths)){
      fprintf(stderr, "serverMonitor: error while allocating memory.\n");
      exit(0);
   }
   server->serverPaths[0] = malloc(sizeof(char) * (strlen(argv[1]) + 1));
   if(!(server->serverPaths[0])){
      fprintf(stderr, "serverMonitor: error while allocating memory.\n");
      exit(0);
   }
   strcpy(server->serverPaths[0], argv[1]);
}

// ===========================================================================
// load_settings
// ===========================================================================
void load_settings(serverMonitor *server){
   settingsList loadedSettings;
   if(parse_settings("settings.txt", &loadedSettings) == -1){
      fprintf(stderr, "serverMonitor: error while loading the settings file.\n");
      exit(0);
   }
   char *t1 = get_setting_by_name("timeout", &loadedSettings);
   if(!t1){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }
   (*server).timeout = atoi(t1);
   free(t1);
   if((*server).timeout == 0){
      fprintf(stderr, "serverMonitor: timeout value not valid.\n");
      exit(0);
   }
   (*server).mapName = get_setting_by_name("mapName", &loadedSettings);
   if(!(*server).mapName){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }
   (*server).mapLockName = get_setting_by_name("mapLockName", &loadedSettings);
   if(!(*server).mapLockName){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }

   char *t2 = get_setting_by_name("mapSize", &loadedSettings);
   if(!t2){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }
   (*server).mapSize = atoi(t2) * (1024 * 1024);
   if((*server).mapSize == 0){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }
   free_settings_structure(&loadedSettings);
}

// ===========================================================================
// initialize_mapping
// ===========================================================================
void initialize_mapping(serverMonitor *server){
 /*create the lock that manages the access to the file mapping*/
   if(syncmapping_createlock(&(server->mapLock), server->mapLockName) == -1){
      fprintf(stderr, "serverMonitor: Error while creating the lock file.\n");
      exit(0);
   }

   /*now acquire the lock.*/
   if(syncmapping_acquire(server->mapLock) == -1){
      fprintf(stderr, "serverMonitor: error while acquiring the lock.\n");
      exit(0);
   }
   //CRITICAL SECTION
   /*check if the mapping alredy exists. If not, create one and map the file in memory. Start memory management*/
   int ret = create_mapping(&(server->mapping), server->mapName, server->mapSize * sizeof(char));
   if(ret == 0){ //the mapping was not there and has been created
      server->ID = 0;
      char *pointer = get_mapping_pointer(server->mapping);
      //start memory management

      if(pmm_initialize_management(pointer, server->mapSize, NULL) == -1){
         fprintf(stderr, "serverMonitor: error while starting memory management.\n");
         exit(0);
      }
      //initialize the mapping structure
      int ret1 = initialize_mapping_structure(NULL, &(server->structure), server->timeout, (server->serverPaths)[0]);
      if(ret1 == -1){
         fprintf(stderr, "serverMonitor: error while initializing mapping structure.\n");
         exit(0);
      }else if(ret1== -2){
         fprintf(stderr, "serverMonitor: the specified directory no longer exists.\n");
         /*since we are the first server and we fail, delete the mapping.*/
         delete_mapping(server->mapping);
         delete_file(server->mapName);
         syncmapping_release(server->mapLock);
         exit(0);
      }
   }else if(ret == 1){ //the mapping was alredy there

      if(open_mapping(&(server->mapping), server->mapName, server->mapSize * sizeof(char)) == -1){
         fprintf(stderr, "serverMonitor: error while opening the mapping.\n");
         exit(0);
      }
      char *pointer = get_mapping_pointer(server->mapping);
      //start memory management
      char *firstBlock = NULL;
      if(pmm_initialize_management(pointer, server->mapSize, (void **) &firstBlock) == -1){
         fprintf(stderr, "serverMonitor: error while starting memory management.\n");
         exit(0);
      }
      //initialize the mapping structure
      int ret1 = initialize_mapping_structure(firstBlock, &(server->structure), server->timeout, (server->serverPaths)[0]);
      if(ret1 == -1){
         fprintf(stderr, "serverMonitor: error while initializing mapping structure.\n");
         exit(0);
      } else if(ret1 == -2){
         fprintf(stderr, "serverMonitor: the specified directory no longer exists.\n");
         syncmapping_release(server->mapLock);
         exit(0);
      }

      server->ID = ret;
   }else if(ret == -1){
      fprintf(stderr, "serverMonitor: error while creating the mapping.\n");
      exit(0);
   }
   if(syncmapping_release(server->mapLock) == -1){
     fprintf(stderr, "Error while releasing the file lock.\n");
     exit(0);
   }
}
