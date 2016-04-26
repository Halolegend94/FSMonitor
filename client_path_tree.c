#include "include/client_path_tree.h"

/*function prototypes*/
int __support_remove_client_registration(pathNode *node, char **tokens, int numTokens, int index,
   int *mustBeRemoved, clientData *data, clientNode **client);

// ===========================================================================
// cpt_create
// ===========================================================================
int cpt_create(pathNode **root){
      *root = malloc(sizeof(pathNode));
      if(!(*root)){
         fprintf(stderr, "cpt_create: error while allocating memory.\n");
         return PROG_ERROR;
      }
      (*root)->name = NULL;
      (*root)->children = NULL;
      (*root)->registrations = NULL;
      (*root)->numChildren = 0;
      (*root)->numRegistrations = -1;
      return PROG_SUCCESS;
}

// ===========================================================================
// cpt_contains_child  [SUPPPORT FUNCTION]
// ===========================================================================
int cpt_contains_child(pathNode *father, char *name, pathNode **node){
   int i; //counter
   for(i = 0; i < father->numChildren; i++){
      if(fname_compare(name, (father->children[i])->name) == 0){
         *node = father->children[i];
         return TRUE;
      }
   }
   return FALSE;
}

// ===========================================================================
// cpt_add_child  [SUPPPORT FUNCTION]
// ===========================================================================
int cpt_add_child(pathNode *father, char *name, pathNode **node){
   father->children = realloc(father->children, sizeof(pathNode *) * (father->numChildren + 1));
   if(!(father->children)){
      fprintf(stderr, "cpt_add_child: error while allocating memory.\n");
      return PROG_ERROR;
   }
   pathNode *newNode = malloc(sizeof(pathNode));
   if(!newNode){
      fprintf(stderr, "cpt_add_child: error while allocating memory.\n");
      return PROG_ERROR;
   }
   newNode->name = malloc(sizeof(char) * (strlen(name) + 1));
   if(!(newNode->name)){
      fprintf(stderr, "cpt_add_child: error while allocating memory.\n");
      return PROG_ERROR;
   }
   strcpy(newNode->name, name);
   newNode->numChildren = 0;
   newNode->children = NULL;
   newNode->numRegistrations = 0;
   newNode->registrations = NULL;
   *node = newNode;
   father->children[father->numChildren++] = newNode;
   return PROG_SUCCESS;
}

// ===========================================================================
// cpt_remove_child  [SUPPPORT FUNCTION]
// NOTE: assumption: child is assumed to be present in father's children list.
// this beacuse child is a pointer that has been taken from that list and no
// other function could have modified the list befare this call.
// ===========================================================================
int cpt_remove_child(pathNode *father, pathNode *child){
   if(father->numChildren < 1) return PROG_ERROR;
   else if(father->numChildren == 1){
      free(father->children);
      father->children = NULL;
      father->numChildren = 0;
   }else{
      pathNode **newList = malloc(sizeof(pathNode *) *(father->numChildren - 1));
      if(!newList){
         fprintf(stderr, "cpt_remove_child: error while allocating memory.\n");
         return PROG_ERROR;
      }
      int i, k; //counter
      for(i = 0, k = 0; i < father->numChildren; i++){
         if(father->children[i] != child){
            newList[k++] = father->children[i];
         }
      }
      free(father->children); //free the old list
      father->children = newList;
      father->numChildren--;
   }
   /*delete the child node children and registrations are null, otherwise the node could not
   be deleted.*/
   free(child->name);
   free(child);
   return PROG_SUCCESS;
}

// ===========================================================================
// __support_add_path  [SUPPPORT FUNCTION]
// ===========================================================================
int __support_add_path(pathNode *node, char **tokens, int numTokens, int index, pathNode **added, char *host){
   pathNode *p;
   if(cpt_contains_child(node, tokens[index], &p)){ //node already present
      if(numTokens > (index + 1)){
         return __support_add_path(p, tokens, numTokens, index + 1, added, host);
      }
   }else{ //the node must be added
      while(index < numTokens){
         if(cpt_add_child(node, tokens[index++], &p) == PROG_ERROR){
            fprintf(stderr, "__support_add_path: error while adding a pathNode child.\n");
            return PROG_ERROR;
         }
         node = p;
      }
   }
   /*at this point, p contains a pointer to the pathNode corresponding to the last token*/
   *added = p;
   return PROG_SUCCESS;
}

