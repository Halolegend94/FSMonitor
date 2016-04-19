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

   typedef struct _clientData{
      struct sockaddr_storage clientAddress;
      int clientLen;
      char *hostName;
   } clientData;

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
   // and binds it to the first ip available (probably the wildcard ANY).
   // Params:
   //    -  port: a string that contains the port number which the socket must connect to;
   //    -  maxConnections: maximum number of queued connection requests (if UDP is selected, this
   //       is ignored.)
   //    -  type: the type of connection (SOCK_STREAM, SOCK_DGRAM)
   // Returns the socket file descriptor if successful, .1 in case of error.
   //
   // ==========================================================================
   int create_server_socket(char *port, int maxConnections, int type);

   // ==========================================================================
   //
   // accept_connection
   // Description: accept an incoming connection.
   // Params:
   //    -  srv: the server socket
   //    -  clt: a pointer to a location where will be stored a pointer to a clientData
   //       structure that will contain information about the client that contacted the server.
   // Returns a socket file descriptor if successful, -1 otherwise.
   //
   // ==========================================================================
   int accept_connection(int serverSocket, clientData **clt);


   // ==========================================================================
   //
   // create_connection
   // Description: opens a communication channel with a server.
   // Params:
   //    -  host: numeric string that contains the server ip address
   //    -  port: the port number the server is listening to
   //    -  type: the type of connection(SOCK_DGRAM, SOCK_STREAM)
   // Returns a socket file descriptor if successful, -1 otherwise.
   //
   // ==========================================================================
   int create_connection(char *host, char *port, int type);


   // ==========================================================================
   //
   // send_data
   // Description: send data (bytes array) through a TPC ord UDP socket.
   // Params:
   //    -  sock: the file descriptor of an open socket
   //    -  buffer: the data buffer pointer
   //    -  buffSize: the data buffer sizeof
   //    -  conn_type: the connection type (SOCK_STREAM, SOCK_DGRAM)
   // Returns 0 if successful, -1 otherwise
   //
   // ==========================================================================
   int send_data(int sock, char *buffer, int buffSize);

   // ==========================================================================
   //
   // receive_data
   // Description: receive data (bytes array) through a TPC ord UDP socket.
   // Params:
   //    -  sock: the file descriptor of an open socket
   //    -  buffer: the pointer to a buffer where to store the received data
   //    -  buffSize: the buffer size
   //    -  conn_type: the connection type (SOCK_STREAM, SOCK_DGRAM)
   // Returns 0 if successful, -1 otherwise
   //
   // ==========================================================================
   int receive_data(int sock, char *buffer, int buffSize);

#endif
