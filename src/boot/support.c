u64 strlen(char *str){
    u32 x=0;
    while(str[x]) x++;
    return x;
};
u32 strcmp(char *restrict str1, char *restrict str2){
    u32 len1 = strlen(str1);
    u32 len2 = strlen(str2);
    if(len1 != len2) return FALSE;
    for(u32 x=0; x<len1; x++){
        if(str1[x] != str2[x]) return FALSE;
    };
    return TRUE;
};
f64 ceil(f64 num) {
    s64 inum = (s64)num;
    if(num == (f64)inum) return inum;
    return inum + 1;
};
void *memset(void *start, s32 ch, u64 size){
    char *mem = (char*)start;
    for(u64 x=0; x<size; x++) mem[x] = ch;
    return start;
};
void *memcpy(void *restrict dst, void *restrict src, u64 size){
    char *dstc = (char*)dst;
    char *srcc = (char*)src;
    for(u64 x=0; x<size; x++) dstc[x] = srcc[x];
    return dst;
}