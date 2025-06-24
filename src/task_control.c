/**
 * MAestro
 * @file task_control.c
 *
 * @author Marcelo Ruaro (marcelo.ruaro@acad.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date June 2016
 * 
 * @brief This module defines the task control block (TCB) functions.
 */

#include <task_control.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <mmr.h>
#include <llm.h>
#include <kernel_pipe.h>

#include <memphis/services.h>
#include <memphis/messaging.h>

list_t _tcbs;

void tcb_init()
{
	list_init(&_tcbs);
}

list_entry_t *tcb_push_back(tcb_t *tcb)
{
	return list_push_back(&_tcbs, tcb);
}

bool _tcb_find_fnc(void *data, void *cmpval)
{
    tcb_t *tcb = (tcb_t*)data;
    int task = *((int*)cmpval);

    return (tcb->id == task);
}

tcb_t *tcb_find(int task)
{
	list_entry_t *entry = list_find(&_tcbs, &task, _tcb_find_fnc);

    if(entry == NULL)
        return NULL;

    return list_get_data(entry);
}

void tcb_alloc(
	tcb_t *tcb, 
	int id, 
	size_t text_size, 
	size_t data_size, 
	size_t bss_size, 
	int mapper_task, 
	int mapper_addr, 
	void *entry_point
)
{
	memset(tcb->registers, 0, HAL_MAX_REGISTERS * sizeof(int));

	tcb->registers[HAL_REG_SP] = MMR_DATA_BASE | (MMR_DMNI_INF_DMEM_PAGE_SZ - (sizeof(int) << 1));
	tcb->pc = entry_point;

	tcb->page = page_acquire();

	if(tcb->page == NULL){
		puts("FATAL: no free pages found");
		while(true);
	}

	tcb->id = id;
	tcb->text_size = text_size;
	tcb->data_size = data_size;
	tcb->bss_size = bss_size;
	tcb->proc_to_migrate = -1;
	
	tcb->heap_end = (void*)(MMR_DATA_BASE + data_size + bss_size);

	tl_set(&(tcb->mapper), mapper_task, mapper_addr);
	list_init(&(tcb->message_requests));
	list_init(&(tcb->data_avs));

	int appid = id >> 8;
	tcb->app = app_find(appid);

	if(tcb->app == NULL){
		/* App is not present, create it */
		tcb->app = app_emplace_back(appid);

		if(tcb->app == NULL){
			puts("FATAL: could not allocate app");
			while(true);
		}
	}

	app_refer(tcb->app);

	/* Scheduler is created upon task release */
	tcb->scheduler = NULL;

	tcb->pipe_in = NULL;
	tcb->pipe_out = NULL;

	tcb->called_exit = false;

	/**
	 * @todo
	 * 0(sp) = argc = 0
	 * 4(sp) = argv = 0
	 */
	// int  *argc = (0x01000000 | MMR_DMNI_INF_DMEM_PAGE_SZ) + page_get_offset(tcb->page) - 4;
	// char *argv = (0x01000000 | MMR_DMNI_INF_DMEM_PAGE_SZ) + page_get_offset(tcb->page) - 8;
	// *argc = 0;
	// *argv = NULL;
}

bool tcb_check_stack(tcb_t *tcb)
{
	// printf("Checking stack pointer %u against heap %d\n", tcb_get_sp(current), tcb_get_heap_end(current));
	return (tcb->registers[HAL_REG_SP] < (unsigned)tcb->heap_end);
}

void _tcb_send_aborted(tcb_t *tcb)
{
	memphis_info_t task_aborted;
	task_aborted.service = TASK_ABORTED;
	task_aborted.task    = tcb->id;
	kpipe_add(
		&task_aborted, 
		sizeof(task_aborted), 
		tl_get_task(&(tcb->mapper)), 
		tl_get_addr(&(tcb->mapper))
	);
}

void tcb_abort_task(tcb_t *tcb)
{
	/* Send TASK_ABORTED */
	if (tl_get_task(&(tcb->mapper)) != -1)
		_tcb_send_aborted(tcb);

	tcb_remove(tcb);
}

