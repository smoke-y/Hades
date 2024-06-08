typedef void(*SysCallFunc)(TrapFrame*);

void SysExit(TrapFrame *trapFrame){
    kprint("exit :)\n");
};

SysCallFunc sysCallTable[] = {
    SysExit,
};