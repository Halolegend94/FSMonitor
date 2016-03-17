#include "../include/myfile.h"

int main(int argc, char **argv){
   if(argc != 2) return 0;

   printf("path assoluto? %d\n", is_absolute_path(argv[1]));

   char **tokens = NULL;
   int num = 0;
   int ret = tokenize_path(argv[1], &tokens, &num);
   if(ret == -1){
      printf("Errore\n");
      return -1;
   }
   int i;
   for (i = 0; i < num; i++){
      printf("-%d %s\n", i, tokens[i]);
   }
   return 0;
}
