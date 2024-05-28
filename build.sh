#!/bin/bash

if [ ! -d "bin/" ]; then
    mkdir bin/
fi

zeus.o src/main.zs bin/kernel.asm
riscv64-unknown-elf-as src/asm/bootloader.asm -o bin/hades.o
riscv64-unknown-elf-ld -nostdlib -melf64lriscv -T script.ld bin/hades.o -o bin/hades.elf
