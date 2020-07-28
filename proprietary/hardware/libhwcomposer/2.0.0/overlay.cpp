#define DEBUG_LOG_TAG "OVL"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdlib.h>

#include "hwc_priv.h"

#include "utils/debug.h"
#include "utils/tools.h"

#include "hwc2_api.h"
#include "overlay.h"
#include "dispatcher.h"
#include "display.h"
#include "queue.h"
#include "composer.h"
#include "sync.h"
#include "platform.h"

#define OLOGV(x, ...) HWC_LOGV("(%" PRIu64 ") " x, m_disp_id, ##__VA_ARGS__)
#define OLOGD(x, ...) HWC_LOGD("(%" PRIu64 ") " x, m_disp_id, ##__VA_ARGS__)
#define OLOGI(x, ...) HWC_LOGI("(%" PRIu64 ") " x, m_disp_id, ##__VA_ARGS__)
#define OLOGW(x, ...) HWC_LOGW("(%" PRIu64 ") " x, m_disp_id, ##__VA_ARGS__)
#define OLOGE(x, ...) HWC_LOGE("(%" PRIu64 ") " x, m_disp_id, ##__VA_ARGS__)

// ---------------------------------------------------------------------------

OverlayEngine::OverlayInput::OverlayInput()
    : connected_state(OVL_PORT_DISABLE)
    , connected_type(OVL_INPUT_NONE)
    , queue(NULL)
{ }

OverlayEngine::OverlayOutput::OverlayOutput()
    : connected_state(OVL_PORT_DISABLE)
    , queue(NULL)
{ }

OverlayEngine::OverlayEngine(const uint64_t& dpy)
    : m_disp_id(dpy)
    , m_handling_job(NULL)
    , m_sync_fence(new SyncFence(dpy))
    , m_stop(false)
    , m_post_processing(NULL)
    , m_need_wakeup(false)
    , m_ignore_av_grouping(false)
    , m_prev_power_mode(0)
    , m_power_mode_changed(0)
{
    // create overlay session
    status_t err = HWCMediator::getInstance().getOvlDevice(m_disp_id)->createOverlaySession(m_disp_id);

    if (err != NO_ERROR)
    {
        m_engine_state = OVL_ENGINE_DISABLED;
        m_max_inputs = 0;

        OLOGE("Failed to create display session");
    }
    else
    {
        m_engine_state = OVL_ENGINE_ENABLED;
        m_max_inputs = HWCMediator::getInstance().getOvlDevice(m_disp_id)->getMaxOverlayInputNum();

        int32_t avail_inputs = m_max_inputs;
        if (HWC_DISPLAY_PRIMARY == m_disp_id)
        {
            avail_inputs = HWCMediator::getInstance().getOvlDevice(m_disp_id)->getAvailableOverlayInput(m_disp_id);
            HWCMediator::getInstance().getOvlDevice(m_disp_id)->initOverlay();
        }

        for (int id = 0; id < m_max_inputs; id++)
        {
            // init input configurations
            OverlayInput* input = new OverlayInput();
            if (id >= avail_inputs)
            {
                input->connected_state = OVL_PORT_ENABLE;
                input->connected_type  = OVL_INPUT_UNKNOWN;
            }
            m_inputs.add(input);
            m_input_params.add(&input->param);
        }
        m_rel_fence_fds.resize(m_max_inputs);

        for (int32_t i = 0; i < m_max_inputs; ++i)
            m_rel_fence_fds[i] = -1;

        m_last_acquired_bufs.resize(m_max_inputs);
        for (int32_t i = 0; i < m_max_inputs; ++i)
            m_last_acquired_bufs[i].index = -1;

        // initialize display data for physical display
        if (HWC_DISPLAY_VIRTUAL > m_disp_id)
            DisplayManager::getInstance().setDisplayDataForPhy(dpy);
    }

    snprintf(m_thread_name, sizeof(m_thread_name), "OverlayEngine_%" PRIu64, dpy);

    String8 pool_name(m_thread_name);
    pool_name.append("_FrameInfo");
    m_pool = new ObjectPool<FrameInfo>(pool_name, 5);
}

OverlayEngine::~OverlayEngine()
{
    size_t size = m_frame_queue.size();
    for (size_t i = 0; i < size; i++)
    {
        closeAllFenceFd(m_frame_queue[i]);
    }
    m_last_frame_info = nullptr;
    m_frame_queue.clear();
    delete m_pool;

    for (int id = 0; id < m_max_inputs; id++)
    {
        m_inputs[id]->queue = NULL;

        delete m_inputs[id];
    }
    m_inputs.clear();
    m_input_params.clear();

    for (size_t i = 0; i < m_rel_fence_fds.size(); ++i)
    {
        if (m_rel_fence_fds[i] != -1)
        {
            ::protectedClose(m_rel_fence_fds[i]);
            m_rel_fence_fds[i] = -1;
        }
    }

    m_output.queue = NULL;

    HWCMediator::getInstance().getOvlDevice(m_disp_id)->destroyOverlaySession(m_disp_id);
}

int OverlayEngine::getAvailableInputNum()
{
    SessionInfo info;
    int avail_inputs;

    HWCMediator::getInstance().getOvlDevice(m_disp_id)->getOverlaySessionInfo(m_disp_id, &info);
    avail_inputs = info.maxLayerNum;

    // only main display need to check if fake display exists
    if (HWC_DISPLAY_PRIMARY == m_disp_id)
    {
        int fake_num = DisplayManager::getInstance().getFakeDispNum();
        avail_inputs = (avail_inputs > fake_num) ? (avail_inputs - fake_num) : avail_inputs;
    }

    return avail_inputs;
}

bool OverlayEngine::waitUntilAvailable()
{
#ifndef MTK_USER_BUILD
    char atrace_tag[128];
    sprintf(atrace_tag, "wait_ovl_avail(%" PRIu64 ")", m_disp_id);
    HWC_ATRACE_NAME(atrace_tag);
#endif

    int avail_inputs;
    int try_count = 0;

    // TODO: use synchronous ioctl instead of busy-waiting
    do
    {
        avail_inputs = getAvailableInputNum();

        if (avail_inputs != 0) break;

        try_count++;

        OLOGW("Waiting for available OVL (cnt=%d)", try_count);

        usleep(5000);
    } while (try_count < 1000);

    if (avail_inputs == 0)
    {
        OLOGE("Timed out waiting for OVL (cnt=%d)", try_count);
        return false;
    }

    return true;
}

