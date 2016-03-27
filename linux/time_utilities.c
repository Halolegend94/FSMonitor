#include <time.h>
#include "../include/time_utilities.h"
// ==========================================================================
// get_current_time
// ==========================================================================
long long get_current_time(){
   long long t = (long long) time(NULL);
   if(t == -1){
      fprintf(stderr, "get_current_time: error while getting the current time.\n");
   }
   return t;
}

// ==========================================================================
// get_relative_time
// ==========================================================================
long long get_relative_time(long long fileModTime, long long serverStartUpTime){
   if(fileModTime < serverStartUpTime){
      fprintf(stderr, "fileModTIme (%lli) is minor than serverStartUpTime (%lli)!\n", fileModTIme,
         serverStartUpTime);
      return -1;
   }
   long long tmp = fileModTIme - serverStartUpTime;
   return tmp;
}
