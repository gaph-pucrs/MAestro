/**
 * MAestro
 * @file osz.c
 *
 * @brief OSZ region: single entrance routing and zone closing
 */

#include <osz.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include <mmr.h>
#include <task_control.h>
#include <task_scheduler.h>
#include <broadcast.h>
#include <kernel_pipe.h>
#include <dmni.h>

#include <memphis/services.h>
#include <memphis/messaging.h>

/* A broadcast fails only while the local BrLite port is busy with the previous one */
#define OSZ_BCAST_RETRIES	100000

/* DMNI_OSZ_BLOCK fields */
#define OSZ_BLOCK_E			(1U << 0)
#define OSZ_BLOCK_W			(1U << 1)
#define OSZ_BLOCK_N			(1U << 2)
#define OSZ_BLOCK_S			(1U << 3)
#define OSZ_BLOCK_ACCESS_E	(1U << 4)
#define OSZ_BLOCK_SRC_VALID	(1U << 5)

static int  _x = 0;				/* Region width, 0 when disabled */
static int  _y = 0;				/* Region height */
static int  _mesh_y = 0;
static int *_entrance = NULL;	/* Entrance (top) row of the app owning each row, -1 when free */

/* This PE's zone, only meaningful inside the region */
static struct {
	bool inside;			/* This PE is inside the region */
	int  x;
	int  y;
	bool claimed;			/* Zone of a secure application (else a one-row zone) */
	int  lo;
	int  hi;
} _pe;

/**
 * The mapper PE: the source of the first RELEASE_PERIPHERAL. The mapper sends
 * it once, when the management application is mapped, before the application
 * injector is released, so before any non-management code runs anywhere.
 * Only it may send OSZ_ROWS and OSZ_ZONE_CLEAR, and the access point accepts
 * it as a source.
 */
static bool     _mapper_valid = false;
static uint16_t _mapper       = 0;

/* Tasks released by the mapper before this PE's zone was claimed */
static int   *_held = NULL;
static size_t _held_max = 0;

static void _osz_close(void)
{
	if (!_pe.inside)
		return;

	uint32_t block = 0;

	/* East edge: the access point on the top row, blocked below it */
	if (_pe.x == _x - 1)
		block |= (_pe.y == _pe.hi) ? OSZ_BLOCK_ACCESS_E : OSZ_BLOCK_E;

	/* North edge: the zone's top row, unless it is the mesh edge */
	if (_pe.y == _pe.hi && _pe.y + 1 < _mesh_y)
		block |= OSZ_BLOCK_N;

	/* South edge: the zone's bottom row, unless it is the mesh edge */
	if (_pe.y == _pe.lo && _pe.y > 0)
		block |= OSZ_BLOCK_S;

	/* West: x = 0 is always the mesh edge */

	if (_mapper_valid)
		block |= OSZ_BLOCK_SRC_VALID | ((uint32_t)_mapper << 16);

	MMR_DMNI_OSZ_BLOCK = block;

	printf(
		"OSZ: PE %d,%d closed in %s zone rows %d..%d, block %lx\n",
		_pe.x, _pe.y, _pe.claimed ? "claimed" : "one-row", _pe.lo, _pe.hi, block
	);
}

void osz_init(void)
{
	uint32_t window = MMR_DMNI_INF_OSZ_WINDOW;
	_x = (window >> 8) & 0xFF;
	_y = window & 0xFF;
	_mesh_y = MMR_DMNI_INF_MANYCORE_SZ & 0xFF;

	_pe.inside = false;

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

	const uint16_t own = MMR_DMNI_INF_ADDRESS;
	_pe.x = own >> 8;
	_pe.y = own & 0xFF;
	_pe.inside = (_pe.x < _x && _pe.y < _y);
	if (!_pe.inside)
		return;

	_held_max = MMR_DMNI_INF_MANYCORE_SZ >> 16;
	_held = malloc(_held_max * sizeof(int));
	if (_held == NULL) {
		puts("FATAL: not enough memory for the OSZ held tasks");
		_held_max = 0;
	}
	for (int i = 0; i < _held_max; i++)
		_held[i] = -1;

	/* Every region row starts as its own closed one-row zone */
	_pe.claimed   = false;
	_pe.lo        = _pe.y;
	_pe.hi        = _pe.y;
	_osz_close();
}

void osz_set_mapper(uint16_t src_addr)
{
	if (_mapper_valid) {
		if (src_addr != _mapper)
			printf("WARNING: OSZ ignoring a mapper claim from %x, the mapper is %x\n", src_addr, _mapper);
		return;
	}

	_mapper_valid = true;
	_mapper       = src_addr;
	printf("OSZ: mapper is PE %x\n", _mapper);
	osz_dbg(OSZ_DBG_MAPPER, _mapper);

	if (_pe.inside)
		_osz_close();
}

