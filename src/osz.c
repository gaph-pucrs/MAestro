/**
 * MAestro
 * @file osz.c
 *
 * @brief OSZ region routing: one entrance per secure application
 */

#include <osz.h>

#include <stdlib.h>
#include <stdio.h>

#include <mmr.h>

#include <memphis/messaging.h>

static int  _x = 0;			/* Region width, 0 when disabled */
static int  _y = 0;			/* Region height */
static int *_entrance = NULL;	/* Entrance (top) row of the app owning each row, -1 when free */

void osz_init(void)
{
	uint32_t window = MMR_DMNI_INF_OSZ_WINDOW;
	_x = (window >> 8) & 0xFF;
	_y = window & 0xFF;

	if (_x == 0 || _y == 0) {
		_x = 0;
		_y = 0;
		return;
	}

	_entrance = malloc(_y * sizeof(int));
	if (_entrance == NULL) {
		puts("FATAL: not enough memory for the OSZ row table");
		_x = 0;
		_y = 0;
		return;
	}

	for (int r = 0; r < _y; r++)
		_entrance[r] = -1;
}

int osz_set_rows(uint16_t payload)
{
	if (_entrance == NULL)
		return 0;

	const bool claim = MEMPHIS_OSZ_ROWS_CLAIM(payload);
	const int  lo    = MEMPHIS_OSZ_ROWS_LO(payload);
	const int  hi    = MEMPHIS_OSZ_ROWS_HI(payload);

	for (int r = lo; r <= hi && r < _y; r++)
		_entrance[r] = claim ? hi : -1;

	return 0;
}

static bool _inside(int x, int y)
{
	return (x < _x && y < _y);
}

int osz_turn(uint16_t own, uint16_t target)
{
	if (_entrance == NULL)
		return -1;

	const int ox = own >> 8;
	const int oy = own & 0xFF;
	const int tx = target >> 8;
	const int ty = target & 0xFF;

	const bool own_in    = _inside(ox, oy);
	const bool target_in = _inside(tx, ty);

	if (own_in == target_in)
		return -1;	/* Both outside, or both inside the same application */

	if (target_in) {
		/* Inbound: down the free column to the entrance row, then west */
		int hi = _entrance[ty];
		if (hi < 0)
			hi = ty;	/* Row not claimed: enter through its own east port */

		const int turn = (_x << 8) | hi;
		return (turn == own) ? -1 : turn;
	}

	/* Outbound: below the entrance row, first go up to it in the own column */
	const int hi = _entrance[oy];
	if (hi < 0 || hi == oy)
		return -1;

	return (ox << 8) | hi;
}
