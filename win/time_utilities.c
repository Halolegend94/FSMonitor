#include <windows.h>
#include "../include/time_utilities.h"

// ==========================================================================
// get_current_time
// ==========================================================================
unsigned long long get_current_time(){
   SYSTEMTIME st;
   FILETIME ft;
   GetSystemTime(&st);
   //Convert from system time to filetime
   if(!SystemTimeToFileTime(&st, &ft)){
      fprintf(stderr, "get_current_time: error while converting from systemtime to filetime.\n");
      return 0;
   }
   ULARGE_INTEGER large;
   large.LowPart = ft.dwLowDateTime;
   large.HighPart = ft.dwHighDateTime;
   return large.QuadPart;
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
   long long tmp = fileModTime - serverStartUpTime;
   return tmp / 1E7;
}
