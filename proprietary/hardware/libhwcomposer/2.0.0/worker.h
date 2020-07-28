#ifndef HWC_WORKER_H_
#define HWC_WORKER_H_

#include <utils/threads.h>
#include <utils/KeyedVector.h>

#include <semaphore.h>

using namespace android;

struct hwc_layer_1;
struct DispatcherJob;
class LayerHandler;
class OverlayEngine;
class SyncControl;
class HWCDisplay;

// ---------------------------------------------------------------------------

// HWC_THREAD_STATE is used for presenting thread's status
enum HWC_THREAD_STATE
{
    HWC_THREAD_IDLE     = 0,
    HWC_THREAD_TRIGGER  = 1,
};

class HWCThread : public Thread
{
public:
    HWCThread() : m_trigger(false), m_state(HWC_THREAD_IDLE)
    {
        sem_init(&m_event, 0, 0);
        memset(m_thread_name, 0, sizeof(m_thread_name));
    }

    // wait() is used to wait until thread is idle
    void wait();

protected:
    mutable Mutex m_lock;
    Condition m_condition;
    sem_t m_event;
    bool m_trigger;

    HWC_THREAD_STATE m_state;
    char m_thread_name[256];

    void waitLocked();

private:
    // readyToRun() would be used to adjust thread priority
    virtual status_t readyToRun();

    // check if need to boost thread priority
    virtual void boost() { }
};

class ComposeThreadBase : public HWCThread
{
public:
    ComposeThreadBase(int dpy, const sp<SyncControl>& sync_ctrl);
    virtual ~ComposeThreadBase();

    // set() is used to config needed data for each layer
    // ex: release fence
    void set(const sp<HWCDisplay>& display, DispatcherJob* job);

    // trigger() is used to start processing layers
    void trigger(DispatcherJob* job);

    // nullop() is used to clear handler state if needed
    virtual void nullop() { }

    // cancelLayers is used to cancel layers of job
    void cancelLayers(DispatcherJob* job);

protected:
    // barrier() is used to check if need to sync with other ComposeThread
    virtual void barrier(DispatcherJob* /*job*/) { }

    // m_disp_id is used to identify this thread is used by which display
    int m_disp_id;

    // m_handler is the main processing module to handle layers in ComposeThread
    sp<LayerHandler> m_handler;

    // m_sync_ctrl is used to do synchronization between each ComposeThread
    sp<SyncControl> m_sync_ctrl;

    char m_trace_tag[256];

private:
    virtual bool threadLoop();
    virtual void onFirstRef();

    virtual void boost();

    void loopHandlerLocked();

    // m_next_job would be set in trigger()
    // and handled in threadLoop()
    DispatcherJob* m_next_job;
};

// UILayerComposer is used to handle layers with CPU/GPU api.
// It should use overlay to blending multiple layers or
// hanle single layer with rotating and scaling request
class UILayerComposer : public ComposeThreadBase
{
public:
    UILayerComposer(int dpy, const sp<SyncControl>& sync_ctrl,
        const sp<OverlayEngine>& ovl_engine);

protected:
    // barrier() is implemetated in UILayerComposer
    // in order to wait MMLayerComposer
    virtual void barrier(DispatcherJob* job);
};

// MMLayerComposer is used to handle layers with MEDIA/CAMERA api
// It should handle N layers with rotating and scaling request
class MMLayerComposer : public ComposeThreadBase
{
public:
    MMLayerComposer(int dpy, const sp<SyncControl>& sync_ctrl,
        const sp<OverlayEngine>& ovl_engine);

    virtual void nullop();

    // increase release fence by specified sync marker
    // This is useful when a mirror job is dropped, and
    // the blit processing needs to be skipped.
    virtual void nullop(int marker, int fd = -1);
};

class LayerComposer : public virtual RefBase
{
public:
    LayerComposer(int dpy, const sp<OverlayEngine>& ovl_engine);
    virtual ~LayerComposer();

    // set() is used to config needed data for each layer
    // ex: release fence
    void set(const sp<HWCDisplay>& display, DispatcherJob* job);

    // trigger() is used to start processing layers
    void trigger(DispatcherJob* job);

    // cancelLayers is used to cancel layers of job
    void cancelLayers(DispatcherJob* job);

    virtual void nullop();

private:
    virtual void boost();

    // m_disp_id is used to identify this thread is used by which display
    int m_disp_id;

    // m_handler is the main processing module to handle layers in ComposeThread
    sp<LayerHandler> m_mm_handler;
    sp<LayerHandler> m_ui_handler;

    // m_next_job would be set in trigger()
    // and handled in threadLoop()
    DispatcherJob* m_next_job;
};
#endif // HWC_WORKER_H_
