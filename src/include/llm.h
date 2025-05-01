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
 * @brief Declares the Low-Level Monitor for Management Application support.
 */

#include <stdint.h>

#include <memphis/monitor.h>

typedef struct _observer {
	int16_t task;
	int16_t addr;
	uint16_t dist;
} observer_t;

/**
 * @brief Initializes the monitoring structures
 */
void llm_init();

/**
 * @brief Sets an observer if is nearer than the already set
 * 
 * @param type Monitoring type
 * @param task Task received
 * @param addr Address received
 */
void llm_set_observer(enum MONITOR_TYPE type, int task, int addr);

/**
 * @brief Check if monitoring type has monitor
 * 
 * @param mon_id ID of the monitoring type
 * @return true If has monitor
 * @return false If has no monitor
 */
bool llm_has_monitor(int mon_id);

/**
 * @brief Monitor real-time constraints
 * 
 * @param last_monitored Pointer to last monitored time
 * @param id ID of the monitored task
 * @param slack_time Slack time of the monitored task
 * @param remaining_exec_time Remaining execution time of the monitored task
 * @return true If monitored, updating last_monitored
 * @return false If should wait more time to monitor
 */
void llm_rt(unsigned *last_monitored, int id, unsigned slack_time, unsigned remaining_exec_time);

/**
 * @bried Monitor security contraints
 * 
 * @param timestamp Timestamp of received message
 * @param size      Size of received message
 * @param src       Source address of received message
 * @param dst       Destination address of received message
 * @param prod      Producer task
 * @param cons      Consumer task
 * @param now       Time now
 */
void llm_sec(unsigned timestamp, unsigned size, int src, int dst, int prod, int cons, unsigned now);
