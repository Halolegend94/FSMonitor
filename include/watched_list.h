#ifndef __WATCHEDLIST__
#define __WATCHEDLIST__
#include "mem_management.h"
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


// ===========================================================================
//
// wl_create
// Description: creates a new empty watchedList data structure
// Params:
// 	- 	list: a pointer to pointer to a memory area where the data structure
// 		will be stored.
// Returns 0 in case of success, -1 otherwise
//
// ===========================================================================
int wl_create(watchedList **list);

// ===========================================================================
//
// wl_add_path
// Description: adds a path to the list of monitored paths
// Params:
// 	-	path: a pointer to a string that represents the path to be inserted.
// 	-	wList: a pointer to a watchedList where the path must be inserted.
// Returns 0 in case of success, -1 otherwise
//
// ===========================================================================
int wl_add_path(char *path, watchedList *wList);

// ===========================================================================
//
// wl_remove_path
// Description: removes a path from the list of watched paths.
// Params:
// 	-	path: the path to be removed
// 	-	wList: the list from where the path must be removed
// 	-	entryRemoved: is set to 1 if path was watched by only one server,
// 		0 otherwise. If 1, the entry is removed, else the counter is
// 		diminished.
// Returns 0 in case of success, -1 otherwise
//
// ===========================================================================
int wl_remove_path(char *path, watchedList *wList, int *entryRemoved);

// ===========================================================================
//
// wl_print_watched_list
// Description: print on stdout the current list of watched paths.
// Params:
// 	- wList: a pointer to the watchedList that must be printed.
//
// ===========================================================================
void wl_print_watched_list(watchedList *wList);
#endif
