#include "../include/syncmapping.h"
#include <windows.h>
#include "utilities.h"
/*this structure is used as wrapper for the mutex handle.*/
struct syncMapping {
    HANDLE mutex;
};

// ===========================================================================
// syncmapping_createlock
// ===========================================================================
int syncmapping_createlock(struct syncMapping **lock){
   *lock = malloc(sizeof(struct syncMapping));
   if(!(*lock)){
      fprintf(stderr, "syncmapping_createlock: error while allocating memory.\n");
      return -1;
   }
   (*lock)->mutex = CreateMutex(NULL, FALSE, "syncmap");
   if((*lock)->mutex == NULL){
      fprintf(stderr, "syncmapping_createlock: error while creating the mutex.\n");
      return -1;
   }
	return 0;
}

// ===========================================================================
// syncmapping_acquire
// ===========================================================================
int syncmapping_acquire(struct syncMapping *lock) {
   DWORD ret = WaitForSingleObject(lock->mutex, INFINITE);
   if(ret == WAIT_FAILED){
      fprintf(stderr, "syncmapping_acquire: error while acquiring the mutex.\n");
      return -1;
   }
	return 0;
}

// ===========================================================================
// syncmapping_release
// ===========================================================================
int syncmapping_release(struct syncMapping *lock) {
	BOOL unlocked = ReleaseMutex(lock->mutex);
   if(unlocked == 0){
      printf("%s\n", GetLastErrorAsString());
      fprintf(stderr, "syncmapping_release: error while releasing the mutex.\n");
      return -1;
   }
	return 0;
}

// ===========================================================================
// syncmapping_closelock
// ===========================================================================
int syncmapping_closelock(struct syncMapping *lock){
	if(!CloseHandle(lock->mutex)){
		fprintf(stderr, "Error while closing the mutex handle.\n");
		return -1;
	}
	free(lock);
	return 0;
}

// ===========================================================================
// syncmapping_deletelock
// ===========================================================================
int syncmapping_deletelock(struct syncMapping *lock){
	return syncmapping_closelock(lock);
}
