#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

//this is the windows implementation of the mapping structure
struct mapping {
	HANDLE fileHandle;
	HANDLE mapHandle;
	char *mapname;
	char *pointer;
	LARGE_INTEGER size;
};

// ===========================================================================
// create_mapping
// ===========================================================================
int create_mapping(struct mapping **map, char *name, long long size) {
	*map = (struct mapping *) malloc(sizeof(struct mapping));
	if(!(*map)){
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}
	(*map)->fileHandle = CreateFile(name, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if ((*map)->fileHandle == INVALID_HANDLE_VALUE) {
		free(*map);
		return 1;
	}
	((*map)->size).QuadPart = size;
	(*map)->mapHandle = CreateFileMapping((*map)->fileHandle, NULL, PAGE_READWRITE,
		(*map)->size.HighPart, (*map)->size.LowPart, name);
	if ((*map)->mapHandle == NULL) {
		fprintf(stderr, "Error while creating the mapping.\n");
		free(*map);
		return -1;
	}
	(*map)->mapname = name;
	(*map)->pointer = MapViewOfFile((*map)->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if ((*map)->pointer == NULL) {
		fprintf(stderr, "Error while mapping the view of the file.\n");
		free(*map);
		return -1;
	}
	return 0;
}

// ===========================================================================
// open_mapping
// ===========================================================================
int open_mapping(struct mapping **map, char *name, long long size) {
	*map = (struct mapping *) malloc(sizeof(struct mapping));
	if(!(*map)){
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}
	(*map)->mapHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
	if ((*map)->mapHandle == NULL) {
		fprintf(stderr, "Error while opening the existing mapping.\n");
		free(*map);
		return -1;
	}
	(*map)->fileHandle = NULL;
	(*map)->mapname = name;
	((*map)->size).QuadPart = size;
	(*map)->pointer = MapViewOfFile((*map)->mapHandle, FILE_MAP_ALL_ACCESS, 0, 0,(SIZE_T) size);
	if ((*map)->pointer == NULL) {
		fprintf(stderr, "Error while mapping the view of the file.\n");
		free(*map);
		return -1;
	}
	return 0;
}

// ===========================================================================
// delete_mapping
// ===========================================================================
int delete_mapping(struct mapping *map) {
	if (map->fileHandle != NULL && !CloseHandle(map->fileHandle)) {
		fprintf(stderr, "Error while closing the mapped file handle.\n");
		return -1;
	}else if (!CloseHandle(map->mapHandle)) {
		fprintf(stderr, "Error while closing the mapping handle.\n");
		return -1;
	}else if (!UnmapViewOfFile(map->pointer)) {
		fprintf(stderr, "Error while unmapping the view of the file\n");
		return -1;
	}
	else {
		return 0;
	}
}

// ===========================================================================
// get_mapping_pointer
// ===========================================================================
char *get_mapping_pointer(struct mapping *map){
	if(!map){
		fprintf(stderr, "Error: the mapping is not valid.\n");
		return NULL;
	}
	return map->pointer;
}

// ===========================================================================
// get_mapping_pointer
// ===========================================================================
char *get_mapping_name(struct mapping *map){
	if(!map){
		fprintf(stderr, "Error: the mapping is not valid.\n");
		return NULL;
	}
	return map->mapname;
}
