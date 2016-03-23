#include "../include/myfile.h"
#include "utilities.h"
#include <windows.h>

#define ACL_SIZE 1024
#define PERM_STR_SIZE 1024 //permissions string length
#define PERM_STR_SIZE_INC 512 //if the space allocated initially is not sufficient, we add this amount
/*file types definition*/
#define T_FILE 0
#define T_DIR 1
#define T_OTHER 2
/*definitions used to allocate space for files info*/
#define MAXFILES 20
#define MAXFILESINC 15

#define MAX_TOKENS 10 //used in tokenize path
#define MAX_TOKENS_INC 10
#define MAX_TOKEN_LEN 20
#define MAX_TOKEN_INC 15

/*function prototypes*/
int __get_file_type(LPWIN32_FIND_DATA);
char *__get_account_name(DWORD*, int *ret);
char *__get_mask_string(DWORD);
char *__get_security_acls(char*, int *ret);

// ===========================================================================
// get_directory_content
// ===========================================================================
int get_directory_content(char *dir, myFileList *fileList) {
	HANDLE searchHandle;
	WIN32_FIND_DATA findData;
	int type;
	BOOL guard;
	int currentCapacity = MAXFILES;
	/*we first append the windcard symbol * to dir*/
	int len = strlen(dir);
	char *path = (char *) malloc((len + 3) * sizeof(char));
	if (!path) {
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}
	strcpy(path, dir);
	strcat(path, "\\*");
	//now we allocate a MAXFILE amount of space for fileList
	fileList->list = (myFile *)malloc(MAXFILES * sizeof(myFile));
	if (!fileList->list) {
		fprintf(stderr, "Error while allocating memory.\n");
		free(path);
		return -1;
	}
	fileList->count = 0;

	searchHandle = FindFirstFile(path, &findData);
	if (searchHandle == INVALID_HANDLE_VALUE) {
		fprintf(stderr, "Invalid search handle returned (%s).\n", path);
		free(path);
		free(fileList->list);
		return -2;
	}
	do { //for each file in dir...
		if (fileList->count >= currentCapacity) { //if the list is full, reallocate memory
			currentCapacity += MAXFILESINC;
			fileList->list = (myFile *) realloc(fileList->list, currentCapacity * sizeof(myFile));
			if (!fileList->list) {
				fprintf(stderr, "Error while reallocating memory.\n");
				free(path);
				return -1;
			}
		}
		type = __get_file_type(&findData); //we need to know if it's a dir or a reg file
		if (type == T_OTHER) continue; //skip . and ..
		fileList->list[fileList->count].isDir = type == T_DIR ? TRUE : FALSE;
		/*if (type == T_DIR) //set the type
			fileList->list[fileList->count].isDir = TRUE;
		else
			fileList->list[fileList->count].isDir = FALSE;*/
		//set the last write time
		LARGE_INTEGER large;
		large.LowPart = findData.ftLastWriteTime.dwLowDateTime;
		large.HighPart = findData.ftLastWriteTime.dwHighDateTime;
		fileList->list[fileList->count].lastWriteTimestamp = large.QuadPart;
		//set the size
		large.LowPart = findData.nFileSizeLow;
		large.HighPart = findData.nFileSizeHigh;
		fileList->list[fileList->count].size = large.QuadPart;
		//set the file name
		int namelen = strlen(findData.cFileName);
		fileList->list[fileList->count].name = (char *) malloc((namelen + 1) * sizeof(char));
		if (!fileList->list[fileList->count].name) {
			fprintf(stderr, "Error while allocating memory.\n");
			free(path);
			free(fileList->list);
			return -1;
		}
		strcpy(fileList->list[fileList->count].name, findData.cFileName);
		//find permissions
		char *fullPath = concatenate_path(dir, findData.cFileName);
		if(!fullPath){
			fprintf(stderr, "Error while getting the full path for %s\n", findData.cFileName);
			return -1;
		}
		int ret = 0; //return value
		char *perms = __get_security_acls(fullPath, &ret);
		if(ret == 1){
			//fprintf(stderr, "Error while getting information %s. Defaults are empty string.\n", findData.cFileName);
			perms = malloc(sizeof(char));
			if(!perms){
				fprintf(stderr, "get_directory_content: error while allocating memory.\n");
				return -1;
			}
			perms[0] = '\0';
		}else if(!perms){
			fprintf(stderr, "Error while getting information %s. Terminating.\n", findData.cFileName);
			free(path);
			free(fullPath);
			free(fileList->list);
			free(fileList->list[fileList->count].name);
			return -1;
		}
		free(fullPath);
		fileList->list[fileList->count].perms = perms;
		fileList->count++;
	} while (FindNextFile(searchHandle, &findData));
	free(path);
	if (!FindClose(searchHandle)) {
		fprintf(stderr, "Error while closing the search handle.\n");
		free(fileList->list);
		free(fileList->list[fileList->count].name);
		free(fileList->list[fileList->count].perms);
		return -1;
	}
	return 0;
}

