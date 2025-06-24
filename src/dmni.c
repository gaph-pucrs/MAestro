/**
 * MAestro
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

#include <dmni.h>

#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

#include <mmr.h>

static const size_t FLIT_SIZE = 4;

size_t dmni_recv(void *dst, size_t size)
{
	if (size % FLIT_SIZE != 0)
		return -EINVAL;
	
	MMR_DMNI_HERMES_SIZE    = size/FLIT_SIZE;
	MMR_DMNI_HERMES_ADDRESS = (unsigned)dst;

	MMR_DMNI_IRQ_STATUS |= (1 << DMNI_STATUS_RECV_ACTIVE);
	while((MMR_DMNI_IRQ_STATUS & (1 << DMNI_STATUS_RECV_ACTIVE)));
	return MMR_DMNI_HERMES_RECD_CNT;
}

int dmni_send(void *pkt, size_t pkt_size, bool pkt_free, void *pld, size_t pld_size, bool pld_free)
{
	static bool free_last_pkt = false;
	static void *outbound_pkt = NULL;

	static bool free_last_pld = false;
	static void *outbound_pld = NULL;

	if (pkt_size % FLIT_SIZE != 0 || pld_size % FLIT_SIZE != 0)
		return -EINVAL;

	/* Wait for DMNI to be released */
	while((MMR_DMNI_IRQ_STATUS & (1 << DMNI_STATUS_SEND_ACTIVE)));

	/* Memory management */
	if (free_last_pkt)
		free(outbound_pkt);

	if (free_last_pld)
		free(outbound_pld);

	free_last_pkt = pkt_free;
	outbound_pkt  = pkt;

	free_last_pld = pld_free;
	outbound_pld  = pld;

	/* Program DMNI */
	MMR_DMNI_HERMES_SIZE      = pkt_size/FLIT_SIZE;
	MMR_DMNI_HERMES_ADDRESS   = (unsigned)pkt;

	MMR_DMNI_HERMES_SIZE_2    = pld_size/FLIT_SIZE;
	MMR_DMNI_HERMES_ADDRESS_2 = (unsigned)pld;

	MMR_DMNI_IRQ_STATUS |= (1 << DMNI_STATUS_SEND_ACTIVE);

	return 0;
}

void dmni_drop_payload(unsigned payload_size)
{
	// printf("Dropping payload - Size = %u\n", payload_size);
	MMR_DMNI_HERMES_SIZE    = payload_size;
	MMR_DMNI_HERMES_ADDRESS = (unsigned)NULL;
	
	MMR_DMNI_IRQ_STATUS |= (1 << DMNI_STATUS_RECV_ACTIVE);
	// printf("Payload dropped\n");
}
