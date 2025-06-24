/**
 * MAestro
 * @file task_migration.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief Defines the task migration functions
 */

#include <task_migration.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <memphis.h>
#include <memphis/services.h>
#include <memphis/messaging.h>

#include <broadcast.h>
#include <dmni.h>
#include <mmr.h>
#include <kernel_pipe.h>
#include <message.h>

list_t _tms;

/**
 * @brief Creates and stores a task migration information
 * 
 * @param task ID of the migrated task
 * @param addr Address where the task has migrated
 * @return tl_t* Pointer to the task location structure created
 */
tl_t *_tm_emplace_back(int task, int addr);

/**
 * @brief Finds a task migration entry by application ID
 * 
 * @param data Pointer to data
 * @param cmpval Pointer to appid int
 * 
 * @return True if comparison equal
 */
bool _tm_find_app_fnc(void *data, void* cmpval);

/**
 * @brief Migrate the data, bss and heap
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Target address
 * 
 * @return
 *  0 on success
 * -ENOMEM if not enough memory
 */
int _tm_send_data(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrate the stack
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Target address
 * 
 * @return
 *  0 on success
 * -ENOMEM if not enough memory
 */
int _tm_send_stack(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrates a message API handshake (DATA_AV + MESSAGE_REQUEST)
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Address to migrate
 * 
 * @return
 * 	0 on success
 * -ENOMEM if not enough memory
 */
int _tm_send_hdshk(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrates the output pipe
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the task
 * @param addr Address to migrate
 * 
 * @return
 * 	0 on success
 * -ENOMEM if not enough memory
 */
int _tm_send_opipe(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrates the application task location
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the migrating task
 * @param addr Address to migrate
 * 
 * @return
 * 	0 on success
 * -ENOMEM if not enough memory
 */
int _tm_send_tl(tcb_t *tcb, int id, int addr);

/**
 * @brief Migrates the TCB (and scheduler)
 * 
 * @param tcb Pointer to the TCB
 * @param id ID of the migrating task
 * @param addr Target address
 * 
 * @return
 * 	0 on success
 * -ENOMEM if not enough memory
 */
int _tm_send_tcb(tcb_t *tcb, int id, int addr);

void tm_init()
{
	list_init(&_tms);
}

tl_t *tm_find(int task)
{
	return tl_find(&_tms, task);
}

int tm_abort_task(int id, int addr)
{
	/* Send it like a MESSAGE_DELIVERY */
	memphis_info_t *abort_task = malloc(sizeof(memphis_info_t));
	if (abort_task == NULL)
		return -ENOMEM;

	abort_task->service = ABORT_TASK;
	abort_task->task    = id;

	return msg_send_message_delivery(
		abort_task, 
		sizeof(memphis_info_t), 
		MMR_DMNI_INF_ADDRESS, 
		(MEMPHIS_KERNEL_MSG | addr), 
		-1, 
		-1
	);
}

void tm_clear_app(int id)
{
	list_entry_t *entry = list_find(&_tms, &id, _tm_find_app_fnc);
	while(entry != NULL){
		tl_t *tl = list_get_data(entry);
		// printf("************* Removed task %d from migration\n", tl->task);
		list_remove(&_tms, entry);
		free(tl);
		entry = list_find(&_tms, &id, _tm_find_app_fnc);
	}
}

bool tm_empty()
{
	return list_empty(&_tms);
}

tl_t *_tm_emplace_back(int task, int addr)
{
	tl_t *tl = malloc(sizeof(tl_t));

	if (tl == NULL)
		return NULL;

	tl_set(tl, task, addr);

	list_entry_t *entry = list_push_back(&_tms, tl);

	if(entry == NULL){
		free(tl);
		return NULL;
	}

	return tl;
}

bool _tm_find_app_fnc(void *data, void* cmpval)
{
	tl_t *tl = (tl_t*)data;
	int app_id = *((int*)cmpval);

	return (((tl->task >> 8) & 0xFF) == app_id);
}

int tm_send_text(tcb_t *tcb, int id, int addr)
{
	printf("Sending text of task %d to address %x\n", id, addr);
	tm_text_t *packet = malloc(sizeof(tm_text_t));
	if (packet == NULL)
		return -ENOMEM;

	tl_t *mapper = tcb_get_mapper(tcb);
	size_t text_size = tcb_get_text_size(tcb);

	packet->hermes.flags   = 0;
    packet->hermes.service = MIGRATION_TEXT;
    packet->hermes.address = addr;
    packet->size           = text_size; 
    packet->mapper_address = tl_get_addr(mapper);
    packet->task           = id;
    packet->mapper_task    = tl_get_task(mapper);

	/* Align */
	text_size = (text_size + 3) & ~3;

	void *offset = tcb_get_offset(tcb);
	
	printf("Sending text of task %d to address %x with size %d\n", id, addr, text_size);

    return dmni_send(packet, sizeof(tm_text_t), true, offset, text_size, false);
}

int tm_recv_text(tm_text_t *packet)
{
	tcb_t *tcb = malloc(sizeof(tcb_t));

	if (tcb == NULL)
		return -ENOMEM;

	list_entry_t *entry = tcb_push_back(tcb);
	if (entry == NULL) {
		free(tcb);
		return -ENOMEM;
	}

	/* Initializes the TCB */
	tcb_alloc(tcb, packet->task, packet->size, 0, 0, packet->mapper_task, packet->mapper_address, 0);

	uint32_t text_size = (packet->size + 3) & ~3;

	/* Obtain the program code */
	void *offset = tcb_get_offset(tcb);
    int ret = dmni_recv(offset, text_size);
	if (ret < 0)
		return ret;

	printf("Received text of task %d with size %lu\n", packet->task, text_size);

	return 0;
}

int tm_migrate(tcb_t *tcb)
{
	printf("Migrating now\n");
	/* Get target address */
	int addr = tcb_get_migrate_addr(tcb);
	int id   = tcb_get_id(tcb);

	tl_t *tm = _tm_emplace_back(id, addr);
	if (tm == NULL)
		return -ENOMEM;

    /* Send data, bss and heap */
    int ret = _tm_send_data(tcb, id, addr);
	if (ret != 0)
		return ret;

    /* Send stack */
	ret = _tm_send_stack(tcb, id, addr);
	if (ret != 0)
		return ret;

	/* Send data available + message request fifo */
	ret = _tm_send_hdshk(tcb, id, addr);
	if (ret != 0)
		return ret;

	/* Send pipe */
	ret = _tm_send_opipe(tcb, id, addr);
	if (ret != 0)
		return ret;

	/* Send task location array */
	ret = _tm_send_tl(tcb, id, addr);
	if (ret != 0)
		return ret;

	/* Send TCB and scheduler info */
	ret = _tm_send_tcb(tcb, id, addr);
	if (ret != 0)
		return ret;
	
	/* Code (.text) is in another function */
	/**
	 * @todo
	 * Create a function to get the 64-bit timer
	 */
	printf(
		"Task id %d migrated at time %d to processor %x\n", 
		id, 
		MMR_RTC_MTIME, 
		addr
	);
	
	/* Update task location of tasks of the same app running locally */
	app_update(tcb_get_app(tcb), id, addr);
	tcb_remove(tcb);
	return 1;
}

int _tm_send_data(tcb_t *tcb, int id, int addr)
{
	size_t data_size  = tcb_get_data_size(tcb);
	size_t bss_size   = tcb_get_bss_size(tcb);

    /* Get the heap size */
	void *heap_start = (void*)(MMR_DATA_BASE + data_size + bss_size);
    void *heap_end   = tcb_get_heap_end(tcb);
    size_t heap_size = (heap_end - heap_start);

	size_t total_size = ((data_size + bss_size + heap_size) + 3) & ~3;

	if (total_size == 0)
		return 0;

    tm_data_t *packet = malloc(sizeof(tm_data_t));
    if (packet == NULL)
        return -ENOMEM;

    packet->hermes.flags   = 0;
    packet->hermes.service = MIGRATION_DATA;
    packet->hermes.address = addr;
    packet->data_size      = data_size;
    packet->bss_size       = bss_size;
    packet->heap_size      = heap_size;
    packet->task           = id;

	printf("Sending data of task %d to address %x with size %d\n", id, addr, total_size);

    return dmni_send(packet, sizeof(tm_data_t), true, tcb_get_offset(tcb) + tcb_get_text_size(tcb), total_size, false);
}

int tm_recv_data(tm_data_t *packet)
{
	tcb_t *tcb = tcb_find(packet->task);

	if (tcb == NULL)
		return -EINVAL;
	
	tcb_set_data_size(tcb, packet->data_size);
	tcb_set_bss_size(tcb, packet->bss_size);
    void *heap_start = (void*)(tcb_get_text_size(tcb) + packet->data_size + packet->bss_size);
    void *heap_end = heap_start + packet->heap_size;
    tcb_set_brk(tcb, heap_end);

	size_t total_size = ((packet->data_size + packet->bss_size + packet->heap_size) + 3) & ~3;

    int ret = dmni_recv(tcb_get_offset(tcb) + tcb_get_text_size(tcb), total_size);
	if (ret < 0)
		return ret;

	printf("Received data of task %d with size %u\n", packet->task, total_size);

	return 0;
}

int _tm_send_stack(tcb_t *tcb, int id, int addr)
{
	/* Get the stack pointer */
	size_t stack_size = (((MMR_DMNI_INF_DMEM_PAGE_SZ - (tcb_get_sp(tcb) - MMR_DATA_BASE)) + 3) & ~3);

	if (stack_size == 0)
		return 0;

    tm_stack_t *packet = malloc(sizeof(tm_stack_t));
    if (packet == NULL)
        return -ENOMEM;

    packet->hermes.flags   = 0;
    packet->hermes.service = MIGRATION_STACK;
    packet->hermes.address = addr;
    packet->size           = stack_size;
    packet->task           = id;

	printf("Sending stack of task %d to address %x with size %d\n", id, addr, stack_size);

    return dmni_send(packet, sizeof(tm_stack_t), true, (tcb_get_offset(tcb) + MMR_DATA_BASE) + (MMR_DMNI_INF_DMEM_PAGE_SZ - stack_size), stack_size, false);
}

int tm_recv_stack(tm_stack_t *packet)
{
	tcb_t *tcb = tcb_find(packet->task);

	if (tcb == NULL)
		return -EINVAL;

	int ret = dmni_recv((tcb_get_offset(tcb) + MMR_DATA_BASE) + (MMR_DMNI_INF_DMEM_PAGE_SZ - packet->size), packet->size);
	if (ret < 0)
		return ret;

	printf("Received stack of task %d with size %lu\n", packet->task, packet->size);

	return 0;
}

int _tm_send_hdshk(tcb_t *tcb, int id, int addr)
{
	list_t *davs = tcb_get_davs(tcb);
	list_t *reqs = tcb_get_msgreqs(tcb);

	size_t available_size = list_get_size(davs);
	size_t request_size   = list_get_size(reqs);
	size_t total_size     = available_size + request_size;

	if (total_size == 0)
		return 0;	/* No data available to migrate */

	tl_t *hdshk = malloc(total_size*sizeof(tl_t));
	if (hdshk == NULL)
		return -ENOMEM;

	list_vectorize(davs, &hdshk[0], sizeof(tl_t));
	list_vectorize(reqs, &hdshk[available_size], sizeof(tl_t));
	
	list_clear(davs);
	list_clear(reqs);

	tm_hdshk_t *packet = malloc(sizeof(tm_hdshk_t));
	if (packet == NULL) {
		free(hdshk);
		return -ENOMEM;
	}

	packet->hermes.flags   = 0;
	packet->hermes.service = MIGRATION_HDSHK;
	packet->hermes.address = addr;
	packet->available_size = available_size;
	packet->request_size   = request_size;
	packet->task           = id;

	printf("Sending hdshk %d to address %x with size %d\n", id, addr, total_size);

	return dmni_send(packet, sizeof(tm_hdshk_t), true, hdshk, total_size*sizeof(tl_t), true);
}

int tm_recv_hdshk(tm_hdshk_t *packet)
{
	tcb_t *tcb = tcb_find(packet->task);
	if (tcb == NULL)
		return -EINVAL;

	size_t total_size = packet->available_size + packet->request_size;

	tl_t *vec = malloc(total_size*sizeof(tl_t));
	if (vec == NULL)
		return -ENOMEM;

	int ret = dmni_recv(vec, total_size*sizeof(tl_t));
	if (ret < 0)
		return ret;
	
	list_t *davs = tcb_get_davs(tcb);
	for (int i = 0; i < packet->available_size; i++){
		if (list_push_back(davs, &(vec[i])) == NULL) {
			/**
			 * @todo clear structure
			 */
			free(vec);
			return -ENOMEM;
		}
	}

	list_t *reqs = tcb_get_msgreqs(tcb);
	for (int i = 0; i < packet->request_size; i++){
		if (list_push_back(reqs, &(vec[packet->available_size + i])) == NULL) {
			/**
			 * @todo clear structure
			 */
			free(vec);
			return -ENOMEM;
		}
	}

	printf("Received hdshk of task id %d with size %u\n", packet->task, total_size);

	return 0;
}

int _tm_send_opipe(tcb_t *tcb, int id, int addr)
{
	opipe_t *opipe = tcb_get_opipe(tcb);

	if (opipe != NULL)
		printf("Has pipe\n");

	if (opipe == NULL)
		return 0;

	size_t size;
	void* buf = opipe_get_buf(opipe, &size);

	tm_opipe_t *packet = malloc(sizeof(tm_opipe_t));
	if (packet == NULL)
		return -ENOMEM;

	packet->hermes.flags   = 0;
	packet->hermes.service = MIGRATION_PIPE;
	packet->hermes.address = addr;
	packet->receiver       = opipe_get_receiver(opipe);
	packet->task           = id;
	packet->size           = size;

	size_t align_size = (size + 3) & ~3;

	printf("Sending pipe of task %d to address %x with size %d\n", id, addr, align_size);
	
	int ret = dmni_send(packet, sizeof(tm_opipe_t), true, buf, align_size, true);
	tcb_destroy_opipe(tcb);
	return ret;
}

int tm_recv_opipe(tm_opipe_t *packet)
{
	tcb_t *tcb = tcb_find(packet->task);

	if (tcb == NULL)
		return -EINVAL;

	opipe_t *opipe = tcb_create_opipe(tcb);

	if (opipe == NULL)
		return -ENOMEM;

	int result = opipe_receive(opipe, packet->size, packet->receiver);

	if (result != packet->size)
		return -ENOMEM;

	printf("Received pipe of task id %d with size %lu\n", packet->task, packet->size);

	return result;
}

int _tm_send_tl(tcb_t *tcb, int id, int addr)
{
	app_t *app = tcb_get_app(tcb);
	size_t task_cnt = app_get_task_cnt(app);

	tm_tl_t *packet = malloc(sizeof(tm_tl_t));
	packet->hermes.flags   = 0;
	packet->hermes.service = MIGRATION_TASK_LOCATION;
	packet->hermes.address = addr;
	packet->task           = id;
	packet->task_cnt       = task_cnt;

	printf("Sending task location of task %d to address %x with size %d\n", id, addr, packet->task_cnt);

	return dmni_send(packet, sizeof(tm_tl_t), true, app_get_locations(app), task_cnt*sizeof(int), false);
}

int tm_recv_tl(tm_tl_t *packet)
{
	app_t *app = app_find(packet->task >> 8);

	if (app == NULL)
		return -EINVAL;

	int *tmploc = malloc(packet->task_cnt*sizeof(int));
	if (tmploc == NULL)
		return -ENOMEM;

	size_t received = dmni_recv(tmploc, packet->task_cnt*sizeof(int));

	size_t current_size = app_get_task_cnt(app);
	if (current_size == 0) {
		/* App created but no location present. Obtain from migration */
		app_set_location(app, packet->task_cnt, tmploc);
	} else {
		/* App already present with locations. Migration can be outdated */
		/* Discard!!!! */
		free(tmploc);
		tmploc = NULL;
	}

	printf("Received app of task id %d with size %u\n", packet->task, packet->task_cnt);

	return received;
}

int _tm_send_tcb(tcb_t *tcb, int id, int addr)
{
	sched_t *sched = tcb_get_sched(tcb);
	uint16_t received = 0;
	ipipe_t *ipipe = tcb_get_ipipe(tcb);
	if (ipipe != NULL) {
		received = ipipe_get_size(ipipe);
		tcb_destroy_ipipe(tcb);
	}

	/* Send TCB */
	tm_tcb_t *packet = malloc(sizeof(tm_tcb_t));

	packet->hermes.flags = 0;
	packet->hermes.service = MIGRATION_TCB;
	packet->hermes.address = addr;
	packet->pc             = (uint32_t)tcb_get_pc(tcb);
	packet->exec_time      = sched_get_exec_time(sched);
	packet->period         = sched_get_period(sched);
	packet->deadline       = sched_get_deadline(sched);
	packet->task           = id;
	packet->source         = MMR_DMNI_INF_ADDRESS;
	packet->waiting        = sched_get_waiting_msg(sched);
	packet->received       = received;

	printf("Sending TCB of task %d to address %x\n", id, addr);

	return dmni_send(packet, sizeof(tm_tcb_t), true, tcb_get_regs(tcb), HAL_MAX_REGISTERS*sizeof(int), false);
}

int tm_recv_tcb(tm_tcb_t *packet)
{
	tcb_t *tcb = tcb_find(packet->task);
	if (tcb == NULL)
		return -EINVAL;

	sched_t *sched = sched_emplace_back(tcb);
	if (sched == NULL)
		return -ENOMEM;

	if (packet->received != 0){
		ipipe_t *ipipe = tcb_create_ipipe(tcb);
		if (ipipe == NULL)
			return -ENOMEM;

		ipipe_set_read(ipipe, packet->received);
	}

	if (packet->period != 0)
		sched_real_time_task(sched, packet->period, packet->deadline, packet->exec_time);

	tcb_set_pc(tcb, (void*)(packet->pc));

	sched_set_waiting_msg(sched, packet->waiting);

	app_t *app = tcb_get_app(tcb);
	app_update(app, packet->task, MMR_DMNI_INF_ADDRESS);

	dmni_recv(tcb_get_regs(tcb), HAL_MAX_REGISTERS*sizeof(int));

	printf(
		"Task id %d allocated by task migration at time %d from processor %x\n", 
		packet->task, 
		MMR_RTC_MTIME, 
		packet->source
	);

	tl_t *mapper = tcb_get_mapper(tcb);
	memphis_info_t task_migrated;
	task_migrated.service = TASK_MIGRATED;
	task_migrated.task    = tcb->id;
	kpipe_add(
		&task_migrated, 
		sizeof(task_migrated), 
		tl_get_task(mapper), 
		tl_get_addr(mapper)
	);

	return 1;
}
