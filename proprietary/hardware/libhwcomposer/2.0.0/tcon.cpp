#define DEBUG_LOG_TAG "PROXY"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "tcon.h"
#include "hwc2.h"

#include <string.h>
#include <dlfcn.h>

#if defined(__LP64__)
#define TCON_LIBRARY_PATH "/system/lib64/"
#else
#define TCON_LIBRARY_PATH "/system/lib/"
#endif

#define TCON_LIBRARY_NAME "libtcon"

TconInterface::TconInterface()
    : mTcon(NULL)
{
    void *handle = NULL;
    struct tcon_module *tcon = NULL;
    char path[256];

    memset(path, 0, sizeof(path));
    sprintf(path, "%s%s.so", TCON_LIBRARY_PATH, TCON_LIBRARY_NAME);
    handle = dlopen(path, RTLD_NOW);
    if (handle == NULL)
    {
        ALOGE("failed to load %s", TCON_LIBRARY_NAME);
        return;
    }

    const char *sym = HWC_TCON_SYM_AS_STR;
    tcon = (struct tcon_module*)dlsym(handle, sym);
    if (tcon == NULL)
    {
        dlclose(handle);
        ALOGE("failed to find symbol: %s", sym);
        return;
    }

    tcon->handle = handle;
    mTcon = tcon;

    ALOGI("%s version: %08x", TCON_LIBRARY_NAME, mTcon->get_version());
    mTcon->create();
}

TconInterface::~TconInterface()
{
    void *handle = NULL;
    if (mTcon != NULL)
    {
        handle = mTcon->handle;
        mTcon->destroy();
    }
    if (handle != NULL)
    {
        dlclose(handle);
    }
}

void TconInterface::requestBufSize(uint32_t src_width, uint32_t src_height, nsecs_t src_rate, uint32_t* dst_width, uint32_t* dst_height)
{
    if (mTcon != NULL)
    {
        mTcon->request_buf_size(src_width, src_height, src_rate, dst_width,dst_height);
    }
}

void TconInterface::init(struct ProxyBuffer* buf, uint32_t num)
{
    if (mTcon != NULL)
    {
        mTcon->init(buf, num);
    }
}

void TconInterface::release(struct ProxyBuffer* buf, uint32_t num)
{
    if (mTcon != NULL)
    {
        mTcon->release(buf, num);
    }
}

unsigned char TconInterface::setSourceBuffer(int32_t mode, void *ptr, size_t size)
{
    unsigned char res = 0;

    if (mTcon != NULL)
    {
        res = mTcon->setSourceBuffer(mode, ptr, size);
    }
    return res;
}

unsigned char TconInterface::getCookedBuffer(void *ptr, size_t size)
{
    unsigned char res = 0;

    if (mTcon != NULL)
    {
        res = mTcon->getCookedBuffer(ptr, size);
    }
    return res;
}

void TconInterface::reset(void)
{
    if (mTcon != NULL)
    {
        mTcon->reset();
    }
}
