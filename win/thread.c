#include "../include/thread.h"
#include <windows.h>

struct thread {
	HANDLE handler;
};

//wrapper structure for the arguments
typedef struct _functionArgsWrapper {
	void *(*pFunction) (void *);
	void *arg;
} functionArgsWrapper;


// ===========================================================================
// __wrapperTFunction
// NOTE: used as wrapper because CreateThread wants a function that returns DWORD
// ===========================================================================
DWORD WINAPI __wrapperTFunction(LPVOID pointer) {
	functionArgsWrapper *params = (functionArgsWrapper *) pointer;
	void *(*myFunction)(void *) = params->pFunction;
	myFunction(params->arg);
	free(pointer);
	return 0;
}

// ===========================================================================
// create_thread
// ===========================================================================
int create_thread(void *(*pFunction)(void *), void *arg, struct thread **thr) {
	*thr = malloc(sizeof(struct thread));
	if(!(*thr)){
		fprintf(stderr, "create_thread: error while allocating memory.\n");
		return -1;
	}
	functionArgsWrapper *func = (functionArgsWrapper *) malloc(sizeof(functionArgsWrapper));
	if (!func) {
		fprintf(stderr, "create_thread: error while allocating memory.\n");
		return -1;
	}
	func->arg = arg;
	func->pFunction = pFunction;
	(*thr)->handler = CreateThread(NULL, 0, __wrapperTFunction, func, 0, NULL);
	if((*thr)->handler == NULL){
		fprintf(stderr, "create_thread: error while creating the thread.\n");
		return -1;
	}
	return 0;
}

// ===========================================================================
// thread_sleep
// ===========================================================================
int thread_sleep(int seconds){
	Sleep(seconds * 1000);
	return 0;
}
