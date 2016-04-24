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
   // create_client_register
   // ===========================================================================
   int create_client_register(clientRegister **clReg);

   // ===========================================================================
   // cr_register_path
   // ===========================================================================
   int cr_register_path(clientRegister *clReg, clientData *data, char *path, registrationMode mod);

   // ===========================================================================
   // cr_unregister_path
   // ===========================================================================
   int cr_unregister_path(clientRegister *clReg, clientData *data, char *path);

   // ===========================================================================
   // print_client_register
   // ===========================================================================
   void print_client_register(clientRegister *cr);
#endif
