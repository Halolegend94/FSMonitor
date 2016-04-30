#ifndef __CLIENTPATHTREE_
   #define __CLIENTPATHTREE__
   #include "client_register_structures.h"
   #include "client_node_list.h"

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

   // ===========================================================================
   // cpt_push_notification
   // ===========================================================================
   int cpt_push_notification(pathNode *root, receivedNotification *not, char *stringNot);

   int cpt_clean_tree(pathNode *node);

   void cpt_print_tree(pathNode *root);

#endif