void tcb_remove(tcb_t *tcb)
{
	app_derefer(tcb->app);

	page_release(tcb->page);

	sched_remove(tcb->scheduler);

	list_entry_t *entry = list_find(&_tcbs, tcb, NULL);
	if(entry != NULL)
		list_remove(&_tcbs, entry);

	MMR_DBG_TERMINATE = tcb->id;

	free(tcb);
}

opipe_t *tcb_get_opipe(tcb_t *tcb)
{
	return tcb->pipe_out;
}

bool _tcb_send_terminated(tcb_t *tcb)
{
	memphis_info_t task_terminated;
	task_terminated.service = TASK_TERMINATED;
	task_terminated.task    = tcb->id;
	return kpipe_add(
		&task_terminated, 
		sizeof(task_terminated),
		tl_get_task(&(tcb->mapper)), 
		tl_get_addr(&(tcb->mapper))
	);
}

void tcb_terminate(tcb_t *tcb)
{
	/* Send TASK_TERMINATED */
	_tcb_send_terminated(tcb);

	tcb_remove(tcb);
}

app_t *tcb_get_app(tcb_t *tcb)
{
	return tcb->app;
}

ipipe_t *tcb_get_ipipe(tcb_t *tcb)
{
	return tcb->pipe_in;
}

bool tcb_need_migration(tcb_t *tcb)
{
	return (tcb->proc_to_migrate != -1);
}

opipe_t *tcb_create_opipe(tcb_t *tcb)
{
	tcb->pipe_out = malloc(sizeof(opipe_t));
	return tcb->pipe_out;
}

void tcb_destroy_opipe(tcb_t *tcb)
{
	free(tcb->pipe_out);
	tcb->pipe_out = NULL;
	/* Note: the actual message is not freed here. Check DMNI */
}

list_t *tcb_get_msgreqs(tcb_t *tcb)
{
	return &(tcb->message_requests);
}

list_t *tcb_get_davs(tcb_t *tcb)
{
	return &(tcb->data_avs);
}

bool tcb_send_allocated(tcb_t *tcb)
{
	memphis_info_t task_allocated;
	task_allocated.service = TASK_ALLOCATED;
	task_allocated.task    = tcb->id;
	return kpipe_add(
		&task_allocated, 
		sizeof(task_allocated),
		tl_get_task(&(tcb->mapper)), 
		tl_get_addr(&(tcb->mapper))
	);
}

void *tcb_get_offset(tcb_t *tcb)
{
	return page_get_offset(tcb->page);
}

int tcb_get_migrate_addr(tcb_t *tcb)
{
	return tcb->proc_to_migrate;
}

void tcb_set_migrate_addr(tcb_t *tcb, int addr)
{
	tcb->proc_to_migrate = addr;
}

void *tcb_get_pc(tcb_t *tcb)
{
	return tcb->pc;
}

unsigned int tcb_get_sp(tcb_t *tcb)
{
	return tcb->registers[HAL_REG_SP];
}

int tcb_get_id(tcb_t *tcb)
{
	return tcb->id;
}

unsigned *tcb_get_regs(tcb_t *tcb)
{
	return tcb->registers;
}

size_t tcb_get_text_size(tcb_t *tcb)
{
	return tcb->text_size;
}

size_t tcb_get_data_size(tcb_t *tcb)
{
	return tcb->data_size;
}

void tcb_set_data_size(tcb_t *tcb, size_t data_size)
{
	tcb->data_size = data_size;
}

size_t tcb_get_bss_size(tcb_t *tcb)
{
	return tcb->bss_size;
}

void tcb_set_bss_size(tcb_t *tcb, size_t bss_size)
{
	tcb->bss_size = bss_size;
}

void tcb_set_pc(tcb_t *tcb, void *pc)
{
	tcb->pc = pc;
}

void tcb_set_called_exit(tcb_t *tcb)
{
	tcb->called_exit = true;
}

