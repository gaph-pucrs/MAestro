/**
 * MAestro
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

/**
 * @brief Converts a sequential address into an hexadecimal address 0xXXYY
 * 
 * @param addr sequential address
 *
 * @return uint16_t hexadecimal ID
 */
uint16_t _bcast_seq2idx(uint16_t seq_addr);

bool bcast_send(bcast_t *packet)
{
	if (!(packet->service & 0x80) || (packet->service & 0x70))
		return false;

	if((MMR_DMNI_STATUS & (1 << DMNI_STATUS_LOCAL_BUSY)))
		return false;

	MMR_DMNI_BRLITE_KSVC     = packet->service;
	MMR_DMNI_BRLITE_PAYLOAD  = packet->payload;

	return true;
}

void bcast_read(bcast_t *packet)
{
	packet->service = MMR_DMNI_BRLITE_KSVC;

	uint32_t payload = MMR_DMNI_BRLITE_PAYLOAD;
	packet->src_addr = _bcast_seq2idx(payload >> 16);
	packet->payload  = payload & 0xFFFF;
}

uint16_t _bcast_seq2idx(uint16_t seq_addr)
{
	if (seq_addr == -1)
		return -1;

	uint8_t x_size = ((MMR_DMNI_MANYCORE_SIZE >> 8) & 0xFF);
	return ((seq_addr % x_size) << 8) | ((seq_addr / x_size) & 0xFF);
}
