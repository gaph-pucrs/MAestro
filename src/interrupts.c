/**
 * MAestro
 * @file interrupts.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Defines the interrupts procedures of the kernel.
 */

#include <interrupts.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <memphis/services.h>

#include <message.h>
#include <rpc.h>
#include <syscall.h>
#include <task_migration.h>
#include <dmni.h>
#include <mmr.h>
#include <llm.h>
#include <task_allocation.h>

/** 
 * @brief Handles the packet coming from the NoC.
 * 
 * @param service Service type of incoming packet.
 * @param packet Pointer to packet read from DMNI
 * 
 * @return True if the scheduler should be called.
 */
bool _isr_handle_pkt(uint8_t service, void* packet);

tcb_t *isr_dispatcher(unsigned status)
{
	// printf("ISR called\n");
	sched_report_interruption();

	if (sched_is_idle())
		sched_update_slack_time();

	bool call_scheduler = false;
	/* Check interrupt source */
	if ((status & (1 << RISCV_IRQ_MEI)) && (MMR_DMNI_IRQ_IP & (1 << DMNI_IP_BRLITE))) {
		// puts("BR");
		bcast_t bcast_packet;
		bcast_read(&bcast_packet);
		call_scheduler |= rpc_bcast_dispatcher(&bcast_packet);
	} else if ((status & (1 << RISCV_IRQ_MEI)) && (MMR_DMNI_IRQ_IP & (1 << DMNI_IP_HERMES))) {
		// puts("NOC");

		uint32_t head = MMR_DMNI_HERMES_HEAD;
		uint8_t service = (head >> 16) & 0xFF;
		
		void *packet = hermes_recv_pkt(service);
		if (packet == NULL) {
			printf("ERROR: Invalid packet handling %lx\n", head);
			return NULL;
		}

		if(
			(MMR_DMNI_IRQ_STATUS & (1 << DMNI_STATUS_SEND_ACTIVE)) && 
			(service == DATA_AV || service == MESSAGE_REQUEST)
		) {
			if (msg_pndg_push_back(packet) == NULL) {
				printf("ERROR: Invalid pndg insertion.\n");
				free(packet);
				return NULL;
			}
		} else {
			int ret = _isr_handle_pkt(service, packet);
			if (ret < 0) {
				printf("ERROR: handle packet returned %d\n", ret);
			}
			call_scheduler = (ret == 1);
			free(packet);
		}
	} else if ((status & (1 << RISCV_IRQ_MEI)) && (MMR_DMNI_IRQ_IP & (1 << DMNI_IP_PENDING))) {
		// puts("PEND");
		/* Pending packet. Handle it */

		msg_hdshk_t *packet = msg_pndg_pop_front();

		if (packet == NULL) {
			puts("FATAL: Pending interrupt but no packet.");
			while(1);
		}

		call_scheduler = _isr_handle_pkt(packet->hermes.service, packet);

		free(packet);
	} else if ((status & (1 << RISCV_IRQ_MTI))) {
		// printf("Sched %u\n", MMR_RTC_MTIME);

		tcb_t *current = sched_get_current_tcb();

		if(current != NULL && tcb_check_stack(current)){
			printf(
				"Task id %d aborted due to stack overflow\n", 
				tcb_get_id(current)
			);

			tcb_abort_task(current);
		}
	}

	call_scheduler |= (status & (1 << RISCV_IRQ_MTI));

	tcb_t *current;
	if (call_scheduler) {
		// printf("Calling scheduler\n");
		sched_run();
		current = sched_get_current_tcb();
	} else {
		current = sched_get_current_tcb();
		if(current == NULL){
			sched_update_idle_time();
		} else {
			int id = tcb_get_id(current);
			sched_report(id);
		}
	}
	
	// printf("Scheduled %p\n", current);
    /* Runs the scheduled task */
    return current;
}

bool _isr_handle_pkt(uint8_t service, void *packet)
{
	// printf("Packet received %x\n", service);
	int ret = 0;
	switch(service){
		case DATA_AV:
			ret = msg_recv_data_av(packet);
			break;
		case MESSAGE_REQUEST:
			ret = msg_recv_message_request(packet);
			break;
		case MESSAGE_DELIVERY:
			ret = msg_recv_message_delivery(packet);
			break;
		case TASK_ALLOCATION:
			ret = talloc_alloc(packet);
			break;
		case MIGRATION_TEXT:
			ret = tm_recv_text(packet);
			break;
		case MIGRATION_DATA:
			ret = tm_recv_data(packet);
			break;
		case MIGRATION_STACK:
			ret = tm_recv_stack(packet);
			break;
		case MIGRATION_HDSHK:
			ret = tm_recv_hdshk(packet);
			break;
		case MIGRATION_PIPE:
			ret = tm_recv_opipe(packet);
			break;
		case MIGRATION_TASK_LOCATION:
			ret = tm_recv_tl(packet);
			break;
		case MIGRATION_TCB:
			ret = tm_recv_tcb(packet);
			break;
		default:
			ret = -EINVAL;
			break;
	}
	return ret;
}
