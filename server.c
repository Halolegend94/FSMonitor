#include "server.h"

1. load settings
(2.) avvia tcp server
2. createlock()
3. acquirelock()
 Se il mapping non esiste
 	create_mapping()
 	initialize_management()
 	initialize_mapping_structure()
 end if
 	scan_itial_path()
releaselock()
 	avvia_deamon()
