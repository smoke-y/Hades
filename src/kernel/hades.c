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
struct Queue;
struct virtioRegs;
typedef struct{
	struct Queue *queue;
	struct virtioRegs *regs;
    char *reqMem;
    u32 reqBit;
	u32 idx;
	u32 ackIdx;
    u8 readOnly;
}VirtioBlkDriver;

typedef struct{
    TrapFrame trapFrame;
    InputContext inputContext;
    VirtioBlkDriver blkDriver;
    VTable *vtable;
    u8     *table;
    char   *pages;
}HadesKernel;

static HadesKernel hades;