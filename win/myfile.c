#include "../include/myfile.h"
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

/*function prototypes*/
int get_directory_content(char*, myFileList*);
char *concatenate_path(char*, char*);
int __get_file_type(LPWIN32_FIND_DATA);
char *__get_account_name(DWORD*);
char *__get_mask_string(DWORD);
char *__get_security_acls(char*);
char *get_current_directory(void);
int delete_file(char*);
int is_directory(char*);

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
		fprintf(stderr, "Invalid search handle returned.\n");
		free(path);
		free(fileList->list);
		return -1;
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
		char *perms = __get_security_acls(fullPath);
		if(!perms){
			fprintf(stderr, "Error while getting the permitions for %s\n.", findData.cFileName);
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


// ===========================================================================
// concatenate_path
// ===========================================================================
char *concatenate_path(char *path, char *lastpiece){
	int len = strlen(path) + strlen(lastpiece) + 2;
	char *temp = (char *) malloc(sizeof(char) * len);
	if(!temp){
		fprintf(stderr, "Error while allocating memory.\n");
		return NULL;
	}
	if(sprintf(temp, "%s\\%s", path, lastpiece) < 0){
		fprintf(stderr, "Error while concatenating the path.\n");
		free(temp);
		return NULL;
	}
	return temp;
}

// ===========================================================================
// __get_file_type [PRIVATE FUNCTION]
// ===========================================================================
int __get_file_type(LPWIN32_FIND_DATA pFileData) {
	DWORD type = T_FILE;
	BOOL isDir = (pFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
	if (isDir) {
		if (strcmp(pFileData->cFileName, ".") == 0 || strcmp(pFileData->cFileName, "..") == 0)
			type = T_OTHER;
		else
			type = T_DIR;
	}
	return type;
}

// ===========================================================================
// __get_account_name [PRIVATE FUNCTION]
// ===========================================================================
 char *__get_account_name(DWORD *sid){
	 DWORD size = 0;
	 DWORD domsize = 0;
	 SID_NAME_USE use;
	 LookupAccountSid(NULL, sid, NULL, &size, NULL, &domsize, &use);
	 char *name = malloc(size * sizeof(char));
	 if(!name){
		 fprintf(stderr, "Error while allocating memory.\n");
		 return NULL;
	 }
	 if(!LookupAccountSid(NULL, sid, name, &size, NULL, &domsize, &use)){
		 fprintf(stderr, "Error while getting the account name size.\n");
		 free(name);
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
char *__get_security_acls(char *filename){
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
		char *sidname = __get_account_name(&pAce->SidStart);
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
	DWORD att = GetFileAttributes(dir);
	if(att == INVALID_FILE_ATTRIBUTES){
		fprintf(stderr, "Error while getting the file attributes.\n");
		return -1;
	}
	if(att & FILE_ATTRIBUTE_DIRECTORY)
		return 1;
	else
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
