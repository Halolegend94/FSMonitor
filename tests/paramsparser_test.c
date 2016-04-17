#include "../include/params_parser.h"

int main(int argc, char **argv){
   optToken **list;
   int count;
   const char *paramsWithValue[3] = {"a", "c", "e"};
   if(parse_params(argc, argv, paramsWithValue, 3, &list, &count) == PROG_ERROR){
      fprintf(stderr, "errr\n");
      return -1;
   }
   int i;
   for(i = 0; i < count; i++){
      if(list[i]->isParam){
         printf("PARAM: %s - value: %s\n", list[i]->name, list[i]->value ? list[i]->value : "W/P");
      }else{
         printf("INPUT %s\n", list[i]->value);
      }
   }
   return 0;
}
