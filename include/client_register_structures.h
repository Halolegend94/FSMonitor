#ifndef __CLIENTREGSTRUCTURES__
   #define __CLIENTREGSTRUCTURES__
   #include "networking.h"
   #include "linked_list.h"
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   #include "common_definitions.h"
   #include "myfile.h" //for tokenize path
   #include "received_notification.h"

   /*this struct represents a node in the client path tree. The client
   path tree is used to store all the paths the clients want the server to monitor.*/
   typedef struct _registration *pRegistration;
   typedef struct _clientNode *pClientNode;

   typedef struct _pathNode{
      char *name;                   //name of the folder this node represents
      struct _pathNode **children;   //subfolders
      int numChildren;               //number of subfolders
      pRegistration *registrations;   //all the registrations for this path.
      int numRegistrations;          //number of registrations for this path
   } pathNode;

   /*this struct represents a client registration to a path.*/
   typedef struct _registration{
      pClientNode client;
      char *path;
      pathNode *father;
      registrationMode mode;
   } registration;

   typedef struct _clientNode{
      clientData *networkData;
      linkedList *updates;
      char deletionMark; //used to tell if at the next notification dispatch event this clientNode must be removed
      int numRegisteredPaths;
      registration **registeredPaths;
      struct _clientNode *next;
   } clientNode;

   typedef struct _clientNodeList{
      clientNode *first;
      clientNode *last;
   } clientNodeList;
#endif
