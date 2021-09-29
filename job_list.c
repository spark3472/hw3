/* QUESTIONS:
    - do you have to malloc char** (to make Process)?
    - how to get status (and see if running or stopped)
*/

/*
 * Modified from: https://www.techiedelight.com/linked-list-implementation-part-1 
 */

#include <stdio.h>
#include <stdlib.h>


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
 * @return The updated Joblist
 */
JobList* push(JobList* jobList, Process* newProcess) {
    // set the `.next` pointer of the new Process to point to the current
    // first Process of the list.
    newProcess->next = jobList->head;
    //updates the head of the joblist
    jobList->head = newProcess;

    return jobList;
}

/* Prints a joblist
 * @param jobList The joblist to print
 */
void printList(JobList* jobList){
    Process* ptr = jobList->head;
    while (ptr)
    {
        printf("Job number: %d, command line: ", ptr->jobNum);
        for(int i = 0; i < ptr->numArgs; i++){
            printf("%s ", ptr->argv[i]);
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
    Process* p1 = makeProcess(1, 0, command, 2, 0);
    Process* p2 = makeProcess(2, 0, command, 2, 1);

    push(jobList, p1);
    push(jobList, p2);
 
    // print linked list
    printList(jobList);
    
    freeJobList(jobList);
    free(command);
    return 0;
}