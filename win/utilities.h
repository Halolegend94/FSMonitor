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

// ===========================================================================
//
// get_current_directory
// Description: return a string that contains the current directory full path
// Return the pointer to the string or NULL in case of error
//
// ===========================================================================
char *get_current_directory(void);