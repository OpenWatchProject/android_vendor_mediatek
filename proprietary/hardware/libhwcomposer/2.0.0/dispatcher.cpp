#define DEBUG_LOG_TAG "JOB"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include "hwc_priv.h"
#include <sched.h>

#include "utils/debug.h"
#include "utils/tools.h"

#include "dispatcher.h"
#include "display.h"
#include "overlay.h"
#include "queue.h"
#include "sync.h"
#include "platform.h"
#include "hwc2.h"
//#include "epaper_post_processing.h"
#include <cutils/properties.h>

#define HWC_ATRACE_JOB(string, n1, n2, n3, n4)                                \
    if (ATRACE_ENABLED()) {                                                   \
        char ___traceBuf[1024];                                               \
        snprintf(___traceBuf, 1024, "%s(%d): %d %d %d", (string),             \
            (n1), (n2), (n3), (n4));                                          \
        android::ScopedTrace ___bufTracer(ATRACE_TAG, ___traceBuf);           \
    }

// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE(HWCDispatcher);

HWCDispatcher::HWCDispatcher()
    : m_sequence(1)
    , m_session_mode_changed(0)
    , m_disable_skip_redundant(false)
    , m_ultra_scenario(false)
{
    // bypass getJob then call setJob first time could cause SIGSEGV
    memset(m_curr_jobs, 0, sizeof(m_curr_jobs));
    m_vsync_callbacks.resize(DisplayManager::MAX_DISPLAYS);

    for (uint32_t i = 0; i < DisplayManager::MAX_DISPLAYS; i++)
    {
        m_prev_fbt[i] = NULL;
    }
}

HWCDispatcher::~HWCDispatcher()
{
    for (uint32_t i = 0; i < DisplayManager::MAX_DISPLAYS; i++)
    {
        if (m_alloc_disp_ids.hasBit(i))
        {
            onPlugOut(i);
        }
    }
}

DispatcherJob* HWCDispatcher::getJob(int dpy)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif

    if (dpy >= DisplayManager::MAX_DISPLAYS)
        return NULL;

    DispatcherJob* job = NULL;
    {
        AutoMutex l(m_workers[dpy].plug_lock_main);

        if (!m_workers[dpy].enable)
        {
            HWC_LOGE("Failed to get job: dpy(%d) is not enabled", dpy);
        }
        else if (!m_workers[dpy].ovl_engine->isEnable())
        {
            if (DisplayManager::getInstance().getDisplayPowerState(dpy))
                HWC_LOGI("ovl_%d is not enable, do not produce a job", dpy);
        }
        else
        {
            job = new DispatcherJob();
            job->disp_ori_id = dpy;
            job->sequence = m_sequence++;

            job->num_layers = m_workers[dpy].ovl_engine->getAvailableInputNum();
            if (job->num_layers <= 0)
            {
                // reserve one dump layer for gpu compoisition with FBT
                // if there's np available input layers
                // then post handler would wait until OVL is available
                if (dpy < HWC_DISPLAY_VIRTUAL ||
                    m_workers[HWC_DISPLAY_PRIMARY].ovl_engine->getOverlaySessionMode() != HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE)
                {
                    HWC_LOGW("No available overlay resource (input_num=%d)", job->num_layers);
                }

                job->num_layers = 1;
            }
            else
            {
                job->ovl_valid = true;
            }

            job->hw_layers = (HWLayer*)calloc(1, sizeof(HWLayer) * job->num_layers);
            LOG_ALWAYS_FATAL_IF(job->hw_layers == nullptr, "hw_layers calloc(%zu) fail",
                sizeof(HWLayer) * job->num_layers);

            for (int32_t i = 0; i < job->num_layers; ++i)
            {
                job->hw_layers[i].hwc2_layer_id = -1;
            }

            HWC_DISP_MODE display_session_mode = m_workers[dpy].ovl_engine->getOverlaySessionMode();
            if (display_session_mode == HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE ||
                display_session_mode == HWC_DISP_SESSION_DIRECT_LINK_MIRROR_MODE) {
                job->need_output_buffer = true;
            }

            if (Platform::getInstance().m_config.is_support_mdp_pmqos_debug)
            {
                MDPFrameInfoDebugger::getInstance().checkMDPLayerExecTime();
                MDPFrameInfoDebugger::getInstance().insertJob(job->sequence);
            }
        }

        m_curr_jobs[dpy] = job;
    }

    return job;
}

DispatcherJob* HWCDispatcher::getExistJob(int dpy)
{
    if (dpy >= DisplayManager::MAX_DISPLAYS)
        return NULL;

    {
        AutoMutex l(m_workers[dpy].plug_lock_main);
        return m_curr_jobs[dpy];
    }
}

bool HWCDispatcher::decideCtDirtyAndFlush(int dpy, int idx)
{
    if (idx >= static_cast<int32_t>(m_workers[dpy].prev_hw_layers.size()))
    {
        return true;
    }

    if (m_workers[dpy].prev_hw_layers[idx].type != HWC_LAYER_TYPE_FBT)
    {
        m_curr_jobs[dpy]->need_flush = true;
        return true;
    }

    return false;
}

bool HWCDispatcher::decideDirtyAndFlush(
    int dpy, const PrivateHandle* priv_handle, int idx, int dirty, int type,
    const int64_t& hwc2_layer_id, const int32_t& layer_caps, const bool& need_pq)
{
    if (idx >= static_cast<int32_t>(m_workers[dpy].prev_hw_layers.size()))
    {
        return true;
    }

    bool ret = dirty ? true : false;

    const WorkerCluster::PrevHwLayer& prev_hw_layer = m_workers[dpy].prev_hw_layers[idx];
    const int pool_id = priv_handle->ext_info.pool_id;

    if (prev_hw_layer.type != type ||
        prev_hw_layer.pool_id != pool_id ||
        !prev_hw_layer.enable||
        prev_hw_layer.hwc2_layer_id != hwc2_layer_id ||
        prev_hw_layer.layer_caps != layer_caps ||
        prev_hw_layer.pq_enable != priv_handle->pq_enable ||
        prev_hw_layer.pq_pos != priv_handle->pq_pos ||
        prev_hw_layer.pq_orientation != priv_handle->pq_orientation ||
        prev_hw_layer.pq_table_idx != priv_handle->pq_table_idx ||
        prev_hw_layer.need_pq != need_pq)
    {
        m_curr_jobs[dpy]->need_flush = true;

        if (ret == HWC_LAYER_DIRTY_NONE)
            ret = true;
    }

    HWC_LOGV("decideDirtyAndFlush() ret:%d type:%d(%d) pool_id:%d(%d), layer_id:%" PRId64 "(%" PRId64") needPQ(%d)",
        ret, type, prev_hw_layer.type, pool_id, prev_hw_layer.pool_id, hwc2_layer_id, prev_hw_layer.hwc2_layer_id, need_pq);

    return ret;
}

void HWCDispatcher::initPrevHwLayers(const sp<HWCDisplay>& display, DispatcherJob* job)
{
    const int64_t disp_id = display->getId();
    AutoMutex l(m_workers[disp_id].plug_lock_main);

    if (static_cast<int32_t>(m_workers[disp_id].prev_hw_layers.size()) != job->num_layers)
        m_workers[disp_id].prev_hw_layers.resize(job->num_layers);
}

void HWCDispatcher::fillPrevHwLayers(const sp<HWCDisplay>& display, DispatcherJob* job)
{
    const int64_t disp_id = display->getId();
    AutoMutex l(m_workers[disp_id].plug_lock_main);
    auto& prev_hw_layers = m_workers[disp_id].prev_hw_layers;
    for (size_t i = 0; i < prev_hw_layers.size(); ++i)
    {
        prev_hw_layers[i].type              = job->hw_layers[i].type;
        prev_hw_layers[i].pool_id           = job->hw_layers[i].priv_handle.ext_info.pool_id;
        prev_hw_layers[i].enable            = job->hw_layers[i].enable;
        prev_hw_layers[i].hwc2_layer_id     = job->hw_layers[i].hwc2_layer_id;
        prev_hw_layers[i].layer_caps        = job->hw_layers[i].layer_caps;
        prev_hw_layers[i].pq_enable         = job->hw_layers[i].priv_handle.pq_enable;
        prev_hw_layers[i].pq_pos            = job->hw_layers[i].priv_handle.pq_pos;
        prev_hw_layers[i].pq_orientation    = job->hw_layers[i].priv_handle.pq_orientation;
        prev_hw_layers[i].pq_table_idx      = job->hw_layers[i].priv_handle.pq_table_idx;
    }
}

HWC_DISP_MODE HWCDispatcher::getSessionMode(const int& dpy)
{
    return m_workers[dpy].ovl_engine->getOverlaySessionMode();
}

void HWCDispatcher::setSessionMode(int dpy, bool mirrored)
{
    HWC_DISP_MODE prev_session_mode = HWC_DISP_INVALID_SESSION_MODE;
    HWC_DISP_MODE curr_session_mode = HWC_DISP_INVALID_SESSION_MODE;

    // get previous and current session mode
    {
        AutoMutex l(m_workers[dpy].plug_lock_main);
        if (m_workers[dpy].enable && m_workers[dpy].ovl_engine != NULL)
        {
            prev_session_mode = m_workers[dpy].ovl_engine->getOverlaySessionMode();
            if (HWC_DISP_INVALID_SESSION_MODE == prev_session_mode)
            {
                HWC_LOGW("Failed to set session mode: dpy(%d)", dpy);
                return;
            }
        }

        curr_session_mode = mirrored ?
            HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE : HWC_DISP_SESSION_DIRECT_LINK_MODE;
    }

    // flush all pending display jobs of displays before
    // session mode transition occurs (mirror <-> extension)
    // TODO: refine this logic if need to flush jobs for other session mode transition
    bool mode_transition =
            mirrored != ((HWC_DISP_SESSION_DECOUPLE_MIRROR_MODE == prev_session_mode) ||
                         (HWC_DISP_SESSION_DIRECT_LINK_MIRROR_MODE == prev_session_mode));
    if (mode_transition)
    {
        for (uint32_t i = 0; i < DisplayManager::MAX_DISPLAYS; i++)
        {
            AutoMutex l(m_workers[i].plug_lock_main);

            if (m_workers[i].enable)
            {
                m_workers[i].dp_thread->wait();
                m_workers[i].ovl_engine->wait();
                HWCMediator::getInstance().getOvlDevice(i)->waitAllJobDone(i);
            }
        }
    }

    // set current session mode
    // TODO: should set HWC_DISPLAY_EXTERNAL/HWC_DISPLAY_VIRTUAL separately?
    {
        AutoMutex l(m_workers[dpy].plug_lock_main);

        if (m_workers[dpy].enable && m_workers[dpy].ovl_engine != NULL)
            m_workers[dpy].ovl_engine->setOverlaySessionMode(curr_session_mode);
    }

    if (CC_UNLIKELY(prev_session_mode != curr_session_mode))
    {
        HWC_LOGD("change session mode (dpy=%d/mir=%c/%s -> %s)",
            dpy, mirrored ? 'y' : 'n',
            getSessionModeString(prev_session_mode).string(),
            getSessionModeString(curr_session_mode).string());

        incSessionModeChanged();
    }
}

