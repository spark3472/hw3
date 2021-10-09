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
This shell does most of the work of a common terminal shell by using execvp as well as calling functions that were implemented to act as built-in commands. 
<h3>Data Structures Implemented</h3>
The two primary data structures used is a linked list and an array. A linked list is used to keep track of all of the backgrounded jobs and can be manipulated by calling functions that add to the list and delete it. The basis of the linked list we decided to implement came from https://www.gnu.org/software/libc/manual/html_node/Data-Structures.html. It uses two structs, a Process struct that holds the pointer to the next Process, the commands to be put into execvp, the number of commands to be put into execvp, the pid of the Process, whether or not the Process is suspended, the status of the Process, the job number of the Process, and another struct inside the Process struct that holds the terminal settings of the Process. The Linked List struct holds head of the pointers to the processes, the number of jobs currently running in the background, and the total number of jobs added since starting the shell. A 2D array was used to store the user-inputed commands and can be manipulated by the parser which mallocs space according to the number of commands and the length of each command. 

<h3>Parser</h3>
For the parser, we implemented pseudocode given in class found in hw2feedback. It first sifts through the commands, taking out spaces as well as delineating ampersands and semicolons. Then, it finds the total number of commands/arguments entered, mallocing the 2D array to hold that many spaces. Then, each "row" of the 2d array is malloced to hold enough space to fit each command/argument. 

<h3>Signal Handling</h3>
The signals that the shell ignores are SIGQUIT, SIGINT, SIGTSTP, SIGTTIN, and SIGTTOU. However, these signals are reset for the child processes. The signals that the shell handles are SIGCHLD in order to determine when a child process has finished running so that it can be removed from the job list. The shell hands SIGCHLDs by masking them around areas of critical regions, namely removing or adding to the job list by using sigprogmask.


<h3>Command Execution</h3>
Command execution by user input is done in the same way as a regular shell with execvp commands such as cat, touch, mk, etc. Build-in commands such as fg, bg, and kill are also implemented. 

<h2>Features</h2>

<h3>Fully Implemented</h3>

<h3>Partially Implemented</h3>

<h3>Not Implemented</h3>
Piping and redirection are not implemented.

<h2>Testing</h2>

<h3>Tests Performed</h3>

<h3>Bugs</h3>
