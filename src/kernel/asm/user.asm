.global _switch_to_user

_switch_to_user:
    #a0 -> TrapFrame
    #a1 -> mepc
    #a2 -> SATP
    csrw mscratch, a0
    li t0, (1 << 7) | (1 << 5) #1<<7 MPIE
    csrw mstatus, t0
    csrw mepc, a1
    csrw satp, a2
    li t1, 0xaaa
    csrw mie, t1
    la t2, _trap_vector
    csrw mtvec, t2
    mv t6, a0
    .set i, 1
    .rept 31
        load_gp %i, t6
        .set i, i+1
    .endr
    mret