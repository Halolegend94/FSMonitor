#include "include/mapping_structure.h"

/*function prototypes*/
int __dfs_clean_subtree(fstNode *root);
int __unmark_server_subtree(fstNode *root, char **tokens, int index, int target);
int __first_scan(fstNode *root, char *path);
int __scan(fstNode *currentNode, char *path, mappingStructure *str, int monitored);

// ===========================================================================
// initialize__mapping_structure
// ===========================================================================
int initialize_mapping_structure(char *memoryBlock, mappingStructure **str,
		unsigned int refTime, char *serverPath){
	/*NOTE: memoryBlock is NULL if the mapping structure has been created by
	the server just before calling this function; in this case this is the
	first function that uses the mapping memory and will be granted the first
	memory block. If memoryBlock is not NULL, then another server created the
	mapping and its structure. We need just to retrieve it (and it is located
	at the first block. The "initialize_management" of pmm has been called before
	this function call*/

	/*filesystem tree root */
	fstNode *root;
	/*notifications bucket */
	notificationsBucket *firstElement;

	if(memoryBlock != NULL){ //structure already present
		*str = (mappingStructure *) memoryBlock;
		/*root of filesystem tree*/
		root = pmm_offset_to_pointer((*str)->off_fileSystemTree);
		/*first notification bucket*/
		firstElement = pmm_offset_to_pointer((*str)->off_notifications);
		/*perform a first scan and create bucket*/
		int ret = __first_scan(pmm_offset_to_pointer((*str)->off_fileSystemTree), serverPath);

		if( ret == PROG_ERROR){ //error
			fprintf(stderr, "initialize_mapping_structure: error while performing the first scan.\n");
			return PROG_ERROR;
		}else if(ret == PATH_NOT_ACCESSIBLE){
				return PATH_NOT_ACCESSIBLE; //path no longer available
		}
		unsigned long long lastUpdate = get_current_time();
		if(lastUpdate == 0){
			fprintf(stderr, "initialize_mapping_structure: error while getting the current time.\n");
			return PROG_ERROR;
		}
		(*str)->lastUpdate = lastUpdate;
		/*update refresh time if necessary*/
		(*str)->refreshTime = (*str)->refreshTime > refTime ? refTime : (*str)->refreshTime;
		(*str)->serverCounter = (*str)->serverCounter + 1;
		/*create an id for the server*/
		(*str)->idCounter = (*str)->idCounter + 1;

		if(nb_add_bucket(firstElement,(*str)->idCounter, serverPath) == -1){
			fprintf(stderr, "initialize_mapping_structure: error while adding a bucket.\n");
			return PROG_ERROR;
		}
		return (*str)->idCounter; //the server id

	}else{ //the structure must be created

		*str = pmm_malloc(sizeof(mappingStructure)); //first allocation
		if(!(*str)){
			fprintf(stderr, "intialize_mapping_structure: error while allocating memory.\n");
			return PROG_ERROR;
		}
		(*str)->refreshTime = refTime;
		(*str)->serverCounter = 1;
		(*str)->idCounter = 0;
		(*str)->daemonServer = 0;

		//the filesystem tree root
		if(fst_build_root(&root) == PROG_ERROR) {
			fprintf(stderr, "initialize_mapping_structure: error creating the filesystree root.\n");
			return PROG_ERROR;
		}
		(*str)->off_fileSystemTree = pmm_pointer_to_offset(root);

		/*first scan */
		int ret = __first_scan(root, serverPath);
		if(ret == PROG_ERROR){
			fprintf(stderr, "intialize_mapping_structure: error while performing the first scan.\n");
			return PROG_ERROR;
		}else if(ret == PATH_NOT_ACCESSIBLE){
			return PATH_NOT_ACCESSIBLE;
		}

		if(nb_create(&firstElement, 0, serverPath) == -1){
			fprintf(stderr, "initialize_mapping_structure: error creating the first notifications bucket.\n");
			return PROG_ERROR;
		}
		(*str)->off_notifications = pmm_pointer_to_offset(firstElement);

		return PROG_SUCCESS;
	}
}

