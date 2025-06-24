/**
 * MAestro
 * @file rpc.h
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Remote Procedure Call (RPC) through BrLite/Hermes
 */

#pragma once

#include <broadcast.h>

#include <memphis/monitor.h>

/**
 * @brief Handles an interruption coming from a broadcast message
 * 
 * @param packet Pointer to BrNoC packet
 * 
 * @return int 
 *  1 if the scheduler should be called
 *  0 otherwise
 * <0 on error
 */
int rpc_bcast_dispatcher(bcast_t *packet);

/**
 * @brief Calls a syscall from a received message (MESSAGE_DELIVERY)
 * 
 * @param message Pointer to the message
 * @param size Size of the message in bytes
 * 
 * @return int
 * 1 if should schedule
 */
int rpc_hermes_dispatcher(void *message, size_t size);
