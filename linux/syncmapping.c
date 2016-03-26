#include "../include/syncmapping.h"
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#include <pthread.h>

//structure that represents a lock file
struct syncMapping{
   sem_t *sem;
   char *semName;
};

// ===========================================================================
// syncmapping_createlock
// ===========================================================================
int syncmapping_createlock(struct syncMapping **lock, char *semName){
   //allocate a new syncMapping object
   *lock = (struct syncMapping *) malloc(sizeof(struct syncMapping));
   if(!(*lock)){
      fprintf(stderr, "Error while allocating memory.\n");
      close(fd);
      return -1;
   }
   char *tmp = malloc(sizeof(char) * (strlen(semName) + 2));
   if(!tmp){
      fprintf(stderr, "syncmapping_createlock: error while allocating memory.\n");
      return -1;
   }
   if(sprintf(tmp, "/%s", semName) < 0){
      fprintf(stderr, "%s\n", "syncmapping_createlock: sprintf error.\n");
      return -1;
   }
   (*lock)->sem = sem_open(tmp, O_CREAT, 0666, 1);
   if((*lock)->sem == SEM_FAILED){
      fprintf(stderr, "syncmapping_createlock: error while creating the semaphore.\n");
      return -1;
   }
   (*lock)->semName = tmp;
   return 0;
}

// ===========================================================================
// syncmapping_acquire
// ===========================================================================
int syncmapping_acquire(struct syncMapping *lock){
      int ret = sem_wait(lock->sem);
      if(ret == -1){
        fprintf(stderr, "Error while acquiring the syncmapping lock.\n");
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
    fprintf(stderr, "Error while releasing the syncmapping lock.\n");
    return -1;
  }
  return 0;
}

// ===========================================================================
// syncmapping_closelock
// ===========================================================================
int syncmapping_closelock(struct syncMapping *lock){
   if(sem_close(lock->sem) == -1){
      fprintf(stderr, "Error while closing the semaphor.\n");
      return -1;
   }
   free(lock->semName);
   free(lock);
   return 0;
}

// ===========================================================================
// syncmapping_deletelock
// ===========================================================================
int syncmapping_deletelock(struct syncMapping *lock){
   if(sem_close(lock->sem) == -1){
      fprintf(stderr, "Error while closing the semaphor.\n");
      return -1;
   }
   if(sem_unlink(lock->name) == -1){
      fprintf(stderr, "Error while deleting the semaphor.\n");
      return -1;
   }
   free(lock->semName);
   free(lock);
   return 0;
}
