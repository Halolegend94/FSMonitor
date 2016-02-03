/*questo file contiene un insieme di funzioni per gestire
thread windows.*/
#include <windows.h>

typedef struct _thread {
	HANDLE mythread;
}thread;

typedef struct _t_args_wrapper {
	int(*pFunction) (void *);
	void *arg;
} threadFunction;

/*funzione wrapper*/
DWORD WINAPI __wrapperTFunction(LPVOID pointer) {
	threadFunction *params = (threadFunction *)pointer;
	int (*myfunction)(void*) = *(params->pFunction);
	myfunction(params->arg);
	free(pointer);
}

thread create_thread(int(*pfunction)(void *), void *arg) {
	thread temp;
	threadFunction *func = (threadFunction*) malloc(sizeof(threadFunction));
	if (!func) {
		printf("Errore nell'allocare memoria!\n");
		exit(1);
	}
	func->arg = arg;
	func->pFunction = pfunction;
	temp.mythread = CreateThread(NULL, 0, __wrapperTFunction, func, 0, NULL);
	return temp;
}
