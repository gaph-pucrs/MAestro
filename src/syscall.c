/**
 * MAestro
 * @file syscall.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date August 2020
 * 
 * @brief Defines the syscall procedures of the kernel.
 */

#include <syscall.h>

#include <stdio.h>
#include <machine/syscall.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#include <interrupts.h>
#include <broadcast.h>
#include <task_migration.h>
#include <mmr.h>
#include <dmni.h>
#include <rpc.h>
#include <kernel_pipe.h>
#include <message.h>
#include <halt.h>
#include <mpipe.h>

#include <memphis/services.h>
#include <memphis/messaging.h>

bool schedule_after_syscall;	//!< Signals the HAL syscall to call scheduler
bool task_terminated;

tcb_t *sys_syscall(
	unsigned arg1, 
	unsigned arg2, 
	unsigned arg3, 
	unsigned arg4, 
	unsigned arg5, 
	unsigned arg6, 
	unsigned arg7, 
	unsigned number
)
{
	// printf("syscall(%d, %d, %d, %d, %d)\n", number, arg1, arg2, arg3, arg4);

	int ret = 0;
	schedule_after_syscall = false;
	task_terminated = false;

	tcb_t *current = sched_get_current_tcb();

	if (tcb_check_stack(current)) {
		printf(
			"Task id %d aborted due to stack overflow\n", 
			tcb_get_id(current)
		);

		tcb_abort_task(current);

		schedule_after_syscall = true;
		task_terminated = true;
	} else {
		switch (number) {
			case SYS_writepipe:
				ret = sys_writepipe(current, (void*)arg1, arg2, arg3, arg4);
				break;
			case SYS_readpipe:
				ret = sys_readpipe(current, (void*)arg1, arg2, arg3, arg4);
				break;
			case SYS_gettick:
				ret = sys_get_tick();
				break;
			case SYS_realtime:
				ret = sys_realtime(current, arg1, arg2, arg3);
				break;
			case SYS_getlocation:
				ret = sys_get_location();
				break;
			case SYS_brall:
				ret = sys_br_send(current, arg1, arg2);
				break;
			case SYS_getctx:
				ret = sys_get_ctx(current, (mctx_t*)arg1);
				break;
			case SYS_halt:
				ret = sys_end_simulation(current);
				break;
			case SYS_close:
				ret = sys_close(arg1);
				break;
			case SYS_write:
				ret = sys_write(current, arg1, (char*)arg2, arg3);
				break;
			case SYS_fstat:
				ret = sys_fstat(current, arg1, (struct stat*)arg2);
				break;
			case SYS_exit:
				ret = sys_exit(current, arg1);
				break;
			case SYS_getpid:
				ret = sys_getpid(current);
				break;
			case SYS_brk:
				ret = sys_brk(current, (void*)arg1);
				break;
			case SYS_safelog:
				ret = sys_safelog(arg1, arg2, arg3, arg4, arg5, arg6);
				break;
			case SYS_mkfifo:
				ret = sys_mkfifo(current, arg1, arg2);
				break;
			default:
				printf("ERROR: Unknown syscall %d\n", number);
				ret = 0;
				break;
		}
	}
	
	if (!task_terminated) {
		// printf("Setting return value %d\n", ret);
		tcb_set_ret(current, ret);
	}

	/* Return from ecall */
	tcb_inc_pc(current, 4);

	/* Schedule if timer has passed */
	schedule_after_syscall |= (sched_enabled() && MMR_RTC_MTIMECMP >= MMR_RTC_MTIME);
	if (schedule_after_syscall) {
		sched_run();
		// printf("Scheduled %d\n", tcb_get_id(current));
		return sched_get_current_tcb();
	}

	// printf("Scheduled %d\n", tcb_get_id(current));
	return current;
}

int sys_exit(tcb_t *tcb, int status)
{
	schedule_after_syscall = true;

	if(tcb_get_opipe(tcb) != NULL){
		/* Don't erase task with message in pipe */
		tcb_set_called_exit(tcb);
		sched_t *sched = tcb_get_sched(tcb);
		sched_set_wait_msgreq(sched);
		return -EAGAIN;
	}

	printf("Task id %d terminated with status %d\n", tcb_get_id(tcb), status);

	tcb_terminate(tcb);
	task_terminated = true;

	return 0;
}

