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
		fprintf(stderr, "fst_get_children: input non valid.\n");
		return -1;
	}
	*size = father->numChildren;
	if(father->numChildren == 0){
		return 1;
	}
	//get the pointer to the children list
	long *chdlist = pmm_offset_to_pointer(father->off_children);

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
// fst_delete_child
// ===========================================================================
int fst_delete_child(fstNode *father, fstNode *node){
	if(!father || !node){
		fprintf(stderr, "fst_delete_child: params not valid.\n");
		return -1;
	}
	if(father->numChildren == 0){
		fprintf(stderr, "fst_delete_child: father node has no children.\n");
		return -1;
	}
	//get the pointer to the children list
	long *chdlist = pmm_offset_to_pointer(father->off_children);

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
		if(node == pmm_offset_to_pointer(chdlist[i])){
			//free resources inside the structs (perms and name)
			pmm_free(pmm_offset_to_pointer(node->off_perms));
			pmm_free(pmm_offset_to_pointer(node->off_name));
			pmm_free(node); //free the struct
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
	if(!father || !file){
		fprintf(stderr, "fst_add_child: one of the params is not valid.\n");
		return -1;
	}


	/*allocate space for a new node*/
	fstNode *newNode = (fstNode *) pmm_malloc(sizeof(fstNode));	
	if(!newNode){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
		return -1;
	}
	
	if(node != NULL) *node = newNode;
	int nlen = strlen(file->name);

	char *pName = (char *) pmm_malloc(sizeof(char) * (nlen + 1));
	if(!pName){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
		pmm_free(newNode);
		return -1;
	}

	if(!(file->perms)){
		printf("Error perms.\n");
		exit(0);
	}
	strcpy(pName, file->name);
	newNode->off_name = pmm_pointer_to_offset(pName);
	int plen = strlen(file->perms);

	char *pPerms = (char *) pmm_malloc(sizeof(char) * (plen + 1));
	if(!pPerms){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
		pmm_free(newNode);
		pmm_free(pName);
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
	unsigned long *newList = (unsigned long *) pmm_malloc(sizeof(unsigned long) * (father->numChildren));
	if(!newList){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
		pmm_free(newNode);
		pmm_free(pName);
		pmm_free(pPerms);
		return -1;
	}
	if(father->numChildren == 1){
		newList[0] = pmm_pointer_to_offset(newNode);
		father->off_children = pmm_pointer_to_offset(newList);
	}else{
		long *list = pmm_offset_to_pointer(father->off_children); //retrieve pointer to the list
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
int fst_add_children(fstNode *father, myFileList *fList, fstNode **node){
	if(!father || !fList){
		fprintf(stderr, "fst_add_child: one of the params is not valid.\n");
		return -1;
	}


	/*allocate space for a new set of nodes*/
	fstNode *nodeList = (fstNode *) pmm_malloc(sizeof(fstNode) * fList->count);

	if(!nodeList){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_children.\n");
		return -1;
	}
	if(node != NULL) *node = nodeList;
	int y; //counter
	for(y = 0; y < fList->count; y++){
	
		myFile *file = &(fList->list[y]); 
		int nlen = strlen(file->name);

		char *pName = (char *) pmm_malloc(sizeof(char) * (nlen + 1));
		if(!pName){
			fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
			pmm_free(nodeList);
			return -1;
		}

		if(!(file->perms)){
			printf("Error perms.\n");
			exit(0);
		}
		strcpy(pName, file->name);
		nodeList[y].off_name = pmm_pointer_to_offset(pName);
		int plen = strlen(file->perms);	

		char *pPerms = (char *) pmm_malloc(sizeof(char) * (plen + 1));
		if(!pPerms){
			fprintf(stderr, "Error while allocating memory with pmm in fst_add_child.\n");
			pmm_free(nodeList);
			pmm_free(pName);
			return -1;
		}

		strcpy(pPerms, file->perms);
		nodeList[y].off_perms = pmm_pointer_to_offset(pPerms);

		nodeList[y].size = file->size; //file size
		nodeList[y].isDir = file->isDir;
		nodeList[y].lastWriteTimestamp = file->lastWriteTimestamp;
		nodeList[y].isRoot = 0;
		nodeList[y].isMonitored = 0;
		nodeList[y].numChildren = 0;
	}
	/*we now add this new node to the father node*/
	father->numChildren += fList->count;
	unsigned long *newList = (unsigned long *) pmm_malloc(sizeof(unsigned long) * (father->numChildren));
	if(!newList){
		fprintf(stderr, "Error while allocating memory with pmm in fst_add_children.\n");
		for(y = 0; y < fList->count; y++){
			pmm_free(pmm_offset_to_pointer(nodeList[y].off_name));
			pmm_free(pmm_offset_to_pointer(nodeList[y].off_perms));
		}
		pmm_free(nodeList);
		return -1;
	}
	if(father->numChildren == fList->count){
		for(y = 0; y < fList->count; y++)
			newList[y] = pmm_pointer_to_offset(nodeList + y);
		father->off_children = pmm_pointer_to_offset(newList);
	}else{
		long *list = pmm_offset_to_pointer(father->off_children); //retrieve pointer to the list
		/*we need to realloc (manually) the list*/
		int i, j;
		for(i = 0; i < (father->numChildren - fList->count); i++){
			newList[i] = list[i];
		}
		for( j = 0; i < father->numChildren;j++, i++)
			newList[i] = pmm_pointer_to_offset(nodeList + j);
		pmm_free(list);
		father->off_children = pmm_pointer_to_offset(newList);
	}
	return 0;
}
// ===========================================================================
// fst_get_name
// ===========================================================================
char *fst_get_name(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_get_name: node is null.\n");
		return NULL;
	}
	return pmm_offset_to_pointer(node->off_name);
}

// ===========================================================================
// fst_get_perms
// ===========================================================================
char *fst_get_perms(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_get_perms: node is null.\n");
		return NULL;
	}
	return pmm_offset_to_pointer(node->off_perms);
}

// ===========================================================================
// fst_get_size
// ===========================================================================
long long fst_get_size(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_get_size: node is null.\n");
		return -1;
	}
	return node->size;
}

// ===========================================================================
// fst_get_wtime
// ===========================================================================
long long fst_get_wtime(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_get_wtime: node is null.\n");
		return -1;
	}
	return node->lastWriteTimestamp;
}

