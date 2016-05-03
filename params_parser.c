#include "include/params_parser.h"
#define NUM_OPTS 10
#define NUM_OPTS_INC 5

// ===========================================================================
// opt [PRIVATE FUNCTION]
// Process a single option
// ===========================================================================
int opt(int argc, char **argv, int *index, const char **pwv, int numPwv, optToken **o){
   if(strlen(argv[*index]) <= 1){
      //lexycal error
      fprintf(stderr, "parse_params::opt - lexycal error within %d param. No name specified.\n", *index);
      return PROG_ERROR;
   }else if(!isalpha(argv[*index][1])){
      //lexycal error
      fprintf(stderr, "parse_params::opt - lexycal error within %d param. Name must begin with an aplha char.\n", *index);
      return PROG_ERROR;
   }
   /*ceate a buffer*/
   *o = malloc(sizeof(optToken));
   if(!(*o)){
      fprintf(stderr, "parse_params::opt: error while allocating memory for the token structure.\n");
      return PROG_ERROR;
   }
   /*get the name*/
   char *name = malloc(sizeof(char) * strlen(argv[*index])); //we will not copy the initial "-"
   if(!name){
      fprintf(stderr, "parse_params::opt: error while allocating memory for the name.\n");
      return PROG_ERROR;
   }
   strcpy(name, argv[*index] + 1);
   /*must be checked whether this option requires an associated value. */
   int i;
   int found = 0;
   for(i = 0; i < numPwv; i++){
      if(strcmp(name, pwv[i]) == 0){
         found = 1;
         break;
      }
   }
   (*o)->name = name;
   (*o)->value = NULL;
   if(found){
      (*index)++; //move the index ahead
      if((*index) >= argc){
         //syntax error
         fprintf(stderr, "parse_params::opt - error, param \"%s\" requires a value to be passed.\n", name);
         return PROG_ERROR;
      }
      if(argv[*index][0] == '-'){
         //syntax error
         fprintf(stderr, "parse_params::opt - error, param \"%s\" requires a value to be passed.\n", name);
         return PROG_ERROR;
      }
      char *value = malloc(sizeof(char) * (strlen(argv[*index]) + 1));
      if(!value){
         fprintf(stderr, "parse_params::opt - error while allocating memory for the alue.\n");
         return PROG_ERROR;
      }
      strcpy(value, argv[*index]);
      (*o)->value = value;
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// parse_params
// ===========================================================================
int parse_params(int argc, char **argv, const char **pwv, int numPwvEntries, optToken ***list, int *count){

   /*list to hold the parameters*/
   int maxNumElements = NUM_OPTS;
   *list = malloc(sizeof(optToken *) * NUM_OPTS);
   if(!(*list)){
      fprintf(stderr, "parse_params: error while allocating memory.\n");
      return PROG_ERROR;
   }
   *count = 0;

   int i;
   for(i = 0; i < argc; i++){ //for each argument
      if(*count >= maxNumElements){ //check if there is enough space in the list
         maxNumElements+= NUM_OPTS_INC;
         *list = realloc(*list, maxNumElements);
         if(!(*list)){
            fprintf(stderr, "parse_params: error while allocating memory.\n");
            return PROG_ERROR;
         }
      }
      switch (argv[i][0]) { //identify the argument
         case '-':{
            /*PNAME expected. PVALUEs, if any, are processed in here*/
            optToken *o;
            if(opt(argc, argv, &i, pwv, numPwvEntries, &o) == PROG_ERROR) return PROG_ERROR;
            o->isParam = 1;
            (*list)[(*count)++] = o;
            break;
         }
         default:{
            /*input string expected*/
            optToken *o = malloc(sizeof(optToken));
            if(!o){
               fprintf(stderr, "parse_params: error while allocating memory.\n");
               return PROG_ERROR;
            }
            char *value =  malloc(sizeof(char) * (strlen(argv[i]) + 1));
            if(!value){
               fprintf(stderr, "parse_params: error while allocating memory.\n");
               return PROG_ERROR;
            }
            strcpy(value, argv[i]);
            o->value = value;
            o->name = NULL;
            o->isParam = 0;
            (*list)[(*count)++] = o;
         }
      }
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// free_optTokenList
// ===========================================================================
void free_optTokenList(optToken **list, int count){
   int i;
   for(i = 0; i < count; i++){
      if(list[i]->isParam){
         free(list[i]->name);
         if(list[i]->value != NULL) free(list[i]->value);
      }else{
         free(list[i]->value);
      }
      free(list[i]);
   }
   free(list);
}
