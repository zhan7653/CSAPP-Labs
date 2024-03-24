# Bomblab: Binary Bomb Defusing Lab

## Overview

Bomblab is a hands-on lab that is part of the Computer Systems: A Programmer's Perspective (CS:APP) course curriculum. It is designed to teach students the principles of machine-level programs and to enhance their skills in using debuggers and reverse engineering. In this lab, students face the challenge of "defusing" binary bombs—Linux executable C programs composed of several phases. Each phase requires a specific input string to defuse. Incorrect inputs lead to an explosion, symbolized by the program printing "BOOM!!!".

## Lab Objective

The primary objective is for students to defuse as many bomb phases as possible. This involves:

- Utilizing debugging tools like gdb or ddd to disassemble and step through the bomb's binary code.
- Analyzing assembly instructions to understand their purpose and to deduce the correct input strings for defusing the bomb.
- Learning to set breakpoints effectively and to avoid triggering the bomb's explosion function.

## Student's Contributions

### Analysis and Solutions

The student's work mainly revolves around analyzing the binary executable of the bomb, identifying the expected inputs for each phase, and successfully defusing the bomb. This process requires a thorough understanding of machine-level code and the ability to apply reverse engineering techniques.

### Key Files

- **Binary Bomb (`bomb`)**: The main executable file that students will work on. It contains the compiled phases that need to be defused.
- **Main Routine Source (`bomb.c`)**: The source code for the bomb's main routine, provided to help students understand the bomb's execution flow.

### Implemented Functions

Solution are located in the `solve_process.pdf` file.

