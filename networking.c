#include "include/networking.h"

#define  REPRESENTATION_LEN 40
#define DEFAULT_PROTOCOL 0

// ==========================================================================
// load_sockets_library PLATFORM DEPENDENT
// ==========================================================================
int load_sockets_library(){
   #ifndef __linux__
      WSADATA data;
      if(WSAStartup(MAKEWORD(2, 0), &data)){
         fprintf(stderr, "load_sockets_library: version not supported.\n");
         return -1;
      }
      return 0;
   #else
      return 0;
   #endif
}

// ==========================================================================
// free_sockets_library
// ==========================================================================
void free_sockets_library(){
   #ifndef __linux__
   if(WSACleanup() != 0)
      fprintf(stderr, "load_sockets_library: error while freeing the Sockets DLL.\n");
   #endif
}

// ==========================================================================
// create_server_socket
// ==========================================================================
serverSocket create_server_socket(char *port, int maxConnections, int type){

   struct _serverSocket *srv = malloc(sizeof(struct _serverSocket));
   if(!srv){
      fprintf(stderr, "create_server_socket: error while allocating memory.\n");
      return NULL;
   }

   /*get the local ip address*/
   ADDRINFO Hints, *AddrInfo, *it;
   memset(&Hints, 0, sizeof (Hints));
   Hints.ai_family = AF_UNSPEC;
   Hints.ai_socktype = type;
   Hints.ai_flags = AI_NUMERICHOST | AI_PASSIVE;
   Hints.ai_protocol = 0;
   Hints.ai_canonname = NULL;
   Hints.ai_addr = NULL;
   Hints.ai_next = NULL;

   if(getaddrinfo(NULL, port, &Hints, &AddrInfo) != 0){
      fprintf(stderr, "create_server_socket: error while getting the local ip address.\n");
      return NULL;
   }

   /*create the socket*/
   srv->sock = socket(AF_INET6, type, DEFAULT_PROTOCOL);

   if(srv->sock == ERROR_CODE){
      fprintf(stderr, "create_server_socket: error while creating the server socket.\n");
      return NULL;
   }

   /*set the dual stack mode*/
   int val = 0;
   if(setsockopt(srv->sock, IPPROTO_IPV6, IPV6_V6ONLY,(char *) &val, sizeof(int)) == -1){
      fprintf(stderr, "create_server_socket: error while setting the dual stack mode.\n");
      return NULL;
   }
   /*bind the socket*/
   int bindSucceded = 0;
   for (it = AddrInfo; it != NULL; it = it->ai_next) {
       if (bind(srv->sock, it->ai_addr, it->ai_addrlen) == 0){
            bindSucceded = 1;
            break;
       }
   }
   if(!bindSucceded){
       fprintf(stderr, "create_server_socket: error binding the socket.\n");
       return NULL;
   }

   freeaddrinfo(AddrInfo);
   /*listening state*/
   if(listen(srv->sock, maxConnections) == -1) {
      fprintf(stderr, "create_server_socket: error while putting the socket in listening state.\n");
      closesocket(srv->sock);
      return NULL;
   }
   return srv;
}

// ==========================================================================
// accept_connection
// ==========================================================================
struct _communicationSocket *accept_connection(struct _serverSocket *srv, struct _clientData **clt){
   *clt = malloc(sizeof(struct _clientData));
   if(!(*clt)){
      fprintf(stderr, "accept_connection: error while allocating memory.\n");
      return NULL;
   }
   struct _communicationSocket *com = malloc(sizeof(struct _communicationSocket));
   if(!com){
      fprintf(stderr, "accept_connection: error while allocating memory.\n");
      return NULL;
   }
   struct sockaddr_storage from;
   int len;
   com->sock = accept(srv->sock, (struct sockaddr *) &from, &len);
   if(com->sock == ERROR_CODE){
      fprintf(stderr, "accept_connection: error while accepting a connection.\n");
      return NULL;
   }
   (*clt)->clientAddress = from;
   (*clt)->clientLen = len;

   int buffSize = sizeof(char) * REPRESENTATION_LEN;
   char *hostName = malloc(buffSize);
   if(!hostName){
      fprintf(stderr, "accept_connection: error while allocating memory.\n");
      return NULL;
   }
   if(getnameinfo((struct sockaddr *) &((*clt)->clientAddress), (*clt)->clientLen, hostName, buffSize, NULL, 0, NI_NUMERICHOST) != 0){
      fprintf(stderr, "accept_connection: error while getting the string representation of the client address.\n");
      return NULL;
   }
   (*clt)->hostName = hostName;
   return com;
}

// ==========================================================================
// create_connection
// ==========================================================================
struct _communicationSocket *create_connection(char *host, char *port, int type){

   struct _communicationSocket *cm = malloc(sizeof(struct _communicationSocket));
   if(!cm){
      fprintf(stderr, "create_connection: error while allocating memory.\n");
      return NULL;
   }

   struct addrinfo Hints, *AddrInfo;

    memset(&Hints, 0, sizeof (Hints));
    Hints.ai_family = PF_UNSPEC;
    Hints.ai_socktype = type == 0 ? SOCK_STREAM : SOCK_DGRAM;
    int retVal = getaddrinfo(host, port, &Hints, &AddrInfo);
    if(retVal != 0){
        fprintf(stderr, "create_connection: error while retrieving the address for the host.\n");
        return NULL;
    }

    cm->sock = socket(AddrInfo->ai_family, AddrInfo->ai_socktype, AddrInfo->ai_protocol);

   if(cm->sock == ERROR_CODE){
      fprintf(stderr, "create_connection: error while creating the socket.\n");
      return NULL;
   }
   //
   // Notice that nothing in this code is specific to whether we
   // are using UDP or TCP.
   //
   // When connect() is called on a datagram socket, it does not
   // actually establish the connection as a stream (TCP) socket
   // would. Instead, TCP/IP establishes the remote half of the
   // (LocalIPAddress, LocalPort, RemoteIP, RemotePort) mapping.
   // This enables us to use send() and recv() on datagram sockets,
   // instead of recvfrom() and sendto().
   //

   if(connect(cm->sock, AddrInfo->ai_addr, (int) AddrInfo->ai_addrlen) == -1){
      fprintf(stderr, "create_connection: error while connecting to the host.\n");
      return NULL;
   }
   freeaddrinfo(AddrInfo);
   return cm;
}