void HWCDispatcher::configMirrorOutput(DispatcherJob* job, const int& display_color_mode)
{
    int dpy = job->disp_ori_id;

    // temporary solution to prevent error of MHL plug-out
    if (dpy >= DisplayManager::MAX_DISPLAYS) return;

    AutoMutex l(m_workers[dpy].plug_lock_main);

    if (!m_workers[dpy].enable)
        return;

    if (job->need_output_buffer)
    {
        status_t err = m_workers[dpy].ovl_engine->configMirrorOutput(job);
        job->hw_outbuf.dataspace = mapColorMode2DataSpace(display_color_mode);
        if (CC_LIKELY(err == NO_ERROR))
        {
            if (!job->mirrored) {
                if (job->hw_outbuf.mir_out_acq_fence_fd != -1)
                {
                    ::protectedClose(job->hw_outbuf.mir_out_acq_fence_fd);
                    job->hw_outbuf.mir_out_acq_fence_fd = -1;
                }
            }
        }
        else
        {
            // error handling, cancel mirror mode if failing to configure mirror output
            job->need_output_buffer = false;
            m_workers[dpy].ovl_engine->setOverlaySessionMode(HWC_DISP_SESSION_DIRECT_LINK_MODE);
        }
    }

}

void HWCDispatcher::configMirrorJob(DispatcherJob* job)
{
    HWC_LOGV("configMirrorJob job->disp_mir_id:%d", job->disp_mir_id);
    int mir_dpy = job->disp_mir_id;
    int ori_dpy = job->disp_ori_id;

    if (mir_dpy >= DisplayManager::MAX_DISPLAYS) return;
    // temporary solution to prevent error of MHL plug-out
    if (ori_dpy >= DisplayManager::MAX_DISPLAYS) return;

    AutoMutex lm(m_workers[ori_dpy].plug_lock_main);
    AutoMutex l(m_workers[mir_dpy].plug_lock_main);

    if (m_workers[ori_dpy].enable)
    {
        DispatcherJob* mir_job = m_curr_jobs[mir_dpy];

        if (NULL == mir_job)
        {
            HWC_LOGI("configMirrorJob mir_job is NULL");
            return;
        }

        // enable mirror source
        mir_job->mirrored = true;

        // keep the orientation of mirror source in mind
        job->disp_mir_rot = mir_job->disp_ori_rot;

        // sync secure setting from mirror source
        job->secure = mir_job->secure;
    }
}

void HWCDispatcher::disableMirror(const sp<HWCDisplay>& display, DispatcherJob* job)
{
    if (job->hw_mirbuf.mir_in_acq_fence_fd != -1)
    {
        ::protectedClose(job->hw_mirbuf.mir_in_acq_fence_fd);
        job->hw_mirbuf.mir_in_acq_fence_fd = -1;
    }
    if (job->fill_black[0].fence != -1)
    {
        ::protectedClose(job->fill_black[0].fence);
        job->fill_black[0].fence = -1;
    }
    if (job->fill_black[1].fence != -1)
    {
        ::protectedClose(job->fill_black[1].fence);
        job->fill_black[1].fence = -1;
    }
    if (job->hw_mirbuf.mir_in_rel_fence_fd != -1)
    {
        ::protectedClose(job->hw_mirbuf.mir_in_rel_fence_fd);
        job->hw_mirbuf.mir_in_rel_fence_fd = -1;
    }
    display->clearAllFences();
}

void HWCDispatcher::setJob(const sp<HWCDisplay>& display)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif
    const uint64_t dpy = display->getId();

    if (dpy >= DisplayManager::MAX_DISPLAYS)
        return;

    {
        AutoMutex l(m_workers[dpy].plug_lock_main);

        if (CC_UNLIKELY(!m_workers[dpy].enable))
        {
            // it can happen if SurfaceFlinger tries to set display job
            // right after hotplug thread just removed a display (e.g. via onPlugOut())
            HWC_LOGW("Failed to set job: dpy(%" PRIu64 ") is not enabled", dpy);
            display->clearAllFences();
            return;
        }

        if (!m_workers[dpy].ovl_engine->isEnable())
        {
            HWC_LOGD("(%" PRIu64 ") SET/bypass/blank", dpy);
            display->clearAllFences();
            return;
        }

        DispatcherJob* job = m_curr_jobs[dpy];
        if (NULL == job)
        {
            HWC_LOGW("(%" PRIu64 ") SET/bypass/nulljob", dpy);
            display->clearAllFences();
            return;
        }

        if (display->getId() >= HWC_DISPLAY_VIRTUAL &&
            display->getClientTarget() != nullptr)
        {
            // SurfaceFlinger might not setClientTarget while VDS disconnect.
            int32_t gles_head = -1, gles_tail = -1;
            display->getGlesRange(&gles_head, &gles_tail);
            if (gles_head != -1 && display->getClientTarget()->getHandle() == nullptr)
            {
                display->clearAllFences();
                HWC_LOGW("(%" PRIu64 ") SET/bypass/no_client_target", display->getId());
                return;
            }
        }

        if (display->getId() >= HWC_DISPLAY_VIRTUAL &&
            (display->getOutbuf() == nullptr ||
             display->getOutbuf()->getHandle() == nullptr))
        {
            // SurfaceFlinger might not set output buffer while VDS disconnect.
            display->clearAllFences();
            HWC_LOGW("(%" PRIu64 ") SET/bypass/no_outbuf", display->getId());
            return;
        }

        auto&& layers = display->getVisibleLayersSortedByZ();
        if (layers.size() == 0 && display->getId() < HWC_DISPLAY_VIRTUAL)
        {
            HWC_LOGI("(%" PRIu64 ") SET/bypass/no_visible_layers %d", display->getId(), __LINE__);
        }
        else if (layers.size() == 0 && display->getId() == HWC_DISPLAY_VIRTUAL)
        {
            // bliter cannot handle buffer < 32x32, bypass current frame for VDS
            if (display->getOutbuf()->getPrivateHandle().width < 32 ||
                display->getOutbuf()->getPrivateHandle().height < 32)
            {
                display->clearAllFences();
                HWC_LOGW("(%" PRIu64 ") SET/bypass/no_visible_layers %d", display->getId(), __LINE__);
                return;
            }
            // refresh black buffer to virtual display
            job->is_black_job = true;
        }

        // todo: close acquired fences of cache layers
        //for (uint32_t i = 0; i < list->numHwLayers; i++)
        //{
        //    if (job->hit_info.hit_layer_head == -1 &&
        //        job->hit_info.hit_layer_tail == -1 &&
        //        !job->hit_info.is_hit)
        //    {
        //        break;
        //    }

        //    if (i >= job->hit_info.hit_layer_head && i <= job->hit_info.hit_layer_tail)
        //    {
        //        hwc_layer_1_t* layer = &list->hwLayers[i];
                //HWC_LOGD("[FBT]Set layer list Cache/rel=%d/acq=%d/handle=%pd",
                //        layer->releaseFenceFd, layer->acquireFenceFd,
                //        layer->handle);
        //        layer->releaseFenceFd = -1;
        //        if (layer->acquireFenceFd != -1) ::protectedClose(layer->acquireFenceFd);
        //    }
        //}

        // TODO: remove this if MHL display can do partial update by DPI
        if ((HWC_DISPLAY_EXTERNAL == job->disp_ori_id) &&
            (HWC_MIRROR_SOURCE_INVALID == job->disp_mir_id)) // extension mode
        {
            DispatcherJob* mir_job = m_curr_jobs[HWC_DISPLAY_PRIMARY];
            if (NULL != mir_job)
            {
                // record main display orientation in disp_mir_rot for MHL extension mode
                job->disp_mir_rot = mir_job->disp_ori_rot;
                if (job->disp_ori_rot != mir_job->disp_ori_rot)
                {
                    HWC_LOGD("change MHL ori %d->%d", job->disp_ori_rot, mir_job->disp_ori_rot);
                }
            }
        }

        if (job->need_flush)
        {
            m_workers[dpy].dp_thread->wait();
            m_workers[dpy].ovl_engine->wait();
        }

        // check if fbt handle is valid
        if (job->fbt_exist)
        {
            sp<HWCLayer>&& fbt_layer = display->getClientTarget();
            if (fbt_layer->getHandle() == NULL)
            {
                int idx = job->num_ui_layers + job->num_mm_layers;
                HWLayer* fbt_hw_layer = &job->hw_layers[idx];
                fbt_hw_layer->enable = false;
#ifdef MTK_HWC_PROFILING
                fbt_hw_layer->fbt_input_layers = 0;
                fbt_hw_layer->fbt_input_bytes  = 0;
#endif

                job->fbt_exist = false;

                if (0 == (job->num_mm_layers + job->num_ui_layers))
                {
                    display->clearAllFences();
                    job->enable = false;
                    HWC_LOGW("(%" PRIu64 ") SET/bypass/no layers", dpy);
                    return;
                }
            }
        }

#ifndef MTK_USER_BUILD
        const int32_t atrace_dpy = dpy;
        HWC_ATRACE_JOB("set", atrace_dpy, job->fbt_exist, job->num_ui_layers, job->num_mm_layers);
#endif

        DbgLogger* logger = Debugger::getInstance().m_logger->set_info[dpy];

        logger->printf("(%" PRIu64 ") SET job=%d/max=%d/fbt=%d(%s)/ui=%d/mm=%d/del=%zu/mir=%d/ult=%d/flush=%d/black=%d",
            dpy, job->sequence, job->num_layers, job->fbt_exist, job->mm_fbt? "MM" : "OVL",
            job->num_ui_layers, job->num_mm_layers, display->getInvisibleLayersSortedByZ().size(), job->disp_mir_id,
            HWCDispatcher::getInstance().m_ultra_scenario, job->need_flush, job->is_black_job);

        auto fbt_layer = display->getClientTarget();
        if (!job->fbt_exist && fbt_layer->getAcquireFenceFd() != -1) {
            HWC_LOGI("%" PRIu64 ") Job does not have fbt, but fbt has set data. Clear them", dpy);
            const int32_t acquire_fence_fd = fbt_layer->getAcquireFenceFd();
            if (acquire_fence_fd != -1) {
                ::protectedClose(acquire_fence_fd);
                fbt_layer->setAcquireFenceFd(-1, display->isConnected());
            }
        }

        // verify
        // 1. if outbuf should be used for virtual display
        // 2. if any input buffer is dirty for physical display
        m_workers[dpy].post_handler->set(display, job);

        logger->tryFlush();

        if ((job->post_state & HWC_POST_CONTINUE_MASK) == 0)
        {
            if (job->num_mm_layers)
            {
                if (HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id)
                {
                    HWC_LOGD("(%d) disable mirror because mirror dst not post", job->disp_ori_id);
                    disableMirror(display, job);
                }
            }
            // disable job
            job->enable = false;
            return;
        }

        bool job_enabled = false;

        if (job->num_ui_layers || (job->fbt_exist && !job->mm_fbt) ||
            (job->num_ui_layers == 0 && job->num_mm_layers == 0 && job->fbt_exist == 0))
        {
            job_enabled = true;
        }

        bool need_set_mirror = false;
        if (job->num_mm_layers || job->mm_fbt)
        {
            int mir_dpy = job->disp_mir_id;
            if (HWC_MIRROR_SOURCE_INVALID == mir_dpy)
            {
                job_enabled = true;
            }
            else
            {
                // mirror mode
                {
                    job_enabled = true;
                    need_set_mirror = true;
                }
            }
        }

        if (job->is_black_job)
        {
            job_enabled = true;
        }

        if (job_enabled && (display->getMirrorSrc() == -1))
        {
            m_workers[dpy].composer->set(display, job);
        }

        dupMMBufferHandle(job);

        // marked job that should be processed
        job->enable = job_enabled;

        if (false == job->enable &&
            HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id &&
            HWC_DISPLAY_VIRTUAL == job->disp_ori_id)
        {
            HWC_LOGD("(%d) disable mirror because mirror dst job not enable", job->disp_ori_id);
            disableMirror(display, job);
        }

        if (display->getIsHBM()) {
            job->is_HBM = true;
        }
    }
}

