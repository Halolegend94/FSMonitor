#include "include/common_utilities.h"

// ===========================================================================
// is_prefix
// ===========================================================================
int is_prefix(char *a, char *b){
   int lenA = strlen(a);
   int lenB = strlen(b);
   if(lenA >= lenB && strncmp(a, b, lenB) == 0) return 1;
   return 0;
}