// ===========================================================================
// fst_is_dir
// ===========================================================================
int fst_is_dir(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_is_dir: node is null.\n");
		return -1;
	}
	return node->isDir;
}

// ===========================================================================
// fst_is_root
// ===========================================================================
int fst_is_root(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_is_root: node is null.\n");
		return -1;
	}
	return node->isRoot;
}

// ===========================================================================
// fst_is_monitored
// ===========================================================================
int fst_is_monitored(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_is_monitored: node is null.\n");
		return -1;
	}
	return node->isMonitored;
}

// ===========================================================================
// fst_get_num_children
// ===========================================================================
int fst_get_num_children(fstNode *node){
	if(!node){
		fprintf(stderr, "fst_get_num_children: node is null.\n");
		return -1;
	}
	return node->numChildren;
}

// ===========================================================================
// __fst_print_tree_rec [SUPPORT FUNCTION]
// ===========================================================================
void __fst_print_tree_rec(fstNode *node, int lev){
	fstNode **list = NULL;
	int size = 0;
	int res = fst_get_children(node, &list, &size);
	if(res == -1){
		fprintf(stderr, "__fst_print_tree_rec: an error occurred while getting the node children.\n");
		return;
	}
	int i;
	for(i = 0; i < size; i++){
		printf("%*s| - Nome: %s, perms: %s, mod: %lld, size: %lld\n", lev, " ", fst_get_name(list[i]),
					fst_get_perms(list[i]), fst_get_wtime(list[i]), fst_get_size(list[i]));
		if(fst_is_dir(list[i])) __fst_print_tree_rec(list[i], lev + 3); //recursive call
	}
}

// ===========================================================================
// fst_print_tree (Debug function)
// ===========================================================================
void fst_print_tree(fstNode *node){
	__fst_print_tree_rec(node, 1);
}
