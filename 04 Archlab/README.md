# Archlab: Processor Architecture Lab

## Overview

Archlab is a component of the "Computer Systems: A Programmer's Perspective" (CS:APP) course, designed to deepen students' understanding of processor architecture and the intricate relationship between software and hardware. In this lab, students engage in the design and implementation of a pipelined Y86-64 processor, focusing on optimizing the performance of a benchmark array copy program named `ncopy.ys`.

## Lab Objective

The primary goal of Archlab is to introduce students to the concepts of processor design and optimization. Students are tasked with either making semantics-preserving transformations to the `ncopy.ys` benchmark program, enhancing the pipelined processor, or both. The ultimate objective is to minimize the Clock Cycles Per Element (CPE) for the array copy operation.

## Student's Contributions

### Processor Design and Benchmark Optimization

Students will explore various aspects of processor architecture and apply optimizations to improve performance. This involves:

- **Understanding Processor Pipelining**: Grasping the fundamentals of how pipelining improves processing speed and where its limitations lie.
- **Optimizing `ncopy.ys`**: Making modifications to the Y86-64 array copy benchmark program to enhance its efficiency without altering the semantics.
- **Enhancing the Processor**: Proposing and implementing improvements to the pipelined Y86-64 processor to support the optimized benchmark program better.

### Key Files

- **`ncopy.ys`**: The Y86-64 benchmark array copy program provided for optimization.
- **Simulator Tools (`sim.tar`)**: Contains the Y86-64 tools required to simulate the performance of the processor and the `ncopy.ys` program.
- **Lab Writeup (`archlab.pdf`)**: Detailed instructions and guidelines for completing the lab.
