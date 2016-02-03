#include "../include/syncmapping.h"
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>

//structure that represents a lock file
struct syncMapping{
   int fileDescriptor;
   char *fileName;
};

// ===========================================================================
// syncmapping_createlock
// ===========================================================================
int syncmapping_createlock(struct syncMapping **lock, char *filename){
   int fd = open(filename, O_CREAT | O_EXCL, 0666);
   if(fd == -1){
      /*the lock is already there, we try to take it*/
      fd = open(filename, O_RDONLY);
      if(fd == -1){
        fprintf(stderr, "An error occurred while opening the syncmapping lock.\n");
        return -1;
      }
   }
   //allocate a new syncMapping object
   *lock = (struct syncMapping *) malloc(sizeof(struct syncMapping));
   if(!(*lock)){
      fprintf(stderr, "Error while allocating memory.\n");
      close(fd);
      return -1;
   }
   (*lock)->fileName = filename;
   (*lock)->fileDescriptor = fd;
   return 0;
}

// ===========================================================================
// syncmapping_acquire
// ===========================================================================
int syncmapping_acquire(struct syncMapping *lock){
      int err = flock(lock->fileDescriptor, LOCK_EX);
      if(err == -1){
        fprintf(stderr, "Error while acquiring the syncmapping lock.\n");
        return -1;
      }
      return 0;
}

// ===========================================================================
// syncmapping_release
// ===========================================================================
int syncmapping_release(struct syncMapping *lock){
  int err = flock(lock->fileDescriptor, LOCK_UN);
  if(err == -1){
    fprintf(stderr, "Error while releasing the syncmapping lock.\n");
    return -1;
  }
  return 0;
}

// ===========================================================================
// syncmapping_deletelock
// ===========================================================================
int syncmapping_deletelock(struct syncMapping *lock){
   if(close(lock->fileDescriptor) == -1){
      fprintf(stderr, "Error while closing the file descriptor of the lock file.\n");
      return -1;
   }
   free(lock->fileName);
   free(lock);
   return 0;
}
