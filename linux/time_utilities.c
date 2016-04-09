#include <time.h>
#include "../include/time_utilities.h"
// ==========================================================================
// get_current_time
// ==========================================================================
unsigned long long get_current_time(){
   long long t = (long long) time(NULL);
   if(t == -1){
      fprintf(stderr, "get_current_time: error while getting the current time.\n");
      return 0;
   }
   return (unsigned long long) t;
}

// ==========================================================================
// get_relative_time
// ==========================================================================
long long get_relative_time(unsigned long long fileModTime, unsigned long long serverStartUpTime){
   if(fileModTime < serverStartUpTime){
      fprintf(stderr, "fileModTIme (%llu) is minor than serverStartUpTime (%llu)!\n", fileModTime,
         serverStartUpTime);
      return -1;
   }
   long long tmp = (long long) (fileModTime - serverStartUpTime);
   return tmp;
}
