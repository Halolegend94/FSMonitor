#ifndef __CLIENTPATHTREE_
   #define __CLIENTPATHTREE__
   #include <stdio.h>
   #include <string.h>
   #include <stdlib.h>
   #include "myfile.h" //for tokenize path
   #include "client_node_list.h"
   #include "networking.h"
   #include "common_definitions.h"

   /*this struct represents a client registration to a path.*/
   typedef struct _registration{
      clientNode *client;
      registrationMode mode;
   } registration;

   /*this struct represents a node in the client path tree. The client
   path tree is used to store all the paths the clients want the server to monitor.*/
   typedef struct _pathNode{
      char *name;                   //name of the folder this node represents
      struct _pathNode **children;   //subfolders
      int numChildren;               //number of subfolders
      registration **registrations;   //all the registrations for this path.
      int numRegistrations;          //number of registrations for this path
   } pathNode;


   // ===========================================================================
   // cpt_remove_client_registration
   // ===========================================================================
   int cpt_remove_client_registration(pathNode *root, char *path, clientNodeList *cnl, clientData *data);

   // ===========================================================================
   // cpt_add_client_registration
   // ===========================================================================
   int cpt_add_client_registration(pathNode *root, char *path, clientNodeList *cnl, clientData *data,
      registrationMode mode);

   // ===========================================================================
   // cpt_create
   // ===========================================================================
   int cpt_create(pathNode **root);

   // ===========================================================================
   // cpt_add_child
   // ===========================================================================
   int cpt_add_child(pathNode *father, char *name, pathNode **node);

   // ===========================================================================
   // cpt_remove_child
   // ===========================================================================
   int cpt_remove_child(pathNode *father, pathNode *child);


   void cpt_print_tree(pathNode *root);

#endif
