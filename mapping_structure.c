#include "include/mapping_structure.h"

// ===========================================================================
// initialize__mapping_structure
// ===========================================================================
int initialize_mapping_structure(char *memoryBlock, mappingStructure **str, unsigned int refTime,
	 										char *serverPath){
	/*NOTE: memoryBlock is NULL if the mapping structure has been created by
	the server just before calling this function; in this case this is the
	first function that uses the mapping memory and will be granted the first
	memory block. If memoryBlock is not NULL, then another server created the
	mapping and its structure. We need just to retrieve it (and it is located
	at the first block. The "initialize_management" of pmm has been called before this function call*/

	/*filesystem tree root */
	fstNode *root;
	/*notifications bucket */
	notificationsBucket *firstElement;

	if(memoryBlock != NULL){ //structure already present
		*str = (mappingStructure *) memoryBlock;

		root = pmm_offset_to_pointer((*str)->off_fileSystemTree);
		firstElement = pmm_offset_to_pointer((*str)->off_notifications);

		/*load info in the tree and create bucket*/
		int ret = __first_scan(pmm_offset_to_pointer((*str)->off_fileSystemTree), serverPath);
		if( ret == -1){
			fprintf(stderr, "register_server_path: error while performing the first scan.\n");
			return -1;
		}else if(ret == 2){
				return -2; //path no longer available
		}
		/*update refresh time if necessary*/
		(*str)->refreshTime = (*str)->refreshTime > refTime ? refTime : (*str)->refreshTime;
		(*str)->serverCounter = (*str)->serverCounter + 1;
		/*create an id for the server*/
		(*str)->idCounter = (*str)->idCounter + 1;

		if(nb_add_bucket(firstElement,(*str)->idCounter, serverPath) == -1){
			fprintf(stderr, "initialize_mapping_structure: error while adding a bucket.\n");
			return -1;
		}

		return (*str)->idCounter; //the server id

	}else{
		*str = pmm_malloc(sizeof(mappingStructure)); //first allocation
		if(!(*str)){
			fprintf(stderr, "intialize_mapping_structure: error while allocating memory.\n");
			return -1;
		}
		(*str)->refreshTime = refTime;
		(*str)->serverCounter = 1;
		(*str)->idCounter = 0;
		(*str)->daemonServer = 0;

		if(fst_build_root(&root) == -1) {
			fprintf(stderr, "initialize_mapping_structure: error creating the filesystree root.\n");
			return -1;
		}
		(*str)->off_fileSystemTree = pmm_pointer_to_offset(root);

		/*first scan */
		int ret = __first_scan(root, serverPath);
		if(ret == -1){
			fprintf(stderr, "intialize_mapping_structure: error while performing the first scan.\n");
			return -1;
		}else if(ret == 2){
			return -2;
		}

		if(nb_create(&firstElement, 0, serverPath) == -1){
			fprintf(stderr, "initialize_mapping_structure: error creating the first notifications bucket.\n");
			return -1;
		}
		(*str)->off_notifications = pmm_pointer_to_offset(firstElement);

		return 0;
	}
}

// ===========================================================================
// register_server_path
// ===========================================================================
int register_server_path(mappingStructure *str, unsigned int serverID, char *path){
	notificationsBucket *start = pmm_offset_to_pointer(str->off_notifications);
	if(nb_exists_bucket(start, serverID, path)){
		fprintf(stderr, "register_server_path: path already registered.\n");
		return -1;
	}
	if(nb_add_bucket(start, serverID, path) == -1){
		fprintf(stderr, "initialize_mapping_structure: error while adding a bucket.\n");
		return -1;
	}
	if(__first_scan(pmm_offset_to_pointer(str->off_fileSystemTree), path) == -1){
		fprintf(stderr, "register_server_path: error while performing the first scan.\n");
		return -1;
	}
}

