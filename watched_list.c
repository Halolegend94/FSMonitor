#include "include/watched_list.h"

// ===========================================================================
// wl_create
// ===========================================================================
int wl_create(watchedList **list){
	*list = (watchedList *) pmm_malloc(sizeof(watchedList));
	if(!(*list)){
		fprintf(stderr, "wl_create: error while allocating memory.\n");
		return -1;
	}
	(*list)->count = 0;
	(*list)->list = NULL;
	return 0;
}

// ===========================================================================
// wl_add_path
// ===========================================================================
int wl_add_path(char *path, watchedList *wList){
	if(!path || !wList){
		fprintf(stderr, "wl_add_path: params not valid.\n");
		return -1;
	}
	
		
	if(wList->count == 0){
		unsigned long *pList = (unsigned long *) pmm_malloc(sizeof(unsigned long));
		if(!pList){
			fprintf(stderr, "wl_add_path: error while allocating memory.\n");
			return -1;
		}
		
		/*create a new path object*/
		watchedPath *wPath = (watchedPath *) pmm_malloc(sizeof(watchedPath));
		if(!wPath){
			fprintf(stderr, "wl_add_path: error while allocating memory.\n");
			pmm_free(pList);
			return -1;
		}
		char *fpath = (char *) pmm_malloc(sizeof(char) * (strlen(path) + 1));
		if(!fpath){
			fprintf(stderr, "wl_add_path: error while allocating memory.\n");
			pmm_free(wPath);
			pmm_free(pList);
			return -1;
		}
		strcpy(fpath, path);
		wPath->off_fullpath = pmm_pointer_to_offset(fpath);
		wPath->serverCount = 1;
		
		/*add the path object in the path list*/
		pList[0] = pmm_pointer_to_offset(wPath);
		wList->off_list = pmm_pointer_to_offset(pList);
		wList->count = 1;
	}else{
		unsigned long *pList = pmm_offset_to_pointer(wList->off_list);
		int i; //counter
		for(i = 0; i < wList->count; i++){
			watchedPath *p = pmm_offset_to_pointer(pList[i]);
			char *name = pmm_offset_to_pointer(p->off_fullpath);
			if(strcmp(name, path) == 0){
				//path already monitored
				p->serverCount++;
				return 0;
			}
		}
		
		
		/*if we are here, no path that matches has been found. create a new path object*/
		watchedPath *wPath = (watchedPath *) pmm_malloc(sizeof(watchedPath));
		if(!wPath){
			fprintf(stderr, "wl_add_path: error while allocating memory.\n");
			pmm_free(newPList);
			return -1;
		}
		char *fpath = (char *) pmm_malloc(sizeof(char) * (strlen(path) + 1));
		if(!fpath){
			fprintf(stderr, "wl_add_path: error while allocating memory.\n");
			pmm_free(wPath);
			pmm_free(newPList);
			return -1;
		}
		strcpy(fpath, path);
		wPath->off_fullpath = pmm_pointer_to_offset(fpath);
		wPath->serverCount = 1;
		
		//new offset list
		unsigned long *newPList = (unsigned long *) pmm_malloc(sizeof(unsigned long) 
						* (wList->count + 1));
						
		if(!newPList){
			fprintf(stderr, "wl_add_path: error while allocating memory.\n");
			return -1;
		}
		
		//copy offset to watched paths already present
		for(i = 0; i < wList->count; i++){
			newPList[i] = pList[i];
		}
		
		/*add the path object in the path list*/
		newPList[i] = pmm_pointer_to_offset(wPath);
		wList->off_list = pmm_pointer_to_offset(newPList);
		wList->count += 1;
	}
	return 0;
}

// ===========================================================================
// wl_remove_path
// ===========================================================================
int wl_remove_path(char *path, watchedList *wList, int *entryRemoved){
	if(!path || !wList){
		fprintf(stderr, "wl_add_path: params not valid.\n");
		return -1;
	}

	int i; //counter
	unsigned long *pList = pmm_offset_to_pointer(wList->off_list);
	for(i = 0; i < wList->count; i++){
		watchedPath *p = pmm_offset_to_pointer(pList[i]);
		char *name = pmm_offset_to_pointer(p->off_fullpath);
		if(strcmp(name, path) == 0){
			//path found
			p->serverCount--;
			if(p->serverCount == 0){
				//We can delete the watched path entry
				unsigned long *pNewList = (unsigned long *) pmm_malloc(
					sizeof(unsigned long) * (wList->count - 1));
				if(!pNewList){
					fprintf(stderr, "wl_remove_path: error while allocating memory.\n");
					return -1;
				}
				int j, k;
				for(j = 0, k = 0; j < wList->count; j++){
					if(j != i){
						pNewList[k++] = pList[j];
					}
				}
				//we can free the resources
				pmm_free(name);
				pmm_free(p);
				free(pList);
				wList->count = wList->count - 1;
				wList->off_list = pmm_pointer_to_offset(pNewList);
				*entryRemoved = 1;
			}
			return 0;
			
		}
	}
	//if we are here, no path has been found with that name
	fprintf(stderr, "wl_remove_path: no path found with that name.\n");
	return -1;
}

// ===========================================================================
// wl_print_watched_list
// ===========================================================================
void wl_print_watched_list(watchedList *wList){
	unsigned long *offs = pmm_offset_to_pointer(wList->off_list);
	int i;
	printf("Num entries: %u\n", wList->count);
	printf("|------------------------------|\n");
	for(i = 0; i < wList->count; i++){
		watchedPath p = pmm_offset_to_pointer(offs[i]);
		printf("PATH: %s, Servers: %u\n", (char *) pmm_offset_to_pointer(
				wList->off_fullpath), wList->serverCount);
	}
	printf("|------------------------------|\n");
	
}

