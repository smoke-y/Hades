.altmacro

.macro save_gp i, baseReg=t6
    sd x\i, ((\i)*8)(\baseReg)
.endm
.macro load_gp i, baseReg=t6
    ld x\i, ((\i)*8)(\baseReg)
.endm

_trap_vector:
    csrrw t6, mscratch, t6  #mscratch contains address to kernelTrapFrame(kernel/kernel.c)

    .set i, 1
    .rept 30
        save_gp %i
        .set i, i+1
    .endr

    mv t5, t6
    csrr t6, mscratch
    save_gp 31, t5
    csrw mscratch, t5

    csrr a0, mepc
    csrr a1, mtval
    csrr a2, mcause
    csrr a3, mhartid
    csrr a4, mstatus
    mv a5, t5
    ld sp, 520(a5)   #520 bytes into TrapFrame: TrapFrame.stack
    call trap        #kernel/trap.c

    csrw mepc, a0

    .set i, 1
    .rept 31
        load_gp %i
        .set i, i+1
    .endr

    mret
