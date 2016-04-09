#ifndef __CLIENTNODELIST__
   #define __CLIENTNODELIST__
   #include "networking.h"
   #include "linked_list.h"
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "common_definitions.h"

   typedef struct _clientNode{
      clientData *networkData;
      linkedList *updates;
      int numRegisteredPaths;
      struct _clientNode *next;
   } clientNode;

   typedef struct _clientNodeList{
      clientNode *first;
      clientNode *last;
   } clientNodeList;

   // ===========================================================================
   // cnl_create
   // ===========================================================================
   int cnl_create(clientNodeList **root);

   // ===========================================================================
   // cnl_exists_node
   // ===========================================================================
   int cnl_exists_node(clientNodeList *root, clientData *data, clientNode **client);

   // ===========================================================================
   // cnl_add_client_node
   // ===========================================================================
   int cnl_add_client_node(clientNodeList *root, clientData *data, int numPaths, clientNode **addedNode);

   // ===========================================================================
   // cnl_remove_client_node
   // ===========================================================================
   int cnl_remove_client_node(clientNodeList *root, clientNode *node);

   // ===========================================================================
   // cnl_print_list
   // ===========================================================================
   void cnl_print_list(clientNodeList *list);
#endif
