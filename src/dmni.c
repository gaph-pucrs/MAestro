/**
 * MA-Memphis
 * @file dmni.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Defines the DMNI functions for payload handling.
 */

#include "dmni.h"

#include <stdlib.h>

#include "mmr.h"

void dmni_read(void *payload_address, size_t payload_size)
{
	MMR_DMNI_HERMES_SIZE = (unsigned int)payload_size;
	MMR_DMNI_HERMES_OPERATION = DMNI_OPERATION_WRITE;
	MMR_DMNI_HERMES_ADDRESS = (unsigned int)payload_address;
	MMR_DMNI_HERMES_START = 1;
	while((MMR_DMNI_STATUS & DMNI_STATUS_RECV_ACTIVE));
}

void dmni_send(packet_t *packet, void *payload, size_t size, bool should_free)
{
	static bool free_outbound = false;
	static void *outbound = NULL;

	/* Wait for DMNI to be released */
	while((MMR_DMNI_STATUS & DMNI_STATUS_SEND_ACTIVE));

	if(free_outbound)
		free(outbound);

	outbound = payload;
	free_outbound = should_free;

	/* Program DMNI */
	MMR_DMNI_HERMES_SIZE = PKT_SIZE;
	MMR_DMNI_HERMES_ADDRESS = (unsigned)packet;

	MMR_DMNI_HERMES_SIZE_2 = size;
	MMR_DMNI_HERMES_ADDRESS_2 = (unsigned)outbound;

	MMR_DMNI_HERMES_OPERATION = DMNI_OPERATION_READ;

	pkt_set_dmni_info(packet, size);

	MMR_DMNI_HERMES_START = 1;
}

void dmni_send_raw(unsigned *packet, size_t size)
{
	/* Wait for DMNI to be released */
	// puts("[DMNI] Waiting for DMNI to be released.");
	while((MMR_DMNI_STATUS & DMNI_STATUS_SEND_ACTIVE));
	// puts("[DMNI] DMNI released, sending.");

	/* Program DMNI */
	MMR_DMNI_HERMES_SIZE = size;
	MMR_DMNI_HERMES_ADDRESS = (unsigned)packet;

	// printf("Addr = %x\n", (unsigned)packet);
	// for(int i = 0; i < size; i++){
	// 	printf("%d\n", packet[i]);
	// }

	MMR_DMNI_HERMES_SIZE_2 = 0;
	MMR_DMNI_HERMES_ADDRESS_2 = 0;

	MMR_DMNI_HERMES_OPERATION = DMNI_OPERATION_READ;

	MMR_DMNI_HERMES_START = 1;

	while((MMR_DMNI_STATUS & DMNI_STATUS_SEND_ACTIVE));
	// puts("[DMNI] Sent.");
}

void dmni_drop_payload(unsigned payload_size)
{
	// printf("Dropping payload - Size = %u\n", payload_size);
	MMR_DMNI_HERMES_SIZE = payload_size;
	MMR_DMNI_HERMES_OPERATION = DMNI_OPERATION_READ;
	MMR_DMNI_HERMES_ADDRESS = 0;
	MMR_DMNI_HERMES_START = 1;
	while((MMR_DMNI_STATUS & DMNI_STATUS_RECV_ACTIVE));
	// printf("Payload dropped\n");
}
