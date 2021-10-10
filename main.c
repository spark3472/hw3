/*****************   TO-DO   *****************
    (for more, see checklist on design doc)
-fg - send CONT if needed
-bg (and update job list)
-weird things after & job ends when just hit enter
-valgrind :/
-can't ctrl-c and exit child
-ctrl-d does weird things to shell
-command "ls " treats " " as thing to ls
-set up SIGTSTP (ctrl-z) handler

**********************************************/
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
#include <termios.h>
#include <signal.h>
#include <sys/wait.h>

#define TRUE  1;
#define FALSE 0;

enum status{BACKGROUNDED, FOREGROUNDED, SUSPENDED, TERMINATED};

//signals to block in the shell
sigset_t sigset;
//set with just sigchld
sigset_t sigset_sigchld;

//global array toks
char** toks;
//start of current command section (breaks up & and ; lines)
char** traverser;

//jobList - MODIFY move up if have .h file for joblist
struct JobList;
struct JobList* jobList;


/******JOBLIST******/

/* Process struct modified from https://www.gnu.org/software/libc/manual/html_node/Data-Structures.html */
// Data scructure to store a Process
typedef struct Process {
  struct Process *next;   	/* next process */
  char **argv;            	/* for exec */
  int numArgs;              /* number of arguments */
  pid_t pid;              	/* process ID */
  //need? 
  char suspended;           /* process suspended? */
  int status;             	/* reported status value */
  int jobNum;			          /* the job number */
  struct termios termSettings;  /*Terminal settings*/
} Process;

//holds the JobList struct
struct JobList{
  //most recent job
  Process* head;
  //number of background jobs currently running
  int length;
  //number of jobs added ever
  int jobsTotal;
} JobList;


/* Creates a process
 * @param pid The PID of the process
 * @param status The status of the process
 * @param argv The command that created the process
 * @param numArgs The number of arguments in the command
 * @param jobNum The job number
 * @return The created process
 */
Process* makeProcess(pid_t pid, int status, char** args, int numArgs, int jobNum) {
    // Allocates a new Process sets its data
    Process* newProcess = malloc(sizeof(Process));
    newProcess->next = NULL;
    newProcess->pid = pid;
    newProcess->status = status;
    newProcess->jobNum = jobNum;
    newProcess->numArgs = numArgs;
    //MODIFY will need to free
    newProcess->argv = malloc(1 * sizeof(char**));
    for(int i = 0; i < numArgs; i++) {
      newProcess->argv[i] = malloc(1 * sizeof(char*));
      strcpy(newProcess->argv[i], args[i]);
    }
    
    //figure out later
    //newProcess->termSettings = NULL;
    tcgetattr(STDIN_FILENO, &newProcess->termSettings);

    return newProcess;
}

/* Makes a new job list
 * @return the initiated joblist
 */
struct JobList* makeJobList() {
    struct JobList* newjobList = malloc(1 * sizeof(JobList));
    newjobList->length = 0;
    newjobList->head = NULL;
    newjobList->jobsTotal = 0;
    return newjobList;
}

/* Takes a joblist and process, and puts the process on the front of the jobList
 * @param jobList The joblist to update
 * @param newProcess the process to add
 * @return Success or failure
 */
int push(struct JobList* jobList2, Process* newProcess) {
  sigprocmask(SIG_BLOCK, &sigset_sigchld, NULL);
  // set the `.next` pointer of the new Process to point to the current
  // first Process of the list.
  newProcess->next = jobList->head;
  //updates the head of the joblist
  jobList->head = newProcess;
  jobList->length += 1;
  jobList->jobsTotal += 1;
  sigprocmask(SIG_UNBLOCK, &sigset_sigchld, NULL);
  return EXIT_SUCCESS;
}

/* Retrieves the most recently-added process from the joblist
 * @return The most recent process
 */
Process* getMostRecent(struct JobList* jobList2) {
  return jobList->head;
}

/* Removes the most recently-added process from the joblist (doesn't free it)
 * @return The most recent process
 */
Process* removeMostRecent(struct JobList* jobList2) {
  sigprocmask(SIG_BLOCK, &sigset_sigchld, NULL);
  if(jobList->length <= 0) {
    return NULL;
  }
  Process* mostRecent = jobList->head;
  jobList->head = mostRecent->next;
  jobList->length--;
  sigprocmask(SIG_UNBLOCK, &sigset_sigchld, NULL);
  return mostRecent;
}

