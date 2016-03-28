#ifndef __SIGNALHANDLER__
    #define __SIGNALHANDLER__

    #include <stdio.h>
    #include <stdlib.h>
    #include "thread.h"
    // ===========================================================================
    //
    // install_signal_handler
    // Description: function used to install a handler to manage the CTRL+C signal
    // from terminal. It is used to terminate the server, so we need to perform
    // cleaning operations when it happens.
    // Params:
    //  -   function: a pointer to a function that will handle the server closure.
    // Returns 0 in case of success, -1 otherwise
    //
    // ===========================================================================
    int install_signal_handler(void *(*function)(void *));

#endif
