/**
 * MAestro
 * @file task_allocation.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Task allocation protocol
 */

#include <task_allocation.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <task_control.h>
#include <dmni.h>
#include <mmr.h>

int talloc_alloc(talloc_t *alloc)
{
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL)
        return -ENOMEM;

    list_entry_t *entry = tcb_push_back(tcb);
    if (entry == NULL) {
        free(tcb);
        return -ENOMEM;
    }

    /* Initializes the TCB */
	tcb_alloc(
		tcb, 
		alloc->task, 
		alloc->text_size, 
		alloc->data_size, 
		alloc->bss_size, 
		alloc->mapper_task, 
		alloc->mapper_address,
		(void*)(alloc->entry_point)
	);

	// printf("Text size: %u\n", alloc->text_size);
	// printf("Data size: %u\n", alloc->data_size);
	// printf("BSS size:  %u\n", alloc->bss_size);

    /* Obtain the program code */
	size_t text_recv = dmni_recv(tcb_get_offset(tcb), ((alloc->text_size + 3) & ~3));
	if (text_recv < 0)
		return text_recv;

	/* Obtain program data */
	size_t data_recv = dmni_recv((void*)(0x01000000 | (unsigned)(tcb_get_offset(tcb))), ((alloc->data_size + 3) & ~3));
	if (data_recv < 0)
		return data_recv;

	// printf("Received %d bytes of text and %d bytes of data\n", text_recv, data_recv);

	printf(
		"Task id %d allocated at %d with entry point %lx and offset %p\n", 
		alloc->task, 
		MMR_RTC_MTIME, 
		alloc->entry_point,
		tcb_get_offset(tcb)
	);

	// printf(
	// 	"Mapper task: %d, Mapper address: %x\n",
	// 	alloc->mapper_task,
	// 	alloc->mapper_address
	// );

    if ((int8_t)(alloc->mapper_task) == -1) {
		// printf("Releasing now!\n");
        /* Task came from Injector directly. Release immediately */
		sched_t *sched = tcb_get_sched(tcb);

		if(sched == NULL)
			sched = sched_emplace_back(tcb);

		if (sched == NULL)
			return -ENOMEM;

		return sched_is_idle();
    }

    /* Sends task allocated to mapper */
    return tcb_send_allocated(tcb);
}
