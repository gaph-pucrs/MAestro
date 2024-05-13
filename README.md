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

* Low-Level Monitoring
```
Dalzotto, A. E., Borges, C. S., Ruaro, M., and Moraes, F. G. (2022). Non-intrusive Monitoring Framework for NoC-based Many-Cores. In Proceedings of the Brazilian Symposium on Computing Systems Engineering (SBESC), pages 1-7.
```

* Microkernel
```
Dalzotto, A. E., Ruaro, M., Erthal, L. V., and Moraes, F. G. (2021). Management Application - a New Approach to Control Many-Core Systems. In Proceedings of the Symposium on Integrated Circuits and Systems Design (SBCCI), pages 1-6.
```

* Kernel services
```
Ruaro, M., Caimi, L. L., Fochi, V., and Moraes, F. G. (2019). Memphis: a framework for heterogeneous many-core SoCs generation and validation. Design Automation for Embedded Systems, 23(3-4):103-122.
```

* Scheduler
```
Ruaro, M., and Moraes, F. G. (2016). Dynamic real-time scheduler for large-scale MPSoCs. In Proceedings of the Great Lakes Symposium on VLSI, pages 341-346.
```
