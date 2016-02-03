#ifndef __FILESYSTREE__
	#define __FILESYSTREE__
	#include "myfile.h"
	#include <string.h>

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
	// fst_get_name
	// Description: retrieves the name of the file represented by the fstNode
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns NULL in case of error, the name string otherwise.
	//
	// ===========================================================================
	char *fst_get_name(fstNode *node);

	// ===========================================================================
	//
	// fst_get_perms
	// Description: gets the permissions string from a fstNode
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns NULL in case of error, the perms string otherwise.
	//
	// ===========================================================================
	char *fst_get_perms(fstNode *node);

	// ===========================================================================
	//
	// fst_get_size
	// Description: gets the size of the file represented by the fstNode
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns -1 in case of error, the size otherwise
	//
	// ===========================================================================
	long long fst_get_size(fstNode *node);

	// ===========================================================================
	//
	// fst_get_wtime
	// Description: gets the last write timestamp of the file represented by the
	// fstNode.
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns -1 in case of error, a poisitive value otherwise
	//
	// ===========================================================================
	long long fst_get_wtime(fstNode *node);

	// ===========================================================================
	//
	// fst_is_dir
	// Description: function that tells if the node associated file is a directory
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns 0 if false, 1 if true, -1 in case of error
	//
	// ===========================================================================
	int fst_is_dir(fstNode *node);

	// ===========================================================================
	//
	// fst_is_root
	// Description: function that tells if the node associated file is the root
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns 0 if false, 1 if true, -1 in case of error
	//
	// ===========================================================================
	int fst_is_root(fstNode *node);

	// ===========================================================================
	//
	// fst_is_monitored
	// Description: function that tells if the subtree at that node is monitored
	// by some server.
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns 0 if false, 1 if true, -1 in case of error
	//
	// ===========================================================================
	int fst_is_monitored(fstNode *node);

	// ===========================================================================
	//
	// fst_get_num_children
	// Description: function that returns the number of children of a fstNode
	// Params:
	// 	-	node: a pointer to a fstNode
	// Returns -1 in case of error, a nonnegative number otherwise.
	//
	// ===========================================================================
	int fst_get_num_children(fstNode *node);

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
