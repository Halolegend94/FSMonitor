#ifndef __CLIENTREGISTER__
   #define __CLIENTREGISTER__

   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "cr_lock.h"
   #include "client_path_tree.h"
   #include "client_node_list.h"
   #include "common_definitions.h"
   #include "myfile.h"

   typedef struct _clientRegister{
      pCRLock lock;
      clientNodeList *nodeList;      //client node list
      pathNode *treeRoot;    //tree of registered path
   } clientRegister;


   typedef enum _clientRequestType {INFO, ADDP, DISC, INNR} clientRequestType;


   /*some rules:
      1) in general, try to add the path to the tree. In case of success, use the pointer
      to the client bucket list to find the client bucket or create a new one.

      addPath*/

   // ===========================================================================
   // create_client_register
   // ===========================================================================
   int create_client_register(clientRegister **clReg);

   int cr_register_path(clientRegister *clReg, clientData *data, char *path, registrationMode mod);

   // ===========================================================================
   // cr_unregister_path
   // ===========================================================================
   int cr_unregister_path(clientRegister *clReg, clientData *data, char *path);

   void print_client_register(clientRegister *cr);
#endif
