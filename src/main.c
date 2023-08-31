/**
 * MA-Memphis
 * @file main.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Initialization procedures of the kernel.
 */

#include "mmr.h"
#include "hal.h"
#include "pending_service.h"
#include "task_migration.h"
#include "stdio.h"
#include "llm.h"
#include "interrupts.h"
#include "pending_msg.h"
#include "paging.h"
#include "application.h"

int main()
{
	printf("Initializing PE %x\n", MMR_NI_CONFIG);

	page_init();
	app_init();
	tcb_init();
	sched_init();
	pmsg_init();
	psvc_init();
	tm_init();
	llm_init();
	pkt_init();

	/**
	 * @todo Change this to properly address PLIC changes
	*/
	// MMR_IRQ_MASK = (
	// 	IRQ_BRNOC |
	// 	IRQ_SCHEDULER | 
	// 	IRQ_NOC | 
	// 	IRQ_PENDING_SERVICE
	// );

	/**
	 * @brief RS5-only: allocate initial app (already in memory)
	 */
	tcb_t *tcb = malloc(sizeof(tcb_t));

	if(tcb == NULL){
		puts("FATAL: could not allocate TCB");
		while(true);
	}

	list_entry_t *entry = tcb_push_back(tcb);
	if(entry == NULL){
		puts("FATAL: could not allocate TCB");
		while(true);
	}

	/* Initializes the TCB */
	tcb_alloc(
		tcb, 
		256, 
		0,	/* FIX */ 
		0,	/* FIX */ 
		0,	/* FIX */ 
		-1, 
		-1,
		0x5c	/* FIX */ 
	);

	sched_t *sched = sched_emplace_back(tcb);

	if(sched == NULL){
		puts("FATAL: unable to allocate scheduler");
		while(true);
	}

	/* Enable timer interrupt to auto-schedule the task that is idle right now */
	MMR_RTC_MTIME = 0;
	MMR_RTC_MTIMEH = 0;
	MMR_RTC_MTIMECMP = 100000; /* 1 ms */
	MMR_RTC_MTIMECMPH = 0;

	return 0;
}
