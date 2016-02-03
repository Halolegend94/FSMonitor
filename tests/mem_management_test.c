#include <stdio.h>
#include <stdlib.h>
#include "../include/mem_management.h"
int main(void) {
	printf("Inizio\n");
	int siz = sizeof(char) * 1024 * 1024 * 9;
	char *p = malloc(siz);
	if (!p) {
		printf("Errore malloc, troppo grande?\n");
		exit(1);
	}
	pmm_initialize_management(p, siz, NULL);
	pmm_print_memory_status();
	char *base = pmm_malloc(3);
	base[0] = 4;
	base[1] = 2;
	base[2] = 1;
	printf("First allocation 3 bytes\n");
	pmm_print_memory_status();

	printf("Second allocation 2 bytes\n");
	char *uno = pmm_malloc(2);
		pmm_print_memory_status();

	printf("Third allocation 1 byte\n");
	char *tre = pmm_malloc(1);
		pmm_print_memory_status();

	printf("Delete second allocation\n");
	pmm_free(uno);
		pmm_print_memory_status();

	printf("Fourth allocation\n");
	char *quat = pmm_malloc(4);
		pmm_print_memory_status();

	printf("Fifth allocation\n");
	char *due = pmm_malloc(2);
		pmm_print_memory_status();

	printf("Delete Forth allocation 4 byte\n");
	pmm_free(quat);
		pmm_print_memory_status();

	printf("Delete Fifth allocation 2 byte\n");
	pmm_free(due);
		pmm_print_memory_status();

	printf("Delete first allocation 3 byte\n");
	pmm_free(base);
		pmm_print_memory_status();

	printf("Delete third allocation 1 byte\n");
	pmm_free(tre);
	pmm_print_memory_status();



	free(p);
	getchar();
}
