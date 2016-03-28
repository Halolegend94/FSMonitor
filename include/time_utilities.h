#include <stdio.h>
#include <stdlib.h>

// ==========================================================================
//
// get_current_time
// Description: get the current time (the internal representation depends on the platform)
// Returns the current time as long long int, 0 in case of error
//
// ==========================================================================
unsigned long long get_current_time();

// ==========================================================================
//
// get_relative_time
// Description: get the difference between fileModTime and serverStartUpTime, in seconds
// Returns the difference as long long int, -1 in case of error.
//
// ==========================================================================
long long get_relative_time(unsigned long long fileModTime, unsigned long long serverStartUpTime);
