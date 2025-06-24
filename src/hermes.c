/**
 * MAestro
 * @file hermes.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Hermes packet handling.
 */

#include <hermes.h>

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include <message.h>
#include <dmni.h>
#include <task_allocation.h>
#include <task_migration.h>

#include <memphis/services.h>

void *hermes_recv_pkt(uint8_t service)
{
	size_t expected = 0;

    switch (service) {
        case DATA_AV:
        case MESSAGE_REQUEST:
            expected = sizeof(msg_hdshk_t);
            break;
        case MESSAGE_DELIVERY:
            expected = sizeof(msg_dlv_t);
            break;
        case TASK_ALLOCATION:
            expected = sizeof(talloc_t);
            break;
        case MIGRATION_TEXT:
            expected = sizeof(tm_text_t);
            break;
        case MIGRATION_DATA:
            expected = sizeof(tm_data_t);
            break;
        case MIGRATION_STACK:
            expected = sizeof(tm_stack_t);
            break;
        case MIGRATION_HDSHK:
            expected = sizeof(tm_hdshk_t);
            break;
        case MIGRATION_PIPE:
            expected = sizeof(tm_opipe_t);
            break;
        case MIGRATION_TASK_LOCATION:
            expected = sizeof(tm_tl_t);
            break;
        case MIGRATION_TCB:
            expected = sizeof(tm_tcb_t);
            break;
        default:
            printf("ERROR: unknown hermes service %x\n", service);
            break;
    }
    
    if (expected == 0)
        return NULL;

    // printf("Expected: %d\n", expected);

    void *packet = malloc(expected);
    if (packet == NULL)
        return NULL;

    size_t received = dmni_recv(packet, expected);

    // printf("Received: %d\n", received);

    if (received != expected) {
        free(packet);
        return NULL;
    }

    return packet;
}
