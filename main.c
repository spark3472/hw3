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

/* Process struct modified from https://www.gnu.org/software/libc/manual/html_node/Data-Structures.html */
// Data scructure to store a Process
typedef struct Process {
  struct Process *next;   	/* next process */
  char **argv;            	/* for exec */
  int numArgs;              /* number of arguments */
  pid_t pid;              	/* process ID */
  int status;             	/* reported status value */
  int jobNum;			    /* the job number */
} Process;

//holds the JobList struct
typedef struct {
  //most recent job
  Process* head;
  //number of background jobs currently running
  int length;
} JobList;


/* Creates a process
 * @param pid The PID of the process
 * @param status The status of the process
 * @param argv The command that created the process
 * @param numArgs The number of arguments in the command
 * @param jobNum The job number
 * @return The created process
 */
Process* makeProcess(pid_t pid, int status, char** argv, int numArgs, int jobNum) {
    // Allocates a new Process sets its data
    Process* newProcess = malloc(sizeof(Process));
    newProcess->next = NULL;
    newProcess->pid = pid;
    newProcess->status = status;
    newProcess->jobNum = jobNum;
    newProcess->numArgs = numArgs;
    //Malloc???
    newProcess->argv = argv;

    return newProcess;
}

/* Makes a new job list
 * @return the initiated joblist
 */
JobList* makeJobList() {
    JobList* jobList = malloc(1 * sizeof(JobList));
    jobList->length = 0;
    jobList->head = NULL;
    return jobList;
}

/* Takes a joblist and process, and puts the process on the front of the jobList
 * @param jobList The joblist to update
 * @param newProcess the process to add
 * @return Success or failure
 */
int push(JobList* jobList, Process* newProcess) {
  // set the `.next` pointer of the new Process to point to the current
  // first Process of the list.
  newProcess->next = jobList->head;
  //updates the head of the joblist
  jobList->head = newProcess;
  jobList->length += 1;
  
  return EXIT_SUCCESS;
}

/* Retrieves the most recently-added process from the joblist
 * @return The most recent process
 */
Process* getMostRecent(JobList* jobList) {
  return jobList->head;
}

/* Removes the most recently-added process from the joblist (doesn't free it)
 * @return The most recent process
 */
Process* removeMostRecent(JobList* jobList) {
  if(jobList->length <= 0) {
    return NULL;
  }
  Process* mostRecent = jobList->head;
  jobList->head = mostRecent->next;
  jobList->length--;
  return mostRecent;
}

/* Remove a process from the job list and frees it
 * @param pid The pid of the process to remove
 * @return Success or failure
 */
int removeJob(JobList* jobList, pid_t targetPid) {
  if(jobList->length <= 0) {
    return EXIT_FAILURE;
  }
  Process* ptr = jobList->head;
  if(ptr->pid == targetPid) {
    Process* toRemove = removeMostRecent(jobList);
    free(toRemove);
    return EXIT_SUCCESS;
  }
  while(ptr->next != NULL) {
    if(ptr->next->pid == targetPid) {
      Process* toRemove = ptr->next;
      ptr->next = toRemove->next;
      jobList->length--;
      free(toRemove);
      return EXIT_SUCCESS;
    }
    ptr = ptr->next;
  }

  return EXIT_FAILURE;
}


/* Prints a joblist
 * @param jobList The joblist to print
 */
void printList(JobList* jobList){
    Process* ptr = jobList->head;
    while (ptr) {
        printf("Job number: %d, command line:", ptr->jobNum);
        for(int i = 0; i < ptr->numArgs; i++){
            printf(" %s", ptr->argv[i]);
        }
        printf(", status: %d\n", ptr->status);
        ptr = ptr->next;
    }
}

//frees processes in the joblist
void freeHelper(Process* node) {
    if(node == NULL) {
        return;
    }
    freeHelper(node->next);
    free(node);
}

/* Frees the joblist, including calling freeHelper to free individual processes
 * @param jobList the jobList to free
 */
void freeJobList(JobList* jobList) {
    freeHelper(jobList->head);
    free(jobList);
}
 
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
  int number;
  char** currentArguments;
  while(1){
    number = parser();
    for (int i = 0; i < number; i++){
      if (toks[i] == '&'){
        //bg yay
      }else if (toks[i] == ';'){
         currentArguments = (char**) malloc(sizeof(char*)*(i+1));
         for (int j = 0; j < i; j++){
           //
         }
      }
    }
    for (int i = 0; i < number; i++){
      pid_t pid;
      if((pid = fork()) == 0) {
        execvp(toks[0], toks);
      } else if (pid > 0) {
      wait(NULL);
    }
      printf("%s\n", toks[i]);


    }
  }
  for (int i = 0; i < number; i++){
      free(toks[i]);
  }
  free(toks);
  free(line);
}