void HWCDispatcher::trigger(const hwc2_display_t& dpy)
{
    AutoMutex l(m_workers[dpy].plug_lock_main);

    if (m_workers[dpy].enable && m_curr_jobs[dpy])
    {
        if (m_workers[dpy].dp_thread->getQueueSize() > 5)
        {
            m_workers[dpy].force_wait = true;
            HWC_LOGW("(%d) Jobs have piled up, wait for clearing!!", dpy);
        }
        else
        {
            m_workers[dpy].force_wait = m_curr_jobs[dpy]->force_wait;
        }

        if (DisplayManager::getInstance().getDisplayData(dpy)->trigger_by_vsync)
        {
            m_workers[dpy].dp_thread->trigger(m_curr_jobs[dpy], true, true);
        }
        else
        {
            m_workers[dpy].dp_thread->trigger(m_curr_jobs[dpy], false, false);
        }

        m_curr_jobs[dpy] = NULL;
    }

    // [WORKAROUND]
    if (m_workers[dpy].force_wait)
    {
        m_workers[dpy].dp_thread->wait();
        m_workers[dpy].force_wait = false;
    }
}

void HWCDispatcher::releaseResourceLocked(int dpy)
{
    // wait until all threads are idle
    if (m_workers[dpy].dp_thread != NULL)
    {
        // flush pending display job of display before
        // destroy display session
        m_workers[dpy].dp_thread->wait();
        m_workers[dpy].dp_thread->requestExit();
        m_workers[dpy].dp_thread->trigger(m_curr_jobs[dpy]);
        m_curr_jobs[dpy] = NULL;
        m_workers[dpy].dp_thread->join();

        if (dpy < HWC_DISPLAY_VIRTUAL)
        {
            removeVSyncListener(dpy, m_workers[dpy].dp_thread);
        }
        else
        {
            removeVSyncListener(HWC_DISPLAY_PRIMARY, m_workers[dpy].dp_thread);
        }
        m_workers[dpy].dp_thread = NULL;
    }

    m_workers[dpy].composer = NULL;

    m_workers[dpy].post_handler = NULL;
    if (dpy < HWC_DISPLAY_VIRTUAL)
        removeVSyncListener(dpy, m_workers[dpy].ovl_engine);

    m_workers[dpy].ovl_engine->removePostProcessingEngine();
    m_workers[dpy].ovl_engine->requestExit();
    m_workers[dpy].ovl_engine->stop();
    m_workers[dpy].ovl_engine->trigger(true, 0, DISP_NO_PRESENT_FENCE, -1, false, nullptr, true);
    m_workers[dpy].ovl_engine->join();
    m_workers[dpy].ovl_engine->setPowerMode(HWC2_POWER_MODE_OFF);
    m_workers[dpy].ovl_engine = NULL;

    HWCMediator::getInstance().getOvlDevice(dpy)->waitAllJobDone(dpy);

    HWC_LOGD("Release resource (dpy=%d)", dpy);

    SessionInfo info;
    if (HWCMediator::getInstance().getOvlDevice(dpy)->getOverlaySessionInfo(dpy, &info) != INVALID_OPERATION)
    {
        // session still exists after destroying overlay engine
        // something goes wrong in display driver?
        HWC_LOGW("Session is not destroyed (dpy=%d)", dpy);
    }
}

void HWCDispatcher::onPlugIn(int dpy)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif

    if (dpy >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("Invalid display(%d) is plugged(%d) !!", dpy);
        return;
    }

    {
        AutoMutex ll(m_workers[dpy].plug_lock_loop);
        AutoMutex lm(m_workers[dpy].plug_lock_main);
        AutoMutex lv(m_workers[dpy].plug_lock_vsync);

        if (m_alloc_disp_ids.hasBit(dpy))
        {
            HWC_LOGE("Display(%d) is already connected !!", dpy);
            return;
        }

        m_workers[dpy].enable = false;

        m_workers[dpy].ovl_engine = new OverlayEngine(dpy);

        struct sched_param param = {0};
        param.sched_priority = 1;
        if (sched_setscheduler(m_workers[dpy].ovl_engine->getTid(), SCHED_FIFO, &param) != 0) {
            ALOGE("Couldn't set SCHED_FIFO for ovl_engine");
        }

        if (m_workers[dpy].ovl_engine == NULL ||
            !m_workers[dpy].ovl_engine->isEnable())
        {
            m_workers[dpy].ovl_engine = NULL;
            HWC_LOGE("Failed to create OverlayEngine (dpy=%d) !!", dpy);
            return;
        }

        if (dpy < HWC_DISPLAY_VIRTUAL)
        {
            const DisplayData* display_data = DisplayManager::getInstance().getDisplayData(dpy);
            if (display_data->subtype == HWC_DISPLAY_EPAPER)
            {
                // sp<PostProcessingEngine> ppe = new EpaperPostProcessingEngine(dpy, m_workers[dpy].ovl_engine);
                // m_workers[dpy].ovl_engine->setPostProcessingEngine(ppe);
                // m_workers[dpy].post_handler =
                //    new PostProcessingHandler(this, dpy, m_workers[dpy].ovl_engine);
            }
            else
            {
                m_workers[dpy].post_handler =
                    new PhyPostHandler(this, dpy, m_workers[dpy].ovl_engine);
            }
            registerVSyncListener(dpy, m_workers[dpy].ovl_engine);
        }
        else
        {
            m_workers[dpy].post_handler =
                new VirPostHandler(this, dpy, m_workers[dpy].ovl_engine);
        }

        if (m_workers[dpy].post_handler == NULL)
        {
            HWC_LOGE("Failed to create PostHandler (dpy=%d) !!", dpy);
            releaseResourceLocked(dpy);
            return;
        }

        // create post listener to know when need to set and trigger overlay engine
        struct PostListener : public SyncControl::SyncListener
        {
            PostListener(sp<PostHandler> handler) : m_handler(handler) { }
            ~PostListener() { m_handler = NULL; }
        private:
            sp<PostHandler> m_handler;
            mutable Mutex m_lock;
            virtual void onTrigger(DispatcherJob* job)
            {
                AutoMutex l(m_lock);
                m_handler->process(job);
            }
        };

        m_workers[dpy].dp_thread = new DispatchThread(dpy);
        param.sched_priority = 1;
        if (sched_setscheduler(m_workers[dpy].dp_thread->getTid(), SCHED_FIFO, &param) != 0) {
            ALOGE("Couldn't set SCHED_FIFO for dp_thread");
        }
        if (m_workers[dpy].dp_thread == NULL)
        {
            HWC_LOGE("Failed to create DispatchThread (dpy=%d) !!", dpy);
            releaseResourceLocked(dpy);
            return;
        }

        if (!DisplayManager::getInstance().getDisplayData(dpy)->trigger_by_vsync)
        {
            if (DisplayManager::getInstance().getDisplayData(dpy)->subtype == HWC_DISPLAY_EPAPER)
            {
                m_workers[dpy].ovl_engine->registerVSyncListener(m_workers[dpy].dp_thread);
            }
            else
            {
                registerVSyncListener(HWC_DISPLAY_PRIMARY, m_workers[dpy].dp_thread);
            }
        }
        else
        {
            registerVSyncListener(dpy, m_workers[dpy].dp_thread);
        }

        m_workers[dpy].composer = new LayerComposer(dpy, m_workers[dpy].ovl_engine);
        if (m_workers[dpy].composer == NULL)
        {
            HWC_LOGE("Failed to create LayerComposer (dpy=%d) !!", dpy);
            releaseResourceLocked(dpy);
            return;
        }

        m_alloc_disp_ids.markBit(dpy);
        m_workers[dpy].enable = true;

        // create mirror buffer of main display if needed
        // TODO: get buffer from gui ext module
        if (HWC_DISPLAY_PRIMARY < dpy &&
            Platform::getInstance().m_config.mirror_state != MIRROR_DISABLED)
        {
            const DisplayData* disp_data = DisplayManager::getInstance().getDisplayData(dpy);
            int format = (HWC_DISPLAY_VIRTUAL == dpy && HWC_DISPLAY_WIRELESS != disp_data->subtype) ?
                HAL_PIXEL_FORMAT_RGB_888 : HAL_PIXEL_FORMAT_YUYV;
            {
                // force output RGB format for debuging
                char value[PROPERTY_VALUE_MAX];
                property_get("vendor.debug.hwc.force_rgb_output", value, "0");
                if (0 != atoi(value))
                {
                    HWC_LOGW("[DEBUG] force RGB format!!");
                    format = HAL_PIXEL_FORMAT_RGB_888;
                }
            }
            HWC_LOGD("set output format 0x%x !!", format);
            m_workers[HWC_DISPLAY_PRIMARY].ovl_engine->createOutputQueue(format, false);
        }
    }
}

