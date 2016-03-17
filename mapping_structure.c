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

	if(memoryBlock != NULL){
		*str = memoryBlock;
		return 0;
	}
	*str = pmm_malloc(sizeof(mappingStructure));
	if(!(*str)){
		fprintf(stderr, "intialize_mapping_structure: error while allocating memory.\n");
		return -1;
	}
	(*str)->refreshTime = refTime;
	(*str)->serverCounter = 0;
	(*str)->daemonServer = 0;

	/*create the filesystem tree */
	fstNode *root;
	if(fst_build_root(&root) == -1) {
		fprintf(stderr, "initialize_mapping_structure: error creating the filesystree root.\n");
		return -1;
	}
	(*str)->off_fileSystemTree = pmm_pointer_to_offset(root);

	//TODO: NEED TO SCANN THE FILESYSTEM

	/*create notifications bucket */
	notificationsBucket *firstElement;
	if(nb_create(&firstElement, 0, serverPath) == -1){
		fprintf(stderr, "initialize_mapping_structure: error creating the first notifications bucket.\n");
		return -1;		;
	}
	(*str)->off_notifications = pmm_pointer_to_offset(firstElement);

	return 0;
}

// ===========================================================================
// generate_server_id
// ===========================================================================
unsigned int generate_server_id(mappingStructure *str){
	str->serverCounter++;
	return str->serverCounter;
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
	//TODO: MUST SCANN THE FILESYSTEM IF NOT ALREADY MONITORED
}

// ===========================================================================
// unregister_server
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
// delete_server_path
// ===========================================================================
int __delete_server_path();

int __first_scan();
