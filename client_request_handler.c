#include "include/client_request_handler.h"

/*TODO this class handle all the command sent by the client. it must interact with the client
bucket and clients path tree.*/

extern serverStructure server;

// ===========================================================================
// create_client_request_handler
// ===========================================================================
void *create_client_request_handler(char *arg){
   CRHParams *param = (CRHParams *) arg;

   /*TODO:
      CREATE a new thread
      1. read the command
      2. call a function for every command
         2.1 - must modify the client bucket and path tree. In some cases, interact with the mappingStructure
         In case of ADDP: first try to add it to the mapping, in case of success, add to the pathtree
         think about a lock mechanism
      3. reply to client
      4. close connection*/

      free(arg);
}
