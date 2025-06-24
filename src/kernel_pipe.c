/**
 * MAestro
 * @file kernel_pipe.c
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 *
 * @brief Outgoing kernel messages encapsulated in the Memphis messaging API.
 */

#include <kernel_pipe.h>

#include <stdlib.h>
#include <errno.h>

#include <mmr.h>
#include <task_control.h>
#include <message.h>
#include <task_migration.h>

#include <memphis.h>
#include <memphis/services.h>
#include <mutils/list.h>

list_t _kpipe;

/**
 * @brief Finds a opipe in kpipe
 * 
 * @param data Pointer to entry data (opipe_t)
 * @param cmpval Pointer to receiver task (int)
 * 
 * @return bool True if found
 */
bool _kpipe_find_fnc(void *data, void *cmpval);

/**
 * @brief Pushes a pending message to the FIFO
 * 
 * @param buf Pointer to the message
 * @param size Size of the message
 * @param cons_task Consumer task of the message
 * 
 * @return int Number of bytes in the message
 */
int _kpipe_emplace_back(void *buf, size_t size, int receiver);

void kpipe_init()
{
	list_init(&_kpipe);
}

opipe_t *kpipe_find(int receiver)
{
	list_entry_t *entry = list_find(&_kpipe, &receiver, _kpipe_find_fnc);

	if(entry == NULL)
		return NULL;

	return list_get_data(entry);
}

void kpipe_remove(opipe_t *pending)
{
	list_entry_t *entry = list_find(&_kpipe, pending, NULL);

	if(entry != NULL)
		list_remove(&_kpipe, entry);

	free(pending);
}

bool _kpipe_find_fnc(void *data, void *cmpval)
{
	opipe_t *opipe = (opipe_t*)data;
	int receiver = *((int*)cmpval);

	return (opipe_get_receiver(opipe) == receiver);
}

bool kpipe_empty()
{
	return list_empty(&_kpipe);
}

int kpipe_add(void *buf, size_t size, int receiver, int target)
{
	// printf("Kernel writing pending message to task %d with size %d\n", cons_task, size);
	/* Insert message in kernel output message buffer */
	int result = _kpipe_emplace_back(buf, size, receiver);

	if (result != size)
		return -ENOMEM;

	/* Check if local consumer / migrated task */
	tcb_t *recv_tcb = NULL;
	if (target == MMR_DMNI_INF_ADDRESS) {
		recv_tcb = tcb_find(receiver);
		if (recv_tcb == NULL) {
			tl_t *tl = tm_find(receiver);
			if (tl == NULL)
				return -EINVAL;

			target = tl_get_addr(tl);
		}
	}

	if (recv_tcb != NULL) {
		/* Insert the packet to TCB */
		list_t *davs = tcb_get_davs(recv_tcb);
		tl_emplace_back(davs, -1, MMR_DMNI_INF_ADDRESS);
		MMR_DBG_ADD_DAV = (MMR_DMNI_INF_ADDRESS << 16) | (receiver & 0xFFFF);

		/* If the consumer task is waiting for a DATA_AV, release it */
		sched_t *sched = tcb_get_sched(recv_tcb);
		if (sched_is_waiting_dav(sched)) {
			sched_release_wait(sched);
			return sched_is_idle();
		}
	} else {
		/* Send data available to the right processor */
		msg_send_hdshk(MMR_DMNI_INF_ADDRESS, target, -1, receiver, DATA_AV);
		// printf("* %x->%x A\n", MEMPHIS_KERNEL_MSG | MMR_DMNI_ADDRESS, cons_task);
	}

	return 0;
}

int _kpipe_emplace_back(void *buf, size_t size, int receiver)
{
	opipe_t *opipe = malloc(sizeof(opipe_t));

	if(opipe == NULL)
		return -ENOMEM;

	int result = opipe_push(
		opipe, 
		buf, 
		size, 
		receiver
	);

	if (list_push_back(&_kpipe, opipe) == NULL)
		return -ENOMEM;

	return result;
}
