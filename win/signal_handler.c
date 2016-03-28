#include <windows.h>
#include "../include/signal_handler.h"
/*global variables*/
//Used to store the pointer to the real handler
void *(*__myHandler)(void *);

// ===========================================================================
// CtrlHandler
// NOTE: this is a wrapper handler
// ===========================================================================
BOOL WindowsHandler(DWORD signal){
    switch(signal){
    case CTRL_C_EVENT:{
        pToThread p;
        if(create_thread(__myHandler, NULL, &p) == -1){
            fprintf(stderr, "WindowsHandler: error while creating the thread for the handler."\
            "The program will terminate leaving the mapping in an unsconsistent state.\n");
            exit(0);
        }
        return TRUE;
    }
    default:
      return FALSE;
    }
}

// ===========================================================================
// install_signal_handler
// ===========================================================================
int install_signal_handler(void *(*function)(void *)){
    __myHandler = function;
    if(SetConsoleCtrlHandler((PHANDLER_ROUTINE) WindowsHandler, TRUE) == 0){
        fprintf(stderr, "install_signal_handler: error while installing the handler.\n");
        return -1;
    }
    return 0;
}
