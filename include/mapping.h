#ifndef __MAPPING__
	#define __MAPPING__
	#include <stdlib.h>
	#include <stdio.h>
	/*the following structure represents a file mapping.
	It's an opaque structure. Must be handled by functions specified in this header.*/
	typedef struct mapping *pMapping;

	//function prototypes

	// ===========================================================================
	//
	// create_mapping
	// Description: this function must be called by the first process in order
	// to create a file and a mapping in the memory associated to that file.
	// Params:
	// - *map: pointer to a mapping structure
	//	- *name: name of the file (also, on windows, of the mapping)
	//	- size: size of the mapping
	// Returns -1 in case of error, 0 if the mapping has been created, 1 if the mapping was already there
	//
	// ===========================================================================
	int create_mapping(pMapping *map, char *name, long long size);

	// ===========================================================================
	//
	// open_mapping
	// Description: this function is called by a process that wants to map
	// in memory an existing mapping.
	// Params:
	// - *map: pointer to a mapping structure
	//	- *name: name of the file (also, on windows, of the mapping)
	//	- size: size of the mapping
	// Returns -1 in case of error, 0 otherwise
	//
	// ===========================================================================
	int open_mapping(pMapping *map, char *name, long long size);

	// ===========================================================================
	//
	// delete_mapping
	// Description: unmap the mapping represented by map. This function
	// does not delete the associated file.
	// Params:
	// - *map: pointer to an existing mapping structure.
	// Returns -1 in case of error, 0 otherwise
	//
	// ===========================================================================
	int delete_mapping(pMapping map);

	// ===========================================================================
	//
	// get_mapping_pointer
	// Description: get the pointer to the first byte of the mapping
	// Params:
	//  - map: a pMapping object representing the mapping.
	// Returns NULL in case of error, a valid pointer otherwise
	//
	// ===========================================================================
	char *get_mapping_pointer(pMapping map);

	// ===========================================================================
	//
	// get_mapping_name
	// Description: get the name of the mapping
	// Params:
	//  - map: a pMapping object representing the mapping.
	// Returns a NULL pointer in case of error, a valid pointer to the mapping
	// name otherwise
	//
	// ===========================================================================
	char *get_mapping_name(pMapping map);

#endif
