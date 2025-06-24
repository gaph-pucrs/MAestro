/**
 * MAestro
 * @file hermes.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Hermes protocol
 */

#pragma once

#include <stdint.h>

/**
 * Hermes packet
 * 
 * flags Routing flags {force io, io ports (2), 0b00000}
 * service Message type. See services.h in libmemphis
 * address Address in XXYY format
 */
typedef struct _hermes {
    /* {flags, service, address} */
    uint16_t address;
    uint8_t  service;
    uint8_t  flags;
} hermes_t;

/**
 * @brief Receives a packet
 * 
 * @param service Packet service
 * 
 * @return void* Pointer to packet
 */
void *hermes_recv_pkt(uint8_t service);
