/**************************************************
Utilities
The utilities library contains functions that don't belong in any other library.
****************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// ===========================================================================
//
// sleep
// Desciption: suspend the thread for a specified amount of seconds
// Params:
//	- seconds: the time in seconds during which the thread must sleep
//
// ===========================================================================
void sleep(int seconds);

char *GetLastErrorAsString();
