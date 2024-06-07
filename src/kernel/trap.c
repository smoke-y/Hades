#define CLOCK_FREQUENCY 10000000  //hz

volatile u64 *MTIMECMP = (u64*)0x02004000;  //when(cycle) should I raise interrupt
volatile u64 *MTIME = (u64*)0x0200bff8;     //current cycle

typedef struct{
    f64 regs[32];
    f64 fregs[32];
    u64 satp;
    void *stack;
    u64 hartID;
}TrapFrame;

u64 trap(u64 epc, u64 tval, u64 cause, u64 hart, u64 status, TrapFrame *frame){
    const u64 causeNum = cause & 0xfff;
    if((cause >> 63) & 1 == 1){
        //async- interrupt raised outside
        switch(causeNum){
            case 3:{
                kprint("Machine software interrupt: hart[%d]\n", hart);
            }break;
            case 7:{
                kprint("time interrupt :)\n");
                *MTIMECMP = (*MTIME) + CLOCK_FREQUENCY;     //raise interrupt after 1 sec
            }break;
            case 11:{
                kprint("Machine external interrupt: hart[%d]\n", hart); //interrupt from PLatform Interrupt Controller
            }break;
            default:{
                PANIC("Unhandled async interrupt: hart[%d] cause[%d]", hart, causeNum);
            }break;
        };
        return epc;
    }else{
        switch(causeNum){
            case 2:{
                PANIC("Illegal instruction: hart[%d] program_counter[%p] trap_value[%p]", hart, epc, tval);
                return epc;
            }break;
            case 8:{
                kprint("Sys-call from user mode: hart[%d] program_counter[%p]", hart, epc);
            }break;
            case 9:{
                kprint("Sys-call from supervisor mode: hart[%d] program_counter[%p]", hart, epc);
            }break;
            case 11:{
                PANIC("Sys-call from machine mode: hart[%d] program_counter[%p]", hart, epc);
                return epc;
            }break;
            case 12:{
                kprint("Instruction page fault: hart[%d] program_counter[%p] trap_value[%d]", hart, epc, tval);
            }break;
            case 13:{
                kprint("Load page fault: hart[%d] program_counter[%p] trap_value[%d]", hart, epc, tval);
            }break;
            case 15:{
                kprint("Store page fault: hart[%d] program_counter[%p] trap_value[%d]", hart, epc, tval);
            }break;
            default:{
                PANIC("Unhandled sync interrupt: hart[%d] cause[%d] trap_value[%d]", hart, causeNum, tval);
                return epc;
            }break;
        };
    };
    u64 instruction = *((u64*)epc);
    if((instruction & 0x3) != 0x3) epc += 2;   //16-bit compressed instruction
    else epc += 4;                             //32-bit instruction
    return epc;
};