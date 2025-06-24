/**
 * MAestro
 * @file message.h
 * 
 * @author Angelo Elias Dal Zotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Message protocol
 */

#pragma once

#include <hermes.h>

#include <mutils/list.h>

/**
 * Message handshake packet
 * Used by DATA_AV and MESSAGE_DELIVERY
 * 
 * hermes Hermes packet
 * source Message origin address
 * sender App+Task ID sending the message. -1 -> Kernel
 * receiver App+Task ID sending the message. -1 -> Kernel
 */
typedef struct _msg_hdshk {
    hermes_t hermes;

    uint32_t source;

    /* {sender, receiver} */
    int16_t receiver;
    int16_t sender;
} msg_hdshk_t;

/**
 * Message delivery packet
 * 
 * hdshk Handshake containing Hermes packet and sender-receiver
 * timestamp Send timestamp for monitoring
 * size Message size in bytes
 */
typedef struct _msg_dlv {
    msg_hdshk_t hdshk;

    uint32_t timestamp;

    uint32_t size;

    /* Payload: message */
} msg_dlv_t;

/**
 * @brief Initializes the pending handshakes FIFO
 */
void msg_pndg_init();

/**
 * @brief Adds a handshake to pending messages
 * 
 * @param hdshk Pointer to service packet
 *
 * @return list_entry_t* Pointer to entry
 */
list_entry_t *msg_pndg_push_back(msg_hdshk_t *hdshk);

/**
 * @brief Removes the first element from the FIFO
 * 
 * @return void* Pointer to packet
 */
msg_hdshk_t *msg_pndg_pop_front();

/**
 * @brief Checks if there are no more pending services
 * 
 * @return true No more pending services
 * @return false Has pending services
 */
bool msg_pndg_empty();

/**
 * @brief Receives a DATA_AV
 * 
 * @param hdshk Pointer to service packet
 * 
 * @return int
 *  0 success. Should not call scheduler.
 *  1 success. Should call scheduler.
 * -EINVAL when receiver task neither found nor migrated.
 * -ENOMEM when unable to emplace handshake into receiver task.
 */
int msg_recv_data_av(msg_hdshk_t *hdshk);

/**
 * @brief Receives a MESSAGE_REQUEST
 * 
 * @param hdshk Pointer to service packet
 * 
 * @return int
 *  0 success. Should not call scheduler.
 *  1 success. Should call scheduler.
 * -ENODATA: No message found in kernel pipe
 * -EINVAL: task not here not migrated or input pipe not present
 */
int msg_recv_message_request(msg_hdshk_t *hdshk);

/**
 * @brief Receives a MESSAGE_DELIVERY
 * 
 * @param dlv Pointer to service packet
 * 
 * @return int
 *  0 success. Should not call scheduler.
 *  1 success. Should call scheduler.
 * -EINVAL: receiver not here; input pipe not present.
 */
int msg_recv_message_delivery(msg_dlv_t *dlv);

/**
 * @brief Sends a DATA_AV/MESSAGE_REQUEST
 * 
 * @param source Source address
 * @param target Target address
 * @param sender Sender ID
 * @param receiver Receiver ID
 * @param service MESSAGE_REQUEST or DATA_AV
 * 
 * @return int
 *  0 success
 * -ENOMEM when unable to create outbound packet
 */
int msg_send_hdshk(uint32_t source, uint32_t target, uint16_t sender, uint16_t receiver, uint8_t service);

/**
 * @brief Sends a MESSAGE_DELIVERY
 * 
 * @param pld Pointer to message payload
 * @param size Payload size in bytes
 * @param source Source PE
 * @param target Target PE
 * @param sender Sender task
 * @param receiver Receiver task
 * 
 * @return int
 *  0 on success
 * -ENOMEM: could not create delivery packet;
 */
int msg_send_message_delivery(void *pld, size_t size, uint32_t source, uint32_t target, uint16_t sender, uint16_t receiver);
