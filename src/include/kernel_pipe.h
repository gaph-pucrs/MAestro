/**
 * MAestro
 * @file kernel_pipe.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 *
 * @brief Outgoing kernel messages encapsulated in the Memphis messaging API.
 */

#pragma once

#include <stdbool.h>

#include <opipe.h>

/**
 * @brief Initializes the pending message FIFO
 */
void kpipe_init();

/**
 * @brief Finds a message in the kpipe FIFO
 * 
 * @param receiver Receiver task of the message
 * 
 * @return opipe_t* Pointer to an output pipe
 */
opipe_t *kpipe_find(int receiver);

/**
 * @brief Sends a message delivery from kernel
 * 
 * @param buf Pointer to the message
 * @param size Size of the message
 * @param receiver Consumer task
 * @param target Consumer address
 * 
 * @return True if should schedule
 */
int kpipe_add(void *buf, size_t size, int receiver, int target);

/**
 * @brief Removes an output message from the kernel pipe
 * 
 * @param pending Pointer to the output pipe
 */
void kpipe_remove(opipe_t *pending);

/**
 * @brief Queries if the kernel has pending messages
 * 
 * @return True if pending messages
 */
bool kpipe_empty();
