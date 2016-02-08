#ifndef __MAPPINGSTRUCTURE__

#define __MAPPINGSTRUCTURE__

/*The following structure is used to organize data inside
the file mapping.*/
typedef struct _mappingStructure{
	fstNode *fileSystemTree;
	unsigned int serverCounter;
	
} mappingStructure;

#endif
