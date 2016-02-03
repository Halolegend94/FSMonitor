#include "../include/syncmapping.h"
#include <windows.h>

/*this structure is used as a reference to the file used as mutex.*/
struct syncMapping {
    HANDLE fileHandle;
	 char *fileName;
	 OVERLAPPED sOverlapped;
};

// ===========================================================================
// syncmapping_createlock
// ===========================================================================
int syncmapping_createlock(struct syncMapping **lock, char *lockname){
	OVERLAPPED sOverlapped;
	sOverlapped.Offset = 0;
	sOverlapped.OffsetHigh = 0;
	sOverlapped.hEvent = 0;
	HANDLE fHandle = CreateFile(lockname, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
	if (fHandle == INVALID_HANDLE_VALUE) {
		/*the lock file is already there, just get the handle*/
		fHandle = CreateFile(lockname, GENERIC_READ | GENERIC_WRITE,
			FILE_SHARE_WRITE | FILE_SHARE_READ, NULL,OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (fHandle == INVALID_HANDLE_VALUE) {
			fprintf(stderr, "An error occurred while opening the syncmapping lock.\n");
			return -1;
		}
	}
	//allocate a sycnMapping structure
	*lock = (struct syncMapping *) malloc(sizeof(struct syncMapping));
	if(!(*lock)){
		fprintf(stderr, "Error while allocating memory.\n");
		CloseHandle(fHandle);
		return -1;
	}
	(*lock)->fileName = lockname;
	(*lock)->sOverlapped = sOverlapped;
	(*lock)->fileHandle = fHandle;
	return 0;
}

// ===========================================================================
// syncmapping_acquire
// ===========================================================================
int syncmapping_acquire(struct syncMapping *lock) {
	BOOL locked = LockFileEx(lock->fileHandle, LOCKFILE_EXCLUSIVE_LOCK, 0, 1, 0, &lock->sOverlapped);
	if (locked == FALSE) {
		fprintf(stderr, "Error while acquiring the syncmapping lock.\n");
		return -1;
	}
	return 0;
}

// ===========================================================================
// syncmapping_release
// ===========================================================================
int syncmapping_release(struct syncMapping *lock) {
	BOOL unlocked = UnlockFileEx(lock->fileHandle, 0, 1, 0, &lock->sOverlapped);
	if (unlocked == FALSE) {
		fprintf(stderr, "Error while releasing the syncmapping lock.\n");
		return -1;
	}
	return 0;
}

// ===========================================================================
// syncmapping_deletelock
// ===========================================================================
int syncmapping_deletelock(struct syncMapping *lock){
	if(!CloseHandle(lock->fileHandle)){
		fprintf(stderr, "Error while closing the lock file handle.\n");
		return -1;
	}
	if(!DeleteFile(lock->fileName)){
		fprintf(stderr, "Error while deleting the lock file.\n");
		return -1;
	}
	free(lock->fileName);
	free(lock);
	return 0;
}
