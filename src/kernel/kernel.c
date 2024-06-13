#include "include.h"

void testProcess(){
    kprint("YEEEEEe\n");
    //makeSysCall(0, 0, 0, 0, 0, 0, 0);
    //asm volatile ("j _stall");
};
u64 kernel_init(){
    //executes in machine mode
    uartInit();
    kprint("[+] Entered kernel_init from bootloader in machine mode\nuart_mem: %p\n", UART_MEM);
    pageMemInit();
    hades.vtable = newVTable();
    hades.pid = 1;     //pid gets given out from 1
    memset(&hades.trapFrame, 0, sizeof(TrapFrame));
    hades.scheduler.count = 0;
    hades.scheduler.cur = 0;
    hades.trapFrame.stack = allocHardPage() + PAGE_SIZE;  //stack grows downwards
    asm volatile ("csrw mscratch, %0" :: "r"(&hades.trapFrame));
    mapMemRange(hades.vtable, &_text_start, &_text_end, ENTRY_EXECUTE | ENTRY_READ | ENTRY_DIRTY | ENTRY_ACCESS);
    mapMemRange(hades.vtable, &_data_start, &_heap + (&_memory_end - &_memory_start), ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, (void*)UART_MEM, (void*)UART_MEM + 100, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, (void*)MTIMECMP, (void*)MTIME, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(hades.vtable, (void*)PLIC_PRIORITY, (void*)PLIC_CLAIM, ENTRY_READ | ENTRY_WRITE);
    kprint("kernel_vtable: %p\n", hades.vtable);
    vmap(hades.vtable, (u64)testProcess, (u64)testProcess, ENTRY_READ | ENTRY_EXECUTE | ENTRY_ACCESS | ENTRY_DIRTY);
    return (u64)8 << 60 | ((u64)hades.vtable >> 12); //8 << 60 for Sv39 scheme
};

void kernel_main(){
    //executes in supervisor mode
    kprint("[+] Entered kernel_main in supervisor mode\n");
    asm volatile (
        "sfence.vma\n"
        "csrw sepc, %0\n"
        "la ra, kernel_main\n"
        "sfence.vma\n"
        "sret\n"
    ::
        "r"(testProcess)
    );
    // plicSetThreshold(0);
    // plicEnable(PLIC_UART_ID, 1);
    // *MTIMECMP = *MTIME + CLOCK_FREQUENCY;
    // Process *test = newProcess(testProcess, &hades.scheduler);
    // SwitchToUserContext stuc = schedule(&hades.scheduler);
    // if(stuc.trapFrame != 0){
    //     //kprint("%p %p %d", stuc.trapFrame, stuc.mepc, stuc.satp);
    //     _switch_to_user(stuc.trapFrame, stuc.mepc, stuc.satp);
    // };
};
