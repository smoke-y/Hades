#define PROCESS_STARTING_ADDRESS 0x69000000
#define STACK_STARTING_ADDRESS   0x42000000

enum ProcessState{
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_WAITING,
    PROCESS_DED,
};

typedef struct{
    TrapFrame trapFrame;
    VTable *vtable;
    char   *stack;
    u64     programCounter;
    u16     pid;
    enum ProcessState state;
}Process;

Process newProcess(void *procHardAddress){
    Process process;
    
    memset(&process.trapFrame, 0, sizeof(TrapFrame));
    process.vtable = newVTable();
    process.stack = allocHardPage();
    process.programCounter = PROCESS_STARTING_ADDRESS;
    process.pid = hades.pid++;
    process.state = PROCESS_WAITING;

    process.trapFrame.regs[2] = (u64)(process.stack + PAGE_SIZE);
    vmap(process.vtable, (u64)STACK_STARTING_ADDRESS, (u64)process.stack, ENTRY_READ | ENTRY_WRITE);
    vmap(process.vtable, (u64)PROCESS_STARTING_ADDRESS, (u64)procHardAddress, ENTRY_READ | ENTRY_EXECUTE);

    return process;
};
void destroyProcess(Process *process){
    destroyVTable(process->vtable);
    freeHardPage(process->stack);
};