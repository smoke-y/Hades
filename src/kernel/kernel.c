#include "include.h"

u64 kernel_init(){
    //executes in machine mode
    uartInit();
    kprint("[+] Entered kernel_init from bootloader in machine mode\nuart_mem: %p\n", UART_MEM);
    pageMemInit();
    hades.vtable = newVTable();
    hades.scheduler.count = 0;
    hades.scheduler.cur = 0;
    hades.pid = 0;
    memset(&hades.trapFrame, 0, sizeof(TrapFrame));
    hades.trapFrame.stack = allocHardPage() + PAGE_SIZE;  //stack grows downwards
    asm volatile ("csrw mscratch, %0" :: "r"(&hades.trapFrame));
    mapMemRange(hades.vtable, &_rodata_start, &_rodata_end, ENTRY_READ);
    mapMemRange(hades.vtable, &_text_start, &_text_end, ENTRY_EXECUTE | ENTRY_READ);
    mapMemRange(hades.vtable, &_data_start, &_data_end, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, &_bss_start, &_bss_end, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, &_bss_end, &_stack, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, &_stack, &_memory_end, ENTRY_READ | ENTRY_WRITE); //heap and pages
    mapMemRange(hades.vtable, (void*)UART_MEM, (void*)UART_MEM + 100, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, (void*)MTIMECMP, (void*)MTIME, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, (void*)PLIC_PRIORITY, (void*)PLIC_CLAIM, ENTRY_READ | ENTRY_WRITE);
    kprint("kernel_vtable: %p\n", hades.vtable);
    return (u64)8 << 60 | ((u64)hades.vtable >> 12); //8 << 60 for Sv39 scheme
};

void kernel_main(){
    //executes in supervisor mode
    kprint("[+] Entered kernel_main in supervisor mode\n");
    plicSetThreshold(0);
    plicEnable(PLIC_UART_ID, 1);
    for(u32 x=0; x<11; x++) plicEnable(x, 1);
    *MTIMECMP = *MTIME + CLOCK_FREQUENCY;
    char buff[1024];
    while(TRUE){
        kprint(">> ");
        kscan(buff, 1024);
        if(strcmp("exit", buff)) break;
    };
    kprint("stalling...\n");
};
