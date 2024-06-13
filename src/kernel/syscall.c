typedef void(*SysCallFunc)(TrapFrame*);


SysCallFunc sysCallTable[] = {

};

void makeSysCall(u64 sysCallNo, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5, u64 a6){
    asm volatile(
        "ecall\n"
        "ret"
    );
};