#define PROCESS_STARTING_VADDRESS 0x69000000
#define STACK_STARTING_VADDRESS   0x42000000

Process *newProcess(void *procHardAddress, Scheduler *scheduler){
    if(scheduler->count+1 == MAX_PROCESS) return 0;
    Process *process = &scheduler->processes[scheduler->count++];
    
    memset(&process->trapFrame, 0, sizeof(TrapFrame));
    process->vtable = newVTable();
    process->stack = allocHardPage();
    process->programCounter = (u64)procHardAddress;
    process->pid = hades.pid++;

    process->trapFrame.regs[2] = (u64)(process->stack + PAGE_SIZE);
    //vmap(process->vtable, (u64)STACK_STARTING_VADDRESS, (u64)process->stack, ENTRY_READ | ENTRY_WRITE);
    mapMemRange(process->vtable, &_text_start, &_text_end, ENTRY_EXECUTE | ENTRY_READ);
    mapMemRange(process->vtable, &_data_start, &_heap + (&_memory_end - &_memory_start), ENTRY_READ | ENTRY_WRITE);

    return process;
};
void destroyProcess(Process *process){
    destroyVTable(process->vtable);
    freeHardPage(process->stack);
};

typedef struct{
    TrapFrame *trapFrame;
    u64 mepc;
    u64 satp;
}SwitchToUserContext;

SwitchToUserContext schedule(Scheduler *scheduler){
    //worst scheduler to be ever written...
    SwitchToUserContext stuc;
    memset(&stuc, 0, sizeof(SwitchToUserContext));
    if(scheduler->count == 0) return stuc;
    scheduler->cur++;
    if(scheduler->cur == scheduler->count) scheduler->cur = 0;
    Process *process = &scheduler->processes[scheduler->cur];
    stuc.trapFrame = &process->trapFrame;
    stuc.mepc = process->programCounter;
    //help cpu by flushing old vtable by using pid
    stuc.satp = ((u64)8 << 60) | ((u64)process->pid << 44) | ((u64)process->vtable >> 12);
    return stuc;
};