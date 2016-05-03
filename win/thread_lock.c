#include "../include/thread_lock.h"
#include <windows.h>
#include "utilities.h"
/*wrapper, used for abstraction*/
struct threadLock {
    HANDLE mutex;
};

// ===========================================================================
// create_threadlock
// ===========================================================================
int create_threadlock(struct threadLock **lock){
   *lock = malloc(sizeof(struct threadLock));
   if(!(*lock)){
      fprintf(stderr, "create_threadlock: error while allocating memory.\n");
      return -1;
   }
   (*lock)->mutex = CreateMutex(NULL, FALSE, NULL);
   if((*lock)->mutex == NULL){
      fprintf(stderr, "create_threadlock: error while creating the mutex.\n");
      return PROG_ERROR;
   }
	return PROG_SUCCESS;
}

// ===========================================================================
// acquire_threadlock
// ===========================================================================
int acquire_threadlock(struct threadLock *lock) {
   DWORD ret = WaitForSingleObject(lock->mutex, INFINITE);
   if(ret == WAIT_FAILED){
      fprintf(stderr, "create_threadlock: error while acquiring the mutex.\n");
      return PROG_ERROR;
   }
	return PROG_SUCCESS;
}

// ===========================================================================
// release_threadlock
// ===========================================================================
int release_threadlock(struct threadLock *lock) {
	BOOL unlocked = ReleaseMutex(lock->mutex);
   if(unlocked == 0){
      printf("%s\n", GetLastErrorAsString());
      fprintf(stderr, "syncmapping_release: error while releasing the mutex.\n");
      return PROG_ERROR;
   }
	return PROG_SUCCESS;
}

// ===========================================================================
// close_threadlock
// ===========================================================================
int close_threadlock(struct threadLock *lock){
	if(!CloseHandle(lock->mutex)){
		fprintf(stderr, "Error while closing the lock handle.\n");
		return PROG_ERROR;
	}
	free(lock);
	return PROG_SUCCESS;
}
