#ifndef __FILESYSTREE__
	#define __FILESYSTREE__
	#include "myfile.h"
	#include <string.h>
	#include "mem_management.h"
	/*The following structure represents a node in the FileSystem Tree. The FileSystem Tree
	is used to mantain the state of the motinored directories by the deamon.*/
	typedef struct _fstnode{
		unsigned long off_name;
		unsigned long off_perms;
		long long size;
		long long lastWriteTimestamp;
		int isDir;
		int numChildren; //number of child nodes
		unsigned long off_children; //offset to a list of offset to child nodes _fstnode
		int isMonitored; //indicates that this substree is monitored by some process
		int isRoot; //it's the filesystree root
	} fstNode;

	/*function prototypes*/

	// ===========================================================================
	//
	// fst_build_root
	// Description: creates the first node of the file system tree. It's the root
	// and cannot be deleted.
	// Params:
	// 	-	a pointer where to allocate a new fstNode structure
	// Returns: -1 in case of error, 0 otherwise
	//
	// ===========================================================================
	int fst_build_root(fstNode **root);

	// ===========================================================================
	//
	// fst_get_children
	// Description: gets the children of the node passed as firt argument. A node
	// can have children if and only if it represents a directory.
	// Params:
	// 	-	father: a pointer to a fstNode.
	// 	-	list: a pointer to a list of pointer to fstNode. Here the function
	// 		will store the pointers to the children of father.
	// 	-	size: a pointer to an int where the function will store the size of
	// 		list.
	//	Returns 0 in case of success, -1 in case of error, 1 if the node has no child
	//
	// ===========================================================================
	int fst_get_children(fstNode *father, fstNode ***list, unsigned int *size);

	// ===========================================================================
	//
	// fst_contains_child
	// Description: check if a node's children list contains a particular child.
	// Params:
	// 	-	father: a pointer the father fstNode
	// 	-	name: a string containing the searched child name
	//	-	child: a pointer to a location where to store the pointer to the child found.
	// Returns -1 in case of error, 0 if the name is not found, 1 if it is found
	//
	// ===========================================================================
	int fst_contains_child(fstNode *father, char *name, fstNode **child);

	// ===========================================================================
	//
	// fst_delete_child
	// Description: delete a child node from a father node.
	// Params:
	// 	-	father: a pointer the father fstNode
	// 	-	node: a pointer the child node to be removed.
	// Returns -1 in case of error, 0 otherwise
	//
	// ===========================================================================
	int fst_delete_child(fstNode *father, fstNode *node);

	// ===========================================================================
	//
	// fst_add_child
	// Description: add a child node to a fstNode
	// Params:
	// 	-	father: a pointer to the father fstNode
	// 	-	file: a pointer to a struct myFile with the info
	// 	-	node: a pointer to a location where a pointer to the added node
	// 		will be stored.
	// Returns -1 in case of error, 0 otherwise
	//
	// ===========================================================================
	int fst_add_child(fstNode *father, myFile *file, fstNode **node);

	// ===========================================================================
	//
	// fst_add_children
	// Description: add a set of fstNode as children to a fstNode
	// Params:
	// 	-	father: a pointer to the father fstNode
	// 	-	fList: a pointer to a struct myFileList with the files list
	// 	-	nodeList: a pointer to a location where a pointer to the added
	// 		list of nodes will be stored.
	// Returns -1 in case of error, 0 otherwise
	//
	// ===========================================================================
	int fst_add_children(fstNode *father, myFileList *fList, fstNode **list);
	
	// ===========================================================================
	//
	// fst_print_tree (Debug function)
	// Description: print on standard output the tree using the filename as node.
	// Params:
	// 	-	a pointer to a fstNode
	//
	// ===========================================================================
	void fst_print_tree(fstNode *node);


#endif
