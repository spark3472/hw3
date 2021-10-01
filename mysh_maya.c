#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>

//the delimiters to parse the command line with
#define DELIMITERS " \n"
//the maximum length of the command line
#define MAXLENGTH 500
//the size of the string that holds the commands
#define SIZE 50
//total number of arguments allowed
#define NUMARGS 10

/* Free a 2D array of character pointers
 * @param array The array to free
 * @param size The size of the array
 */
void freeArray(char** array, int size) {
  for(int i = 0; i < size; i++) {
    free(array[i]);
  }
  free(array);
}

int main(int argc, char *argv[]) {

  printf("Welcome to mysh\n");

  char buffer[MAXLENGTH + 1];
  char command[SIZE + 1];

  //takes in arguments
  char **arguments = malloc(NUMARGS * sizeof(char*));
  for(int i = 0; i < NUMARGS; i++) {
    arguments[i] = malloc(SIZE * sizeof(char*));
  }

  //will hold customized array of arguments to pass to execvp
  char** realArguments;

  //holds the current number of arguments
  int count;

  //if the command was successfully executed
  int completedAction;
  
  while(1) {
    printf("> ");
    fgets(buffer, MAXLENGTH, stdin);
    
    //if user types "exit", leave
    if(0 == strcmp(buffer, "exit\n")) {
      exit(0);
    }

    //if nothing entered, ask for input again
    if(buffer[0] == '\n') {
      continue;
    }

    //doesn't read new input ....
    if(strchr(buffer, '\n') == NULL) {
      printf("The line is too long. Try something shorter\n");
      completedAction = 0;
      //buffer[0] = '\0';
      char c;
      while((c = getchar()) != '\n');
      buffer[0] = '\0';
      continue;
    }

    //resets count and completed action
    count = 0;
    completedAction = 1;
    
    //reads the initial command
    char arg[SIZE + 1];
    strcpy(arg, strtok(buffer, DELIMITERS));
    strcpy(command, arg);
    char *token = &arg[0];
    
    //reads all other arguments given
    while(token != NULL) {
      strcpy(arguments[count], token);
      count++;

      if(count >= NUMARGS) {
        printf("Too many arguments. Try again\n");
        completedAction = 0;
        break;
      }
      
      //reads next section
      token = strtok(NULL, DELIMITERS);
    }

    //if too many arguments given, skip loop and move to the next line
    if(!completedAction) {
      continue;
    }

    //sets action to completed if it's reached this point with no problems
    completedAction = 1;

    //creates memory for small arguments
    char** realArguments = malloc(count * sizeof(char*));
    for(int i = 0; i < count; i++) {
      realArguments[i] = malloc(SIZE * sizeof(char*));
    }

    //copies relevant arguments into them
    for(int i = 0; i < count; i++) {
      strcpy(realArguments[i], arguments[i]);
    }

    //forks and calls the relevant command
    pid_t pid;
    if((pid = fork()) == 0) {
      execvp(command, realArguments);
    } else if (pid > 0) {
      wait(NULL);
    }
    
  }

  //frees the memory from the two arrays
  freeArray(arguments, NUMARGS);
  //if action not completed so realArguments not malloced, don't free
  if(completedAction) {
    freeArray(realArguments, count);
  }
  
  printf("\n");
}