status_t OverlayEngine::prepareInput(OverlayPrepareParam& param)
{
    AutoMutex l(m_lock);

    int id = param.id;

    if (id >= m_max_inputs)
    {
        OLOGE("Failed to prepare invalid overlay input(%d)", id);
        return BAD_INDEX;
    }

    HWCMediator::getInstance().getOvlDevice(m_disp_id)->prepareOverlayInput(m_disp_id, &param);

    return NO_ERROR;
}

status_t OverlayEngine::setInputQueue(int id, sp<DisplayBufferQueue> queue)
{
    AutoMutex l(m_lock);

    if (id >= m_max_inputs)
    {
        OLOGE("Failed to set invalid overlay input(%d)", id);
        return BAD_INDEX;
    }

    if (OVL_INPUT_QUEUE == m_inputs[id]->connected_type)
    {
        OLOGW("Already set overlay input(%d)", id);
        return BAD_INDEX;
    }

    struct InputListener : public DisplayBufferQueue::ConsumerListener
    {
        InputListener(sp<OverlayEngine> ovl, int id)
            : m_engine(ovl)
            , m_id(id)
        { }
    private:
        sp<OverlayEngine> m_engine;
        int m_id;
        virtual void onBufferQueued()
        {
            m_engine->updateInput(m_id);
        }
    };
    if (m_post_processing != NULL)
    {
        m_post_processing->setChannelId(id);
        queue->setConsumerListener(m_post_processing);
    }
    else
    {
        queue->setConsumerListener(new InputListener(this, id));
    }

    m_inputs[id]->queue = queue;
    m_inputs[id]->connected_state = OVL_PORT_ENABLE;
    m_inputs[id]->connected_type = OVL_INPUT_QUEUE;

    return NO_ERROR;
}

status_t OverlayEngine::setInputDirect(int id)
{
    AutoMutex l(m_lock);

    if (id >= m_max_inputs)
    {
        OLOGE("Failed to set invalid overlay input(%d)", id);
        return BAD_INDEX;
    }

    if (OVL_INPUT_QUEUE == m_inputs[id]->connected_type)
    {
        OLOGI("Overlay input(%d) was used with queue previously", id);
    }

    m_inputs[id]->queue = NULL;
    m_inputs[id]->connected_state = OVL_PORT_ENABLE;
    m_inputs[id]->connected_type = OVL_INPUT_DIRECT;
    m_last_acquired_bufs[id].index = DisplayBufferQueue::INVALID_BUFFER_SLOT;
    if (m_rel_fence_fds[id] != -1)
    {
        ::protectedClose(m_rel_fence_fds[id]);
        m_rel_fence_fds[id] = -1;
    }

    return NO_ERROR;
}

status_t OverlayEngine::setInputs(int num)
{
    AutoMutex l(m_lock);

    if (DisplayManager::m_profile_level & PROFILE_TRIG)
    {
        char atrace_tag[128];
        sprintf(atrace_tag, "set_ovl(%" PRIu64 "): set inputs", m_disp_id);
        HWC_ATRACE_NAME(atrace_tag);
        OLOGV("HWC->OVL: set inputs (max=%d)", num);

        HWCMediator::getInstance().getOvlDevice(m_disp_id)->updateOverlayInputs(
            m_disp_id, m_input_params.array(), num, nullptr);
    }
    else
    {
        HWCMediator::getInstance().getOvlDevice(m_disp_id)->updateOverlayInputs(
            m_disp_id, m_input_params.array(), num, nullptr);
    }

    return NO_ERROR;
}

status_t OverlayEngine::disableInput(int id)
{
    AutoMutex l(m_lock);

    if (id >= m_max_inputs)
    {
        OLOGE("Failed to disable invalid overlay input(%d)", id);
        return BAD_INDEX;
    }

    if (OVL_INPUT_NONE == m_inputs[id]->connected_type)
    {
        //OLOGW("Not using overlay input(%d)", id);
        return BAD_INDEX;
    }

    disableInputLocked(id);

    return NO_ERROR;
}

status_t OverlayEngine::disableOutput()
{
    AutoMutex l(m_lock);

    disableOutputLocked();

    return NO_ERROR;
}

void OverlayEngine::disableInputLocked(int id)
{
    // set overlay params
    m_input_params[id]->state = OVL_IN_PARAM_DISABLE;

    // clear input infomation
    m_inputs[id]->queue = NULL;
    m_inputs[id]->connected_state = OVL_PORT_DISABLE;
    m_inputs[id]->connected_type = OVL_INPUT_NONE;
    m_last_acquired_bufs[id].index = DisplayBufferQueue::INVALID_BUFFER_SLOT;
    if (m_rel_fence_fds[id] != -1)
    {
        ::protectedClose(m_rel_fence_fds[id]);
        m_rel_fence_fds[id] = -1;
    }
}

void OverlayEngine::disableOutputLocked()
{
    // clear output infomation
    m_output.connected_state = OVL_PORT_DISABLE;
    memset(&m_output.param, 0, sizeof(OverlayPortParam));
    m_output.param.ion_fd = -1;
    m_output.param.fence = -1;
    m_output.param.mir_rel_fence_fd = -1;
}

status_t OverlayEngine::prepareOutput(OverlayPrepareParam& param)
{
    AutoMutex l(m_lock);

    param.id = m_max_inputs;
    HWCMediator::getInstance().getOvlDevice(m_disp_id)->prepareOverlayOutput(m_disp_id, &param);

    return NO_ERROR;
}

status_t OverlayEngine::setOutput(OverlayPortParam* param, bool need_output_buffer)
{
    AutoMutex l(m_lock);

    if (CC_UNLIKELY(param == NULL))
    {
        OLOGE("HWC->OVL: output param is NULL, disable output");
        disableOutputLocked();
        return INVALID_OPERATION;
    }

    if (need_output_buffer && (m_output.queue != NULL))
    {
        DisplayBufferQueue::DisplayBuffer mir_buffer;
        m_output.queue->acquireBuffer(&mir_buffer, true);
        m_output.queue->releaseBuffer(mir_buffer.index, param->mir_rel_fence_fd);
        m_cond.signal();
    }

    m_output.connected_state = OVL_PORT_ENABLE;

    memcpy(&m_output.param, param, sizeof(OverlayPortParam));

    return NO_ERROR;
}

status_t OverlayEngine::preparePresentFence(OverlayPrepareParam& param)
{
    AutoMutex l(m_lock);

    if (HWC_DISPLAY_PRIMARY == m_disp_id ||
        HWC_DISPLAY_EXTERNAL == m_disp_id)
    {
        HWCMediator::getInstance().getOvlDevice(m_disp_id)->prepareOverlayPresentFence(m_disp_id, &param);
    }
    else
    {
        param.fence_fd = -1;
        param.fence_index = 0;
    }

    return NO_ERROR;
}

