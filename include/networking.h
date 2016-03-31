#ifndef __NETWORKING__
   #define __NETWORKING__
   #include <stdio.h>
   #include <stdlib.h>
   #include <string.h>
   /*follows some platform dependent stuff*/
   #ifdef __linux__
      #include <sys/socket.h>
      #include <sys/types.h>
      #include <netinet/in.h>
      #include <unistd.h>
      #include <arpa/inet.h>
      #include <netdb.h>
      #include <fcntl.h>
      #define ERROR_CODE -1
      #define closesocket close
   #else //Windows
      #include <winsock2.h>
      #include <ws2tcpip.h>
      #include <mstcpip.h>
      #include <windows.h>
      #pragma comment(lib, "Ws2_32.lib") //link the library
      #define ERROR_CODE INVALID_SOCKET
   #endif

   typedef struct _serverSocket{
      struct sockaddr_storage serverAddress;
      int sock;
      int serverLen;
   } *serverSocket;

   typedef struct _clientData{
      struct sockaddr_storage clientAddress;
      int clientLen;
      char *hostName;
   } *clientData;

   typedef struct _communicationSocket{
      int sock;
   } *communicationSocket;


   // ==========================================================================
   //
   // load_sockets_library
   // Description: loads the needed library to use sockets. This function is used
   // because Windows needs to load a particular DLL (WS2_32.DLL), while linux
   // doesn't. Therefore this function has a funcion call in the windows implementation,
   // and an empty body in the linux implementation.
   //
   // ==========================================================================
   int load_sockets_library(void);

   // ==========================================================================
   //
   // free_sockets_library
   // Description: unload the DLL on windows, does nothing on linux. See load_sockets_library.
   //
   // ==========================================================================
   void free_sockets_library(void);


   // ==========================================================================
   // create_server_socket
   // ==========================================================================
   serverSocket create_server_socket(char *port, int maxConnections, int type);

   // ==========================================================================
   // accept_connection
   // ==========================================================================
   communicationSocket accept_connection(serverSocket srv, clientData *clt);


   // ==========================================================================
   // create_connection
   // ==========================================================================
   struct _communicationSocket *create_connection(char *host, char *port, int type);
#endif
