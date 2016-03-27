#ifndef __SERVERMONITOR__
   #define __SERVERMONITOR__

   #include <stdio.h>
   #include <stdlib.h>
   #include "settings_parser.h"
   #include "mapping_structure.h"
   #include "mem_management.h"
   #include "mapping.h"
   #include "syncmapping.h"
   #include "myfile.h"
   #include "thread.h"
   #include "time_utilities.h"

   #define DELAY_TOLLERANCE_FACTOR 1.3

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
       pToThread *thread;            //object that points to the daemon thread, if this process owns it
       long long startUpTime;        //the time the server was started
       //TODO: Clients list
   } serverMonitor;

   /*function prototypes*/
   // ==========================================================================
   //
   // main [ENTRY POINT]
   //
   // ==========================================================================
   int main(int argc, char **argv);

   // ==========================================================================
   //
   // check_params
   // Description: this function check if the params to the main function are
   // correct (the path is valid).
   // Params:
   //    -  argc, argv: main params
   //    -  server: the pointer to the serverMonitor structure of the process.
   //
   // ==========================================================================
   void check_params(int argc, char **argv, serverMonitor *server);

   // ==========================================================================
   //
   // load_settings
   // Description: this function loads all the settings from the settings.txt file
   // into the structure pointed by "server" parameter
   // Params:
   //    -  server: the pointer to the serverMonitor structure of the process
   //
   // ==========================================================================
   void load_settings(serverMonitor *server);

   // ==========================================================================
   //
   // initialize_server
   // Description: this function intializes all the structures needed by the process.
   // It creates the mapping, the lock, and performs the first scan for the path specified
   // through command line.
   // Params:
   //    -  server: the pointer to the serverMonitor structure of the process
   //
   // ==========================================================================
   void initialize_server(serverMonitor *server);


   // ===========================================================================
   // cs_terminate_sever
   // NOTE: Assumption: this function is called inside a critical section
   // ===========================================================================
   void cs_terminate_sever(serverMonitor *str);


   // ===========================================================================
   // cs_terminate_sever_with_errors
   // NOTE: Assumption: this function is called inside a critical section
   // ===========================================================================
   void cs_terminate_sever_with_errors(serverMonitor *str);
#endif
