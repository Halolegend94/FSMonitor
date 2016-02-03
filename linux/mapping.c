#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include "../include/mapping.h"

/*the following structure represents a file mapping*/
struct mapping {
	int file;
	char *mapname;
	char *pointer;
	long long size;
};

// ===========================================================================
// __support_mapping [PRIVATE FUNCTION]
// ===========================================================================
int __support_mapping(struct mapping *map, long long size) {
	map->pointer = mmap(NULL, (size_t) size, PROT_READ | PROT_WRITE, MAP_SHARED, 
	map->file, 0);
	if (map->pointer == MAP_FAILED) {
		fprintf(stderr, "mmap failed.\n");
		free(map);
		return -1;
	}else {
		map->size = size;
		return 0;
	}
}

// ===========================================================================
// create_mapping
// ===========================================================================
int create_mapping(struct mapping **map, char *name, long long size) {
	*map = (struct mapping *) malloc(sizeof(struct mapping));
	if(!(*map)){
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}
	//create the file
	(*map)->mapname = name;
	(*map)->file = open(name, O_CREAT | O_RDWR | O_EXCL, 0666);
	if ((*map)->file == -1) {
		fprintf(stderr, "An error occurred while create/opening the file associated \
		to the mapping.\n");
		free(*map);
		return -1;
	}
	if(ftruncate((*map)->file, size) == -1){
			fprintf(stderr, "An error occurred while truncating the file.\n");
			close((*map)->file);
			free(*map);
			return -1;
	}
	return __support_mapping(*map, size);
}

// ===========================================================================
//open_mapping
// ===========================================================================
int open_mapping(struct mapping **map, char *name, long long size){
	*map = (struct mapping *) malloc(sizeof(struct mapping));
	if(!(*map)){
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}
	(*map)->mapname = name;
	(*map)->file = open(name, O_RDWR);
	if ((*map)->file == -1) {
		fprintf(stderr, "An error occurred while opening the file associated to the \
		mapping.\n");
		free(*map);
		return -1;
	}
	return __support_mapping(*map, size);
}

// ===========================================================================
// delete_mapping
// ===========================================================================
int delete_mapping(struct mapping *map) {
	if(close(map->file) == -1){
		fprintf(stderr, "An error occurred while closing the file descriptor \
		of the mapped file.\n");
		return -1;
	}
	if(munmap(map->pointer, map->size) == -1){
		fprintf(stderr, "An error occurred while unmapping the file.\n");
		return -1;
	}
	return 0;
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