/* Remove a process from the job list and frees it
 * @param pid The pid of the process to remove
 * @return jobID of the removed job, or -1 if failure
 */
int removeJob(struct JobList* jobList2, pid_t targetPid) {
  int jobID = -1;
  if(jobList->length <= 0) {
    return jobID;
  }
  Process* ptr = jobList->head;
  if(ptr->pid == targetPid) {
    jobID = ptr->jobNum;
    Process* toRemove = removeMostRecent(jobList);
    free(toRemove);
    return jobID;
  }
  while(ptr->next != NULL) {
    if(ptr->next->pid == targetPid) {
      sigprocmask(SIG_BLOCK, &sigset_sigchld, NULL);
      Process* toRemove = ptr->next;
      jobID = ptr->next->jobNum;
      ptr->next = toRemove->next;
      jobList->length--;
      sigprocmask(SIG_UNBLOCK, &sigset_sigchld, NULL);
      free(toRemove);
      return jobID;
    }
    ptr = ptr->next;
  }

  return jobID;
}

/* Finds a process in the job list using its pid
 * @param pid The pid of the process to find
 * @return Process* of the job, or NULL if failure to fine
 */
Process* findJob(struct JobList* jobList2, pid_t targetPid) {
  if(jobList->length <= 0) {
    return NULL;
  }
  Process* ptr = jobList->head;
  //if first job is the target, return
  if(ptr->pid == targetPid) {
    return ptr;
  }
  //otherwise, iter through the list
  while(ptr->next != NULL) {
    if(ptr->next->pid == targetPid) {
      return ptr->next;
    }
    ptr = ptr->next;
  }

  return NULL;
}

/* Finds a process in the job list using its placement in the list
 * @param int The placement of the process in the list
 * @return Process* of the job, or NULL if failure to find
 */
Process* getJob(struct JobList* jobList2, int jobNum) {
  if(jobList->length <= 0) {
    return NULL;
  }
  Process* ptr = jobList->head;

  if(ptr->jobNum == jobNum) {
    return ptr;
  }
  //otherwise, iter through the list
  while(ptr->next != NULL) {
    if(ptr->next->jobNum == jobNum) {
      return ptr->next;
    }
    ptr = ptr->next;
  }

  return NULL;
  
  /*for (int i = 0; i<jobNum; i++){
    ptr = ptr->next;
  }*/
  //return ptr;
}

/* Prints a joblist
 * @param jobList The joblist to print
 */
//block signal here too
void printList(struct JobList* jobList2){
  printf("Printing job list\n");
  
  if(jobList->head == NULL) {
    printf("No jobs running\n");
    return;
  }

  Process* ptr = jobList->head;
    
  while (ptr) {
    char* statusWord;
    switch(ptr->status) {
      case BACKGROUNDED:
        statusWord = "Running";
        break;
      case FOREGROUNDED:
        statusWord = "Running";
        break;
      case SUSPENDED:
        statusWord = "Stopped";
        break;
      case TERMINATED:
        statusWord = "Terminated";
        break;
    }

    printf("[%d] %s\t\t", ptr->jobNum, statusWord);
    for(int i = 0; i < ptr->numArgs; i++){
        printf(" %s", ptr->argv[i]);
    }
    printf("\n");
    ptr = ptr->next;
  }
}

//frees processes in the joblist
void freeHelper(Process* node) {
    if(node == NULL) {
        return;
    }
    freeHelper(node->next);
    for(int i = 0; i < node->numArgs; i++) {
      free(node->argv[i]);
    }
    free(node->argv);
    free(node);
}

/* Frees the joblist, including calling freeHelper to free individual processes
 * @param jobList the jobList to free
 */
void freeJobList(struct JobList* jobList) {
    freeHelper(jobList->head);
    free(jobList);
}

/******PARSER******/


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
char* shell_prompt = "shell> ";
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
  toks = (char**) malloc(sizeof(char*) * (n+1));
  toks[n] = NULL;
  //start from beginning again
  u = init_tokenizer(line);
  a = 0;
 
  for(int i = 0; i < n; i++){
    char* string = get_next_token(&u);
    toks[i] = (char*)malloc((strlen(string)+1)*sizeof(char));
    strcpy(toks[i], string);
    //maybe don't need
    //toks[i][strlen(string)] = '\0';
    free(string);
  }
  free(line);
  return n;
}


