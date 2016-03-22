#include "include/common_utilities.h"

// ===========================================================================
// is_prefix
// ===========================================================================
int is_prefix(char *a, char *b){
   int lenA = strlen(a);
   int lenB = strlen(b);
   if(lenA >= lenB && strcincmp(a, b, lenB) == 0) return 1;
   return 0;
}

// ==========================================================================
// strcicmp
// ==========================================================================
int strcicmp(char *a, char *b){
	int l1 = strlen(a);
	int l2 = strlen(b);
	if (l1 != l2) return 1;
	for (;; a++, b++) {
		int d = tolower(*a) - tolower(*b);
		if (d != 0 ||  *a == '\0')
			return d;
	}
}

// ==========================================================================
// strcicmp
// ==========================================================================
int strcincmp(char *a, char *b, int len){
	int i;
	for (i = 0; i < len; a++, b++) {
		int d = tolower(*a) - tolower(*b);
		if (d != 0) return d;
	}
   return 0;
}