// ==========================================================================
// fname_compare
// ==========================================================================
int fname_compare(char *a, char *b){
	int l1 = strlen(a);
	int l2 = strlen(b);
	if (l1 != l2) return 1;
	for (;; a++, b++) {
		int d = tolower(*a) - tolower(*b);
		if (d != 0 ||  *a == '\0')
			return d;
	}
}

// ==========================================================================
// is_prefix
// ==========================================================================
int is_prefix(char *a, char *b){
   int lenA = strlen(a);
   int lenB = strlen(b);
   if(lenA >= lenB && strcincmp(a, b, lenB) == 0) return 1;
   return 0;
}

// ==========================================================================
// strcincmp
// ==========================================================================
int strcincmp(char *a, char *b, int len){
	int i;
	for (i = 0; i < len; a++, b++, i++) {
		int d = tolower(*a) - tolower(*b);
		if (d != 0 || *a == '\0') return d;
	}
   return 0;
}


// ===========================================================================
// concatenate_path
// ===========================================================================
char *concatenate_path(char *path, char *lastpiece){
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
		ret = sprintf(temp, "%s", lastpiece);
	}else if(path[l1-1] == '\\'){
		ret = sprintf(temp, "%s%s", path, lastpiece);
	}else{
		ret = sprintf(temp, "%s\\%s", path, lastpiece);
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
	if(strlen(path) < 3) return 0;
	if(isalpha(path[0]) && (path[1] == ':') && (path[2] == '\\')) return 1;
	return 0;
}

