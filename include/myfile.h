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
	long long lastWriteTimestamp;
	char *perms;
	int isDir;
} myFile;

//a list of files. This struct is filled by get_directory_content
typedef struct _myFileList {
	int count;
	myFile *list;
} myFileList;

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
// Returns 0 in case of success, -1 otherwise
//
// ===========================================================================
int get_directory_content(char *dir, myFileList *fileList);

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
// print_file_info
// Description: prints the myfile content
// Params:
//	- file: a pointer to a myFile structure to be printed
// Return -1 in cse of error, 0 otherwise
//
// ===========================================================================
int print_file_info(myFile *file);
#endif
