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

#define MAX_TOKENS 10 //used in tokenize path
#define MAX_TOKENS_INC 10
#define MAX_TOKEN_LEN 20
#define MAX_TOKEN_INC 15

/*function prototypes*/
char *__get_perm_string(mode_t mode);

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
		return -2;
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
		//we need the fullpath name
		char *fullPath = concatenate_path(dir, dirEntry->d_name);
		if(!fullPath){
			fprintf(stderr, "Error while getting the full path for %s\n", dirEntry->d_name);
			return -1;
		}
		struct stat fileData;
		mode_t mode;
		int result = stat(fullPath, &fileData);
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
		free(fullPath);
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
	int l1 = strlen(path);
	int l2 = strlen(lastpiece);
	int len = l1 + l2 + 2;
	char *temp = (char *) malloc(sizeof(char) * len);
	if(!temp){
		fprintf(stderr, "Error while allocating memory.\n");
		return NULL;
	}
	int ret = 0;
	if(l1 == 0){
		ret = sprintf(temp, "%s", suffix);
	}else if(prefix[0] == '/'){
		ret = sprintf(temp, "%s%s", prefix, suffix);
	}else{
		ret = sprintf(temp, "%s/%s", prefix, suffix);
	}
	if(ret < 0){
		fprintf(stderr, "Error while concatenating the path.\n");
		free(temp);
		return NULL;
	}
	return temp;
}

// ===========================================================================
// is_absolute_path
// ===========================================================================
int is_absolute_path(char *path){
	return is_prefix(path, "/");
}

// ===========================================================================
// tokenize_path
// ===========================================================================
int tokenize_path(char *path, char ***tokenList, int *tokenListSize){
	if(path == NULL){
		fprintf(stderr, "tokenize_path: path parameter is null\n");
		return -1;
	}

	if(MAX_TOKENS < 1){
		fprintf(stderr, "tokenize_path: MAX_TOKENS value not valid.\n");
		return -1;
	}

	*tokenList = (char **) malloc(sizeof(char *) * MAX_TOKENS);
	if(!(*tokenList)){
		fprintf(stderr, "tokenize_path: error while allocating memory.\n");
		return -1;
	}
	int charIndex = 0;
	int charsRead = 0;
	int maxTokCapacity = MAX_TOKENS;
	int maxCapacity = MAX_TOKEN_LEN;
	int pathLen = strlen(path);

	char *tempToken = malloc(sizeof(char) * maxCapacity);
	if(!tempToken){
		fprintf(stderr, "tokenize_path: error while allocating memory.\n");
		return -1;
	}

	//IMPORTANT: the first token will always be "/"
	if(sprintf(tempToken, "%s", "/") < 0){
		fprintf(stderr, "tokenize_path: Error while creating a token\n");
		return -1;
	}
	(*tokenList)[0] = tempToken;
	*tokenListSize = 1;
	charIndex = 1;

	tempToken = malloc(sizeof(char) * maxCapacity);
	if(!tempToken){
		fprintf(stderr, "tokenize_path: error while allocating memory.\n");
		return -1;
	}

	while(charIndex < pathLen){
		if(charsRead - 1 >= maxCapacity){ //we need to allocate more space for the token name
			maxCapacity += MAX_TOKEN_INC;
			tempToken = realloc(tempToken, maxCapacity);
			if(!tempToken){
				fprintf(stderr, "tokenize_path: error while reallocating memory.\n");
				return -1;
			}
		}
		if(path[charIndex] == '/') { //token end reached
			tempToken[charsRead] = '\0';
			(*tokenList)[*tokenListSize] = tempToken;
			(*tokenListSize)++;
			charsRead = 0;
			charIndex++;
			tempToken = malloc(sizeof(char) * maxCapacity);
			if(!tempToken){
				fprintf(stderr, "tokenize_path: error while allocating memory.\n");
				return -1;
			}
			if(*tokenListSize >= maxTokCapacity){
				maxTokCapacity += MAX_TOKENS_INC;
				*tokenList = realloc(*tokenList, sizeof(char *) * maxTokCapacity);
				if(!(*tokenList)){
					fprintf(stderr, "tokenize_path: error while reallocating memory.\n");
					return -1;
				}
			}
		}else{
			tempToken[charsRead++] = path[charIndex++];
		}
	}
	if(charsRead > 0){
		tempToken[charsRead] = '\0';
		(*tokenList)[*tokenListSize] = tempToken;
		(*tokenListSize)++;
	}else{
		free(tempToken);
	}
	return 0;
}

// ===========================================================================
// get_file_info
// ===========================================================================
int get_file_info(myFileList *fileList, char *filename, myFile **file){
	int i;
	for(i = 0; i < fileList->count; i++){
		if(strcicmp(fileList->list[i].name, filename) == 0){
			*file = &(fileList->list[i]);
			return 1;
		}
	}
	*file = NULL;
	return 0;
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
