#ifndef __SYNCMAPPING__
  #define __SYNCMAPPING__
  #include <stdlib.h>
  #include <stdio.h>

  /*this structure is used as a reference to the file used as mutex.*/
  typedef struct syncMapping *pSyncMapping;

  // ===========================================================================
  //
  // syncmapping_createlock
  // Description: this function creates a new file that will be used as lock
  // by the application in order to coordinate accesses to the mapping. If the
  // file already exists, just opens it to create a pSyncMapping.
  // Params:
  //  - lock: used to store a pointer to a newly created pSyncMapping structure
  //  - lockname: the name of the file that will be created. IMPORTANT: lockname
  //    must be allocated dynamically (it will be freed).
  // Returns -1 in case of error, 0 otherwise
  //
  // ===========================================================================
  int syncmapping_createlock(pSyncMapping *lock);

  // ===========================================================================
  //
  // syncmapping_acquire
  // Description: acquires a pSyncMapping lock
  // Params:
  //  - lock: a pSyncMapping object previously created with syncmapping_createlock.
  // Returns -1 in case of error, 0 otherwise
  //
  // ===========================================================================
  int syncmapping_acquire(pSyncMapping lock);

  // ===========================================================================
  //
  // syncmapping_release
  // Description: release the lock that was acquired by synchmappig_acquire.
  // Params:
  //  - lock: a previously locked pSyncMapping object
  // Returns -1 in case of error, 0 otherwise
  //
  // ===========================================================================
  int syncmapping_release(pSyncMapping lock);

  // ===========================================================================
  //
  // syncmapping_closelock
  // Description: free all the resources of this server created with syncmapping_ceatelock.
  // Other servers can still use the lock.
  // Params:
  //  - lock: a pSyncMapping object previously created with syncmapping_createlock
  // Returns 0 on success, -1 otherwise
  //
  // ===========================================================================
  int syncmapping_closelock(pSyncMapping lock);

  // ===========================================================================
  //
  // syncmapping_deletelock
  // Description: delete the lock file previously created with syncmapping_ceatelock
  // Must be called by the last active process.
  // Params:
  //  - lock: a pSyncMapping object previously created with syncmapping_createlock
  // Returns 0 on success, -1 otherwise
  //
  // ===========================================================================
  int syncmapping_deletelock(pSyncMapping lock);
#endif
