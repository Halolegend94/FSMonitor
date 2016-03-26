#include <pthread.h>
#include <time.h>
/*thread structure*/
struct thread{
   pthread_t handler;
}

// ===========================================================================
// create_thread
// ===========================================================================
int create_thread(void *(*pFunction)(void*), void *arg, thread **thr){
   *thr = malloc(sizeof(thread));
   if(!(*thr)){
      fprintf(stderr, "create_thread: error while allocating memory.\n");
      return -1;
   }
   int ret = pthread_create(&((*thr)->handler), NULL, pFunction, arg);
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
	int ret = nanosleep(&spec, NULL);
   if(ret == -1){
      fprintf(stderr, "thread_sleep: sleep interrupted.\n");
   }
	return ret;
}
