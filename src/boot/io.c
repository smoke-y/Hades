volatile char *UART_MEM = (char*)0x10000000;

void uartInit(){
    const u16 divisor = 592;               //BAUD = 2400
    const u8  divisorMost = divisor>>8;
    const u8  divisorLeast = divisor&0xff;

    *(UART_MEM+3) = 1<<7;                  //open "divisor latch"
    *(UART_MEM + 1) = divisorMost;
    *(UART_MEM) = divisorLeast;

    *(UART_MEM + 3) = 1 | 1<<1;            //setup word length = 8
    *(UART_MEM + 2) = 1;                   //FIFO
    *(UART_MEM + 1) = 1;                   //raise interupts on input

};
char uartGet(){
    if(*(UART_MEM + 5)) return *(UART_MEM);
    return 0;
};
void uartPut(char c){*(UART_MEM) = c;};

void kprint(char *fmt, ...){
    const u32 len = strlen(fmt);
    va_list args;
    va_start(args, fmt);
    for(u32 x=0; x<len; x++){
        if(fmt[x] == '%'){
            switch(fmt[++x]){
                case '%':
                    uartPut('%');
                    break;
                case 'd':{
                    s64 num = va_arg(args, s64);
                    if(num == 0){
                        uartPut('0');
                        break;
                    };
                    if(num < 0) uartPut('-');
                    u8 arr[20];
                    u32 i=0;
                    while(num > 0){
                        arr[i++] = (num % 10) + '0';
                        num /= 10;
                    };
                    while(i > 0) uartPut(arr[--i]);
                }break;
                case 'p':{
                    uartPut('0');
                    uartPut('x');
                    s64 num = va_arg(args, s64);
                    if(num == 0){
                        uartPut('0');
                        break;
                    };
                    u8 arr[20];
                    u32 i = 0;
                    while(num != 0){
                        u8 temp = num % 16;
                        if(temp < 10) temp += '0';
                        else temp += '7';
                        arr[i++] = temp;
                        num /= 16;
                    };
                    while(i > 0) uartPut(arr[--i]);
                }break;
                case 's':{
                    char *str = va_arg(args, char*);
                    u32 stringLen = strlen(str);
                    for(u32 i=0; i<stringLen; i++) uartPut(str[i]);
                }break;
            };
        }else if(fmt[x] == '\\'){
            switch(fmt[++x]){
                case 'n':
                    uartPut('\n');
                    break;
            };
        }else uartPut(fmt[x]);
    };
    va_end(args);
};

u32 kscan(char *buff, u32 len){
    hades.inputContext.buff = buff;
    hades.inputContext.len = len;
    hades.inputContext.enteredUpto = 0;
    while(hades.inputContext.buff) asm volatile ("wfi");
};