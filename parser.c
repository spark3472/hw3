<<<<<<< HEAD
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
=======
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
>>>>>>> b814c4d22d3428ebcde7da8bc1eaa94a0fcc1faf

#define TRUE  1;
#define FALSE 0;

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

int a = 0;
char* get_next_token(TOKENIZER *t){
	//if current char is a delimiter, just return it
//else go until next char is a delimiter
//return the substring without white spaces
//returned strings are malloced copies
//return NULL when string ends
  
  char* string;
  int b = 0;
  if(*(t->pos) == '\0'){
    return NULL;
  }
  if (*(t->pos) == '&'||*(t->pos)==';'){
    b++;
    t->pos++;
    string = (char*) malloc((b+1)*sizeof(char));
    memcpy(string, &t->str[a], b);
    a += b;
    printf("%s\n", string);
    return string;
  }

  while(*(t->pos) != '\0'){
    if (*(t->pos) == '&'||*(t->pos) == ';'){
      break;
    }else{
      t->pos++;
      b++;
    }
  }
  string = (char*) malloc((b+1)*sizeof(char));
  memcpy(string, &t->str[a], b);
  a += b;
  printf("%s\n", string);
  return string;
}

/* Gets the starting point of the line
 * @param line
 * @return pointer that points to the start of the line
 */
TOKENIZER init_tokenizer(char* line){
  TOKENIZER t;
  t.str = line;
  t.pos = t.str;
  return t;
}

/* Stores pointers to tokens in global array toks
 * @return Number of tokens
 */
TOKENIZER t;
char* line;
char* shell_prompt = "parser> ";
int parser(){
  int i = 0;int n = 0;a = 0;
  line = readline(shell_prompt);
  //ctrl-d
  if(line == NULL){
    return 0;
  }
  //newline
  if(strcmp(line,"") == 0){
    return 0;
  }
  t = init_tokenizer(line);
  //how many tokens
  while(get_next_token(&t) != NULL){
    n++;
  }
  //allocate pointers to tokens +1 for the ending NULL
  /*toks = (char*) malloc(sizeof(char*)*(n+1));
	//start from beginning again
	t = init_tokenizer(line);
	while ((toks[i++] = get_next_token(t)) != NULL);
	free(line);*/
  return n;
}

int main(){
  while(1){
    int number = parser();
    printf("%d\n", number);
  }
}
