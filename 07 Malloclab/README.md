# Malloclab: Implementing a Dynamic Memory Allocator

## Overview

Malloclab is a part of the "Computer Systems: A Programmer's Perspective" (CS:APP) course. It challenges students to understand and implement dynamic memory allocation by writing their versions of the `malloc`, `free`, and optionally `realloc` functions. This lab emphasizes the importance of memory efficiency and allocation speed.

## Lab Objectives

The objective of Malloclab is for students to design and implement a memory allocator that can efficiently manage dynamic memory. Students will:

- Implement memory allocation, deallocation, and optionally resizing functions.
- Explore different strategies for free block organization and allocation algorithms.
- Aim for high utilization and throughput while minimizing fragmentation.

## Student's Contributions

### Memory Allocator Implementation

Students implement the memory allocator in the file `mm.c`. This involves:

- Designing a strategy to manage free memory blocks, which could involve explicit lists, segregated fits, or other algorithms.
- Implementing the `malloc` function to allocate memory blocks of requested sizes.
- Implementing the `free` function to deallocate and return memory blocks to the pool of free memory.
- Optionally, implementing the `realloc` function to resize allocated memory blocks.

### Key File

- **`mm.c`**: The central file containing the student's memory allocator implementation. This file includes the logic for block allocation, deallocation, and memory block management.
