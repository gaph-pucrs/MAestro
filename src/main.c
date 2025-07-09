/**
 * MAestro
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

#include <stdio.h>

#include <mmr.h>
#include <paging.h>
#include <application.h>
#include <task_control.h>
#include <task_scheduler.h>
#include <kernel_pipe.h>
#include <mpipe.h>
#include <message.h>
#include <task_migration.h>
#include <llm.h>

int main()
{
	printf("Initializing PE %x\n", MMR_DMNI_INF_ADDRESS);

	page_init();
	app_init();
	tcb_init();
	sched_init();
	kpipe_init();
	msg_pndg_init();
	tm_init();
	llm_init();
	mpipe_init();

	MMR_DMNI_IRQ_IE = ((1 << DMNI_IE_PENDING) | (1 << DMNI_IE_BRLITE) | (1 << DMNI_IE_HERMES));
	MMR_PLIC_IE     = (1 << PLIC_IE_DMNI);

	return 0;
}
