/**
 * MAestro
 * @file broadcast.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2021
 * 
 * @brief API for Broadcast Network communication
 */

#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct _bcast {
	uint8_t  service;
	uint16_t src_addr;
	uint16_t payload;
} bcast_t;

/**
 * @brief Sends a message via BrNoC
 * 
 * @param packet Pointer to packet to send (copied). src_addr is ignored.
 * 
 * @return True if success. False if BrNoC is busy.
 */
bool bcast_send(bcast_t *packet);

/**
 * @brief Reads a packet via BrNoC
 * 
 * @param packet Pointer to packet to save
 */
void bcast_read(bcast_t *packet);
