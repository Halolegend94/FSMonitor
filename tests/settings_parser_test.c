#include "../include/settings_parser.h"
#include <stdio.h>

int main(int argc, char **argv){
   if(argc != 3) {
      printf("Argomenti insufficienti.\n");
      return 0;
   }
   settingsList impostazioni;
   if(parse_settings(argv[1], &impostazioni) == -1){
         printf("Errore durante il parsing.\n");
         return 1;
   }
   /*cerchiamo per una particolare impostazione*/
   setting *part = get_setting_by_name(argv[2], &impostazioni);
   if(!part){
      printf("impostazione non trovata.\n");
      return 1;
   }
   printf("Valore: %s\n", part->value);
}
