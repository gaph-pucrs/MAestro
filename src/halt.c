/**
 * MAestro
 * @file halt.c
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Gracefully halts the PE
 */

#include <halt.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <task_location.h>
#include <task_migration.h>
#include <message.h>
#include <mmr.h>
#include <kernel_pipe.h>

#include <memphis/services.h>
#include <memphis/messaging.h>

tl_t *_halter = NULL;

bool halt_pndg()
{
    return (_halter != NULL);
}

int halt_try()
{
	// printf("Trying to halt PE...\n");
	if (!kpipe_empty()) {
		// printf("Not halting due to pending message\n");
		return -EAGAIN;
	}

	/* Check if there are migrated tasks in the list and halt later */
	if (!(tm_empty() && msg_pndg_empty()))
		return -EAGAIN;
	
	/* Inform the mapper that this PE is ready to halt */
	memphis_info_t pe_halted;
	pe_halted.service = PE_HALTED;
	pe_halted.addr    = MMR_DMNI_INF_ADDRESS;
	kpipe_add(
		&pe_halted, 
		sizeof(pe_halted), 
		tl_get_task(_halter), 
		tl_get_addr(_halter)
	);

	return 0;
}

int halt_set(int task, int addr)
{
	_halter = malloc(sizeof(tl_t));
	if (_halter == NULL)
		return -ENOMEM;

	tl_set(_halter, task, addr);
	return 0;
}

void halt_clear()
{
    free(_halter);
    _halter = NULL;
}
