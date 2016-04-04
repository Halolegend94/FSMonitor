#ifdef __CLIENTPATHSTHREE_
   #define __CLIENTPATHSTHREE__
   #include <stdio.h>
   #include <string.h>
   #include <stdlib.h>
   #include "myfile.h" //for tokenize path
   #include "client_bucket.h"
   #define ERROR -1
   #define SUCCESS 0

   typedef struct _pathnode{
      char *value;                   //name of the folder this node represents
      struct _pathnode **children;   //subfolders
      int numChildren;               //number of subfolders
      clientBucket **clients;        //clents registered to this path
      int numClients;                //number of clients
   } pathNode;

#endif

int cpt_create(pathNode **root){
      *root = malloc(sizeof(pathNode));
      if(!(*root)){
         fprintf(stderr, "cpt_create: error while allocating memory.\n");
         return ERROR;
      }
      (*root)->value = NULL;
      (*root)->children = NULL;
      (*root)->numChildren = 0;
      return SUCCESS;
}

int cpt_contains_child(pathNode *father, char *name, pathNode **node){
   int i; //counter
   for(i = 0; i < father->numChildren; i++){
      if(fname_compare(name, (father->children[i])->value) == 0){
         *node = father->children[i];
         return 1;
      }
   }
   return 0;
}

// ===========================================================================
// cpt_add_child
// ===========================================================================
int cpt_add_child(pathNode *father, char *name, pathNode **node){
   father->children = realloc(father->children, sizeof(pathNode *) * (father->numChildren + 1));
   if(!(father->children)){
      fprintf(stderr, "cpt_add_child: error while allocating memory.\n");
      return ERROR;
   }
   pathNode *newNode = malloc(sizeof(pathNode));
   if(!newNode){
      fprintf(stderr, "cpt_add_child: error while allocating memory.\n");
      return ERROR;
   }
   newNode->value = malloc(sizeof(char) * (strlen(name) + 1));
   if(!(newNode->value)){
      fprintf(stderr, "cpt_add_child: error while allocating memory.\n");
      return ERROR;
   }
   strcpy(newNode->value, name);
   newNode->numChildren = 0;
   newNode->children = NULL;
   *node = newNode;
   father->children[father->numChildren++] = newNode;
   return SUCCESS;
}

// ===========================================================================
// __support_add_path
// ===========================================================================
int __support_add_path(pathNode *node, char **tokens, int numTokens, int index, clientBucket *cb){
   pathNode *p;
   if(cpt_contains_child(node, tokens[index], &p)){ //node already present
      if(numTokens > (index + 1)){ //path already requested and present in the tree
         return __support_add_path(p, tokens, numTokens, index + 1);
      }
   }else{ //the node must be added
      while(index < numTokens){
         if(cpt_add_child(node, tokens[index++], &p) == ERROR){
            fprintf(stderr, "__support_add_path: error while adding a pathNode child.\n");
            return ERROR;
         }
         node = p;
      }
   }
   /*at this point, p contains a pointer to the pathNode corresponding to the last token*/
   //debug
   printf("DEB: last added (got) token: %s\n", p->value);
   //TODO: add a pointer to the data structure containing client info
}

// ===========================================================================
// cpt_add_path
// ===========================================================================
int cpt_add_path(pathNode *root, char *path, clientBucket *cb){
   char **tokens;
   int numTokens;
   if(tokenize_path(path, &tokens, &numTokens) == -1){
      fprintf(stderr, "cpt_add_path: error while adding a path to the client paths tree.\n");
      return -1;
   }
   if(__support_add_path(root, tokens, numTokens, 0, cb) == -1){
      fprintf() //TODO
   }

}
