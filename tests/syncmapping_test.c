#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/syncmapping.h"
int masin(void){
  printf("Acquire the lock\n");
  char *nam = malloc(30 *sizeof(char));
  strcpy(nam, "ciao.lock");
  pSyncMapping stLock = NULL;
  syncmapping_createlock(&stLock, nam);
  syncmapping_acquire(stLock);
  printf("Sono dentro! \n");
  sleep(5);
  printf("Sto uscendo! \n");
  syncmapping_release(stLock);
  return 0;
}
