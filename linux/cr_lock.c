#include "../include/cr_lock.h"
#include <pthread.h>

struct crLock{
   pthread_mutex_t mutex;
};

// ===========================================================================
// create_cr_lock
// ===========================================================================
int create_cr_lock(struct crLock **lock){
   *lock = malloc(sizeof(struct crLock));
   if(!(*lock)){
      fprintf(stderr, "create_cr_lock: error while allocating memory.\n");
      return ERROR;
   }
   (*lock)->mutex = PTHREAD_MUTEX_INITIALIZER;
   return SUCCESS;
}

// ===========================================================================
// acquire_cr_lock
// ===========================================================================
int acquire_cr_lock(struct crlock *lock){
   if(pthread_mutex_lock(&(lock->mutex)) != 0){
      fprintf(stderr, "acquire_cr_lock: error while acquiring the cr_lock.\n");
      return ERROR;
   }
   return SUCCESS;
}

// ===========================================================================
// release_cr_lock
// ===========================================================================
int release_cr_lock(struct crLock *lock){
   if(pthread_mutex_unlock(&(lock->mutex)) != 0){
      fprintf(stderr, "release_cr_lock: error while releasing the cr_lock.\n");
      return ERROR;
   }
   return SUCCESS;
}

}

// ===========================================================================
// close_cr_lock
// ===========================================================================
int close_cr_lock(struct crLock **lock){
   if(pthread_mutex_destroy(&(lock->mutex)) != 0){
      fprintf(stderr, "close_cr_lock: error while destroying the lock.\n");
      return ERROR;
   }
   return SUCCESS;
}