int sys_writepipe(tcb_t *tcb, void *buf, size_t size, int receiver, bool sync)
{
	// printf("%x Called writepipe\n", tcb->id);
	const int sender = tcb_get_id(tcb);

	if ((receiver & 0xFFFFFF00) && sender >> 8 != 0)
		return -EACCES;

	if ((receiver & 0xFFFFFF00) && !sync)
		return -EINVAL;

	/* Points the message in the task page. Address composition: offset + msg address */
	if (buf == NULL)
		return -EINVAL;

	/* Sets real address of message buffer */
	buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));

	uint32_t target = receiver;
	if ((target & (MEMPHIS_FORCE_PORT | MEMPHIS_KERNEL_MSG)) == 0) {
		/* Message to task of same app */
		app_t *app = tcb_get_app(tcb);
		if (app == NULL)
			return -EINVAL;

		target = app_get_address(app, receiver);
		if (target == -1)
			return -EINVAL;
		
		/* Set app ID */
		receiver |= (sender & 0x0000FF00);
	}
	target &= ~MEMPHIS_KERNEL_MSG;
	if ((receiver & (MEMPHIS_FORCE_PORT | MEMPHIS_KERNEL_MSG)) != 0)
		receiver = -1;

	// printf("Calling writepipe from %x to %x; SYNC=%d\n", sender, receiver, sync);
	// printf("Target is %x\n", target);

	if ((receiver == -1) && (target == MMR_DMNI_INF_ADDRESS)) {
		/* Kernel can bypass the message request */
		schedule_after_syscall = rpc_hermes_dispatcher(buf, size);
		return size;
	}

	/* Searches if there is a message request to the produced message */
	list_t *msgreqs = tcb_get_msgreqs(tcb);
	tl_t   *request = tl_find(msgreqs, receiver != -1 ? receiver : target);

	if ((request != NULL) && (tl_get_addr(request) == MMR_DMNI_INF_ADDRESS)) {
		/* Request with local receiver: no need to buffer the transfer */
		tcb_t *recv_tcb = tcb_find(receiver);
		if (recv_tcb == NULL)
			return -EBADMSG;

		ipipe_t *dst = tcb_get_ipipe(recv_tcb);
		if (dst == NULL)
			return -EBADMSG;

		int result = ipipe_transfer(
			dst, 
			tcb_get_offset(recv_tcb), 
			buf, 
			size
		);

		if (result != size)
			return -EBADMSG;

		/* Remove the message request from buffer */
		tl_remove(msgreqs, request);
		MMR_DBG_REM_REQ = ((sender << 16) | (receiver & 0xFFFF));

		/* Release consumer task */
		sched_t *sched = tcb_get_sched(recv_tcb);
		sched_release_wait(sched);

		// if (tcb_need_migration(recv_tcb)) {
		// 	tm_migrate(recv_tcb);
		// 	schedule_after_syscall = 1;
		// }

		return size;		
	}

	if (tcb_get_opipe(tcb) != NULL) {
		/* Pipe full: wait for a message request to release the pipe */
		// printf("**** pipe is full\n");
		sched_t *sched = tcb_get_sched(tcb);
		sched_set_wait_msgreq(sched);
		schedule_after_syscall = true;
			
		return -EAGAIN;
	}

	if (request != NULL || (sync && (target != MMR_DMNI_INF_ADDRESS))) {
		/* Needs to send packet through NoC to complete*/
		if ((MMR_DMNI_IRQ_STATUS & (1 << DMNI_STATUS_SEND_ACTIVE))) {
			/* Deadlock avoidance: avoid sending a packet when the DMNI is busy */
			schedule_after_syscall = true;
			return -EAGAIN;
		}
	}

	/* Bufferize the message to transfer through NoC */
	opipe_t *opipe = tcb_create_opipe(tcb);
	if (opipe == NULL)
		return -ENOMEM;

	// Modificar receiver para o original enviado no writepipe
	int result = opipe_push(opipe, buf, size, receiver != -1 ? receiver : ((target & MEMPHIS_FORCE_PORT) ? target : (target | MEMPHIS_KERNEL_MSG)));

	if (result != size)
		return -ENOMEM;
		
	MMR_DBG_ADD_PIPE = ((sender << 16) | (receiver & 0xFFFF));

	if (request != NULL) {
		/* Can send immediately */
		int req_addr = tl_get_addr(request);
		msg_send_message_delivery(opipe->buf, opipe->size, MMR_DMNI_INF_ADDRESS, req_addr, sender, receiver);

		tcb_destroy_opipe(tcb);
		MMR_DBG_REM_PIPE = ((sender << 16) | (receiver & 0xFFFF));
		
		/* Remove the message request from buffer */
		tl_remove(msgreqs, request);
		MMR_DBG_REM_REQ = (sender << 16) | (receiver & 0xFFFF);
	} else if (sync) {
		if (target == MMR_DMNI_INF_ADDRESS) {
			/* Insert a DATA_AV to consumer table */
			tcb_t *recv_tcb = tcb_find(receiver);
			if (recv_tcb == NULL)
				return -EINVAL;

			/* Insert DATA_AV to the consumer TCB */
			list_t *davs = tcb_get_davs(recv_tcb);
			tl_t   *dav  = tl_emplace_back(davs, sender, MMR_DMNI_INF_ADDRESS);
			if (dav == NULL)
				return -ENOMEM;
			
			MMR_DBG_ADD_DAV = (sender << 16) | (receiver & 0xFFFF);
			
			/* If consumer waiting for a DATA_AV, release the task */
			sched_t *sched = tcb_get_sched(recv_tcb);
			if (sched_is_waiting_dav(sched))
				sched_release_wait(sched);
		} else {
			/* Send DATA_AV to consumer PE */
			// printf("Target = %x\n", target);
			msg_send_hdshk(MMR_DMNI_INF_ADDRESS, target, sender, receiver, DATA_AV);
		}
	}

	return result;
}

