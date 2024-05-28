volatile char *UART_MEM = (char*)0x10000000;

char uartGet(){
    if(*(UART_MEM + 5)) return *(UART_MEM);
    return 0;
};
void uartPut(char c){*(UART_MEM) = c;};