char** getArgs(int start, int end){
  int args = end - start;
  char** currentArguments = (char**) malloc(sizeof(char*)*(args+1));

  currentArguments[args] = NULL;
  int count = 0;
  for (int j = start; j < end; j++){
    currentArguments[count] = malloc(sizeof(char*) * (strlen(toks[j]) + 1));
    strcpy(currentArguments[count], toks[j]);
    //printf("%s\n", currentArguments[count]);
    count++;
  }

  return currentArguments;
}


/*
Edit so stopped job shows as stopped (WIFSIGNALED(status) will return 1, then check info)
Prints when done and not right before shell because we're doing is sychronously
*/

void sigchld_handler(int signo, siginfo_t* info, void* ucontext) {
  //handle sigchld
  //if touching joblist, mask those parts to avoid race condition
  //check if status is terminated	

  pid_t childPid = info->si_pid;
  int childStatus;
  int waitResult = waitpid(childPid, &childStatus, WUNTRACED || WCONTINUED);

  printf("status %d, wifstopped %d, wifexited %d, wifsignaled %d, wifcontinued %d\n", childStatus, WIFSTOPPED(childStatus), WIFEXITED(childStatus), WIFSIGNALED(childStatus), WIFCONTINUED(childStatus));
  if(WIFSIGNALED(childStatus) || WIFEXITED(childStatus)) {
    //printf("child ended.\n");
    Process* job;
    if((job = findJob(jobList, childPid)) != NULL) {
      printf("Stopped because of %d, SIGTSTP is %d", WTERMSIG(childStatus), SIGTSTP);

      if(WIFSIGNALED(childStatus) && SIGTSTP == WTERMSIG(childStatus)) {
        job->status = SUSPENDED;
        job->suspended = 'y';
      } else {
        printf("\n[%d]+ Done\t\t", job->jobNum);
        for(int i = 0; i < job->numArgs; i++){
            printf(" %s", job->argv[i]);
        }
        printf("\n");
        removeJob(jobList, childPid);
      }
    }
  }

	//remove child from job list if so
	//check waitpid with WNOHANG to see if other children terminated (because multiple children could have terminated if SIGCHLD blocked

}
pid_t pid;
pid_t shell_pgid;
struct termios shellTermSettings;
//implement
Process* newProcess;
//so ctrl-z stops a process
void handler_SIGSTP(int signo){
  kill(pid, SIGSTOP);
  char** currentArgs = toks;
  int start = 0;
  int end = 0;
  Process* newProcess = makeProcess(pid, SUSPENDED, currentArgs, (end - start), jobList->jobsTotal+1);
  push(jobList, newProcess);
  
  printf("\n[%d]+ Stopped\t\t", newProcess->jobNum);
  for(int i = 0; i < newProcess->numArgs; i++){
    printf(" %s", newProcess->argv[i]);
  }
  printf("\n");
  //printList(jobList);
  //put shell back in control
  tcsetpgrp(STDIN_FILENO, shell_pgid);
  //Restore the shell’s terminal modes
  tcgetattr(STDIN_FILENO, &newProcess->termSettings);
  tcsetattr(STDIN_FILENO, TCSADRAIN, &shellTermSettings);
}

/* Put job j in the foreground.  If cont is nonzero,
   restore the saved terminal modes and send the process group a
   SIGCONT signal to wake it up before we block.  */

void put_job_in_foreground (Process *job, int cont){
  /* Put the job into the foreground.  */
  tcsetpgrp(STDOUT_FILENO, job->pid);

  /* Send the job a continue signal, if necessary.  */
  if (cont)
    {
      tcsetattr (STDOUT_FILENO, TCSADRAIN, &job->termSettings);
      if (kill(job->pid, SIGCONT) < 0)
        perror ("kill (SIGCONT)");
    }
  //figure out termSettings for job
  //figure out how to send CONT if stopped

  /* Wait for it to report.  */
  waitpid(job->pid, NULL, 0);


  //put shell back in control
  tcsetpgrp(STDIN_FILENO, shell_pgid);

  //Restore the shell’s terminal modes
  //tcgetattr(STDIN_FILENO, &job->termSettings);
  //tcsetattr(STDIN_FILENO, TCSADRAIN, &shellTermSettings);
}



