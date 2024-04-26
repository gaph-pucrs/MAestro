##
# MA-Memphis
# @file hal.s
#
# @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
# GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
# PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
#
# @date September 2019
#
# @brief Hardware abstraction layer for OS bootloader, syscalls, interrupts and
# context switching
##

#include <hal.h>

.equ INTR_MASK,  0x80000000
.equ ECALL_MASK, 0x00000008

.equ HAL_REG_PC, 	HAL_MAX_REGISTERS
.equ HAL_REG_PAGE, (HAL_MAX_REGISTERS+1)

.equ mvmdo,  0x7C0
.equ mvmds,  0x7C1
.equ mvmio,  0x7C2
.equ mvmis,  0x7C3
.equ mvmctl, 0x7C4
.equ mvmdm,  0x7C5
.equ mvmim,  0x7C6

.section .init
.align 4

.globl _start
_start:

	# Configure system status MPP=0, MPIE=0, MIE=0
	csrw	mstatus, zero

	# Clear pending interrupts
	csrw	mip, zero

	# Enable MEI and when unmasked (if MIE=1)
	li		t0,  0x800
	csrw	mie, t0

	.option push
    .option norelax
    la		gp, __global_pointer$
    .option pop

	# Configure Syscall
	la		t1,vector_entry		# Load the vector_entry address to t0
	csrw	mtvec,t1			# Write vector_entry address to mtvec
								# Last bit is 0, means DIRECT mode

	# Load data memory size to sp
	li		t2, MMR_DMNI_BASE
	lw		sp, 0x20(t2)

	# Configure virtual memory data size mask
	addi 	sp, sp, -1
	csrw	mvmds, sp
	addi	sp, sp, 1

	# Set stack to top of data memory
	li		t4, MMR_DATA_BASE
	or		sp, sp, t4
	
	# Load instruction memory size
	lw		t5, 0x1C(t2)
	addi 	t5, t5, -1
	csrw	mvmis, t5

	# Enable XOSVM
	li		t3, 1
	csrw	mvmctl, t3

	# Set XOSVM allowed mask
	csrw	mvmim, zero
	li		t6, MMR_DATA_BASE
	csrw 	mvmdm, t6

	# Clear the BSS
	la a0, _edata
	la a2, _end
	sub a2, a2, a0
	li a1, 0
	call memset

	# Initialize newlib
	la a0, __libc_fini_array
	call atexit
	call __libc_init_array

	# Clear argc, argv, envp
	li a0, 0
	li a1, 0
	li a2, 0

	jal		main

	# Save kernel sp to mscratch
	csrw	mscratch, sp

idle:
	# Globally enable interrupts
	li		t0, 0x8
	csrs	mstatus, t0

wfi_loop:
	wfi
	j		wfi_loop

.section .text
.align 4

vector_entry:					# Set to mtvec.BASE and DIRECT	
	# Swap task sp with kernel sp
	csrrw	sp, mscratch, sp

	# Save gp and t0 to stack
	addi	sp, sp, -8
	sw		gp, 4(sp)
	sw		s0, 0(sp)

	# Load kernel gp
	.option push
    .option norelax
    la		gp, __global_pointer$
    .option pop

	# Load "current" to s0
	lw		s0, current	# Get current
	# If scheduled task was 'idle', no need to save minimum context
	# And is obviously an interruption because idle cant call
	# When interrupting idle, there is no need to save context
	bnez	s0, save_minimum

	addi	sp, sp, 8	# Pop s0 and gp from stack
	j 		isr_entry

