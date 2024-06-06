#include "include.h"

#define SHELL_INPUT_BUFF_SIZE  1000

VTable *kernelVTable;

//executes in machine mode
u64 kernel_init(){
    uartInit();
    kprint("[+] Entered kernel_init from bootloader in machine mode\nuart_mem: %p\n", UART_MEM);
    pageMemInit();
    kernelVTable = newVTable();
    mapMemRange(kernelVTable, &_text_start, &_rodata_end, ENTRY_READ | ENTRY_EXECUTE);
    mapMemRange(kernelVTable, &_data_start, &_bss_end, ENTRY_READ | ENTRY_WRITE | ENTRY_EXECUTE);
    mapMemRange(kernelVTable, &_bss_end, (&_heap) + (PAGE_SIZE*TABLE_PAGE_COUNT), ENTRY_READ | ENTRY_WRITE);  //map stack -> page_table
    kprint("kernel_vtable: %p\n", kernelVTable);
    return (u64)8 << 60 | ((u64)kernelVTable >> 12); //8 << 60 for Sv39 scheme
};

//executes in supervisor mode
void kernel_main(){
    char shellInputBuff[SHELL_INPUT_BUFF_SIZE];
    while(TRUE){
        kprint("\n>> ");
        u32 bytes = kscan(shellInputBuff, SHELL_INPUT_BUFF_SIZE);
        if(kstrcmp(shellInputBuff, "exit")) break;
    };
    destroyVTable(kernelVTable);
    kprint("---stalling---\n");
};