// ===========================================================================
// cpt_add_client_registration
// ===========================================================================
int cpt_add_client_registration(pathNode *root, char *path, clientNodeList *cnl, clientData *data,
   registrationMode mode){
   char **tokens;
   int numTokens;
   int i; //counter
   /*first we check if the client already receives notifications for this path*/
   int updated = 0;
   clientNode *foundNode;
   if(cnl_exists_node(cnl, data, &foundNode)){
      for(i = 0; i < foundNode->numRegisteredPaths; i++){
         if(strcmp(path, foundNode->registeredPaths[i]) == 0){
            break; //this case will be handled later
         }
         else if(is_prefix(path, foundNode->registeredPaths[i])){
            return PATH_ALREADY_REGISTERED;
         }else if(is_prefix(foundNode->registeredPaths[i], path)){
            //we must update the registration.
            char *oldPath = foundNode->registeredPaths[i];
            foundNode->registeredPaths[i] = NULL;
            if(tokenize_path(oldPath, &tokens, &numTokens) == PROG_ERROR){
               fprintf(stderr, "cpt_add_client_registration: error while tokenizing the path.\n");
               return PROG_ERROR;
            }
            int mustBeRemoved;
            clientNode *nod;
            int retRM = __support_remove_client_registration(root, tokens, numTokens, 0, &mustBeRemoved, data, &nod);
            if(retRM == PROG_ERROR){
               fprintf(stderr, "cpt_add_client_registration: error while updating a client registration.\n");
               return PROG_ERROR;
            }else if(retRM == PATH_NOT_REGISTERED){
               fprintf(stderr, "cpt_add_client_registration: error while updating a client registration (PATH NOT REGISTERED).\n");
               return PROG_ERROR;
            }
            int k;
            for(k = 0; k < numTokens; k++) free(tokens[k]);
            free(tokens);
            free(oldPath);
            updated++;
         }
      }
      if(updated > 0){
         char **newPathList = malloc(sizeof(char *) * (foundNode->numRegisteredPaths - updated + 1));
         if(!newPathList){
            fprintf(stderr, "cpt_add_client_registration: error while allocating memory.\n");
            return PROG_ERROR;
         }
         int k;
         for(i = 0, k = 0; i < foundNode->numRegisteredPaths; i++){
            if(foundNode->registeredPaths[i] != NULL){
               newPathList[k++] = foundNode->registeredPaths[i];
            }
         }
         newPathList[k] = path;
         free(foundNode->registeredPaths);
         foundNode->registeredPaths = newPathList;
         foundNode->numRegisteredPaths = foundNode->numRegisteredPaths - updated + 1;

      }
   }else{
      if(cnl_add_client_node(cnl, data, &foundNode) == PROG_ERROR){
         fprintf(stderr, "cpt_add_client_registration: error while adding a client node.\n");
         return PROG_ERROR;
      }
   }

   //PART 2

   pathNode *added;
   if(tokenize_path(path, &tokens, &numTokens) == PROG_ERROR){
      fprintf(stderr, "cpt_add_client_registration: error while tokenizing the path.\n");
      return PROG_ERROR;
   }
   int retSAP = __support_add_path(root, tokens, numTokens, 0, &added, data->hostName);
   if(retSAP == PROG_ERROR){
      fprintf(stderr, "cpt_add_client_registration: error while adding a new path.\n");
      return PROG_ERROR;
   }

   /*need to add a registration item. first we check if the client already monitors the path.
   If so, we only update the "mode" field.*/
   for(i = 0; i < added->numRegistrations; i++){
      if(strcmp((added->registrations)[i]->client->networkData->hostName, data->hostName) == 0){
         added->registrations[i]->mode = mode;
         return PROG_SUCCESS;
      }
   }

   if(!updated){
      foundNode->numRegisteredPaths++;
      foundNode->registeredPaths = realloc(foundNode->registeredPaths, sizeof(char *) * foundNode->numRegisteredPaths);
      if(!foundNode->registeredPaths){
         fprintf(stderr, "cpt_add_client_registration: error while allocating memory.\n");
         return PROG_ERROR;
      }
      foundNode->registeredPaths[foundNode->numRegisteredPaths - 1] = path;
   }
   /*now foundNode is a pointer to the clientNode structure to be associated with the registration*/
   registration *nReg = malloc(sizeof(registration));
   if(!nReg){
      fprintf(stderr, "cpt_add_client_registration: error while allocating memory.\n");
      return PROG_ERROR;
   }
   nReg->client = foundNode;
   nReg->mode = mode;

   if(added->numRegistrations == 0){
      added->numRegistrations++;
      added->registrations = malloc(sizeof(registration *));
      if(!(added->registrations)){
         fprintf(stderr, "cpt_add_client_registration: error while allocating memory.\n");
         return PROG_ERROR;
      }
      (added->registrations)[0] = nReg;
   }else{
      added->registrations = realloc(added->registrations, sizeof(registration *) * (added->numRegistrations + 1));
      if(!added->registrations){
         fprintf(stderr, "cpt_add_client_registration: error while reallocating memory.\n");
         return PROG_ERROR;
      }
      added->registrations[added->numRegistrations++] = nReg;
   }
   return updated ? PATH_UPDATED : PROG_SUCCESS;
}