// ===========================================================================
// __get_file_type [PRIVATE FUNCTION]
// ===========================================================================
int __get_file_type(LPWIN32_FIND_DATA pFileData) {
	DWORD type = T_OTHER;
	if(pFileData->dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
					(pFileData->dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ||
							(pFileData->dwFileAttributes & FILE_ATTRIBUTE_DEVICE) ||
							  (pFileData->dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) ||
						      (pFileData->dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)) return T_OTHER;
	BOOL isDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
	if (isDir) {
		if (strcmp(pFileData->cFileName, ".") == 0 || strcmp(pFileData->cFileName, "..") == 0)
			type = T_OTHER;
		else
			type = T_DIR;
	}else{
			type = T_FILE;
	}
	return type;
}

// ===========================================================================
// __get_account_name [PRIVATE FUNCTION]
// ===========================================================================
 char *__get_account_name(DWORD *sid, int *ret){
	 DWORD size = 0;
	 DWORD domsize = 0;
	 SID_NAME_USE use;
	 *ret = 0; //return value
	 LookupAccountSid(NULL, sid, NULL, &size, NULL, &domsize, &use);
	 char *name = malloc(size * sizeof(char));
	 if(!name){
		 fprintf(stderr, "Error while allocating memory.\n");
		 return NULL;
	 }
	 if(!LookupAccountSid(NULL, sid, name, &size, NULL, &domsize, &use)){
		 //fprintf(stderr, "%s\n", GetLastErrorAsString());
		 free(name);
		 *ret = 1;
		 return NULL;
	 }
	 return name;
 }

// ===========================================================================
// __get_mask_string [PRIVATE FUNCTION]
// ===========================================================================
char *__get_mask_string(DWORD mask){
	char *maskString = malloc(sizeof(char) * 4);
	if(!maskString){
		fprintf(stderr, "Error while allocating memory.\n");
		return NULL;
	}
	int ret = 0;
	int permread = 0;
	if(mask & FILE_GENERIC_READ){
		ret = sprintf(maskString, "%c", 'r');
		if(ret < 0) {
			fprintf(stderr, "Error while getting the mask bit.\n");
			free(maskString);
			return NULL;
		}
		permread++;
	}
	if(mask & FILE_GENERIC_WRITE){
		ret = sprintf(maskString + permread, "%c", 'w');
		if(ret < 0) {
			fprintf(stderr, "Error while getting the mask bit.\n");
			free(maskString);
			return NULL;
		}
		permread++;
	}
	if(mask & FILE_GENERIC_EXECUTE){
		ret = sprintf(maskString + permread, "%c", 'x');
		if(ret < 0) {
			fprintf(stderr, "Error while getting the mask bit.\n");
			free(maskString);
			return NULL;
		}
		permread++;
	}
	maskString[permread] = '\0';
	return maskString;
}

// ===========================================================================
// __get_security_acls [PRIVATE FUNCTION]
// ===========================================================================
char *__get_security_acls(char *filename, int *ret){
	PSECURITY_DESCRIPTOR pSec = NULL;
	DWORD lenNeeded, iAce;
	BOOL fileDacl, aclDefaulted;
	DWORD dacl[ACL_SIZE/sizeof(DWORD)];
	PACL pAcl = (PACL) &dacl;
	ACL_SIZE_INFORMATION aclSizeInfo;
	PACCESS_ALLOWED_ACE pAce;
	BYTE aclType;

	/*get the required size for the security descriptor*/
	GetFileSecurity(filename, DACL_SECURITY_INFORMATION, NULL, 0, &lenNeeded);
	pSec = (PSECURITY_DESCRIPTOR) malloc(lenNeeded);
	if(!pSec){
		fprintf(stderr, "Error while allocating memory.\n");
		return NULL;
	}
	/*get the info*/
	if(!GetFileSecurity(filename, DACL_SECURITY_INFORMATION, pSec, lenNeeded, &lenNeeded)){
		fprintf(stderr, "An error occurred while getting security information.\n");
		free(pSec);
		*ret = 1;
		return NULL;
	}

	if(!GetSecurityDescriptorDacl(pSec, &fileDacl, &pAcl, &aclDefaulted)){
		fprintf(stderr, "An error occurred while getting security descriptor Dacl.\n");
		free(pSec);
		return NULL;
	}
	if(!GetAclInformation(pAcl, &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation)){
		fprintf(stderr, "An error occurred while getting the acl size information.\n");
		free(pSec);
		return NULL;
	}
	/*get each ace*/
	char *tempPerm = malloc(sizeof(char) * PERM_STR_SIZE);
	if(!tempPerm){
		fprintf(stderr, "Error while allocating memory.\n");
		free(pSec);
		return NULL;
	}
	tempPerm[0] = '\0';
	int currentCapacity = PERM_STR_SIZE;
	int bytesUsed = 0;
	for(iAce = 0; iAce < aclSizeInfo.AceCount; iAce++){
		if(!GetAce(pAcl, iAce, &pAce)){
			fprintf(stderr, "An error occurred while getting the ACE.\n");
			free(pSec);
			free(tempPerm);
			return NULL;
		}
		int byteReq = 0;
		aclType = pAce->Header.AceType;
		char *perms = __get_mask_string(pAce->Mask);
		if(!perms) {
			free(pSec);
			free(tempPerm);
			return NULL;
		}
		char *sidname = __get_account_name(&pAce->SidStart, ret);
		if(!sidname){
			free(pSec);
			free(perms);
			free(tempPerm);
			return NULL;
		}
		char modeace[10];
		if(aclType == ACCESS_ALLOWED_ACE_TYPE){
			sprintf(modeace, "%s", "ALLOW");
		}else if(aclType == ACCESS_DENIED_ACE_TYPE){
			sprintf(modeace, "%s", "DENY");
		}
		/*join the information collected in a unique string*/
		byteReq = strlen(modeace) + strlen(sidname) + strlen(perms) + 5;
		char *stringace = malloc(sizeof(char) * byteReq);
		if(!stringace){
			fprintf(stderr, "Error while allocating memory.\n");
			free(pSec);
			free(perms);
			free(sidname);
			free(tempPerm);
			return NULL;
		}
		if(sprintf(stringace, "%s %s %s; ", modeace, sidname, perms) < 0){
			fprintf(stderr, "Error during the generation of the ace string.\n");
			free(pSec);
			free(perms);
			free(sidname);
			free(tempPerm);
			free(stringace);
			return NULL;
		}
		if(bytesUsed + byteReq >= currentCapacity){
			currentCapacity = bytesUsed + byteReq + PERM_STR_SIZE_INC;
			tempPerm = realloc(tempPerm, sizeof(char) * currentCapacity);
			if(!tempPerm){
				fprintf(stderr, "Error while reallocating memory.\n");
				free(pSec);
				free(perms);
				free(sidname);
				free(stringace);
				return NULL;
			}
		}
		strcat(tempPerm, stringace);
		bytesUsed += byteReq;
		free(sidname);
		free(perms);
		free(stringace);
	}
	free(pSec);
	return tempPerm;
}

// ===========================================================================
// get_current_directory
// ===========================================================================
char *get_current_directory(void) {
	LPTSTR path = NULL;
	int len = 0;
	len = GetCurrentDirectory(len, path); //get the size required
	if (len == 0) {
		fprintf(stderr, "Error while getting the current directory.\n");
		return NULL;
	}
	path = (LPTSTR)malloc(len * sizeof(char));
	if (!path) {
		fprintf(stderr, "Error while allocating memory main.\n");
		return NULL;
	}
	len = GetCurrentDirectory(len, path);
	if (len == 0) {
		fprintf(stderr, "Error while getting the current directory.\n");
		free(path);
		return NULL;
	}
	return path;
}

// ===========================================================================
// delete_file
// ===========================================================================
int delete_file(char *filename) {
	if (DeleteFile(filename)) {
		return 0;
	}else{
		return -1;
	}
}

// ===========================================================================
// is_directory
// ===========================================================================
int is_directory(char *dir){
	DWORD att;
	att = GetFileAttributes(dir);
	return (att != INVALID_FILE_ATTRIBUTES && att & FILE_ATTRIBUTE_DIRECTORY);
}

// ===========================================================================
// tokenize_path
// ===========================================================================
int tokenize_path(char *path, char ***tokenList, int *tokenListSize){
	if(path == NULL){
		fprintf(stderr, "tokenize_path: path parameter is null\n");
		return -1;
	}

	if(MAX_TOKENS < 1 || MAX_TOKEN_LEN < 3){
		fprintf(stderr, "tokenize_path: DEFINITIONS values not valid.\n");
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

	//IMPORTANT: the first token will always be the disk name (eg C:\)
	if(sprintf(tempToken, "%c:\\", path[0]) < 0){
		fprintf(stderr, "tokenize_path: Error while creating a token\n");
		return -1;
	}
	(*tokenList)[0] = tempToken;
	*tokenListSize = 1;
	charIndex = 3;

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
		if(path[charIndex] == '\\' || path[charIndex] == '/') { //token end reached
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
		if(fname_compare(fileList->list[i].name, filename) == 0){
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
