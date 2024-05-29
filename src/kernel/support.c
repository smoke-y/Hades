u32 kstrlen(char *str){
    u32 x=0;
    while(str[x]) x++;
    return x;
};
u32 kstrcmp(char *str1, char *str2){
    u32 len1 = kstrlen(str1);
    u32 len2 = kstrlen(str2);
    if(len1 != len2) return FALSE;
    for(u32 x=0; x<len1; x++){
        if(str1[x] != str2[x]) return FALSE;
    };
    return TRUE;
};
s32 kceil(f32 num) {
    s32 inum = (s32)num;
    if(num == (f32)inum) return inum;
    return inum + 1;
};
void kmemset(void *start, char ch, u64 size){
    char *mem = (char*)start;
    for(u64 x=0; x<size; x++) mem[x] = ch;
};