#define DEBUG_LOG_TAG "HWCBuffer"
#include "hwcbuffer.h"

HWCBuffer::~HWCBuffer()
{
    if (getReleaseFenceFd() != -1)
        protectedClose(getReleaseFenceFd());

    if (getPrevReleaseFenceFd() != -1)
        protectedClose(getPrevReleaseFenceFd());

    if (m_hnd != nullptr)
        freeDuppedBufferHandle(m_hnd);
}

void HWCBuffer::setReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected)
{
    if (fence_fd >= 0 && m_release_fence_fd != -1)
    {
        if (is_disp_connected)
        {
            HWC_LOGE("(%" PRIu64 ") fdleak detect: %s layer_id:%d release_fence_fd:%d hnd:%p",
                m_disp_id, __func__, m_layer_id, m_release_fence_fd, m_hnd);
            AbortMessager::getInstance().abort();
        }
        else
        {
            HWC_LOGW("(%" PRIu64 ") fdleak detect: %s layer_id:%d release_fence_fd:%d hnd:%p",
                m_disp_id, __func__, m_layer_id, m_release_fence_fd, m_hnd);
            ::protectedClose(m_release_fence_fd);
            m_release_fence_fd = -1;
        }
    }
    m_release_fence_fd = fence_fd;
    HWC_LOGV("(%" PRIu64 ") fdleak detect: %s layer id:%d release_fence_fd:%d hnd:%p",
         m_disp_id, __func__, m_layer_id, m_release_fence_fd, m_hnd);
}

void HWCBuffer::setPrevReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected)
{
    if (fence_fd >= 0 && m_prev_release_fence_fd != -1)
    {
        if (is_disp_connected)
        {
            HWC_LOGE("(%" PRIu64 ") fdleak detect: %s layer id:%d prev_release_fence_fd:%d hnd:%p",
                m_disp_id, __func__, m_layer_id, m_prev_release_fence_fd, m_hnd);
            AbortMessager::getInstance().abort();
        }
        else
        {
            HWC_LOGE("(%" PRIu64 ") fdleak detect: %s layer id:%d prev_release_fence_fd:%d hnd:%p",
                m_disp_id, __func__, m_layer_id, m_prev_release_fence_fd, m_hnd);
            ::protectedClose(m_prev_release_fence_fd);
            m_prev_release_fence_fd = -1;
        }
    }
    m_prev_release_fence_fd = fence_fd;
    HWC_LOGV("(%" PRIu64 ") fdleak detect: %s layer id:%d prev_release_fence_fd:%d hnd:%p",
         m_disp_id, __func__, m_layer_id, m_prev_release_fence_fd, m_hnd);
}

void HWCBuffer::setAcquireFenceFd(const int32_t& fence_fd, const bool& is_disp_connected)
{
    if (fence_fd >= 0 && m_acquire_fence_fd != -1)
    {
        if (is_disp_connected)
        {
            HWC_LOGE("(%" PRIu64 ") fdleak detect: %s layer id:%d acquire_fence_fd:%d hnd:%p",
                m_disp_id, __func__, m_layer_id, m_acquire_fence_fd, m_hnd);
            AbortMessager::getInstance().abort();
        }
        else
        {
            HWC_LOGE("(%" PRIu64 ") fdleak detect: %s layer id:%d acquire_fence_fd:%d hnd:%p",
                m_disp_id, __func__, m_layer_id, m_acquire_fence_fd, m_hnd);
            ::protectedClose(m_acquire_fence_fd);
            m_acquire_fence_fd = -1;
        }
    }

    m_acquire_fence_fd = fence_fd;
    HWC_LOGV("(%" PRIu64 ") fdleak detect: %s layer id:%d acquire_fence_fd:%d hnd:%p",
        m_disp_id, __func__, m_layer_id, m_acquire_fence_fd, m_hnd);
}

int32_t HWCBuffer::afterPresent(const bool& is_disp_connected, const bool& /*is_ct*/)
{
    if (getAcquireFenceFd() > -1)
    {
        if (is_disp_connected)
        {
            HWC_LOGE("(%" PRIu64 ") error: layer(%d) has fd(%d)", m_disp_id, m_layer_id, m_acquire_fence_fd);
            return 1;
        }
        else
        {
            HWC_LOGE("(%" PRIu64 ") error: layer(%d) has fd(%d)", m_disp_id, m_layer_id, m_acquire_fence_fd);
            ::protectedClose(getAcquireFenceFd());
            setAcquireFenceFd(-1, is_disp_connected);
        }
    }

    recordPrevOriginalInfo();
    setBufferChanged(false);
    return 0;
}
