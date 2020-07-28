#define DEBUG_LOG_TAG "WKR"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "hwc_priv.h"

#include "utils/debug.h"
#include "dispatcher.h"
#include "overlay.h"
#include "worker.h"
#include "display.h"
#include "composer.h"
#include "bliter_async.h"
#include "sync.h"
#include "platform.h"

// ---------------------------------------------------------------------------

status_t HWCThread::readyToRun()
{
    boost();
    m_state = HWC_THREAD_IDLE;

    return NO_ERROR;
}

void HWCThread::waitLocked()
{
    // make sure m_state is not out of expect
    assert((m_state == HWC_THREAD_TRIGGER) || (m_state == HWC_THREAD_IDLE));

    int timeout_count = 0;
    while (m_state == HWC_THREAD_TRIGGER)
    {

        // WORKAROUND: After HWCThread know which display uses itself,
        // it can set a more reasonable timeout value
        if (m_condition.waitRelative(m_lock, ms2ns(50)) == TIMED_OUT)
        {
            int sem_value;
            sem_getvalue(&m_event, &sem_value);

            if (20 == timeout_count)
            {
                HWC_LOGE("Timed out waiting for %s (cnt=%d/val=%d)",
                        m_thread_name, timeout_count, sem_value);
            }

            if (timeout_count & 0x1)
            {
                HWC_LOGW("Timed out waiting for %s (cnt=%d/val=%d)",
                        m_thread_name, timeout_count, sem_value);
            }

            timeout_count++;
        }
    }
}

void HWCThread::wait()
{
    HWC_ATRACE_CALL();
    AutoMutex l(m_lock);
    HWC_LOGD("Waiting for %s...", m_thread_name);
    waitLocked();
}

// ---------------------------------------------------------------------------

ComposeThreadBase::ComposeThreadBase(int dpy, const sp<SyncControl>& sync_ctrl)
    : HWCThread()
    , m_disp_id(dpy)
    , m_handler(NULL)
    , m_sync_ctrl(sync_ctrl)
    , m_next_job(NULL)
{
    memset(m_thread_name, 0, sizeof(m_thread_name));
    memset(m_trace_tag, 0, sizeof(m_trace_tag));
}

ComposeThreadBase::~ComposeThreadBase()
{
    m_handler = NULL;
    m_sync_ctrl = NULL;
}

void ComposeThreadBase::onFirstRef()
{
    run(m_thread_name, PRIORITY_URGENT_DISPLAY);
}

void ComposeThreadBase::boost()
{
    m_handler->boost();
}

void ComposeThreadBase::loopHandlerLocked()
{
    if (m_next_job == NULL)
        return;

    // check if need to wait other compose thread
    barrier(m_next_job);

    m_handler->process(m_next_job);

    m_sync_ctrl->setOverlay(m_next_job);

    m_next_job = NULL;
}

bool ComposeThreadBase::threadLoop()
{
    sem_wait(&m_event);

    {
        AutoMutex l(m_lock);

#ifndef MTK_USER_BUILD
        HWC_ATRACE_NAME(m_trace_tag);
#endif

        loopHandlerLocked();

        m_state = HWC_THREAD_IDLE;
        m_condition.signal();
    }

    return true;
}

void ComposeThreadBase::set(
    const sp<HWCDisplay>& display,
    DispatcherJob* job)
{
    if (m_handler != NULL) m_handler->set(display, job);
}

void ComposeThreadBase::trigger(DispatcherJob* job)
{
    AutoMutex l(m_lock);

    m_next_job = job;

    m_state = HWC_THREAD_TRIGGER;

    sem_post(&m_event);
}

void ComposeThreadBase::cancelLayers(DispatcherJob* job)
{
    if (m_handler != NULL) m_handler->cancelLayers(job);
}

// ---------------------------------------------------------------------------

UILayerComposer::UILayerComposer(
    int dpy, const sp<SyncControl>& sync_ctrl, const sp<OverlayEngine>& ovl_engine)
    : ComposeThreadBase(dpy, sync_ctrl)
{
    m_handler = new ComposerHandler(m_disp_id, ovl_engine);

    snprintf(m_trace_tag, sizeof(m_trace_tag), "compose1_%d", dpy);

    snprintf(m_thread_name, sizeof(m_thread_name), "UICompThread_%d", dpy);
}

void UILayerComposer::barrier(DispatcherJob* job)
{
    m_sync_ctrl->wait(job);
}

// ---------------------------------------------------------------------------

MMLayerComposer::MMLayerComposer(
    int dpy, const sp<SyncControl>& sync_ctrl, const sp<OverlayEngine>& ovl_engine)
    : ComposeThreadBase(dpy, sync_ctrl)
{
    m_handler = new AsyncBliterHandler(m_disp_id, ovl_engine);

    snprintf(m_trace_tag, sizeof(m_trace_tag), "compose2_%d", dpy);

    snprintf(m_thread_name, sizeof(m_thread_name), "MMCompThread_%d", dpy);
}

void MMLayerComposer::nullop()
{
    if (m_handler != NULL) m_handler->nullop();
}

void MMLayerComposer::nullop(int marker, int fd)
{
    if (m_handler != NULL) m_handler->nullop(marker, fd);
}

// -----------------------------------------------------------------------------------------------

LayerComposer::LayerComposer(int dpy, const sp<OverlayEngine>& ovl_engine)
    : m_disp_id(dpy)
    , m_next_job(NULL)
{
    m_mm_handler = new AsyncBliterHandler(m_disp_id, ovl_engine);
    m_ui_handler = new ComposerHandler(m_disp_id, ovl_engine);

    if (m_mm_handler == NULL || m_ui_handler == NULL)
    {
        HWC_LOGE("NULL LayerComposer handler");
    }
}

LayerComposer::~LayerComposer()
{
    m_mm_handler = NULL;
    m_ui_handler = NULL;
}

void LayerComposer::boost()
{
    if (m_mm_handler == NULL || m_ui_handler == NULL)
    {
        HWC_LOGE("NULL LayerComposer handler");
        return;
    }

    m_mm_handler->boost();
    m_ui_handler->boost();
}

void LayerComposer::set(
    const sp<HWCDisplay>& display,
    DispatcherJob* job)
{
    if (m_mm_handler == NULL || m_ui_handler == NULL)
    {
        HWC_LOGE("NULL LayerComposer handler");
        return;
    }

    m_mm_handler->set(display, job);
    m_ui_handler->set(display, job);
}

void LayerComposer::trigger(DispatcherJob* job)
{
    if (m_mm_handler == NULL || m_ui_handler == NULL)
    {
        HWC_LOGE("NULL LayerComposer handler");
        return;
    }

    m_next_job = job;

    if (m_next_job == NULL)
        return;

    if (0 == job->num_mm_layers && !job->is_black_job)
        m_mm_handler->nullop();
    else
        m_mm_handler->process(m_next_job);

    m_ui_handler->process(m_next_job);

    m_next_job = NULL;
}

void LayerComposer::cancelLayers(DispatcherJob* job)
{
    if (m_mm_handler == NULL || m_ui_handler == NULL)
    {
        HWC_LOGE("NULL LayerComposer handler");
        return;
    }

    m_mm_handler->cancelLayers(job);
    m_ui_handler->cancelLayers(job);
}

void LayerComposer::nullop()
{
    if (m_mm_handler == NULL)
    {
        HWC_LOGE("NULL LayerComposer handler");
        return;
    }

    m_mm_handler->nullop();
}
