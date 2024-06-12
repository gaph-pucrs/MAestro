/**
 * MA-Memphis
 * @file ipipe.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2022
 * 
 * @brief This module declares the input pipe message structure.
 */

#pragma once

#include <stddef.h>
#include <stdbool.h>

/**
 * @brief This structure stores a task message in task space (NoC -> task)
 */
typedef struct _ipipe {
	void *buf;
	size_t size;
    bool read;
	bool with_ecc;
} ipipe_t;

/**
 * @brief Initializes the input pipe
 * 
 * @param ipipe Pointer to the ipipe structure
 */
void ipipe_init(ipipe_t *ipipe);

/**
 * @brief Sets the message pointer for the input pipe
 * 
 * @param ipipe Pointer to the ipipe structure
 * @param msg Pointer to the buffer
 * @param size Allocated buffer size
 * @param with_ecc If the message has ECC
 */
void ipipe_set(ipipe_t *ipipe, void *msg, size_t size, bool with_ecc);

/**
 * @brief Gets the input pipe size
 * 
 * @param ipipe Pointer to the ipipe structure
 * @return size_t Size of the ipipe
 */
size_t ipipe_get_size(ipipe_t *ipipe);

/**
 * @brief Gets if the message has been read
 * 
 * @param ipipe Pointer to the ipipe structure
 * 
 * @return true When read
 * @return false When not read yet
 */
bool ipipe_is_read(ipipe_t *ipipe);

/**
 * @brief Transfers a message to the input pipe
 * 
 * @param ipipe Pointer to the input pipe that will receive a message
 * @param offset Page offset of the receiving task
 * @param src Pointer to the message to transfer from
 * @param size Size of the message to transfer
 * 
 * @return size_t Number of bytes written
 */
size_t ipipe_transfer(ipipe_t *ipipe, void *offset, void *src, size_t size);

/**
 * @brief Receives a message to the input pipe
 * 
 * @param ipipe Pointer to the input pipe that will receive a message
 * @param offset 
 * @param size 
 * @return int 
 */
int ipipe_receive(ipipe_t *pipe, void *offset, size_t size);

/**
 * @brief Sets an ipipe as received
 * 
 * @param ipipe Pointer to the ipipe
 * @param received Number of bytes received
 */
void ipipe_set_read(ipipe_t *ipipe, unsigned received);

/**
 * @brief Checks if the message has ECC
 * 
 * @param ipipe Pointer to ipipe
 * 
 * @return true If should read ECC flits from DMNI
 */
bool ipipe_has_ecc(ipipe_t *ipipe);

/**
 * @brief Gets the input pipe buffer
 * 
 * @param ipipe Pointer to ipipe structure
 * @param flit_cntr Pointer to store buffer size (in flits)
 * 
 * @return int* Pointer to input pipe buffer
 */
int *ipipe_get_buf(ipipe_t *ipipe, size_t *flit_cntr);
