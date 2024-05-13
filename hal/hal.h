/**
 * MAestro
 * @file hal.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date October 2019
 * 
 * @brief Hardware abstraction layer for RISC-V processors.
 */

#pragma once

#include <asm-macros.h>

#define MMR_DATA_BASE	0x01000000U
#define MMR_RTC_BASE 	0x02000000U
#define MMR_PLIC_BASE	0x04000000U
#define MMR_DMNI_BASE	0x08000000U
#define MMR_DBG_BASE	0x80000000U

ENUM_BEGIN
	// HAL_REG_ZERO,
	ENUM_VAL(HAL_REG_RA)
	ENUM_VAL(HAL_REG_SP)
	ENUM_VAL(HAL_REG_GP)
	// HAL_REG_TP,
	ENUM_VAL(HAL_REG_T0)
	ENUM_VAL(HAL_REG_T1)
	ENUM_VAL(HAL_REG_T2)
	ENUM_VAL(HAL_REG_S0)
	ENUM_VAL(HAL_REG_S1)
	ENUM_VAL(HAL_REG_A0)
	ENUM_VAL(HAL_REG_A1)
	ENUM_VAL(HAL_REG_A2)
	ENUM_VAL(HAL_REG_A3)
	ENUM_VAL(HAL_REG_A4)
	ENUM_VAL(HAL_REG_A5)
	ENUM_VAL(HAL_REG_A6)
	ENUM_VAL(HAL_REG_A7)
	ENUM_VAL(HAL_REG_S2)
	ENUM_VAL(HAL_REG_S3)
	ENUM_VAL(HAL_REG_S4)
	ENUM_VAL(HAL_REG_S5)
	ENUM_VAL(HAL_REG_S6)
	ENUM_VAL(HAL_REG_S7)
	ENUM_VAL(HAL_REG_S8)
	ENUM_VAL(HAL_REG_S9)
	ENUM_VAL(HAL_REG_S10)
	ENUM_VAL(HAL_REG_S11)
	ENUM_VAL(HAL_REG_T3)
	ENUM_VAL(HAL_REG_T4)
	ENUM_VAL(HAL_REG_T5)
	ENUM_VAL(HAL_REG_T6)

	ENUM_VAL(HAL_MAX_REGISTERS)
ENUM_END(hal_reg_e)

ENUM_BEGIN
	ENUM_VAL(HAL_EXC_INST_ADDR_MISALG)
	ENUM_VAL(HAL_EXC_INST_ACCS_FLT)
	ENUM_VAL(HAL_EXC_ILLGL_INST)
	ENUM_VAL(HAL_EXC_BREAKPOINT)
	ENUM_VAL(HAL_EXC_LOAD_ADDR_MISALG)
	ENUM_VAL(HAL_EXC_LOAD_ACCS_FLT)
	ENUM_VAL(HAL_EXC_STORE_ADDR_MISALG)
	ENUM_VAL(HAL_EXC_STORE_ACCS_FLT)
	ENUM_VAL(HAL_EXC_ENV_CALL_FROM_U)
	ENUM_VAL(HAL_EXC_ENV_CALL_FROM_S)
	ENUM_VALASSIGN(HAL_EXC_ENV_CALL_FROM_M, 11)
	ENUM_VAL(HAL_EXC_INST_PAGE_FLT)
	ENUM_VAL(HAL_EXC_LOAD_PAGE_FLT)
	ENUM_VALASSIGN(HAL_EXC_STORE_PAGE_FLT, 15)
ENUM_END(hal_exc_e)

#ifndef __ASSEMBLY__

/* Forward Declaration */
typedef struct _tcb tcb_t;

void _hal_enable_mti();
void _hal_disable_mti();

/**
 * @brief Handles an exception
 * 
 * @details Abort the running task and schedules a new one
 * 
 * @param cause Interruption cause -- machine dependent
 * @param value Interruption value -- cause dependent
 * @param pc Program counter where the exception occurred
 * 
 * @return Pointer to the scheduled TCB
 */
tcb_t *hal_exception_handler(unsigned cause, unsigned value, unsigned pc);

#endif
