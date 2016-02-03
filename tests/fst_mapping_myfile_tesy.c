#include <stdlib.h>
#include <stdio.h>
#include "../include/mem_management.h"
#include "../include/filesystree.h"
#include "../include/mapping.h"

#define MAPSIZE 1024*1024*4

int main(void){
   /*create a mapping*/
 pMapping map;
   int ret = create_mapping(&map, "mymappingm", MAPSIZE);
   if(ret == -1){
      fprintf(stderr, "Error while creating the mapping.\n");
      exit(0);
   }

   char *pointerMapping = get_mapping_pointer(map);
   if(pmm_initialize_management(pointerMapping, MAPSIZE, NULL) == -1){
      fprintf(stderr, "Error while initializing the mapping memory.\n");
	    getchar();
      exit(0);
   }
   /*create the root node*/
   fstNode *root = NULL;
   if(fst_build_root(&root) == -1){
      fprintf(stderr, "Error while creating the root node.\n");
      exit(0);
   }
   /*the the list of files in the current dir*/
   myFileList lista;
   char *dir =  get_current_directory();
   if(get_directory_content(dir, &lista) == -1){
      fprintf(stderr, "Error while getting the directory content.\n");
	  exit(0);
   }

   /*add every file*/
   int i;
   fstNode *child;
   for(i = 0; i < lista.count; i++){
      if(fst_add_child(root, &lista.list[i], &child) == -1){
            fprintf(stderr, "Error while adding child node.");
            exit(0);
      }

   }
   fst_print_tree(root);
   delete_mapping(map);
   printf("Success! it seems\n");
}
