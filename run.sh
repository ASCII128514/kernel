#!/bin/bash

# qemu-system-x86_64 -m 2G -curses -cdrom boot.iso

qemu-system-x86_64 -m 2G -smp 10 -cdrom boot.iso
