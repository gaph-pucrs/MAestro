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
#define MMR_DMNI_ADDRESS			(*(volatile unsigned int*)0x08000008U)
#define MMR_DMNI_MANYCORE_SIZE		(*(volatile unsigned int*)0x0800000CU)
#define MMR_DMNI_IMEM_PAGE_SIZE		(*(volatile unsigned int*)0x08000010U)
#define MMR_DMNI_DMEM_PAGE_SIZE		(*(volatile unsigned int*)0x08000014U)
#define MMR_DMNI_HERMES_SIZE		(*(volatile unsigned int*)0x08000018U)
#define MMR_DMNI_HERMES_SIZE_2		(*(volatile unsigned int*)0x0800001CU)
#define MMR_DMNI_HERMES_ADDRESS		(*(volatile unsigned int*)0x08000020U)
#define MMR_DMNI_HERMES_ADDRESS_2	(*(volatile unsigned int*)0x08000024U)
#define MMR_DMNI_BRLITE_KSVC		(*(volatile unsigned int*)0x08000028U)
#define MMR_DMNI_BRLITE_PAYLOAD		(*(volatile unsigned int*)0x0800002CU)
#define MMR_DMNI_RCV_TIMESTAMP		(*(volatile unsigned int*)0x08000030U)

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

#define MMR_DBG_SAFE_SND_TIME		(*(volatile unsigned int*)0x80000050U)
#define MMR_DBG_SAFE_INF_TIME		(*(volatile unsigned int*)0x80000054U)
#define MMR_DBG_SAFE_EDGE			(*(volatile unsigned int*)0x80000058U)
#define MMR_DBG_SAFE_INF_LAT		(*(volatile unsigned int*)0x8000005CU)
#define MMR_DBG_SAFE_LAT_PRED		(*(volatile unsigned int*)0x80000060U)
#define MMR_DBG_SAFE_LAT_MON		(*(volatile unsigned int*)0x80000064U)

enum PLIC_IE {
	PLIC_IE_NONE,
	PLIC_IE_DMNI
};

enum DMNI_STATUS {
	DMNI_STATUS_SEND_ACTIVE,
	DMNI_STATUS_RECV_ACTIVE,
	DMNI_STATUS_LOCAL_BUSY,

	DMNI_STATUS_REL_PERIPHERAL = 4
};

enum DMNI_IP {
	DMNI_IP_HERMES,
	DMNI_IP_BRLITE,
	DMNI_IP_PENDING
};