status_t OverlayEngine::createOutputQueue(int format, bool secure)
{
    AutoMutex l(m_lock);
    return createOutputQueueLocked(format, secure);
}

status_t OverlayEngine::createOutputQueueLocked(int format, bool secure)
{
#ifndef MTK_USER_BUILD
    char atrace_tag[128];
    sprintf(atrace_tag, "create_out_queue(%" PRIu64 ")", m_disp_id);
    HWC_ATRACE_NAME(atrace_tag);
#endif

    bool need_init = false;
    int buffer_slots = DisplayBufferQueue::NUM_BUFFER_SLOTS;
    if (Platform::getInstance().m_config.buffer_slots > 0){
        buffer_slots = Platform::getInstance().m_config.buffer_slots;
    }

    // verify if need to create output queue
    if (m_output.queue == NULL)
    {
        need_init = true;

        m_output.queue = new DisplayBufferQueue(DisplayBufferQueue::QUEUE_TYPE_OVL, buffer_slots);
        m_output.queue->setSynchronousMode(true);

        OLOGD("Create output queue");
    }

    int bpp = getBitsPerPixel(format);

    const DisplayData* disp_data = DisplayManager::getInstance().getDisplayData(m_disp_id);
    DisplayBufferQueue::BufferParam buffer_param;
    buffer_param.width  = disp_data->width;
    buffer_param.height = disp_data->height;
    buffer_param.pitch  = disp_data->width;
    buffer_param.format = mapGrallocFormat(format);
    buffer_param.size   = (disp_data->width * disp_data->height * bpp / 8);
    buffer_param.dequeue_block = false;
    m_output.queue->setBufferParam(buffer_param);

    if (need_init)
    {
        // allocate buffers
        DisplayBufferQueue::DisplayBuffer mir_buffer[buffer_slots];
        for (size_t i = 0; i < buffer_slots; i++)
        {
            m_output.queue->dequeueBuffer(&mir_buffer[i], false, secure);
        }

        for (size_t i = 0; i < buffer_slots; i++)
        {
            m_output.queue->cancelBuffer(mir_buffer[i].index);
        }

        OLOGD("Initialize buffers for output queue");
    }

    return NO_ERROR;
}

status_t OverlayEngine::releaseOutputQueue()
{
    AutoMutex l(m_lock);

    m_output.connected_state = OVL_PORT_DISABLE;
    m_output.queue = NULL;

    OLOGD("Output buffer queue is relased");

    return NO_ERROR;
}

status_t OverlayEngine::configMirrorOutput(DispatcherJob* job)
{
#ifndef MTK_USER_BUILD
    char atrace_tag[128];
    sprintf(atrace_tag, "set_mirror(%" PRIu64 ")", m_disp_id);
    HWC_ATRACE_NAME(atrace_tag);
#endif

    AutoMutex l(m_lock);

    HWBuffer* outbuf = &job->hw_outbuf;

    // if virtial display is used as mirror source
    // no need to use extra buffer since it already has its own output buffer
    if ((HWC_DISPLAY_VIRTUAL == m_disp_id) || (outbuf == NULL))
        return NO_ERROR;

    // it can happen if SurfaceFlinger tries to access output buffer queue
    // right after hotplug thread just released it (e.g. via onPlugOut())
    if (CC_UNLIKELY(m_output.queue == NULL))
    {
        OLOGW("output buffer queue has been released");
        return INVALID_OPERATION;
    }

    // prepare overlay output buffer
    DisplayBufferQueue::DisplayBuffer out_buffer;
    unsigned int acq_fence_idx = 0;
    int if_fence_fd = -1;
    unsigned int if_fence_idx = 0;
    {
        status_t err;
        m_output.queue->updateBufferSize(job->disp_data->width, job->disp_data->height);

        do
        {
            err = m_output.queue->dequeueBuffer(&out_buffer, true, job->secure);
            if (NO_ERROR != err)
            {
                OLOGW("cannot find available buffer, wait...");
                m_cond.wait(m_lock);
                OLOGW("wake up to find available buffer");
            }
        } while (NO_ERROR != err);

        OverlayPrepareParam prepare_param;
        prepare_param.id            = m_max_inputs;
        prepare_param.ion_fd        = out_buffer.out_ion_fd;
        prepare_param.is_need_flush = 0;
        HWCMediator::getInstance().getOvlDevice(m_disp_id)->prepareOverlayOutput(m_disp_id, &prepare_param);
        if (prepare_param.fence_fd <= 0)
        {
            OLOGW("Failed to get mirror acquireFence !!");
        }

        out_buffer.acquire_fence = prepare_param.fence_fd;
        acq_fence_idx            = prepare_param.fence_index;
        if_fence_fd              = prepare_param.if_fence_fd;
        if_fence_idx             = prepare_param.if_fence_index;

        m_output.queue->queueBuffer(&out_buffer);
    }

    // fill mirror output buffer info
    outbuf->mir_out_sec_handle    = out_buffer.out_sec_handle;
    outbuf->mir_out_rel_fence_fd  = out_buffer.release_fence;
    outbuf->mir_out_acq_fence_fd  = out_buffer.acquire_fence;
    outbuf->mir_out_acq_fence_idx = acq_fence_idx;
    outbuf->mir_out_if_fence_fd   = if_fence_fd;
    outbuf->mir_out_if_fence_idx  = if_fence_idx;
    outbuf->handle                = out_buffer.out_handle;
    int32_t err = getPrivateHandle(outbuf->handle, &outbuf->priv_handle);
    if (err != NO_ERROR)
    {
        OLOGE("%s: Failed to get private handle of outbuf(%d)!", __func__, err);
    }

    OLOGV("%s if_fence_fd:%d", __func__, if_fence_fd);
    if (DisplayManager::m_profile_level & PROFILE_TRIG)
    {
        OLOGI("HWC->OVL: config output (rel_fd=%d acq_fd=%d/idx=%u)",
            out_buffer.release_fence, out_buffer.acquire_fence, acq_fence_idx);
    }

    return NO_ERROR;
}

status_t OverlayEngine::setOverlaySessionMode(HWC_DISP_MODE mode)
{
    return HWCMediator::getInstance().getOvlDevice(m_disp_id)->setOverlaySessionMode(m_disp_id, mode);
}

HWC_DISP_MODE OverlayEngine::getOverlaySessionMode()
{
    return HWCMediator::getInstance().getOvlDevice(m_disp_id)->getOverlaySessionMode(m_disp_id);
}

void OverlayEngine::setPostProcessingEngine(sp<PostProcessingEngine> ppe)
{
    AutoMutex l(m_lock);
    m_post_processing = ppe;
}

