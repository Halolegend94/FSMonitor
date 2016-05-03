#ifndef __CLIENTREGISTER__
   #define __CLIENTREGISTER__

   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "client_path_tree.h"
   #include "client_node_list.h"
   #include "common_definitions.h"
   #include "myfile.h"

   typedef struct _clientRegister{
      clientNodeList *nodeList;      //client node list
      pathNode *treeRoot;    //tree of registered path
   } clientRegister;


   // ===========================================================================
   //
   // create_client_register
   // Description: intitializes the clientRegister structure.
   // Params:
   //    -  clReg a pointer to a location where a pointer to the allocated structure
   //       will be saved.
   // Returns -1 in case of error, 0 otherwise
   // ===========================================================================
   int create_client_register(clientRegister **clReg);

   // ===========================================================================
   //
   // cr_register_path
   // Description: insert a new registration requested by a client for a path.
   // Params:
   //    -  clReg: a pointer to the clientRegister structure
   //    -  data: the network data of the client (ip address, hostname)
   //    -  path: the path to which the client wants to be registered to.
   //    -  mod: the registration mode: RECURSIVE, NONRECURSIVE
   // Returns PROG_ERROR, PATH_ALREADY_REGISTERED, PATH_UPDATED, PROG_SUCCESS
   //
   // ===========================================================================
   int cr_register_path(clientRegister *clReg, clientData *data, char *path, registrationMode mod);

   // ===========================================================================
   //
   // cr_unregister_path
   // Description: remove a client registration.
   // Params:
   //    -  clReg: a pointer to the clientRegister structure
   //    -  data: a pointer to the client network data
   //    -  path: the path involved.
   // Returns PROG_SUCCESS, PATH_NOT_REGISTERED, PROG_ERROR
   //
   // ===========================================================================
   int cr_unregister_path(clientRegister *clReg, clientData *data, char *path);

   // ===========================================================================
   //
   // print_client_register
   // Description: print a representation of the structure and its content
   //
   // ===========================================================================
   void print_client_register(clientRegister *cr);
#endif
