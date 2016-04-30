#include <pthread.h>
#include <time.h>
#include "../include/thread.h"

/*thread structure*/
struct thread{
   pthread_t handle;
};

// ===========================================================================
// create_thread
// ===========================================================================
int create_thread(void *(*pFunction)(void*), void *arg, struct thread **thr){
   *thr = malloc(sizeof(struct thread));
   if(!(*thr)){
      fprintf(stderr, "create_thread: error while allocating memory.\n");
      return -1;
   }
   int ret = pthread_create(&((*thr)->handle), NULL, pFunction, arg);
   if(ret != 0){
      fprintf(stderr, "create_thread: error while creating the thread (code %d)\n", ret);
      return -1;
   }
   return 0;
}

// ===========================================================================
// thread_sleep
// ===========================================================================
int thread_sleep(int seconds){
   struct timespec spec = {seconds, 0};
   return nanosleep(&spec, NULL);
}

// ===========================================================================
// terminate_thread
// ===========================================================================
int terminate_thread(struct thread *t){
   if(pthread_cancel(t->handle) == 0) return 0;
   else return -1;
}
