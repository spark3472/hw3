# README
<h1>Maya Johnson and Ashley Park</h1>
<h2>To Compile and Run</h2>
make <br  >
./main

<h2>Introduction</h2>
This shell does most of the work of a common terminal shell by using execvp as well as calling functions that were implemented to act as built-in commands (fg, bg, jobs, and kill). Each new line on the shell is parsed into individual words and delimiters and read into a global char* array. We use a linked list to keep track of backgrounded or stopped jobs. The job list does not include jobs running in the foreground or the shell itself. For each job, we're storing the PID, job number, command line arguments that started the job, the number of arguments in that command, the status value, and the termios settings of the job. The shell ignores SIGQUIT, SIGTTIN, SIGTTOU, SIGSTOP, and SIGINT, and handles SIGCHLD and SIGTSTP. A new background job will be added to the joblist and continues to run while the shell is put back in the foreground, and a new foreground job will be run to completion (unless it is ctrl-z'd), at which point the shell will take back over. When a job is stopped, it send a SIGCHLD to the shell which updates the joblist as necessary. We use signal masking around critical regions in the joblists to handle concurrency issues.

<h3>Data Structures Implemented</h3>
The two primary data structures used is a linked list and an array. A linked list is used to keep track of all of the backgrounded jobs and can be manipulated by calling functions that add to the list and delete it. The basis of the linked list we decided to implement came from https://www.gnu.org/software/libc/manual/html_node/Data-Structures.html. It uses two structs, a Process struct that holds the pointer to the next Process, the commands to be put into execvp, the number of commands to be put into execvp, the pid of the Process, whether or not the Process is suspended, the status of the Process, the job number of the Process, and another struct inside the Process struct that holds the terminal settings of the Process. The Linked List struct holds head of the pointers to the processes, the number of jobs currently running in the background, and the total number of jobs added since starting the shell. A 2D array was used to store the user-inputed commands and can be manipulated by the parser which mallocs space according to the number of commands and the length of each command. 

<h3>Parser</h3>
For the parser, we implemented pseudocode given in class found in hw2feedback. It first sifts through the commands, taking out spaces as well as delineating ampersands and semicolons. Then, it finds the total number of commands/arguments entered, mallocing the 2D array to hold that many spaces. Then, each "row" of the 2d array is malloced to hold enough space to fit each command/argument. 

<h3>Signal Handling</h3>
The signals that the shell ignores are SIGQUIT, SIGTTIN, and SIGTTOU. However, these signals are reset for the child processes. The shell handles SIGCHLD in order to determine when a child process has finished running so that it can be removed from the job list. The shell handles SIGINT and SIGTSTP by sending it to a signal handler which sends the received signal to the child process and updates the job list if necessary. The shell deals with concurrency issues resulting from this by masking SIGCHLD around critical regions, namely removing or adding to the job list by using sigprogmask. The shell handles SIGTSTP to background a job running in the foreground, at which point the job is stopped and added to the joblist.

<h3>Command Execution</h3>
Command execution by user input is done in the same way as a regular shell by forking and execvp-ing commands such as cat, touch, mk, etc. The built-in commands fg, bg, jobs, and kill are also implemented by checking for those strings in the command line executing them appropriately. Built-in commands are not supported with & or ;.

<h2>Features</h2>

<h3>Fully Implemented</h3>
<h4>JobList</h4>
Functions for JobList: makeJobList(), push(), getMostRecent(), removeMostRecent(), removeJob(), findJob(), getJob(), printList(), freeJobList()
<h4>struct Process</h4>
Functions for struct Process: makeProcess() and freeProcess()
<h4>Parser</h4>
struct tokenizer <br />
Functions for parser: parser(), init_tokenizer(), get_next_token(), getArgs()
<h4>Signal Handlers</h4>
Functions for signal handlers: sigchld_handler() and handler_toChild() <br />
Used sigset and sigaction to block a set of signals.
<h4>Child Terminated</h4>
Function for Child Terminated: sigchld_handler() in which it handles a SIGCHLD signal sent from a finished child process to the shell (parent process).
<h4>CTRL-Z Suspension</h4>
Function for CTRL-Z Suspension: handler_toChild() in which it catches a CTRL-Z SIGTSTP signal sent to the shell and sends it to the child process and puts the child process into the joblist.
<h4>Process Groups</h4>
The shell is put into its own process group. The child processes are put into their own process group after being forked.
<h4>Built-in Commands </h4>
<h5>jobs</h4>
If the argument is jobs, it calls the function printList() which prints the job list.
<h5>kill %#</h4>
Sends a SIGTERM to the specified or most recent child process. If -9 is specified, then it sends a SIGKILL to the specified or most recent process.
<h5>bg %#</h5>
Is it's own "function" that restarts a child process that is suspended in the background.
<h5>fg %#</h5>
Function for fg: put_job_in_foreground() that restarts a child process and puts it to the foreground.

<h3>Partially Implemented</h3>
emacs, vi, and cat do not show up in the foreground of the terminal because if the child process is given the terminal foreground and catches ctrl-z, then it is suspended but not added to the job list.

<h3>Not Implemented</h3>
Extra Credit is not implemented.

<h2>Testing</h2>
Tested the program by first writing smaller programs (one for joblist and another for the parser) to make sure those features were working seperately, then each time a feature was finished being implemented. Valgrind was used to check for memory leaks. Printf() debugging was also used to check for errors. <br  >
[hw3tests_shared.txt](https://github.com/spark3472/hw3/files/7319483/hw3tests_shared.txt)


<h3>Tests Performed</h3>
Small tests at each step of building the code were performed. When near completion, the test cases given by hw3texts_shared.txt were used. 

<h3>Bugs</h3>
Backgrounding emacs or cat with an ampersand ends up sending a signal which terminates those child processes as well as creating a double free() error. <br  >
Double free() errors are most of our errors <br  >
Press enter after fg to return to shell <br  >
Memory leaks (tried to free everything)
