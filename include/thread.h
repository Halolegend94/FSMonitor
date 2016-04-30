#ifndef __THREAD__
	#define __THREAD__
   #include <stdio.h>
   #include <stdlib.h>

	/*define a type that is a pointer to a thread structure.
	Definition of the structure is platform dependent*/
	typedef struct thread *pToThread;

	// ===========================================================================
	//
	// create_thread
	// Description: creates a new thread that executes a particular function.
	// Params:
	// 	-	pFunction: the pointer to the function that must be executed
	//		-	arg: a pointer to an argumnt for that function
	// 	-	thr: a pointer to a location where a pThread will be stored
	// Returns 0 in case of success, -1 otherwise
	//
	// ===========================================================================
	int create_thread(void *(*pfunction)(void *), void *arg, pToThread *thr);

	// ===========================================================================
	//
	// thread_sleep
	// Description: makes the calling thread sleep for "seconds" seconds
	// Params:
	// 	-	seconds: the number of seconds the calling thread must sleep
	// Returns: in case of error, -1; 0 otherwise
	//
	// ===========================================================================
	int thread_sleep(int seconds);

	// ===========================================================================
	//
	// terminate_thread
	// Description: terminates the execution af a thread.
	// Params:
	// 	-	t: a thread structure created during the create_thread called
	// Returns 0 in case of success, -1 otherwise
	//
	// ===========================================================================
	int terminate_thread(pToThread t);
#endif
