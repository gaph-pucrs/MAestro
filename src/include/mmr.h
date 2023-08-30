/**
 * MA-Memphis
 * @file mmr.h
 * 
 * @author Unknown
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date Unknown
 * 
 * @brief Define memory mapped registers adressing.
 */

#pragma once

#define MMR_RTC_MTIME				(*(volatile unsigned int*)0x20000000U)
#define MMR_RTC_MTIMEH				(*(volatile unsigned int*)0x20000004U)
#define MMR_RTC_MTIMECMP			(*(volatile unsigned int*)0x20000008U)
#define MMR_RTC_MTIMECMPH			(*(volatile unsigned int*)0x2000000CU)

/* IRQ_STATUS = PLIC IP */
#define MMR_IRQ_STATUS				(*(volatile unsigned int*)0x40000004U)
/* IRQ_MASK   = PLIC IE */
#define MMR_IRQ_MASK 				(*(volatile unsigned int*)0x40000008U)

/* Memphis MMR */
#define MMR_PAGE_SIZE				(*(volatile unsigned int*)0x40200000U)
#define MMR_NI_CONFIG				(*(volatile unsigned int*)0x40200004U)
#define MMR_MAX_LOCAL_TASKS			(*(volatile unsigned int*)0x40200008U)
#define MMR_N_PE_X      			(*(volatile unsigned int*)0x4020000CU)
#define MMR_N_PE_Y      			(*(volatile unsigned int*)0x40200010U)
#define MMR_PENDING_SERVICE_INTR	(*(volatile unsigned int*)0x40200400U)

/* Memphis Peripherals */
#define MMR_DMNI_SIZE 				(*(volatile unsigned int*)0x40400200U)
#define MMR_DMNI_SIZE_2 			(*(volatile unsigned int*)0x40400204U)
#define MMR_DMNI_ADDRESS			(*(volatile unsigned int*)0x40400210U)
#define MMR_DMNI_ADDRESS_2 			(*(volatile unsigned int*)0x40400214U)
#define MMR_DMNI_OP					(*(volatile unsigned int*)0x40400220U)
#define MMR_DMNI_START				(*(volatile unsigned int*)0x40400230U)
#define MMR_DMNI_SEND_ACTIVE		(*(volatile unsigned int*)0x40400250U)
#define MMR_DMNI_RECEIVE_ACTIVE		(*(volatile unsigned int*)0x40400260U)
#define MMR_MEM_REG_PERIPHERALS     (*(volatile unsigned int*)0x40400500U)
#define MMR_BR_LOCAL_BUSY     		(*(volatile unsigned int*)0x40400600U)
#define MMR_BR_PAYLOAD     			(*(volatile unsigned int*)0x40400604U)
#define MMR_BR_TARGET     			(*(volatile unsigned int*)0x40400608U)
#define MMR_BR_SERVICE     			(*(volatile unsigned int*)0x4040060CU)
#define MMR_BR_START     			(*(volatile unsigned int*)0x40400610U)
#define MMR_BR_HAS_MESSAGE			(*(volatile unsigned int*)0x40400614U)
#define MMR_BR_READ_PAYLOAD			(*(volatile unsigned int*)0x40400618U)
#define MMR_MON_PTR_QOS				(*(volatile unsigned int*)0x4040061CU)
#define MMR_MON_PTR_PWR				(*(volatile unsigned int*)0x40400620U)
#define MMR_MON_PTR_2				(*(volatile unsigned int*)0x40400624U)
#define MMR_MON_PTR_3				(*(volatile unsigned int*)0x40400628U)
#define MMR_MON_PTR_4				(*(volatile unsigned int*)0x4040062CU)
#define MMR_BR_PRODUCER				(*(volatile unsigned int*)0x40400630U)
#define MMR_DMNI_CLEAR_MONITOR		(*(volatile unsigned int*)0x40400634U)
#define MMR_BR_READ_PRODUCER		(*(volatile unsigned int*)0x40400638U)
#define MMR_BR_KSVC					(*(volatile unsigned int*)0x4040063CU)
#define MMR_BR_READ_KSVC			(*(volatile unsigned int*)0x40400640U)
#define MMR_BR_POP					(*(volatile unsigned int*)0x40400644U)

/* Testbench MMR */
#define MMR_END_SIM 				(*(volatile unsigned int*)0x80000000U)
#define MMR_UART_CHAR               (*(volatile unsigned int*)0x80001000U)
#define MMR_TASK_TERMINATED			(*(volatile unsigned int*)0x80200000U)
#define MMR_SCHEDULING_REPORT		(*(volatile unsigned int*)0x80200270U)
#define MMR_ADD_PIPE_DEBUG			(*(volatile unsigned int*)0x80200280U)
// #define MMR_REM_PIPE_DEBUG 			(*(volatile unsigned int*)0x80200284U)
// #define MMR_ADD_REQUEST_DEBUG 		(*(volatile unsigned int*)0x80200290U)
// #define MMR_REM_REQUEST_DEBUG		(*(volatile unsigned int*)0x80200294U)
// #define MMR_SLACK_TIME_MONITOR 		(*(volatile unsigned int*)0x80200370U)

/* @todo: change below */
// #define MMR_UART_DATA 				(*(volatile unsigned int*)0x20000000U)	//!< Read/Write data from/to UART
// #define MMR_UART_LEN 				(*(volatile unsigned int*)0x20000008U)	//!< Read/Write data from/to UART
// #define MMR_UART_START 				(*(volatile unsigned int*)0x2000000CU)	//!< Read/Write data from/to UART
