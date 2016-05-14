#include "include/client.h"
#define NUM_PWV 6
#define MAX_COMMAND_NAME_LEN 15
#define UDP_BUFF_SIZE 512
/*global var
pwv is the set of options that requires a value
(in this case "path"). This will be needed by the parse_params function to work properly.*/
const char *pwv[NUM_PWV] = {"r", "R", "a", "A", "d", "t"};

clientStruct myClient;

// ===========================================================================
// client_server
// ===========================================================================
void *client_server(void *p){
   int s = *((int *)p); //get the socket descriptor
   int currentUpdateSize = UDP_BUFF_SIZE;
   int updateIndex = 0;
   int state = 0; //needed to recognize \r\n.\r\n
   char *update = malloc(sizeof(char) * currentUpdateSize);
   if(!update){
      fprintf(stderr, "client_server: error while allocating memory.\n");
      exit(0);
   }
   char buffer[UDP_BUFF_SIZE];
   //LOOP FOREVER
   while(1){
      //READ DATAGRAMS
      int charsRead = 0;
      int charsReceived = recv(s, buffer, UDP_BUFF_SIZE, 0);
      if(charsReceived == -1){
         fprintf(stderr, "client_server: error while reading from UDP socket.\n");
         exit(0);
      }
      while(charsRead < charsReceived && state != 5){
         if(updateIndex >= (currentUpdateSize - 2)){ //no more space in update
            currentUpdateSize += UDP_BUFF_SIZE;
            update = realloc(update, sizeof(char) * currentUpdateSize);
            if(!update){
               fprintf(stderr, "client_server: error while allocating memory.\n");
               exit(0);
            }
         }
         update[updateIndex] = buffer[charsRead++];

         switch(update[updateIndex++]){
            case '\r':
               if(state == 0) state = 1;
               else if(state == 3) state = 4;
               else state = 0;
               break;
            case '\n':
               if(state == 1) state = 2;
               else if(state == 4) state = 5;
               else state = 0;
               break;
            case '.':
               if(state == 2) state = 3;
               else state = 0;
               break;
         }
      }

      if(state == 5){ //print the update
         update[updateIndex] = '\0';
         state = 0;
         fprintf(stdout, "%s", update);
         updateIndex = 0;
      }
   }
   return NULL;
}


// ===========================================================================
// execute_command
// ===========================================================================
int execute_command(optToken *comm){
   /*create a tcp socket*/
   int s = create_connection(myClient.serverAddress, myClient.tcpPort, SOCK_STREAM);
   if(s == -1){
      fprintf(stderr, "execute_command: error while creating the server socket. Terminating the client.\n");
      exit(0);
   }
   char *buffer = malloc(sizeof(char) * (strlen(comm->value) + MAX_COMMAND_NAME_LEN + 1)); //9 chars for the command name
   if(!buffer){
      fprintf(stderr, "execute_command: error while allocating memory.\n");
      return -1;
   }
   char *commString = NULL;
   switch (comm->name[0]) {
      case 'r': //register in recursive mode
         commString = "INFO";
         break;
      case 'R': //register in nonrecurive mode
         commString = "INNR";
         break;
      case 'a': //add path and register in recursive mode
         commString = "ADDP";
         break;
      case 'A':
         commString = "ADDPNR";
         break;
      case 'd':
         commString = "DISC";
         break;
   }
   if(sprintf(buffer, "%s %s\n", commString, comm->value) < 0){
      fprintf(stderr, "execute_command: error while creating the command string.\n");
      return -1;
   }
   /*send command*/
   if(send_data(s, buffer, strlen(buffer) + 1) == -1){
      fprintf(stderr, "execute_command: error while sending data through the socket.\n");
      return -1;
   }
   /*wait for response*/
   fprintf(stdout, "Waiting for response from the server..\n");
   free(buffer);
   buffer = malloc(sizeof(char) * 4); //we expect just the error code, so this is enough
   if(!buffer){
      fprintf(stderr, "execute_command: error while allocating memory.\n");
      return -1;
   }
   if(receive_data(s, buffer, sizeof(char) * 4) == -1){
      fprintf(stderr, "execute_command: error while allocating memory.\n");
      return -1;
   }
   closesocket(s);
   buffer[3] = '\0';
   /*check return code*/
   if(strcmp(buffer, "200") == 0) {
      fprintf(stdout, "Command executed successfully!.\n");
      free(buffer);
      return 0;
   }else if(strcmp(buffer, "201") == 0){
      fprintf(stdout, "Code 201: you are already registered for the specified folder.\n");
      return 0;
   }else if(strcmp(buffer, "202") == 0){
      fprintf(stdout, "Code 202: the specified folder replaced an existent registration to a subfolder.\n");
      return 0;
   }else if(strcmp(buffer, "400") == 0){
      if(comm->name[0] == 'r' || comm->name[0] == 'R')
         fprintf(stdout, "Error 400: the desired path is not among the monitored ones.\n");
      else
         fprintf(stdout, "Error 400: you were not registered for the specified path.\n");
      free(buffer);
      return -1;
   }else if(strcmp(buffer, "404") == 0){
      fprintf(stdout, "Error 404: path not found.\n");
      free(buffer);
      return -1;
   }else if(strcmp(buffer, "403") == 0){
      fprintf(stdout, "Error 403: path not accessible.\n");
      free(buffer);
      return -1;
   }else if(strcmp(buffer, "300") == 0){
      fprintf(stdout, "Error 300: server internal error.\n");
      free(buffer);
      return -1;
   }else{
      fprintf(stdout, "execute_command: response from server is not valid.\n");
      free(buffer);
      return -1;
   }
}

