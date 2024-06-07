#include "include.h"

#define SHELL_INPUT_BUFF_SIZE  1000

VTable *kernelVTable;
TrapFrame kernelTrapFrame;

u64 kernel_init(){
    //executes in machine mode
    uartInit();
    kprint("[+] Entered kernel_init from bootloader in machine mode\nuart_mem: %p\n", UART_MEM);
    pageMemInit();
    kernelVTable = newVTable();
    kmemset(&kernelTrapFrame, 0, sizeof(TrapFrame));
    kernelTrapFrame.stack = allocHardPage();
    asm volatile ("csrw mscratch, %0" :: "r"(&kernelTrapFrame));
    mapMemRange(kernelVTable, &_text_start, &_text_end, ENTRY_EXECUTE | ENTRY_READ);
    mapMemRange(kernelVTable, &_data_start, &_heap + (&_memory_end - &_memory_start), ENTRY_READ | ENTRY_WRITE);
    mapMemRange(kernelVTable, (void*)UART_MEM, (void*)UART_MEM + 100, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(kernelVTable, (void*)MTIMECMP, (void*)MTIME, ENTRY_READ | ENTRY_WRITE);
    kprint("kernel_vtable: %p\n", kernelVTable);
    return (u64)8 << 60 | ((u64)kernelVTable >> 12); //8 << 60 for Sv39 scheme
};

void kernel_main(){
    //executes in supervisor mode
    kprint("[+] Entered kernel_main in supervisor mode\n");
    char shellInputBuff[SHELL_INPUT_BUFF_SIZE];
    while(TRUE){
        kprint("\n>> ");
        u32 bytes = kscan(shellInputBuff, SHELL_INPUT_BUFF_SIZE);
        if(kstrcmp(shellInputBuff, "exit")) break;
    };
    destroyVTable(kernelVTable);
    kprint("---stalling---\n");
};