#include "../include/myfile.h"
int main(void) {
	char *dir = get_current_directory();
	myFileList list;
	if (get_directory_content(dir, &list) == -1) {
		fprintf(stderr, "Errore\n");
		return 1;
	}
	int i;
	for (i = 0; i < list.count; i++) {
		printf("Nome: %s, Dimensione: %lli, Last Mod: %lli, Perms: %s\n", list.list[i].name, \
		list.list[i].size, list.list[i].lastWriteTimestamp, list.list[i].perms);
	}

	//test is directory
	int x = is_directory("include");
	if(x == -1){
		printf("error no such dir.\n");
		return 0;
	}else if(x == 0){
		printf("no dir.\n");
	}else{
		printf("si dir.\n");
	}
	return 0;
}
