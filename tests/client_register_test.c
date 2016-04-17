#include "../include/client_register.h"
#include "../include/networking.h"
#include "../include/received_notification.h"
int main(void){

   //prima fase di tests
   clientRegister *reg;
   create_client_register(&reg);
   clientData data1;
   data1.hostName = "127.0.0.1";

   clientData data2;
   data2.hostName = "127.3.0.1";

   clientData data3;
   data3.hostName = "127.0.5.1";

   clientData data4;
   data4.hostName = "127.0.0.7";

   clientData data5;
   data5.hostName = "127.0.0.3";

   printf("Prima registrazione data1: \n");
   cr_register_path(reg, &data1, "/home/cristian/Documenti", RECURSIVE);
   //print_client_register(reg);

   printf("Quarta registrazione data2: \n");

   cr_register_path(reg, &data3, "/home/cristian/Documenti/Repo V&V", RECURSIVE);
   cr_register_path(reg, &data2, "/home/cristian/Scaricati", RECURSIVE);
   print_client_register(reg);

   receivedNotification not1;
   receivedNotification not2;
   receivedNotification not3;
   receivedNotification not4;

   not1.path = "/home/cristian/Documenti/Nuovo file.txt";
   not1.size = 1;
   not1.modTimestamp = 3;
   not1.perms = "perms";
   not1.isDir = 0;
   not1.type = creation;

   not2.path = "/home/cristian/Documenti/file.txt";
   not2.size = 1;
   not2.modTimestamp = 3;
   not2.perms = "perms";
   not2.isDir = 0;
   not2.type = creation;

   not3.path = "/home/cristian/Documenti";
   not3.size = 1;
   not3.modTimestamp = 3;
   not3.perms = "perms";
   not3.isDir = 1;
   not3.type = deletion;


   //PUSH NOT
   if(cpt_push_notification(reg->treeRoot, &not1, get_string_representation(&not1, 1))==PROG_ERROR){
      printf("Error pushing not1\n");
      return -1;
   }
   cnl_print_list(reg->nodeList);

   //PUSH NOT
   if(cpt_push_notification(reg->treeRoot, &not2, get_string_representation(&not2, 1))==PROG_ERROR){
      printf("Error pushing not2\n");
      return -1;
   }
   cnl_print_list(reg->nodeList);

   //PUSH NOT
   if(cpt_push_notification(reg->treeRoot, &not3, get_string_representation(&not3, 1))==PROG_ERROR){
      printf("Error pushing not3\n");
      return -1;
   }
   cnl_print_list(reg->nodeList);

   return 0;
}
