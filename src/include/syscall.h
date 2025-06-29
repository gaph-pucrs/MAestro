/**
 * MAestro
 * @file syscall.h
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date September 2020
 * 
 * @brief Declares the syscall procedures of the kernel.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#include <sys/stat.h>

#include <memphis.h>
#include <memphis/monitor.h>

#include "task_control.h"

/**
 * @brief Decodes a syscall
 * 
 * @param arg1 Argument in a0
 * @param arg2 Argument in a1
 * @param arg3 Argument in a2
 * @param arg4 Argument in a3
 * @param arg5 Argument in a4
 * @param arg6 Argument in a5
 * @param arg7 Argument in a6
 * @param number Syscall number
 * @return tcb_t* Scheduled tcb 
 */
tcb_t *sys_syscall(
	unsigned arg1, 
	unsigned arg2, 
	unsigned arg3, 
	unsigned arg4, 
	unsigned arg5, 
	unsigned arg6, 
	unsigned arg7, 
	unsigned number
);

/**
 * @brief Exit the task and deallocate resources
 * 
 * @param tcb Pointer to the TCB
 * @param status Return status code
 * 
 * @return 0 if exited, -EAGAIN if not
 */
int sys_exit(tcb_t *tcb, int status);

/**
 * @brief Sends a message
 * 
 * @param tcb Pointer to the producer TCB
 * @param buf Pointer to the message
 * @param size Size of the message
 * @param cons_task ID of the consumer task
 * @param sync If it should send a data available message before delivery
 * 
 * @return Number of bytes sent/stored in pipe/transferred.
 * 		   -EINVAL on invalid argument
 *         -EAGAIN if must retry
 *         -EBADMSG on message protocol errors
 *         -EACCES on unauthorized targets
 */
int sys_writepipe(tcb_t *tcb, void *buf, size_t size, int cons_task, bool sync);

/**
 * @brief Receives a message
 * 
 * @param tcb Pointer to the producer TCB
 * @param buf Pointer to message to save to
 * @param size Size of the allocated buffer
 * @param prod_task ID of the producer task
 * @param sync If it should wait for a data available message before requesting
 * 
 * @return Number of bytes read.
 * 		   -EINVAL on invalid argument
 *         -EAGAIN if must retry (busy/waiting for interruption)
 *         -EBADMSG on message protocol errors
 */
int sys_readpipe(tcb_t *tcb, void *buf, size_t size, int prod_task, bool sync);

/**	
 * @brief Get the tick count	
 * 	
 * @return Value of tick count	
 */	
unsigned int sys_get_tick();

/**
 * @brief Configures a task real time
 * 
 * @param tcb Pointer to the producer TCB
 * @param period Task period in cycles
 * @param deadline Task deadline in cycles
 * @param exec_time Task execution time in cycles
 * 
 * @return 0.
 */
int sys_realtime(tcb_t *tcb, unsigned int period, int deadline, unsigned int exec_time);

/**
 * @brief Sends a raw packet
 * 
 * @param tcb Pointer to the producer TCB
 * @param buf Pointer to the packet
 * @param size Packet size in flits
 * 
 * @return 0 if sent the packet, -EINVAL on invalid argument
 */
int sys_sendraw(tcb_t *tcb, void *buf, size_t size);

/**
 * @brief Gets the net address.
 * 
 * @return Net address value.
 */
int sys_get_location();

/**
 * @brief Gets the running complete task ID
 * 
 * @param tcb Pointer to the TCB to get PID
 * 
 * @return Task ID (with application ID)
 */
int sys_getpid(tcb_t *tcb);

/**
 * @brief Sends a message via broadcast
 * 
 * @param tcb Pointer to the producer TCB
 * @param ksvc Kernel service used (see services.h)
 * @param payload Message to send
 * 
 * @return 0 if success. 1 if BrNoC is busy. 2 unauthorized.
 */
int sys_br_send(tcb_t *tcb, uint8_t ksvc, uint16_t payload);

/**
 * @brief Sets the brk (heap end) of a task
 * 
 * @param tcb Pointer to the TCB
 * @param addr Address to set
 * @return int Address of the new heap end if modified, previous heap end if
 * unmodified, and -1 if error
 */
int sys_brk(tcb_t *tcb, void *addr);

/**
 * @brief Writes to a file
 * 
 * @param tcb Pointer to the TCB
 * @param file File number
 * @param buf Pointer of the buffer to write
 * @param nbytes Number of bytes to write
 * 
 * @return int Number of bytes written
 */
int sys_write(tcb_t *tcb, int file, char *buf, int nbytes);

/**
 * @brief Get status of a file
 * 
 * @param tcb Pointer to the TCB
 * @param file File number
 * @param st Pointer to stat structure
 * 
 * @return int -1 if invalid file, 0 if valid
 */
int sys_fstat(tcb_t *tcb, int file, struct stat *st);

/**
 * @brief Closes a file
 * 
 * @details There are no valid files for closing.
 * 
 * @param file File number
 * 
 * @return int -EBADF
 */
int sys_close(int file);

/**
 * @brief Gets the system context
 * 
 * @param tcb Pointer to the TCB
 * @param ctx Pointer to the ctx
 * 
 * @return int 0
 */
int sys_get_ctx(tcb_t *tcb, mctx_t *ctx);

/**
 * @brief Ends the simulation
 * 
 * @param tcb TCB of the requester task
 * 
 * @return int 0 if success, EACCES if not permitted
 */
int sys_end_simulation(tcb_t *tcb);

/**
 * @brief Logs SAFE inference
 * 
 * @param snd_time Timestamp of originating message
 * @param inf_time Timestamp after inference
 * @param edge     Edge of monitored message
 * @param inf_lat  Inference latency (XGBoost)
 * @param lat_pred Predicted latency
 * 
 * @return 0
 */
int sys_safelog(unsigned snd_time, unsigned inf_time, unsigned edge, unsigned inf_lat, unsigned lat_pred, unsigned lat_mon);
