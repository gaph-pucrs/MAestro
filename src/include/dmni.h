/**
 * MAestro
 * @file dmni.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Declares the DMNI functions for payload handling.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>

/**
 * @brief Receive data from NoC and copy to memory.
 * 
 * @param dst Address where the payload will be saved
 * @param size Number of bytes to copy. Must be multiple of flit size.
 * 
 * @return size_t Number of bytes received
 * -EINVAL case size is not multiple of flit size.
 */
size_t dmni_recv(void *dst, size_t size);

/**
 * @brief Abstracts the DMNI programming for writing data to NoC and copy from memory.
 * 
 * @param pkt Pointer to the packet to send
 * @param pkt_size Size of the packet to send (in bytes)
 * @param pkt_free True if should free the packet after the message is sent
 * @param pld Pointer to the payload to send, NULL if none
 * @param pld_size Size of the payload to send in bytes, 0 if none
 * @param pld_free True if should free the payload after the message is sent
 * 
 * @return int
 *  0 on success
 * -EINVAL if either pkt_size of pld_size not multiple of flit size.
 */
int dmni_send(void *pkt, size_t pkt_size, bool pkt_free, void *pld, size_t pld_size, bool pld_free);

/**
 * @brief Requests the DMNI to drop flits from a message payload.
 * 
 * @param payload_size Number of flits to drop from packet payload
 */
void dmni_drop_payload(unsigned payload_size);
