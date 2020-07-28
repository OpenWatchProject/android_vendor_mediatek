#include <hwc_tcon.h>

#include "tcon_impl.h"

static class dummySwTcon* gSwTcon = NULL;

static uint16_t dummy_get_version(void)
{
    return TCON_VERSION_1_0;
}

static status_t dummy_create(void)
{
    gSwTcon = new class dummySwTcon();
    if (gSwTcon == NULL)
    {
        return -ENOMEM;
    }

    return 0;
}

static status_t dummy_destroy(void)
{
    if (gSwTcon) {
        delete gSwTcon;
        gSwTcon = NULL;
    }

    return 0;
}

static void dummy_request_buf_size(uint32_t src_width, uint32_t src_height, nsecs_t src_rate, uint32_t* dst_width, uint32_t* dst_height)
{
    gSwTcon->requestBufSize(src_width, src_height, src_rate, dst_width, dst_height);
}

static void dummy_init(struct ProxyBuffer* buf, uint32_t num)
{
    gSwTcon->init(buf, num);
}

static void dummy_release(struct ProxyBuffer* buf, uint32_t num)
{
    gSwTcon->release(buf, num);
}

static unsigned char dummy_set_source_buffer(int32_t mode, void *ptr, size_t size)
{
    return gSwTcon->setSourceBuffer(mode, ptr, size);
}

static unsigned char dummy_get_cooked_buffer(void *ptr, size_t size)
{
    return gSwTcon->getCookedBuffer(ptr, size);
}

static void dummy_reset(void)
{
    gSwTcon->reset();
}

struct tcon_module HWC_TCON_SYM {
    handle: NULL,
    get_version: dummy_get_version,
    create: dummy_create,
    destroy: dummy_destroy,
    request_buf_size: dummy_request_buf_size,
    init: dummy_init,
    release: dummy_release,
    setSourceBuffer: dummy_set_source_buffer,
    getCookedBuffer: dummy_get_cooked_buffer,
    reset: dummy_reset,
};
