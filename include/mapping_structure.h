#ifndef __MAPPINGSTRUCTURE__
#define __MAPPINGSTRUCTURE__

#include "notifications_bucket.h"
#include "filesystree.h"
#include "mem_management.h"
#include "common_utilities.h"
#include "received_notification.h"
#include <stdio.h>
#include <stdlib.h>

/*The following structure is used to organize data inside
the file mapping.*/
typedef struct _mappingStructure{
	unsigned long off_fileSystemTree; //offset to filesystem tree structure
	unsigned int serverCounter; //used to generate server ID
	unsigned int daemonServer;  //indicates witch is the server that runs the deamon
	unsigned long off_notifications; //offset to notifications bucket
	unsigned int refreshTime;   //how much time needs to pass before another
				    //check is done by the deamon

} mappingStructure;

/*function prototypes*/

// ===========================================================================
// initialize__mapping_structure
// ===========================================================================
int initialize_mapping_structure(char *memoryBlock, mappingStructure **str, unsigned int refTime,
												char *path);

// ===========================================================================
// generate_server_id
// ===========================================================================
unsigned int generate_server_id(mappingStructure *str);

// ===========================================================================
// register_server_path
// ===========================================================================
int register_server_path(mappingStructure *str, unsigned int serverID, char *path);

// ===========================================================================
// unregister_server
// ===========================================================================
int unregister_server(mappingStructure *str, unsigned int sid, char **list, int count);


#endif