// ===========================================================================
// register_server_path
// ===========================================================================
int register_server_path(mappingStructure *str, int serverID, char *path){
	/*add the necessary branch to the filesystree*/
	int ret = __first_scan(pmm_offset_to_pointer(str->off_fileSystemTree), path);
	if( ret == PROG_ERROR){ //error
		fprintf(stderr, "register_server_path: error while performing the first scan.\n");
		return PROG_ERROR;
	}else if(ret == PATH_NOT_ACCESSIBLE){
			return PATH_NOT_ACCESSIBLE; //path no longer available
	}

	/*add the notification bucket*/
	notificationsBucket *start = pmm_offset_to_pointer(str->off_notifications);
	notificationsBucket *buck;
	ret = nb_exists_bucket(start, serverID, path, &buck);
	if(ret == 1){
			if(buck == NULL){
				return PATH_ALREADY_MONITORED;
			}else{
				/*need to update the bucket's associated path*/
				 if(nb_update_bucket_path(buck, path) == PROG_ERROR){
					 fprintf(stderr, "register_server_path: error while updating a bucket path.\n");
					 return PROG_ERROR;
				 }
			}
	}else{
		if(nb_add_bucket(start, serverID, path) == -1){
			fprintf(stderr, "register_server_path: error while adding a bucket.\n");
			return PROG_ERROR;
		}
	}
	return 0;
}

