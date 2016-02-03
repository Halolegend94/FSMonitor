//DEBUG PURPOSES
#include <stdio.h>
#include <stdlib.h>
#include "include/mem_management.h"

/*global variables*/
unsigned int pmmArrayLength = 0; //this value must always be minor of MANAGEMENT_ZONE_SIZE / sizeof(FreeMemoryArea)
FreeMemoryArea *pmmFreeLocationsList = NULL; //array of free memory zones
char *pmmBasePointer = NULL;


int pmm_initialize_management(void *basepointer, unsigned long byteNumber, void **returnPointer) {
	if (returnPointer != NULL) {
		*returnPointer = ((char *) basepointer) + MANAGEMENT_ZONE_SIZE;
		return 0;
	}
	/*check if the byteNumber is suffucient*/
	if (byteNumber <= MANAGEMENT_ZONE_SIZE) {
		fprintf(stderr, "PersonalMemoryManagement: Error! byteNumber insufficient.\n");
		return -1;
	}
	pmmBasePointer = (char *) basepointer;

	/*initialize the array with 1 item that represent the whole memory block (aside the memory management part)*/
	pmmFreeLocationsList = ((FreeMemoryArea *)pmmBasePointer);
	pmmFreeLocationsList[pmmArrayLength].baseOffset = MANAGEMENT_ZONE_SIZE;
	pmmFreeLocationsList[pmmArrayLength++].byteCount = byteNumber - MANAGEMENT_ZONE_SIZE;
	return 0;
}

// ---------------------------------------------------------------------------------------
//
// pmm_pointer_to_offset and pmm_offset_to_pointer
// Description: converts the pointer to an offset (in byte size) from the starting address of the memory area and back.
// ---------------------------------------------------------------------------------------
unsigned long pmm_pointer_to_offset(void *pointer) {
	return ((char *)pointer) - pmmBasePointer;
}

void *pmm_offset_to_pointer(unsigned long offset) {
	return (void *)(pmmBasePointer + offset);
}


// ---------------------------------------------------------------------------------------
//
// pmm_malloc
// Description: This function reserves a block or req_size bytes if available, and return a pointer to the first
// byte of the block.
// Params:
//	- long req_size: number of bytes that has to be reserved.
//
// ---------------------------------------------------------------------------------------
void *pmm_malloc(unsigned long req_size){
	if (req_size <= 0) return NULL;

	/*we use extra bytes to store the size located at the beginning of the block. In this way
	we will know how many bytes to free when pmm_free() il called*/
	long realSize = req_size + sizeof(unsigned long);
	int i; //counter
	for (i = 0; i < pmmArrayLength; i++) {
		if (pmmFreeLocationsList[i].byteCount >= realSize) {
			//we have found a suitable block of free memory
			unsigned long _offset = pmmFreeLocationsList[i].baseOffset;
			unsigned long  _count = pmmFreeLocationsList[i].byteCount;

			long *pSizeBlock = (unsigned long *)(pmmBasePointer + _offset);
			*pSizeBlock = req_size;
			char *pointer = pmmBasePointer + _offset + sizeof(unsigned long);

			//resize memory block to display changes
			pmmFreeLocationsList[i].byteCount = _count - realSize;
			pmmFreeLocationsList[i].baseOffset = _offset + realSize;
			if (_count - realSize == 0 && __pmm_shift_left(i) > 0) {
				fprintf(stderr, "PersonalMemoryManagement: error while allocating memory (shift_left function)\n");
				return NULL;
			}
			return (void *) pointer;
		}
	}
	/*No memory available!*/
	return NULL;
}

// ---------------------------------------------------------------------------------------
//
// __pmm_shift_right (auxiliary function)
// Description: this function shift the content of the pmmFreeLocationsList by one element to
// the right in order to have space for a new entry at position i
// Params:
//	 - i: new position to be created
//
// ---------------------------------------------------------------------------------------

int __pmm_shift_right(int i) {
	if (pmmArrayLength + 1 > NUM_FMA_ENTRIES) return -1;
	int j; //counter
	for (j = pmmArrayLength - 1; j >= i; j--) {
		pmmFreeLocationsList[j + 1].baseOffset = pmmFreeLocationsList[j].baseOffset;
		pmmFreeLocationsList[j + 1].byteCount = pmmFreeLocationsList[j].byteCount;
	}
	pmmArrayLength++;
	return 0;
}

