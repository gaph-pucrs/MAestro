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
#include <stdio.h>

#include "mmr.h"

static const unsigned DMNI_READ  = 0;
static const unsigned DMNI_WRITE = 1;

int dmni_read(void *payload_address, size_t payload_size)
{
	MMR_DMNI_SIZE = (unsigned int)payload_size;
	MMR_DMNI_OP = DMNI_WRITE;
	MMR_DMNI_ADDRESS = (unsigned int)payload_address;
	MMR_DMNI_START = 1;
	while(MMR_DMNI_RECEIVE_ACTIVE);
	return MMR_DMNI_READ_FLITS;
}

void dmni_send(packet_t *packet, void *payload, size_t size, bool should_free, bool with_ecc)
{
	static bool free_outbound = false;
	static void *outbound = NULL;

	/* Wait for DMNI to be released */
	while(MMR_DMNI_SEND_ACTIVE);

	if(free_outbound)
		free(outbound);

	outbound = payload;
	free_outbound = should_free;

	/* Program DMNI */
	MMR_DMNI_SIZE = PKT_SIZE;
	MMR_DMNI_ADDRESS = (unsigned)packet;

	MMR_DMNI_SIZE_2 = size + (with_ecc ? 4 : 0);
	MMR_DMNI_ADDRESS_2 = (unsigned)outbound;

	MMR_DMNI_OP = DMNI_READ;

	pkt_set_dmni_info(packet, size + (with_ecc ? 4 : 0));

	if (with_ecc)
		dmni_set_ecc(packet, payload, size);

	MMR_DMNI_START = 1;
}

void dmni_send_raw(unsigned *packet, size_t size)
{
	/* Wait for DMNI to be released */
	// puts("[DMNI] Waiting for DMNI to be released.");
	while(MMR_DMNI_SEND_ACTIVE);
	// puts("[DMNI] DMNI released, sending.");

	/* Program DMNI */
	MMR_DMNI_SIZE = size;
	MMR_DMNI_ADDRESS = (unsigned)packet;

	// printf("Addr = %x\n", (unsigned)packet);
	// for(int i = 0; i < size; i++){
	// 	printf("%d\n", packet[i]);
	// }

	MMR_DMNI_SIZE_2 = 0;
	MMR_DMNI_ADDRESS_2 = 0;

	MMR_DMNI_OP = DMNI_READ;

	MMR_DMNI_START = 1;

	while(MMR_DMNI_SEND_ACTIVE);
	// puts("[DMNI] Sent.");
}

void dmni_drop_payload()
{
	// printf("Dropping payload - Size = %u\n", payload_size);
	MMR_DMNI_OP = DMNI_WRITE;
	MMR_DMNI_ADDRESS = 0;
	MMR_DMNI_START = 1;
	while(MMR_DMNI_RECEIVE_ACTIVE);
	// printf("Payload dropped\n");
}

void dmni_set_ecc(packet_t *packet, int *payload, size_t flit_cnt)
{
	/* Computar ECC para packet + payload 				  */
	/* Como exemplo estou adicionando os dados 0, 1, 2, 3 */
	/* Os 4 flits adicionais já estão alocados em memória */
	/* A cada 4 mensagens com ecc eu gero um ecc inválido */
	/* Disparando um re-envio 						      */

	/* ATENÇÃO: ignorar packet[0] e packet[1] (TARGET e SIZE) */

	static int ecc_cnt = 0;
	if (ecc_cnt++ % 4 == 0)
	{
		payload[flit_cnt  ] = 3;
		payload[flit_cnt+1] = 2;
		payload[flit_cnt+2] = 1;
		payload[flit_cnt+3] = 0;
	} else {
		payload[flit_cnt  ] = 0;
		payload[flit_cnt+1] = 1;
		payload[flit_cnt+2] = 2;
		payload[flit_cnt+3] = 3;
	}

	printf("ECC enviado: %x %x %x %x\n", payload[flit_cnt], payload[flit_cnt+1], payload[flit_cnt+2], payload[flit_cnt+3]);
}
