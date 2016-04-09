#include <stdlib.h>
#include <stdio.h>
#include "../include/mem_management.h"
#include "../include/filesystree.h"
#include "../include/mapping.h"

#define MAPSIZE 1024 * 1024 * 4

int main(int argc, char **argv){

   if(argv[1][0] == 'm'){
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
     
      fstNode *child = NULL;
      if(fst_add_children(root, &lista, &child) == -1){
               fprintf(stderr, "Error while adding child node.");
               exit(0);
         }

      fst_print_tree(root);
      getchar();
        if(fst_add_children(root, &lista, &child) == -1){
               fprintf(stderr, "Error while adding child node.");
               exit(0);
         }
      fst_print_tree(root);
  
	
      delete_mapping(map);
      printf("Success! it seems\n");
   }else{
      /*create a mapping*/
      pMapping map;
      int ret = open_mapping(&map, "mymappingm", MAPSIZE);
      printf("Mapp opened.\n");
      if(ret == -1){
         fprintf(stderr, "Error while creating the mapping.\n");
         exit(0);
      }

      char *pointerMapping = get_mapping_pointer(map);
      fstNode *root = NULL;
      if(pmm_initialize_management(pointerMapping, MAPSIZE, (void **)&root) == -1){
         fprintf(stderr, "Error while getting the block of memory.\n");
   	    getchar();
         exit(0);
      }
      printf("point %p\n", root);
      fst_print_tree(root);
      getchar();
      fst_print_tree(root);
      delete_mapping(map);
      printf("Success! it seems\n");
   }

}
