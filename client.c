#include "include/client.h"
#define NUM_PWV 6
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

   //LOOP FOREVER
   while(1){
      //READ DATAGRAMS
   }

   return NULL;
}

// ===========================================================================
// execute_command
// ===========================================================================
int execute_command(optToken *comm){
   /*first we need a server on another thread*/
   pToThread p;
   int returnValue = create_thread(client_server, NULL, &p);
   if(returnValue == ERROR){
      fprintf(stderr, "start_tcp_server: error while creating the thread.\n");
      return ERROR;
   }

   /*now we can send the command to the client*/
}

// ===========================================================================
// main
// ===========================================================================
int main(int argc, char **argv){
      /*first parse the parameters.*/
      optToken **list; //list of parsed options
      int numItems; //number of options parsed
      if(parse_params(argc, argv, pwv, 5, &list, &count) == PROG_ERROR){
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
      for(i = 1; i < 3; i++){
         if(!list[i]->isParam){ //error, this is not an option
            fprintf(stderr, "the argument %d is not an option\n.", i);
            exit(0);
         }
         if(strcmp(list[i]->name, "w") == 0) w = 1;
         else if(is_command(list[i]->name) && c == -1){
            c = i;
         }
      }
      if(c == -1){
         fprintf(stderr, "No valid command to execute.\n");
         exit(0);
      }

      /*create UDP server socket */
      int s = create_server_socket(myClient.udpPort, 0, SOCK_DGRAM);
      if(s == PROG_ERROR){
         fprintf(stderr, "tcp_server_function: error while creating the server socket. Terminating the client.\n");
         exit(0);
      }
      /*start the UDP server on another thread*/
      pToThread p;
      int returnValue = create_thread(client_server,(void *)&s, &p);
      if(returnValue == PROG_ERROR){
         fprintf(stderr, "start_tcp_server: error while creating the thread.\n");
         return ERROR;
      }

      /*now we can send the command to the client*/
      if(execute_command(list[i]) == PROG_ERROR){
         fprintf(stderr, "Error while executing the command.\n");
         exit(0);
      }

      if(w) {
         fprintf(stdout, "Waiting for updates. Press any key to exit.\n\n");
         getchar();
      }

      free(myClient.serverAddress);
      free(myClient.tcpPort);
      free(myClient.udpPort);
      free(p);
      free_optTokenList(list, numItems);
      return 0;
   }
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
