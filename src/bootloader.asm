.option norvc

.section .text.init

.global _start
_start:
	# Any hardware threads (hart) that are not bootstrapping
	# need to wait for an IPI
	csrr	t0, mhartid
	bnez	t0, _stall
	csrw	satp, zero

    #clear bss
    la a0, _bss_start
    la a1, _bss_end
_bss_clear_start:
    sd zero, (a0)
    addi a0, a0, 8
    bltu a0, a1, _bss_clear_start

    la sp, _stack
    la gp, _global_pointer
    li t0, (0b11 << 11) | (1 << 7) | (1 << 3) #11: "machine mode", 7,3: enable interrupts
    csrw mstatus, t0
    la t1, kernel_main    #src/kernel/kernel.c
    csrw mepc, t1
    la t2, _asm_trap_vector
    csrw mtvec, t2
    li t3, (1 << 3) | (1 << 7) | (1 << 11) #enable further interrupts
    csrw mie, t3
    la ra, _stall
    mret

_stall:
    wfi
    j _stall

_asm_trap_vector: mret
