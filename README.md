# MAestro

MAestro is the operating system kernel for [Memphis-V](https://github.com/gaph-pucrs/Memphis-5) [MA-Memphis](https://github.com/gaph-pucrs/MA-Memphis).

It includes multiprogramming capabilities, scheduling, message-passing drivers for data and broadcast NoC, and some
POSIX system calls.

## Installation

Currently, MAestro supports RISC-V cores.

### Pre-requisites

* riscv64-elf-gcc
* [libmutils](https://github.com/gaph-pucrs/libmutils)

### Building

Run `make`.
It should produce `kernel.{elf, bin, lst, txt}`

## Acknowledgements

* Scheduler
```
Ruaro, M., and Moraes, F. G. (2016). Dynamic real-time scheduler for large-scale MPSoCs. In Proceedings of the Great Lakes Symposium on VLSI, pages 341-346.
```
