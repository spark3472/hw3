#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

#define TRUE  1;
#define FALSE 0;
#define PROMPT “MYSH>”

/***** Code outline for parser and tokenizer from HW2Feedback slides *****/
//holds a string and the current position in it
typedef struct tokenizer{
  char* str;
  char *pos;
} TOKENIZER;
//global array toks
char** toks;
/* Gets the next delimiter or the string between delimiters
 * @param tokenizer
 * @return Pointer to the string between the delimiters or the delimiter
 */
char *get_next_token(TOKENIZER *tokenizer){
	//if current char is a delimiter, just return it
//else go until next char is a delimiter
//return the substring without white spaces
//returned strings are malloced copies
//return NULL when string ends
}

/* Gets the starting point of the line
 * @param line
 * @return pointer that points to the start of the line
 */
char* init_tokenizer(char* line){
  char*p = line;
  return p;
}

/* Stores pointers to tokens in global array toks
 * @return Number of tokens
 */
char* t;
int n = 0;
char* line;
int parser(){
	int i = 0; int n = 0;
	line = readline(PROMPT);
	//ctrl-d
	if(line == NULL)
	  return 0;
	//newline
	if(strcmp(line, “”) == 0)
	  return 0;
	t = init_tokenizer(line);
	//how many tokens
	while(get_next_token(t) != NULL)
	  n++; 
	//allocate pointers to tokens +1 for the ending NULL
	toks = (char*) malloc(sizeof(char*)*(n+1));
	//start from beginning again
	t = init_tokenizer(line);
	while ((toks[i++] = get_next_token(t)) != NULL);
	return n;
}