void HWCDispatcher::onPlugOut(int dpy)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif

    if (dpy >= DisplayManager::MAX_DISPLAYS)
    {
        HWC_LOGE("Invalid display(%d) is unplugged !!", dpy);
        return;
    }

    if (dpy == HWC_DISPLAY_PRIMARY)
    {
        HWC_LOGE("Should not disconnect primary display !!");
        return;
    }

    {
        AutoMutex lm(m_workers[dpy].plug_lock_main);
        AutoMutex lv(m_workers[dpy].plug_lock_vsync);

        // flush pending display job of mirror source before
        // destroy display session
        {
            DispatcherJob* ori_job = m_curr_jobs[dpy];

            if ( ori_job && (HWC_MIRROR_SOURCE_INVALID != ori_job->disp_mir_id))
            {
                const int mir_dpy = ori_job->disp_mir_id;

                AutoMutex l(m_workers[mir_dpy].plug_lock_main);

                if (m_workers[mir_dpy].enable)
                {
                    DispatcherJob* mir_job = m_curr_jobs[mir_dpy];

                    if (mir_job && mir_job->enable && mir_job->mirrored)
                    {
                        m_workers[mir_dpy].dp_thread->trigger(mir_job);
                        m_curr_jobs[mir_dpy] = NULL;
                    }

                    m_workers[mir_dpy].dp_thread->wait();
                    m_workers[mir_dpy].ovl_engine->wait();
                    HWCMediator::getInstance().getOvlDevice(mir_dpy)->waitAllJobDone(mir_dpy);
                }
            }
        }

        if (m_workers[dpy].enable)
        {
            releaseResourceLocked(dpy);
        }
        else
        {
            HWC_LOGE("Failed to disconnect invalid display(%d) !!", dpy);
        }

        AutoMutex ll(m_workers[dpy].plug_lock_loop);

        m_alloc_disp_ids.clearBit(dpy);
        m_workers[dpy].enable = false;

        // release mirror buffer of main display if needed
        // TODO: release buffer to gui ext module
        if (HWC_DISPLAY_PRIMARY < dpy && (m_alloc_disp_ids.count() == 1))
        {
            m_workers[HWC_DISPLAY_PRIMARY].ovl_engine->releaseOutputQueue();
        }

        if (dpy == HWC_DISPLAY_EXTERNAL)
        {
            DisplayManager::getInstance().hotplugExtOut();
        }
    }
}

void HWCDispatcher::setPowerMode(int dpy, int mode)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif

    if (HWC2_POWER_MODE_OFF == mode || HWC2_POWER_MODE_DOZE_SUSPEND == mode)
    {
        setSessionMode(HWC_DISPLAY_PRIMARY, false);
    }

    if (HWC_DISPLAY_VIRTUAL > dpy)
    {
        AutoMutex l(m_workers[dpy].plug_lock_main);

        if (HWC2_POWER_MODE_OFF == mode || HWC2_POWER_MODE_DOZE_SUSPEND == mode)
        {
            if (m_workers[dpy].dp_thread != NULL)
            {
                m_workers[dpy].dp_thread->wait();
            }
            if (m_workers[dpy].ovl_engine != NULL)
            {
                m_workers[dpy].ovl_engine->wait();
            }
            HWCMediator::getInstance().getOvlDevice(dpy)->waitAllJobDone(dpy);
        }

        if (m_workers[dpy].enable)
        {
            m_workers[dpy].ovl_engine->setPowerMode(mode);
        }

        if (HWC2_POWER_MODE_OFF == mode || HWC2_POWER_MODE_DOZE_SUSPEND == mode)
        {
            if (m_workers[dpy].composer != NULL) m_workers[dpy].composer->nullop();
        }
    }
}

void HWCDispatcher::onVSync(int dpy)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif

    {
        AutoMutex l(m_vsync_lock);

        // dispatch vsync signal to listeners
        const size_t count = m_vsync_callbacks[dpy].size();
        for (size_t i = 0; i < count; i++)
        {
            const sp<VSyncListener>& callback(m_vsync_callbacks[dpy][i]);
            callback->onVSync();
        }
    }
}

void HWCDispatcher::registerVSyncListener(int dpy, const sp<VSyncListener>& listener)
{
    AutoMutex l(m_vsync_lock);

    m_vsync_callbacks.editItemAt(dpy).add(listener);
    HWC_LOGD("(%d) register VSyncListener", dpy);
}

void HWCDispatcher::removeVSyncListener(int dpy, const sp<VSyncListener>& listener)
{
    AutoMutex l(m_vsync_lock);

    m_vsync_callbacks.editItemAt(dpy).remove(listener);
    HWC_LOGD("(%d) remove VSyncListener", dpy);
}

void HWCDispatcher::dump(String8 *dump_str)
{
    for (int dpy = 0; dpy < DisplayManager::MAX_DISPLAYS; dpy++)
    {
        AutoMutex l(m_workers[dpy].plug_lock_main);

        if (m_workers[dpy].enable)
        {
            m_workers[dpy].ovl_engine->dump(dump_str);
        }
    }
}

bool HWCDispatcher::saveFbtHandle(int dpy, buffer_handle_t handle)
{
    bool res = m_prev_fbt[dpy] != handle;
    m_prev_fbt[dpy] = handle;
    return res;
}

void HWCDispatcher::ignoreJob(int dpy, bool ignore)
{
    AutoMutex l(m_workers[dpy].plug_lock_loop);
    m_workers[dpy].ignore_job = ignore;
}

int HWCDispatcher::getOvlEnginePowerModeChanged(const int32_t& dpy) const
{
    return m_workers[dpy].ovl_engine->getPowerModeChanged();
}

void HWCDispatcher::decOvlEnginePowerModeChanged(const int32_t& dpy) const
{
    return m_workers[dpy].ovl_engine->decPowerModeChanged();
}

void HWCDispatcher::dupMMBufferHandle(DispatcherJob* job)
{
    uint32_t total_num = job->num_layers;
    for (uint32_t i = 0; i < total_num; i++)
    {
        HWLayer* hw_layer = &job->hw_layers[i];

        // skip non mm layers
        if (HWC_LAYER_TYPE_MM != hw_layer->type && HWC_LAYER_TYPE_MM_FBT != hw_layer->type) continue;

        // this layer is not enable
        if (!hw_layer->enable) continue;

        if (hw_layer->dirty)
        {
            buffer_handle_t out_hnd;
            dupBufferHandle(hw_layer->layer.handle, &out_hnd);
            hw_layer->layer.handle = out_hnd;
            hw_layer->priv_handle.handle = out_hnd;
            addBufToBufRecorder(hw_layer->layer.handle);
            HWC_LOGD("dup (%d) hw_layer->layer.handle: %p", hw_layer->index, hw_layer->layer.handle);
        }
    }
}

void HWCDispatcher::freeMMDuppedBufferHandle(DispatcherJob* job)
{
    uint32_t total_num = job->num_layers;
    for (uint32_t i = 0; i < total_num; i++)
    {
        HWLayer* hw_layer = &job->hw_layers[i];

        // skip non mm layers
        if (HWC_LAYER_TYPE_MM != hw_layer->type && HWC_LAYER_TYPE_MM_FBT != hw_layer->type) continue;

        // this layer is not enable
        if (!hw_layer->enable) continue;

        if (hw_layer->dirty)
        {
            removeBufFromBufRecorder(hw_layer->layer.handle);
            freeDuppedBufferHandle(hw_layer->layer.handle);
        }
    }
}

void HWCDispatcher::prepareMirror(const std::vector<sp<HWCDisplay> >& displays)
{
    // In the function prepareMirror(), all of the flows and functions
    // like configMirrorJob(), configMirrorOutput() based on the result
    // of checkMirrorPath to decide do things or not

    for (auto& hwc_display : displays)
    {
        if (!hwc_display->isValid())
        continue;

        DispatcherJob* job = m_curr_jobs[hwc_display->getId()];
        if (NULL != job)
        {
            if (hwc_display->getMirrorSrc() != -1)
            {
                job->disp_mir_id = hwc_display->getMirrorSrc();
                //job->disp_ori_rot   = (getMtkFlags() & HWC_ORIENTATION_MASK) >> 16;
                job->mir_disp_data = m_curr_jobs[job->disp_mir_id]->disp_data;
                configMirrorJob(job);
            }
            else
            {
                job->disp_mir_id    = HWC_MIRROR_SOURCE_INVALID;
                //job->disp_ori_rot   = (getMtkFlags() & HWC_ORIENTATION_MASK) >> 16;
                job->mir_disp_data  = nullptr;
            }
        }
    }

    for (auto& hwc_display : displays)
    {
        if (!hwc_display->isValid())
            continue;

        if (HWC_DISPLAY_VIRTUAL == hwc_display->getId())
            continue;

        DispatcherJob* job = m_curr_jobs[hwc_display->getId()];
        if (NULL != job)
        {
            configMirrorOutput(job, DisplayManager::getInstance().getSupportedColorMode(hwc_display->getId()));
        }
    }

    for (auto& hwc_display : displays)
    {
        if (!hwc_display->isValid())
            continue;

        if (hwc_display->getId() <= HWC_DISPLAY_PRIMARY)
            continue;

        DispatcherJob* job = m_curr_jobs[hwc_display->getId()];
        if (NULL != job && (HWC_MIRROR_SOURCE_INVALID != hwc_display->getMirrorSrc()))
        {
            DispatcherJob* mir_src_job = m_curr_jobs[job->disp_mir_id];
            if (NULL != mir_src_job && mir_src_job->need_output_buffer)
            {
                m_workers[hwc_display->getId()].composer->set(hwc_display, job);
                m_workers[job->disp_mir_id].post_handler->setMirror(mir_src_job, job);
            }
            else
            {
                HWC_LOGD("%" PRIu64 " disable mirror because mirror src not avaliable", hwc_display->getId());
                hwc_display->setMirrorSrc(HWC_MIRROR_SOURCE_INVALID);
                job->disp_mir_id = HWC_MIRROR_SOURCE_INVALID;
                if (NULL != mir_src_job)
                {
                    mir_src_job->mirrored = false;
                }
            }
        }
    }
}
// ---------------------------------------------------------------------------

