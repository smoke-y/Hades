typedef void(*SysCallFunc)(TrapFrame*);

void SysExit(TrapFrame *trapFrame){
    kprint("exit :)\n");
};

SysCallFunc sysCallTable[] = {
    SysExit,
};

void makeSysCall(u64 sysCallNo, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 a6){
    asm volatile(
        "mv a0, %0\n"
        "mv a1, %1\n"
        "mv a2, %2\n"
        "mv a3, %3\n"
        "mv a4, %4\n"
        "mv a5, %5\n"
        "mv a6, %6\n"
        "ecall\n"
        "ret"
    ::  
        "r"(sysCallNo),
        "r"(a1),
        "r"(a2),
        "r"(a3),
        "r"(a4),
        "r"(a5),
        "r"(a6)
    );
};