save_minimum:
	# Else, if running task, save caller registers
	# Callee registers are saved by the ABI.
	# Despite being a call, syscall needs to save everything (except a0, which will hold the return value)

	sw		ra, (HAL_REG_RA*4)(s0)
	sw		t0, (HAL_REG_T0*4)(s0)
	sw		t1, (HAL_REG_T1*4)(s0)
	sw		t2, (HAL_REG_T2*4)(s0)
	# a0 will be modified by syscall
	sw		a1, (HAL_REG_A1*4)(s0)
	sw		a2, (HAL_REG_A2*4)(s0)
	sw		a3, (HAL_REG_A3*4)(s0)
	sw		a4, (HAL_REG_A4*4)(s0)
	sw		a5, (HAL_REG_A5*4)(s0)
	sw		a6, (HAL_REG_A6*4)(s0)
	sw		a7, (HAL_REG_A7*4)(s0)
	sw		t3, (HAL_REG_T3*4)(s0)
	sw		t4, (HAL_REG_T4*4)(s0)
	sw		t5, (HAL_REG_T5*4)(s0)
	sw		t6, (HAL_REG_T6*4)(s0)

	csrr	t3, mscratch			# Load sp that is in mscratch
	sw		t3, (HAL_REG_SP*4)(s0)	# Save sp to allow stack check by kernel

	csrr	t4, mepc		# Always save mepc because we must add 4 before returning
	addi	t4, t4, 4
	sw		t4, (HAL_REG_PC*4)(s0)
	
	# Check if it was ecall
	csrr	t0, mcause					# Load mcause
	li		t1, INTR_MASK				# Bit 31 (interrupt)
	and		t1, t1, t0					# t1 = mcause AND interrupt mask
	bnez	t1, intr_handler			# If INTR mask is true, jump to interrupt handler

	li 		t2, ECALL_MASK				# Bit 3 (ecall)
	and 	t2, t2, t0					# t2 = mcause AND ecall mask
	bnez	t2, ecall_handler			# If ECALL mask is true, jump to ecall handler
	
	# If its neither interrupt not ecall, it is exception
	# We can lose the context here.
	# For now, all exceptions will be fatal.
	csrr	 a0, mcause
	csrr	 a1, mtval
	csrr	 a2, mepc
	
	jal hal_exception_handler
	# It will return the next scheduled task

	j restore_complete

intr_handler:
	# An interruption breaks the task control
	# So it is needed to save the caller-registers, because the callee-registers 
	# are saved by the ABI. Caller registers are already saved at this point.
	# To ease the migration process, save ALL, and not just caller registers.
	# Therefore, save callee registers here.

	# sp is already saved

	lw 		 t0,  4(sp)
	sw		 t0, (HAL_REG_GP*4)(s0) # Save gp that is in stack

	lw		 t1,  0(sp)
	sw		 t1, (HAL_REG_S0*4)(s0) # Save s0 that is in stack

	addi	 sp, sp, 8  # Pop s0 and gp from stack

	sw		 s1,  (HAL_REG_S1*4)(s0)
	sw		 a0,  (HAL_REG_A0*4)(s0)	# a0 was not saved before
	sw		 s2,  (HAL_REG_S2*4)(s0)
	sw		 s3,  (HAL_REG_S4*4)(s0)
	sw		 s4,  (HAL_REG_S4*4)(s0)
	sw		 s5,  (HAL_REG_S5*4)(s0)
	sw		 s6,  (HAL_REG_S6*4)(s0)
	sw		 s7,  (HAL_REG_S7*4)(s0)
	sw		 s8,  (HAL_REG_S8*4)(s0)
    sw		 s9,  (HAL_REG_S9*4)(s0)
	sw		s10, (HAL_REG_S10*4)(s0)
	sw		s11, (HAL_REG_S11*4)(s0)

isr_entry:
	# JUMP TO INTERRUPT SERVICE ROUTINE WITH ARGS
	csrr	t0, mie			
	csrr	t1, mip
	and		a0, t0, t1		# Function arg
	jal		isr_isr
	# The function returned the scheduled task pointer in a0

	# Save kernel context
	csrw	mscratch, sp	# Save sp to mscratch -- it will not be used anymore

	# If the idle task was scheduled, no need to restore the context
	bnez	a0, should_restore_isr
	j 		idle
