#include "../include/thread_lock.h"
#include <pthread.h>

struct threadLock{
   pthread_mutex_t mutex;
};

// ===========================================================================
// create_threadlock
// ===========================================================================
int create_threadlock(struct threadLock **lock){
   *lock = malloc(sizeof(struct threadLock));
   if(!(*lock)){
      fprintf(stderr, "create_threadlock: error while allocating memory.\n");
      return PROG_ERROR;
   }
   pthread_mutex_init(&((*lock)->mutex), NULL);
   return PROG_SUCCESS;
}

// ===========================================================================
// acquire_threadlock
// ===========================================================================
int acquire_threadlock(struct threadLock *lock){
   if(pthread_mutex_lock(&(lock->mutex)) != 0){
      fprintf(stderr, "acquire_threadlock: error while acquiring the threadlock.\n");
      return PROG_ERROR;
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// release_threadlock
// ===========================================================================
int release_threadlock(struct threadLock *lock){
   if(pthread_mutex_unlock(&(lock->mutex)) != 0){
      fprintf(stderr, "release_threadlock: error while releasing the threadlock.\n");
      return PROG_ERROR;
   }
   return PROG_SUCCESS;
}

// ===========================================================================
// close_threadlock
// ===========================================================================
int close_threadlock(struct threadLock *lock){
   if(pthread_mutex_destroy(&(lock->mutex)) != 0){
      fprintf(stderr, "close_threadlock: error while destroying the lock.\n");
      return PROG_ERROR;
   }
   return PROG_SUCCESS;
}
