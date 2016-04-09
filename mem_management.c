#include "include/mem_management.h"

/*global variables*/
char *pmmBasePointer = NULL; //memory start pointer
char *pmmLastValidPointer = NULL; //memory end pointer

// ===========================================================================
// pmm_pointer_to_offset
// ===========================================================================
unsigned long pmm_pointer_to_offset(void *pointer) {
	return (unsigned long)(((char *)pointer) - pmmBasePointer);
}

// ===========================================================================
// pmm_offset_to_pointer
// ===========================================================================
void *pmm_offset_to_pointer(unsigned long offset) {
	return (void *)(pmmBasePointer + offset);
}

// ===========================================================================
// pmm_initialize_management
// ===========================================================================
int pmm_initialize_management(char *basepointer, unsigned long byteNumber, void **returnPointer){
   if(!basepointer){
      fprintf(stderr, "Error in pmm_initialize_management: base pointer not valid.\n");
      return -1;
   }
   pmmBasePointer =  basepointer;
   pmmLastValidPointer = pmmBasePointer + byteNumber - 1;
   if(returnPointer){
      //user requires only the first memory block. We suppose memory is already initialized
      *returnPointer = pmmBasePointer + sizeof(metadata);
      return 0;
   }

	/*check if the byteNumber is suffucient*/
	if (byteNumber <= sizeof(metadata)) {
		fprintf(stderr, "PersonalMemoryManagement: Error! byteNumber insufficient.\n");
		return -1;
	}
   /*mark the firt free block of memory*/
   metadata *startMeta = (metadata *) basepointer;
   startMeta->isFree = 1;
   startMeta->previous = 0;
   startMeta->size = byteNumber - sizeof(metadata);
   return 0;
}

// ===========================================================================
// pmm_malloc
// ===========================================================================
void *pmm_malloc(unsigned long req_size){
	if (req_size <= 0) {
		fprintf(stderr, "The number of requested bytes is invalid.\n");
		return NULL;
	}
   metadata *pBlock = (metadata *) pmmBasePointer;
   /*search a free block with enough memory*/
   while(!pBlock->isFree || pBlock->size < req_size){
      pBlock = (metadata *) (((char *)pBlock) + pBlock->size + sizeof(metadata)); //jump to the next block
      if((char *)pBlock > pmmLastValidPointer) return NULL; //reached the end of the memory region
   }
   //we have found a suitable block of memory. Do we need to split it?
   pBlock->isFree = 0;
   if(pBlock->size > req_size + sizeof(metadata)){ //yes, there is space for another free block
      unsigned long newSize = pBlock->size - req_size - sizeof(metadata);
      pBlock->size = req_size;
      metadata *newBlock =(metadata *) (((char *) pBlock) + sizeof(metadata) + req_size);
      newBlock->size = newSize;
      newBlock->isFree = 1;
      newBlock->previous = pmm_pointer_to_offset((char *) pBlock);
   }
   return (char *) pBlock + sizeof(metadata);
}

// ===========================================================================
// pmm_free
// ===========================================================================
void pmm_free(void *pointer){
  if(!pointer || ((char *) pointer) > pmmLastValidPointer || ((char *)pointer) < pmmBasePointer){
      fprintf(stderr, "pmm_free called on invalid pointer (%p).\n", pointer);
      exit(0);
   }

   metadata *block = (metadata *) (((char *) pointer) - sizeof(metadata));
   metadata *previous = (metadata *) pmm_offset_to_pointer(block->previous);

   metadata *next = NULL;
	 metadata *nextToNext = NULL;

	 char *tnext = ((char *) pointer) + block->size;
	 if(tnext < pmmLastValidPointer){
		 next = (metadata *) tnext;
		 tnext = tnext + next->size + sizeof(metadata);
		 if(tnext < pmmLastValidPointer){
			 nextToNext = (metadata *) tnext;
		 }
	 }

	 if(next != NULL){
		 if( next->isFree && previous->isFree){
			 previous->size = previous->size  + block->size + next->size + (2 * sizeof(metadata));
       if(nextToNext != NULL) nextToNext->previous = block->previous;

		 }else if(next->isFree){
			 block->size += next->size + sizeof(metadata);
			 block->isFree = 1;
			 if(nextToNext != NULL) nextToNext->previous = next->previous;

		 }else if(previous->isFree){
			 previous->size += block->size + sizeof(metadata);
			 next->previous = block->previous;
		 }else{
			 	block->isFree = 1;
		 }
   }else{
		 if(previous->isFree)
			 previous->size += block->size + sizeof(metadata);
		 else
			 block->isFree = 1;
   }
}

// ===========================================================================
// pmm_print_memory_state
// ===========================================================================
void pmm_print_memory_state(){
   metadata *p = (metadata *) pmmBasePointer;
   unsigned long counter = 0;
   while((char *)p < pmmLastValidPointer){
      unsigned long start =  pmm_pointer_to_offset(p);
      unsigned long end = start + p->size + sizeof(metadata) - 1;
      printf("\n");
   	printf("---------- %lu: start %lu offset. previous: %lu, size: %lu, Libero: %d\n", start, counter,p->previous, p->size, p->isFree);
   	printf("|          |\n");
   	printf("|          |\n");
   	printf("-----------%lu: end %lu offset\n", end, counter);
   	printf("|          |\n");
      counter++;
      p = (metadata *)(((char *) p) + p->size + sizeof(metadata));
   }
}
