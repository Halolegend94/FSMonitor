#ifndef __PARAMSPARSER__
   #define __PARAMSPARSER__
   /***************************
   GRAMMAR:
      //this version supports only abbreviated param name
      S      -> OPT S | INPUT S | e
      OPT    -> -PNAME PVALUE | -PNAME

      PNAME must begin with an aplha char, PVALUE an PNAME have no restrictions
   *************************/
   #include <stdio.h>
   #include <ctype.h>
   #include <string.h>
   #include <stdlib.h>
   #define PROG_ERROR -1
   #define PROG_SUCCESS 0

   /*this structure represents a single option read from the command line.*/
   typedef struct _optToken{
      char *name;
      char *value;
      char isParam; //param = 1, input = 0
   } optToken;

   // ===========================================================================
   //
   // parse_params
   // Description: process the command line to extrapolate options and inputs.
   // NOTE: "input" denotes everything else than parameter (-options)
   // Params:
   //    -  argc and argv: main params.
   //    -  pwv: paramsWithValue, a list of strings, where each string is the name of a param
   //       that requires an associated value (the next element of argv).
   //    -  numPwvEntries: number of strings in pwv
   //    -  list: a pointer to a location where will be stored a pointer to a list of optToken, where
   //       each optToken contains an option with associated value (if any), or an input value.
   //    -  count: a pointer to a location where will be stored the number of entries in list.
   // Returns 0 in case of success, -1 in case of error.
   //
   // ===========================================================================
   int parse_params(int argc, char **argv, const char **pwv, int numPwvEntries, optToken ***list, int *count);

   // ===========================================================================
   //
   // free_optTokenList
   // Description: free the memory occupied by a optToken array of count elements.
   //
   // ===========================================================================
   void free_optTokenList(optToken **list, int count);
#endif
