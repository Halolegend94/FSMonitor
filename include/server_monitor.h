#ifndef __SERVERMONITOR__
   #define __SERVERMONITOR__
   #include "daemon.h"
   #include "server_commons.h"
   #include "signal_handler.h"

   //GLOBAL VARIABLES: serverStructure server

   #define DELAY_TOLLERANCE_FACTOR 1.3
   #define INITIAL_DELAY 2
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
   //
   // ==========================================================================
   void check_params(int argc, char **argv);

   // ==========================================================================
   //
   // load_settings
   // Description: this function loads all the settings from the settings.txt file
   // into the global structure server
   //
   // ==========================================================================
   void load_settings();

   // ===========================================================================
   //
   // ctrlc_handler
   // Description: the handler for the ctrl+c terminal input
   // NOTE: this will be executed on a different thread. Param is not used. See signal_handler.c
   // NOTE: this function call terminate_server
   //
   // ===========================================================================
   void *ctrlc_handler(void *p);

   // ==========================================================================
   //
   // initialize_server
   // Description: this function intializes all the structures needed by the process.
   // It creates the mapping, the lock, and performs the first scan for the path specified
   // through command line.
   //
   // ==========================================================================
   void initialize_server();

#endif