void OverlayEngine::removePostProcessingEngine()
{
    AutoMutex l(m_lock);
    m_post_processing = NULL;
}

void OverlayEngine::trigger(const bool& ovl_valid, const int& num_layers,
                            const int& present_fence_idx, const int& prev_present_fence,
                            const bool& av_grouping, sp<ColorTransform> color_transform,
                            const bool& do_nothing)
{
    if (!do_nothing)
    {
        sp<FrameInfo> frame_info = m_pool->getFreeObject();
        AutoMutex l(m_lock);

        // start to duplicate the parameters of this frame
        packageFrameInfo(frame_info, ovl_valid, num_layers,
                         present_fence_idx, prev_present_fence,
                         av_grouping, color_transform);

        m_frame_queue.add(frame_info);
    }
    else
    {
        DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D');
        logger.printf("(%" PRIu64 ") Trigger with idx: %d/ do_nothing", m_disp_id, present_fence_idx);
    }

    {
        AutoMutex l(m_lock);
        m_state = HWC_THREAD_TRIGGER;
        sem_post(&m_event);
    }
}

void OverlayEngine::stop()
{
    AutoMutex l(m_lock);
    m_stop = true;
}

status_t OverlayEngine::loopHandler(sp<FrameInfo>& info)
{
    if (HWC_DISPLAY_VIRTUAL <= m_disp_id)
    {
        if (info->overlay_info.enable_output == false)
        {
            if (DisplayManager::m_profile_level & PROFILE_TRIG)
            {
                char atrace_tag[128];
                sprintf(atrace_tag, "trig_ovl(%" PRIu64 "): fail w/o output", m_disp_id);
                HWC_ATRACE_NAME(atrace_tag);
            }

            OLOGE("Try to trigger w/o set output port !!");
            return -EINVAL;
        }
    }

    status_t err;

    int fence_idx = info->present_fence_idx;
    int ovlp_layer_num = info->ovlp_layer_num;
    const uint32_t& hrt_weight = info->hrt_weight;
    const uint32_t& hrt_idx = info->hrt_idx;
    hwc2_config_t config = info->active_config;
    int prev_present_fence = info->prev_present_fence;

    {
        DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D');
        logger.printf("(%" PRIu64 ") triggerOverlaySession ovlp:%d idx:%d prev_present_fence:%d",
            m_disp_id, ovlp_layer_num, fence_idx, prev_present_fence);
    }

    if (DisplayManager::m_profile_level & PROFILE_TRIG)
    {
        char atrace_tag[128];
        sprintf(atrace_tag, "trig_ovl(%" PRIu64 ")", m_disp_id);
        HWC_ATRACE_NAME(atrace_tag);
        OLOGV("HWC->OVL: trig");
    }

    err = HWCMediator::getInstance().getOvlDevice(m_disp_id)->triggerOverlaySession(
        m_disp_id, fence_idx, ovlp_layer_num, prev_present_fence, config, hrt_weight, hrt_idx, info->is_HBM);

    return err;
}

OverlayPortParam* const* OverlayEngine::getInputParams()
{
    return m_input_params.array();
}

void OverlayEngine::setPowerMode(int mode)
{
    AutoMutex l(m_lock);

    switch (mode)
    {
        case HWC2_POWER_MODE_OFF:
            {
                m_engine_state = OVL_ENGINE_PAUSED;

                // ePaper still need DBQ after suspend, so we should not release resource in here
                if (DisplayManager::getInstance().getDisplayData(m_disp_id)->subtype != HWC_DISPLAY_EPAPER)
                {
                    int num = HWCMediator::getInstance().getOvlDevice(m_disp_id)->getAvailableOverlayInput(m_disp_id);

                    if (HWC_DISPLAY_VIRTUAL > m_disp_id)
                    {
                        HWCMediator::getInstance().getOvlDevice(m_disp_id)->disableOverlaySession(
                            m_disp_id, m_input_params.array(), num);
                    }

                    for (int id = 0; id < m_max_inputs; id++)
                    {
                        if (OVL_INPUT_NONE != m_inputs[id]->connected_type)
                            disableInputLocked(id);
                    }
                }
            }
            break;

        case HWC2_POWER_MODE_DOZE:
        case HWC2_POWER_MODE_ON:
            m_engine_state = OVL_ENGINE_ENABLED;
            break;

        case HWC2_POWER_MODE_DOZE_SUSPEND:
            m_engine_state = OVL_ENGINE_PAUSED;
            break;
    }

    if (HWC_DISPLAY_VIRTUAL > m_disp_id)
    {
        HWCMediator::getInstance().getOvlDevice(m_disp_id)->setPowerMode(m_disp_id, mode);
    }

    if (m_prev_power_mode != mode)
        m_power_mode_changed = POWER_MODE_CHANGED_DO_VALIDATE_NUM;

    m_prev_power_mode = mode;
}

sp<DisplayBufferQueue> OverlayEngine::getInputQueue(int id) const
{
    AutoMutex l(m_lock);

    if (id >= m_max_inputs)
    {
        OLOGE("Failed to get overlay input queue(%d)", id);
        return NULL;
    }

    if (OVL_INPUT_QUEUE != m_inputs[id]->connected_type)
    {
        OLOGW("No overlay input queue(%d)", id);
        return NULL;
    }

    return m_inputs[id]->queue;
}

