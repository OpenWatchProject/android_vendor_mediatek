#define DEBUG_LOG_TAG "SYNC"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#include "hwc_priv.h"

#include <sync/sync.h>
#include <sw_sync.h>
#ifdef BOARD_VNDK_SUPPORT
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "utils/debug.h"

#include "dispatcher.h"
#include "sync.h"
#include "queue.h"
#include "overlay.h"

//#define DUMP_SYNC_INFO

#ifndef USE_NATIVE_FENCE_SYNC
//#error "Native Fence Sync is not supported!!"
#endif

#define SYNC_LOGV(x, ...) HWC_LOGV("(%p) " x, this, ##__VA_ARGS__)
#define SYNC_LOGD(x, ...) HWC_LOGD("(%p) " x, this, ##__VA_ARGS__)
#define SYNC_LOGI(x, ...) HWC_LOGI("(%p) " x, this, ##__VA_ARGS__)
#define SYNC_LOGW(x, ...) HWC_LOGW("(%p) " x, this, ##__VA_ARGS__)
#define SYNC_LOGE(x, ...)                                           \
        {                                                           \
            DbgLogger logger(DbgLogger::TYPE_HWC_LOG                \
                            | DbgLogger::TYPE_FENCE,                \
                            'E',                                    \
                            " ! " x, ##__VA_ARGS__);                \
        }

#ifdef BOARD_VNDK_SUPPORT
struct sw_sync_create_fence_data {
  __u32 value;
  char name[32];
  __s32 fence;
};

#define SW_SYNC_IOC_MAGIC 'W'
#define SW_SYNC_IOC_CREATE_FENCE _IOWR(SW_SYNC_IOC_MAGIC, 0, struct sw_sync_create_fence_data)
#define SW_SYNC_IOC_INC _IOW(SW_SYNC_IOC_MAGIC, 1, __u32)

int hwc_sw_sync_timeline_create(void)
{
    int fd = open("/sys/kernel/debug/sync/sw_sync", O_RDWR);
    if (fd < 0)
    {
        fd = open("/dev/sw_sync", O_RDWR);
    }

    return fd;
}
int hwc_sw_sync_timeline_inc(int fd, unsigned count)
{
    uint32_t arg = count;
    int err;
    err = ioctl(fd, SW_SYNC_IOC_INC, &arg);

    return err;

}
int hwc_sw_sync_fence_create(int fd, const char *name, unsigned value)
{
    struct sw_sync_create_fence_data data;

    int err;

    data.value = value;
    strlcpy(data.name, name, sizeof(data.name));

    err = ioctl(fd, SW_SYNC_IOC_CREATE_FENCE, &data);
    if (err < 0)
    {
        HWC_LOGE("SyncFence: createFence error %d\n", err);
        fd = err;
    }
    else
    {
        fd = data.fence;
    }

    return fd;
}
#endif

// ---------------------------------------------------------------------------
SyncFence::SyncFence(int client)
    : m_client(client)
    , m_sync_timeline_fd(-1)
    , m_curr_marker(0)
    , m_last_marker(0)
{
}

SyncFence::~SyncFence()
{
    if (m_sync_timeline_fd != -1)
    {
        SYNC_LOGD("Close timeline(%d) Clear Marker(%d->%d)",
            m_sync_timeline_fd, m_curr_marker, m_last_marker);
        int diff = m_last_marker - m_curr_marker;
        while (diff--) inc(-1);
        protectedClose(m_sync_timeline_fd);
    }
}

status_t SyncFence::initLocked()
{
#ifdef USE_NATIVE_FENCE_SYNC
#ifdef BOARD_VNDK_SUPPORT
    m_sync_timeline_fd = hwc_sw_sync_timeline_create();
#else
    m_sync_timeline_fd = sw_sync_timeline_create();
#endif
    if (m_sync_timeline_fd < 0)
    {
        SYNC_LOGE("Failed to create sw_sync_timeline: %s", strerror(errno));
        m_sync_timeline_fd = -1;
        return INVALID_OPERATION;
    }

    SYNC_LOGD("Open timeline(%d)", m_sync_timeline_fd);
#endif

    return NO_ERROR;
}

status_t SyncFence::wait(int fd, int timeout, const char* log_name)
{
    char atrace_tag[256];
    sprintf(atrace_tag, "wait_fence(%d)\n", fd);
    HWC_ATRACE_NAME(atrace_tag);

    if (fd == -1) return NO_ERROR;

    int err = sync_wait(fd, timeout);
    if (err < 0 && errno == ETIME)
    {
        HWC_ATRACE_NAME("timeout");

        SYNC_LOGE("[%s] (%d) fence %d didn't signal in %u ms",
            log_name, m_client, fd, timeout);

        dumpLocked(fd);
    }

    protectedClose(fd);

    SYNC_LOGV("[%s] (%d) wait and close fence %d within %d",
        log_name, m_client, fd, timeout);

    return err < 0 ? -errno : status_t(NO_ERROR);
}

status_t SyncFence::waitForever(int fd, int warning_timeout, const char* log_name)
{
    if (fd == -1) return NO_ERROR;

    int err = sync_wait(fd, warning_timeout);
    if (err < 0 && errno == ETIME)
    {
        //SYNC_LOGE("[%s] (%d) fence %d didn't signal in %u ms",
        //    log_name, m_client, fd, warning_timeout);

        dumpLocked(fd);
        err = sync_wait(fd, TIMEOUT_NEVER);
    }

    protectedClose(fd);

    SYNC_LOGV("[%s] (%d) wait and close fence %d", log_name, m_client, fd);

    return err < 0 ? -errno : status_t(NO_ERROR);
}

void SyncFence::dump(int fd)
{
    AutoMutex l(m_lock);
    dumpLocked(fd);
}

void SyncFence::dumpLocked(int fd)
{
    // sync fence info
    SYNC_LOGE("timeline fd(%d) curr(%d) last(%d)",
        m_sync_timeline_fd, m_curr_marker, m_last_marker);

    if (-1 == fd) return;

    // sync point info
    struct sync_file_info *finfo = sync_file_info(fd);
    if (finfo)
    {
        // status: active(0) signaled(1) error(<0)
        SYNC_LOGE("fence(%s) status(%d)\n", finfo->name, finfo->status);

        // iterate all sync points
        struct sync_fence_info* pinfo = sync_get_fence_info(finfo);
        for (size_t i = 0; i < finfo->num_fences; i++)
        {
            int ts_sec = pinfo[i].timestamp_ns / 1000000000LL;
            int ts_usec = (pinfo[i].timestamp_ns % 1000000000LL) / 1000LL;

            SYNC_LOGE("sync point: timeline(%s) drv(%s) status(%d) timestamp(%d.%06d)",
                    pinfo[i].obj_name,
                    pinfo[i].driver_name,
                    pinfo[i].status,
                    ts_sec, ts_usec);
        }
        sync_file_info_free(finfo);
    }
}

int SyncFence::create()
{
    AutoMutex l(m_lock);

    if (m_sync_timeline_fd < 0)
    {
        SYNC_LOGW("create fence fail: timeline doesn't exist, try to create");
        if (NO_ERROR != initLocked()) return -1;
        SYNC_LOGD("timeline is created");
    }

    m_last_marker = m_last_marker + 1;

#ifdef BOARD_VNDK_SUPPORT
    int fd = hwc_sw_sync_fence_create(m_sync_timeline_fd, DEBUG_LOG_TAG, m_last_marker);
#else
    int fd = sw_sync_fence_create(m_sync_timeline_fd, DEBUG_LOG_TAG, m_last_marker);
#endif
    if (fd < 0)
    {
        SYNC_LOGE("can't create sync point: %s", strerror(errno));
        return -1;
    }

    SYNC_LOGD("create fence(%d) curr(%d) last(%d)", fd, m_curr_marker, m_last_marker);

#ifdef DUMP_SYNC_INFO
    dumpLocked(fd);
#endif

    return fd;
}

status_t SyncFence::inc(int fd)
{
    AutoMutex l(m_lock);

    if (m_sync_timeline_fd < 0)
    {
        SYNC_LOGE("signal fence fail: timeline doesn't exist");
        return INVALID_OPERATION;
    }

    m_curr_marker = m_curr_marker + 1;

#ifdef BOARD_VNDK_SUPPORT
    int err = hwc_sw_sync_timeline_inc(m_sync_timeline_fd, 1);
#else
    int err = sw_sync_timeline_inc(m_sync_timeline_fd, 1);
#endif
    if (err < 0)
    {
        SYNC_LOGE("can't increment sync object: %s", strerror(errno));
        // align sync counter to driver data
        dumpLocked(fd);
        m_curr_marker -= 1;
        return -errno;
    }

    SYNC_LOGD("inc fence(%d) curr(%d) last(%d)", fd, m_curr_marker, m_last_marker);

#ifdef DUMP_SYNC_INFO
    dumpLocked(fd);
#endif

    return NO_ERROR;
}

int SyncFence::merge(int fd1, int fd2, const char* name)
{
    int fd3;

    if (fd1 >= 0 && fd2 >= 0)
    {
        fd3 = sync_merge(name, fd1, fd2);
    }
    else if (fd1 >= 0)
    {
        fd3 = sync_merge(name, fd1, fd1);
    }
    else if (fd2 >= 0)
    {
        fd3 = sync_merge(name, fd2, fd2);
    }
    else
    {
        return -1;
    }

    // check status of merged fence
    if (fd3 < 0)
    {
        HWC_LOGE("merge fences(%d, %d) fail: %s (%d)", fd1, fd2, strerror(errno), -errno);
        return -1;
    }

    HWC_LOGD("merge fences(%d, %d) into fence(%d)", fd1, fd2, fd3);

    return fd3;
}

// ---------------------------------------------------------------------------

SyncControl::~SyncControl()
{
    m_listener = NULL;
}

void SyncControl::setSync(DispatcherJob* job)
{
    job->need_sync = ((job->mm_fbt ? 0 : (int)job->fbt_exist) + job->num_ui_layers) &&
                      (job->num_mm_layers || job->mm_fbt);
    return;
}

void SyncControl::wait(DispatcherJob* job)
{
    HWC_ATRACE_NAME("wait_sync");

    AutoMutex l(m_lock);

    while (job->need_sync)
    {
        HWC_LOGD("(%d) Wait bliter done", job->disp_ori_id);
        status_t ret = m_condition.waitRelative(m_lock, ms2ns(5));
        if (ret != NO_ERROR)
        {
            if (ret == TIMED_OUT)
            {
                HWC_LOGW("(%d) Bliting is still not finished", job->disp_ori_id);
            }
            else
            {
                HWC_LOGE("(%d) To wait bliter failed(%d)", job->disp_ori_id, ret);
            }
        }
    }
}

void SyncControl::setOverlay(DispatcherJob* job)
{
    if (job->need_sync)
    {
        job->need_sync = false;
        m_condition.signal();
        return;
    }

    if (!job->triggered)
    {
        m_listener->onTrigger(job);
        job->triggered = true;
    }
    else
    {
        if (job->disp_ori_id < HWC_DISPLAY_VIRTUAL)
            m_listener->onTrigger(job);
    }
}
