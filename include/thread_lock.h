#ifndef __THREADLOCK__
   #define __THREADLOCK__
   #include <stdlib.h>
   #include <stdio.h>
   #define PROG_SUCCESS 0
   #define PROG_ERROR -1

   typedef struct threadLock *pThreadLock;

   // ===========================================================================
   //
   // create_threadlock
   //
   // ===========================================================================
   int create_threadlock(pThreadLock *lock);

   // ===========================================================================
   //
   // acquire_threadlock
   //
   // ===========================================================================
   int acquire_threadlock(pThreadLock lock);

   // ===========================================================================
   // release_threadlock
   // ===========================================================================
   int release_threadlock(pThreadLock lock);

   // ===========================================================================
   // close_threadlock
   // ===========================================================================
   int close_threadlock(pThreadLock lock);
#endif
