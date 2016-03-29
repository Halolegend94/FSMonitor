#include "include/server_monitor.h"

/*Global variables*/
 serverStructure server; //all the information about the server will be stored here

// ===========================================================================
// MAIN
// ===========================================================================
int main(int argc, char **argv){

   check_params(argc, argv);
   load_settings();
   initialize_server();  //include a critical section, the daemon starts in here

   //Install CTRL+C handler
   if(install_signal_handler(ctrlc_handler) == -1){
       fprintf(stderr, "serverMonitor: error while installing the ctrl+c handler.\n");
       terminate_server();
   }

   //TODO: avvia server tcp
   printf("Server (ID=%u) is active!\n", server.ID);

   thread_sleep(INITIAL_DELAY); //so serverMonitor and daemon will be active at different time windows

   //**********************    MAIN LOOP   ********************************************
   mappingStructure *structure = server.structure; //mapping structure, for easier access
   while(1){
         if(thread_sleep(server.timeout) == -1) //signal received..
            while(1); //wait for imminent termination

         syncmapping_acquire(server.mapLock); //Begin critical section
         /*the following check is done to avoid race conditions:
            - Signal ctrl+c is received and the handler executes, leave the critical section,
            and it is stopped before it can terminate the process.
            - This thread (monitor) enters the critical section and accesses the mapping
            that has been deleted. ERROR.
        */
         if(server.isActive == 0){
             syncmapping_release(server.mapLock);
             break;
         }
         /*some checks first. If the daemon is signaled to be dead, start a new one.
         /*Also, we need to see if the deamon is inactive for a long period of time.
         This could mean that the process which owned it has been killed. If, instead, it is
         still alive, then it will kill itself once noticed the new daemon.*/
         unsigned long long current_time = get_current_time();
         if(current_time == 0){
            fprintf(stderr, "serverMonitor: error while getting the current time. Aborting execution.\n");
            cs_terminate_server();
         }
         if(structure->daemonServer == -1 || (server.ID != structure->daemonServer &&
            get_relative_time(current_time, structure->lastUpdate) > (structure->refreshTime * DELAY_TOLLERANCE_FACTOR))){
            printf("Creating a new daemon..\n");
            if(create_daemon() == -1){
               fprintf(stderr, "serverMonitor: error while creating the daemon. Aborting execution..\n");
               cs_terminate_server();
            }
            structure->daemonServer = server.ID; //this server is the owner of the daemon
         }

         //Now we can get notifications
         receivedNotification *notificationsList;
         int count;
         if(get_notifications(server.structure, server.ID, &notificationsList, &count) == -1){
            fprintf(stderr, "serverMonitor: error while getting the notifications list. Aborting execution..\n");
            cs_terminate_server();
         }
         //send notifications to clients
         int i;
         for(i = 0; i < count; i++){
            printf("%s\n", get_string_representation(&(notificationsList[i]), server.startUpTime));
         }
         syncmapping_release(server.mapLock);
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
      fprintf(stderr, "serverMonitor: you need to pass the initial path to monitor.\n");
      exit(0);
   }
   if(is_absolute_path(argv[1]) == 0){
      fprintf(stderr, "serverMonitor: the path must be expressed as absolute path.\n");
      exit(0);
   }
   if(is_directory(argv[1]) == 0){
      fprintf(stderr, "serverMonitor: the specified path is not a valid directory.\n");
      exit(0);
   }
   int len = strlen(argv[1]);
   if(argv[1][len-1] == '\\' || argv[1][len-1] == '/') argv[1][len-1] = '\0';

   server.serverPaths = malloc(sizeof(char *));
   if(!(server.serverPaths)){
      fprintf(stderr, "serverMonitor: error while allocating memory.\n");
      exit(0);
   }
   server.serverPaths[0] = malloc(sizeof(char) * (strlen(argv[1]) + 1));
   if(!(server.serverPaths[0])){
      fprintf(stderr, "serverMonitor: error while allocating memory.\n");
      exit(0);
   }
   strcpy(server.serverPaths[0], argv[1]);
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
   free_settings_structure(&loadedSettings);

   server.isActive = 1;
}

// ===========================================================================
// initialize_server
// ===========================================================================
void initialize_server(){
    printf("Initializing data structures..\n");
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
   if(server.startUpTime == -1){
      fprintf(stderr, "serverMonitor: error while getting the current time.\n");
      exit(0);
   }
}
