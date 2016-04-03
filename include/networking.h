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
      #include <errno.h>
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
   //
   // create_server_socket
   // Description: creates a server socket that is compatible with both IPv6 and IPv4,
   // and binds it to the first ip available.
   // Params:
   //    -  port: a string that contains the port number which the socket must connect to;
   //    -  maxConnections: maximum number of queued connection requests
   //    -  type: the type of connection (SOCK_STREAM, SOCK_DGRAM)
   // Returns a server socket object if successful, NULL in case of error.
   //
   // ==========================================================================
   serverSocket create_server_socket(char *port, int maxConnections, int type);

   // ==========================================================================
   //
   // accept_connection
   // Description: accept an incoming connection.
   // Params:
   //    -  srv: the server socket
   //    -  clt: a pointer to a location where will be stored a pointer to a clientData
   //       structure that will contain information about the client that contacted the server.
   // Returns a communicationSocket if successful, NULL otherwise.
   //
   // ==========================================================================
   communicationSocket accept_connection(serverSocket srv, clientData *clt);


   // ==========================================================================
   //
   // create_connection
   // Description: opens a communication channel with a server.
   // Params:
   //    -  host: numeric string that contains the server ip address
   //    -  port: the port number the server is listening to
   //    -  type: the type of connection(SOCK_DGRAM, SOCK_STREAM)
   // Returns a communicationSocket if successful, NULL otherwise.
   //
   // ==========================================================================
   struct _communicationSocket *create_connection(char *host, char *port, int type);


   // ==========================================================================
   // send_data
   // ==========================================================================
   int send_data(communicationSocket sock, char *buffer, int buffSize);

   // ==========================================================================
   // receive_data
   // ==========================================================================
   int receive_data(communicationSocket sock, char *buffer, int buffSize);

#endif
