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
The two primary data structures used is a linked list and an array. A linked list is used to keep track of all of the backgrounded jobs and can be manipulated by calling functions that add to the list and delete it. A 2D array was used to store the user-inputed commands and can be manipulated by the parser which mallocs space according to the number of commands and the length of each command. 

<h3>Parser</h3>

<h3>Signal Handling</h3>


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
