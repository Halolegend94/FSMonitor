#ifndef __CLIENTNODELIST__
   #define __CLIENTNODELIST__
   #include "client_register_structures.h"

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
   int cnl_add_client_node(clientNodeList *root, clientData *data, clientNode **addedNode);

   // ===========================================================================
   // cnl_remove_client_node
   // ===========================================================================
   int cnl_remove_client_node(clientNodeList *root, clientNode *node);

   // ===========================================================================
   // cnl_add_notification
   // ===========================================================================
   int cnl_add_notification(clientNode *client, char *notString);

   // ===========================================================================
   // cnl_signal_deletion
   // ===========================================================================
   int cnl_signal_deletion(clientNode *client, char *fld);

   // ===========================================================================
   // cnl_print_list
   // ===========================================================================
   void cnl_print_list(clientNodeList *list);

   // ===========================================================================
   // cnl_send_notifications
   // ===========================================================================
   int cnl_send_notifications(clientNodeList *list, char *udpPort);
#endif
