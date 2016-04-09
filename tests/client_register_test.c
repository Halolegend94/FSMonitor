#include "../include/client_register.h"
#include "../include/networking.h"

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

   printf("Seconda registrazione data1 NONRECURSIVE: \n");
   cr_register_path(reg, &data1, "/home/cristian/Documenti", NONRECURSIVE);
   //print_client_register(reg);

   printf("Terza registrazione data1: \n");
   cr_register_path(reg, &data1, "/home/cristian/Scaricati", RECURSIVE);
   //print_client_register(reg);

   printf("Quarta registrazione data2: \n");
   cr_register_path(reg, &data2, "/home/cristian/Scaricati", RECURSIVE);
   //print_client_register(reg);

   printf("Quinta registrazione data2: \n");
   cr_register_path(reg, &data2, "/home/cristian", NONRECURSIVE);
   //print_client_register(reg);


   printf("Cancelliamo un po di registrazioni.\n\n");
   printf("Cancello scaricati data2: \n");
   cr_unregister_path(reg, &data2, "/home/cristian/Scaricati");
   print_client_register(reg);

   printf("Cancello scaricati data1: \n");
   cr_unregister_path(reg, &data1, "/home/cristian/Scaricati");
   print_client_register(reg);

   printf("Cancello cristian data2: \n");
   cr_unregister_path(reg, &data2, "/home/cristian");
   print_client_register(reg);

   printf("Cancello scaricati data1: \n");
   cr_unregister_path(reg, &data1, "/home/cristian/Documenti");
   print_client_register(reg);
   return 0;
}
