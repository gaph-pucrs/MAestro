/**
 * MAestro
 * @file rpc.c
 * 
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date May 2025
 * 
 * @brief Remote Procedure Call (RPC) through BrLite/Hermes
 */

#include <rpc.h>

#include <errno.h>
#include <stdio.h>

#include <llm.h>
#include <mmr.h>
#include <halt.h>
#include <task_control.h>
#include <task_migration.h>

#include <memphis/services.h>
#include <memphis/messaging.h>

/**
 * @brief Registers an announced observer
 * 
 * @param type Monitoring type
 * @param task Task ID of the observer
 * @param addr Address of the observer
 * 
 * @return 0
 */
int _rpc_announce_monitor(enum MONITOR_TYPE type, int task, int addr);

/**
 * @brief Releases peripherals connected to this PE
 * 
 * @return 0
 */
int _rpc_release_peripheral();

/**
 * @brief Clears application migration data
 * 
 * @param id ID of the application
 * 
 * @return 0
 */
int _rpc_app_terminated(int id);

/**
 * @brief Asks to halt the PE
 * 
 * @param task Mapper task ID that requested the halt
 * @param addr Address of the mapper task
 * 
 * @return 1 if halted, 0 if not
 * -ENOMEM if not enough memory
 */
int _rpc_halt_pe(int task, int addr);

/**
 * @brief Handles a task release packet
 * 
 * @param packet Pointer to packet
 * @param payload Pointer to location array
 * 
 * @return int
 *  0 should no call scheduler
 *  1 should call scheduler
 * -EINVAL: task not found
 * -ENOMEM: could not allocate memory for task location or scheduler
 */
int _rpc_task_release(memphis_info_t *packet, int32_t *payload);

/**
 * @brief Aborts a task
 * 
 * @param packet Pointer to packet
 * 
 * @return 
 *  0 if the task was already terminated or migrated
 * 	1 if the current running task is the aborted
 * -EINVAL if the task was not found
 */
int _rpc_abort_task(memphis_info_t *packet);

/**
 * @brief Handles a task migration order
 * @details This function is called to the source processor (the old processor)
 * 
 * @param packet Pointer to packet
 * 
 * @return 
 * 	0 if scheduler should not be called
 *  1 if scheduler should be called
 */
int _rpc_task_migration(memphis_task_migration_t *packet);

int rpc_bcast_dispatcher(bcast_t *packet)
{
	// printf("Broadcast received %x\n", packet->service);
	uint8_t id_field = packet->payload & 0xFF;
	uint8_t other_field = packet->payload >> 8;

	int ret = -EINVAL;
	switch(packet->service){
		case ANNOUNCE_MONITOR:
			ret = _rpc_announce_monitor(other_field, id_field, packet->src_addr);
			break;
		case RELEASE_PERIPHERAL:
			ret = _rpc_release_peripheral();
			break;
		case APP_TERMINATED:
			ret = _rpc_app_terminated(id_field);
			break;
		case HALT_PE:
			ret = _rpc_halt_pe(id_field, packet->src_addr);
			break;
		default:
			printf(
				"ERROR: unknown broadcast %x at time %d\n", 
				packet->service, 
				MMR_RTC_MTIME
			);
			break;
	}

	return ret;
}

int rpc_hermes_dispatcher(void *message, size_t size)
{
	uint8_t service = (((uint32_t*)message)[0] >> 16) & 0xFF;

	int ret = -EINVAL;
	switch (service) {
		case TASK_RELEASE:
			ret = _rpc_task_release(message, (int32_t*)&((uint8_t*)message)[sizeof(memphis_info_t)]);
			break;
		case ABORT_TASK:
			ret = _rpc_abort_task(message);
			break;
		case TASK_MIGRATION:
			ret = _rpc_task_migration(message);
			break;
		default:
			printf(
				"ERROR: Unknown service %x inside MESSAGE_DELIVERY\n", 
				service
			);
			break;
	}
	return ret;
}

int _rpc_announce_monitor(enum MONITOR_TYPE type, int task, int addr)
{
	llm_set_observer(type, task, addr);
	return 0;
}

int _rpc_release_peripheral()
{
	MMR_DMNI_IRQ_STATUS |= (1 << DMNI_STATUS_REL_PERIPHERAL);
	// puts("Peripherals released\n");
	return 0;
}

int _rpc_app_terminated(int id)
{
	tm_clear_app(id);

	if (!halt_pndg())
		return 0;
    
    return (halt_try() == 0);
}

int _rpc_halt_pe(int task, int addr)
{
	/* Halt this PE */
	/* We can only halt when all resources are released */
	/* This allow us to check for memory leaks! */
    int ret = halt_set(task, addr);
	printf("*** Halt requested\n");

    if (ret < 0)
        return ret;

    if (halt_try() != 0)
        return 1;

    halt_clear();

	printf("Halt done!\n");

	return 0;
}

int _rpc_task_release(memphis_info_t *packet, int32_t *payload)
{
	/* Get task to release */
	tcb_t *tcb = tcb_find(packet->task);

	if (tcb == NULL)
		return -EINVAL;

	printf("-> TASK RELEASE received to task %d\n", tcb_get_id(tcb));

	/* Write task location */
	app_t *app = tcb_get_app(tcb);

	size_t current_size = app_get_task_cnt(app);
	if (current_size != packet->task_cnt) {
		int result = app_copy_location(app, packet->task_cnt, (int*)payload);

		if (result < 0)
			return -ENOMEM;
	}

	/* If the task is blocked, release it */
	sched_t *sched = tcb_get_sched(tcb);
	if (sched == NULL)
		sched = sched_emplace_back(tcb);
	
	if (sched == NULL)
		return -ENOMEM;

	return sched_is_idle();
}

int _rpc_abort_task(memphis_info_t *packet)
{
	tcb_t *tcb = tcb_find(packet->task);
	if (tcb == NULL) {
		/* Task already terminated or migrated from here */
		tl_t *tl = tm_find(packet->task);
		int addr = tl_get_addr(tl);
		if (addr == -1)
			return -EINVAL;
		tm_abort_task(packet->task, addr);
		return 0;
	}

	printf("Task id %d aborted by application\n", packet->task);

	int mig_addr = tcb_get_migrate_addr(tcb);
	if (mig_addr != -1) {
		/* Task is migrating. Inform the destination processor of this */
		tm_abort_task(packet->task, mig_addr);
	}

	tcb_remove(tcb);

	return (sched_get_current_tcb() == tcb);
}

int _rpc_task_migration(memphis_task_migration_t *packet)
{
	tcb_t *task = tcb_find(packet->task);

	if (task == NULL || tcb_has_called_exit(task)) {
		printf("Tried to migrate task %x but it already terminated\n", packet->task);
		return 0;
	}

	int old_addr = tcb_get_migrate_addr(task);
	if (old_addr != -1) {
		printf(
			"ERROR: task %x PE already assigned to %x when tried to assign %x\n", 
			packet->task, 
			old_addr, 
			packet->address
		);

		return 0;
	}

	printf("Trying to migrate task %d to address %d\n", packet->task, packet->address);

	tcb_set_migrate_addr(task, packet->address);

	/* Send constant .text section */
	int ret = tm_send_text(task, packet->task, packet->address);
	printf("Text returned %d\n", ret);
	if (ret != 0)
		return ret;

	sched_t *sched = tcb_get_sched(task);
	if (sched_is_waiting_delivery(sched)) {
		printf("Task %d is waiting for message delivery, cannot migrate\n", packet->task);
		return 0;
	}

	return tm_migrate(task);
}
