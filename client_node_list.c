#include "include/client_node_list.h"
// ===========================================================================
// cnl_create
// ===========================================================================
int cnl_create(clientNodeList **root){
   *root = malloc(sizeof(clientNodeList));
   if(!(*root)){
      fprintf(stderr, "cb_create: error while allocating memory.\n");
      return PROG_ERROR;
   }
   (*root)->first = NULL;
   (*root)->last = NULL;
   return PROG_SUCCESS;
}

// ===========================================================================
// cnl_exists_node
// ===========================================================================
int cnl_exists_node(clientNodeList *root, clientData *data, clientNode **client){
   clientNode *current = root->first;
   while(current){
      if(strcmp(current->networkData->hostName, data->hostName) == 0){
         *client = current;
         return TRUE;
      }
      current = current->next;
   }
   *client = NULL;
   return FALSE;
}

// ===========================================================================
// __copy_networkData [SUPPORT FUNCTION]
// ===========================================================================
int __copy_networkData(clientData *data, clientData **copy){
   *copy = malloc(sizeof(clientData));
   if(!(*copy)){
      fprintf(stderr, "%s\n", "__copy_networkData: error while allocating memory.\n");
      return PROG_ERROR;
   }
   (*copy)->hostName = malloc(sizeof(char) * (strlen(data->hostName) + 1));
   if(!(*copy)->hostName){
      fprintf(stderr, "__copy_networkData: error while allocating memory.\n");
      return PROG_ERROR;
   }
   strcpy((*copy)->hostName, data->hostName);
   (*copy)->clientLen = data->clientLen;
   (*copy)->clientAddress = data->clientAddress;
   return PROG_SUCCESS;
}

// ===========================================================================
// cnl_add_client_node
// ===========================================================================
int cnl_add_client_node(clientNodeList *root, clientData *data, int numPaths, clientNode **addedNode){
   clientNode *newNode = malloc(sizeof(clientNode));
   if(!newNode){
      fprintf(stderr, "cnl_add_client_node: error while allocating memory.\n");
      return PROG_ERROR;
   }
   /*making a copy of network data.*/
   if(__copy_networkData(data, &(newNode->networkData)) == PROG_ERROR){
      fprintf(stderr, "cnl_add_client_node: error while copying the network data.\n");
      return PROG_ERROR;
   }
   newNode->next = NULL;
   if(ll_create(&(newNode->updates)) == PROG_ERROR){
      fprintf(stderr, "cnl_add_client_node: error while creating the linked list.\n");
      return PROG_ERROR;
   }
   newNode->numRegisteredPaths = numPaths;
   if(root->first == NULL){
      root->first = newNode;
   }else{
      (root->last)->next = newNode;
   }
   root->last = newNode;
   newNode->deletionMark = 0;
   *addedNode = newNode;
   return PROG_SUCCESS;
}

// ===========================================================================
// cnl_remove_client_node
// ===========================================================================
int cnl_remove_client_node(clientNodeList *root, clientNode *node){
   clientNode *current = root->first;
   clientNode *prev = NULL;
   while(current){
      if(current == node){
         if(prev == NULL){ //it's the first node
            root->first = current->next;
         }else{
            prev->next = current->next;
         }
         if(root->last == current){ //it's the last node
            root->last = prev;
         }

         //we can free the structure
         free(current->networkData->hostName);
         free(current->networkData);
         ll_free(current->updates);
         free(current);
         return PROG_SUCCESS;
      }
      prev = current;
      current = current->next;
   }
   return PROG_ERROR;
}

// ===========================================================================
// cnl_add_notification
// ===========================================================================
int cnl_add_notification(clientNode *client, char *notString){
   char *tcopy = malloc(sizeof(char) * (strlen(notString) + 1));
   if(!tcopy){
      fprintf(stderr, "cnl_add_notification: error while allocating memory.\n");
      return PROG_ERROR;
   }
   strcpy(tcopy, notString);
   if(ll_add_item(client->updates, tcopy) == PROG_ERROR){
      fprintf(stderr, "cnl_add_notification: error while adding a notification to a client's updates list.\n");
      return PROG_ERROR;
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// cnl_signal_deletion
// ===========================================================================
int cnl_signal_deletion(clientNode *client, char *fld){
   char *not = malloc(sizeof(char) * DEL_MSG_SIZE);
   if(!not){
      fprintf(stderr, "cnl_add_notification: error while allocating memory.\n");
      return PROG_ERROR;
   }
   if(sprintf(not, "The path \"%s\" has been deleted and one of your registrations has been removed accordingly.\n", fld) < 0){
      fprintf(stderr, "cnl_signal_deletion: error while creating a string.\n");
      return PROG_ERROR;
   }
   if(ll_add_item(client->updates, not) == PROG_ERROR){
      fprintf(stderr, "cnl_add_notification: error while adding a notification to a client's updates list.\n");
      return PROG_ERROR;
   }
   client->numRegisteredPaths--;
   if(client->numRegisteredPaths == 0) client->deletionMark = 1;
   else if(client->numRegisteredPaths < 0){ //DEBUG PURPOSES
      fprintf(stderr, "cnl_signal_deletion: inconsistent number of registed path.\n");
      return PROG_ERROR;
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// cnl_print_list
// ===========================================================================
void cnl_print_list(clientNodeList *list){
   printf("________CLIENT NODE LIST_____________\n\n");
   clientNode *current = list->first;
   while(current != NULL){
      printf("NODE: %s - marked: %d\n", current->networkData->hostName, current->deletionMark);
      printf("     Notification for this node:\n");
      linkedItem *ci = current->updates->first;
      while(ci){
         printf("       - %s\n", (char *) ci->item);
         ci = ci->next;
      }  
      current = current->next;
   }

}
