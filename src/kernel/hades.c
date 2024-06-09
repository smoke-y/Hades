#define VTABLE_ENTRIES_COUNT 512
#define MAX_PROCESS 7

enum ProcessState{
    PROCESS_RUNNING,
    PROCESS_SLEEPING,
    PROCESS_WAITING,
    PROCESS_DED,
};

typedef struct{
    u64 entries[VTABLE_ENTRIES_COUNT];
} VTable;
typedef struct{
    u64 regs[32];
    u64 fregs[32];
    u64 satp;
    void *stack;
    u64 hartID;
}TrapFrame;
typedef struct{
    char *buff;
    u64   len;
    u64   enteredUpto;
}InputContext;
typedef struct{
    TrapFrame trapFrame;
    VTable *vtable;
    char   *stack;
    u64     programCounter;
    u16     pid;
    enum ProcessState state;
}Process;
typedef struct{
    //note this entrire struct should fit inside a page
    u16 count;
    u16 cur;
    Process processes[MAX_PROCESS];
}Scheduler;

typedef struct{
    VTable *vtable;
    TrapFrame trapFrame;
    InputContext inputContext;
    Scheduler scheduler;
    u16 pid;
}HadesKernel;

static HadesKernel hades;