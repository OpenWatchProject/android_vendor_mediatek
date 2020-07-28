#ifndef HWC_SYNC_H_
#define HWC_SYNC_H_

#include <utils/threads.h>

using namespace android;

struct DispatcherJob;
#ifdef BOARD_VNDK_SUPPORT
int hwc_sw_sync_timeline_create(void);
int hwc_sw_sync_timeline_inc(int fd, unsigned count);
int hwc_sw_sync_fence_create(int fd, const char *name, unsigned value);
#endif

// ---------------------------------------------------------------------------

class SyncFence : public LightRefBase<SyncFence>
{
public:
    // TIMEOUT_NEVER may be passed to wait() to indicate that it
    // should wait indefinitely for the fence to signal.
    enum { TIMEOUT_NEVER = -1 };

    // SyncFence constructs a new SynceFence object.
    SyncFence(int client);

    ~SyncFence();

    // wait() waits for fd to be signaled or have an error
    // waits indefinitely if warning_timeout < 0
    // return NO_ERROR on success, otherwise, -errno is returned.
    //
    // the log_name argument should be a string identifying the caller
    // and will be included in the log message.
    //
    // <fd> will be closed implicitly before exiting wait()
    status_t wait(int fd, int timeout, const char* log_name = "");

    // waitForever() is a convenience function for waiting forever for a fence
    // to signal (just like wait(TIMEOUT_NEVER)), but issuing an error to the
    // system log and fence state to the kernel log if the wait lasts longer
    // than warning_timeout.
    //
    // the log_name argument should be a string identifying the caller
    // and will be included in the log message.
    //
    // <fd> will be closed implicitly before exiting waitForever()
    status_t waitForever(int fd, int warning_timeout, const char* log_name = "");

    // dump() is used to show sync fence information.
    // sync point information is available only if <fd> is valid
    void dump(int fd = -1);

    // create() creates a sync fence with an internal auto increment sync counter
    //
    // the creator is responsible for closing it when no longer needed
    //
    // return a valid file descriptor on success; otherwise, -1 is returned
    int create();

    // inc() increases thimeline with 1 unit
    status_t inc(int fd);

    // merge() merges two sync fences into a new one sync fence
    // return a valid file descriptor on success; otherwise, -1 is returned
    static int merge(int fd1, int fd2, const char* name = "merged_fence");

    int getLastMarker() { AutoMutex l(m_lock); return m_last_marker; }

    int getCurrMarker() { AutoMutex l(m_lock); return m_curr_marker; }

private:
    // initLocked() initializes timeline
    status_t initLocked();

    // dumpLocked() is the implementation of dump()
    void dumpLocked(int fd);

    int m_client;

    mutable Mutex m_lock;

    int m_sync_timeline_fd;

    // m_curr_marker is the marker on timeline
    int m_curr_marker;

    // m_last_marker is the maker count on timeline
    // to addressing the created sync fence
    int m_last_marker;
};

class SyncControl : public LightRefBase<SyncControl>
{
public:
    struct SyncListener : public virtual RefBase
    {
        // onTrigger() is called to notify to trigger overlay engine
        virtual void onTrigger(DispatcherJob* job) = 0;
    };

    SyncControl(const sp<SyncListener>& listener)
        : m_listener(listener)
    { }

    ~SyncControl();

    void setSync(DispatcherJob* job);

    void wait(DispatcherJob* job);

    void setOverlay(DispatcherJob* job);

private:
    mutable Mutex m_lock;
    Condition m_condition;

    sp<SyncListener> m_listener;
};

#endif // HWC_SYNC_H_
