#!/bin/bash

if [ ! -d "bin/" ]; then
    mkdir bin/
    touch bin/hdd.dsk
fi

riscv64-unknown-elf-gcc -mcmodel=medany -nostdlib -c src/kernel/kernel.c -o bin/kernel.o -g
if [ $? -eq 0 ]; then
    riscv64-unknown-elf-as src/bootloader.asm -o bin/hades.o -g
    riscv64-unknown-elf-ld -nostdlib -melf64lriscv -T script.ld bin/kernel.o bin/hades.o -o bin/hades.elf -g
    echo "done generating code"
    qemu-system-riscv64 -machine virt -cpu rv64 -smp 4 -m 128M -drive if=none,format=raw,file=bin/hdd.dsk,id=foo -device virtio-blk-device,scsi=off,drive=foo -nographic -serial mon:stdio -bios none -device virtio-rng-device -device virtio-gpu-device -device virtio-net-device -device virtio-tablet-device -device virtio-keyboard-device -kernel bin/hades.elf -d int
fi