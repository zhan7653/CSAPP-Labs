# Shlab: Writing a Unix Shell

## Overview

Shlab, part of the "Computer Systems: A Programmer's Perspective" (CS:APP) course, offers students hands-on experience with the inner workings of a Unix shell. By implementing a simplified version of a shell, students learn about process control, signal handling, and the execution of commands within the Unix environment.

## Lab Objectives

The primary objective of Shlab is to create a functional Unix shell, named Tsh (Tiny Shell), that supports:

- Executing commands in the foreground and background
- Signal handling (e.g., `CTRL-C`, `CTRL-Z`)
- Implementing built-in commands such as `quit`, `jobs`, `bg`, and `fg`

## Student's Contributions

### Tiny Shell Implementation

Students are responsible for implementing the Tiny Shell in the file `tsh.c`. Key aspects of the implementation include:

- Parsing and interpreting command line inputs
- Creating and managing processes for executing commands
- Implementing job control, including background execution and moving jobs between foreground and background
- Handling Unix signals to appropriately respond to user inputs and manage child processes

### Key File

- **`tsh.c`**: The central file where students implement the functionalities of the Tiny Shell. This includes parsing command line inputs, handling execution of commands, managing processes, and implementing signal handling.