should_restore_isr:
	# Else if the same task that was interrupted is scheduled, restore only needed
	beq		a0, s0, restore_minimum

	# Else if a new task was scheduled, restore a bit more of the context
	lw		 s1,  (HAL_REG_S1*4)(a0)
	lw		 s2,  (HAL_REG_S2*4)(a0)
	lw		 s3,  (HAL_REG_S3*4)(a0)
	lw		 s4,  (HAL_REG_S4*4)(a0)
	lw		 s5,  (HAL_REG_S5*4)(a0)
	lw		 s6,  (HAL_REG_S6*4)(a0)
	lw		 s7,  (HAL_REG_S7*4)(a0)
	lw		 s8,  (HAL_REG_S8*4)(a0)
    lw		 s9,  (HAL_REG_S9*4)(a0)
	lw		s10, (HAL_REG_S10*4)(a0)
	lw		s11, (HAL_REG_S11*4)(a0)

	# Load offset from scheduled task
	lw		t1, (HAL_REG_PAGE*4)(a0)
	lw		t1, 0(t1)
	csrw	mvmdo, t1
	csrw	mvmio, t1

	# Continue to restore the remaining context
restore_minimum:
	# Load epc from scheduled task
	lw		t0, (HAL_REG_PC*4)(a0)
	csrw	mepc, t0

	/* MPPRIV = user, MPIE = en */
	li		t1, 0x80
	csrw	mstatus, t1

	lw		 ra, (HAL_REG_RA*4)(a0)
	lw		 sp, (HAL_REG_SP*4)(a0)
	lw		 gp, (HAL_REG_GP*4)(a0)
	lw		 t0, (HAL_REG_T0*4)(a0)
	lw		 t1, (HAL_REG_T1*4)(a0)
	lw		 t2, (HAL_REG_T2*4)(a0)
	lw		 s0, (HAL_REG_S0*4)(a0)	# Only restore s0 because we used to store current.
	lw		 a1, (HAL_REG_A1*4)(a0)
	lw		 a2, (HAL_REG_A2*4)(a0)
	lw		 a3, (HAL_REG_A3*4)(a0)
	lw		 a4, (HAL_REG_A4*4)(a0)
	lw		 a5, (HAL_REG_A5*4)(a0)
	lw		 a6, (HAL_REG_A6*4)(a0)
	lw		 a7, (HAL_REG_A7*4)(a0)
	lw		 t3, (HAL_REG_T3*4)(a0)
	lw		 t4, (HAL_REG_T4*4)(a0)
	lw		 t5, (HAL_REG_T5*4)(a0)
	lw		 t6, (HAL_REG_T6*4)(a0)
	# No need to restore remaining registers. ABI takes care of this.

	lw		 a0, (HAL_REG_A0*4)(a0)

	mret

ecall_handler:
	jal		 sys_syscall

	lb		 t0, task_terminated
	bnez	 t0, restore_complete # If the called syscall terminated the calling task, do not save its context

	beq		 a0, s0, ecall_return	# If scheduled the same TCB, simply return

	# Otherwise it is needed to save the previous task (s0) context
	# Caller registers and sp are already saved

	lw 		 t1, 4(sp)
	sw		 t1, (HAL_REG_GP*4)(s0) # Save gp that is in stack

	lw		 t2, 0(sp)
	sw		 t2, (HAL_REG_S0*4)(s0) # Save s0 that is in stack

	addi	 sp, sp, 8  # Pop s0 and gp from stack

	sw		 s1,  (HAL_REG_S1*4)(s0)
	sw		 s2,  (HAL_REG_S2*4)(s0)
	sw		 s3,  (HAL_REG_S3*4)(s0)
	sw		 s4,  (HAL_REG_S4*4)(s0)
	sw		 s5,  (HAL_REG_S5*4)(s0)
	sw		 s6,  (HAL_REG_S6*4)(s0)
	sw		 s7,  (HAL_REG_S7*4)(s0)
	sw		 s8,  (HAL_REG_S8*4)(s0)
    sw		 s9,  (HAL_REG_S9*4)(s0)
	sw		s10, (HAL_REG_S10*4)(s0)
	sw		s11, (HAL_REG_S11*4)(s0)

restore_complete:
	csrw	 mscratch, sp	# Save kernel sp

	# If the idle task was scheduled, no need to restore the context
	bnez	 a0, should_restore_complete
	j 		 idle
