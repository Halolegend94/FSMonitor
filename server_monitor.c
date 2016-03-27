#include "include/server_monitor.h"



// ===========================================================================
// cs_terminate_sever
// NOTE: Assumption: this function is called inside a critical section
// ===========================================================================
void cs_terminate_sever(serverMonitor *str){
   fprintf(stderr, "Terminating server...\n");
   (str->structure)->serverCounter--;
   if((str->structure)->daemonServer == str->ID) (str->structure)->daemonServer = -1;
   if(unregister_server(str->structure, str->ID, str->serverPaths, str->serverPathsCount) == -1){
      printf(stderr, "there have been errors while removing the server data from the mapping. Probably it is corrupted.\n");
   }
   syncmapping_release(str->mapLock);
   if((str->structure)->serverCounter == 0){ //if it is the last server, delete the mapping
      syncmapping_deletelock(str->mapLock);
      if(delete_file(str->mapName) == -1) fprintf(stderr, "Error while deleting the mapping file. Delete it manually.\n");
   }else{
      syncmapping_closelock(str->mapLock);
   }
   exit(0);
}

// ===========================================================================
// cs_terminate_sever_with_errors
// NOTE: Assumption: this function is called inside a critical section
// ===========================================================================
void cs_terminate_sever_with_errors(serverMonitor *str){
   fprintf(stderr, "Some errors have happened (errors message should have appeared before this one),"\
   " so the server cannot continue executing. Before terminating, i'll try to leave the mapping normally.\n");
   cs_terminate_sever(str);
}



// ===========================================================================
// MAIN
// ===========================================================================
int main(int argc, char **argv){

   serverMonitor server; //all the information about the server will be stored here
   check_params(argc, argv, &server);

   fprintf(stdout, "Loading settings..\n");
   //load settings
   load_settings(&server);

   /********CRITICAL SECTION**********/
   initialize_server(&server);  //the daemon starts in here
   /********END CRITICAL SECTION******/

   //TODO: avvia server tcp

   printf("Server (ID=%u) is active!\n", server.ID);

   //**********************    MAIN LOOP   ********************************************
   mappingStructure *structure = server.structure; //more clean in this way
   while(1){
         thread_sleep(server.timeout);
         syncmapping_acquire(server.mapLock);
         /*some checks first. If the daemon is signaled to be dead, start a new one.
         /*Also, we need to see if the deamon is inactive for a long period of time.
         This could mean that the process which owned it has been killed. If it was
         still alive, then it would kill itself having noticed a new daemon*/
         long long current_time = get_current_time();
         if(current_time == -1){
            fprintf(stderr, "serverMonitor: error while getting the current time. Exit. \n");
            cs_terminate_sever_with_errors(&server);
         }
         if(structure->daemonServer == -1 || (current_time - structure->lastUpdate) > (structure->refreshTime * 1.5)){
            if(create_daemon(server) == -1){
               fprintf(stderr, "serverMonitor: error while creating the daemon.\n");
               cs_terminate_sever_with_errors(&server);
            }
            structure->daemonServer = server.ID; //this server is the owner of the daemon
         }

         //Now we can get notifications


         //send notifications to clients

   }
}

// ===========================================================================
// check_params
// ===========================================================================
void check_params(int argc, char **argv, serverMonitor *server){
   if(argc != 2){
      fprintf(stderr, "serverMonitor: you need to pass the initial path to monitor.\n");
      exit(0);
   }
   if(is_absolute_path(argv[1]) == 0){
      fprintf(stderr, "serverMonitor: the path must be expressed as absolute path.\n");
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
   server->serverPathsCount = 1;
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
   free(t2);
   free_settings_structure(&loadedSettings);
}

// ===========================================================================
// initialize_server
// ===========================================================================
void initialize_server(serverMonitor *server){
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
   /*check if the mapping alredy exists. If not, create one and map the file in
   memory. Start memory management*/
   int crMapReturnValue = create_mapping(&(server->mapping), server->mapName, server->mapSize * sizeof(char));

   if(crMapReturnValue == 0){ //the mapping was not there and has been created
      server->ID = 0; //so this is the first server
      char *pointer = get_mapping_pointer(server->mapping);
      //start memory management
      if(pmm_initialize_management(pointer, server->mapSize, NULL) == -1){
         fprintf(stderr, "serverMonitor: error while starting memory management.\n");
         syncmapping_release(server->mapLock);
         syncmapping_deletelock(server->mapLock);
         delete_file(server->mapName);
         exit(0);
      }
      //initialize the mapping structure
      int ims = initialize_mapping_structure(NULL, &(server->structure), server->timeout, (server->serverPaths)[0]);
      if(ims == -1){
         fprintf(stderr, "serverMonitor: error while initializing mapping structure.\n");
         syncmapping_release(server->mapLock);
         syncmapping_deletelock(server->mapLock);
         delete_file(server->mapName);
         exit(0);
      }else if(ims == -2){
         fprintf(stderr, "serverMonitor: the specified directory no longer exists.\n");
         /*since we are the first server and we fail, delete the mapping.*/
         syncmapping_release(server->mapLock);
         syncmapping_deletelock(server->mapLock);
         delete_file(server->mapName);
         exit(0);
      }

      /*since it's the first process, we need to start the daemon that update the data structure*/
      ims = create_daemon(server);
      if(ims == -1){
         fprintf(stderr, "serverMonitor: error while creating the daemon.\n");
         exit(0);
      }
   }else if(crMapReturnValue == 1){ //the mapping was alredy there
      if(open_mapping(&(server->mapping), server->mapName, server->mapSize * sizeof(char)) == -1){
         fprintf(stderr, "serverMonitor: error while opening the mapping.\n");
         syncmapping_release(server->mapLock);
         syncmapping_closelock(server->mapLock);
         exit(0);
      }
      char *pointer = get_mapping_pointer(server->mapping);
      //start memory management
      char *firstBlock = NULL;
      if(pmm_initialize_management(pointer, server->mapSize, (void **) &firstBlock) == -1){
         fprintf(stderr, "serverMonitor: error while starting memory management.\n");
         syncmapping_release(server->mapLock);
         syncmapping_closelock(server->mapLock);
         exit(0);
      }
      //initialize the mapping structure
      int ims = initialize_mapping_structure(firstBlock, &(server->structure),
                server->timeout, (server->serverPaths)[0]);
      if(ims == -1){
         fprintf(stderr, "serverMonitor: error while initializing mapping structure.\n");
         syncmapping_release(server->mapLock);
         syncmapping_closelock(server->mapLock);
         exit(0);
      } else if(ims == -2){
         fprintf(stderr, "serverMonitor: the specified directory no longer exists.\n");
         syncmapping_release(server->mapLock);
         syncmapping_closelock(server->mapLock);
         exit(0);
      }
      server->ID = ims;
   }else if(crMapReturnValue == -1){
      fprintf(stderr, "serverMonitor: error while creating the mapping.\n");
      syncmapping_release(server->mapLock);
      syncmapping_closelock(server->mapLock);
      exit(0);
   }
   //FINISH with the mapping
   if(syncmapping_release(server->mapLock) == -1){
     fprintf(stderr, "Error while releasing the file lock.\n");
     exit(0);
   }
   //set the startup time
   server->startUpTime = get_current_time();
   if(server->startUpTime == -1){
      fprintf(stderr, "serverMonitor: error while getting the current time.\n");
      return -1;
   }
}
