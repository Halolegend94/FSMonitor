#include "../include/cr_lock.h"
#include "../include/thread.h"
void *func(void *);
  //create a lock
   pCRLock mylock;

int main(int argc, char **argv){
 
   if(create_cr_lock(&mylock) == PROG_ERROR){
       printf("Error.\n");
       return -1;
   }
   pToThread p;
   create_thread(func, NULL, &p);
   printf("acquiring 1.\n");
   if(acquire_cr_lock(mylock) == -1){
       printf("error\n");
       return -1;
   }
   printf("Dentro 1!\n");
   thread_sleep(6);
   
   if(release_cr_lock(mylock) == -1){
       printf("Error relearis\n");
       return -1;
   }
   printf("fuori 1\n");
   getchar();
}


void *func(void *p){
   
   printf("acquiring.\n");
   if(acquire_cr_lock(mylock) == -1){
       printf("error\n");
       return NULL;
   }
   printf("Dentro!\n");
   thread_sleep(6);
   
   if(release_cr_lock(mylock) == -1){
       printf("Error relearis\n");
       return NULL;
   } 
   getchar(); 
}