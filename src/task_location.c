/**
 * MA-Memphis
 * @file task_location.c
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 * 
 * @brief Controls task locations for messaging and mapping
 */

#include "task_location.h"

#include <stdlib.h>

#include <memphis.h>
#include <memphis/services.h>

#include "dmni.h"
#include "broadcast.h"

bool _tl_find_fnc(void *data, void* cmpval)
{
	tl_t *tl = (tl_t*)data;
	int task = *((int*)cmpval);

	return (tl->task == task);
}

tl_t *tl_find(list_t *list, int task)
{
	list_entry_t *entry = list_find(list, &task, _tl_find_fnc);

	if(entry == NULL)
		return NULL;

	return list_get_data(entry);
}

void tl_remove(list_t *list, tl_t *tl)
{
	list_entry_t *entry = list_find(list, tl, NULL);
	
	if(entry == NULL)
		return;
	
	list_remove(list, entry);

	free(tl);
}

tl_t *tl_emplace_back(list_t *list, int task, int addr)
{
	tl_t *tl = malloc(sizeof(tl_t));
	
	if(tl == NULL)
		return NULL;

	tl->task = task;
	tl->addr = addr;

	if(list_push_back(list, tl) == NULL){
		free(tl);
		return NULL;
	}

	return tl;
}

int tl_get_task(tl_t *tl)
{
	return tl->task;
}

int tl_get_addr(tl_t *tl)
{
	return tl->addr;
}

void tl_send_dav(tl_t *dav, int cons_task, int cons_addr)
{
	packet_t *packet = pkt_slot_get();

	pkt_set_data_av(packet, cons_addr, dav->task, cons_task, dav->addr);

	dmni_send(packet, NULL, 0, false);
}

void tl_send_msgreq(tl_t *msgreq, int prod_task, int prod_addr)
{
	packet_t *packet = pkt_slot_get();

	pkt_set_message_request(packet, prod_addr, msgreq->addr, prod_task, msgreq->task);

	dmni_send(packet, NULL, 0, false);
}

void tl_set(tl_t *tl, int task, int addr)
{
	tl->task = task;
	tl->addr = addr;
}
