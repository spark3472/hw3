/* QUESTIONS:
    - do you have to malloc char** (to make Process)?
    - how to get status (and see if running or stopped)
    - add failure condition for push? (cases where it doesn't work?)
    - is semaphore mutual exclusion implemented correctly?
    - add mutual exclusion for printing too?
*/

/*
 * Modified from: https://www.techiedelight.com/linked-list-implementation-part-1 
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


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
  pthread_mutex_t mutex;
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
    pthread_mutex_init(&(jobList->mutex), NULL);
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
  pthread_mutex_lock(&(jobList->mutex));
  newProcess->next = jobList->head;
  //updates the head of the joblist
  jobList->head = newProcess;
  jobList->length += 1;
  pthread_mutex_unlock(&(jobList->mutex));
  
  return EXIT_SUCCESS;
}

/* Retrieves the most recently-added process from the joblist
 * @return The most recent process
 */
Process* getMostRecent(JobList* jobList) {
  return jobList->head;
}

/* Removes the most recently-added process from the joblist
 * @return The most recent process
 */
Process* removeMostRecent(JobList* jobList) {
  pthread_mutex_lock(&(jobList->mutex));
  if(jobList->length <= 0) {
    return NULL;
  }
  Process* mostRecent = jobList->head;
  jobList->head = mostRecent->next;
  jobList->length--;
  pthread_mutex_unlock(&(jobList->mutex));
  return mostRecent;
}

/* Remove a process from the job list
 * @param pid The pid of the process to remove
 * @return Success or failure
 */
int removeJob(JobList* jobList, pid_t targetPid) {
  pthread_mutex_lock(&(jobList->mutex));
  if(jobList->length <= 0) {
    return EXIT_FAILURE;
  }
  Process* ptr = jobList->head;
  if(ptr->pid == targetPid) {
    removeMostRecent(jobList);
    return EXIT_SUCCESS;
  }
  while(ptr->next != NULL) {
    if(ptr->next->pid == targetPid) {
      ptr->next = ptr->next->next;
      jobList->length--;
      return EXIT_SUCCESS;
    }
    ptr = ptr->next;
  }

  pthread_mutex_unlock(&(jobList->mutex));
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
 

int main(void) {
    // Makes a new job list
    JobList* jobList = makeJobList();
    //makes a fake command
    char** command = malloc(2*sizeof(char*));
    command[0] = "cat";
    command[1] = "test.txt";
    // Makes some test processes
    Process* p1 = makeProcess(1, 0, command, 2, 1);
    Process* p2 = makeProcess(2, 0, command, 2, 2);
    Process* p3 = makeProcess(3, 0, command, 2, 3);

    push(jobList, p1);
    push(jobList, p2);
 
    // print linked list
    printList(jobList);

    printf("Jobnum of most recent: %d\n", getMostRecent(jobList)->jobNum);
    removeMostRecent(jobList);

    printf("\nRemoved first item\n");
    printList(jobList);

    push(jobList, p3);

    printf("\nAdded p3:\n");
    printList(jobList);

    printf("\nTrying to remove job 3: %d\n", removeJob(jobList, 3)==EXIT_SUCCESS);
    printList(jobList);
    
    freeJobList(jobList);
    free(command);
    return 0;
}
