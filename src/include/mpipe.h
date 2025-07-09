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
 * @brief Waits for the mpipe semaphore for available messages
 * 
 * @param id ID of the reading task
 * @param max_size Size allocated at receiving task buffer
 * 
 * @return
 * >0 number of available messages
 *  0 unavailable (wait fail)
 * -EAGAIN mpipe not available
 * -EINVAL can't receive message (not directed to this task/not enough space)
 */
int mpipe_wait(int id, size_t max_size);

/**
 * @brief Transfers the monitoring message
 * 
 * @details Never call this function without mpipe_wait returning >0 first
 * 
 * @param dst Pointer to destination buffer
 * 
 * @return size of message read
 */
size_t mpipe_read(void *dst);

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