// ===========================================================================
// unregister_server TODO:
// ===========================================================================
int unregister_server(mappingStructure *str, unsigned int sid, char **pathList, int count){
	int i;
	notificationsBucket *start = pmm_offset_to_pointer(str->off_notifications);
	for(i = 0; i < count; i++){
		if(nb_remove_bucket(&start, sid, pathList[i]) == -1){
			fprintf(stderr, "unregister_server: error while removing a bucket.\n");
			return -1;
		}
		//we need to update the offset to the start pointer. Maybe it has been changed in nb_remove_bucket
		str->off_notifications = pmm_pointer_to_offset(start);
	}
	/*now we check if those paths are monitored by other servers. If not, we remove them from the
	filesystem tree (only the branches that are not monitored by others)*/
	short monitored; //used to tell if the current path is monitored by another server
	for(i = 0; i < count; i++){
		  notificationsBucket *current = start;
		  monitored = 0;
		  do{
			if(is_prefix(pathList[i], pmm_offset_to_pointer(current->off_path)) == 0){
			 	//pathList[i] is monitored
				monitored = 1;
				break;
			}
			if(current->off_next == 0) break;
			current = pmm_offset_to_pointer(current->off_next);
		}while(1);

		if(monitored == 0){//we can delete it
			//TODO: delete function
		}
	}
	return 0;
}

// ===========================================================================
// delete_server_path TODO
// ===========================================================================
int __delete_server_path(fstNode *root, char *path){

}

// ===========================================================================
// __support_first_scan
// ===========================================================================
int __support_first_scan(fstNode *root, char *path){

	myFileList fList;
	int ret = get_directory_content(path, &fList);
	if(ret == -1){
		fprintf(stderr, "scan: error while getting the directory content.\n");
		return -1;
	}else if(ret == -2){
		fprintf(stderr, "DEBUG: support first scan torna -2 in %s\n", path);
		return -2;
	}
	if(fList.count > 0){
		fstNode *addedNodes;
		if(fst_add_children(root, &fList, &addedNodes) == -1){
			fprintf(stderr, "scan: error while adding nodes to the tree at \"%s\"\n", path);
			return -1;
		}
		int i;
		for(i = 0; i < fList.count; i++){
			if(fList.list[i].isDir){ //recursive call
				char *fullPath = concatenate_path(path, fList.list[i].name);
				if(!fullPath){
					fprintf(stderr, "scan: error while concantenating the path for %s\n",
						fList.list[i].name);
				return -1;
				}
				if(__support_first_scan(&(addedNodes[i]), fullPath) == -1){
					return -1;
				}
				free(fullPath);
			}
		}
	}
	return 0;
}

// ===========================================================================
// __first_scan
// ===========================================================================
int __first_scan(fstNode *root, char *path){
	//STEP 1: Add a path from the root node to the monitored node
	char **tokens = NULL;
	int numTok = 0;
	if(tokenize_path(path, &tokens, &numTok) == -1){
		fprintf(stderr, "__first_scan: error while tokenizing the path.\n");
		return -1;
	}
	//check if there is already a path that brings to the monitored path
	int i;
	fstNode *current = root;

	//conc_path is used to retrieve the current path that corresponds to our position in the tree
	char *conc_path = malloc(sizeof(char) * (strlen(tokens[0]) + 1));
	if(!conc_path){
		fprintf(stderr, "__first_scan: error while allocating memory.\n");
		return -1;
	}
	strcpy(conc_path, tokens[0]); // copy "/" or "c:\"
	//for each token
	for(i = 0; i < numTok; i++){
		fstNode *nod;
		int ret = fst_contains_child(current, tokens[i], &nod);

		if(ret == -1){ //Error
			fprintf(stderr, "__first_scan: error in contains_child\n");
			return -1;

		}else if(ret == 1){ //true
			current = nod;
			if(i > 0){
				//update the current path
				char *tmp = concatenate_path(conc_path, tokens[i]);
				if(tmp == NULL){
					fprintf(stderr, "__first_scan: error while concatenating a path.\n");
					return -1;
				}
				free(conc_path);
				conc_path = tmp;
			}
		}else{ // ret == 0, so we need to add all the path (without monitor it)
			for(; i < numTok; i++){
				if(i == 0){
					myFile fileRoot;
					fileRoot.name = tokens[i];
					fileRoot.size = 0;
					fileRoot.lastWriteTimestamp = 0;
					fileRoot.perms = "\0";
					fileRoot.isDir = 1;

					fstNode *newNode;
					if(fst_add_child(current, &fileRoot,&newNode) == -1){
						fprintf(stderr, "__first_scan: error while adding a new node.\n");
						return -1;
					}
					newNode->isRoot = 1;
					current = newNode;
				}else{
					//we retrieve the information about the directory being added to the tree (although we don't monitor it)
					myFileList fList;
					int ret = get_directory_content(conc_path, &fList);
					if(ret == -1){
						fprintf(stderr, "__first_scan: error while getting the directory content.\n");
						return -1;
					}else if(ret == -2){ //path has been deleted
						for(i = 0; i < numTok; i++) free(tokens[i]);
						free(tokens);
						free(conc_path);
						return -2;
					}
					//the directory is assumed to exist beacuse the whole path specified by the client must
					//be checked when it is received (so it is checked BEFORE this function call).
					myFile *file;
					if(get_file_info(&fList, tokens[i], &file) == 0){
						fprintf(stderr, "__first_scan: error while getting the file info (%s).\n", tokens[i]);
						return -1;
					}
					fstNode *newNode;
					if(fst_add_child(current, file, &newNode) == -1){
						fprintf(stderr, "__first_scan: error while adding a new node.\n");
						return -1;
					}
					char *tmp = concatenate_path(conc_path, tokens[i]);
					if(tmp == NULL){
						fprintf(stderr, "__first_scan: error while concatenating a path.\n");
						return -1;
					}
					free(conc_path);
					conc_path = tmp;
					current = newNode;
				}
			}
		}
	}
	if(!current->isMonitored){
		current->isMonitored = 1;
		//we need to perform a scan
		if(__support_first_scan(current, conc_path) == -1){
			fprintf(stderr, "Error while performing the first scan.\n");
			return -1;
		}
	}

	for(i = 0; i < numTok; i++) free(tokens[i]);
	free(tokens);
	free(conc_path);
	return 0;
}