// ===========================================================================
// main
// ===========================================================================
int main(int argc, char **argv){
   /*first parse the parameters.*/
   optToken **list; //list of parsed options
   int numItems; //number of options parsed
   if(parse_params(argc, argv, pwv, 5, &list, &numItems) == -1){
      fprintf(stderr, "Error while parsing the arguments.\n");
      exit(0);
   }

   /*check the correctness of the params.*/
   if(numItems > 3 || numItems < 2){
      fprintf(stderr, "Invalid number of arguments.\n");
      exit(0);
   }

   /*load settings from file. IP and Port number of the server*/
   settingsList settList;
   if(parse_settings("ClientSettings.txt", &settList) == -1){
      fprintf(stderr, "Error while loading the settings.\n");
      exit(0);
   }
   char *serverAddress = get_setting_by_name("ServerAddress", &settList);
   char *tcpPort = get_setting_by_name("tcpPort", &settList);
   char *udpPort = get_setting_by_name("udpPort", &settList);

   if(!serverAddress || !tcpPort || !udpPort){
      fprintf(stderr, "Missing some network setting from ClientSettings file.\n");
      exit(0);
   }

   myClient.tcpPort = tcpPort;
   myClient.udpPort = udpPort;
   myClient.serverAddress = serverAddress;

   free_settings_structure(&settList);
   /*load networking lib*/
   if(load_sockets_library() == -1){
      fprintf(stderr, "Error while loading the networking library.\n");
      exit(0);
   }

   /*the following lines get the first valid command and check for the -w option.*/
   int i; //counter
   int c = -1; //valid command found?
   int w = 0; //wait for updates?
   for(i = 1; i < numItems; i++){
      if(!list[i]->isParam){ //error, this is not an option
         fprintf(stderr, "the argument %s is not an option.\n", list[i]->value);
         exit(0);
      }
      if(strcmp(list[i]->name, "w") == 0) w = 1;
      else if(is_command(list[i]->name) && c == -1){
         c = i;
      }
   }

   /*create UDP server socket */
   int s = -1;
   pToThread p = NULL;
   if(w){
      s = create_server_socket(myClient.udpPort, 0, SOCK_DGRAM);
      if(s == -1){
         fprintf(stderr, "tcp_server_function: error while creating the server socket. Terminating the client.\n");
         exit(0);
      }
      /*start the UDP server on another thread*/
      int returnValue = create_thread(client_server,(void *)&s, &p);
      if(returnValue == -1){
         fprintf(stderr, "start_tcp_server: error while creating the thread.\n");
         return -1;
      }
   }

   /*now we can send the command to the client, if any*/
   if(c != -1)
      if(execute_command(list[c]) == -1)
         exit(0);

   /*free memory*/
   free(myClient.serverAddress);
   free(myClient.tcpPort);
   free(myClient.udpPort);
   free_optTokenList(list, numItems);
   /*wait for updates*/
   if(w) {
      fprintf(stdout, "Waiting for updates. Press any key to exit.\n\n");
      getchar();
      terminate_thread(p);
   }
   free_sockets_library();
   return 0;
}

// ===========================================================================
// is_command
// ===========================================================================
int is_command(char *str){
   int i;
   for(i = 0; i < NUM_PWV; i++){
      if(strcmp(str, pwv[i]) == 0) return 1;
   }
   return 0;
}
