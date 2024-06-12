/**
 * MA-Memphis
 * @file ipipe.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 * 
 * @brief This module defines the input pipe message structure.
 */

#include "ipipe.h"

#include <stdlib.h>
#include <string.h>

#include "dmni.h"

void ipipe_init(ipipe_t *ipipe)
{
    ipipe->buf = NULL;
    ipipe->size = 0;
    ipipe->read = false;
}

void ipipe_set(ipipe_t *ipipe, void *msg, size_t size, bool with_ecc)
{
    ipipe->buf = msg;
    ipipe->size = size;
	ipipe->with_ecc = with_ecc;
}

size_t ipipe_get_size(ipipe_t *ipipe)
{
	return ipipe->size;
}

bool ipipe_is_read(ipipe_t *ipipe)
{
	return ipipe->read;
}

size_t ipipe_transfer(ipipe_t *ipipe, void *offset, void *src, size_t size)
{
	if(ipipe->buf == NULL)
		return -1;

	if(ipipe->size < size)
		return 0;

	void *real_ptr = (void*)((unsigned)ipipe->buf | (unsigned)offset);

	memcpy(real_ptr, src, size);

    ipipe->size = size;
	ipipe->read = true;

	return size;
}

int ipipe_receive(ipipe_t *ipipe, void *offset, size_t size)
{
	if(ipipe->buf == NULL || ipipe->size < size)
		return -1;

	void *real_ptr = (void*)((unsigned)ipipe->buf | (unsigned)offset);
	size_t align_size = (size + 3) & ~3;

	int read_flits = 0;
	if(ipipe->size < align_size){
		void *tmpbuf = malloc(align_size);
		if(tmpbuf == NULL)
			return -1;

		read_flits = dmni_read(tmpbuf, align_size >> 2);

		memcpy(real_ptr, tmpbuf, size);
		free(tmpbuf);
	} else {
		/* Obtain message from DMNI */
		read_flits = dmni_read(real_ptr, align_size >> 2);
	}

	int read_bytes = read_flits << 2;
	
	ipipe->size = align_size > read_bytes ? read_bytes : size;
	ipipe->read = true;

	return ipipe->size;
}

void ipipe_set_read(ipipe_t *ipipe, unsigned received)
{
	ipipe->read = true;
	ipipe->size = received;
}

bool ipipe_has_ecc(ipipe_t *ipipe)
{
	return ipipe->with_ecc;
}

int *ipipe_get_buf(ipipe_t *ipipe, size_t *flit_cntr)
{
	*flit_cntr = ipipe->size >> 2;
	return ipipe->buf;
}