bool tcb_has_called_exit(tcb_t *tcb)
{
	return tcb->called_exit;
}

void *tcb_get_heap_end(tcb_t *tcb)
{
	return tcb->heap_end;
}

void tcb_set_brk(tcb_t *tcb, void *addr)
{
	tcb->heap_end = addr;
}

void tcb_set_sched(tcb_t *tcb, sched_t *sched)
{
	tcb->scheduler = sched;
}

ipipe_t *tcb_create_ipipe(tcb_t *tcb)
{
	tcb->pipe_in = malloc(sizeof(ipipe_t));

	if(tcb->pipe_in == NULL)
		return NULL;

	ipipe_init(tcb->pipe_in);
	return tcb->pipe_in;
}

void tcb_destroy_ipipe(tcb_t *tcb)
{
	free(tcb->pipe_in);
	tcb->pipe_in = NULL;
}

sched_t *tcb_get_sched(tcb_t *tcb)
{
	return tcb->scheduler;
}

tl_t *tcb_get_mapper(tcb_t *tcb)
{
	return &(tcb->mapper);
}

void tcb_set_ret(tcb_t *tcb, int ret)
{
	tcb->registers[HAL_REG_A0] = ret;
}

size_t tcb_size()
{
	return list_get_size(&_tcbs);
}

void tcb_inc_pc(tcb_t *tcb, unsigned inc)
{
	tcb->pc += inc;
}

void tcb_dump_regs(tcb_t *tcb)
{
	printf("ra  =%x\n", tcb->registers[HAL_REG_RA]);
	printf("sp  =%x\n", tcb->registers[HAL_REG_SP]);
	printf("gp  =%x\n", tcb->registers[HAL_REG_GP]);
	printf(" t0 =%x\n", tcb->registers[HAL_REG_T0]);
	printf(" t1 =%x\n", tcb->registers[HAL_REG_T1]);
	printf(" t2 =%x\n", tcb->registers[HAL_REG_T2]);
	printf(" s0 =%x\n", tcb->registers[HAL_REG_S0]);
	printf(" s1 =%x\n", tcb->registers[HAL_REG_S1]);
	printf(" a0 =%x\n", tcb->registers[HAL_REG_A0]);
	printf(" a1 =%x\n", tcb->registers[HAL_REG_A1]);
	printf(" a2 =%x\n", tcb->registers[HAL_REG_A2]);
	printf(" a3 =%x\n", tcb->registers[HAL_REG_A3]);
	printf(" a4 =%x\n", tcb->registers[HAL_REG_A4]);
	printf(" a5 =%x\n", tcb->registers[HAL_REG_A5]);
	printf(" a6 =%x\n", tcb->registers[HAL_REG_A6]);
	printf(" a7 =%x\n", tcb->registers[HAL_REG_A7]);
	printf(" s2 =%x\n", tcb->registers[HAL_REG_S2]);
	printf(" s3 =%x\n", tcb->registers[HAL_REG_S3]);
	printf(" s4 =%x\n", tcb->registers[HAL_REG_S4]);
	printf(" s5 =%x\n", tcb->registers[HAL_REG_S5]);
	printf(" s6 =%x\n", tcb->registers[HAL_REG_S6]);
	printf(" s7 =%x\n", tcb->registers[HAL_REG_S7]);
	printf(" s8 =%x\n", tcb->registers[HAL_REG_S8]);
	printf(" s9 =%x\n", tcb->registers[HAL_REG_S9]);
	printf(" s10=%x\n", tcb->registers[HAL_REG_S10]);
	printf(" s11=%x\n", tcb->registers[HAL_REG_S11]);
	printf(" t3 =%x\n", tcb->registers[HAL_REG_T3]);
	printf(" t4 =%x\n", tcb->registers[HAL_REG_T4]);
	printf(" t5 =%x\n", tcb->registers[HAL_REG_T5]);
	printf(" t6 =%x\n", tcb->registers[HAL_REG_T6]);
}
