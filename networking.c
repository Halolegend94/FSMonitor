#include "include/networking.h"
#define REPRESENTATION_LEN 40
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
int create_server_socket(char *port, int maxConnections, int type){

   int sock; //will contain the socket FD

   /*get the local ip address*/
   struct addrinfo Hints, *AddrInfo, *it;
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
      return -1;
   }

   /*create the socket*/
   sock = socket(AF_INET6, type, DEFAULT_PROTOCOL);

   if(sock == ERROR_CODE){
      fprintf(stderr, "create_server_socket: error while creating the server socket.\n");
      return -1;
   }

   /*set the dual stack mode*/
   int val = 0;
   if(setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,(char *) &val, sizeof(int)) == -1){
      fprintf(stderr, "create_server_socket: error while setting the dual stack mode.\n");
      return -1;
   }
   /*bind the socket*/
   int bindSucceded = 0;
   for (it = AddrInfo; it != NULL; it = it->ai_next) {
       if (bind(sock, it->ai_addr, it->ai_addrlen) == 0){
            bindSucceded = 1;
            break;
       }
   }
   if(!bindSucceded){
       fprintf(stderr, "create_server_socket: error binding the socket.\n");
       closesocket(sock);
       return -1;
   }

   freeaddrinfo(AddrInfo);

   /*listening state if SOCK_STREAM*/
   if(type == SOCK_STREAM){
      if(listen(sock, maxConnections) == -1) {
         fprintf(stderr, "create_server_socket: error while putting the socket in listening state.\n");
         closesocket(sock);
         return -1;
      }
   }
   return sock;
}

// ==========================================================================
// accept_connection
// ==========================================================================
int accept_connection(int srv, struct _clientData **clt){
   *clt = malloc(sizeof(struct _clientData));
   if(!(*clt)){
      fprintf(stderr, "accept_connection: error while allocating memory.\n");
      return -1;
   }
   int sock;
   struct sockaddr_storage from;
   int len = sizeof(from);
   sock = accept(srv, (struct sockaddr*) &from, &len);
   if(sock == ERROR_CODE){
      fprintf(stderr, "accept_connection: error while accepting a connection.\n");
      return -1;
   }
   (*clt)->clientAddress = from;
   (*clt)->clientLen = len;

   int buffSize = sizeof(char) * REPRESENTATION_LEN;
   char *hostName = malloc(buffSize);
   if(!hostName){
      fprintf(stderr, "accept_connection: error while allocating memory.\n");
      return -1;
   }
   if(getnameinfo((struct sockaddr *) &((*clt)->clientAddress), (*clt)->clientLen, hostName,
      buffSize, NULL, 0, NI_NUMERICHOST) != 0){
      fprintf(stderr, "accept_connection: error while getting the string representation of the client address.\n");
      return -1;
   }
   (*clt)->hostName = hostName;
   return sock;
}

// ==========================================================================
// create_connection
// ==========================================================================
int create_connection(char *host, char *port, int type){
   int sock;
   struct addrinfo Hints, *AddrInfo;
   memset(&Hints, 0, sizeof (Hints));
   Hints.ai_family = AF_INET6;
   Hints.ai_socktype = type;
   Hints.ai_flags = AI_NUMERICHOST | AI_V4MAPPED;
   Hints.ai_protocol = 0;
   Hints.ai_canonname = NULL;
   Hints.ai_addr = NULL;
   Hints.ai_next = NULL;

   int retVal = getaddrinfo(host, port, &Hints, &AddrInfo);
   if(retVal != 0){
      fprintf(stderr, "create_connection: error while retrieving the address for the host.\n");
      return -1;
   }

   sock = socket(AF_INET6, type, DEFAULT_PROTOCOL);
   if(sock == ERROR_CODE){
      fprintf(stderr, "create_server_socket: error while creating the server socket.\n");
      return -1;
   }
   /*set the dual stack mode*/
   int val = 0;
   if(setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY,(char *) &val, sizeof(int)) == -1){
      fprintf(stderr, "create connection: error while setting the dual stack mode.\n");
      return -1;
   }
   if(sock== ERROR_CODE){
      fprintf(stderr, "create_connection: error while creating the socket.\n");
      return -1;
   }

   if(connect(sock, AddrInfo->ai_addr, AddrInfo->ai_addrlen) == -1){
      fprintf(stderr, "create_connection: error while connecting to the host.\n");
      return -1;
   }

   freeaddrinfo(AddrInfo);
   return sock;
}

// ==========================================================================
// send_data
// ==========================================================================
int send_data(int sock, char *buffer, int buffSize){
   int totBytesSent = 0;
   while(totBytesSent < buffSize){
      int ret = send(sock, buffer + totBytesSent, buffSize - totBytesSent, 0);
      if(ret == -1){
         fprintf(stderr, "send_data: error while sending data through the socket.\n");
         return -1;
      }
      totBytesSent += ret;
   }
   return 0;
}

// ==========================================================================
// receive_data
// ==========================================================================
int receive_data(int sock, char *buffer, int buffSize){
   int totBytesReceived = 0;
   while(totBytesReceived < buffSize){
      int ret = recv(sock, buffer + totBytesReceived, buffSize - totBytesReceived, 0);
      if(ret == -1){
         fprintf(stderr, "receive_data: error while reading data through the socket.\n");
         return -1;
      }
      totBytesReceived += ret;
   }
   return 0;
}
