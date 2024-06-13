.global _switch_to_user

_switch_to_user:
    #a0 -> TrapFrame
    #a1 -> mepc
    #a2 -> SATP
    csrw sepc, a1
    #csrw satp, a2
    la ra, kernel_main
    sfence.vma
    sret
    