#include "include/tcp_server.h"
#define BUFFSIZE 512
extern serverStructure server;

/*function prototypes*/
void *__tcp_server_function(void *arg);
void *client_request_handler(void *arg);

// ===========================================================================
//start_tcp_server
// ===========================================================================
int start_tcp_server(){
   create_client_register(&(server.clRegister));
   pToThread p;
   int returnValue = create_thread(__tcp_server_function, NULL, &p);
   if(returnValue == PROG_ERROR){
      fprintf(stderr, "start_tcp_server: error while creating the thread.\n");
      return PROG_ERROR;
   }
}

// ===========================================================================
// tcp_server_function [PRIVATE]
// ===========================================================================
void *__tcp_server_function(void *arg){
   /*create a server socket. This function is the only one that accesses
   the tcpPort value, so no need for mutual exlusion.*/
   int s = create_server_socket(server.tcpPort, server.maxClientConnections, SOCK_STREAM);
   if(s == PROG_ERROR){
      fprintf(stderr, "tcp_server_function: error while creating the server socket. Terminating the server.\n");
      terminate_server();
   }

   //LOOP FOREVER
   while(1){
      clientData *d;
      int c = accept_connection(s, &d);
      if(c == PROG_ERROR){
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
      if(create_thread(client_request_handler, (void *) param, &p) == PROG_ERROR){
         fprintf(stderr, "__tcp_server_function: error while creating the handler thread.\n");
         closesocket(s);
         closesocket(c);
         terminate_server();
      }
   }
}

// ===========================================================================
// convert_abs_path
// ===========================================================================
char *convert_abs_path(char *path){
   char *temp = get_current_directory();
   if(!temp){
      fprintf(stderr, "client_request_handler: error while getting the current directory.\n");
      return NULL;
   }
   char *newpath = concatenate_path(temp, path);
   if(!newpath){
      fprintf(stderr,"client_request_handler: error while concatenating a path.\n");
      return NULL;
   }
   free(temp);
   free(path);
   return newpath;
}

// ===========================================================================
// client_request_handler [PRIVATE]
// ===========================================================================
void *client_request_handler(void *p){

   CRHParams *params = (CRHParams *)p;

   char *stringReceived = malloc(sizeof(char) * BUFFSIZE);
   if(!stringReceived){
      closesocket(params->sock);
      fprintf(stderr, "client_request_handler: error while allocating memory.\n");
      return NULL;
   }
   char buffer[BUFFSIZE];

   int currentSRSize = BUFFSIZE;
   int charsRead = 0;
   int buffIndex = 0;

   /********************************
   STEP 1: first, we get the entire text sent, terminated with a newline.
   *********************************/
   while(1){
      int charsReceived = recv(params->sock, buffer, BUFFSIZE, 0);

      if(charsReceived == -1){
         fprintf(stderr, "client_request_handler: error while reading from buffer.\n");
         closesocket(params->sock);
         return NULL; //close this handler
      }
      else if(charsReceived + charsRead > currentSRSize - 1){
         currentSRSize += BUFFSIZE + charsReceived;
         stringReceived = realloc(stringReceived, sizeof(char) * currentSRSize);
         if(!stringReceived){
            closesocket(params->sock);
            fprintf(stderr, "client_request_handler: error while allocating memory.\n");
            return NULL;
         }
      }
      do{
         stringReceived[charsRead++] = buffer[buffIndex++];
      }while(stringReceived[charsRead - 1] != '\n' && buffIndex < charsReceived);

      if(stringReceived[charsRead - 1] == '\n'){
         stringReceived[charsRead - 1] = '\0';
         break;
      }
   }
   /************************************************
   Step 2: now we get the command to execute from the first part of the string.
   ************************************************/
   char *command = malloc(sizeof(char) * 10); //nine chars + \0 are enough
   if(!command){
      closesocket(params->sock);
      fprintf(stderr, "client_request_handler: error while allocatiing memory.\n");
      return NULL;
   }

   int i;
   charsRead--;
   for(i = 0; i < charsRead; i++){
      if(stringReceived[i] == ' ' || i == 9) break;
      command[i] = stringReceived[i];
   }
   if(i == 9){
      closesocket(params->sock);
      fprintf(stderr, "client_request_handler: the command received is not valid.\n");
      return NULL;
   }
   command[i] = '\0';
   int pathlen = strlen(stringReceived + i + 1);
   char *path = malloc(sizeof(char) * (pathlen + 1));
   if(!path){
      closesocket(params->sock);
      fprintf(stderr, "client_request_handler: error while allocating memory.\n");
      return NULL;
   }
   strcpy(path, stringReceived + i + 1);
   free(stringReceived);

   int commandCode;
   if(strcmp(command, "INFO") == 0) commandCode = INFO;
   else if(strcmp(command, "INNR") == 0) commandCode = INNR;
   else if(strcmp(command, "ADDP") == 0) commandCode = ADDP;
   else if(strcmp(command, "ADDPNR") == 0) commandCode = ADDPNR;
   else if(strcmp(command, "DISC") == 0) commandCode = DISC;
   else commandCode = -1;

   free(command);
   if(commandCode == -1){
      fprintf(stderr, "client_request_handler: command not valid.\n");
      if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
      closesocket(params->sock);
      return NULL;
   }
   /**********************************************************
   Step 3: Recognize and execute command
   ***********************************************************/
   if(commandCode != DISC){
      //check if the directory is expressed as absolute path. If not, build the absolute path
      if(!is_absolute_path(path)){
         path = convert_abs_path(path);
         if(!path){
            fprintf(stderr, "client_request_handler: error while converting relative to absolute path.\n");
            if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
            closesocket(params->sock);
            terminate_server();
         }
      }
      int len = strlen(path);
      if(path[len-1] == '\\' || path[len-1] == '/') path[len-1] = '\0';
      //check if the directory exists
      if(!is_directory(path)){
         if(send_data(params->sock, "404", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
         closesocket(params->sock);
         free(params->data->hostName);
         free(params->data);
         free(params);
         free(path);
         return NULL;
      }
      if(!is_dir_accessible(path)){
         if(send_data(params->sock, "403", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
         closesocket(params->sock);
         free(params->data->hostName);
         free(params->data);
         free(params);
         free(path);
         return NULL;
      }
   }
   //to check for disc, we must enter the critical section
   /*specific checks, need the lock*/
   if(acquire_threadlock(server.crLock) == PROG_ERROR){
      fprintf(stderr, "client_request_handler: error while acquiring the threadlock.\n");
      if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
      closesocket(params->sock);
      terminate_server();
   }
   /**********CRITICAL SECTION***************/
   if(commandCode == DISC){
      int retValue =  cr_unregister_path(server.clRegister, params->data, path);
      /*****END CRITICAL SECTION************/
      if(release_threadlock(server.crLock) == PROG_ERROR){
         fprintf(stderr, "client_request_handler: error while releasing the threadlock.\n");
         if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
         closesocket(params->sock);
         terminate_server();
      }
      if(retValue == PROG_ERROR){
         fprintf(stderr, "client_request_handler: error while removing a registration.\n");
         if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
         closesocket(params->sock);
         terminate_server();
      }else if(retValue == PATH_NOT_REGISTERED){
         if(send_data(params->sock, "400", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
         closesocket(params->sock);
         free(params->data->hostName);
         free(params->data);
         free(params);
         free(path);
         return NULL;
      }else{
         //PROG_SUCCESS
         if(send_data(params->sock, "200", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
         closesocket(params->sock);
         free(params->data->hostName);
         free(params->data);
         free(params);
         free(path);
         return NULL;
      }
   }else{
      int recurs = (commandCode == ADDP || commandCode == INFO) ? 1 : 0;
      int add = (commandCode == ADDP || commandCode == ADDPNR) ? 1 : 0;
      if(add){
         //add the path to the tree
         //=================SYNCMAPPING LOCK============================

         if(syncmapping_acquire(server.mapLock) == PROG_ERROR){
               fprintf(stderr, "client_request_handler: error while acquiring the syncmapping lock.\n");
               if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
               closesocket(params->sock);
               exit(0);
         }
         int regValue = register_server_path(server.structure, server.ID, path);
         if(syncmapping_release(server.mapLock) == PROG_ERROR){
            fprintf(stderr, "client_request_handler: error while releasing the syncmapping lock.\n");
            if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
            closesocket(params->sock);
            cs_terminate_server();
         }
         //===============END SYNCMAPPING LOCK============================

         if(regValue == PATH_NOT_ACCESSIBLE){
            if(send_data(params->sock, "403", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
            closesocket(params->sock);
            free(params->data->hostName);
            free(params->data);
            free(params);
            free(path);

            if(release_threadlock(server.crLock) == PROG_ERROR){
               fprintf(stderr, "client_request_handler: error while releasing the threadlock.\n");
               terminate_server();
            }
            return NULL;
         }else if(regValue == PROG_ERROR){
            fprintf(stderr, "client_request_handler: error while adding the path.\n");
            if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
            closesocket(params->sock);
            terminate_server();
         }else if(regValue == PATH_UPDATED){
            int i;
            for(i = 0; i < server.serverPathsCount; i++){
               if(is_prefix(path, server.serverPaths[i])){
                  server.serverPaths[i] = realloc(server.serverPaths[i], sizeof(char) * (strlen(path)+1));
                  if(!server.serverPaths[i]){
                     fprintf(stderr, "client_request_handler: error while allocating memory.\n");
                     if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
                     closesocket(params->sock);
                     terminate_server();
                  }
                  strcpy(server.serverPaths[i], path);
               }
            }
         }else if(regValue == PROG_SUCCESS){
            //add the path to the list
            server.serverPathsCount++;
            server.serverPaths = realloc(server.serverPaths, sizeof(char *) * server.serverPathsCount);
            if(!server.serverPaths){
               fprintf(stderr, "client_request_handler: error while allocating memory.\n");
               if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
               closesocket(params->sock);
               terminate_server();
            }
            server.serverPaths[server.serverPathsCount - 1] = malloc(sizeof(char) * (strlen(path) + 1));
            if(!(server.serverPaths[server.serverPathsCount - 1])){
               fprintf(stderr, "client_request_handler: error while allocating memory.\n");
               if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
               closesocket(params->sock);
               terminate_server();
            }
            strcpy(server.serverPaths[server.serverPathsCount - 1], path);
         }
      }else{
         //check if the path is monitored
         int i;
         int found = 0;
         for(i = 0; i < server.serverPathsCount; i++){
            if(is_prefix(server.serverPaths[i], path)){
               found = 1;
               break;
            }
         }
         if(!found){
            if(send_data(params->sock, "400", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
            closesocket(params->sock);
            free(params->data->hostName);
            free(params->data);
            free(params);
            free(path);
            if(release_threadlock(server.crLock) == PROG_ERROR){
               fprintf(stderr, "client_request_handler: error while releasing the threadlock.\n");
               terminate_server();
            }
            return NULL;
         }
      }

      //add the registration to the client register
      if(cr_register_path(server.clRegister, params->data, path, recurs ? RECURSIVE : NONRECURSIVE) == PROG_ERROR){
         fprintf(stderr, "client_request_handler: error while registering a client.\n");
         if(send_data(params->sock, "300", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
         closesocket(params->sock);
         terminate_server();
      }

      //it's all ok
      if(send_data(params->sock, "200", 4) == -1) fprintf(stderr, "client_request_handler: error while replying to the client.\n");
      closesocket(params->sock);
      free(params->data->hostName);
      free(params->data);
      free(params);
      free(path);
      if(release_threadlock(server.crLock) == PROG_ERROR){
         fprintf(stderr, "client_request_handler: error while releasing the threadlock.\n");
         terminate_server();
      }
      return NULL; //this thread will die
   }
}
