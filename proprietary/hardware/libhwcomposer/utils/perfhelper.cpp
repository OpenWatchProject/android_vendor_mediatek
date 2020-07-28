#define ATRACE_TAG ATRACE_TAG_GRAPHICS
#include "perfhelper.h"

#include <dlfcn.h>
#include <cutils/log.h>

#define LIBRARY_NAME "libmtkperf_client_vendor.so"

PerfHelper::PerfHelper()
    : m_so_handle(nullptr)
    , m_fn_perf_lock_acq(nullptr)
    , m_fn_perf_lock_rel(nullptr)
{
    m_so_handle = dlopen(LIBRARY_NAME, RTLD_LAZY);
    if (m_so_handle)
    {
        int res = loadApi();
        if (res != 0)
        {
            // failed to load some api, so function may work abnormally.
            // clear all api for disable this feature
            clearApi();
        }
    }
    else
    {
        ALOGE("failed to open library[%s]", LIBRARY_NAME);
    }
}

PerfHelper::~PerfHelper()
{
    if (m_so_handle != nullptr)
    {
        dlclose(m_so_handle);
    }
}

int PerfHelper::loadApi()
{
    int res = 0;
    if (m_so_handle != nullptr)
    {
        m_fn_perf_lock_acq = reinterpret_cast<PerfLockAcq>(dlsym(m_so_handle, "perf_lock_acq"));
        m_fn_perf_lock_rel = reinterpret_cast<PerfLockRel>(dlsym(m_so_handle, "perf_lock_rel"));
        if (m_fn_perf_lock_acq == nullptr)
        {
            ALOGW("failed to find perf_lock_acq");
            res = -EPERM;
        }
        if (m_fn_perf_lock_rel == nullptr)
        {
            ALOGW("failed to find perf_lock_rel");
            res = -EPERM;
        }
    }
    else
    {
        res = -ENOENT;
    }
    return res;
}

void PerfHelper::clearApi()
{
    m_fn_perf_lock_acq = nullptr;
    m_fn_perf_lock_rel = nullptr;
}

void PerfHelper::perfLockAcq(int handle, int duration, int list[], int num_args)
{
    if (m_fn_perf_lock_acq != nullptr)
    {
        m_fn_perf_lock_acq(handle, duration, list, num_args);
    }
}

void PerfHelper::perfLockRel(int handle)
{
    if (m_fn_perf_lock_rel != nullptr)
    {
        m_fn_perf_lock_rel(handle);
    }
}
