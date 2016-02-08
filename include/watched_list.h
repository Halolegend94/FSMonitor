#ifndef __WATCHEDLIST__
#define __WATCHEDLIST__
#include "include/mem_management.h"
#include <string.h>

/*this struct represents a path that is monitored by one or more
server. The field serverCount indicates how many servers are watching
a path.*/
typedef struct __watchedPath{
	unsigned long off_fullpath;
	unsigned int serverCount;
} watchedPath;

/*the following structure represents a list of watched paths*/
typedef struct __watchedList{
	unsigned int count;
	unsigned long off_list; //offset to a list of offset to watchedPaths
} watchedList;

#endif
