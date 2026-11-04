/**
 * MAestro
 * @file osz.h
 *
 * @brief OSZ region: single entrance routing and zone closing
 *
 * @details The region (DMNI_OSZ_WINDOW) starts at PE 0,0 and is X PEs wide
 * and Y rows high; column X stays free. A secure application owns region rows
 * lo..hi and is entered only through the east port of PE (X-1, hi):
 *
 * Routing (dmni_send):
 * - outside -> region: source-route turn at (X, hi), in the free column
 * - region  -> outside from a row below hi: turn at (own x, hi)
 * - anything else follows plain XY
 *
 * Closing (DMNI_OSZ_BLOCK): every region row is always inside a closed zone.
 * A row no application owns is its own one-row zone; rows lo..hi claimed by a
 * secure application form one zone. The links on the zone's edge are blocked,
 * except the east port of (X-1, hi), which is the zone's access point.
 *
 * The mapper broadcasts OSZ_ROWS when an application claims or releases rows.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

#include <mmr.h>

/**
 * OSZ events written to MMR_DBG_OSZ as {event[31:24], data[23:0]}. In
 * simulation, Phivers/sim/OszPeTrace.sv (bound under `ifdef OSZ_TRACE) logs
 * them with the cycle to debug/osz_trace.txt for tools/meshviz. Keep the
 * names there in sync.
 */
enum OSZ_DBG {
	OSZ_DBG_MAPPER        = 1,	/* data: mapper PE, learned from RELEASE_PERIPHERAL */
	OSZ_DBG_TASK_HELD     = 2,	/* data: task held at TASK_RELEASE until the zone is claimed */
	OSZ_DBG_TASK_RELEASED = 3,	/* data: held task released */
	OSZ_DBG_TASK_CLEARED  = 4,	/* data: task whose memory was cleared */
	OSZ_DBG_TASK_DONE     = 5,	/* data: task reported with OSZ_TASK_DONE */
	OSZ_DBG_ZONE_CLEAR    = 6,	/* data: OSZ_ZONE_CLEAR payload handled here */
	OSZ_DBG_ZONE_CLEARED  = 7,	/* data: OSZ_ZONE_CLEARED payload sent */
	OSZ_DBG_BCAST_IGNORED = 8	/* data: {service, source PE} of a zone broadcast not from the mapper */
};

static inline void osz_dbg(enum OSZ_DBG event, uint32_t data)
{
	MMR_DBG_OSZ = ((uint32_t)event << 24) | (data & 0xFFFFFF);
}

/**
 * @brief Reads the region, clears the row table and closes this PE's
 * one-row zone if it is inside the region
 */
void osz_init(void);

/**
 * @brief Handles OSZ_ROWS: rows lo..hi were claimed (entrance row hi) or
 * released. Closes this PE's zone accordingly and releases held tasks.
 *
 * @param payload OSZ_ROWS broadcast payload
 * @param src_addr Address of the PE that sent it (must be the mapper)
 *
 * @return 1 if the scheduler should be called, 0 otherwise
 */
int osz_set_rows(uint16_t payload, uint16_t src_addr);

/**
 * @brief Records the mapper PE: the source of the first RELEASE_PERIPHERAL.
 * Later claims are ignored. Only the mapper may send OSZ_ROWS and
 * OSZ_ZONE_CLEAR, and the access point accepts it as a source.
 *
 * @param src_addr Address of the PE that sent RELEASE_PERIPHERAL
 */
void osz_set_mapper(uint16_t src_addr);

/**
 * @brief Source-route turn point for a packet
 *
 * @param own Address of this PE (0xXXYY)
 * @param target Address of the packet target (0xXXYY)
 *
 * @return Turn point address, or -1 when the packet follows plain XY
 */
int osz_turn(uint16_t own, uint16_t target);

/**
 * @brief Whether a task on this PE may be released now
 *
 * @details A PE outside the region always may. A PE inside it only once it
 * belongs to a zone claimed by a secure application.
 */
bool osz_release_allowed(void);

/**
 * @brief Whether this PE is inside the region (it then only hosts tasks of
 * secure applications)
 */
bool osz_inside(void);

/**
 * @brief Reports over BrLite that a task ended and its memory was cleared
 *
 * @param id Task ID
 */
void osz_report_task_done(int id);

/**
 * @brief Handles OSZ_ZONE_CLEAR: on a PE of rows lo..hi, removes and clears
 * every task still here, then reports OSZ_ZONE_CLEARED
 *
 * @param payload OSZ_ZONE_CLEAR payload (rows lo..hi)
 * @param src_addr PE that sent it (must be the mapper)
 *
 * @return 1 if the scheduler should be called, 0 otherwise
 */
int osz_zone_clear(uint16_t payload, uint16_t src_addr);

/**
 * @brief On the PE hosting the mapper task, forwards an OSZ teardown report
 * (OSZ_TASK_DONE, OSZ_ZONE_CLEARED) to it through the kernel pipe
 *
 * @param service Broadcast service
 * @param payload Broadcast payload
 * @param src_addr PE that broadcast it
 *
 * @return 1 if the scheduler should be called, 0 otherwise
 */
int osz_forward_report(uint8_t service, uint16_t payload, uint16_t src_addr);

/**
 * @brief Keeps a task held until this PE's zone is claimed
 *
 * @param id Task ID
 *
 * @return 0 on success, -ENOMEM if there is no room
 */
int osz_hold(int id);
