/**
 * MAestro
 * @file osz.h
 *
 * @brief OSZ region routing: one entrance per secure application
 *
 * @details The region (DMNI_OSZ_WINDOW) starts at PE 0,0 and is X PEs wide
 * and Y rows high; column X stays free. A secure application owns region rows
 * lo..hi and is entered only through the east port of PE (X-1, hi):
 *
 * - outside -> region: source-route turn at (X, hi), in the free column
 * - region  -> outside from a row below hi: turn at (own x, hi)
 * - anything else follows plain XY
 *
 * The mapper broadcasts OSZ_ROWS when an application claims or releases rows.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Reads the region from the DMNI and clears the row table
 */
void osz_init(void);

/**
 * @brief Records that rows lo..hi were claimed (entrance row hi) or released
 *
 * @param payload OSZ_ROWS broadcast payload
 *
 * @return 0
 */
int osz_set_rows(uint16_t payload);

/**
 * @brief Source-route turn point for a packet
 *
 * @param own Address of this PE (0xXXYY)
 * @param target Address of the packet target (0xXXYY)
 *
 * @return Turn point address, or -1 when the packet follows plain XY
 */
int osz_turn(uint16_t own, uint16_t target);
