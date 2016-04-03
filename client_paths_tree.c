#ifdef __CLIENTPATHSTHREE_
   #define __CLIENTPATHSTHREE__
   #include <stdio.h>
   #include <string.h>
   #include <stdlib.h>
   #include "myfile.h" //for tokenize path
   
   typedef struct _pathnode{
      char *value;
      struct _pathnode **children;
      int numChildren;
   } pathNode;

#endif

int cpt_create(pathNode **root){
      *root = malloc(sizeof(pathNode));
      if(!(*root)){
         fprintf(stderr, "cpt_create: error while allocating memory.\n");
         return -1;
      }
      (*root)->value = NULL;
      (*root)->children = NULL;
      (*root)->numChildren = 0;
      return 0;
}

int cpt_contains_child(pathNode *node, char *name){

}

int __support_add_path(pathNode *node, char **tokens, int numTokens, int index){
   /*get current node's children, if any.*/
   if(node->numChildren > 0){

   }
}

int cpt_add_path(pathNode *root, char *path){
   char **tokens;
   int numTokens;
   if(tokenize_path(path, &tokens, &numTokens) == -1){
      fprintf(stderr, "cpt_add_path: error while adding a path to the client paths tree.\n");
      return -1;
   }
   if(__support_add_path(root, tokens, numTokens, 0) == -1){
      fprintf() //TODO
   }

}