// ===========================================================================
// __support_remove_client_registration  [SUPPPORT FUNCTION]
// ===========================================================================
int __support_remove_client_registration(pathNode *node, char **tokens, int numTokens, int index,
   int *mustBeRemoved, clientData *data, clientNode **client){

   if(index <= numTokens - 1){ //recursive call
      pathNode *p;
      if(cpt_contains_child(node, tokens[index], &p)){ //node already present
         int removeChild;
         int ret = __support_remove_client_registration(p, tokens, numTokens, index + 1, &removeChild, data, client);
         if(ret != PROG_SUCCESS) return ret;
         if(removeChild){
            if(cpt_remove_child(node, p) == PROG_ERROR){
               fprintf(stderr, "__support_remove_client_registration: error while removing a child node.\n");
               return PROG_ERROR;
            }
            if(node->numChildren == 0 && node->numRegistrations == 0)
               *mustBeRemoved = 1;
            else
               *mustBeRemoved = 0;
         }else{
            *mustBeRemoved = 0;
         }
            return PROG_SUCCESS;
      }else{ //the path was not registered by any client
         return PATH_NOT_REGISTERED;
      }
   }else{ //BASE CASE
      /*at this point, node contains a pointer to the pathNode corresponding to the last token.
      We need to check if the client was registered for this path*/

      int i;
      int found = 0;
      for(i = 0; i < node->numRegistrations; i++){
         if(strcmp(node->registrations[i]->client->networkData->hostName, data->hostName) == 0){ //same IP
            found = 1;
            break;
         }
      }
      if(!found) return PATH_NOT_REGISTERED;

      registration **newList = NULL;
      if(node->numRegistrations > 1){
         /*copy all registrations but the one with index "i"*/
         newList =  malloc(sizeof(registration *) * (node->numRegistrations - 1));
         if(!newList){
            fprintf(stderr, "%s\n", "__support_remove_path: error while allocating memory.");
            return PROG_ERROR;
         }
         int j, k;
         for(j = 0, k = 0; j < node->numRegistrations; j++){
            if(j != i){
               newList[k++] = node->registrations[j];
            }
         }
      }

      *client = node->registrations[i]->client;

      /*delete registration (but not the clientNode, we still need it)*/
      free(node->registrations[i]);
      free(node->registrations);
      node->numRegistrations--;
      if(node->numRegistrations == 0){
            node->registrations = NULL;
            if(node->numChildren == 0)
               *mustBeRemoved = 1;
            else
               *mustBeRemoved = 0;
      }else{
         node->registrations = newList;
         *mustBeRemoved = 0; //at least one registration is still present
      }
      return PROG_SUCCESS;
   }
}