/* Only the mapper may change the region's zones */
static bool _osz_from_mapper(uint16_t src_addr, uint8_t service)
{
	if (_mapper_valid && src_addr == _mapper)
		return true;

	printf("WARNING: OSZ ignoring broadcast %x from %x, not the mapper\n", service, src_addr);
	osz_dbg(OSZ_DBG_BCAST_IGNORED, ((uint32_t)service << 16) | src_addr);
	return false;
}

static int _osz_release_held(void)
{
	bool released = false;
	for (int i = 0; i < _held_max; i++) {
		if (_held[i] == -1)
			continue;

		tcb_t *tcb = tcb_find(_held[i]);
		_held[i] = -1;

		if (tcb == NULL || tcb_get_sched(tcb) != NULL)
			continue;	/* Aborted meanwhile, or already running */

		if (sched_emplace_back(tcb) == NULL) {
			printf("ERROR: could not release held task %d\n", tcb_get_id(tcb));
			continue;
		}

		printf("OSZ: released held task %d\n", tcb_get_id(tcb));
		osz_dbg(OSZ_DBG_TASK_RELEASED, tcb_get_id(tcb));
		released = true;
	}

	return released && sched_is_idle();
}

int osz_set_rows(uint16_t payload, uint16_t src_addr)
{
	if (_entrance == NULL || !_osz_from_mapper(src_addr, OSZ_ROWS))
		return 0;

	const bool claim = MEMPHIS_OSZ_ROWS_CLAIM(payload);
	const int  lo    = MEMPHIS_OSZ_ROWS_LO(payload);
	const int  hi    = MEMPHIS_OSZ_ROWS_HI(payload);

	for (int r = lo; r <= hi && r < _y; r++)
		_entrance[r] = claim ? hi : -1;

	if (!_pe.inside || _pe.y < lo || _pe.y > hi)
		return 0;

	if (claim) {
		_pe.claimed = true;
		_pe.lo      = lo;
		_pe.hi      = hi;
	} else {
		/* Back to a one-row zone */
		_pe.claimed = false;
		_pe.lo      = _pe.y;
		_pe.hi      = _pe.y;
	}
	_osz_close();

	return claim ? _osz_release_held() : 0;
}

bool osz_inside(void)
{
	return _pe.inside;
}

static bool _osz_bcast(uint8_t service, uint16_t payload)
{
	bcast_t packet;
	packet.service  = service;
	packet.src_addr = MMR_DMNI_INF_ADDRESS;
	packet.payload  = payload;

	for (int i = 0; i < OSZ_BCAST_RETRIES; i++) {
		if (bcast_send(&packet))
			return true;
	}

	/* The application stays open and its rows are never released: the safe side */
	printf("ERROR: OSZ could not broadcast %x payload %x\n", service, payload);
	return false;
}

void osz_report_task_done(int id)
{
	if (_osz_bcast(OSZ_TASK_DONE, (uint16_t)id)) {
		printf("OSZ: reported task %d done\n", id);
		osz_dbg(OSZ_DBG_TASK_DONE, id);
	}
}

int osz_zone_clear(uint16_t payload, uint16_t src_addr)
{
	if (!_pe.inside || !_osz_from_mapper(src_addr, OSZ_ZONE_CLEAR))
		return 0;

	const int lo = MEMPHIS_OSZ_ROWS_LO(payload);
	const int hi = MEMPHIS_OSZ_ROWS_HI(payload);
	if (_pe.y < lo || _pe.y > hi)
		return 0;

	osz_dbg(OSZ_DBG_ZONE_CLEAR, payload);

	/* Anything of the application still here (e.g. an abort still on its way) */
	int removed_current = tcb_remove_all();

	for (int i = 0; i < _held_max; i++)
		_held[i] = -1;

	/* The last buffers this PE sent are still in the kernel heap */
	dmni_clear_sent();

	if (_osz_bcast(OSZ_ZONE_CLEARED, payload)) {
		printf("OSZ: PE %d,%d cleared for rows %d..%d\n", _pe.x, _pe.y, lo, hi);
		osz_dbg(OSZ_DBG_ZONE_CLEARED, payload);
	}

	return removed_current;
}

int osz_forward_report(uint8_t service, uint16_t payload, uint16_t src_addr)
{
	/* The mapper is management task 0 */
	if (tcb_find(0) == NULL)
		return 0;

	memphis_osz_report_t report;
	report.payload  = payload;
	report.service  = service;
	report.pad8     = 0;
	report.src_addr = src_addr;

	int ret = kpipe_add(&report, sizeof(report), 0, MMR_DMNI_INF_ADDRESS);
	if (ret < 0) {
		printf("ERROR: OSZ could not forward %x to the mapper\n", service);
		return 0;
	}

	return ret;
}

bool osz_release_allowed(void)
{
	return (!_pe.inside || _pe.claimed);
}

int osz_hold(int id)
{
	for (int i = 0; i < _held_max; i++) {
		if (_held[i] == -1) {
			_held[i] = id;
			return 0;
		}
	}
	return -ENOMEM;
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
