#include "include.h"

#define SHELL_INPUT_BUFF_SIZE  1000

void kernel_main(){
    uartInit();
    kprint("[+] Entered kernel from bootloader\nuart_mem: %p", UART_MEM);
    char shellInputBuff[SHELL_INPUT_BUFF_SIZE];
    while(TRUE){
        kprint("\n>> ");
        u32 bytes = kscan(shellInputBuff, SHELL_INPUT_BUFF_SIZE);
        if(strCmp(shellInputBuff, "exit")) break;
    };
    kprint("---stalling---\n");
};