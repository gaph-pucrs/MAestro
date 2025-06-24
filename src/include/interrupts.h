/**
 * MAestro
 * @file interrupts.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Declares the interrupts procedures of the kernel.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <memphis/monitor.h>

#include <task_control.h>
#include <broadcast.h>

enum RISCV_IRQ_FLAGS {
	RISCV_IRQ_MTI = 7,
	RISCV_IRQ_MEI = 11
};

/**
 * @brief Function called by the HAL interruption handler.
 * 
 * @details It cannot send a packet when the DMNI is already sending a packet.
 * This function implementation should assure this behavior.
 * 
 * @param status Status of the interruption. Signals the interruption type.
 * 
 * @return Pointer to the scheduled task
 */
tcb_t *isr_dispatcher(unsigned status);
