#include "include/filesystree.h"

// ===========================================================================
// fst_build_root
// ===========================================================================
int fst_build_root(fstNode **root){
	*root = (fstNode *) pmm_malloc(sizeof(fstNode));
	if(!(*root)){
		fprintf(stderr, "Error while allocating memory for a node with pmm.\n");
		return -1;
	}
	(*root)->off_name = 0;
	(*root)->off_perms = 0;
	(*root)->size = 0;
	(*root)->lastWriteTimestamp = 0;
	(*root)->isRoot = 1;
	(*root)->isMonitored = 0;
	(*root)->numChildren = 0;
	(*root)->off_children = 0;
	(*root)->isDir = 1;
	return 0;
}


// ===========================================================================
// fst_get_children
// ===========================================================================
int fst_get_children(fstNode *father, fstNode ***list, unsigned int *size){
	if(!father || !list){
		fprintf(stderr, "fst_get_children: input not valid.\n");
		return -1;
	}
	*size = father->numChildren;
	if(father->numChildren == 0){
		return 1;
	}
	//get the pointer to the children list
	unsigned long *chdlist = pmm_offset_to_pointer(father->off_children);

	//allocate space
	*list = (fstNode**) malloc(sizeof(fstNode*) * father->numChildren);
	if(!(*list)){
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}

	int i;
	//get the pointers
	for(i = 0; i < father->numChildren; i++){
		(*list)[i] = pmm_offset_to_pointer(chdlist[i]);
	}
	return 0;
}

// ===========================================================================
// fst_contains_child
// ===========================================================================
int fst_contains_child(fstNode *father, char *name, fstNode **child){
	//get the current node children
	int numChld = 0;
	fstNode **list = NULL;
	if(fst_get_children(father, &list, &numChld) == -1){
		fprintf(stderr, "fst_contains_child: error while getting the node's children.\n");
		return -1;
	}
	if(numChld == 0) return 0;
	int i;
	for(i = 0; i < numChld; i++){
		if(fname_compare(pmm_offset_to_pointer(list[i]->off_name), name) == 0){
			*child = list[i];
			return 1;
		}
	}
	*child = NULL;
	return 0;

}

// ===========================================================================
// __support_delete_child
// ===========================================================================
void __support_delete_child(fstNode *toDelete){
	pmm_free(pmm_offset_to_pointer(toDelete->off_perms));
	pmm_free(pmm_offset_to_pointer(toDelete->off_name));
	if(toDelete->numChildren  > 0){
		//get the pointer to the children list
		long *chdlist = pmm_offset_to_pointer(toDelete->off_children);

		int i;
		for(i = 0; i < toDelete->numChildren; i++){
			fstNode *node = pmm_offset_to_pointer(chdlist[i]);
			if(node->isDir){
				__support_delete_child(node);
			}
		}
		pmm_free(chdlist);
	}
	pmm_free(toDelete);
}

// ===========================================================================
// fst_delete_child
// ===========================================================================
int fst_delete_child(fstNode *father, fstNode *node){

	if(father->numChildren == 0){
		fprintf(stderr, "fst_delete_child: father node has no children.\n");
		return -1;
	}
	//get the pointer to the children list
	unsigned long *chdlist = pmm_offset_to_pointer(father->off_children);

	unsigned long *newList = NULL;

	if(father->numChildren > 1){ //we need to create a new list for the remaining items
		newList = pmm_malloc((father->numChildren - 1) * sizeof(unsigned long));
		if(!newList){
			fprintf(stderr, "Error while allocating memory.\n");
			return -1;
		}
	}

	int i, j;

	//we need to find the entry that have to be deleted
	int deleted = 0;
	for(i = 0, j = 0; i < father->numChildren; i++){
		if(pmm_pointer_to_offset(node) == chdlist[i]){
			__support_delete_child(node);
			deleted = 1;
		}else{
			newList[j] = chdlist[i];
			j++;
		}
	}

	if(!deleted){
		fprintf(stderr, "fst_delete_child: entry not found.\n");
		return -1;
	}
	father->numChildren = father->numChildren - 1;

	pmm_free(chdlist);
	if(father->numChildren > 0) //set the new list
		father->off_children = pmm_pointer_to_offset(newList);
	return 0;
}

// ===========================================================================
// fst_add_child
// ===========================================================================
int fst_add_child(fstNode *father, myFile *file, fstNode **node){

	/*allocate space for a new node*/
	fstNode *newNode = (fstNode *) pmm_malloc(sizeof(fstNode));
	if(!newNode){
		fprintf(stderr, "Error while allocating memory with pmm (1) in fst_add_child.\n");
		return -1;
	}

	if(node != NULL) *node = newNode;

	int nlen = strlen(file->name);
	char *pName = (char *) pmm_malloc(sizeof(char) * (nlen + 1));
	if(!pName){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
		return -1;
	}

	strcpy(pName, file->name);
	newNode->off_name = pmm_pointer_to_offset(pName);
	int plen = strlen(file->perms);

	char *pPerms = (char *) pmm_malloc(sizeof(char) * (plen + 1));
	if(!pPerms){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
		return -1;
	}

	strcpy(pPerms, file->perms);
	newNode->off_perms = pmm_pointer_to_offset(pPerms);

	newNode->size = file->size; //file size
	newNode->isDir = file->isDir;
	newNode->lastWriteTimestamp = file->lastWriteTimestamp;
	newNode->isRoot = 0;
	newNode->isMonitored = 0;
	newNode->numChildren = 0;

	/*we now add this new node to the father node*/
	father->numChildren++;
	unsigned long *newList = pmm_malloc(sizeof(unsigned long) * (father->numChildren));
	if(!newList){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
		return -1;
	}
	//if this is the first child
	if(father->numChildren == 1){
		newList[0] = pmm_pointer_to_offset(newNode);
		father->off_children = pmm_pointer_to_offset(newList);
	}else{ //otherwhise
		/*get the pointer to the old list*/
		unsigned long *list = pmm_offset_to_pointer(father->off_children);
		/*we need to realloc (manually) the list*/
		int i;
		for(i = 0; i < (father->numChildren - 1); i++){
			newList[i] = list[i];
		}
		newList[i] = pmm_pointer_to_offset(newNode);
		pmm_free(list);
		father->off_children = pmm_pointer_to_offset(newList);
	}
	return 0;
}

