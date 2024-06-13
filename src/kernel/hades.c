#define VTABLE_ENTRIES_COUNT 512

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
    VTable *vtable;
    TrapFrame trapFrame;
    InputContext inputContext;
}HadesKernel;

static HadesKernel hades;