DispatchThread::DispatchThread(int dpy)
    : m_disp_id(dpy)
    , m_vsync_signaled(false)
    , m_continue_skip(0)
    , m_first_trigger(true)
{
    snprintf(m_thread_name, sizeof(m_thread_name), "Dispatcher_%d", dpy);
}

void DispatchThread::onFirstRef()
{
    run(m_thread_name, PRIORITY_URGENT_DISPLAY);
}

void DispatchThread::trigger(DispatcherJob* job, bool async, bool skip)
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_NAME("dispatcher_set");
#endif

    bool need_drop = false;
    if (async)
    {
        need_drop = markDroppableJob();
    }

    AutoMutex l(m_lock);

    if (job != NULL)
    {
        m_job_queue.push_back(job);
    }

    HWCDispatcher::WorkerCluster& worker(
            HWCDispatcher::getInstance().m_workers[m_disp_id]);
    // when ignore_job is set by DisplayManager when disconnect this display, so we do not care
    // these jobs. They will be dropped in thread loop, and therefore trigger it without HW VSync.
    if (!skip || worker.ignore_job || m_first_trigger)
    {
        m_state = HWC_THREAD_TRIGGER;
        sem_post(&m_event);

        if (m_first_trigger)
        {
            m_first_trigger = false;
        }
    }
    else
    {
        if (need_drop)
        {
            // thread may wait for vsync, so wake it up to swap job
            m_vsync_cond.signal();

            // thread does not trigger, so trigger it to drop job
            sem_post(&m_event);
        }

        ++m_continue_skip;

        // if Dispatcher Thread skips 10 times continuous and does not receive VSync,
        // the VSync source may be corrupt.
        if (m_continue_skip > 10)
        {
            HWC_LOGE("(%d) VSync source may be corrupt", m_disp_id);
        }
    }
}

bool DispatchThread::markDroppableJob()
{
    AutoMutex l(m_lock);

    if (!m_job_queue.size())
    {
        return false;
    }

    DispatcherJob* last = m_job_queue.editItemAt(m_job_queue.size() - 1);
    if (last == NULL || (last->fbt_exist && !last->mm_fbt))
    {
        return false;
    }

    last->dropped = true;
    return true;
}

bool DispatchThread::dropJob()
{
    bool res = false;
    {
        AutoMutex l(m_lock);
        res |= m_job_queue[0]->dropped;
    }

    HWCDispatcher::WorkerCluster& worker(
                HWCDispatcher::getInstance().m_workers[m_disp_id]);
    {
        AutoMutex l(worker.plug_lock_loop);
        res |= worker.ignore_job;
    }

    if (res)
    {
        DispatcherJob* job = NULL;
        {
            AutoMutex l(m_lock);
            Fifo::iterator front(m_job_queue.begin());
            job = *front;
            m_job_queue.erase(front);
        }
        HWC_LOGD("(%d) Drop a job %d", m_disp_id, job->sequence);

        if (job->enable)
        {
            AutoMutex l(worker.plug_lock_loop);
            if (job->num_mm_layers || job->mm_fbt || job->num_ui_layers)
            {
                if (worker.composer != NULL)
                {
                    worker.composer->cancelLayers(job);
                }
                else
                {
                    HWC_LOGE("No LayerComposer");
                }
            }
        }
        clearUsedJob(job);
    }

    return res;
}

int DispatchThread::getQueueSize()
{
    AutoMutex l(m_lock);
    return m_job_queue.size();
}

bool DispatchThread::threadLoop()
{
    sem_wait(&m_event);

    bool need_check_next_period = false;

    while (1)
    {
        DispatcherJob* job = NULL;

        {
            AutoMutex l(m_lock);
            if (m_job_queue.empty())
            {
                HWC_LOGV("(%d) Job is empty...", m_disp_id);
                break;
            }
        }

#ifndef MTK_USER_BUILD
        HWC_ATRACE_NAME("dispatcher_loop");
#endif

        if (dropJob())
        {
            continue;
        }

        // If this display is not triggered by VSync, it will not drop any job.
        // Furthermore, it does not enable VSync, so it always wait VSync in here.
        // This behavior increases the response time.
        // Therefore, we should not wait VSync if the dispaly is not triggered by VSync.
        //
        // TODO:
        // if ovl is decoupled, need to make sure if ovl could have internal queue
        // if yes, there is no need to wait for next vsync for handling next composition
        if (DisplayManager::getInstance().getDisplayData(m_disp_id)->trigger_by_vsync)
        {
            AutoMutex l(m_vsync_lock);

            if (m_disp_id < HWC_DISPLAY_VIRTUAL && (need_check_next_period || !m_vsync_signaled))
            {
                HWC_LOGD("(%d) Wait to handle next job...", m_disp_id);

#ifndef MTK_USER_BUILD
                HWC_ATRACE_NAME("dispatcher_wait");
#endif

                waitNextVSyncLocked(m_disp_id);
            }
        }

        if (dropJob())
        {
            continue;
        }
        {
            AutoMutex l(m_lock);
            Fifo::iterator front(m_job_queue.begin());
            job = *front;
            m_job_queue.erase(front);
        }
        m_vsync_signaled = false;

        bool need_sync = true;

        // handle jobs
        // 1. set synchronization between composer threads
        // 2. trigger ui/mm threads to compose layers
        // 3. wait until the composition of ui/mm threads is done
        // 4. clear used job
        {
            HWCDispatcher::WorkerCluster& worker(
                HWCDispatcher::getInstance().m_workers[m_disp_id]);

            AutoMutex l(worker.plug_lock_loop);

            HWC_LOGD("(%d) Handle job %d /queue_size=%d", m_disp_id, job->sequence, getQueueSize());

            sp<OverlayEngine> ovl_engine = worker.ovl_engine;
            if (job->enable)
            {
                {
                    if (ovl_engine != NULL) ovl_engine->setHandlingJob(job);

#ifndef MTK_USER_BUILD
                    HWC_ATRACE_JOB("trigger",
                        m_disp_id, job->fbt_exist, job->num_ui_layers, job->num_mm_layers);
#endif

                    sp<LayerComposer> composer = worker.composer;

                    if (composer != NULL)
                    {
                        if (job->num_ui_layers || (job->fbt_exist && !job->mm_fbt) ||
                            job->num_mm_layers || job->mm_fbt)
                        {
                            composer->trigger(job);
                            worker.post_handler->process(job);
                        }
                        else if (job->is_black_job)
                        {
                            HWC_LOGE("(%d) Handle black job(%d)", m_disp_id, job->is_black_job);
                            composer->trigger(job);
                            worker.post_handler->process(job);
                        }
                        else if (job->num_ui_layers == 0 && job->num_mm_layers == 0 && job->fbt_exist == 0)
                        {
                            HWC_LOGE("(%d) Handle a job with no visible layer", m_disp_id);
                            composer->trigger(job);
                            worker.post_handler->process(job);
                        }
                        else
                        {
                            composer->nullop();
                        }
                    }

                    if (m_disp_id < HWC_DISPLAY_VIRTUAL && (job->num_mm_layers || job->mm_fbt))
                        need_sync = false;
                }
            }
#ifndef MTK_USER_BUILD
            else
            {
                HWC_ATRACE_NAME("dispatcher_bypass");
            }
#endif

            // clear used job
            clearUsedJob(job);
        }

        need_check_next_period = need_sync;
    }

    {
        AutoMutex l(m_lock);
        if (m_job_queue.empty())
        {
            m_state = HWC_THREAD_IDLE;
            m_condition.signal();
        }
    }

    return true;
}

void DispatchThread::waitNextVSyncLocked(int dpy)
{
    // TODO: pass display id to DisplayManager to get the corresponding vsync signal

    // It's a warning that how long HWC does not still receive the VSync
    const nsecs_t VSYNC_TIMEOUT_THRESHOLD_NS = 4000000;

    // request next vsync
    if (DisplayManager::getInstance().getDisplayData(dpy)->trigger_by_vsync)
    {
        DisplayManager::getInstance().requestNextVSync(dpy);
    }
    else
    {
        DisplayManager::getInstance().requestNextVSync(HWC_DISPLAY_PRIMARY);
    }

    // There are various VSync periods for each display.
    // Especially, the vsync rate of MHL is dynamical and can be 30fps or 60fps.
    const uint32_t refresh = DisplayManager::getInstance().getDisplayData(dpy)->trigger_by_vsync ?
        DisplayManager::getInstance().getDisplayData(dpy)->refresh : ms2ns(16) ;
    if (m_vsync_cond.waitRelative(m_vsync_lock, refresh + VSYNC_TIMEOUT_THRESHOLD_NS) == TIMED_OUT)
    {
        HWC_LOGW("(%d) Timed out waiting for vsync...", dpy);
    }
}

void DispatchThread::onVSync()
{
#ifndef MTK_USER_BUILD
    HWC_ATRACE_CALL();
#endif

    AutoMutex l(m_vsync_lock);
    m_vsync_signaled = true;
    m_vsync_cond.signal();

    if (DisplayManager::getInstance().getDisplayData(m_disp_id)->trigger_by_vsync)
    {
        m_continue_skip = 0;

        // check queue is empty to avoid redundant execution of threadloop
        if (!m_job_queue.empty())
        {
            m_state = HWC_THREAD_TRIGGER;
            sem_post(&m_event);
        }
    }
}

void DispatchThread::clearUsedJob(DispatcherJob* job)
{
    if(job == NULL)
        return;

    if(m_disp_id >= HWC_DISPLAY_VIRTUAL)
    {
        freeDuppedBufferHandle(job->hw_outbuf.handle);
    }

    HWCDispatcher::getInstance().freeMMDuppedBufferHandle(job);

    free(job->hw_layers);
    delete job;
}

// ---------------------------------------------------------------------------

#define PLOGD(x, ...) HWC_LOGD("(%d) " x, m_disp_id, ##__VA_ARGS__)
#define PLOGI(x, ...) HWC_LOGI("(%d) " x, m_disp_id, ##__VA_ARGS__)
#define PLOGW(x, ...) HWC_LOGW("(%d) " x, m_disp_id, ##__VA_ARGS__)
#define PLOGE(x, ...) HWC_LOGE("(%d) " x, m_disp_id, ##__VA_ARGS__)

HWCDispatcher::PostHandler::PostHandler(
    HWCDispatcher* dispatcher, int dpy, const sp<OverlayEngine>& ovl_engine)
    : m_dispatcher(dispatcher)
    , m_disp_id(dpy)
    , m_ovl_engine(ovl_engine)
    , m_sync_fence(new SyncFence(dpy))
    , m_curr_present_fence_fd(-1)
{ }

HWCDispatcher::PostHandler::~PostHandler()
{
    m_ovl_engine = NULL;
    m_sync_fence = NULL;
}

