/**
 * 
 * @file data_available.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Defines the DATA_AV fifo functions.
 */

#include <stddef.h>

#include "data_available.h"
#include "services.h"
#include "packet.h"

void data_av_init(tcb_t *tcb)
{
	tcb->data_av.head = 0;
	tcb->data_av.tail = 0;
	tcb->data_av.empty = true;
	tcb->data_av.full = false;
}

bool data_av_insert(tcb_t *tcb, int producer_task, int producer_addr)
{
	if(tcb->data_av.full){
		puts("ERROR: data_av fifo full\n");
		return false;
	}

	tcb->data_av.buffer[tcb->data_av.tail].requester = producer_task;
	tcb->data_av.buffer[tcb->data_av.tail].requester_addr = producer_addr;

	tcb->data_av.tail++;
	tcb->data_av.tail %= PKG_MAX_TASKS_APP;

	tcb->data_av.full = (tcb->data_av.tail == tcb->data_av.head);
	tcb->data_av.empty = false;

	/** @todo Only for debug purposes */
	// MemoryWrite(ADD_DATA_AV_DEBUG, (producer_task << 16) | (consumer_task & 0xFFFF));

	return true;
}

void data_av_send(int consumer_task, int producer_task, int consumer_addr, int producer_addr)
{
	packet_t *packet = pkt_slot_get();

	packet->header = consumer_addr;
	packet->service = DATA_AV;
	packet->producer_task = producer_task;
	packet->consumer_task = consumer_task;
	packet->requesting_processor = producer_addr;

	pkt_send(packet, 0, 0);
}

data_av_t *data_av_pop(tcb_t *tcb)
{
	if(tcb->data_av.empty)
		return NULL;

	data_av_t *ret = &(tcb->data_av.buffer[tcb->data_av.head++]);
	tcb->data_av.head %= PKG_MAX_TASKS_APP;

	if(tcb->data_av.head == tcb->data_av.tail)
		tcb->data_av.empty = true;

	tcb->data_av.full = false;

	/** @todo Only for debug purposes */
	// MemoryWrite(REM_DATA_AV_DEBUG, (data_av[i].requester << 16) | (consumer_task & 0xFFFF));

	return ret;
}

data_av_t *data_av_peek(tcb_t *tcb)
{
	if(tcb->data_av.empty)
		return NULL;

	return &(tcb->data_av.buffer[tcb->data_av.head]);
}