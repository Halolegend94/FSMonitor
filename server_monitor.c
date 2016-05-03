#include "include/server_monitor.h"

/*Global variables*/
 serverStructure server; //all the information about the server will be stored here

// ===========================================================================
// MAIN
// ===========================================================================
int main(int argc, char **argv){

   check_params(argc, argv);
   load_settings();

   //Install CTRL+C handler
   if(install_signal_handler(ctrlc_handler) == PROG_ERROR){
       fprintf(stderr, "serverMonitor: error while installing the ctrl+c handler.\n");
       exit(0);
   }

   initialize_server();  //include a critical section, the daemon starts in here

   //create the threadlock
   if(create_threadlock(&server.crLock) == PROG_ERROR){
      fprintf(stderr, "serverMonitor: error while creating the threadLock.\n");
      terminate_server();
   }

   //Start the TCP server
   if(start_tcp_server() == PROG_ERROR){
      fprintf(stderr, "serverMonitor: error while starting the tcp server.\n");
      terminate_server();
   }

   printf("Server (ID=%u) is active! Press CTRL+C to terminate the server.\n", server.ID);

   thread_sleep(INITIAL_DELAY); //so serverMonitor and daemon will be active at different time windows

   //**********************    MAIN LOOP   ********************************************
   mappingStructure *structure = server.structure; //mapping structure, for easier access
   while(1){
      if(thread_sleep(server.timeout) == -1) //signal received..
         while(1); //wait for imminent termination

      /*critical section 1*/
      if(acquire_threadlock(server.activeLock) == PROG_ERROR){
         fprintf(stderr, "daemon: error while acquiring the activeLock. Terminating execution.\n");
         cs1_terminate_server();
      }
      /**************************************************
      part 1: get notification from the mapping
      ***************************************************/
      if(syncmapping_acquire(server.mapLock) == -1){
         fprintf(stderr, "serverMonitor: error while acquiring the mapLock. Terminating the server.\n");
         exit(0); //just shutdown the server, we cannot clean the mapping whitout mutual exclusion
      }
      /*some checks first. If the daemon is signaled to be dead, start a new one.
      /*Also, we need to see if the deamon is inactive for a long period of time.
      This could mean that the process which owned it has been killed. If, instead, it is
      still alive, then it will kill itself once noticed the new daemon.*/
      unsigned long long current_time = get_current_time();
      if(current_time == 0){
         fprintf(stderr, "serverMonitor: error while getting the current time. Aborting execution.\n");
         cs2_terminate_server();
      }
      long long relTime = get_relative_time(current_time, structure->lastUpdate);
      if(relTime == -1){
         fprintf(stderr, "serverMonitor: error while getting the relative time.\n");
         cs2_terminate_server();
      }
      if(structure->daemonServer == -1 || (server.ID != structure->daemonServer &&
          relTime > (structure->refreshTime * DELAY_TOLLERANCE_FACTOR))){
         printf("Creating a new daemon..\n");
         if(create_daemon() == -1){
            fprintf(stderr, "serverMonitor: error while creating the daemon. Aborting execution..\n");
            cs2_terminate_server();
         }
         structure->daemonServer = server.ID; //this server is the owner of the daemon
      }

      //Now we can get notifications
      receivedNotification **notificationsList;
      int numNotifications;
      char **deletedPaths;
      int numDeletedPaths;
      if(get_notifications(server.structure, server.ID, &notificationsList, &numNotifications, &deletedPaths, &numDeletedPaths) == PROG_ERROR){
         fprintf(stderr, "serverMonitor: error while getting the notifications list. Aborting execution..\n");
         cs2_terminate_server();
      }
      if(syncmapping_release(server.mapLock) == PROG_ERROR){
         fprintf(stderr, "serverMonitor: error while releasing the mapLock.\n");
         cs2_terminate_server();
      }

      if(release_threadlock(server.activeLock) == PROG_ERROR){
         fprintf(stderr, "daemon: error while acquiring the activeLock. Terminating execution.\n");
         cs1_terminate_server();
      }
      /***************************************************************
      send notifications to clients
      ****************************************************************/
      if(acquire_threadlock(server.crLock) == PROG_ERROR){
         fprintf(stderr, "serverMonitor: error while acquiring the threadLock.\n");
         terminate_server();
      }
      //check if there are monitored paths that has been deleted
      int i; //counter
      if(numDeletedPaths > 0){
         if(server.serverPathsCount == numDeletedPaths){
            for(i = 0; i < server.serverPathsCount; i++) free(server.serverPaths[i]);
            free(server.serverPaths);
            server.serverPaths = NULL;
            server.serverPathsCount = 0;
         }else{
            char **newPathList = malloc(sizeof(char *) * (server.serverPathsCount - numDeletedPaths));
            if(!newPathList){
               fprintf(stderr, "serverMonitor: error while allocating memory.\n");
               terminate_server();
            }
            int j,k, found;
            for(i = 0, k = 0; i < server.serverPathsCount; i++){
               found = 0;
               for(j = 0; j < numDeletedPaths; j++){
                  if(strcmp(server.serverPaths[i], deletedPaths[j]) == 0){
                     found = 1;
                     break;
                  }
               }
               if(found){ //this one has not been deleted, so we copy it into the new array
                  newPathList[k++] = server.serverPaths[i];
               }else{
                  free(server.serverPaths[i]); //has been deleted
               }
            }
            free(server.serverPaths);
            server.serverPaths = newPathList;
            server.serverPathsCount = server.serverPathsCount - numDeletedPaths;
         }
         for(i = 0; i < numDeletedPaths; i++) free(deletedPaths[i]);
         free(deletedPaths);
      }
      //now put in queue all the received notifications.
      for(i = 0; i < numNotifications; i++){
         char *strNot = get_string_representation(notificationsList[i], server.startUpTime);
         if(!strNot){
            fprintf(stderr, "serverMonitor: error while getting the string representation of a notification.\n");
            terminate_server();
         }
         if(cpt_push_notification((server.clRegister)->treeRoot, notificationsList[i], strNot) == PROG_ERROR){
            fprintf(stderr, "serverMonitor: error while adding a notification to a client's queue.\n");
            terminate_server();
         }
         free(strNot);
      }
      print_mappingstructure_state(structure);
      print_client_register(server.clRegister);
      //send all the notifications
      if(cnl_send_notifications((server.clRegister)->nodeList, server.udpPort) == PROG_ERROR){
         fprintf(stderr, "serverMonitor: error while sending notifications to clients.\n");
         terminate_server();
      }
      //release Lock
      if(release_threadlock(server.crLock) == PROG_ERROR){
         fprintf(stderr, "serverMonitor: error while releasing the threadlock.\n");
         terminate_server();
      }
      //free resources
      for(i = 0; i < numNotifications; i++){
         free(notificationsList[i]->path);
         free(notificationsList[i]->perms);
         free(notificationsList[i]);
      }
      if(numNotifications > 0) free(notificationsList);
   }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
//                     SUPPORT FUNCTIONS
//
/////////////////////////////////////////////////////////////////////////////////////////

// ===========================================================================
// ctrlc_handler
// NOTE: this will be executed on a different thread. Param is not used. See signal_handler.c
// ===========================================================================
void *ctrlc_handler(void *p){
    printf("\nCTRL+C received: starting shutdown process, wait.\n");
    terminate_server();
    return NULL; //this will never be executed
}

// ===========================================================================
// check_params
// ===========================================================================
void check_params(int argc, char **argv){
   if(argc != 2){
      fprintf(stderr, "serverMonitor: you must provide the initial path to monitor.\n");
      exit(0);
   }
   char *path;
   if(is_absolute_path(argv[1]) == 0){
      fprintf(stderr, "serverMonitor: the path must be expressed as absolute path.\n");
      exit(0);
   }
    path = malloc(sizeof(char) * (strlen(argv[1]) +1));
    if(!path){
       fprintf(stderr, "serverMonitor: error while allocating memory.\n");
       exit(0);
    }
    strcpy(path, argv[1]);

   if(is_directory(path) == 0){
      fprintf(stderr, "serverMonitor: the specified path is not a valid directory.\n");
      exit(0);
   }
   int len = strlen(path);
   if(path[len-1] == '\\' || path[len-1] == '/') path[len-1] = '\0';

   server.serverPaths = malloc(sizeof(char *));
   if(!(server.serverPaths)){
      fprintf(stderr, "serverMonitor: error while allocating memory.\n");
      exit(0);
   }
   server.serverPaths[0] = path;
   server.serverPathsCount = 1;
}

// ===========================================================================
// load_settings
// ===========================================================================
void load_settings(){
    printf("Loading settings..\n");
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
   server.timeout = atoi(t1);
   free(t1);
   if(server.timeout == 0){
      fprintf(stderr, "serverMonitor: timeout value not valid.\n");
      exit(0);
   }
   if(server.timeout < 3){
      fprintf(stderr, "serverMonitor: refresh time must be greater than 2 seconds.\n");
      exit(0);
   }
   server.mapName = get_setting_by_name("mapName", &loadedSettings);
   if(!server.mapName){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }

   char *t2 = get_setting_by_name("mapSize", &loadedSettings);
   if(!t2){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }
   server.mapSize = atoi(t2) * (1024 * 1024);
   if(server.mapSize == 0){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name\n");
      exit(0);
   }
   free(t2);
   server.udpPort = get_setting_by_name("udpPort", &loadedSettings);
   if(!server.udpPort){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name.\n");
      exit(0);
   }
   server.tcpPort = get_setting_by_name("tcpPort", &loadedSettings);
   if(!server.tcpPort){
      fprintf(stderr, "serverMonitor: error while calling get_setting_by_name.\n");
      exit(0);
   }
   t2 = get_setting_by_name("maxConn", &loadedSettings);
   server.maxClientConnections = atoi(t2);
   if(server.maxClientConnections == 0){
      fprintf(stderr, "serverMonitor: error while loading the maxConn param.\n");
      exit(0);
   }
   free_settings_structure(&loadedSettings);

}

// ===========================================================================
// initialize_server
// ===========================================================================
void initialize_server(){
    printf("Initializing data structures..\n");
    //create the activelock
    if(create_threadlock(&server.activeLock) == PROG_ERROR){
       fprintf(stderr, "serverMonitor: error while creating the threadLock.\n");
       exit(0);
    }
   /*create the lock that manages the access to the file mapping*/
   if(syncmapping_createlock(&(server.mapLock)) == -1){
      fprintf(stderr, "serverMonitor: Error while creating the lock file.\n");
      exit(0);
   }
   /*now acquire the lock.*/
   if(syncmapping_acquire(server.mapLock) == -1){
      fprintf(stderr, "serverMonitor: error while acquiring the lock.\n");
      exit(0);
   }
   /*check if the mapping alredy exists. If not, create one and map the file in
   memory. Start memory management*/
   int crMapReturnValue = create_mapping(&(server.mapping), server.mapName, server.mapSize * sizeof(char));

   if(crMapReturnValue == 0){ //the mapping was not there and has been created
      server.ID = 0; //so this is the first server
      char *pointer = get_mapping_pointer(server.mapping);
      //start memory management
      if(pmm_initialize_management(pointer, server.mapSize, NULL) == -1){
         fprintf(stderr, "serverMonitor: error while starting memory management.\n");
         syncmapping_release(server.mapLock);
         syncmapping_deletelock(server.mapLock);
         delete_file(server.mapName);
         exit(0);
      }
      //initialize the mapping structure
      int ims = initialize_mapping_structure(NULL, &(server.structure), server.timeout, (server.serverPaths)[0]);
      if(ims == -1){
         fprintf(stderr, "serverMonitor: error while initializing mapping structure.\n");
         syncmapping_release(server.mapLock);
         syncmapping_deletelock(server.mapLock);
         delete_file(server.mapName);
         exit(0);
      }else if(ims == -2){
         fprintf(stderr, "serverMonitor: the specified directory no longer exists.\n");
         /*since we are the first server and we fail, delete the mapping.*/
         syncmapping_release(server.mapLock);
         syncmapping_deletelock(server.mapLock);
         delete_file(server.mapName);
         exit(0);
      }
      /*since it's the first process, we need to start the daemon that update the data structure*/
      ims = create_daemon();
      if(ims == -1){
         fprintf(stderr, "serverMonitor: error while creating the daemon.\n");
         exit(0);
      }
   }else if(crMapReturnValue == 1){ //the mapping was alredy there
      if(open_mapping(&(server.mapping), server.mapName, server.mapSize * sizeof(char)) == -1){
         fprintf(stderr, "serverMonitor: error while opening the mapping.\n");
         syncmapping_release(server.mapLock);
         syncmapping_closelock(server.mapLock);
         exit(0);
      }
      char *pointer = get_mapping_pointer(server.mapping);
      //start memory management
      char *firstBlock = NULL;
      if(pmm_initialize_management(pointer, server.mapSize, (void **) &firstBlock) == -1){
         fprintf(stderr, "serverMonitor: error while starting memory management.\n");
         syncmapping_release(server.mapLock);
         syncmapping_closelock(server.mapLock);
         exit(0);
      }
      //initialize the mapping structure
      int ims = initialize_mapping_structure(firstBlock, &(server.structure),
                server.timeout, (server.serverPaths)[0]);
      if(ims == -1){
         fprintf(stderr, "serverMonitor: error while initializing mapping structure.\n");
         syncmapping_release(server.mapLock);
         syncmapping_closelock(server.mapLock);
         exit(0);
      } else if(ims == -2){
         fprintf(stderr, "serverMonitor: the specified directory no longer exists.\n");
         syncmapping_release(server.mapLock);
         syncmapping_closelock(server.mapLock);
         exit(0);
      }
      server.ID = ims;
   }else if(crMapReturnValue == -1){
      fprintf(stderr, "serverMonitor: error while creating the mapping.\n");
      syncmapping_release(server.mapLock);
      syncmapping_closelock(server.mapLock);
      exit(0);
   }
   //FINISH with the mapping
   if(syncmapping_release(server.mapLock) == -1){
     fprintf(stderr, "Error while releasing the file lock.\n");
     exit(0);
   }
   //set the startup time
   server.startUpTime = get_current_time();
   if(server.startUpTime == 0){
      fprintf(stderr, "serverMonitor: error while getting the current time.\n");
      exit(0);
   }
}