void HWCDispatcher::PostHandler::setOverlayInput(DispatcherJob* job)
{
    // disable unused input layer
    for (int i = 0; i < job->num_layers; i++)
    {
        if (!job->hw_layers[i].enable)
            m_ovl_engine->disableInput(i);
    }
    for (int i = job->num_layers; i < m_ovl_engine->getMaxInputNum(); i++)
    {
        m_ovl_engine->disableInput(i);
    }
}

// ---------------------------------------------------------------------------

void HWCDispatcher::PhyPostHandler::set(
    const sp<HWCDisplay>& display, DispatcherJob* job)
{
    job->hw_outbuf.phy_present_fence_fd = -1;
    job->hw_outbuf.phy_present_fence_idx = DISP_NO_PRESENT_FENCE;

    if (HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id)
    {
        job->post_state = HWC_POST_MIRROR;
        return;
    }

    uint32_t total_num = job->num_layers;

    bool is_dirty = (job->post_state & HWC_POST_CONTINUE_MASK) != 0;

    HWC_ATRACE_FORMAT_NAME("BeginTransform");
    if (is_dirty)
    {
        auto&& layers = display->getCommittedLayers();
#ifdef PHYPOSTHANDLER_DEBUG
        HWC_LOGD("Dispatcher::PhyPostHandler::set() commit layers size:%d", layers.size());
#endif
        for (uint32_t i = 0; i < total_num; i++)
        {
            HWLayer* hw_layer = &job->hw_layers[i];

            if (!hw_layer->enable)
            {
                // uipq debug case
                if (HWCMediator::getInstance().m_features.global_pq &&
                    (job->uipq_index >= 0) &&
                    (i == (uint32_t)job->uipq_index) &&
                    (HWC_LAYER_TYPE_UI == hw_layer->type))
                {
                    sp<HWCLayer> layer = layers[i];
                    layer->setReleaseFenceFd(-1, display->isConnected());
                    if (layer->getAcquireFenceFd() != -1)
                        ::protectedClose(layer->getAcquireFenceFd());
                    layer->setAcquireFenceFd(-1, display->isConnected());
                }
                continue;
            }

            if (HWC_LAYER_TYPE_DIM == hw_layer->type) continue;

#ifdef PHYPOSTHANDLER_DEBUG
            HWC_LOGD("Dispatcher::PhyPostHandler::set() %d i:%d hw_layer->index:%d", __LINE__, i, hw_layer->index);
#endif
            sp<HWCLayer> layer = layers[hw_layer->index];
#ifdef PHYPOSTHANDLER_DEBUG
            HWC_LOGD("Dispatcher::PhyPostHandler::set() %d layer->handle:%p", __LINE__, layer->getHandle());
#endif
            PrivateHandle* priv_handle = const_cast<PrivateHandle*>(&layer->getPrivateHandle());

            HWC_ATRACE_FORMAT_NAME("InputLayer(h:%p)", layer->getHandle());
            // check if fbt is dirty
            if (HWC_LAYER_TYPE_FBT == hw_layer->type)
            {
                if (priv_handle == nullptr)
                {
                    hw_layer->enable = false;
                    continue;
                }

                if (layer->isBufferChanged())
                {
                    hw_layer->dirty |= HWC_LAYER_DIRTY_BUFFER;
                }
#ifdef PHYPOSTHANDLER_DEBUG
                HWC_LOGD("Dispatcher::PhyPostHandler::set() %d i:%d FBT LAYER_TYPE ion_fd:%d", __LINE__, i, priv_handle->ion_fd);
#endif
            }

            // check if any layer is dirty
            is_dirty |= (hw_layer->dirty != HWC_LAYER_DIRTY_NONE);

            gralloc_extra_sf_set_status(
                    &priv_handle->ext_info, GRALLOC_EXTRA_MASK_SF_DIRTY, GRALLOC_EXTRA_BIT_UNDIRTY);

            gralloc_extra_perform(
                    layer->getHandle(), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &priv_handle->ext_info);
        }
    }

    if (is_dirty)
    {
        job->post_state = HWC_POST_INPUT_DIRTY;

        {
            // get present fence from display driver
            OverlayPrepareParam prepare_param;
            {
                if (m_disp_id == HWC_DISPLAY_PRIMARY ||
                    m_disp_id == HWC_DISPLAY_EXTERNAL)
                {
                    status_t err = m_ovl_engine->preparePresentFence(prepare_param);
                    if (NO_ERROR != err)
                    {
                        prepare_param.fence_index = 0;
                        prepare_param.fence_fd = -1;
                    }

                    if (prepare_param.fence_fd <= 0)
                    {
                        PLOGE("Failed to get presentFence !!");
                    }
                }
                else
                {
                    prepare_param.fence_index = 0;
                    prepare_param.fence_fd = -1;
                }
            }

            HWBuffer* hw_outbuf = &job->hw_outbuf;
            hw_outbuf->phy_present_fence_fd  = prepare_param.fence_fd;
            hw_outbuf->phy_present_fence_idx = prepare_param.fence_index;
            hw_outbuf->handle                = NULL;

            job->prev_present_fence_fd = m_curr_present_fence_fd;
            m_curr_present_fence_fd = (prepare_param.fence_fd >= 0) ? ::dup(prepare_param.fence_fd) : -1;
            if (m_disp_id == HWC_DISPLAY_PRIMARY &&
                Platform::getInstance().m_config.is_support_mdp_pmqos) {
                HWVSyncEstimator::getInstance().pushPresentFence(
                    m_curr_present_fence_fd >= 0 ? ::dup(m_curr_present_fence_fd): -1);
                HWVSyncEstimator::getInstance().update(job->disp_data->refresh);
            }

            if (Platform::getInstance().m_config.is_support_mdp_pmqos_debug)
            {
                MDPFrameInfoDebugger::getInstance().setJobPresentFenceFd(job->sequence,
                    m_curr_present_fence_fd >= 0 ? ::dup(m_curr_present_fence_fd): -1);
            }

            if (HWCMediator::getInstance().m_features.without_primary_present_fence
                    && m_disp_id == HWC_DISPLAY_PRIMARY)
            {
                display->setRetireFenceFd(-1, display->isConnected());
                ::protectedClose(prepare_param.fence_fd);
            }
            else
            {
                display->setRetireFenceFd(prepare_param.fence_fd, display->isConnected());
            }

            DbgLogger* logger = Debugger::getInstance().m_logger->set_info[job->disp_ori_id];
            logger->printf("/PF(fd=%d, idx=%d, curr_pf_fd=%d,%d)",
                hw_outbuf->phy_present_fence_fd, hw_outbuf->phy_present_fence_idx, prepare_param.fence_fd, m_curr_present_fence_fd);
            HWC_ATRACE_FORMAT_NAME("TurnInto(%d)", prepare_param.fence_index);
        }
    }
    else
    {
        // set as nodirty since could not find any dirty layers
        job->post_state = HWC_POST_INPUT_NOTDIRTY;

        DbgLogger* logger = Debugger::getInstance().m_logger->set_info[job->disp_ori_id];
        logger->printf(" / skip composition: no dirty layers");
        // clear all layers' acquire fences
        display->clearAllFences();
    }
}

void HWCDispatcher::PhyPostHandler::setMirror(
    DispatcherJob* src_job, DispatcherJob* dst_job)
{
    HWBuffer* phy_outbuf = &src_job->hw_outbuf;
    HWBuffer* dst_mirbuf = &dst_job->hw_mirbuf;
    HWBuffer* dst_outbuf = &dst_job->hw_outbuf;

    dst_mirbuf->mir_in_acq_fence_fd = phy_outbuf->mir_out_acq_fence_fd;
    dst_mirbuf->handle              = phy_outbuf->handle;
    memcpy(&dst_mirbuf->priv_handle, &phy_outbuf->priv_handle, sizeof(PrivateHandle));
    dst_mirbuf->dataspace = phy_outbuf->dataspace;

    if (dst_job->disp_ori_id == HWC_DISPLAY_EXTERNAL)
    {
        unsigned int dst_format = phy_outbuf->priv_handle.format;
        switch (Platform::getInstance().m_config.format_mir_mhl)
        {
            case MIR_FORMAT_RGB888:
                dst_format = HAL_PIXEL_FORMAT_RGB_888;
                break;
            case MIR_FORMAT_YUYV:
                dst_format = HAL_PIXEL_FORMAT_YUYV;
                break;
            case MIR_FORMAT_YV12:
                dst_format = HAL_PIXEL_FORMAT_YV12;
                break;
            case MIR_FORMAT_UNDEFINE:
                // use same format as source, so do nothing
                break;
            default:
                HWC_LOGW("Not support mir format(%d), use same format as source(%x)",
                    Platform::getInstance().m_config.format_mir_mhl,
                    phy_outbuf->priv_handle.format);
         }

        dst_outbuf->priv_handle.format = dst_format;
        dst_outbuf->priv_handle.usage = phy_outbuf->priv_handle.usage;
    }

    // in decouple mode, need to wait for
    // both display and MDP finish reading this buffer
    {
        char name[32];
        snprintf(name, sizeof(name), "merged_fence(%d/%d)\n",
            phy_outbuf->mir_out_if_fence_fd, dst_mirbuf->mir_in_rel_fence_fd);

        // There are two components need MDP fence in the mirror path.
        // Memory session has duplicated it in set function of bliter, then return it to SF.
        // External session does not duplicated it, so we duplicated it in here.
        int tmp_fd = (dst_job->disp_ori_id == HWC_DISPLAY_EXTERNAL) ?
                ::dup(dst_mirbuf->mir_in_rel_fence_fd) :
                dst_mirbuf->mir_in_rel_fence_fd;

        int merged_fd = SyncFence::merge(
            phy_outbuf->mir_out_if_fence_fd,
            tmp_fd,
            name);

        ::protectedClose(phy_outbuf->mir_out_if_fence_fd);
        ::protectedClose(tmp_fd);

        // TODO: merge fences from different virtual displays to phy_outbuf->mir_out_mer_fence_fd
        phy_outbuf->mir_out_if_fence_fd = merged_fd;
    }

    PLOGD("set mirror (rel_fd=%d(%u)/handle=%p/ion=%d/dataspace=%d -> acq_fd=%d/handle=%p/ion=%d/dataspace=%d)",
        dst_mirbuf->mir_in_rel_fence_fd, dst_mirbuf->mir_in_sync_marker,
        dst_mirbuf->handle, dst_mirbuf->priv_handle.ion_fd, dst_mirbuf->dataspace,
        dst_outbuf->mir_in_rel_fence_fd, dst_outbuf->handle, dst_outbuf->priv_handle.ion_fd,
        dst_outbuf->dataspace);
}

