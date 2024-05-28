u32 strLen(char *str){
    u32 x=0;
    while(str[x]) x++;
    return x;
};
u32 strCmp(char *str1, char *str2){
    u32 len1 = strLen(str1);
    u32 len2 = strLen(str2);
    if(len1 != len2) return FALSE;
    for(u32 x=0; x<len1; x++){
        if(str1[x] != str2[x]) return FALSE;
    };
    return TRUE;
};