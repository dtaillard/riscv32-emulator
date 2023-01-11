## RISC-V Emulator

This project implements a RISC-V emulator that targets the RV32IMA instruction set.
The emulator fully supports the user (U) and supervisor (S) modes of the RV32 privileged architecture.
In doing so, it is capable of demonstrating Linux boot with a simple Busybox initramfs image.
M-mode was not implemented as it is not necessary for a simple Linux demonstration.
Console I/O is supported through the RISC-V SBI (supervisor binary interface) specification.
In particular, a ECALL instruction in S-mode is handled by the emulator to implement getchar() and putchar() calls.

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