// ===========================================================================
// unregister_server
// ===========================================================================
int unregister_server(mappingStructure *str, int sid, char **pathList, int count){
	int i;
	notificationsBucket *start = pmm_offset_to_pointer(str->off_notifications);
	fstNode *root = pmm_offset_to_pointer(str->off_fileSystemTree);

	for(i = 0; i < count; i++){
		if(nb_remove_bucket(start, sid, pathList[i]) == -1){
			fprintf(stderr, "unregister_server: error while removing a bucket.\n");
			return PROG_ERROR;
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
		  if(start != NULL){
			  char *tPath = concatenate_path(pathList[i], "");
			  if(!tPath){
				  fprintf(stderr, "unregister_server: error while concatenating a path (%s).\n", pathList[i]);
				  return PROG_ERROR;
			  }
			  do{
				  if(fname_compare(tPath, pmm_offset_to_pointer(current->off_path)) == 0){
					  //pathList[i] is monitored
					  monitored = 1;
					  break;
				  }
				  if(current->off_next == 0) break;
				  current = pmm_offset_to_pointer(current->off_next);
			  }while(1);
			  free(tPath);
		  }
		  if(monitored == 0){//we can delete the subtrees that are not monitored
			  char **tokens = NULL;
			  int numTok = 0;
			  int monitored = 0; //tells if the path was already monitored
			  if(tokenize_path(pathList[i], &tokens, &numTok) == -1){
				  fprintf(stderr, "__first_scan: error while tokenizing the path.\n");
				  return PROG_ERROR;
			  }
			  if(__unmark_server_subtree(root, tokens, 0, numTok - 1) == -1) return PROG_ERROR;
		  }
	  }
	  if(__dfs_clean_subtree(root) == -1) return PROG_ERROR;
	  return 0;
  }

// ===========================================================================
// __dfs_clean_subtree
// ===========================================================================
int __dfs_clean_subtree(fstNode *root){
	int delete = !(root->isMonitored);
	if(delete){
		//first step: we need to see if all of its children can be deleted
		fstNode **nodes;
		unsigned int size;
		if(fst_get_children(root, &nodes, &size) == -1){
			fprintf(stderr, "__dfs_clean_subtree: error while getting the root's children.\n");
			return PROG_ERROR;
		}
		int i;
		int numDeleted = 0;
		for(i = 0; i < size; i++){
			int ret = __dfs_clean_subtree(nodes[i]);
			if(ret == -1) return PROG_ERROR; //error
			else if( ret == 1){ //we can delete the node, all its children have been deleted
				nodes[i] = NULL; //mark the element
				numDeleted++;
			}else{
				delete = 0; //root cannot be deleted
			}
		}
		if(delete) {
			if(root->numChildren > 0){
				pmm_free(pmm_offset_to_pointer(root->off_children));
				root->numChildren = 0;
				free(nodes);
			}
			if(!root->isRoot){
				pmm_free(pmm_offset_to_pointer(root->off_name));
				pmm_free(pmm_offset_to_pointer(root->off_perms));
				pmm_free(root);
			}
			return 1;
		}else{
			if(size > 0 && numDeleted > 0){
				unsigned long *newList = pmm_malloc(sizeof(unsigned long) * (size - numDeleted));
				if(!newList){
					fprintf(stderr, "__dfs_clean_subtree: error while allocating memory.\n");
					return PROG_ERROR;
				}
				int j;
				for(i = 0, j = 0; i < size; i++){
					if(nodes[i] != NULL){
						newList[j] = pmm_pointer_to_offset(nodes[i]);
						j++;
					}
				}
				pmm_free(pmm_offset_to_pointer(root->off_children));
				root->off_children = pmm_pointer_to_offset(newList);
				root->numChildren = j;
				free(nodes);
			}
			return 0;
		}
	}
	return 0;
}

// ===========================================================================
// delete_server_path
// ===========================================================================
int __unmark_server_subtree(fstNode *root, char **tokens, int index, int target){
	fstNode *nod;
	int ret = fst_contains_child(root, tokens[index], &nod);
	if(ret == -1){
		fprintf(stderr, "__delete_server_subtree: error while checking for a child "\
		"to be in root's children list.\n");
		return PROG_ERROR;
	}else if(ret == 1){
		if(index < target){
			return __unmark_server_subtree(nod, tokens, index + 1, target);
		}else{
			nod->isMonitored = 0;
			return 0;
		}
	}else{ //the path must have been deleted on the filesystem
		return 0;
	}
}


// ===========================================================================
// __support_first_scan
// ===========================================================================
int __support_first_scan(fstNode *root, char *path){

	myFileList fList;
	int ret = get_directory_content(path, &fList);
	if(ret == -1){
		fprintf(stderr, "scan: error while getting the directory content.\n");
		return PROG_ERROR;
	}else if(ret == -2){
		return -2;
	}
	if(fList.count > 0){
		fstNode **addedNodes;
		int count;
		if(fst_add_children(root, &fList, &addedNodes, &count) == -1){
			fprintf(stderr, "scan: error while adding nodes to the tree at \"%s\"\n", path);
			return PROG_ERROR;
		}
		int i;
		for(i = 0; i < count; i++){
			if(addedNodes[i]->isDir && !(addedNodes[i]->isMonitored)){ //recursive call
				char *fullPath = concatenate_path(path, pmm_offset_to_pointer(addedNodes[i]->off_name));
				if(!fullPath){
					fprintf(stderr, "scan: error while concantenating the path\n");
				return PROG_ERROR;
				}
				if(__support_first_scan(addedNodes[i], fullPath) == -1){
					return PROG_ERROR;
				}
				free(fullPath);
			}
		}
		free(addedNodes);
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
	int monitored = 0; //tells if the path was already monitored
	if(tokenize_path(path, &tokens, &numTok) == -1){
		fprintf(stderr, "__first_scan: error while tokenizing the path.\n");
		return PROG_ERROR;
	}
	//check if there is already a path that brings to the monitored path
	int i;
	fstNode *current = root;

	//conc_path is used to retrieve the current path that corresponds to our position in the tree
	char *conc_path = malloc(sizeof(char) * (strlen(tokens[0]) + 1));
	if(!conc_path){
		fprintf(stderr, "__first_scan: error while allocating memory.\n");
		return PROG_ERROR;
	}
	strcpy(conc_path, tokens[0]); // copy "/" or "c:\"
	//for each token
	int ret = 0; //return value
	for(i = 0; i < numTok; i++){
		fstNode *nod;
		ret = fst_contains_child(current, tokens[i], &nod);

		if(ret == -1){ //Error
			fprintf(stderr, "__first_scan: error in contains_child\n");
			return PROG_ERROR;

		}else if(ret == 1){ //true
			current = nod;
			monitored = monitored == 0 ? nod->isMonitored : monitored;
			if(i > 0){
				//update the current path
				char *tmp = concatenate_path(conc_path, tokens[i]);
				if(tmp == NULL){
					fprintf(stderr, "__first_scan: error while concatenating a path.\n");
					return PROG_ERROR;
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
					fileRoot.perms = "";
					fileRoot.isDir = 1;

					fstNode *newNode;
					if(fst_add_child(current, &fileRoot,&newNode) == -1){
						fprintf(stderr, "__first_scan: error while adding a new node.\n");
						return PROG_ERROR;
					}
					newNode->isRoot = 0;
					current = newNode;
				}else{
					//we retrieve the information about the directory being added to the tree (although we don't monitor it)
					myFileList fList;
					int ret = get_directory_content(conc_path, &fList);
					if(ret == -1){
						fprintf(stderr, "__first_scan: error while getting the directory content.\n");
						return PROG_ERROR;
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
						return PROG_ERROR;
					}
					fstNode *newNode;
					if(fst_add_child(current, file, &newNode) == -1){
						fprintf(stderr, "__first_scan: error while adding a new node (%s).\n", tokens[i]);
						return PROG_ERROR;
					}
					char *tmp = concatenate_path(conc_path, tokens[i]);
					if(tmp == NULL){
						fprintf(stderr, "__first_scan: error while concatenating a path.\n");
						return PROG_ERROR;
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
		if(monitored == 0)
		 if(__support_first_scan(current, conc_path) == -1){
			fprintf(stderr, "Error while performing the first scan.\n");
			return PROG_ERROR;
		}
	}

	for(i = 0; i < numTok; i++) free(tokens[i]);
	free(tokens);
	free(conc_path);
	return 0;
}

// ===========================================================================
// update
// ===========================================================================
int update(mappingStructure *str){
	fstNode *root = pmm_offset_to_pointer(str->off_fileSystemTree);
	if(__scan(root, "", str, 0) == -1){
		fprintf(stderr, "update: error while scanning the filesystem.\n");
		return PROG_ERROR;
	}
	unsigned long long ct = get_current_time();
	if(ct == 0){
		fprintf(stderr, "update: error while getting the current time.\n");
		return PROG_ERROR;
	}
	str->lastUpdate = ct;
	return 0;
}

// ===========================================================================
// __scan
// ===========================================================================
int __scan(fstNode *currentNode, char *path, mappingStructure *str, int monitored){
	/*the following line is used to tell the function not to scan directories that
	only serve as path to other monitored direcotries.*/
	if(!monitored)monitored = currentNode->isMonitored;
	notificationsBucket *startBucket = pmm_offset_to_pointer(str->off_notifications);
	/*get the node's children*/
	fstNode **nList = NULL;
	int nListSize;
	if(fst_get_children(currentNode, &nList, &nListSize) == -1){
		fprintf(stderr, "scan: error while getting the node's children.\n");
		return PROG_ERROR;
	}

	if(monitored){ //need to scan
		//used to push notifications
		//get all the files in the current directory
		myFileList fList;
		int ret = get_directory_content(path, &fList);
		if(ret == -1){
			fprintf(stderr, "scan: error while getting the directory content.\n");
			return PROG_ERROR;
		}else if(ret == -2){ //error opening the directory. skip
			return 0;
		}

		int i, j; //counters
		/*the following vector is used to memorize which node of nList is found in fList*/
		int *checkVector = calloc(nListSize, sizeof(int));
		if(!checkVector){
			fprintf(stderr, "scan: error while allocating memory.\n");
			return PROG_ERROR;
		}
		unsigned long long mtime = get_current_time(); //the time of the check
		if(mtime == 0){
			fprintf(stderr, "scan: error while getting the current timestamp.\n");
			return PROG_ERROR;
		}
		/*for each file in the direcory*/
		for(i = 0; i < fList.count; i++){
			int ffound = 0; //used to tell if it is already present in the fs-tree
			char *fullFPath = concatenate_path(path, fList.list[i]->name);
			if(!fullFPath){
				fprintf(stderr, "scan: error while concantenating the path for %s\n",
									fList.list[i]->name);
				return PROG_ERROR;
			}
			for(j = 0; j < nListSize; j++){
				/*if there is a match..*/
				if(fname_compare(pmm_offset_to_pointer(nList[j]->off_name),
						fList.list[i]->name) == 0 && fList.list[i]->isDir  == nList[j]->isDir){

					checkVector[j] = 1; //this means that it has not been deleted
					ffound = 1;
					//check changes
					int ck_perms = 0;
					int ck_size = 0;
					int cambio_con = 0;
					//size
					if(nList[j]->size != fList.list[i]->size) ck_size = 1;
					//perms
					if(fname_compare(pmm_offset_to_pointer(nList[j]->off_perms),
									fList.list[i]->perms) != 0) ck_perms = 1;

					//cambiocon
					if(nList[j]->lastWriteTimestamp != fList.list[i]->lastWriteTimestamp &&
						nList[j]->size == fList.list[i]->size && !nList[j]->isDir)
							cambio_con = 1; //file changed but same size. (not much sense for folders)

					//update info
				  	nList[j]->size = fList.list[i]->size;
					nList[j]->lastWriteTimestamp = fList.list[i]->lastWriteTimestamp;

					if(ck_perms == 1){
						pmm_free(pmm_offset_to_pointer(nList[j]->off_perms));
						int newPermsLen = strlen(fList.list[i]->perms) + 1;
						char *np = pmm_malloc(sizeof(char) * newPermsLen);
						if(!np){
							fprintf(stderr, "scan: error while allocating memory.\n");
							return PROG_ERROR;
						}
						strcpy(np, fList.list[i]->perms);
						nList[j]->off_perms = pmm_pointer_to_offset(np);
						/*send notification*/
						if(nb_push_notification(startBucket, np, fullFPath,
							fList.list[i]->size, mtime,
								fList.list[i]->isDir, perms) == -1){
							fprintf(stderr, "scan: error while pushing notification for %s.\n", fList.list[i]->name);
							return PROG_ERROR;
						}
					}
					if(ck_size == 1){
						if(nb_push_notification(startBucket, fList.list[i]->perms, fullFPath,
							fList.list[i]->size, fList.list[i]->lastWriteTimestamp,
								fList.list[i]->isDir, dimension) == -1){
							fprintf(stderr, "scan: error while pushing notification for %s.\n", fList.list[i]->name);
							return PROG_ERROR;
						}
					}else if(cambio_con == 1){
						if(nb_push_notification(startBucket, fList.list[i]->perms, fullFPath,
							fList.list[i]->size, fList.list[i]->lastWriteTimestamp,
								fList.list[i]->isDir, cambiocon) == -1){
							fprintf(stderr, "scan: error while pushing notification for %s.\n", fList.list[i]->name);
							return PROG_ERROR;
						}
					}
					break;
				}
			}

			if(!ffound){ //this means that its a new file
				fstNode *newNode;
				if(fst_add_child(currentNode, fList.list[i], &newNode) == -1){
					fprintf(stderr, "__first_scan: error while adding a new node.\n");
					return PROG_ERROR;
				}
				if(nb_push_notification(startBucket, fList.list[i]->perms, fullFPath,
					fList.list[i]->size, mtime,
						fList.list[i]->isDir, creation) == -1){
					fprintf(stderr, "scan: error while pushing notification for %s.\n",
							fList.list[i]->name);
					return PROG_ERROR;
				}
			}
			//free memory
			free(fList.list[i]->name);
			free(fList.list[i]->perms);
			free(fList.list[i]);
			free(fullFPath);
		}
		free(fList.list);

		//finally we check for deleted nodes
 		for(j = 0; j < nListSize; j++){
			if(checkVector[j] == 0){
				char *name = pmm_offset_to_pointer(nList[j]->off_name);
				char *fullFPath = concatenate_path(path, name);

				if(!fullFPath){
					fprintf(stderr, "scan: error while concantenating the path for %s\n",
						name);
					return PROG_ERROR;
				}

				if(nb_push_notification(startBucket, pmm_offset_to_pointer(nList[j]->off_perms),
						fullFPath, nList[j]->size, mtime,
								nList[j]->isDir, deletion) == -1){
					fprintf(stderr, "scan: error while pushing notification for %s.\n", name);
					return PROG_ERROR;
				}
				free(fullFPath);
				if(fst_delete_child(currentNode, nList[j]) == -1){
					fprintf(stderr, "scan: error while deleting a node.\n");
					return PROG_ERROR;
				}
			}
		}

		if(!nList){
			free(nList);
			nList = NULL;
		}
		if(fst_get_children(currentNode, &nList, &nListSize) == -1){
			fprintf(stderr, "scan: error while getting the node's children.\n");
			return PROG_ERROR;
		}
	}

	//recursive calls
	int i;
	for(i = 0; i < nListSize; i++){
		if(nList[i]->isDir == 1){
			char *newPath = concatenate_path(path, pmm_offset_to_pointer(nList[i]->off_name));
			if(!newPath){
				fprintf(stderr, "scan: error while concantenating the path for %s\n",
					(char *) pmm_offset_to_pointer(nList[i]->off_name));
				return PROG_ERROR;

			}
			if(!monitored){
				if(!currentNode->isRoot && !is_directory(newPath)){ //the path has been deleted
					if(nb_push_notification(startBucket, pmm_offset_to_pointer(nList[i]->off_perms),
							newPath, nList[i]->size, nList[i]->lastWriteTimestamp,
									nList[i]->isDir, deletion) == -1){
						fprintf(stderr, "scan: error while pushing notification\n");
						return PROG_ERROR;
					}
					if(fst_delete_child(currentNode, nList[i]) == -1){
						fprintf(stderr, "scan: error while deleting a node.\n");
						return PROG_ERROR;
					}
					continue;
				}
			}
			if(__scan(nList[i], newPath, str, monitored) == -1) {
				fprintf(stderr, "scan: error while processing \"%s\"\n", path);
				return PROG_ERROR;
			}
			free(newPath);
		}
	}
	if(!nList){
		free(nList);
		nList = NULL;
	}
	return 0;
}

// ===========================================================================
// get_notifications
// ===========================================================================
int get_notifications(mappingStructure *str, int sid, receivedNotification ***list, int *count){
	 if(nb_read_notifications(pmm_offset_to_pointer(str->off_notifications), list, count, sid) == -1){
		 fprintf(stderr, "get_notifications: error while reading the notifications list.\n");
		 return PROG_ERROR;
	 }
	 return 0;
}

// ===========================================================================
// print_mappingstructure_state
// ===========================================================================
void print_mappingstructure_state(mappingStructure *str){
	printf("-------------------MAPPING STRUCTURE----------------------------\n\n");
	printf("SYSTREE:\n");
	fst_print_tree(pmm_offset_to_pointer(str->off_fileSystemTree));
	printf("\nNOTIFICATIONS BUCKETS\n");
	nb_print_notification_buckets(pmm_offset_to_pointer(str->off_notifications));
}
