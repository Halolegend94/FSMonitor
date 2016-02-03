#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define LEXEME_LEN 20
#define LEXEME_LEN_INC 10
#define BUFF_SIZE 2
/*token names*/
#define WORD 0
#define NUMBER 1
#define SEPARATOR 2
#define ENDLINE 3

#define NUM_SETTINGS 10
#define NUM_SETTINGS_INC 10

typedef struct _token{
	int name;
	char *value;
} token;

typedef struct _setting{
	char *name;
	char *value;
}setting;

typedef struct _settingsList {
	int count;
	setting *list;
} settingsList;

//=================================================================
//SCANNING [LEXER]
//=================================================================
/*global variables*/
char __current = ' ';
int __isStart = 1;
int __streamEnded = 0;
token __curr_token;
FILE *__pFileSettings;
int __num_lines = 0;

void __remove_whitespaces(FILE *__pFileSettings){
	for( ; ; __current = fgetc(__pFileSettings)){
		if(__current != ' ' && __current != '\t' && __current != '\r') break;
	}
}

int __check_space(int charsRead, int *currentMaxLen, char **pointer){
	if(charsRead + 2 >= *currentMaxLen){ //if there is not sufficient space
		*currentMaxLen = *currentMaxLen + LEXEME_LEN_INC;
		*pointer = (char *) realloc(*pointer, *currentMaxLen * sizeof(char));
		if(!(*pointer)){
			fprintf(stderr, "Error while reallocating memory.\n");
			return -1;
		}
	}
	return 1;
}

int __get_next_token(){
	__remove_whitespaces(__pFileSettings);
	int charsRead = 0;
	int currentMaxLen = LEXEME_LEN;
	char *temp = (char *) malloc(currentMaxLen * sizeof(char));
	if(!temp){
		fprintf(stderr, "Error while allocating memory.\n");
		return -1;
	}
	while(__current != EOF){
		if(__check_space(charsRead + 2, &currentMaxLen, &temp) == -1) return -1;
		if(__current == ':'){ //SEPARATOR
			__curr_token.name = SEPARATOR;
			__curr_token.value = ":";
			__current = fgetc(__pFileSettings);
			return 0;
		}else if(__current == '\n'){ //ENDLINE
			__curr_token.name = ENDLINE;
			__curr_token.value = "\n";
			__current = fgetc(__pFileSettings);
			return 0;
		}else if(isalpha(__current)){ //read a WORD
			do{
				temp[charsRead++] = __current;
				if(__check_space(charsRead + 2, &currentMaxLen, &temp) == -1) return -1;
				__current = fgetc(__pFileSettings);
			}while(isalnum(__current));
			temp[charsRead] = '\0';
		__curr_token.name = WORD;
			__curr_token.value = temp;
			return 0;
		}else if(isdigit(__current)){ //read a NUMBER
			do{
				temp[charsRead++] = __current;
				if(__check_space(charsRead + 2, &currentMaxLen, &temp) == -1) return -1; //if there is not sufficient space
				__current = fgetc(__pFileSettings);
			}while(isdigit(__current));
			temp[charsRead] = '\0';
		__curr_token.name = NUMBER;
			__curr_token.value = temp;
			return 0;
		}else if(__current == '#'){ //eliminiamo i commenti
			do{
				__current = fgetc(__pFileSettings);
			}while(__current != '\n' && __current != EOF);
		}else{
			fprintf(stderr, "Lexical error while parsing the settings.\n");
			return -1;
		}
	}
	return 1;
}

//================================================================
//PARSING [PARSER]
//================================================================

int __line(setting*);

int parse_settings(char *filename, settingsList *settings){
	__pFileSettings = fopen(filename, "r");
	if(!__pFileSettings){
		fprintf(stderr, "Error while opening the settings file.\n");
		return -1;
	}
	/*now we allocate space for the settings list*/
	settings->list = (setting*) malloc(NUM_SETTINGS * sizeof(setting));
	if(!settings->list){
		fprintf(stderr, "Error while allocating memory\n");
		return -1;
	}
	int currentCapacity = NUM_SETTINGS;
	settings->count = 0;

	__streamEnded = __get_next_token();
	if(	__streamEnded == -1)	return -1; //an error occurred.
	while(!__streamEnded){ //while there are tokens available..
		__num_lines++;
		if(settings->count >= currentCapacity){
			currentCapacity+= NUM_SETTINGS_INC;
			settings->list = realloc(settings->list, currentCapacity * sizeof(setting));
			if(!settings->list){
				fprintf(stderr, "Error while reallocating memory\n");
				return -1;
			}
		}
		setting set;
		int rval = __line(&set);
		if(rval == -1){
			return -1;
		}else if(rval == 1){ //empty line, skip
			continue;
		}else{
			settings->list[settings->count++] = set;
		}
	}
	fclose(__pFileSettings);
	return 0;
}

int __line(setting *set){
	if(__curr_token.name == ENDLINE){
		__streamEnded = __get_next_token();
		return 1;
	}else{
		if(__curr_token.name != WORD){
			fprintf(stderr, "Syntax error while parsing the settings file (line %d): a setting " \
			"name is missing.\n", __num_lines);
			return -1;
		}
		set->name = __curr_token.value;
		__streamEnded = __get_next_token();
		if(__streamEnded == 1){
			fprintf(stderr, "Syntax error while parsing the settings file (line %d): line not " \
			"ended.\n", __num_lines);
			return -1;
		}
		if(__curr_token.name != SEPARATOR){
			fprintf(stderr, "Syntax error while parsing the settings file (line %d): a separator " \
			"\":\" is missing.\n", __num_lines);
			return -1;
		}
		__streamEnded = __get_next_token();
		if(__streamEnded == 1){
			fprintf(stderr, "Syntax error while parsing the settings file (line %d): line not "\
			"finished.\n", __num_lines);
			return -1;
		}
		if(__curr_token.name != WORD && __curr_token.name != NUMBER){
			fprintf(stderr, "Syntax error while parsing the setting file (line %d): value is " \
			"missing.\n", __num_lines);
			return -1;
		}
		set->value = __curr_token.value;
		__streamEnded = __get_next_token();
		if(__streamEnded == 1){
			fprintf(stderr, "Syntax error while parsing the setting (line %d): line not "\
			"finished.\n", __num_lines);
			return -1;
		}
		if(__curr_token.name != ENDLINE){
			fprintf(stderr, "Syntax error while parsing the setting file (line %d): a newline \"\\n\" "\
			"is missing.\n", __num_lines);
			return -1;
		}
		__streamEnded = __get_next_token();
		return 0;
	}
}
