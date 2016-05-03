#ifndef __CLIENTNODELIST__
   #define __CLIENTNODELIST__
   #include "client_register_structures.h"

   // ===========================================================================
   //
   // cnl_create
   // Description: create the client_node_list
   // Returns -1 in case of error, 0 otherwise
   //
   // ===========================================================================
   int cnl_create(clientNodeList **root);

   // ===========================================================================
   //
   // cnl_exists_node
   // Description: tells if a node with the same network data as "data" exists. If
   // this is the case, the pointer to that node will be stored in "client", otherwise
   // client points to NULL.
   // Returns 1 if true, 0 if false
   //
   // ===========================================================================
   int cnl_exists_node(clientNodeList *root, clientData *data, clientNode **client);

   // ===========================================================================
   //
   // cnl_add_client_node
   // Description: add a new client node.
   // Returns -1 in case of error, 0 in case of success
   //
   // ===========================================================================
   int cnl_add_client_node(clientNodeList *root, clientData *data, clientNode **addedNode);

   // ===========================================================================
   //
   // cnl_remove_client_node
   // Description: remove a client node from the structure
   // Returns -1 if node is not present in the client node list, 0 in case of
   // success.
   //
   // ===========================================================================
   int cnl_remove_client_node(clientNodeList *root, clientNode *node);

   // ===========================================================================
   //
   // cnl_add_notification
   // add a notification to the client's update list
   // Returns -1 in case of error, 0 in case of success.
   //
   // ===========================================================================
   int cnl_add_notification(clientNode *client, char *notString);

   // ===========================================================================
   //
   // cnl_signal_deletion
   // Description: this functions marks the client as to be removed after all the
   // pending notification will be sent.
   // Returns -1 in case of error, 0 in case of success.
   //
   // ===========================================================================
   int cnl_signal_deletion(clientNode *client, char *fld);

   // ===========================================================================
   // cnl_print_list
   // ===========================================================================
   void cnl_print_list(clientNodeList *list);

   // ===========================================================================
   //
   // cnl_send_notifications
   // Description: send the notifications in the unpdates lists to the clients,
   // using UDP.
   // Returns -1 in case of error, 0 in case of success
   //
   // ===========================================================================
   int cnl_send_notifications(clientNodeList *list, char *udpPort);
#endif
