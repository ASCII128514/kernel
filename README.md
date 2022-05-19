# Starter Kernel

This is the starter kernel code for the spring 2022 offering of CSC 395: Advanced Operating Systems at Grinnell College. This project includes the basic elements required to build and run a simple kernel on an x86_64 machine emulated with QEMU.

## Acknowledgements

This starter code is based on the following example projects:

- [OSDev.org Bare Bones Kernel](https://wiki.osdev.org/Bare_bones)
- [Stivale2 Barebones Kernel](https://github.com/stivale/stivale2-barebones)

In addition to the above example projects, the following references were used when setting up this code:

- [OSDev.org GCC Cross Compiler Instructions](https://wiki.osdev.org/GCC_Cross-Compiler)
- [Stivale2 Reference](https://github.com/stivale/stivale/blob/master/STIVALE2.md)

# Multi-core processing

The multi-core elements are all in the letter_counter2.c, letter_counter2.h, cpu.c, cpu.h, lock.c, and lock.h files.
The demo is in the boot.c file. The demos can be independently ran or all together.
Demo 1 and 3 just demonstate that it can correctly tally letters.
Demo 2 and 4 show the multi-core aspects of the code. Demo 2 shows what happens when only 2 of the 3 CPUs run the code
Demo 4 shows what happens when the main CPU does not wait for the child processes to finish. In this case the result depends
on the order that the CPUs run. In some cases it may finish the counting before printing, in other cases it won't count all the letters.

letter_counter.c and letter_counter.h was our original version of letter counting with locks. This version was able to accomplish
the task without requiring the wait and sleep cpu functions. We thought the solution was pretty poggers so we just kept it in.
