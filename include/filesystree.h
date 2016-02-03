#ifndef __FILESYSTREE__
	#define __FILESYSTREE__
	#include "myfile.h"
	#include <string.h>

	typedef struct _fstnode{
		unsigned long off_name;
		unsigned long off_perms;
		unsigned long long size;
		unsigned long long lastWriteTimestamp;
		int isDir;
		unsigned int numChildren; //number of child nodes
		unsigned long off_children; //offset to a list of offset to child nodes _fstnode
		int isMonitored; //indicates that this substree is monitored by some process
		int isRoot; //it's the filesystree root
	} fstNode;


#endif
