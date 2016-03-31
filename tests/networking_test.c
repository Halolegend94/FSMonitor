#include "../include/networking.h"

int main(int argc, char **argv){
   load_sockets_library();
   if(argv[1][0] == 'm'){
      serverSocket s = create_server_socket("6767", 4, SOCK_STREAM);
      clientData d;
      communicationSocket c = accept_connection(s, &d);
      printf("Finito.\n");
   }else{
      create_connection("192.168.1.4", "6767", 0);
   }

   free_sockets_library();
}