int sys_readpipe(tcb_t *tcb, void *buf, size_t size, int sender, bool sync)
{
	// puts("Calling readpipe");

	ipipe_t *ipipe = tcb_get_ipipe(tcb);
	if (ipipe != NULL) {
		if (ipipe_is_read(ipipe)) {
			// puts("Returning from readpipe");
			int ret = ipipe_get_size(ipipe);
			tcb_destroy_ipipe(tcb);
			return ret;
		}

		/* This should never happen, but is here just in case */
		return -EAGAIN;
	}

	if (buf == NULL) {
		// printf("READPIPE WITH NULL BUFFER\n");
		return -EINVAL;
	}

	const int receiver = tcb_get_id(tcb);

	uint32_t source;
	if (sync) {
		list_t *davs = tcb_get_davs(tcb);
		tl_t   *dav  = list_get_data(list_front(davs));
		if (dav == NULL) {
			if (receiver == mpipe_owner() && mpipe_trywait() == 0) {
				buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));
				size_t ret = mpipe_read(buf, size);
				mpipe_post();
				if (halt_pndg()) {
					if (halt_try() == 0)
						halt_clear();
				}
				return ret;
			}

			/* Block task and wait for DATA_AV packet */
			sched_t *sched = tcb_get_sched(tcb);
			sched_set_wait_dav(sched);
			schedule_after_syscall = 1;

			return -EAGAIN;
		}

		sender = tl_get_task(dav);
		source = tl_get_addr(dav);

		/* DATA_AV is processed, erase it */
		tl_remove(davs, dav);
		MMR_DBG_REM_DAV = (sender << 16) | (receiver & 0xFFFF);
		// printf("Readpipe: received DATA_AV from task %x with address %x\n", sender, source);
	} else {
		/* Not synced READ must define the producer */
		sender &= 0x000000FF;

		app_t *app = tcb_get_app(tcb);
		if (app == NULL)
			return -EINVAL;

		source = app_get_address(app, sender);
		if (source == -1)
			return -EINVAL;

		sender |= (receiver & 0xFF00);

		// printf("Trying to read from task %x at address %x\n", prod_task, prod_addr);
		// printf("Readpipe: trying to read from task %x with address %x\n", prod_task, prod_addr);
	}

	if ((sender == -1) && (source == MMR_DMNI_INF_ADDRESS)) {
		/* Message from local Kernel. No request needed */
		/* Search for the kernel-produced message */
		opipe_t *pending = kpipe_find(receiver);

		/* Store it like a MESSAGE_DELIVERY */
		buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));
		
		// putsv("Message length is ", msg->size);
		// putsv("First word is ", msg->message[0]);
		int result = opipe_transfer(pending, buf, size);

		if (result <= 0)
			return -EBADMSG;

		opipe_pop(pending);
		MMR_DBG_REM_PIPE = (sender << 16) | (receiver & 0xFFFF);
		kpipe_remove(pending);

		if (halt_pndg()) {
            if (halt_try() == 0)
                halt_clear();
        }

		return size;
	}

	tcb_t   *send_tcb = tcb_find(sender);
	opipe_t *opipe    = NULL;
	if (source == MMR_DMNI_INF_ADDRESS) {
		if (send_tcb == NULL)
			return -EBADMSG;
		opipe = tcb_get_opipe(send_tcb);
	}

	if (opipe != NULL && opipe_get_receiver(opipe) == (receiver != -1 ? receiver : ((source & MEMPHIS_FORCE_PORT) ? source : (source | MEMPHIS_KERNEL_MSG)))) {
		/* Message was found in pipe, writes to the consumer page address (local producer) */
		buf = (void*)((unsigned)buf | (unsigned)tcb_get_offset(tcb));

		int result = opipe_transfer(opipe, buf, size);

		if (result <= 0)
			return -EBADMSG;

		opipe_pop(opipe);
		MMR_DBG_REM_PIPE = (sender << 16) | (receiver & 0xFFFF);
		tcb_destroy_opipe(send_tcb);

		sched_t *sched = tcb_get_sched(send_tcb);

		if(sched_is_waiting_msgreq(sched)){
			sched_release_wait(sched);
			if(tcb_has_called_exit(send_tcb))
				tcb_terminate(send_tcb);
		}

		return result;
	}

	/* Stores the message pointer to receive */
	ipipe = tcb_create_ipipe(tcb);
	if (ipipe == NULL) {
		// printf("NOT ENOUGH MEMORY TO CREATE IPIPE\n");
		return -ENOMEM;
	}

	// printf("Allocated ipipe at %p\n", current->pipe_in);
	ipipe_set(ipipe, buf, size);
	// printf("Set ipipe to %p size %d\n", ipipe->buf, ipipe->size);

	if (source == MMR_DMNI_INF_ADDRESS) {
		/* Stores the request into the message request table */
		list_t *msgreqs = tcb_get_msgreqs(send_tcb);
		tl_emplace_back(msgreqs, receiver, MMR_DMNI_INF_ADDRESS);
		MMR_DBG_ADD_REQ = (sender << 16) | (receiver & 0xFFFF);
	} else {
		/* Remote sender : Sends the message request */
		msg_send_hdshk(MMR_DMNI_INF_ADDRESS, source, sender, receiver, MESSAGE_REQUEST);
	}

	/* Sets task as waiting blocking its execution, it will execute again when the message is produced by a WRITEPIPE or incoming MSG_DELIVERY */
	sched_t *sched = tcb_get_sched(tcb);
	sched_set_wait_msgdlvr(sched);
	schedule_after_syscall = 1;

	return -EAGAIN;
}

