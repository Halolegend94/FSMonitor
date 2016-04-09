#include "../include/mapping.h"
#include <stdio.h>
#include <stdlib.h>

int ssmain(int argc, char **argv) {
	if (argv[1][0] == 'm') {
		pMapping c = NULL;
		int i = create_mapping(&c, "fott", 1024 * 1024);
		if (i == -1) {
			printf("Errore\n");
			exit(1);
		}
		char *pointer = get_mapping_pointer(c);
		pointer[0] = 0;
		pointer[1] = 1;
		pointer[2] = 2;
		sleep(6);
		i = delete_mapping(c);
		if (i == -1) {
			printf("Err del\n");
			return 1;
		}
		return 0;
	}
	else {
		pMapping c;
		int i = open_mapping(&c, "fott", 1024 * 1024);
		char *pointer = get_mapping_pointer(c);
		if (i == -1) {
			printf("Errore v\n");
			exit(1);
		}
		int cc = 0;
		while (cc < 12) {
			printf("cont: %d, %d, %d\n", pointer[0], pointer[1], pointer[2]);
			sleep(2);
			cc++;
		}

		i = delete_mapping(c);
		if (i == -1) {
			printf("Err del\n");
			return 1;
		}
		return 0;
	}
}
