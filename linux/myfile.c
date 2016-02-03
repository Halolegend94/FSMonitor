#include "../include/myfile.h"
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>

/*definitions used to allocate space for files info*/
#define MAXFILES 20
#define MAXFILES_INC 15
#define MAXPATHLEN 1024 //used in get current directory

/*function prototypes*/
int get_directory_content(char*, myFileList*);
char *__get_perm_string(mode_t);
int delete_file(char*);
int is_directory(char*);
char *get_current_directory(void);
char *concatenate_path(char*, char*);

// ===========================================================================
// get_directory_content
// ===========================================================================
int get_directory_content(char *dir, myFileList *fileList){
	/*allocate initial resources*/
	fileList->list = (myFile *) malloc(MAXFILES * sizeof(myFile));
	if(!fileList->list){
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}
	fileList->count = 0;
	int currentListCapacity = MAXFILES;

	/*open directory*/
	struct dirent *dirEntry;
	DIR *dpointer;
	dpointer = opendir(dir);
	if(!dpointer){
		fprintf(stderr, "Error while opening the directory.\n");
		free(fileList->list);
		return -1;
	}
	dirEntry = readdir(dpointer);
	/*scan directory*/
	while(dirEntry != NULL){
		if(strcmp(dirEntry->d_name, ".") == 0 ||
		strcmp(dirEntry->d_name, "..") == 0){
			dirEntry = readdir(dpointer);
			continue; //skip special dirs
		}
		if(fileList->count >= currentListCapacity){ //if we exceed the initial space, realloc it
			currentListCapacity+= MAXFILES_INC;
			fileList->list = (myFile *) realloc(fileList->list, currentListCapacity * sizeof(myFile));
			if(!fileList->list){
				fprintf(stderr, "Error while reallocating memory.\n");
				return -1;
			}
		}
		int nameLen = strlen(dirEntry->d_name);
		fileList->list[fileList->count].name = (char *) malloc((nameLen + 1) * sizeof(char));
		if(!fileList->list[fileList->count].name){
			fprintf(stderr, "Error while allocating memory.\n");
			free(fileList->list);
			return -1;
		}
		//set the file name
		strcpy(fileList->list[fileList->count].name, dirEntry->d_name);
		struct stat fileData;
		mode_t mode;
		int result = stat(dirEntry->d_name, &fileData);
		if(result == -1){
			fprintf(stderr, "Error while retrieving file \"%s\" information.\n", dirEntry->d_name);
			free(fileList->list);
			free(fileList->list[fileList->count].name);
			return -1;
		}
		mode = fileData.st_mode;
		//set isDir
		if(S_ISDIR(mode))
			fileList->list[fileList->count].isDir = 1;
		else
			fileList->list[fileList->count].isDir = 0;
		//set file size and modification time
		fileList->list[fileList->count].size = fileData.st_size;
		fileList->list[fileList->count].lastWriteTimestamp = fileData.st_mtime;
		fileList->list[fileList->count].perms = __get_perm_string(mode);
		if(!fileList->list[fileList->count].perms){
			fprintf(stderr, "Error while retrieving the permissions string for \"%s\".\n", dirEntry->d_name);
			free(fileList->list);
			free(fileList->list[fileList->count].name);
			return -1;
		}
		fileList->count++;
		//read next entry
		dirEntry = readdir(dpointer);
	}
	return 0;
}

// ===========================================================================
// __get_perm_string
// ===========================================================================
char *__get_perm_string(mode_t mode){
	char *temp = (char *) malloc(10 * sizeof(char));
	if(!temp){
		fprintf(stderr, "Error while allocating memory.\n");
		return NULL;
	}
	if(sprintf(temp, "%s", (mode & S_IRUSR) ? "r" : "-") < 0 ||
	sprintf(temp + 1, "%s", (mode & S_IWUSR) ? "w" : "-") < 0 ||
	sprintf(temp + 2, "%s", (mode & S_IXUSR) ? "x" : "-") < 0 ||
	sprintf(temp + 3, "%s", (mode & S_IRGRP) ? "r" : "-") < 0 ||
	sprintf(temp + 4, "%s", (mode & S_IWGRP) ? "w" : "-") < 0 ||
	sprintf(temp + 5, "%s", (mode & S_IXGRP) ? "x" : "-") < 0 ||
	sprintf(temp + 6, "%s", (mode & S_IROTH) ? "r" : "-") < 0 ||
	sprintf(temp + 7, "%s", (mode & S_IWOTH) ? "w" : "-") < 0 ||
	sprintf(temp + 8, "%s", (mode & S_IXOTH) ? "x" : "-") < 0){
		fprintf(stderr, "Error while getting the file permission string.\n");
		free(temp);
		return NULL;
	}
	return temp;
}

// ===========================================================================
// delete_file
// ===========================================================================
int delete_file(char *filename){
	return unlink(filename);
}

// ===========================================================================
// is_directory
// ===========================================================================
int is_directory(char *name){
	struct stat dirData;
	mode_t mode;
	int result = stat(name, &dirData);
	if(result == -1){
		fprintf(stderr, "Error while retrieving file \"%s\" information.\n", name);
		return -1;
	}
	mode = dirData.st_mode;
	if(S_ISDIR(mode))
		return 1;
	else
		return 0;
}

// ===========================================================================
// get_current_directory
// ===========================================================================
char *get_current_directory(void){
	size_t size = MAXPATHLEN;
	char *buff = (char*) malloc(MAXPATHLEN * sizeof(char));
	if(!buff){
		fprintf(stderr, "Error while allocating memory.\n");
		return NULL;
	}
	if(!getcwd(buff, size)){
		fprintf(stderr, "Error while getting the current working directory.\n");
		return NULL;
	}
	return buff;
}

// ===========================================================================
// concatenate_path
// ===========================================================================
char *concatenate_path(char *prefix, char *suffix){
	int len = strlen(prefix) + strlen(suffix) + 2;
	char *temp = (char *) malloc(sizeof(char) * len);
	if(!temp){
		fprintf(stderr, "Error while allocating memory.\n");
		return NULL;
	}
	if(sprintf(temp, "%s/%s", prefix, suffix) < 0){
		fprintf(stderr, "Error while concatenating the path.\n");
		free(temp);
		return NULL;
	}
	return temp;
}

// ===========================================================================
// print_file_info [DEBUG]
// ===========================================================================
int print_file_info(myFile *file){
	if(!file){
		fprintf(stderr, "Error in print_file_info: file is null.\n");
		return -1;
	}
	printf("name: %s\nsize: %lld\nperms: %s\nlast write: %lld\nisDir: %d\n", file->name, file->size,
				file->perms, file->lastWriteTimestamp, file->isDir);
	return 0;
}