// ===========================================================================
// uodate
// ===========================================================================
int update(mappingStructure *str){
	fstNode *root = pmm_offset_to_pointer(str->off_fileSystemTree);
	return scan(root, "", str, 0);
}

// ===========================================================================
// scan
// ===========================================================================
int scan(fstNode *currentNode, char *path, mappingStructure *str, int monitored){

	if(!monitored)monitored = currentNode->isMonitored;
	/*get the node's children*/
	fstNode **nList;
	int nListSize;
	if(fst_get_children(currentNode, &nList, &nListSize) == -1){
		fprintf(stderr, "scan: error while getting the node's children.\n");
		return -1;
	}

	if(monitored){ //check this level
		notificationsBucket *startBucket = pmm_offset_to_pointer(str->off_notifications);
		//get all the files in the current directory
		myFileList fList;
		int ret = get_directory_content(path, &fList);
		if(ret == -1){
			fprintf(stderr, "scan: error while getting the directory content.\n");
			return -1;
		}else if(ret == -2){
			return 0;
		}

		int i, j;
		int *checkVector = calloc(nListSize, sizeof(int));
		if(!checkVector){
			fprintf(stderr, "scan: error while allocating memory.\n");
			return -1;
		}
		//DEB:printf("Inizio dir %s\n", path);
		for(i = 0; i < fList.count; i++){

			int ffound = 0;
			char *fullFPath = concatenate_path(path, fList.list[i].name);
			if(!fullFPath){
				fprintf(stderr, "scan: error while concantenating the path for %s\n",
					fList.list[i].name);
				return -1;
			}
			for(j = 0; j < nListSize; j++){
				if(strcicmp(pmm_offset_to_pointer(nList[j]->off_name), fList.list[i].name) == 0 &&
					fList.list[i].isDir  == nList[j]->isDir){
					checkVector[j] = 1; //this means that it has not been deleted
					ffound = 1;
					//check changes
					int ck_perms = 0;
					int ck_size = 0;
					int ck_mod = 0;

					//size
					if(nList[j]->size != fList.list[i].size) ck_size = 1;
					//perms
					if(strcicmp(pmm_offset_to_pointer(nList[j]->off_perms), fList.list[i].perms) != 0) ck_perms = 1;
					//mode
					if(nList[j]->lastWriteTimestamp != fList.list[i].lastWriteTimestamp &&
						nList[j]->size == fList.list[i].size) ck_mod = 1;
					//update info
				   nList[j]->size = fList.list[i].size;
					nList[j]->lastWriteTimestamp = fList.list[i].lastWriteTimestamp;
					pmm_free(pmm_offset_to_pointer(nList[j]->off_perms));
					int newPermsLen = strlen(fList.list[i].perms) + 1;
					char *np = pmm_malloc(sizeof(char) * newPermsLen);
					if(!np){
						fprintf(stderr, "scan: error while allocating memory.\n");
						return -1;
					}
					strcpy(np, fList.list[i].perms);
					nList[j]->off_perms = pmm_pointer_to_offset(np);
					//DEB:printf("fine modifiche\n");
					//send notifications
					if(ck_size == 1){
						if(nb_push_notification(startBucket, fList.list[i].perms, fullFPath,
							fList.list[i].size, fList.list[i].lastWriteTimestamp,
								fList.list[i].isDir, dimension) == -1){
							fprintf(stderr, "scan: error while pushing notification for %s.\n", fList.list[i].name);
							return -1;
						}
					}else if(ck_mod == 1){
						if(nb_push_notification(startBucket, fList.list[i].perms, fullFPath,
							fList.list[i].size, fList.list[i].lastWriteTimestamp,
								fList.list[i].isDir, cambiocon) == -1){
							fprintf(stderr, "scan: error while pushing notification for %s.\n", fList.list[i].name);
							return -1;
						}
					}
					if(ck_perms == 1){
						if(nb_push_notification(startBucket, fList.list[i].perms, fullFPath,
							fList.list[i].size, fList.list[i].lastWriteTimestamp,
								fList.list[i].isDir, perms) == -1){
							fprintf(stderr, "scan: error while pushing notification for %s.\n", fList.list[i].name);
							return -1;
						}
					}
					break;
				}
			}
			//DEB:printf("inizio processing nuovi file.\n");
			if(!ffound){ //this means that its a new file
				fstNode *newNode;
				if(fst_add_child(currentNode, fList.list + i, &newNode) == -1){
					fprintf(stderr, "__first_scan: error while adding a new node.\n");
					return -1;
				}
				if(nb_push_notification(startBucket, fList.list[i].perms, fullFPath,
					fList.list[i].size, fList.list[i].lastWriteTimestamp,
						fList.list[i].isDir, creation) == -1){
					fprintf(stderr, "scan: error while pushing notification for %s.\n", fList.list[i].name);
					return -1;
				}
			}
			//free memory
			free(fList.list[i].name);
			free(fList.list[i].perms);
			free(fullFPath);
		}

		free(fList.list);
		//DEB:printf("inizio cancellati.\n");
		//finally we check for deleted nodes
 		for(j = 0; j < nListSize; j++){
			if(checkVector[j] == 0){
				char *name = pmm_offset_to_pointer(nList[j]->off_name);
				char *fullFPath = concatenate_path(path, name);

				if(!fullFPath){
					fprintf(stderr, "scan: error while concantenating the path for %s\n",
						name);
					return -1;
				}
				if(nb_push_notification(startBucket, pmm_offset_to_pointer(nList[j]->off_perms), fullFPath,
					nList[j]->size, nList[j]->lastWriteTimestamp, nList[j]->isDir, deletion) == -1){
					fprintf(stderr, "scan: error while pushing notification for %s.\n", name);
					return -1;
				}
				if(nList[j]->isDir) free(fullFPath);
				if(fst_delete_child(currentNode, nList[j]) == -1){
					fprintf(stderr, "scan: error while deleting a node.\n");
					return -1;
				}
			}
		}

		if(!nList) free(nList);
		if(fst_get_children(currentNode, &nList, &nListSize) == -1){
			fprintf(stderr, "scan: error while getting the node's children.\n");
			return -1;
		}
	}
	//DEB:printf("Chiamate ricorsive.\n");
	//recursive calls
	int i;
	for(i = 0; i < nListSize; i++){
		if(nList[i]->isDir){
			char *newPath = concatenate_path(path, pmm_offset_to_pointer(nList[i]->off_name));
			if(!newPath){
				fprintf(stderr, "scan: error while concantenating the path for %s\n",
					(char *) pmm_offset_to_pointer(nList[i]->off_name));
				return -1;

			}
			if(scan(nList[i], newPath, str, monitored) == -1) {
				fprintf(stderr, "scan: error while processing \"%s\"\n", path);
				return -1;
			}
			free(newPath);
		}
	}
	return 0;
}

// ===========================================================================
// print_mappingstructure_state
// ===========================================================================
void print_mappingstructure_state(mappingStructure *str){
	printf("-------------------MAPPING STRUCTURE----------------------------\n\n");
	/*printf("SYSTREE:\n");
	fst_print_tree(pmm_offset_to_pointer(str->off_fileSystemTree));*/
	printf("\n\nNOTIFICATIONS BUCKETS\n");
	nb_print_notification_buckets(pmm_offset_to_pointer(str->off_notifications));
}
