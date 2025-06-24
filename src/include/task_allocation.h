/**
 * MAestro
 * @file task_allocation.h
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Task allocation protocol
 */

#pragma once

#include <hermes.h>

/**
 * Task allocation packet
 * 
 * hermes Hermes packet
 * entry_point Starting PC
 * text_size Text section size in bytes
 * data_size Data section size in bytes
 * bss_size BSS section size in bytes
 * task App+Task ID allocated
 * mapper_address Address of mapper task
 * mapper_task ID of the mapping task (App ID is 0)
 */
typedef struct _talloc {
    hermes_t hermes;

    uint32_t entry_point;

    uint32_t text_size;

    uint32_t data_size;

    uint32_t bss_size;
    
    /* {task, mapper_address} */
    uint16_t mapper_address;
    uint16_t task;

    /* {pad16, pad8, mapper_task} */
    int8_t   mapper_task;
    uint8_t  pad8;
    uint16_t pad16;

    /* Payload: binary with text+data+bss */
} talloc_t;

int talloc_alloc(talloc_t *alloc);