int main(){
  //puts the shell in its own process group
  setpgid(0,0);
  shell_pgid = getpgrp();
  int number;
  char** currentArguments;
  int aftersemi = 0;
  //maybe move into while loop? - does sigaction stuff (creates struct and handler to fill)
  struct sigaction act = {0};
  act.sa_sigaction = &sigchld_handler;
  act.sa_flags = SA_SIGINFO;
  //sets up handler for SIGCHLD
  sigaction(SIGCHLD, &act, NULL);

  //masks signals using sigprocmask() [instead of sigaction()...???]
  sigemptyset(&sigset);
  sigaddset(&sigset, SIGQUIT);

  //catch SIGTSTP instead
  //sigaddset(&sigset, SIGTSTP);
  signal(SIGTSTP, handler_SIGSTP);
  sigaddset(&sigset, SIGTTIN);
  sigaddset(&sigset, SIGTTOU);
  sigaddset(&sigset, SIGINT);
  sigprocmask(SIG_SETMASK, &sigset, NULL);
  //handle SIGINT and SIGTERM? I forget
  //add sigchld to its sigset to use later
  sigemptyset(&sigset_sigchld);
  sigaddset(&sigset_sigchld, SIGCHLD);

  //save terminal settings of shell
  //struct termios shellTermSettings;
  if (tcgetattr(STDIN_FILENO, &shellTermSettings) != 0) {
    perror("tcgetattr() error");
  }

  jobList = makeJobList();

  while(1){
    number = parser();

    //if user hits enter, prompt again
    if(toks == NULL) {
      continue;
    }
    
    //MODIFY for if user just does " ", or sometimes even returning

    //if user types "exit", leave
    if(0 == strcmp(toks[0], "exit")) {
      exit(0);
    }

    //int count = 0;
    //int place = 0;
    int ampOrSemi = 0;
    for (int i = 0; i < number; i++){
      if ((strcmp(toks[i], "&") == 0) || (strcmp(toks[i], ";") == 0)){
        ampOrSemi++;
      }      
    }

    //making an array of the location of the &'s and ;'s
    int countPlaces = 0;
    char ampSemiPlaces[ampOrSemi][2];
    for (int i = 0; i < number; i++){
      if ((strcmp(toks[i], "&") == 0) || (strcmp(toks[i], ";") == 0)){
        ampSemiPlaces[countPlaces][0] = i;
        ampSemiPlaces[countPlaces][1] = toks[i][0];
        countPlaces++;
      }      
    }

    //for now, assuming built-in commands run without & or ; -- change later
    if(0 == strcmp(toks[0], "fg")) {
      //if (number == 2){
        //use of this if statement? - I changed it from 1 to 0
        if (toks[1] == NULL){

          Process *toStart = getMostRecent(jobList);
          if (toStart == NULL){
            printf("There are no jobs\n");
          }else{
            put_job_in_foreground(toStart, 1);
          }
        }else if (strlen(toks[1]) > 0){
          memmove(&toks[1][0], &toks[1][1], strlen(toks[1] - 0));
          int jobNum = atoi(toks[1]);
          printList(jobList);

          //iterates through the list and finds the job
          Process* ptr = getJob(jobList, jobNum);
          if (ptr == NULL){
            printf("Job %d does not exist\n", jobNum);
          }else{
            //follow procedure here: https://www.gnu.org/software/libc/manual/html_node/Foreground-and-Background.html 
            put_job_in_foreground(ptr, 1);
            //figure out termSettings for job
            //figure out how to send CONT if stopped
            removeJob(jobList, ptr->pid);
        
          }
        }else{
          printf("Error: Job Number not specified or too many arguments\n");          
        }
      /*}else if (number == 1){
        //retrieves the most recent process
        Process* recent = getMostRecent(jobList);
        if (recent == NULL){
          printf("No backgrounded job to foreground\n");
        }else{
          //follow procedure here: https://www.gnu.org/software/libc/manual/html_node/Foreground-and-Background.html 
          //same as above, maybe change format so code simpler
          tcsetpgrp(STDIN_FILENO, recent->pid);
          //figure out termSettings for job
          //figure out how to send CONT if stopped
          removeJob(jobList, recent->pid);

          //implement
          waitpid(recent->pid, &recent->status, 0);

          //put shell back in control
          tcsetpgrp(STDIN_FILENO, shell_pgid);

          //Restore the shell’s terminal modes
          tcgetattr(STDIN_FILENO, &recent->termSettings);
          tcsetattr(STDIN_FILENO, TCSADRAIN, &shellTermSettings);
        }*/
      //}
      continue;
    } else if(0 == strcmp(toks[0], "bg")) {
        //resumes job suspended by ctrl-z in the background
        //should maybe get ctrl-z-ing working first lol
        //   - and then find suspended processes in joblist :/
        if (toks[1] == NULL){
          Process *toStart = getMostRecent(jobList);
          if (toStart == NULL){
            printf("There are no jobs\n");
          }else if (kill (toStart->pid, SIGCONT) < 0){
            perror ("kill (SIGCONT)");
          }
        }else if (strlen(toks[1]) > 0){
            memmove(&toks[1][0], &toks[1][1], strlen(toks[1] - 0));
            int jobNum = atoi(toks[1]);
            
            //iterates through the list and finds the job
            Process* ptr = getJob(jobList, jobNum);
            if (ptr == NULL){
              printf("Job %d does not exist\n", jobNum);
            }else{
              if (kill(ptr->pid, SIGCONT) < 0){
                perror("kill (SIGCONT)");
              }
            }
        }else{
          printf("Error: Job Number not specified or too many arguments\n");          
        }
      
      continue;
    } else if(0 == strcmp(toks[0], "jobs")) {
      //print the jobList
      printList(jobList);
      continue;
    } else if(0 == strcmp(toks[0], "kill")) {
      //stop a job using its PID
      //of form: kill [optional: -9] PID 
      if(number == 2) {
        //kills the job nicely
        if(-1 == kill(atoi(toks[1]), SIGTERM)) {
          char errmsg[64];
          snprintf( errmsg, sizeof(errmsg), "kill (%d) ", atoi(toks[1]));
          perror( errmsg );
        }
      } else if(number == 3) {
        if(0 == strcmp(toks[1], "-9")) {
          //kills the job firmly
          if(-1 == kill(atoi(toks[2]), SIGKILL)) {
            char errmsg[64];
            snprintf( errmsg, sizeof(errmsg), "kill (%d) ", atoi(toks[1]));
            perror( errmsg );
          }
        }
      } else if(number == 1) {
        printf("Please enter the PID of the job to kill\n");
      } else {
        printf("Too many arguments. Command is of form \"kill [optional: -9] PID\"\n");
      }
      //job removed from jobList by SIGCHLD handler
      continue;
    }
    
    int tokensExamined = 0;
    int commandsRun = 0;
    while(tokensExamined < number) {
      //background the job?
      int background = 0;
      //start and end of the current command section
      int start = 0;
      int end = number;

      if(commandsRun > 0) {
        //start one after the last & or ;
        start = ampSemiPlaces[commandsRun-1][0] + 1;
      } 

      if(commandsRun < ampOrSemi) {
        end = ampSemiPlaces[commandsRun][0];
        if(ampSemiPlaces[commandsRun][1] == '&') {
          background = 1;
        }
      }

      char** currentArgs = getArgs(start, end);
      //pid_t pid;
      if((pid = fork()) == 0) {
        //puts the child process in its own process group
        setpgid(pid,0);
        //reset signal masks to default
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);

        if( -1 == execvp(currentArgs[0], currentArgs) ){
          //error message for our use
          /*char errmsg[64];
          snprintf( errmsg, sizeof(errmsg), "exec '%s' failed", currentArgs[0] );
          perror( errmsg );*/
          //error message for user use
          printf("%s: command not found\n", currentArgs[0]);
          exit(0);
        }
      } else if (pid > 0) {
        if (!background) {
          waitpid(pid, NULL, 0);
        } else {
          //add to jobList
          Process* newProcess = makeProcess(pid, BACKGROUNDED, currentArgs, (end - start), jobList->jobsTotal+1);
          push(jobList, newProcess);
          //printList(jobList);
          printf("[%d] %d\n", newProcess->jobNum, pid);
          //if job in the background, shell just continues in the foreground
        }
        
      }
      for(int i = 0; i < (end - start); i++) {
        free(currentArgs[i]);
      }
      free(currentArgs);

      commandsRun++;
      tokensExamined = end + 1;
    }

    for(int i = 0; i < number; i++) {
      free(toks[i]);
    }
    free(toks);
  }

  for (int i = 0; i < number; i++){
      free(toks[i]);
  }
  free(toks);
  free(line);

}
