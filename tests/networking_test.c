#include "../include/networking.h"

int main(int argc, char **argv){
   load_sockets_library();
   if(argv[1][0] == 'm'){
      int s = create_server_socket("6767", 4, SOCK_DGRAM);
      if(s == -1) return 0;
      clientData d;
      //int c = accept_connection(s, &d);
      char rec[100];
      receive_data(s, rec, 100);
      printf("Data rec (form ): %s.\n", rec);
   }else{
      int c = create_connection("127.0.0.1", "6767", SOCK_DGRAM);
      if(c==-1) return;
      char sed[100];
      strcpy(sed, "Ciao come stai.\n");
      send_data(c, sed, 100);
      printf("Data has been sent.\n");

   }
   free_sockets_library();
}
