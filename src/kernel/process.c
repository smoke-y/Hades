#define PROCESS_STARTING_ADDRESS 0x69000000
#define STACK_STARTING_ADDRESS   0x42000000

Process *newProcess(void *procHardAddress, Scheduler *scheduler){
    if(scheduler->count+1 == MAX_PROCESS) return 0;
    Process *process = &scheduler->processes[scheduler->count++];
    
    memset(&process->trapFrame, 0, sizeof(TrapFrame));
    process->vtable = newVTable();
    process->stack = allocHardPage();
    process->programCounter = PROCESS_STARTING_ADDRESS;
    process->pid = hades.pid++;
    process->state = PROCESS_WAITING;

    process->trapFrame.regs[2] = (u64)(process->stack + PAGE_SIZE);
    vmap(process->vtable, (u64)STACK_STARTING_ADDRESS, (u64)process->stack, ENTRY_READ | ENTRY_WRITE);
    vmap(process->vtable, (u64)PROCESS_STARTING_ADDRESS, (u64)procHardAddress, ENTRY_READ | ENTRY_EXECUTE);

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
    switch(process->state){
        case PROCESS_RUNNING:{
            stuc.trapFrame = &process->trapFrame;
            stuc.mepc = process->programCounter;
            //help cpu by flushing old vtable by using pid
            stuc.satp = ((u64)8 << 60) | ((u64)process->pid << 44) | ((u64)process->vtable >> 12);
        }break;
    };
    return stuc;
};