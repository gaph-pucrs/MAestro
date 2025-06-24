/**
 * MAestro
 * @file task_migration.h
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Declares the task migration functions
 */

#pragma once

#include <task_location.h>
#include <task_control.h>
#include <hermes.h>

typedef struct _tm_text {
    hermes_t hermes;

    uint32_t size;
    
    /* {task, mapper_address} */
    uint16_t mapper_address;
    uint16_t task;

    /* {pad16, pad8, mapper_task} */
    int8_t   mapper_task;
    uint8_t  pad8;
    uint16_t pad16;

    /* Payload: binary with text */
} tm_text_t;

typedef struct _tm_data {
    hermes_t hermes;

    uint32_t data_size;
    
    uint32_t bss_size;

    uint32_t heap_size;
    
    /* {pad16, task} */
    uint16_t task;
    uint16_t pad16;

    /* Payload: binary with data+bss+heap */
} tm_data_t;

typedef struct _tm_stack {
    hermes_t hermes;

    uint32_t size;
    
    /* {pad16, task} */
    uint16_t task;
    uint16_t pad16;

    /* Payload: binary with stack */
} tm_stack_t;

typedef struct _tm_hdshk {
    hermes_t hermes;

    /* {request_size, available_size, task} */
    uint16_t task;
    uint8_t available_size;
    uint8_t request_size;
    
    /* Payload: vector with tl_t */
} tm_hdshk_t;

typedef struct _tm_opipe {
    hermes_t hermes;

    /* {task, receiver} */
    int16_t  receiver;
    uint16_t task;

    uint32_t size;

    /* Payload: opipe buffer */
} tm_opipe_t;

typedef struct _tm_tl {
    hermes_t hermes;

    /* {task, task_cnt, pad8} */
    uint8_t  pad8;
    uint8_t  task_cnt;
    uint16_t task;

    /* Payload: task location vector */
} tm_tl_t;

typedef struct _tm_tcb {
    hermes_t hermes;

    uint32_t pc;

    uint32_t exec_time;
    
    uint32_t period;

    int32_t  deadline;

    /* {task, source} */
    uint16_t task;
    uint16_t source;

    /* {pad8, waiting, received} */
    uint16_t received;
    uint8_t  waiting;
    uint8_t  pad8;

    /* Payload: TCB registers */
} tm_tcb_t;

/**
 * @brief Initializes the task migration structures
 */
void tm_init();

/**
 * @brief Finds a task migration
 * 
 * @param task ID of the migrated task
 * @return tl_t* Task location structure
 */
tl_t *tm_find(int task);

/**
 * @brief Sends a message to abort a migrating task
 * 
 * @param id ID of the task to abort
 * @param addr Address of the migration destination
 * 
 * @return ?
 */
int tm_abort_task(int id, int addr);

/**
 * @brief Clears all entries corresponding to an application
 * 
 * @param id Application ID
 */
void tm_clear_app(int id);

/**
 * @brief Checks if the migrated app list is empty
 * 
 * @return true Is empty
 * @return false Not empty
 */
bool tm_empty();

/**
 * @brief Migrates the code section of the task
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the TCB
 * @param addr Address of the TCB
 * 
 * @return
 *  0 on success
 * -ENOMEM when unable to allocate memory for the migration packet
 */
int tm_send_text(tcb_t *tcb, int id, int addr);

/**
 * @brief Handles the code received from migration
 * 
 * @param packet Pointer to received packet
 * 
 * @return
 *  Text size on success
 * -ENOMEM when unable to allocate memory for TCB
 */
int tm_recv_text(tm_text_t *packet);

/**
 * @brief Migrates a task dynamic memory
 * 
 * @param tcb Pointer to the TCB
 * 
 * @return 
 */
int tm_migrate(tcb_t *tcb);

/**
 * @brief Handles the data, bss and heap received from migration
 * 
 * @param packet Pointer to received packet
 * 
 * @return
 *  Data+BSS+Heap size on success
 * -EINVAL when the task is not found
 */
int tm_recv_data(tm_data_t *packet);

/**
 * @brief Handles the stack received from migration
 * 
 * @param packet Pointer to received packet
 * 
 * @return
 *  Stack size on success
 * -EINVAL when the task is not found
 */
int tm_recv_stack(tm_stack_t *packet);

/**
 * @brief Handles the task location received from migration (DATA_AV/MESSAGE_REQUEST)
 * 
 * @param packet Pointer to received packet
 * 
 * @return
 *  0 on success
 * -EINVAL when the task is not found
 * -ENOMEM when unable to allocate memory for the task location vector
 */
int tm_recv_hdshk(tm_hdshk_t *packet);

/**
 * @brief Handles the pipe received from migration
 * 
 * @param packet Pointer to received packet
 * 
 * @return
 *  Size received on success
 * -EINVAL when the task is not found
 * -ENOMEM when unable to allocate memory
 */
int tm_recv_opipe(tm_opipe_t *packet);

/**
 * @brief Handles the application data (task location) migrated
 * 
 * @param packet Pointer to received packet
 * 
 * @return
 *  Size on success
 * -EINVAL app not found
 * -ENOMEM not enough memory
 */
int tm_recv_tl(tm_tl_t *packet);

/**
 * @brief Handles the TCB received from migration with scheduler info
 * 
 * @param packet Pointer to received packet
 *
 * @return
 * 
 */
int tm_recv_tcb(tm_tcb_t *packet);
