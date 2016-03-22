#ifndef __COMMONUTILITIES__
   #define __COMMONUTILITIES__
   #include <string.h>
   #include <stdio.h>
   #include <stdlib.h>
#include <ctype.h>

   // ===========================================================================
   //
   // is_prefix
   // Description: check if a string "b" is a prefix of a string "a"
   // Params:
   //    -  a: the string that must be checked for a prefix
   //    -  b: the prefix string
   // Returns 1 if true, 0 if false
   //
   // ===========================================================================
   int is_prefix(char *a, char *b);

   // ==========================================================================
   // strcicmp (Case insensitive strcmp)
   // ==========================================================================
   int strcicmp(char *a, char *b);

   // ==========================================================================
   // strcincmp (case insensitive)
   // ==========================================================================
   int strcincmp(char *a, char *b, int len);

#endif
