#include "../include/cr_lock.h"
#include <windows.h>
#include "utilities.h"
/*wrapper, used for abstraction*/
struct crLock {
    HANDLE mutex;
};

// ===========================================================================
// create_cr_lock
// ===========================================================================
int create_cr_lock(struct crLock **lock){
   *lock = malloc(sizeof(struct crLock));
   if(!(*lock)){
      fprintf(stderr, "create_cr_lock: error while allocating memory.\n");
      return -1;
   }
   (*lock)->mutex = CreateMutex(NULL, FALSE, NULL);
   if((*lock)->mutex == NULL){
      fprintf(stderr, "create_cr_lock: error while creating the mutex.\n");
      return ERROR;
   }
	return SUCCESS;
}

// ===========================================================================
// acquire_cr_lock
// ===========================================================================
int acquire_cr_lock(struct crLock *lock) {
   DWORD ret = WaitForSingleObject(lock->mutex, INFINITE);
   if(ret == WAIT_FAILED){
      fprintf(stderr, "create_cr_lock: error while acquiring the mutex.\n");
      return ERROR;
   }
	return SUCCESS;
}

// ===========================================================================
// release_cr_lock
// ===========================================================================
int release_cr_lock(struct crLock *lock) {
	BOOL unlocked = ReleaseMutex(lock->mutex);
   if(unlocked == 0){
      printf("%s\n", GetLastErrorAsString());
      fprintf(stderr, "syncmapping_release: error while releasing the mutex.\n");
      return ERROR;
   }
	return SUCCESS;
}

// ===========================================================================
// close_cr_lock
// ===========================================================================
int close_cr_lock(struct crLock *lock){
	if(!CloseHandle(lock->mutex)){
		fprintf(stderr, "Error while closing the lock handle.\n");
		return ERROR;
	}
	free(lock);
	return SUCCESS;
}
