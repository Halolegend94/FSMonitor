#include <signal.h>
#include <unistd.h>
#include "../include/signal_handler.h"

/*global variables*/
//Used to store the pointer to the real handler
void *(*__myHandler)(void *);

// ===========================================================================
// install_signal_handler
// ===========================================================================
void linux_handler(int param){
    pToThread p;
    if(create_thread(__myHandler, NULL, &p) == -1){
        fprintf(stderr, "linux_handler: error while creating the thread for the handler. "\
        "The program will leave the mapping in an unsconsistent state.\n");
        exit(0);
    }
}


// ===========================================================================
// install_signal_handler
// ===========================================================================
int install_signal_handler(void *(*function)(void *)){
    __myHandler = function;
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = linux_handler;
    if(sigemptyset(&sigIntHandler.sa_mask) == -1){
        fprintf(stderr, "install_signal_handler: error while getting the sigaction mask.\n");
        return -1;
    }
    sigIntHandler.sa_flags = 0;
    if(sigaction(SIGINT, &sigIntHandler, NULL) == -1){
        fprintf(stderr, "install_signal_handler: error while installing the handler.\n");
        return -1;
    }
    return 0;
}
