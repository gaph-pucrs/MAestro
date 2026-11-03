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
 * @param src_addr Address of the PE that sent it (the mapper)
 *
 * @return 1 if the scheduler should be called, 0 otherwise
 */
int osz_set_rows(uint16_t payload, uint16_t src_addr);

/**
 * @brief Records the mapper PE as the source the access point accepts
 *
 * @param src_addr Address of the mapper PE
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
 * @brief Keeps a task held until this PE's zone is claimed
 *
 * @param id Task ID
 *
 * @return 0 on success, -ENOMEM if there is no room
 */
int osz_hold(int id);
