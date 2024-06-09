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

_machine_mode:
    la sp, _stack
    la gp, _global_pointer
    li t0, (0b11 << 11)
    csrw mstatus, t0
    la t1, kernel_init    #kernel/kernel.c
    csrw mepc, t1
    la t2, _trap_vector
    csrw mtvec, t2
    li t3, (1 << 3) | (1 << 7) | (1 << 11)
	csrw mie, t3
    la ra, _supervisor_mode
    mret
_supervisor_mode:
    li t0, (1 << 8) | (1 << 5)  #8: "supervisor mode", 5: enable interrupt
    csrw sstatus, t0
    la t1, kernel_main    #kernel/kernel.c
    csrw sepc, t1
    li t2, 1 | (1 << 5) | (1 << 9) #software, timer, external interrupts
    csrw sie, t2                   #enable above
    csrw mideleg, t2               #delegate them to supervisor mode
    la t3, _trap_vector
    csrw stvec, t3
    csrw satp, a0
    la ra, _stall
    #each pmpcfg register is divided into 4 pmpXcfg. We are writing 0b11111 pmpcfg0. So we are clearing pmp0cfg to 1(NATO + RWX)
    li t0, 0x1F
    csrw pmpcfg0, t0
    #we are setting the corresponding addr for pmp0cfg to -1(Full memory)
    li t0, -1
    csrw pmpaddr0, t0
    sfence.vma        #force CPU to flush cache and grab a fresh copy as table in cache might be outdated
    sret

_stall:
    wfi
    j _stall

.include "src/kernel/asm/trap.asm"
.include "src/kernel/asm/user.asm"
