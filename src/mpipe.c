#include <mpipe.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <mmr.h>
#include <hermes.h>
#include <dmni.h>

#include <memphis/services.h>

typedef struct _mpipe {
    void    *buffer;
    int      owner; 
    size_t   size;
    size_t   length;
    size_t   index;
} mpipe_t;

static mpipe_t _pipe;

void mpipe_init()
{
    _pipe.buffer = NULL;
    _pipe.owner  = -1;
    _pipe.size   = 0;
    _pipe.length = 0;
    _pipe.index  = 0;
}

int mpipe_create(size_t size, size_t len, int task)
{
    if (task >> 8 != 0 || size % 4 != 0)
        return -EINVAL;

    _pipe.buffer = malloc(size*len);
    if (_pipe.buffer == NULL)
        return -ENOMEM;

    _pipe.length = len;
    _pipe.size   = size;
    _pipe.owner  = task;
    _pipe.index  = 0;

    MMR_DMNI_MON_BASE   = (unsigned)_pipe.buffer;
    MMR_DMNI_MON_SEM_OC = 0;
    MMR_DMNI_MON_SEM_AV = len;
    return 0;
}

int mpipe_wait(int id, size_t max_size)
{
    if (_pipe.buffer == NULL)
        return -EAGAIN;

    if (id != _pipe.owner || max_size < _pipe.size)
        return -EINVAL;

    return MMR_DMNI_MON_SEM_OC;
}

size_t mpipe_read(void *dst)
{
    /* Semaphore should have waited here */
    memcpy(dst, _pipe.buffer + _pipe.index*_pipe.size, _pipe.size);
    _pipe.index = (_pipe.index + 1) % _pipe.length;

    return _pipe.size;
}

void mpipe_post()
{
    MMR_DMNI_MON_SEM_AV = -1;
}

int mpipe_write(void *buf, size_t size, int16_t addr)
{
    if (size % 4 != 0)
        return -EINVAL;
    
    void *packet = malloc(sizeof(hermes_t) + size);
    if (packet == NULL)
        return -ENOMEM;

    ((hermes_t*)packet)->flags   = 0;
    ((hermes_t*)packet)->service = MONITOR;
    ((hermes_t*)packet)->address = addr;
    memcpy(packet + sizeof(hermes_t), buf, size);

    return dmni_send(packet, sizeof(hermes_t) + size, true, NULL, 0, false);
}