void OverlayEngine::updateInput(int id)
{
    AutoMutex l(m_lock);

    if (m_inputs[id]->connected_state == OVL_PORT_ENABLE)
    {
        DisplayBufferQueue::DisplayBuffer* buffer = &(m_last_acquired_bufs[id]);
        if (buffer->index != DisplayBufferQueue::INVALID_BUFFER_SLOT)
        {
            const int32_t rel_fence_fd = m_rel_fence_fds[id];
            m_rel_fence_fds[id] = -1;
            // release previous buffer
            if (m_inputs[id]->queue->releaseBuffer(buffer->index, rel_fence_fd) != NO_ERROR)
            {
                ::protectedClose(rel_fence_fd);
            }
        }

        // acquire next buffer
        m_inputs[id]->queue->acquireBuffer(buffer, true);

        PrivateHandle priv_handle;
        int32_t err = getPrivateHandleInfo(buffer->out_handle, &priv_handle);
        err |= getPrivateHandleBuff(buffer->out_handle, &priv_handle);

        // get release fence for input queue
        OverlayPrepareParam prepare_param;
        {
            prepare_param.id            = id;
            prepare_param.ion_fd        = buffer->out_ion_fd;
            prepare_param.is_need_flush = 0;

            HWCMediator::getInstance().getOvlDevice(m_disp_id)->prepareOverlayInput(m_disp_id, &prepare_param);

            if (prepare_param.fence_fd <= 0)
            {
                OLOGE("(%d) Failed to get releaseFence for input queue", id);
            }
            m_rel_fence_fds[id] = prepare_param.fence_fd;
        }

        // fill struct for enable layer
        OverlayPortParam* param = m_input_params[id];
        param->state          = OVL_IN_PARAM_ENABLE;
        if (buffer->secure)
        {
            param->va         = (void*)(uintptr_t)buffer->out_sec_handle;
            param->mva        = (void*)(uintptr_t)buffer->out_sec_handle;
        }
        else
        {
            param->va         = NULL;
            param->mva        = NULL;
        }
        param->pitch          = buffer->data_pitch;
        param->v_pitch        = priv_handle.vstride;
        param->format         = buffer->data_format;
        param->color_range    = buffer->data_color_range;
        param->dataspace      = buffer->dataspace;
        param->src_crop       = buffer->data_info.src_crop;
        param->dst_crop       = buffer->data_info.dst_crop;
        param->is_sharpen     = buffer->data_info.is_sharpen;
        param->fence_index    = prepare_param.fence_index;
        param->identity       = HWLAYER_ID_DBQ;
        param->connected_type = OVL_INPUT_QUEUE;
        param->protect        = buffer->protect;
        param->secure         = buffer->secure;
        param->alpha_enable   = buffer->alpha_enable;
        param->alpha          = buffer->alpha;
        param->blending       = buffer->blending;
        param->sequence       = buffer->sequence;
        param->dim            = false;
        param->ion_fd         = buffer->out_ion_fd;
        param->is_s3d_layer   = buffer->is_s3d_buffer;
        param->s3d_type       = buffer->s3d_buffer_type;
        param->ext_sel_layer  = buffer->ext_sel_layer;
        param->fence          = buffer->acquire_fence;
        buffer->acquire_fence  = -1;

        // partial update - process mm layer dirty rect roughly
        param->ovl_dirty_rect_cnt = 1;
        param->ovl_dirty_rect[0].left   = param->src_crop.left;
        param->ovl_dirty_rect[0].top    = param->src_crop.top;
        param->ovl_dirty_rect[0].right  = param->src_crop.right;
        param->ovl_dirty_rect[0].bottom = param->src_crop.bottom;
        param->compress        = isCompressData(&priv_handle);

        if (DisplayManager::m_profile_level & PROFILE_TRIG)
        {
            char atrace_tag[128];
            sprintf(atrace_tag, "set_ovl: input(%d) queue\n", id);
            HWC_ATRACE_NAME(atrace_tag);

            OLOGI("HWC->OVL: input(%d) queue", id);
        }
    }
}

void OverlayEngine::updateSettingOfInput(const int32_t& id, const int32_t& ext_sel_layer)
{
    AutoMutex l(m_lock);

    if (m_inputs[id]->connected_state == OVL_PORT_ENABLE)
    {
        // get release fence for input queue
        DisplayBufferQueue::DisplayBuffer* buffer = &(m_last_acquired_bufs[id]);

        PrivateHandle priv_handle;
        int32_t err = getPrivateHandleInfo(buffer->out_handle, &priv_handle);
        err |= getPrivateHandleBuff(buffer->out_handle, &priv_handle);
        if (err != NO_ERROR)
        {
            HWC_LOGE("%s: Failed to get PrivateHandle of buffer(%d)!", __func__, err);
        }

        OverlayPrepareParam prepare_param;
        {
            prepare_param.id            = id;
            prepare_param.ion_fd        = buffer->out_ion_fd;
            prepare_param.is_need_flush = 0;

            HWCMediator::getInstance().getOvlDevice(m_disp_id)->prepareOverlayInput(m_disp_id, &prepare_param);

            if (prepare_param.fence_fd <= 0)
            {
                OLOGE("(%d) Failed to get releaseFence for input queue", id);
            }

            if (m_rel_fence_fds[id] != -1)
            {
                ::protectedClose(m_rel_fence_fds[id]);
            }
            m_rel_fence_fds[id] = prepare_param.fence_fd;

            HWC_LOGD("updateSettingOfInput() id:%d ion_fd:%d rel_fence_idx:%d rel_fence_fd:%d",
                prepare_param.id, prepare_param.ion_fd, prepare_param.fence_index, prepare_param.fence_fd);
        }

        // fill struct for enable layer
        OverlayPortParam* param = m_input_params[id];
        param->state          = OVL_IN_PARAM_ENABLE;
        if (buffer->secure)
        {
            param->va         = (void*)(uintptr_t)buffer->out_sec_handle;
            param->mva        = (void*)(uintptr_t)buffer->out_sec_handle;
        }
        else
        {
            param->va         = NULL;
            param->mva        = NULL;
        }
        buffer->ext_sel_layer = ext_sel_layer;
        param->pitch          = buffer->data_pitch;
        param->v_pitch        = priv_handle.vstride;
        param->format         = buffer->data_format;
        param->color_range    = buffer->data_color_range;
        param->dataspace      = buffer->dataspace;
        param->src_crop       = buffer->data_info.src_crop;
        param->dst_crop       = buffer->data_info.dst_crop;
        param->is_sharpen     = buffer->data_info.is_sharpen;
        param->fence_index    = prepare_param.fence_index;
        param->identity       = HWLAYER_ID_DBQ; // todo
        param->connected_type = OVL_INPUT_QUEUE;
        param->protect        = buffer->protect;
        param->secure         = buffer->secure;
        param->alpha_enable   = buffer->alpha_enable;
        param->alpha          = buffer->alpha;
        param->blending       = buffer->blending;
        param->sequence       = buffer->sequence;
        param->dim            = false;
        param->ion_fd         = buffer->out_ion_fd;
        param->is_s3d_layer   = buffer->is_s3d_buffer;
        param->s3d_type       = buffer->s3d_buffer_type;
        param->ext_sel_layer  = buffer->ext_sel_layer;
        param->fence          = buffer->acquire_fence;
        buffer->acquire_fence  = -1;

        // partial update - process mm layer dirty rect roughly
        param->ovl_dirty_rect_cnt = 1;
        param->ovl_dirty_rect[0].left   = param->src_crop.left;
        param->ovl_dirty_rect[0].top    = param->src_crop.top;
        param->ovl_dirty_rect[0].right  = param->src_crop.right;
        param->ovl_dirty_rect[0].bottom = param->src_crop.bottom;
        param->compress = isCompressData(&priv_handle);
        HWC_LOGD("%s hnd:%p compress:%d vs:%d", __func__, buffer->out_handle, param->compress, param->v_pitch);

        if (DisplayManager::m_profile_level & PROFILE_TRIG)
        {
            char atrace_tag[128];
            sprintf(atrace_tag, "set_ovl: input(%d) queue\n", id);
            HWC_ATRACE_NAME(atrace_tag);

            OLOGI("HWC->OVL: input(%d) queue", id);
        }
    }
}

