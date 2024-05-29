#define PAGE_SIZE 4096
#define TABLE_PAGE_COUNT 2

enum PageFlag{
    PAGE_FREE,
    PAGE_NOT_FREE,
};

extern char _heap_start;

static u8   *table;
static char *pages;

void pageMemInit(){
    pages = &_heap_start + (TABLE_PAGE_COUNT * PAGE_SIZE);
    table = (u8*)&_heap_start;
    kmemset(table, PAGE_FREE, PAGE_SIZE * TABLE_PAGE_COUNT);

    kprint("heap_start: %p\npage_table_len: %d\npage_table_start: %p\npage_table_end: %p\n", &_heap_start, TABLE_PAGE_COUNT, table, pages);
};
char* allocHardPage(){
    const u32 entriesInATable = kceil((f32)(TABLE_PAGE_COUNT * PAGE_SIZE)/(f32)sizeof(u8));
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