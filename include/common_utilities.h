#ifndef __COMMONUTILITIES__
   #define __COMMONUTILITIES__
   #include <string.h>
   #include <stdio.h>
   #include <stdlib.h>

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
#endif
