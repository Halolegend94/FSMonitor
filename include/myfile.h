#ifndef __MYFILE__
#define __MYFILE__
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

//this struct will contain all the info of a file.
typedef struct _myFile {
	char *name;
	long long size;
	unsigned long long lastWriteTimestamp;
	char *perms;
	int isDir;
} myFile;

//a list of files. This struct is filled by get_directory_content
typedef struct _myFileList {
	int count;
	myFile **list;
} myFileList;

// ==========================================================================
//
// is_prefix
// Description: tells if b is a prefix of a
// Params:
// 	-	a, b two null-terminated strings
// Returns 1 if b is a prefix of a, 0 otherwise
//
// ==========================================================================
int is_prefix(char *a, char *b);

// ===========================================================================
//
// fname_compare
// Description: this functions tells if two paths are equals.
// Params:
// 	-	a, b, two null-terminated paths
// Returns 0 if the paths are equal, a non-zero value otherwise.
//
// ===========================================================================
int fname_compare(char *a, char *b);

// ===========================================================================
//
// get_directory_content
// Description: this function saves in fileList the list of files and folder
// in dir. Each file and folder is represented by a myFile struct that contains
// all the needed information.
// Params:
// 	- dir: a string that represents a directory
// 	- fileList: a pointer to a variable of type myFileList that will receive
//			the information.
// Returns 0 in case of success, -1 in case of error, -2 if the path is no longer available
//
// ===========================================================================
int get_directory_content(char *dir, myFileList *fileList);

// ===========================================================================
//
// get_file_info
// Description: this function gets information about a a file given a list of file
// Params:
// 	- fileList: a pointer to a variable of type myFileList
// 	- filename: the name of the file
//      - file: a pointer to a location where a pointer to the file struct will be saved
// Returns 1 if the file has been found, 0 otherwise
//
// ===========================================================================
int get_file_info(myFileList *fileList, char *filename, myFile **file);

// ===========================================================================
//
// delete_file
// Description: delete a file from disk.
// Params:
//	- filename: the name of the file to be deleted.
// Returns 0 in case of success, -1 otherwise
//
// ===========================================================================
int delete_file(char *filename);

// ===========================================================================
//
// get_current_directory
// Description: get the complete path of the current directory.
// Returns a pointer to the path string. NULL in case of error.
//
// ===========================================================================
char *get_current_directory();

// ===========================================================================
//
// concatenate_path
// Description: given a path prefix and a string suffix, the function will
// return prefix\postfix on windows (prefix/postfix on linux).
// Params:
// 	- prefix, suffix: strings
// Returns NULL in case of error, thw newly formed string in case of success
//
// ===========================================================================
char *concatenate_path(char *prefix, char *suffix);

// ===========================================================================
//
// is_directory
// Description: tests if the path represts a directory.
// Params:
// 	- path: the absolute path string to be tested.
// Returns 0 if it is not a directory, 1 if it is a directory, -1 in case of error.
//
// ===========================================================================
int is_directory(char *path);

// ===========================================================================
//
// is_absolute_path
// Description: tests if the path represts an absolute path.
// Params:
// 	- path: the path to be tested.
// Returns 0 if it is not an absolute path, 1 if it is
//
// ===========================================================================
int is_absolute_path(char *path);

// ===========================================================================
//
// tokenize_path
// Description: given an absolute path, returns a list of the folders that are
// part of the path.
// Params:
// 	-	path: the path strings
// 	-	tokenList: a pointer to a location where the list will be saved
// 	-	tokenListSize: a pointer to an integer which will contain the list size
// Returns 0 in case of success, -1 otherwise
//
// ===========================================================================
int tokenize_path(char *path, char ***tokenList, int *tokenListSize);

// ===========================================================================
//
// get_file_info
// Description: given a list of myFile objcects, this function sets the parameter
// file to point to the myFile that has "filename" as name.
// Returns 1 if the file is in the list, 0 otherwise (file is set at NULL)
//
// ===========================================================================
int get_file_info(myFileList *fileList, char *filename, myFile **file);

// ===========================================================================
//
// print_file_info
// Description: prints the myfile content
// Params:
//	- file: a pointer to a myFile structure to be printed
// Return -1 in cse of error, 0 otherwise
//
// ===========================================================================
int print_file_info(myFile *file);
#endif
