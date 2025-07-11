/**
 * MAestro
 * @file mpipe.h
 *
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date July 2025
 * 
 * @brief This module declares the monitoring pipe message structure.
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

/**
 * @brief Initializes mpipe as null
 */
void mpipe_init();

/**
 * @brief Creates the mpipe
 * 
 * @param size Number of bytes of monitoring message (multiple of 4)
 * @param len Number of entries in FIFO
 * @param task Monitoring task calling this function
 * 
 * @return
 *  0 success
 * -EINVAL if size is not multiple of 4 or task is not from management app
 * -ENOMEM if cannot allocate the FIFO
 */
int mpipe_create(size_t size, size_t len, int task);

/**
 * @brief Gets the number of messages in the mpipe
 * 
 * @return int number of messages in the mpipe
 */
int mpipe_getvalue();

/**
 * @brief Tries to wait for the mpipe semaphore for available messages
 *  
 * @return
 *  0 success
 * -EAGAIN mpipe not available
 */
int mpipe_trywait();

/**
 * @brief Transfers the monitoring message
 * 
 * @details Never call this function without mpipe_wait returning >0 first
 * 
 * @param dst Pointer to destination buffer
 * @param size Size of the destination buffer (must be >= mpipe size)
 * 
 * @return size of message read
 */
int mpipe_read(void *dst, size_t size);

/**
 * @brief Posts the mpipe semaphore for available spaces in FIFO
 */
void mpipe_post();

/**
 * @brief Writes a message to the mpipe, sending it through NoC
 * 
 * @param buf Pointer to the message buffer
 * @param size Size of the message (multiple of 4)
 * @param addr Address of the destination PE
 * 
 * @return
 *  0 success
 * -EINVAL if size is not multiple of 4
 * -ENOMEM if cannot allocate the message packet
 */
int mpipe_write(void *buf, size_t size, int16_t addr);

/**
 * @brief Gets the ID of the mpipe owner
 * 
 * @return int
 *  id of the mpipe owner
 * -1 if not initialized
 */
int mpipe_owner();