void OverlayEngine::updateInput(int id, sp<GraphicBuffer>& buf, int *releaseFd, int sequence)
{
    PrivateHandle priv_handle;
    int32_t err = getPrivateHandleInfo(buf->handle, &priv_handle);
    err |= getPrivateHandleBuff(buf->handle, &priv_handle);
    if (err != NO_ERROR)
    {
        HWC_LOGE("%s: Failed to get PrivateHandle of buffer(%d)!", __func__, err);
    }

    AutoMutex l(m_lock);

    if (m_inputs[id]->connected_state == OVL_PORT_ENABLE)
    {
        // get release fence for input queue
        OverlayPrepareParam prepare_param;
        {
            prepare_param.id            = id;
            prepare_param.ion_fd        = priv_handle.ion_fd;
            prepare_param.is_need_flush = 0;

            HWCMediator::getInstance().getOvlDevice(m_disp_id)->prepareOverlayInput(m_disp_id, &prepare_param);

            if (prepare_param.fence_fd <= 0)
            {
                OLOGE("(%d) Failed to get releaseFence for input queue", id);
            }
            *releaseFd = prepare_param.fence_fd;
        }

        // fill struct for enable layer
        OverlayPortParam* param = m_input_params[id];
        param->state          = OVL_IN_PARAM_ENABLE;
        if (priv_handle.usage & GRALLOC_USAGE_SECURE)
        {
            SECHAND sec_handle;
            getSecureHwcBuf(buf->handle, &sec_handle);
            param->va         = (void*)(uintptr_t)sec_handle;
            param->mva        = (void*)(uintptr_t)sec_handle;
        }
        else
        {
            param->va         = NULL;
            param->mva        = NULL;
        }
        param->pitch          = buf->getWidth();
        param->v_pitch        = priv_handle.vstride;
        param->format         = priv_handle.format;
        param->color_range    = 0;
        param->src_crop       = Rect(0, 0, buf->getWidth(), buf->getHeight());
        param->dst_crop       = Rect(0, 0, buf->getWidth(), buf->getHeight());
        param->is_sharpen     = false;
        param->fence_index    = prepare_param.fence_index;
        param->identity       = HWLAYER_ID_DBQ;
        param->connected_type = OVL_INPUT_QUEUE;
        param->protect        = (priv_handle.usage & GRALLOC_USAGE_PROTECTED);
        param->secure         = (priv_handle.usage & GRALLOC_USAGE_SECURE);
        param->alpha_enable   = false;
        param->alpha          = 0xff;
        param->blending       = HWC2_BLEND_MODE_NONE;
        param->sequence       = sequence++;
        param->dim            = false;
        param->ion_fd         = priv_handle.ion_fd;
        param->is_s3d_layer   = false;
        param->s3d_type       = 0;
        //TODO: epaper, acquire fence has waited by proxy post consumer, so move it to here?
        //param->fence          = buffer.acquire_fence;
        //buffer.acquire_fence  = -1;

        // partial update - process mm layer dirty rect roughly
        param->ovl_dirty_rect_cnt = 1;
        param->ovl_dirty_rect[0].left   = param->src_crop.left;
        param->ovl_dirty_rect[0].top    = param->src_crop.top;
        param->ovl_dirty_rect[0].right  = param->src_crop.right;
        param->ovl_dirty_rect[0].bottom = param->src_crop.bottom;
        param->compress        = isCompressData(&priv_handle);

        if (DisplayManager::m_profile_level & PROFILE_TRIG)
        {
            char atrace_tag[128];
            sprintf(atrace_tag, "set_ovl: input(%d) queue\n", id);
            HWC_ATRACE_NAME(atrace_tag);

            OLOGD("HWC->OVL: input(%d) queue", id);
        }
    }
}

status_t OverlayEngine::acquireBuffer(DisplayBufferQueue::DisplayBuffer* buffer, int id)
{
    return m_inputs[id]->queue->acquireBuffer(buffer);
}

status_t OverlayEngine::releaseBuffer(DisplayBufferQueue::DisplayBuffer* buffer, int id, int fence)
{
    return m_inputs[id]->queue->releaseBuffer(buffer->index, fence);
}

static char getIdentitySymbol(const int32_t& layer_identity)
{
    char ret = '?';
    switch (layer_identity)
    {
        case HWLAYER_ID_DBQ:
            ret = 'M';
            break;

        case HWC_LAYER_TYPE_UI:
            ret = 'U';
            break;

        case HWC_LAYER_TYPE_FBT:
            ret = 'C';
            break;

        default:
            ret = '?';
            break;
    }
    return ret;
}

void OverlayEngine::dump(String8* dump_str)
{
    AutoMutex l(m_lock);

    if (Platform::getInstance().m_config.dump_buf)
    {
        waitAllFence(m_last_frame_info);
        const int32_t downsample = Platform::getInstance().m_config.dump_buf;
        const int32_t num_layers = m_last_frame_info->overlay_info.num_layers;
        for (int32_t i = 0; i < num_layers; i++)
        {
            OverlayPortParam* layer = m_last_frame_info->overlay_info.input.editItemAt(i);
            if (layer->state)
            {
                char identity = getIdentitySymbol(layer->identity);
                if (Platform::getInstance().m_config.dump_buf_type == 'A' ||
                    Platform::getInstance().m_config.dump_buf_type == identity)
                {
                    String8 path;
                    path.appendFormat("/data/SF_dump/%" PRIu64 "_%d_%c", m_disp_id, i, identity);
                    dump_buf(layer->format, layer->ion_fd, layer->pitch, layer->src_crop, downsample, path.string(),
                        Platform::getInstance().m_config.dump_buf_log_enable);
                }
            }
        }
    }

    int total_size = 0;

    dump_str->appendFormat("\n[HWC Compose State (%" PRIu64 ")]\n", m_disp_id);
    for (int id = 0; id < m_max_inputs; id++)
    {
        if (m_inputs[id]->connected_state == OVL_PORT_ENABLE)
        {
            OverlayPortParam* param = m_input_params[id];

            dump_str->appendFormat("  (%d) f=%#x x=%d y=%d w=%d h=%d -> x=%d y=%d w=%d h=%d\n",
                id, param->format,
                param->src_crop.left, param->src_crop.top,
                param->src_crop.getWidth(), param->src_crop.getHeight(),
                param->dst_crop.left, param->dst_crop.top,
                param->dst_crop.getWidth(), param->dst_crop.getHeight());

            int layer_size = param->dst_crop.getWidth() * param->dst_crop.getHeight() * getBitsPerPixel(param->format) / 8;
            total_size += layer_size;

#ifdef MTK_HWC_PROFILING
            if (HWC_LAYER_TYPE_FBT == param->identity)
            {
                dump_str->appendFormat("  FBT(n=%d, bytes=%d)\n",
                    param->fbt_input_layers, param->fbt_input_bytes + layer_size);
            }
#endif
        }
    }

    dump_str->appendFormat("  Total size: %d bytes\n", total_size);
}

