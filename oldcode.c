//remove all registrations to subdirectories
foundNode->registeredPaths[i]->mode = RECURSIVE;
int k;
for(k = 0; k < foundNode->numRegisteredPaths; k++){
   printf("k: %d/%s\n", k, foundNode->numRegisteredPaths);
   if(is_prefix(foundNode->registeredPaths[k]->path, path) && k != i){
      /*delete registration from tree*/
      pathNode *father = foundNode->registeredPaths[k]->father;
      if(father->numRegistrations == 1){
         free(father->registrations[0]->path);
         free(father->registrations[0]);
         free(father->registrations);
         father->registrations = NULL;
      }else{
         registration **newRegList = malloc(sizeof(registration *) * (father->numRegistrations - 1));
         if(!newRegList){
            fprintf(stderr, "cpt_add_client_registration: error while allocating memory.\n");
            return PROG_ERROR;
         }
         int l, j;
         for(l = 0, j = 0; l < father->numRegistrations; l++){
            if(father->registrations[l] != foundNode->registeredPaths[k]){
               newRegList[j++] = father->registrations[l];
            }else{
               //delete registration structure
               free(father->registrations[l]->path);
               free(father->registrations[l]);
            }
         }
         free(father->registrations);
         father->registrations = newRegList;
      }
      foundNode->registeredPaths[k] = NULL; //mark
      removed++;
   }
}
//delete all marked entries
if(removed > 0){
   //removed cannot be equal to the number of registerd paths because of i != k
   registration **newRegList = malloc(sizeof(registration *) * (foundNode->numRegisteredPaths - removed));
   if(!newRegList){
      fprintf(stderr, "cpt_add_client_registration: error while allocating memory.\n");
      return PROG_ERROR;
   }
   int l, j;
   for(l = 0, j = 0; l < foundNode->numRegisteredPaths; l++){
      if(foundNode->registeredPaths[l] != NULL){
         newRegList[j++] = foundNode->registeredPaths[l];
      }
   }
   free(foundNode->registeredPaths);
   foundNode->registeredPaths = newRegList;
   foundNode->numRegisteredPaths = foundNode->numRegisteredPaths - removed;
}
}else{
