/**
 * MAestro
 * @file llm.c
 *
 * @author Angelo Elias Dalzotto (angelo.dalzotto@edu.pucrs.br)
 * GAPH - Hardware Design Support Group (https://corfu.pucrs.br/)
 * PUCRS - Pontifical Catholic University of Rio Grande do Sul (http://pucrs.br/)
 * 
 * @date March 2020
 * 
 * @brief Implements the Low-Level Monitor for Management Application support.
 */

#include "llm.h"

#include <stdlib.h>

#include <memphis.h>
#include <memphis/monitor.h>
#include <memphis/services.h>

#include "mmr.h"
#include "interrupts.h"
#include "broadcast.h"
#include "syscall.h"

observer_t _observers[MON_MAX];

void llm_init()
{
	for(int i = 0; i < MON_MAX; i++)
		_observers[i].addr = -1;
}

void llm_set_observer(enum MONITOR_TYPE type, int task, int addr)
{
	int pe_addr = MMR_DMNI_ADDRESS;
	uint8_t pe_x = pe_addr >> 8;
	uint8_t pe_y = pe_addr & 0xFF;
	uint8_t obs_x = addr >> 8;
	uint8_t obs_y = addr & 0xFF;
	uint16_t dist = abs(pe_x - obs_x) + abs(pe_y - obs_y);

	if(_observers[type].addr == -1 || _observers[type].dist > dist){
		_observers[type].task = task;
		_observers[type].addr = addr;
		_observers[type].dist = dist;
	}
}

void llm_clear_table(int task_id)
{
	isr_clear_mon_table(task_id);

	bcast_t packet;
	packet.service = CLEAR_MON_TABLE;
	packet.src_id = -1;

	packet.payload = task_id;

	while(!bcast_send(&packet, MMR_DMNI_ADDRESS, BR_SVC_ALL));
}

bool llm_has_monitor(int mon_id)
{
	return (_observers[mon_id].addr != -1);
}

void llm_rt(unsigned *last_monitored, int id, unsigned slack_time, unsigned remaining_exec_time)
{
	unsigned now = MMR_RTC_MTIME;

	if(now - (*last_monitored) < MON_INTERVAL_QOS)
		return;

	bcast_t packet;
	packet.service = MONITOR;
	packet.src_id = id;
	packet.payload = slack_time - remaining_exec_time;
	
	if(bcast_send(&packet, _observers[MON_QOS].addr, MON_QOS))
		*last_monitored = now;
}

void llm_sec(unsigned timestamp, unsigned size, int src, int dst, int prod, int cons, unsigned now)
{
	unsigned src_x = (src >> 8) & 0xFF;
	unsigned src_y = (src & 0xFF);
	unsigned dst_x = (dst >> 8) & 0xFF;
	unsigned dst_y = (dst & 0xFF);
	unsigned hops = abs(src_x - dst_x) + abs(src_y - dst_y);
	unsigned edge = (prod << 16) | (cons & 0xFFFF);
	unsigned latency = (now - timestamp);

	unsigned size_hops = (size << 16) | (hops & 0xFFFF);

	unsigned msg[] = {MONITOR, timestamp, size_hops, edge, latency};
	sys_kernel_writepipe(msg, sizeof(msg), _observers[MON_SEC].task, _observers[MON_SEC].addr);
}
