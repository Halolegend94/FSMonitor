#include "../include/syncmapping.h"
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>
#include <sys/stat.h>

//structure that represents a lock file
struct syncMapping{
   sem_t *sem;
};

// ===========================================================================
// syncmapping_createlock
// ===========================================================================
int syncmapping_createlock(struct syncMapping **lock){
   //allocate a new syncMapping object
   *lock = (struct syncMapping *) malloc(sizeof(struct syncMapping));
   if(!(*lock)){
      fprintf(stderr, "Error while allocating memory.\n");
      return -1;
   }
   (*lock)->sem = sem_open("/syncmap", O_CREAT, 0777, 1);
   if((*lock)->sem == SEM_FAILED){
      fprintf(stderr, "syncmapping_createlock: error while creating the semaphore.\n");
      return -1;
   }
   return 0;
}

// ===========================================================================
// syncmapping_acquire
// ===========================================================================
int syncmapping_acquire(struct syncMapping *lock){
      int ret = sem_wait(lock->sem);
      if(ret == -1){
        fprintf(stderr, "syncmapping_acquire: error while acquiring the syncmapping lock.\n");
        return -1;
      }
      return 0;
}

// ===========================================================================
// syncmapping_release
// ===========================================================================
int syncmapping_release(struct syncMapping *lock){
   int err = sem_post(lock->sem);
   if(err == -1){
     fprintf(stderr, "syncmapping_release: error while releasing the syncmapping lock.\n");
     return -1;
   }
   return 0;
}

// ===========================================================================
// syncmapping_closelock
// ===========================================================================
int syncmapping_closelock(struct syncMapping *lock){
   if(sem_close(lock->sem) == -1){
      fprintf(stderr, "syncmapping_closelock: error while closing the semaphore.\n");
      return -1;
   }
   free(lock);
   return 0;
}

// ===========================================================================
// syncmapping_deletelock
// ===========================================================================
int syncmapping_deletelock(struct syncMapping *lock){
   if(sem_close(lock->sem) == -1){
      fprintf(stderr, "syncmapping_deletelock: error while closing the semaphore.\n");
      return -1;
   }
   if(sem_unlink("/syncmap") == -1){
      fprintf(stderr, "syncmapping_deletelock: error while deleting the semaphore.\n");
      return -1;
   }
   free(lock);
   return 0;
}
