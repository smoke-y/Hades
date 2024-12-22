#define CLOCK_FREQUENCY 10000000  //hz
#define MAX_PLIC_PRIORITY 7

enum PlicId{
    PLIC_UART_ID = 10
};

volatile u32 *PLIC_ENABLE    = (u32*)0x0c002000;
volatile u32 *PLIC_PRIORITY  = (u32*)0x0c000000;
volatile u32 *PLIC_THRESHOLD = (u32*)0x0c200000;
volatile u32 *PLIC_CLAIM     = (u32*)0x0c200004;
volatile u64 *MTIMECMP       = (u64*)0x02004000;  //when(cycle) should I raise interrupt
volatile u64 *MTIME          = (u64*)0x0200bff8;  //current cycle

void plicEnable(enum PlicId id, u8 priority){
    *PLIC_ENABLE |= 1 << id;
    /*
        BIT TRICK(replace 7 with any number)
        a = x & 7 cause a to wrap from 0 to 7.
        when x < 7, a = x
    */
    *(PLIC_PRIORITY + id) = priority & MAX_PLIC_PRIORITY;
};

void plicSetThreshold(u8 threshold){*PLIC_THRESHOLD = threshold & MAX_PLIC_PRIORITY;};

u64 trap(u64 epc, u64 tval, u64 cause, u64 hart, u64 status, TrapFrame *frame){
    const u64 causeNum = cause & 0xfff;
    if((cause >> 63) & 1 == 1){
        //async- interrupt raised outside
        switch(causeNum){
            case 3:{
                kprint("Machine software interrupt: hart[%d]\n", hart);
            }break;
            case 7:{
                SwitchToUserContext stuc = schedule(&hades.scheduler);
                if(stuc.trapFrame != 0){
                    //_switch_to_user(stuc.trapFrame, stuc.mepc, stuc.satp);
                };
                *MTIMECMP = (*MTIME) + CLOCK_FREQUENCY;     //raise interrupt after 1 sec
            }break;
            case 11:{
                u32 interrupt = *PLIC_CLAIM;
                ASSERT(interrupt != 0);
                switch(interrupt){
                    case 10:{
                        char c = uartGet();
                        if(hades.inputContext.buff == 0){
                            *PLIC_CLAIM = interrupt;       //by writing it back, we tell the PLIC that it has been claimed
                            return epc;
                        };
                        if(hades.inputContext.enteredUpto > hades.inputContext.len-1){     //-1 for null byte
                            hades.inputContext.buff[hades.inputContext.enteredUpto++] = '\0';
                            hades.inputContext.buff = 0;
                            *PLIC_CLAIM = interrupt;
                            return epc;
                        };
                        switch(c){
                            case 13:           //carriage return
                                uartPut('\n');
                                hades.inputContext.buff[hades.inputContext.enteredUpto++] = '\0';
                                hades.inputContext.buff = 0;
                                *PLIC_CLAIM = interrupt;
                                return epc;
                            case 127:          //backspace
                                uartPut(8);    //move cursor to left
                                uartPut(' ');  //write space
                                uartPut(8);    //move cursor back to left
                                break;
                            default: uartPut(c);
                        };
                        hades.inputContext.buff[hades.inputContext.enteredUpto++] = c;
                    }break;
                    default:{
                        PANIC("Unkown plic interrupt\n");
                    }break;
                };
                *PLIC_CLAIM = interrupt;
            }break;
            default:{
                PANIC("Unhandled async interrupt: hart[%d] cause[%d]\n", hart, causeNum);
            }break;
        };
        return epc;
    }else{
        switch(causeNum){
            case 2:{
                PANIC("Illegal instruction: hart[%d] program_counter[%p] trap_value[%p]\n", hart, epc, tval);
                return epc;
            }break;
            case 5:{
                PANIC("Load access fault: hart[%d] program_counter[%p] trap_value[%d]\n", hart, epc, tval);
                return epc;
            }break;
            case 8:{
                kprint("Sys-call from user mode: hart[%d] program_counter[%p]\n", hart, epc);
            }break;
            case 9:{
                u32 index = frame->regs[10];   //a0(x10)
                sysCallTable[index](frame);
            }break;
            case 11:{
                PANIC("Sys-call from machine mode: hart[%d] program_counter[%p]\n", hart, epc);
                return epc;
            }break;
            case 12:{
                kprint("Instruction page fault: hart[%d] program_counter[%p] trap_value[%d]\n", hart, epc, tval);
                asm volatile ("j _stall");
            }break;
            case 13:{
                kprint("Load page fault: hart[%d] program_counter[%p] trap_value[%d]\n", hart, epc, tval);
                asm volatile ("j _stall");
            }break;
            case 15:{
                kprint("Store page fault: hart[%d] program_counter[%p] trap_value[%d]\n", hart, epc, tval);
            }break;
            default:{
                PANIC("Unhandled sync interrupt: hart[%d] cause[%d] trap_value[%d]\n", hart, causeNum, tval);
                return epc;
            }break;
        };
    };
    u64 instruction = *((u64*)epc);
    if((instruction & 0x3) != 0x3) epc += 2;   //16-bit compressed instruction
    else epc += 4;                             //32-bit instruction
    return epc;
};