bool OverlayEngine::threadLoop()
{
    sem_wait(&m_event);

    while (1)
    {
        HWC_ATRACE_NAME("OE_thread");
        sp<FrameInfo> frame_info = NULL;
        {
            AutoMutex l(m_lock);

            if (m_stop)
            {
                OLOGD("thread stops");
                break;
            }

            if (m_frame_queue.empty())
            {
                //OLOGD("job is empty");
                break;
            }
            Vector< sp<FrameInfo> >::iterator front(m_frame_queue.begin());
            frame_info = *front;
            m_frame_queue.erase(front);
        }

        FrameOverlayInfo* overlay_info = &frame_info->overlay_info;
        if (frame_info->av_grouping)
        {
            AutoMutex l(m_lock_av_grouping);
            if (!m_ignore_av_grouping)
            {
                m_need_wakeup = true;
                DisplayManager::getInstance().requestNextVSync(m_disp_id);
                const nsecs_t refresh =
                    DisplayManager::getInstance().getDisplayData(m_disp_id, frame_info->active_config)->refresh;
                if (m_cond_threadloop.waitRelative(m_lock_av_grouping, refresh + ms2ns(4)) == TIMED_OUT)
                {
                    OLOGW("timeout to wait vsync to trigger display driver");
                }
            }
        }
        if (!HWCMediator::getInstance().getOvlDevice(m_disp_id)->isFenceWaitSupported())
        {
            waitAllFence(frame_info);
        }
        if (Platform::getInstance().m_config.dump_buf_cont)
        {
            if (HWCMediator::getInstance().getOvlDevice(m_disp_id)->isFenceWaitSupported())
                waitAllFence(frame_info);

            static int32_t cnt = 0;
            const int32_t downsample = Platform::getInstance().m_config.dump_buf_cont;
            const int32_t num_layers = frame_info->overlay_info.num_layers;
            for (int32_t i = 0; i < num_layers; i++)
            {
                OverlayPortParam* layer = frame_info->overlay_info.input.editItemAt(i);
                if (layer->state)
                {
                    char identity = getIdentitySymbol(layer->identity);
                    if (Platform::getInstance().m_config.dump_buf_cont_type == 'A' ||
                        Platform::getInstance().m_config.dump_buf_cont_type == identity)
                    {
                        String8 path;
                        path.appendFormat("/data/SF_dump/%05d_%" PRIu64 "_%d_%c", cnt, m_disp_id, i, identity);
                        dump_buf(layer->format, layer->ion_fd, layer->pitch, layer->src_crop, downsample, path.string(),
                            Platform::getInstance().m_config.dump_buf_log_enable);
                    }
                }
            }
            ++cnt;
        }
        setInputsAndOutput(overlay_info);
        loopHandler(frame_info);
        if (HWCMediator::getInstance().getOvlDevice(m_disp_id)->isFenceWaitSupported())
        {
            // because display driver uses the reference count of fence, it does not need fence fd,
            // and therefore let us close all fence fd in here
            closeAllFenceFd(frame_info);
        }

        m_last_frame_info = frame_info;
    }

    {
        AutoMutex l(m_lock);
        if (m_frame_queue.empty())
        {
            m_state = HWC_THREAD_IDLE;
            m_condition.signal();
        }
    }

    return true;
}

void OverlayEngine::closeOverlayFenceFd(FrameOverlayInfo* info)
{
    for (int i = 0; i < info->num_layers; i++)
    {
        OverlayPortParam* layer = info->input.editItemAt(i);
        if (layer->fence != -1)
        {
            protectedClose(layer->fence);
            layer->fence = -1;
        }
    }

    if (info->enable_output && info->output.fence != -1)
    {
        protectedClose(info->output.fence);
        info->output.fence = -1;
    }
}

void OverlayEngine::closeAllFenceFd(const sp<FrameInfo>& info)
{
    closeOverlayFenceFd(&info->overlay_info);
    if (info->prev_present_fence != -1)
    {
        protectedClose(info->prev_present_fence);
        info->prev_present_fence = -1;
    }
}

void OverlayEngine::waitAllFence(sp<FrameInfo>& info)
{
    char tag[128];
    DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D');
    logger.printf("(%" PRIu64 ") Wait present fence for idx: %d", m_disp_id, info->present_fence_idx);

    waitOverlayFence(&info->overlay_info);

    if (info->prev_present_fence != -1)
    {
        sprintf(tag, "%s-PF", DEBUG_LOG_TAG);
#ifdef FENCE_DEBUG
        HWC_LOGD("+ OverlayEngine::waitPresentFence prev_present_fence:%d", info->prev_present_fence);
#endif
        m_sync_fence->wait(info->prev_present_fence, 1000, tag);
#ifdef FENCE_DEBUG
        HWC_LOGD("- OverlayEngine::waitPresentFence");
#endif
        info->prev_present_fence = -1;
    }
}

void OverlayEngine::waitOverlayFence(FrameOverlayInfo* info)
{
    char tag[128];

    for (int i = 0; i < info->num_layers; i++)
    {
        OverlayPortParam* layer = info->input.editItemAt(i);
        if (layer->fence != -1)
        {
            sprintf(tag, "%s-IN-%d", DEBUG_LOG_TAG, i);
#ifdef FENCE_DEBUG
            HWC_LOGD("+ OverlayEngine::waitOverlayFence() layer->fence:%d", layer->fence);
#endif
            m_sync_fence->wait(layer->fence, 1000, tag);
#ifdef FENCE_DEBUG
            HWC_LOGD("- OverlayEngine::waitOverlayFence()");
#endif
            layer->fence = -1;
        }
    }

    if (info->enable_output && info->output.fence != -1)
    {
        sprintf(tag, "%s-OUT", DEBUG_LOG_TAG);
#ifdef FENCE_DEBUG
        HWC_LOGD("+ OverlayEngine::waitOverlayFence() output.fence:%d", info->output.fence);
#endif
        m_sync_fence->wait(info->output.fence, 1000, tag);
#ifdef FENCE_DEBUG
        HWC_LOGD("- OverlayEngine::waitOverlayFence()");
#endif
        info->output.fence = -1;
    }
}