unsigned int sys_get_tick()	
{	
	return MMR_RTC_MTIME;	
}

int sys_realtime(tcb_t *tcb, unsigned period, int deadline, unsigned exec_time)
{
	// printf("RT: %u %d %u\n", period, deadline, exec_time);
	sched_t *sched = tcb_get_sched(tcb);
	sched_real_time_task(sched, period, deadline, exec_time);

	schedule_after_syscall = 1;

	return 0;
}

int sys_get_location()
{
	return MMR_DMNI_INF_ADDRESS;
}

int sys_getpid(tcb_t *tcb)
{
	return tcb_get_id(tcb);
}

int sys_br_send(tcb_t *tcb, uint8_t ksvc, uint16_t payload)
{
	int prod_task = tcb_get_id(tcb);
	
	if(prod_task >> 8 != 0)	/* AppID should be 0 */
		return -EINVAL;

	bcast_t packet;
	packet.service = ksvc;
	packet.src_addr = MMR_DMNI_INF_ADDRESS;
	packet.payload = payload;

	if(!bcast_send(&packet)){
		schedule_after_syscall = true;
		return -EAGAIN;
	}
		
	schedule_after_syscall = rpc_bcast_dispatcher(&packet);
	return 0;
}

int sys_brk(tcb_t *tcb, void *addr)
{
	// printf("brk(%u)\n", addr);
	void *heap_end = tcb_get_heap_end(tcb);

	if(addr == NULL || addr == heap_end){
		// printf("Returning %u\n", heap_end);
		return (int)heap_end;
	}

	// if(addr < heap_end)
	// 	return addr;

	unsigned sp = tcb_get_sp(tcb);

	if((unsigned)addr > sp){
		fprintf(
			stderr, 
			"Heap and stack collision in task %d\n", 
			tcb_get_id(tcb)
		);
		return -1;
	}

	// printf("Growing heap from %u to %u\n", heap_end, (unsigned)addr);

	tcb_set_brk(tcb, addr);

	return (int)addr;
}

