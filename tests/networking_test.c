#include "../include/networking.h"

int main(int argc, char **argv){
   load_sockets_library();
   if(argv[1][0] == 'm'){
      serverSocket s = create_server_socket("6767", 4, SOCK_STREAM);
      if(s == NULL) return 0;
      clientData d;
      communicationSocket c = accept_connection(s, &d);
      char rec[100];
      receive_data(c, rec, 100);
      printf("Data: %s.\n", rec);
   }else{
      communicationSocket c = create_connection("127.0.0.1", "6767", SOCK_STREAM);
      if(c==NULL) return;
      char sed[100];
      strcpy(sed, "Ciao come stai.\n");
      send_data(c, sed, 100);
      printf("Data has been sent.\n");

   }

   free_sockets_library();
}