should_restore_complete:
	# Otherwise, restore context of scheduled task
	# We don't know if the scheduled task stopped on a call or intr, so restore everything
	lw		 t0, (HAL_REG_PC*4)(a0)
	csrw	 mepc, t0			# Load task PC

	lw		 t1, (HAL_REG_PAGE*4)(a0)
	lw		 t1, 0(t1)
	csrw	mvmdo, t1
	csrw	mvmio, t1

	/* MPPRIV = user, MPIE = en */
	li		t2, 0x80
	csrw	mstatus, t2

	lw		 ra,  (HAL_REG_RA*4)(a0)
	lw		 sp,  (HAL_REG_SP*4)(a0)
	lw		 gp,  (HAL_REG_GP*4)(a0)
	lw		 t0,  (HAL_REG_T0*4)(a0)
	lw		 t1,  (HAL_REG_T1*4)(a0)
	lw		 t2,  (HAL_REG_T2*4)(a0)
	lw		 s0,  (HAL_REG_S0*4)(a0)
	lw		 s1,  (HAL_REG_S1*4)(a0)
	# a0 is being used
	lw		 a1,  (HAL_REG_A1*4)(a0)
	lw		 a2,  (HAL_REG_A2*4)(a0)
	lw		 a3,  (HAL_REG_A3*4)(a0)
	lw		 a4,  (HAL_REG_A4*4)(a0)
	lw		 a5,  (HAL_REG_A5*4)(a0)
	lw		 a6,  (HAL_REG_A6*4)(a0)
	lw		 a7,  (HAL_REG_A7*4)(a0)
	lw		 s2,  (HAL_REG_S2*4)(a0)
	lw		 s3,  (HAL_REG_S3*4)(a0)
	lw		 s4,  (HAL_REG_S4*4)(a0)
	lw		 s5,  (HAL_REG_S5*4)(a0)
	lw		 s6,  (HAL_REG_S6*4)(a0)
	lw		 s7,  (HAL_REG_S7*4)(a0)
	lw		 s8,  (HAL_REG_S8*4)(a0)
    lw		 s9,  (HAL_REG_S9*4)(a0)
	lw		s10, (HAL_REG_S10*4)(a0)
	lw		s11, (HAL_REG_S11*4)(a0)
	lw		 t3,  (HAL_REG_T3*4)(a0)
	lw		 t4,  (HAL_REG_T4*4)(a0)
	lw		 t5,  (HAL_REG_T5*4)(a0)
	lw		 t6,  (HAL_REG_T6*4)(a0)

	lw		 a0,  (HAL_REG_A0*4)(a0)
	
	mret

ecall_return:
	# Restore minimum context
	# Need to restore all caller-saved registers
	# Callee registers are taken care by the ABI (except s0)
	lw		 t0, (HAL_REG_PC*4)(a0)
	csrw	 mepc, t0			# Load task PC

	/* MPPRIV = user, MPIE = en */
	li		t1, 0x80
	csrw	mstatus, t1

	lw		 ra, (HAL_REG_RA*4)(s0)
	lw		 t0, (HAL_REG_T0*4)(s0)
	lw		 t1, (HAL_REG_T1*4)(s0)
	lw		 t2, (HAL_REG_T2*4)(s0)
	lw		 a0, (HAL_REG_A0*4)(s0)
	lw 		 a1, (HAL_REG_A1*4)(s0)
	lw		 a2, (HAL_REG_A2*4)(s0)
	lw		 a3, (HAL_REG_A3*4)(s0)
	lw		 a4, (HAL_REG_A4*4)(s0)
	lw		 a5, (HAL_REG_A5*4)(s0)
	lw		 a6, (HAL_REG_A6*4)(s0)
	lw		 a7, (HAL_REG_A7*4)(s0)
	lw		 t3, (HAL_REG_T3*4)(s0)
	lw		 t4, (HAL_REG_T4*4)(s0)
	lw		 t5, (HAL_REG_T5*4)(s0)
	lw		 t6, (HAL_REG_T6*4)(s0)

	lw 		 gp,  4(sp)	# Restore gp that is in stack
	lw		 s0,  0(sp) # Restore s0 that is in stack
	addi	 sp, sp, 8	# Pop stack
	csrrw	 sp, mscratch, sp	# Swap back kernel sp with task sp, faster than loading from memory

	mret
