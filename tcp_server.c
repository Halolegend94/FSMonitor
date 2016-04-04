#include "include/tcp_server.h"

extern serverStructure server;

/*function prototypes*/
void *__tcp_server_function(void *arg);
void *tcp_conn_handler(void *arg);

// ===========================================================================
//start_tcp_server
// ===========================================================================
int start_tcp_server(){
   pToThread p;
   int returnValue = create_thread(__tcp_server_function, NULL, &p);
   if(returnValue == ERROR){
      fprintf(stderr, "start_tcp_server: error while creating the thread.\n");
      return ERROR;
   }
}

// ===========================================================================
// tcp_server_function [PRIVATE]
// ===========================================================================
void *__tcp_server_function(void *arg){
   /*create a server socket. This function is the only one that accesses
   the tcpPort value, so no need for mutual exlusion.*/
   int s = create_server_socket(server.tcpPort, server.maxClientConnections, SOCK_STREAM);
   if(s == ERROR){
      fprintf(stderr, "tcp_server_function: error while creating the server socket. Terminating the server.\n");
      terminate_server();
   }

   //LOOP FOREVER
   while(1){
      clientData d;
      int c = accept_connection(s, &d);
      if(c == ERROR){
         fprintf(stderr, "tcp_server_function: error while accepting a connection. Terminating the server.\n");
         closesocket(s);
         terminate_server();
      }
      CRHParams *param = malloc(sizeof(CRHParams));
      if(!param){
         fprintf(stderr, "tcp_server_function: error while allocating memory.\n");
         closesocket(s);
         closesocket(c);
         terminate_server();
      }
      param->data = d;
      param->sock = c;
      pToThread p;
      if(create_thread(create_client_request_handler, (void *) param, &p) == ERROR){
         fprintf(stderr, "__tcp_server_function: error while creating the handler thread.\n");
         closesocket(s);
         closesocket(c);
         terminate_server();
      }
   }
}
