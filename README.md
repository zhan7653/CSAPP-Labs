# CS:APP Lab Solutions

This repository contains my solutions to the labs for the "Computer Systems: A Programmer's Perspective" (CS:APP) course. Each lab focuses on a different aspect of computer systems, from data manipulation and system programming to understanding how computer systems manage memory and execute programs. The goal of these labs is to provide a hands-on experience with the inner workings of computer systems, enhancing both theoretical knowledge and practical skills.

## Labs Overview

### Datalab

- **Objective**: Bit manipulation and coding with limited operations.
- **Key File**: `bits.c`
- **[More Details](./01%20Datalab/README.md)**

### Bomblab

- **Objective**: Binary bomb defusal through assembly code analysis.
- **Key File**: Solution strings in `solve_process.pdf`
- **[More Details](./02%20Bomblab/README.md)**

### Attacklab

- **Objective**: Code injection and return-oriented programming.
- **Key Files**: `ctarget` and `rtarget` exploitation.
- **[More Details](./03%20Attacklab/README.md)**

### Archlab

- **Objective**: Simulating a pipelined Y86-64 processor.
- **Key File**: Modifications to `sim/pipe/ncopy.ys`
- **[More Details](./04%20Archlab/README.md)**

### Cachelab

- **Objective**: Writing a cache simulator and optimizing matrix transpose.
- **Key Files**: `csim.c` for the simulator and `trans.c` for the optimization.
- **[More Details](./05%20Cachelab/README.md)**

### Shlab

- **Objective**: Implementing a Unix shell.
- **Key File**: `tsh.c`
- **[More Details](./06%20Shlab/README.md)**

### Malloclab

- **Objective**: Implementing a dynamic memory allocator.
- **Key File**: `mm.c`
- **[More Details](./07%20Malloclab/README.md)**

## Getting Started

To work with the labs, clone this repository and navigate into each lab's directory. Each lab has its own README with specific setup instructions and objectives.

```bash
git clone https://github.com/yourusername/csapp-lab-solutions.git
cd csapp-lab-solutions/<lab-directory>
```

## Tools and Environment
The labs were developed and tested on a Unix-like environment, using tools such as `gcc`, `gdb`, and Unix shell commands.
For specific tools and versions required in each lab, refer to the individual README files.

## Acknowledgements
This work is based on the labs provided by the "Computer Systems: A Programmer's Perspective" course by Randal E. Bryant and David R. O'Hallaron. The solutions are my own and reflect my understanding of computer systems as taught in the course.

## License
This project is provided for educational purposes only. It is not affiliated with the official CS:APP course or its authors.
