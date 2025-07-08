#include <mpipe.h>
#include <mmr.h>

#include <errno.h>
#include <string.h>
#include <stdlib.h>

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

    if (max_size < _pipe.size || id != _pipe.owner)
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
