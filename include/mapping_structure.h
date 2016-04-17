#ifndef __MAPPINGSTRUCTURE__
	#define __MAPPINGSTRUCTURE__
	#include "notifications_bucket.h"
	#include "filesystree.h"
	#include "mem_management.h"
	#include "common_definitions.h"
	#include "myfile.h"
	#include "time_utilities.h"
	#include "received_notification.h"
	#include <stdio.h>
	#include <stdlib.h>

	/*The following structure is used to organize data inside
	the file mapping.*/
	typedef struct _mappingStructure{
		unsigned long off_fileSystemTree;//offset to filesystem tree structure
		int serverCounter; 					//how many servers are online
		int daemonServer;  					//indicates witch is the server that runs the deamon
		unsigned long off_notifications; //offset to notifications bucket
		int refreshTime;   					//how much time needs to pass before another
					    							//check is done by the deamon
		int idCounter;    					//incremented by 1 each time a server registers. Used to generate server ID
		unsigned long long lastUpdate; 			//the last time the structure was updated
	} mappingStructure;


	/*function prototypes*/

	// ===========================================================================
	//
	// initialize__mapping_structure
	// Description: this function initializes the mapping with all the needed structures.
	// Params:
	// 	-	memoryBlock: if NULL, this function must be called by the first server that
	// 		created the mapping, otherwise it is the first allocated block trough the
	// 		personal malloc.
	// 	-	str: a pointer to a memory area where a ponter to the mapping structure
	// 		will be saved.
	// 	-	refTime: refresh rime requested by the calling server.
	// 	-	path: the first path to monitor
	// Returns -1 in case of error, -2 in case the path doesn't exist, the server id otherwise.
	//
	// ===========================================================================
	int initialize_mapping_structure(char *memoryBlock, mappingStructure **str,
		 unsigned int refTime, char *path);

	// ===========================================================================
	//
	// uodate
	// Description: this function update the data structure to match the OS filesystem
	// and send notifications of changes to the listening servers.
	// Params:
	// 	- str: the pointer to the mapping structure.
	// Returns -1 in case of error, o otherwise
	//
	// ===========================================================================
	int update(mappingStructure *str);

	// ===========================================================================
	//
	// register_server_path
	// Description: this function adds a server path to monitor
	// Params:
	// 	-	str: the poiter to the mapping structure
	// 	-	serverID: the id of the server that requests the the path monitoring
	// 	-	path: the path to be monitored
	// Returns: -1 in case of error, -2 in case the path doesn't exist, 0 in case of success.
	//
	// ===========================================================================
	int register_server_path(mappingStructure *str, int serverID, char *path);

	// ===========================================================================
	//
	// unregister_server
	// Description: removes the data of the calling server from the mapping.
	// That is, all its bucket will be deleted, as the filesystree braches monitored only by it
	// Params:
	// 	-	str: the pointer to the mapping structure
	// 	-	sid: the server id
	// 	-	list: a pointer to a list of all the paths monitored by a server.
	// 	-	count: the number of strings in list
	// Returns 0 in case of success, -1 otherwise
	//
	// ===========================================================================
	int unregister_server(mappingStructure *str, int sid, char **list, int count);

	// ===========================================================================
	//
	// get_notifications
	// Description: get the notification for a server from its buckets
	// Params:
	// 	-	str: the pointer to the mapping structure
	//		-	sid: the server id
	//		-	list: a pointer to a location where a pointer to a list of receivedNotification will
	// 	 	be stored.
	// 	-	count: a pointer to a location where the number of notifications retrieved will be stored
	// Returns: 0 in case of success, -1 otherwise
	//
	// ===========================================================================
	int get_notifications(mappingStructure *str, int sid, receivedNotification ***list, int *count,
		char ***deletedPaths, int *numDeletedPaths);

	// ===========================================================================
	//
	// print_mappingstructure_state
	// print a representation of the mapping data structure [DEBUG PURPOSES]
	//
	// ===========================================================================
	void print_mappingstructure_state(mappingStructure *str);

#endif