void HWCDispatcher::PhyPostHandler::process(DispatcherJob* job)
{
    setOverlayInput(job);

    // set mirror output buffer if job is a mirror source
    if (job->need_output_buffer)
    {
        HWBuffer* hw_outbuf = &job->hw_outbuf;
        PrivateHandle* priv_handle = &hw_outbuf->priv_handle;

        OverlayPortParam param;

        bool is_secure = isSecure(priv_handle);
        if (is_secure)
        {
            param.va           = (void*)(uintptr_t)hw_outbuf->mir_out_sec_handle;
            param.mva          = (void*)(uintptr_t)hw_outbuf->mir_out_sec_handle;
        }
        else
        {
            param.va           = NULL;
            param.mva          = NULL;
        }

        param.pitch            = priv_handle->y_stride;
        param.format           = priv_handle->format;
        param.dst_crop         = Rect(priv_handle->width, priv_handle->height);
        param.fence_index      = hw_outbuf->mir_out_acq_fence_idx;
        param.if_fence_index   = hw_outbuf->mir_out_if_fence_idx;
        param.secure           = is_secure;
        param.sequence         = job->sequence;
        param.ion_fd           = priv_handle->ion_fd;
        param.mir_rel_fence_fd = hw_outbuf->mir_out_if_fence_fd;
        param.fence            = hw_outbuf->mir_out_rel_fence_fd;
        param.dataspace        = hw_outbuf->dataspace;

        hw_outbuf->mir_out_rel_fence_fd = -1;

        m_ovl_engine->setOutput(&param, job->need_output_buffer);

        if (DisplayManager::m_profile_level & PROFILE_TRIG)
        {
            char atrace_tag[256];
            sprintf(atrace_tag, "OVL0-MDP");
            HWC_ATRACE_ASYNC_BEGIN(atrace_tag, job->sequence);
        }
    }
    else if (!job->need_output_buffer)
    {
        // disable overlay output
        m_ovl_engine->disableOutput();
    }

    // trigger overlay engine
    m_ovl_engine->trigger(job->ovl_valid,
                          job->num_layers,
                          job->hw_outbuf.phy_present_fence_idx,
                          job->prev_present_fence_fd,
                          job->need_av_grouping,
                          job->color_transform);

    job->prev_present_fence_fd = -1;
}

// ---------------------------------------------------------------------------

void HWCDispatcher::VirPostHandler::setError(DispatcherJob* job)
{
    for (int i = 0; i < job->num_layers; i++)
    {
        m_ovl_engine->disableInput(i);
    }
}

void HWCDispatcher::VirPostHandler::set(
    const sp<HWCDisplay>& display, DispatcherJob* job)
{
    if (display->getMtkFlags() & HWC_SKIP_DISPLAY)
    {
        PLOGD("skip composition: display has skip flag");
        job->post_state = HWC_POST_INPUT_NOTDIRTY;
        display->clearAllFences();
        return;
    }

    // For WFD extension mode without OVL is available, let GPU to queue buffer to encoder directly.
    if ((!HWCMediator::getInstance().m_features.copyvds) &&
        ((job->num_ui_layers + job->num_mm_layers) == 0) && !job->is_black_job)
    {
        PLOGD("No need to handle outbuf with GLES mode");
        job->post_state = HWC_POST_OUTBUF_DISABLE;
        display->clearAllFences();
        setError(job);
        return;
    }

    if (display->getOutbuf() == nullptr || display->getOutbuf()->getHandle() == nullptr)
    {
        PLOGE("Fail to get outbuf");
        job->post_state = HWC_POST_INVALID;
        display->clearAllFences();
        setError(job);
        return;
    }

    HWBuffer* hw_outbuf = &job->hw_outbuf;
    buffer_handle_t outbuf_hnd = display->getOutbuf()->getHandle();
    // Because hidle may release the buffer handle during dispatcher doing the job,
    // we need to control the lifecycle by HWC.
    dupBufferHandle(display->getOutbuf()->getHandle(), &outbuf_hnd);
    PrivateHandle* priv_handle = &hw_outbuf->priv_handle;
    priv_handle->ion_fd = HWC_NO_ION_FD;

    status_t err = getPrivateHandle(outbuf_hnd, priv_handle);
    if (err != NO_ERROR)
    {
        PLOGE("Failed to get private handle !! (outbuf=%p) !!", outbuf_hnd);
        job->post_state = HWC_POST_INVALID;
        display->clearAllFences();
        setError(job);
        return;
    }

    job->post_state = HWC_POST_OUTBUF_ENABLE;

    hw_outbuf->dataspace = mapColorMode2DataSpace(display->getColorMode());
    if (HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id)
    {
        // mirror mode
        //
        // set mirror output buffer
        hw_outbuf->mir_in_rel_fence_fd = dupCloseFd(display->getOutbuf()->getReleaseFenceFd());
        display->getOutbuf()->setReleaseFenceFd(-1, display->isConnected());
        hw_outbuf->handle              = outbuf_hnd;
    }
    else if (job->is_black_job)
    {
        // Virtual Black mode
        //
        // set output buffer
        hw_outbuf->mir_in_rel_fence_fd = dupCloseFd(display->getOutbuf()->getReleaseFenceFd());
        display->getOutbuf()->setReleaseFenceFd(-1, display->isConnected());
        hw_outbuf->handle              = outbuf_hnd;

        // Extension path doesn't need this mirror_out_roi info, clear it can avoid to effect
        // bliter_async's clearBackground function no work.
        gralloc_extra_ion_hwc_info_t* hwc_ext_info = &priv_handle->hwc_ext_info;
        if (hwc_ext_info != nullptr)
        {
            if (hwc_ext_info->mirror_out_roi.x != 0 || hwc_ext_info->mirror_out_roi.y != 0 ||
                    hwc_ext_info->mirror_out_roi.w != 0 || hwc_ext_info->mirror_out_roi.h != 0)
            {
                hwc_ext_info->mirror_out_roi.x = 0;
                hwc_ext_info->mirror_out_roi.y = 0;
                hwc_ext_info->mirror_out_roi.w = 0;
                hwc_ext_info->mirror_out_roi.h = 0;
                gralloc_extra_perform(hw_outbuf->handle, GRALLOC_EXTRA_SET_HWC_INFO, hwc_ext_info);
            }
        }
    }
    else
    {
        // extension mode
        //
        // get retire fence from display driver
        OverlayPrepareParam prepare_param;
        {
            prepare_param.ion_fd        = priv_handle->ion_fd;
            prepare_param.is_need_flush = 0;

            err = m_ovl_engine->prepareOutput(prepare_param);
            if (NO_ERROR != err)
            {
                prepare_param.fence_index = 0;
                prepare_param.fence_fd = -1;
            }

            if (prepare_param.fence_fd <= 0)
            {
                PLOGE("Failed to get retireFence !!");
            }
        }

        hw_outbuf->out_acquire_fence_fd = dupCloseFd(display->getOutbuf()->getReleaseFenceFd());
        display->getOutbuf()->setReleaseFenceFd(-1, display->isConnected());
        hw_outbuf->out_retire_fence_fd  = prepare_param.fence_fd;
        hw_outbuf->out_retire_fence_idx = prepare_param.fence_index;
        hw_outbuf->handle               = outbuf_hnd;

        // Extension path doesn't need this mirror_out_roi info, clear it can avoid to effect
        // bliter_async's clearBackground function no work.
        gralloc_extra_ion_hwc_info_t* hwc_ext_info = &priv_handle->hwc_ext_info;
        if (hwc_ext_info != nullptr)
        {
            if (hwc_ext_info->mirror_out_roi.x != 0 || hwc_ext_info->mirror_out_roi.y != 0 ||
                    hwc_ext_info->mirror_out_roi.w != 0 || hwc_ext_info->mirror_out_roi.h != 0)
            {
                hwc_ext_info->mirror_out_roi.x = 0;
                hwc_ext_info->mirror_out_roi.y = 0;
                hwc_ext_info->mirror_out_roi.w = 0;
                hwc_ext_info->mirror_out_roi.h = 0;
                gralloc_extra_perform(hw_outbuf->handle, GRALLOC_EXTRA_SET_HWC_INFO, hwc_ext_info);
            }
        }

        display->setRetireFenceFd(prepare_param.fence_fd, display->isConnected());

        DbgLogger* logger = Debugger::getInstance().m_logger->set_info[job->disp_ori_id];

        logger->printf("/Outbuf(ret_fd=%d(%u), acq_fd=%d, handle=%p, ion=%d)",
            hw_outbuf->out_retire_fence_fd, hw_outbuf->out_retire_fence_idx,
            hw_outbuf->out_acquire_fence_fd, hw_outbuf->handle, priv_handle->ion_fd);

        if (!job->fbt_exist)
        {
            auto fbt_layer = display->getClientTarget();
            fbt_layer->setReleaseFenceFd(-1, display->isConnected());
            fbt_layer->setAcquireFenceFd(-1, display->isConnected());
        }
    }

    // set video usage and timestamp into output buffer handle
    gralloc_extra_ion_sf_info_t* ext_info = &hw_outbuf->priv_handle.ext_info;
    ext_info->timestamp = job->timestamp;
    if (DisplayManager::m_profile_level & PROFILE_TRIG)
    {
        // set token to buffer handle for profiling latency purpose
        ext_info->sequence = job->sequence;
    }
    gralloc_extra_perform(
        hw_outbuf->handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, ext_info);
}

void HWCDispatcher::VirPostHandler::setMirror(
    DispatcherJob* /*src_job*/, DispatcherJob* /*dst_job*/)
{
}