// ===========================================================================
// fst_add_children
// ===========================================================================
int fst_add_children(fstNode *father, myFileList *fList, fstNode ***node, int *count){
	if(fList->count == 0){
			fprintf(stderr, "fst_set_children: no children to add at node %s\n",
				(char *) pmm_offset_to_pointer(father->off_name));
			return -1;
	}
	/*allocate space for a new set of nodes*/
	fstNode **nodeList = (fstNode **) malloc(sizeof(fstNode *) * (fList->count + father->numChildren));
	if(!nodeList){
		fprintf(stderr, "Error while allocating memory with pmm in fst_set_children.\n");
		return -1;
	}

	/*allocate space for the offset list */
	unsigned long *offsetList = pmm_malloc(sizeof(unsigned long) * (father->numChildren + fList->count));
	if(!offsetList){
		fprintf(stderr, "Error while allocating memory with pmm in fst_set_children.\n");
		return -1;
	}
	int tempCount = 0;
	*node = nodeList;
	unsigned long *previousChildren = NULL;
	if(father->numChildren > 0)
		previousChildren = pmm_offset_to_pointer(father->off_children);

	int y; //counter
	//copy old children in the new list
	for(y = 0; y < father->numChildren; y++){
		nodeList[y] = pmm_offset_to_pointer(previousChildren[y]);
		offsetList[y] = previousChildren[y];
		tempCount++;
	}

	if(father->numChildren > 0)
		pmm_free(previousChildren); //delete the old list

	father->off_children = pmm_pointer_to_offset(offsetList);

	int numPreviousChildren = y;
	int j; //counter
	for(j = 0; j < fList->count; j++){

		myFile *file = fList->list[j]; //file
		/*we need to check if the file is already present among the children*/
		int i;
		int present = 0;
		for(i = 0; i < numPreviousChildren; i++){
			if(fname_compare(pmm_offset_to_pointer(nodeList[i]->off_name),
					fList->list[j]->name) == 0){
				present = 1;
				break;
			}
		}
		if(present) continue; //skip
		nodeList[y] = (fstNode *) pmm_malloc(sizeof(fstNode)); //new node

		if(!nodeList[y]){
			fprintf(stderr, "fst_set_children: error while allocating memory.\n");
			return -1;
		}
	  	offsetList[y] = pmm_pointer_to_offset(nodeList[y]);

		/*get the information*/
		int nlen = strlen(file->name);
		char *pName = (char *) pmm_malloc(sizeof(char) * (nlen + 1));
		if(!pName){
			fprintf(stderr, "Error while allocating memory with pmm in fst_set_children.\n");
			return -1;
		}
		strcpy(pName, file->name);
		nodeList[y]->off_name = pmm_pointer_to_offset(pName);

		int plen = strlen(file->perms);
		char *pPerms = (char *) pmm_malloc(sizeof(char) * (plen + 1));
		if(!pPerms){
			fprintf(stderr, "Error while allocating memory with pmm in fst_set_children.\n");
			return -1;
		}
		strcpy(pPerms, file->perms);
		nodeList[y]->off_perms = pmm_pointer_to_offset(pPerms);

		nodeList[y]->size = file->size; //file size
		nodeList[y]->isDir = file->isDir;
		nodeList[y]->lastWriteTimestamp = file->lastWriteTimestamp;
		nodeList[y]->isRoot = 0;
		nodeList[y]->isMonitored = 0;
		nodeList[y]->numChildren = 0;
		y++;
		tempCount++;
	}
	father->numChildren = tempCount;
	*count = tempCount;
	return 0;
}

// ===========================================================================
// __fst_print_tree_rec [SUPPORT FUNCTION]
// ===========================================================================
void __fst_print_tree_rec(fstNode *node, int lev){
	fstNode **list = NULL;
	int size = 0;
	int res = fst_get_children(node, &list, &size);
	if(res == -1){
		fprintf(stderr, "__fst_print_tree_rec: an error occurred while getting" \
		"the node children.\n");
		return;
	}
	int i;
	for(i = 0; i < size; i++){
		printf("%*s| - (marked: %d) Name: %s, type: %d, perms: %s, mod: %lld, size: %lld\n", lev,
		" ", list[i]->isMonitored, (char *) pmm_offset_to_pointer(list[i]->off_name),list[i]->isDir,
			(char *)	pmm_offset_to_pointer(list[i]->off_perms),
			 list[i]->lastWriteTimestamp, list[i]->size);
		if(list[i]->isDir) __fst_print_tree_rec(list[i], lev + 3); //recursive call
	}
}

// ===========================================================================
// fst_print_tree (Debug function)
// ===========================================================================
void fst_print_tree(fstNode *node){
	__fst_print_tree_rec(node, 1);
}
