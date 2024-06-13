#define PAGE_SIZE 4096
#define TABLE_PAGE_COUNT 2

enum PageFlag{
    PAGE_FREE,
    PAGE_NOT_FREE,
};

extern char _heap;
extern char _stack;
extern char _text_start;
extern char _text_end;
extern char _data_start;
extern char _data_end;
extern char _bss_start;
extern char _bss_end;
extern char _memory_start;
extern char _memory_end;

static u8   *table;
static char *pages;

void pageMemInit(){
    pages = &_heap + (TABLE_PAGE_COUNT * PAGE_SIZE);
    u64 order = (1 << 12) - 1;
    pages = (char*)(((u64)pages + order) & ~order);
    table = (u8*)&_heap;
    memset(table, PAGE_FREE, PAGE_SIZE * TABLE_PAGE_COUNT);

    kprint("heap_start: %p\npage_table_len: %d\npage_table_start: %p\npages_start: %p\n", &_heap, TABLE_PAGE_COUNT, table, pages);
};
char* allocHardPage(){
    const u32 entriesInATable = ceil((f32)(TABLE_PAGE_COUNT * PAGE_SIZE)/(f32)sizeof(u8));
    for(u32 x=0; x<entriesInATable; x++){
        if(table[x] == PAGE_FREE){
            table[x] = PAGE_NOT_FREE;
            return &pages[PAGE_SIZE * x];
        };
    };
    return 0;
};
void freeHardPage(void *ptr){
    const u32 off = ((char*)ptr - pages) / PAGE_SIZE;
    table[off] = PAGE_FREE;
};
void dumpHardPageTable(){
    kprint("---hard pages---\n");
    const u32 entriesInATable = ceil((f32)(TABLE_PAGE_COUNT * PAGE_SIZE)/(f32)sizeof(u8));
    for(u32 x=0; x<entriesInATable; x++){
        if(table[x] == PAGE_NOT_FREE) kprint("%d -> %p\n", x, &pages[PAGE_SIZE * x]);
    };  
};

enum EntryBits{
    ENTRY_VALID = 1,
    ENTRY_READ  = 1 << 1,
    ENTRY_WRITE = 1 << 2,
    ENTRY_EXECUTE = 1 << 3,
    ENTRY_LEAF  = ENTRY_READ | ENTRY_WRITE | ENTRY_EXECUTE,
    ENTRY_USER = 1 << 4,
    ENTRY_DIRTY = 1 << 7,
    ENTRY_ACCESS = 1 << 6,
};
VTable *newVTable(){
    void *table = allocHardPage();
    memset(table, 0, PAGE_SIZE);
    return table;
};
void vmap(VTable *root, u64 vaddr, u64 paddr, u64 bits){
    ASSERT((bits & 0xe) != 0);
    u64 vpn[] = {
        (vaddr >> 12) & 0x1ff,
        (vaddr >> 21) & 0x1ff,
        (vaddr >> 30) & 0x1ff,
    };
    u64 ppn[] = {
        (paddr >> 12) & 0x1ff,
        (paddr >> 21) & 0x1ff,
        (paddr >> 30) & 0x1ff,
    };
    u64 *v = &root->entries[vpn[2]];
    for(u32 x=2; x>0;){
        --x;
        if((*v & ENTRY_VALID) == 0){
            VTable *table = newVTable();
            *v = ((u64)table >> 2) | ENTRY_VALID;
        };
        VTable *table = (VTable*)((*v & ~0x3ff) << 2);
        v = &table->entries[vpn[x]];
    };
    *v = (ppn[2] << 28) | (ppn[1] << 19) | (ppn[0] << 10) | bits | ENTRY_VALID;
};
u64 getPhyFromVirt(VTable *root, u64 vaddr){
    u64 vpn[] = {
        (vaddr >> 12) & 0x1ff,
        (vaddr >> 21) & 0x1ff,
        (vaddr >> 30) & 0x1ff,
    };
    u64 *v = &root->entries[vpn[2]];
    for(u32 x=2; x>0;){
        --x;
        if((*v & ENTRY_VALID) == 0) return 0;
        VTable *table = (VTable*)((*v & ~0x3ff) << 2);
        v = &table->entries[vpn[x]];
    };
    /*
    BIT TRICK(replace 12 by any desired number)
    1 << 12 gives us       0b1000000000000      1 followed by 12 zero
    (1 << 12) - 1 gives us 0b111111111111       12 ones :)
    */
    const u64 offsetMask = (1 << 12) - 1;
    u64 offsetAddr = vaddr & offsetMask;
    u64 addr = (*v & ~0x3ff) << 2;
    return addr | offsetAddr;
};
void destroyVTable(VTable *root){
    for(u32 lv2=0; lv2<VTABLE_ENTRIES_COUNT; lv2++){
        u64 lv2Entry = root->entries[lv2];
        if(lv2Entry & ENTRY_VALID != 0){
            VTable *lv2VTable = (VTable*)((lv2Entry & ~0x3ff) << 2);
            for(u32 lv1=0; lv1<VTABLE_ENTRIES_COUNT; lv1++){
                u64 lv1Entry = lv2VTable->entries[lv1];
                if(lv1Entry & ENTRY_VALID != 0){
                    VTable *lv0VTable = (VTable*)((lv1Entry & ~0x3ff) << 2);
                    freeHardPage(lv0VTable);
                };
            };
            freeHardPage(lv2VTable);
        };
    };
    freeHardPage(root);
};
void mapMemRange(VTable *root, void *restrict beginPtr, void *restrict endPtr, u64 bits){
    u64 begin = (u64)beginPtr;
    u64 end = (u64)endPtr;
    u64 order = (1 << 12) - 1;
    begin = begin & ~order;
    end = (end + order) & ~order;

    while(begin != end){
        vmap(root, begin, begin, bits);
        begin += PAGE_SIZE;
    };
};