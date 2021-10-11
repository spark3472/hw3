# README

Your README must explain any particular design decisions or features of your shell. It
should contain at least three parts:
- An introduction to the basic structure of your shell
- A list of which features are fully implemented, partially implemented and not implemented
- A discussion of how you tested your shell for robustness and correctness

You should get used to providing your software with an overview document which will make
granting credits easier. You should write this document especially well if you are unable get
things working completely but have the bones right.

<h2>Introduction</h2>
This shell does most of the work of a common terminal shell by using execvp as well as calling functions that were implemented to act as built-in commands (fg, bg, jobs, and kill). Each new line on the shell is parsed into individual words and delimiters and read into a global char* array. We use a linked list to keep track of backgrounded or stopped jobs. The job list does not include jobs running in the foreground or the shell itself. For each job, we're storing the PID, job number, command line arguments that started the job, the number of arguments in that command, the status value, and the termios settings of the job. The shell ignores SIGQUIT, SIGTTIN, SIGTTOU, SIGSTOP, and SIGINT, and handles SIGCHLD and SIGTSTP. A new background job will be added to the joblist and continues to run while the shell is put back in the foreground, and a new foreground job will be run to completion (unless it is ctrl-z'd), at which point the shell will take back over. When a job is stopped, it send a SIGCHLD to the shell which updates the joblist as necessary. We use signal masking around critical regions in the joblists to handle concurrency issues.

<h3>Data Structures Implemented</h3>
The two primary data structures used is a linked list and an array. A linked list is used to keep track of all of the backgrounded jobs and can be manipulated by calling functions that add to the list and delete it. The basis of the linked list we decided to implement came from https://www.gnu.org/software/libc/manual/html_node/Data-Structures.html. It uses two structs, a Process struct that holds the pointer to the next Process, the commands to be put into execvp, the number of commands to be put into execvp, the pid of the Process, whether or not the Process is suspended, the status of the Process, the job number of the Process, and another struct inside the Process struct that holds the terminal settings of the Process. The Linked List struct holds head of the pointers to the processes, the number of jobs currently running in the background, and the total number of jobs added since starting the shell. A 2D array was used to store the user-inputed commands and can be manipulated by the parser which mallocs space according to the number of commands and the length of each command. 

<h3>Parser</h3>
For the parser, we implemented pseudocode given in class found in hw2feedback. It first sifts through the commands, taking out spaces as well as delineating ampersands and semicolons. Then, it finds the total number of commands/arguments entered, mallocing the 2D array to hold that many spaces. Then, each "row" of the 2d array is malloced to hold enough space to fit each command/argument. 

<h3>Signal Handling</h3>
The signals that the shell ignores are SIGQUIT, SIGINT, SIGTSTP, SIGTTIN, and SIGTTOU. However, these signals are reset for the child processes. The shell handles SIGCHLD in order to determine when a child process has finished running so that it can be removed from the job list. The shell deals with concurrency issues resulting from this by masking SIGCHLD around critical regions, namely removing or adding to the job list by using sigprogmask. The shell handles SIGTSTP to background a job running in the foreground, at which point the job is stopped and added to the joblist.

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
<h5>bg</h5>
Is it's own "function" that puts the shell in the foreground and sends a SIGCONT (if necessary) to restart a suspended process.
<h5>Foreground Job</h5>
<br />

<h3>Partially Implemented</h3>
Built-in commands are not implemented with "&" and ";".


<h3>Not Implemented</h3>
Piping and redirection are not implemented.

<h2>Testing</h2>

<h3>Tests Performed</h3>

<h3>Bugs</h3>
