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
#define MMR_RTC_MTIME				(*(volatile unsigned int*)0xA0000000U)
#define MMR_RTC_MTIMEH				(*(volatile unsigned int*)0xA0000004U)
#define MMR_RTC_MTIMECMP			(*(volatile unsigned int*)0xA0000008U)
#define MMR_RTC_MTIMECMPH			(*(volatile unsigned int*)0xA000000CU)

/* PLIC MMR */
#define MMR_PLIC_IP					(*(volatile unsigned int*)0x80001000U)
#define MMR_PLIC_IE					(*(volatile unsigned int*)0x80002000U)
#define MMR_PLIC_ID					(*(volatile unsigned int*)0x80200004U)

/* DMNI MMR */
#define MMR_DMNI_IRQ_STATUS			(*(volatile unsigned int*)0x90000000U)
#define MMR_DMNI_IRQ_IE				(*(volatile unsigned int*)0x90000004U)
#define MMR_DMNI_IRQ_IP				(*(volatile unsigned int*)0x90000008U)

#define MMR_DMNI_INF_ADDRESS		(*(volatile unsigned int*)0x90000010U)
#define MMR_DMNI_INF_MANYCORE_SZ	(*(volatile unsigned int*)0x90000014U)
#define MMR_DMNI_INF_IMEM_PAGE_SZ	(*(volatile unsigned int*)0x90000018U)
#define MMR_DMNI_INF_DMEM_PAGE_SZ	(*(volatile unsigned int*)0x9000001CU)

#define MMR_DMNI_HERMES_HEAD		(*(volatile unsigned int*)0x90000020U)
#define MMR_DMNI_HERMES_RECD_CNT	(*(volatile unsigned int*)0x90000024U)
#define MMR_DMNI_HERMES_TIMESTAMP	(*(volatile unsigned int*)0x90000028U)

#define MMR_DMNI_HERMES_SIZE		(*(volatile unsigned int*)0x90000030U)
#define MMR_DMNI_HERMES_SIZE_2		(*(volatile unsigned int*)0x90000034U)
#define MMR_DMNI_HERMES_ADDRESS		(*(volatile unsigned int*)0x90000038U)
#define MMR_DMNI_HERMES_ADDRESS_2	(*(volatile unsigned int*)0x9000003CU)

#define MMR_DMNI_BRLITE_KSVC		(*(volatile unsigned int*)0x90000040U)
#define MMR_DMNI_BRLITE_PAYLOAD		(*(volatile unsigned int*)0x90000044U)

#define MMR_DMNI_MON_BASE			(*(volatile unsigned int*)0x90000050U)
#define MMR_DMNI_MON_SEM_OC			(*(volatile unsigned int*)0x90000054U)
#define MMR_DMNI_MON_SEM_AV			(*(volatile unsigned int*)0x90000058U)
#define MMR_DMNI_MON_FLITS			(*(volatile unsigned int*)0x9000005CU)

/* DEBUG MMR */
#define MMR_DBG_PUTC				(*(volatile unsigned int*)0xF0000000U)
#define MMR_DBG_HALT				(*(volatile unsigned int*)0xF0000004U)
#define MMR_DBG_TERMINATE			(*(volatile unsigned int*)0xF0000008U)
#define MMR_DBG_SCHED_REPORT		(*(volatile unsigned int*)0xF0000010U)
#define MMR_DBG_ADD_PIPE			(*(volatile unsigned int*)0xF0000020U)
#define MMR_DBG_REM_PIPE			(*(volatile unsigned int*)0xF0000024U)
#define MMR_DBG_ADD_REQ				(*(volatile unsigned int*)0xF0000030U)
#define MMR_DBG_REM_REQ				(*(volatile unsigned int*)0xF0000034U)
#define MMR_DBG_ADD_DAV				(*(volatile unsigned int*)0xF0000040U)
#define MMR_DBG_REM_DAV				(*(volatile unsigned int*)0xF0000044U)

#define MMR_DBG_SAFE_SND_TIME		(*(volatile unsigned int*)0xF0000050U)
#define MMR_DBG_SAFE_INF_TIME		(*(volatile unsigned int*)0xF0000054U)
#define MMR_DBG_SAFE_EDGE			(*(volatile unsigned int*)0xF0000058U)
#define MMR_DBG_SAFE_INF_LAT		(*(volatile unsigned int*)0xF000005CU)
#define MMR_DBG_SAFE_LAT_PRED		(*(volatile unsigned int*)0xF0000060U)
#define MMR_DBG_SAFE_LAT_MON		(*(volatile unsigned int*)0xF0000064U)

enum PLIC_IE {
	PLIC_IE_NONE,
	PLIC_IE_DMNI
};

enum DMNI_STATUS {
	DMNI_STATUS_SEND_START,
	DMNI_STATUS_RECV_START,

	DMNI_STATUS_REL_PERIPHERAL = 3,
	DMNI_STATUS_SEND_ACTIVE,
	DMNI_STATUS_RECV_ACTIVE,
	DMNI_STATUS_LOCAL_BUSY,
	DMNI_STATUS_MON_ACTIVE
};

enum DMNI_IE {
	DMNI_IE_HERMES = 1,
	DMNI_IE_BRLITE,
	DMNI_IE_PENDING,
	DMNI_IE_MONITOR
};

enum DMNI_IP {
	DMNI_IP_HERMES = 1,
	DMNI_IP_BRLITE,
	DMNI_IP_PENDING,
	DMNI_IP_MONITOR
};
