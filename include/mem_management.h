#ifndef __PERSONAL_MEMORY_MANAGEMENT__
	#define __PERSONAL_MEMORY_MANAGEMENT__
	#include <stdio.h>
	#include <stdlib.h>

	/*When allocating memory, we use extra bytes to store the size, the availability
	and the previous block offset at the beginning of the block. In this way we will
	know what are the free blocks and how many bytes to free when pmm_free() is called*/
	typedef struct _metadata{
	   unsigned long size;
	   int isFree;
	   unsigned long previous;
	} metadata;

/*function prototypes*/

// ===========================================================================
//
// pmm_initialize_management
// Descripton: when called with NULL as returnPointer, this function initializes the
// memory management by creating the first free block at the very beginning of the
// memory area. This represents the whole free memory available. If a not NULL
// returnPointer is passed, then it is set to the the address of the first byte of
// the first block of memory. If an error has occurred, -1 is returned, 0 otherwise.
//
// ===========================================================================
int pmm_initialize_management(char *basepointer, unsigned long byteNumber, void **returnPointer);

// ===========================================================================
//
// pmm_pointer_to_offset
// Description: converts the pointer to an offset (in byte size) from the starting
// address of the memory area
// Params:
//	- void *pointer: pointer to convert
//
// ===========================================================================
unsigned long pmm_pointer_to_offset(void *pointer);

// ===========================================================================
//
// pmm_offset_to_pointer
// Description: convert an offset to a pointer at a location
//
// ===========================================================================
void *pmm_offset_to_pointer(unsigned long offset);

// ===========================================================================
//
// pmm_malloc
// Description: This function reserves a block or req_size bytes, if available
// Params:
//	- req_size: number of bytes that has to be reserved.
// Returns the pointer to the allocated memory or NULL in case there isn't memory
//
// ===========================================================================
void *pmm_malloc(unsigned long req_size);

// ===========================================================================
//
// pmm_free
// Description: This function frees memory that was allocated in precedence.
// Params:
//	- void *pointer: pointer to the block of memory that has to be freed.
//
// ===========================================================================
int pmm_free(void *pointer);

// ===========================================================================
//
// pmm_print_memory_state
// Description: print a representation of the memory blocks. Used for debugging
//
// ===========================================================================
void pmm_print_memory_state();

#endif
