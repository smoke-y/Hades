#define VTABLE_ENTRIES_COUNT 512
#define MAX_PROCESS 5

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
}Process;
typedef struct{
    //note this entrire struct should fit inside a page
    u16 count;
    u16 cur;
    Process processes[MAX_PROCESS];
}Scheduler;

typedef struct{
    TrapFrame trapFrame;
    InputContext inputContext;
    Scheduler scheduler;
    VTable *vtable;
    u8     *table;
    char   *pages;
    u8      pid;
}HadesKernel;

static HadesKernel hades;