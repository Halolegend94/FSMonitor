#include "../include/watched_list.h"
#include "../include/mem_management.h"

int main(void){
	
	char *mem = malloc(sizeof(char) * 1024);
	pmm_initialize_management(mem, sizeof(char) * 1024, NULL);
	
	watchedList *listW;	
	wl_create(&listW);
	
	wl_add_path("ciao", listW);
	wl_add_path("bella", listW);
	wl_add_path("come", listW);
	wl_add_path("stai", listW);
	
	wl_print_watched_list(listW);
	
	wl_add_path("come", listW);
	wl_add_path("stai", listW);
	wl_add_path("come", listW);
	wl_add_path("stai", listW);
	
	wl_print_watched_list(listW);
	int rem;
	wl_remove_path("ciao", listW, &rem);
	printf("remved %d\n", rem);
	wl_remove_path("bella", listW, &rem);
	printf("remved %d\n", rem);
	wl_remove_path("come", listW, &rem);
	printf("remved %d\n", rem);
	wl_remove_path("stai", listW, &rem);
	printf("remved %d\n", rem);
	
	wl_print_watched_list(listW);
	wl_add_path("ciao", listW);
	wl_add_path("bella", listW);
	wl_add_path("come", listW);
	wl_add_path("stai", listW);
	wl_add_path("come", listW);
	wl_add_path("stai", listW);
	
	wl_print_watched_list(listW);
	
	printf("FINE\n");
	return 0;
}
