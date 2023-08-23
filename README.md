# MAestro

MAestro is the operating system kernel for [MA-Memphis](https://github.com/gaph-pucrs/MA-Memphis).

It includes multiprogramming capabilities, scheduling, message-passing drivers for data and broadcast NoC, and some
POSIX system calls.

## Installation

Currently, MA-Memphis supports RISC-V cores simulated in Linux hosts.

### Pre-requisites

* riscv64-elf-gcc
* [libmutils](https://github.com/gaph-pucrs/libmutils)

### Building

Run `make`.
It should produce `kernel.{elf, bin, lst, txt}`
