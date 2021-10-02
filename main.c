#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE  1;
#define FALSE 0;

//global array toks
char** toks;
//start of current command section (breaks up & and ; lines)
char** traverser;

/***** Code outline for parser and tokenizer from HW2Feedback slides *****/
//holds a string and the current position in it
typedef struct tokenizer{
  char* str;
  char *pos;
} TOKENIZER;

/* Gets the next delimiter or the string between delimiters
 * @param tokenizer
 * @return Pointer to the string between the delimiters or the delimiter
 */

int a = 0;
char* get_next_token(TOKENIZER *v){
	//if current char is a delimiter, just return it
  //else go until next char is a delimiter
  //return the substring without white spaces
  //returned strings are malloced copies
  //return NULL when string ends
  
  char* string;
  int b = 0;
  if(*(v->pos) == '\0'){
    return NULL;
  }
  
  if (*(v->pos) == ' '){
    a++;
    v->pos++;
  }
  if (*(v->pos) == '&'||*(v->pos)==';'){
    b++;
    v->pos++;
  }else {
    while(*(v->pos) != '\0'){
      if (*(v->pos) == '&'||*(v->pos) == ';'|| *(v->pos)== ' '){
        break;
        }else{
          v->pos++;
          b++;
          }
    }
  }
  string = (char*)malloc((b+1)*sizeof(char));
  //valgrind doesn't like this
  memcpy(string, &v->str[a], b);
  string[b] = '\0';
  a += b;
  return string;
}

/* Gets the starting point of the line
 * @param line
 * @return pointer that points to the start of the line
 */
TOKENIZER init_tokenizer(char* line){
  TOKENIZER s;
  s.str = line;
  s.pos = s.str;
  return s;
}

/* Stores pointers to tokens in global array toks
 * @return Number of tokens
 */
TOKENIZER t;
TOKENIZER u;
char* line;
char* shell_prompt = "parser> ";
int parser(){
  int n = 0;int i = 0;
  //valgrind doesn't like this line??
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
  char* string;
  while((string = get_next_token(&t)) != NULL){
    n++;
  }
  //allocate pointers to tokens +1 for the ending NULL
  toks = (char**) malloc(sizeof(char*)*(n+1));
  //start from beginning again
  u = init_tokenizer(line);
  a = 0;
 
  for(int i = 0; i < n; i++){
    char* string = get_next_token(&u);
    toks[i] = (char*)malloc((strlen(string)+1)*sizeof(char));
    strcpy(toks[i], string);
    free(string);
  }
  free(line);
  return n;
}

int main(){
  
  int tokensRead;
  while(1){
    tokensRead = parser();
    for (int i = 0; i < tokensRead; i++){
      printf("%s\n", toks[i]);
      
      if(0 == strcmp(toks[i], "exit")) {
        exit(0);
      }

    }

    for(int i = 0; i < tokensRead; i++) {
      free(toks[i]);
    }
    free(toks);
  }

}
