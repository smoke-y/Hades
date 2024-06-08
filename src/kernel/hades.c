#define VTABLE_ENTRIES_COUNT 512

typedef struct{
    u64 entries[VTABLE_ENTRIES_COUNT];
} VTable;
typedef struct{
    f64 regs[32];
    f64 fregs[32];
    u64 satp;
    void *stack;
    u64 hartID;
}TrapFrame;
typedef struct{
    char *buff;
    u64   len;
    u64   enteredUpto;
    u8    done;
}InputContext;

typedef struct{
    VTable *vtable;
    TrapFrame trapFrame;
    InputContext inputContext;
}HadesKernel;

static HadesKernel hades;