void OverlayEngine::onFirstRef()
{
    run(m_thread_name, PRIORITY_URGENT_DISPLAY);
}

void OverlayEngine::packageFrameInfo(sp<FrameInfo>& info, const bool& ovl_valid, const int& num_layers,
                                     const int& present_fence_idx, const int& prev_present_fence,
                                     const bool& av_grouping, sp<ColorTransform> color_transform)
{
    DbgLogger logger(DbgLogger::TYPE_HWC_LOG, 'D');
    logger.printf("(%" PRIu64 ") Trigger with idx: %d", m_disp_id, present_fence_idx);

    info->ovlp_layer_num = m_handling_job->layer_info.max_overlap_layer_num;
    info->hrt_weight = m_handling_job->layer_info.hrt_weight;
    info->hrt_idx = m_handling_job->layer_info.hrt_idx;
    info->active_config = m_handling_job->active_config;
    info->present_fence_idx = present_fence_idx;
    info->prev_present_fence = prev_present_fence;
    info->av_grouping = av_grouping;
    logger.printf("/ PF: %d", info->prev_present_fence);

    FrameOverlayInfo* overlay_info = &info->overlay_info;
    overlay_info->ovl_valid = ovl_valid;
    overlay_info->num_layers = num_layers;
    overlay_info->color_transform = color_transform;
    for (int i = 0; i < num_layers; i++)
    {
        OverlayPortParam* layer = overlay_info->input.editItemAt(i);
        memcpy(layer, m_input_params[i], sizeof(OverlayPortParam));
        m_input_params[i]->fence = -1;
        logger.printf("/ %d,s:%d,f_fd:%d", i, layer->state, layer->fence);
    }

    info->is_HBM = (m_handling_job->is_HBM) ? true : false ;

    overlay_info->enable_output = (m_output.connected_state == OVL_PORT_ENABLE)? true : false;
    memcpy(&overlay_info->output, &m_output.param, sizeof(OverlayPortParam));
    m_output.param.fence = -1;
}

void OverlayEngine::setInputsAndOutput(FrameOverlayInfo* info)
{
    // wait untill HW status has been switched to split mode
    if (!info->ovl_valid)
    {
        info->ovl_valid = waitUntilAvailable();
        if (!info->ovl_valid) return;
    }

    // set input parameter to display driver
    if (DisplayManager::m_profile_level & PROFILE_TRIG)
    {
        char atrace_tag[128];
        sprintf(atrace_tag, "set_ovl(%" PRIu64 "): set inputs", m_disp_id);
        HWC_ATRACE_NAME(atrace_tag);
        OLOGV("HWC->OVL: set inputs (max=%d)", info->num_layers);

        HWCMediator::getInstance().getOvlDevice(m_disp_id)->updateOverlayInputs(
            m_disp_id, info->input.array(), info->num_layers, info->color_transform);
    }
    else
    {
        HWCMediator::getInstance().getOvlDevice(m_disp_id)->updateOverlayInputs(
            m_disp_id, info->input.array(), info->num_layers, info->color_transform);
    }

    // set output parameter to display driver
    if (info->enable_output)
    {
        if (DisplayManager::m_profile_level & PROFILE_TRIG)
        {
            char atrace_tag[128];
            sprintf(atrace_tag, "set_ovl(%" PRIu64 "): set output", m_disp_id);
            HWC_ATRACE_NAME(atrace_tag);
            OLOGD("HWC->OVL: set output");

            HWCMediator::getInstance().getOvlDevice(m_disp_id)->enableOverlayOutput(m_disp_id, &info->output);
        }
        else
        {
            HWCMediator::getInstance().getOvlDevice(m_disp_id)->enableOverlayOutput(m_disp_id, &info->output);
        }
    }
    else
    {
        HWCMediator::getInstance().getOvlDevice(m_disp_id)->disableOverlayOutput(m_disp_id);
    }
}

void OverlayEngine::wakeup()
{
    AutoMutex l(m_lock_av_grouping);
    if (m_need_wakeup)
    {
        m_need_wakeup = false;
        m_cond_threadloop.signal();
    }
}

void OverlayEngine::ignoreAvGrouping(const bool& ignore)
{
    AutoMutex l(m_lock_av_grouping);
    m_ignore_av_grouping = ignore;
    if (m_need_wakeup)
    {
        m_need_wakeup = false;
        m_cond_threadloop.signal();
    }
}

void OverlayEngine::onVSync()
{
    {
        AutoMutex l(m_lock);
        if (m_post_processing != NULL)
        {
            m_post_processing->onVSync();
        }
    }
    wakeup();
}

void OverlayEngine::registerVSyncListener(const sp<VSyncListener>& listener)
{
    AutoMutex l(m_lock);
    if (m_post_processing != NULL)
    {
        m_post_processing->registerVSyncListener(listener);
    }
}

FrameOverlayInfo::FrameOverlayInfo()
    : color_transform(nullptr)
{
    int max_inputs = getHwDevice()->getMaxOverlayInputNum();
    for (int i = 0; i < max_inputs; i++)
    {
        OverlayPortParam* tmp = new OverlayPortParam;
        if (tmp == NULL)
        {
            HWC_LOGE("failed to allocate OverlayPortParam %d/%d", i, max_inputs);
            // TODO find a suitable way to check allocate memory
            // This class is created by pool when construct HWC
            // If we can not allocate it when device boot, I think that system is abnormal.
            abort();
        }
        else
        {
            input.add(tmp);
        }
    }

    initData();
}

FrameOverlayInfo::~FrameOverlayInfo()
{
    int size = input.size();
    for (int i = 0; i < size; i++)
    {
        delete input[i];
    }
    input.clear();
}

void FrameOverlayInfo::initData()
{
    ovl_valid = false;
    num_layers = 0;
    enable_output = false;

    int size = input.size();
    for (int i = 0; i < size; i++)
    {
        resetOverlayPortParam(input[i]);
    }
    resetOverlayPortParam(&output);
}

void FrameOverlayInfo::resetOverlayPortParam(OverlayPortParam* param)
{
    memset(param, 0, sizeof(OverlayPortParam));

    param->state = OVL_IN_PARAM_DISABLE;
    param->identity = HWLAYER_ID_NONE;
    param->alpha = 0xff;
    param->ion_fd = -1;
    param->fence = -1;
    param->mir_rel_fence_fd = -1;
}

void FrameInfo::initData()
{
    present_fence_idx = -1;
    ovlp_layer_num = 0;
    prev_present_fence = -1;
    overlay_info.initData();
    av_grouping = false;
    hrt_weight = 0;
    hrt_idx = 0;
    active_config = 0;
}