void HWCDispatcher::VirPostHandler::process(DispatcherJob* job)
{
    if (HWC_MIRROR_SOURCE_INVALID == job->disp_mir_id && !job->is_black_job)
    {
        // extension mode

        setOverlayInput(job);

        // set output buffer for virtual display
        {
            HWBuffer* hw_outbuf = &job->hw_outbuf;
            PrivateHandle* priv_handle = &hw_outbuf->priv_handle;

            // Reset used bit for mirror mode setBackGround
            // USED = 1xxx
            gralloc_extra_ion_sf_info_t* ext_info = &priv_handle->ext_info;
            int prev_orient = (ext_info->status & GRALLOC_EXTRA_MASK_ORIENT) >> 12;
            if (((prev_orient>>3) & 0x01) == 1)
            {
                gralloc_extra_sf_set_status(
                    ext_info, GRALLOC_EXTRA_MASK_ORIENT, (prev_orient<<12) & ~(0x01<<15));

                gralloc_extra_perform(
                    hw_outbuf->handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, ext_info);
            }

            OverlayPortParam param;

            bool is_secure = isSecure(priv_handle);
            if (is_secure)
            {
                param.va      = (void*)(uintptr_t)priv_handle->sec_handle;
                param.mva     = (void*)(uintptr_t)priv_handle->sec_handle;
            }
            else
            {
                param.va      = NULL;
                param.mva     = NULL;
            }
            param.pitch       = priv_handle->y_stride;
            param.format      = priv_handle->format;
            param.dst_crop    = Rect(priv_handle->width, priv_handle->height);
            param.fence_index = hw_outbuf->out_retire_fence_idx;
            param.secure      = is_secure;
            param.sequence    = job->sequence;
            param.ion_fd      = priv_handle->ion_fd;
            param.fence       = hw_outbuf->out_acquire_fence_fd;
            param.dataspace   = hw_outbuf->dataspace;
            hw_outbuf->out_acquire_fence_fd = -1;

            m_ovl_engine->setOutput(&param);
        }

        // trigger overlay engine
        m_ovl_engine->trigger(job->ovl_valid, job->num_layers, DISP_NO_PRESENT_FENCE,
                              -1, job->need_av_grouping, job->color_transform);
    }
    else
    {
        // mirror mode
        if (DisplayManager::m_profile_level & PROFILE_TRIG)
        {
            char atrace_tag[256];
            sprintf(atrace_tag, "MDP-SMS");
            HWC_ATRACE_ASYNC_END(atrace_tag, job->sequence);
        }
    }
}

// ---------------------------------------------------------------------------

HWCDispatcher::PostProcessingHandler::PostProcessingHandler(HWCDispatcher* dispatcher, int dpy, const sp<OverlayEngine>& ovl_engine)
    : PostHandler(dispatcher, dpy, ovl_engine)
{
}

void HWCDispatcher::PostProcessingHandler::set(
    const sp<HWCDisplay>& display, DispatcherJob* job)
{
    job->hw_outbuf.phy_present_fence_fd = -1;
    job->hw_outbuf.phy_present_fence_idx = DISP_NO_PRESENT_FENCE;

    if (HWC_MIRROR_SOURCE_INVALID != job->disp_mir_id)
    {
        job->post_state = HWC_POST_MIRROR;
        return;
    }

    uint32_t total_num = job->num_layers;

    bool is_dirty = (job->post_state & HWC_POST_CONTINUE_MASK) != 0;

    HWC_ATRACE_FORMAT_NAME("BeginTransform");
    if (is_dirty)
    {
        for (uint32_t i = 0; i < total_num; i++)
        {
            HWLayer* hw_layer = &job->hw_layers[i];

            if (!hw_layer->enable) continue;

            if (HWC_LAYER_TYPE_DIM == hw_layer->type) continue;

            sp<HWCLayer> layer = display->getCommittedLayers()[hw_layer->index];
            PrivateHandle* priv_handle = &hw_layer->priv_handle;

            HWC_ATRACE_FORMAT_NAME("InputLayer(h:%p)", layer->getHandle());
            // check if fbt is dirty
            if (HWC_LAYER_TYPE_FBT == hw_layer->type || HWC_LAYER_TYPE_MM_FBT == hw_layer->type)
            {
                if (priv_handle == nullptr)
                {
                    hw_layer->enable = false;
                    continue;
                }

                if (layer->isBufferChanged())
                {
                    hw_layer->dirty |= HWC_LAYER_DIRTY_BUFFER;
                }
            }

            // check if any layer is dirty
            is_dirty |= (hw_layer->dirty != HWC_LAYER_DIRTY_NONE);

            gralloc_extra_sf_set_status(
                &priv_handle->ext_info, GRALLOC_EXTRA_MASK_SF_DIRTY, GRALLOC_EXTRA_BIT_UNDIRTY);

            gralloc_extra_perform(
                layer->getHandle(), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &priv_handle->ext_info);
        }
    }

    if (is_dirty)
    {
        job->post_state = HWC_POST_INPUT_DIRTY;

        {
            // get present fence from display driver
            OverlayPrepareParam prepare_param;
            {
                if (m_disp_id == HWC_DISPLAY_PRIMARY)
                {
                    status_t err = m_ovl_engine->preparePresentFence(prepare_param);
                    if (NO_ERROR != err)
                    {
                        prepare_param.fence_index = 0;
                        prepare_param.fence_fd = -1;
                    }

                    if (prepare_param.fence_fd <= 0)
                    {
                        PLOGE("Failed to get presentFence !!");
                    }
                }
                else
                {
                    prepare_param.fence_index = 0;
                    prepare_param.fence_fd = -1;
                }
            }

            HWBuffer* hw_outbuf = &job->hw_outbuf;
            hw_outbuf->phy_present_fence_fd  = prepare_param.fence_fd;
            hw_outbuf->phy_present_fence_idx = prepare_param.fence_index;
            hw_outbuf->handle                = NULL;

            job->prev_present_fence_fd = m_curr_present_fence_fd;
            m_curr_present_fence_fd = (prepare_param.fence_fd >= 0) ? dup(prepare_param.fence_fd) : -1;
            if (m_disp_id == HWC_DISPLAY_PRIMARY &&
                Platform::getInstance().m_config.is_support_mdp_pmqos) {
                HWVSyncEstimator::getInstance().pushPresentFence(
                    (m_curr_present_fence_fd >= 0) ? ::dup(m_curr_present_fence_fd) : -1);
                HWVSyncEstimator::getInstance().update(job->disp_data->refresh);
            }

            if (Platform::getInstance().m_config.is_support_mdp_pmqos_debug)
            {
                MDPFrameInfoDebugger::getInstance().setJobPresentFenceFd(job->sequence,
                    m_curr_present_fence_fd >= 0 ? ::dup(m_curr_present_fence_fd): -1);
            }

            if (HWCMediator::getInstance().m_features.without_primary_present_fence
                    && m_disp_id == HWC_DISPLAY_PRIMARY)
            {
                display->setRetireFenceFd(-1, display->isConnected());
                protectedClose(prepare_param.fence_fd);
            }
            else
            {
                display->setRetireFenceFd(prepare_param.fence_fd, display->isConnected());
            }

            DbgLogger* logger = Debugger::getInstance().m_logger->set_info[job->disp_ori_id];
            logger->printf("/PF(fd=%d, idx=%d, curr_pf_fd=%d)",
                hw_outbuf->phy_present_fence_fd, hw_outbuf->phy_present_fence_idx, m_curr_present_fence_fd);
            HWC_ATRACE_FORMAT_NAME("TurnInto(%d)", prepare_param.fence_index);
        }
    }
    else
    {
        // set as nodirty since could not find any dirty layers
        job->post_state = HWC_POST_INPUT_NOTDIRTY;

        DbgLogger* logger = Debugger::getInstance().m_logger->set_info[job->disp_ori_id];
        logger->printf(" / skip composition: no dirty layers");
        // clear all layers' acquire fences
        display->clearAllFences();
    }
}

void HWCDispatcher::PostProcessingHandler::setMirror(
    DispatcherJob* src_job, DispatcherJob* dst_job)
{
    HWBuffer* phy_outbuf = &src_job->hw_outbuf;
    HWBuffer* dst_mirbuf = &dst_job->hw_mirbuf;
    HWBuffer* dst_outbuf = &dst_job->hw_outbuf;

    dst_mirbuf->mir_in_acq_fence_fd = phy_outbuf->mir_out_acq_fence_fd;
    dst_mirbuf->handle              = phy_outbuf->handle;
    memcpy(&dst_mirbuf->priv_handle, &phy_outbuf->priv_handle, sizeof(PrivateHandle));

    if (dst_job->disp_ori_id == HWC_DISPLAY_EXTERNAL)
    {
        unsigned int dst_format = phy_outbuf->priv_handle.format;
        switch (Platform::getInstance().m_config.format_mir_mhl)
        {
            case MIR_FORMAT_RGB888:
                dst_format = HAL_PIXEL_FORMAT_RGB_888;
                break;
            case MIR_FORMAT_YUYV:
                dst_format = HAL_PIXEL_FORMAT_YUYV;
                break;
            case MIR_FORMAT_YV12:
                dst_format = HAL_PIXEL_FORMAT_YV12;
                break;
            case MIR_FORMAT_UNDEFINE:
                // use same format as source, so do nothing
                break;
            default:
                HWC_LOGW("Not support mir format(%d), use same format as source(%x)",
                    Platform::getInstance().m_config.format_mir_mhl,
                    phy_outbuf->priv_handle.format);
         }

        dst_outbuf->priv_handle.format = dst_format;
        dst_outbuf->priv_handle.usage = phy_outbuf->priv_handle.usage;
    }

    // in decouple mode, need to wait for
    // both display and MDP finish reading this buffer
    {
        char name[32];
        snprintf(name, sizeof(name), "merged_fence(%d/%d)\n",
            phy_outbuf->mir_out_if_fence_fd, dst_mirbuf->mir_in_rel_fence_fd);

        // There are two components need MDP fence in the mirror path.
        // Memory session has duplicated it in set function of bliter, then return it to SF.
        // External session does not duplicated it, so we duplicated it in here.
        int tmp_fd = (dst_job->disp_ori_id == HWC_DISPLAY_EXTERNAL) ?
                ::dup(dst_mirbuf->mir_in_rel_fence_fd) :
                dst_mirbuf->mir_in_rel_fence_fd;

        int merged_fd = SyncFence::merge(
            phy_outbuf->mir_out_if_fence_fd,
            tmp_fd,
            name);

        ::protectedClose(phy_outbuf->mir_out_if_fence_fd);
        ::protectedClose(tmp_fd);

        // TODO: merge fences from different virtual displays to phy_outbuf->mir_out_mer_fence_fd
        phy_outbuf->mir_out_if_fence_fd = merged_fd;
    }

    PLOGD("set mirror (rel_fd=%d(%u)/handle=%p/ion=%d/dataspace=%d -> acq_fd=%d/handle=%p/ion=%d/dataspace=%d)",
        dst_mirbuf->mir_in_rel_fence_fd, dst_mirbuf->mir_in_sync_marker,
        dst_mirbuf->handle, dst_mirbuf->priv_handle.ion_fd, dst_mirbuf->dataspace,
        dst_outbuf->mir_in_rel_fence_fd, dst_outbuf->handle, dst_outbuf->priv_handle.ion_fd,
        dst_outbuf->dataspace);
}

void HWCDispatcher::PostProcessingHandler::process(DispatcherJob* /*job*/)
{
}