int sys_write(tcb_t *tcb, int file, char *buf, int nbytes)
{
	if(file != STDOUT_FILENO && file != STDERR_FILENO)
		return -EBADF;

	if(buf == NULL){
		printf("ERROR: buffer is null\n");
		return -EINVAL;
	}

	buf = (char*)((unsigned)tcb_get_offset(tcb) | (unsigned)buf);


	int rv = 0;
	if(nbytes == 1 && buf[0] == '\n'){
		rv = write(file, buf, nbytes);
	} else {
		int id = tcb_get_id(tcb);
		int addr = MMR_DMNI_INF_ADDRESS;

		printf("$$$_%dx%d_%d_%d_", addr >> 8, addr & 0xFF, id >> 8, id & 0xFF);
		fflush(stdout);

		rv = write(file, buf, nbytes);
	}

	if(rv == -1)
		return -errno;

	return rv;
}

int sys_fstat(tcb_t *tcb, int file, struct stat *st)
{
	if(st == NULL){
		printf("ERROR: st is null");
		return false;
	}

	st = (struct stat*)((unsigned)tcb_get_offset(tcb) | (unsigned)st);
	int ret = fstat(file, st);

	if(ret == -1)
		return -errno;

	return ret;
}

int sys_close(int file)
{
	return -EBADF;
}

int sys_get_ctx(tcb_t *tcb, mctx_t *ctx)
{
	mctx_t *real_ptr = (mctx_t*)((unsigned)tcb_get_offset(tcb) | (unsigned)ctx);
	uint32_t size_register = MMR_DMNI_INF_MANYCORE_SZ;
	real_ptr->PE_X_CNT 	= (size_register >> 8) & 0xFF;
	real_ptr->PE_Y_CNT 	= size_register & 0xFF;
	real_ptr->PE_CNT	= real_ptr->PE_X_CNT * real_ptr->PE_Y_CNT;
	real_ptr->PE_SLOTS 	= size_register >> 16;
	real_ptr->MC_SLOTS 	= real_ptr->PE_SLOTS * real_ptr->PE_CNT;

	return 0;
}

int sys_end_simulation(tcb_t *tcb)
{
	if(tcb_get_id(tcb) >> 8 != 0)
		return -EACCES;

	MMR_DBG_HALT = 1;
	return 0;
}

int sys_safelog(unsigned snd_time, unsigned inf_time, unsigned edge, unsigned inf_lat, unsigned lat_pred, unsigned lat_mon)
{
	MMR_DBG_SAFE_SND_TIME = snd_time;
	MMR_DBG_SAFE_INF_TIME = inf_time;
	MMR_DBG_SAFE_EDGE     = edge;
	MMR_DBG_SAFE_INF_LAT  = inf_lat;
	MMR_DBG_SAFE_LAT_PRED = lat_pred;
	MMR_DBG_SAFE_LAT_MON  = lat_mon;
	return 0;
}

int sys_mkfifo(tcb_t *tcb, int size, int len)
{
	const int id = tcb_get_id(tcb);
	return mpipe_create(size, len, id);
}
