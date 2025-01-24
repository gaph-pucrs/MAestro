/**
 * MAestro
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

/* RTC MMR */
#define MMR_RTC_MTIME				(*(volatile unsigned int*)0x02000000U)
#define MMR_RTC_MTIMEH				(*(volatile unsigned int*)0x02000004U)
#define MMR_RTC_MTIMECMP			(*(volatile unsigned int*)0x02000008U)
#define MMR_RTC_MTIMECMPH			(*(volatile unsigned int*)0x0200000CU)

/* PLIC MMR */
#define MMR_PLIC_IP					(*(volatile unsigned int*)0x04001000U)
#define MMR_PLIC_IE					(*(volatile unsigned int*)0x04002000U)
#define MMR_PLIC_ID					(*(volatile unsigned int*)0x04200004U)

/* DMNI MMR */
#define MMR_DMNI_STATUS				(*(volatile unsigned int*)0x08000000U)
#define MMR_DMNI_IP					(*(volatile unsigned int*)0x08000004U)
#define MMR_DMNI_PENDING_SVC        (*(volatile unsigned int*)0x08000008U)
#define MMR_DMNI_REL_PERIPHERAL		(*(volatile unsigned int*)0x0800000CU)
#define MMR_DMNI_ADDRESS			(*(volatile unsigned int*)0x08000010U)
#define MMR_DMNI_MANYCORE_SIZE		(*(volatile unsigned int*)0x08000014U)
#define MMR_DMNI_MAX_LOCAL_TASKS	(*(volatile unsigned int*)0x08000018U)
#define MMR_DMNI_IMEM_PAGE_SIZE		(*(volatile unsigned int*)0x0800001CU)
#define MMR_DMNI_DMEM_PAGE_SIZE		(*(volatile unsigned int*)0x08000020U)
#define MMR_DMNI_HERMES_START		(*(volatile unsigned int*)0x08000024U)
#define MMR_DMNI_HERMES_OPERATION	(*(volatile unsigned int*)0x08000028U)
#define MMR_DMNI_HERMES_SIZE		(*(volatile unsigned int*)0x0800002CU)
#define MMR_DMNI_HERMES_SIZE_2		(*(volatile unsigned int*)0x08000030U)
#define MMR_DMNI_HERMES_ADDRESS		(*(volatile unsigned int*)0x08000034U)
#define MMR_DMNI_HERMES_ADDRESS_2	(*(volatile unsigned int*)0x08000038U)
#define MMR_DMNI_BRLITE_START		(*(volatile unsigned int*)0x08000040U)
#define MMR_DMNI_BRLITE_SERVICE		(*(volatile unsigned int*)0x08000044U)
#define MMR_DMNI_BRLITE_KSVC		(*(volatile unsigned int*)0x08000048U)
#define MMR_DMNI_BRLITE_TARGET		(*(volatile unsigned int*)0x0800004CU)
#define MMR_DMNI_BRLITE_PRODUCER	(*(volatile unsigned int*)0x08000050U)
#define MMR_DMNI_BRLITE_PAYLOAD		(*(volatile unsigned int*)0x08000054U)
#define MMR_DMNI_BRSVC_POP			(*(volatile unsigned int*)0x08000058U)
#define MMR_DMNI_BRSVC_KSVC			(*(volatile unsigned int*)0x0800005CU)
#define MMR_DMNI_BRSVC_PRODUCER		(*(volatile unsigned int*)0x08000060U)
#define MMR_DMNI_BRSVC_PAYLOAD		(*(volatile unsigned int*)0x08000064U)
#define MMR_DMNI_BRMON_CLEAR		(*(volatile unsigned int*)0x08000068U)
#define MMR_DMNI_BRMON_QOS_PTR		(*(volatile unsigned int*)0x0800006CU)
#define MMR_DMNI_BRMON_SEC_PTR		(*(volatile unsigned int*)0x08000070U)
#define MMR_DMNI_RCV_TIMESTAMP		(*(volatile unsigned int*)0x08000074U)

/* DEBUG MMR */
#define MMR_DBG_PUTC				(*(volatile unsigned int*)0x80000000U)
#define MMR_DBG_HALT				(*(volatile unsigned int*)0x80000004U)
#define MMR_DBG_TERMINATE			(*(volatile unsigned int*)0x80000008U)
#define MMR_DBG_SCHED_REPORT		(*(volatile unsigned int*)0x80000010U)
#define MMR_DBG_ADD_PIPE			(*(volatile unsigned int*)0x80000020U)
#define MMR_DBG_REM_PIPE			(*(volatile unsigned int*)0x80000024U)
#define MMR_DBG_ADD_REQ				(*(volatile unsigned int*)0x80000030U)
#define MMR_DBG_REM_REQ				(*(volatile unsigned int*)0x80000034U)
#define MMR_DBG_ADD_DAV				(*(volatile unsigned int*)0x80000040U)
#define MMR_DBG_REM_DAV				(*(volatile unsigned int*)0x80000044U)
#define MMR_DBG_SAFE_TIMESTAMP		(*(volatile unsigned int*)0x80000050U)
#define MMR_DBG_SAFE_LATENCY		(*(volatile unsigned int*)0x80000054U)
#define MMR_DBG_SAFE_EDGE			(*(volatile unsigned int*)0x80000058U)
#define MMR_DBG_SAFE_ANOMALY		(*(volatile unsigned int*)0x8000005CU)

enum PLIC_IE {
	PLIC_IE_NONE = 0x1,
	PLIC_IE_DMNI = 0x2
};

enum DMNI_STATUS {
	DMNI_STATUS_SEND_ACTIVE		= 0x00000001,
	DMNI_STATUS_RECV_ACTIVE		= 0x00000002,
	DMNI_STATUS_LOCAL_BUSY		= 0x00000004,
	DMNI_STATUS_MON_CLEAR		= 0x00000008,
	DMNI_STATUS_REL_PERIPHERAL	= 0x00000010
};

enum DMNI_IP {
	DMNI_IP_HERMES				= 0x00000001,
	DMNI_IP_BRLITE				= 0x00000002,
	DMNI_IP_PENDING				= 0x00000004
};

enum DMNI_OPERATION {
	DMNI_OPERATION_SEND,
	DMNI_OPERATION_RECEIVE
};
