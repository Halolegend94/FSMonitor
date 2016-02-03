#ifndef __SETTINGSPARSER__
   #define __SETTINGSPARSER__

   #include <stdio.h>
   #include <stdlib.h>
   #include <ctype.h>
   #include <string.h>
   /*structure that represents a single setting*/
   typedef struct _setting{
   	char *name;
   	char *value;
   }setting;

   /*struct that represents a list of settings*/
   typedef struct _settingsList {
   	int count;
   	setting *list;
   } settingsList;


   // ===========================================================================
   //
   // parse_settings
   // Description: parse a list of settings from a file. The settings file must
   // have a setting of the form KEY : VALUE per line. Every line must be ended
   // by a return character. Comments can be written after a # symbol.
   // Params:
   //    -  filename: the filename string
   //    -  settings: a pointer to a settingsList. This structure will be filled
   //       by the function.
   // Returns 0 in case of success, -1 in case of error
   //
   // ===========================================================================
   int parse_settings(char *filename, settingsList *settings);

   // ===========================================================================
   //
   // get_setting_by_name
   // Description: retrieves a particular setting froma settingsList given the
   // key value (the setting name)
   // Params:
   //    -  name: the setting name
   //    -  list: a pointer to a settingsList where to search for the setting
   // Returns: a pointer to a setting in case of success, NULL otherwise
   //
   // ===========================================================================
   setting *get_setting_by_name(char *name, settingsList *list);
#endif