// ---------------------------------------------------------------------------------------
//
// __pmm_shift_left (auxiliary function)
// Description: this function shift the content of the pmmFreeLocationsList by one element to
// the left, starting from position i + 1, in order to delete the entry at position i
// Params:
//	 - i: new position to be created
//
// ---------------------------------------------------------------------------------------
int __pmm_shift_left(int i) {
	if (pmmArrayLength - 1 < i) return -1;
	int j; //counter
	for (j = i; j < (pmmArrayLength - 1); j--) {
		pmmFreeLocationsList[j].baseOffset = pmmFreeLocationsList[j + 1].baseOffset;
		pmmFreeLocationsList[j].byteCount = pmmFreeLocationsList[j + 1].byteCount;
	}
	pmmArrayLength--;
	return 0;
}

// ---------------------------------------------------------------------------------------
//
// pmm_free
// Description: This function frees memory that was allocated in precedence.
// Params:
//	- void *pointer: pointer to the block of memory that has to be freed.
//
// ---------------------------------------------------------------------------------------
int pmm_free(void *pointer) {
	//find the size of the corresponding block
	unsigned long size = *(((unsigned long *)pointer) - 1) + sizeof(unsigned long);
	unsigned long startOffset = pmm_pointer_to_offset(pointer) - sizeof(unsigned long);
	//minimal error checking
	if (startOffset < 0) return -1;

	/*now we would like to find, if exists, the free block that immediately follows this one
	in order to merge the two. If the memory that follows this newly free block is used, we
	create a new entry in the list.*/

	int i; //contatore
	int endOffset = startOffset + size;

	for (i = 0; i < pmmArrayLength; i++) {
		if (pmmFreeLocationsList[i].baseOffset == endOffset) {
			/*if the freed memory block is attached to the left side of the current block, merge the two*/
			pmmFreeLocationsList[i].baseOffset = startOffset;
			pmmFreeLocationsList[i].byteCount = pmmFreeLocationsList[i].byteCount + size;
			return 0;
		}else if(pmmFreeLocationsList[i].baseOffset + pmmFreeLocationsList[i].byteCount == startOffset){
			/*the newly freed block is attached to the right side of the current block..*/
			if ((i + 1) < pmmArrayLength && pmmFreeLocationsList[i + 1].baseOffset == endOffset) {
				/*.. but also to the left of the next block, so merge free block i and i + 1*/
				pmmFreeLocationsList[i].byteCount += pmmFreeLocationsList[i + 1].byteCount + size;
				if (__pmm_shift_left(i + 1)) {
					fprintf(stderr, "PersonalMemoryManagement: error while shifting left.\n");
					exit(0);
				}
				return 0;
			}else {
				/*merge at the right side of the current block only*/
				pmmFreeLocationsList[i].byteCount = pmmFreeLocationsList[i].byteCount + size;
				return 0;
			}
		} else if(pmmFreeLocationsList[i].baseOffset > endOffset){
			/*the freed block is in the middle of a allocated memory zone and the next free block of memory has the
			baseOffset which is not the subsequent number of endOffset. we need to create a new block.
			First, shift data by one place to right*/
			if (__pmm_shift_right(i)) {
				fprintf(stderr, "PersonalMemoryManagement: error. Impossible to add a new entry in FMA list.\n");
				exit(0);
			}
			pmmFreeLocationsList[i].baseOffset = startOffset;
			pmmFreeLocationsList[i].byteCount = size;
			return 0;
		}
	}
	/*if we get here it is because this block of memory is beyond the last block of free memory.
	Create a new block of memory at the end of the list*/
	if (pmmArrayLength + 1 > NUM_FMA_ENTRIES) {
		fprintf(stderr, "PersonalMemoryManagement: error. Impossible to add a new entry in FMA list.\n");
		exit(0); //muore il processo
	}
	pmmFreeLocationsList[pmmArrayLength].baseOffset = startOffset;
	pmmFreeLocationsList[pmmArrayLength++].byteCount = size;
	return 0;
}

// ---------------------------------------------------------------------------------------
//
// __pmm_print_memory_state (auxiliary function)
// Description: Show the free blocks of memory
//
// ---------------------------------------------------------------------------------------
void __pmm_print_memory_state() {
	printf("\n");
	printf("---------- 0: start area\n");
	printf("|          |\n");
	printf("|          |\n");
	printf("-----------%d: end management\n", MANAGEMENT_ZONE_SIZE - 1);
	printf("|          |\n");
	int i;
	for (i = 0; i < pmmArrayLength; i++) {
		printf("-----------%d: start free block %d\n", pmmFreeLocationsList[i].baseOffset, i);
		printf("|          |\n");
		printf("-----------%d: end free block %d\n", pmmFreeLocationsList[i].baseOffset +
			pmmFreeLocationsList[i].byteCount - 1, i);
		printf("|          |\n");
		printf("|          |\n");
	}
}
