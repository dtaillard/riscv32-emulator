## RISC-V Emulator

![demo.gif](demo.gif)

This project implements a RISC-V emulator that targets the RV32IMA instruction set.
The emulator fully supports the user (U) and supervisor (S) modes of the RV32 privileged architecture.
The goal of this project is to demonstrate Linux boot with a simple Busybox initramfs image.

### Features
 * Supervisor (S) and user (U) modes
 * Interrupts (timer, external, software)
 * SBI calls for console I/O
 * Exception handling

### Building

This project uses CMake:

```shell
$ cmake -B build .
$ make
```

### Demo

Type `./rv32-emulator` for a simple Linux demonstration.