// ===========================================================================
// cpt_remove_client_registration
// ===========================================================================
int cpt_remove_client_registration(pathNode *root, char *path, clientNodeList *cnl, clientData *data){
   char **tokens;
   int numTokens;
   int i; //counter
   pathNode *added;
   if(tokenize_path(path, &tokens, &numTokens) == PROG_ERROR){
      fprintf(stderr, "cpt_remove_client_registration: error while tokenizing the path.\n");
      return PROG_ERROR;
   }
   int mustBeRemoved;
   clientNode *client;
   int retVal = __support_remove_client_registration(root, tokens, numTokens, 0, &mustBeRemoved, data, &client);
   if( retVal == PROG_ERROR){
      fprintf(stderr, "cpt_remove_client_registration: error in __support_remove_path.\n");
      return PROG_ERROR;
   }else if(retVal == PATH_NOT_REGISTERED){
      return PATH_NOT_REGISTERED;
   }
   /*remove the path from the client's registeredPaths list*/
   char **newRegPaths = NULL;
   if(client->numRegisteredPaths > 1){
      newRegPaths = malloc(sizeof(char *) * (client->numRegisteredPaths - 1));
      if(!newRegPaths){
         fprintf(stderr, "__support_remove_path: error while allocating memory.\n");
         return PROG_ERROR;
      }
      int j, k;
      for(j = 0, k = 0; j < client->numRegisteredPaths; j++){
         if(strcmp(path, client->registeredPaths[j]) != 0){
            newRegPaths[k++] = client->registeredPaths[j];
         }else{
            free(client->registeredPaths[j]);
         }
      }
      free(client->registeredPaths);
      client->registeredPaths = newRegPaths;
   }else{
      free(client->registeredPaths[0]);
      free(client->registeredPaths);
   }
   client->numRegisteredPaths--;

   if(client->numRegisteredPaths == 0){
      /*there were just the path that has been deleted. So now there are no registered path for
      the client; we can delete its entry in the clientNodeList*/
      if(cnl_remove_client_node(cnl, client) == PROG_ERROR){
         fprintf(stderr, "cpt_remove_client_registration: error in cnl_remove_client_node.\n");
         return PROG_ERROR;
      }
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// cpt_delete_subtree
// ===========================================================================
int cpt_delete_subtree(pathNode *node, char *str){
      int i;
      for(i = 0; i < node->numChildren; i++){
            cpt_delete_subtree(node->children[i], str);
      }
      //delete node
      //send notification to clients
      int j;
      for(j = 0; j < node->numRegistrations; j++){
            if(cnl_signal_deletion(node->registrations[j]->client, str) == PROG_ERROR){
                  fprintf(stderr, "cnl_delete_subtree: error while signaling a deletion.\n");
                  return PROG_ERROR;
            }
            free(node->registrations[j]);
      }
      //delete node
      if(j > 0) free(node->registrations);
      if(i > 0) free(node->children);
      free(node->name);
      free(node);
}

// ===========================================================================
// cpt_clean_tree
// ===========================================================================
int cpt_clean_tree(pathNode *node){
      int canDelete = node->numRegistrations == -1 ? 0 : 1;
      if(node->numChildren > 0){
         int i;
         int numDeleted = 0;
         for(i = 0; i < node->numChildren; i++){
            int ret = cpt_clean_tree(node->children[i]);
            if(ret == PROG_ERROR) return PROG_ERROR;
            else if(ret){
               node->children[i] = NULL;
               numDeleted++;
            }
            else canDelete = 0;
         }

         if(numDeleted > 0){
            if(numDeleted != node->numChildren){
               pathNode **newList = malloc(sizeof(pathNode *) * (node->numChildren - numDeleted));
               if(!newList){
                  fprintf(stderr, "cpt_clen_tree: error while allocating memory.\n");
                  return PROG_ERROR;
               }
               int k;
               for(i = 0, k = 0; i < node->numChildren; i++){
                  if(node->children[i] != NULL){
                     newList[k++] = node->children[i];
                  }
               }
               free(node->children);
               node->children = newList;
               node->numChildren = node->numChildren - numDeleted;
            }else{
               free(node->children);
               node->children = NULL;
               node->numChildren = 0;
            }
         }
      }
      if(canDelete){
         if(node->numRegistrations == 0){
            free(node);
         }else{
            canDelete = 0;
         }
      }
      return canDelete;
}


// ===========================================================================
// cpt_push_notification
// ===========================================================================
int cpt_push_notification(pathNode *root, receivedNotification *not, char *stringNot){
      char **tokens;
      int numTokens;
      if(tokenize_path(not->path, &tokens, &numTokens) == PROG_ERROR){
            fprintf(stderr, "cpt_push_notification: error while tokenizing the path.\n");
            return PROG_ERROR;
      }
      pathNode *current = root;
      pathNode *child;
      int i = 0;
      int maxIndex = not->isDir ? numTokens : numTokens - 1;
      int maxInternIndex = maxIndex - 1;
      /*now we get to the tree node corresponding to not->path, if any.
      During the visit, it must be checked whether there is a node along the
      path that presents a client which is registered in RECURSIVE mode.
      In that case the client will receive the notification, too.*/
      while(i < maxIndex){
         if(cpt_contains_child(current, tokens[i], &child)){ //node present
            int j; //counter
            for(j = 0; j < child->numRegistrations; j++){
               if((i < maxInternIndex && child->registrations[j]->mode == RECURSIVE) || i == maxInternIndex) {
                  //add this notification
                  if(cnl_add_notification(child->registrations[j]->client, stringNot) == PROG_ERROR){
                     fprintf(stderr, "cpt_push_notification: error while adding a notification to a client.\n");
                     return PROG_ERROR;
                  }
               }
            }

            if(i++ != maxInternIndex) current = child; //beacuse we'll need father and child
         }else{ //there are no more clients we can send this notification to
            for(i = 0; i < numTokens; i++) free(tokens[i]);
            free(tokens);
            return PROG_SUCCESS;
         }
      }

      /*there is one last case to handle. A branch of the filesystem is deleted and all the clients
      registered for a subdirectory of that branch must be notified. If this is the case, child
      represents the deleted node. All the clients registered to descendants of this node must be
      notified.*/
      if(not->isDir == 1 && not->type == deletion){
         if(cpt_delete_subtree(child, not->path) == PROG_ERROR){
               fprintf(stderr, "cpt_push_notification: error while removing a subtree.\n");
               return PROG_ERROR;
         }
         if(current->numChildren == 1){
               free(current->children);
               current->numChildren = 0;
               if(cpt_clean_tree(root) == PROG_ERROR){
                  fprintf(stderr, "cpt_push_notification: error while cleaning the tree.\n");
                  return PROG_ERROR;
               }
         }else{
            pathNode **newList = malloc(sizeof(pathNode *) *(current->numChildren - 1));
            if(!newList){
                  fprintf(stderr, "cpt_push_notification: error while allocating memory.\n");
                  return PROG_ERROR;
            }
            int i, k; //counter
            for(i = 0, k = 0; i < current->numChildren; i++){
               if(current->children[i] != child){
                  newList[k++] = current->children[i];
               }
            }
            free(current->children); //free the old list
            current->children = newList;
            current->numChildren--;
         }
      }
      for(i = 0; i < numTokens; i++) free(tokens[i]);
      free(tokens);
      return PROG_SUCCESS;
}

// ===========================================================================
// __cpt_print_tree_rec [SUPPPORT FUNCTION]
// ===========================================================================
void __cpt_print_tree_rec(pathNode *node, int lev){
   int i;
   for(i = 0; i < node->numChildren; i++){
      printf("%*s| - Name: %s, numReg: %d\n", lev, " ", node->children[i]->name, node->children[i]->numRegistrations);
      __cpt_print_tree_rec(node->children[i], lev + 3); //recursive call
   }
}

// ===========================================================================
// cpt_print_tree
// ===========================================================================
void cpt_print_tree(pathNode *root){
   printf("_________CLIENT PATH TREE______________\n\n");
   __cpt_print_tree_rec(root, 1);
}
