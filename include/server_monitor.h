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
       pToThread *thread;             //object that points to the daemon thread, if this process owns it
       //TODO: Clients list
   } serverMonitor;

   /*function prototypes*/
   void check_params(int argc, char **argv, serverMonitor *server);
   void load_settings(serverMonitor *server);
   void initialize_mapping(serverMonitor *server);
#endif
