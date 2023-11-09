/**
 * MA-Memphis
 * @file broadcast.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2021
 * 
 * @brief API for Broadcast Network communication
 */

#include "broadcast.h"

#include <stdio.h>

#include <memphis.h>
#include <memphis/services.h>

#include "mmr.h"

bool bcast_send(bcast_t *packet, int16_t tgt_addr, uint8_t service)
{
	if((MMR_DMNI_STATUS & DMNI_STATUS_LOCAL_BUSY))
		return false;

	MMR_DMNI_BRLITE_SERVICE = service & 0x3;
	MMR_DMNI_BRLITE_KSVC = packet->service;
	MMR_DMNI_BRLITE_PRODUCER = packet->src_id;
	MMR_DMNI_BRLITE_TARGET = tgt_addr;
	MMR_DMNI_BRLITE_PAYLOAD = packet->payload;
	
	MMR_DMNI_BRLITE_START = 1;
	return true;
}

void bcast_read(bcast_t *packet)
{
	packet->service = MMR_DMNI_BRSVC_KSVC;

	uint32_t producer = MMR_DMNI_BRSVC_PRODUCER;
	packet->src_addr = producer >> 16;
	packet->src_id = producer & 0xFFFF;
	
	packet->payload = MMR_DMNI_BRSVC_PAYLOAD;

	MMR_DMNI_BRSVC_POP = 1;
}

void bcast_fake_packet(bcast_t *bcast_packet, packet_t *packet)
{
	packet->service = bcast_packet->service;

	int16_t id_field = bcast_packet->payload;
	int16_t addr_field = bcast_packet->payload >> 16;

	switch(packet->service){
		case DATA_AV:
			packet->producer_task = bcast_convert_id(bcast_packet->src_id, addr_field);
			packet->consumer_task = bcast_convert_id(id_field, MMR_DMNI_ADDRESS);
			packet->requesting_processor = addr_field;
			break;
		case MESSAGE_REQUEST:
			packet->producer_task = bcast_convert_id(id_field, MMR_DMNI_ADDRESS);
			packet->consumer_task = bcast_convert_id(bcast_packet->src_id, addr_field);
			packet->requesting_processor = addr_field;
			break;
		case TASK_MIGRATION:
			packet->task_ID = id_field;
			packet->allocated_processor = addr_field;
			break;
		default:
			printf("ERROR: Service %x not implemented in br->hermes\n", packet->service);
			break;
	}
}

int bcast_convert_id(int16_t id, int16_t addr)
{
	return (id == -1) ? (MEMPHIS_KERNEL_MSG | (int)addr) : id;
}
