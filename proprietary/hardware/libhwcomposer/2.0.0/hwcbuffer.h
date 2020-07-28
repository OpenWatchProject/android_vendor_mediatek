#ifndef HWC_HWCBUFFER_H
#define HWC_HWCBUFFER_H

#include <utils/RefBase.h>
#include "utils/tools.h"

class HWCBuffer : public android::LightRefBase<HWCBuffer>
{
public:
    HWCBuffer(const uint64_t& disp_id, const int32_t& layer_id, const bool& is_ct):
        m_hnd(nullptr),
        m_original_hnd(nullptr),
        m_prev_original_hnd(nullptr),
        m_original_hnd_alloc_id(0),
        m_prev_original_hnd_alloc_id(0),
        m_release_fence_fd(-1),
        m_prev_release_fence_fd(-1),
        m_acquire_fence_fd(-1),
        m_is_ct(is_ct),
        m_disp_id(disp_id),
        m_layer_id(layer_id),
        m_buffer_changed(false),
        m_prexform_changed(false)
    { }

    ~HWCBuffer();

    void setHandle(const buffer_handle_t& hnd)
    {
        int32_t err = 0;
        if (hnd != nullptr)
            err = gralloc_extra_query(hnd, GRALLOC_EXTRA_GET_ID, &m_priv_hnd.alloc_id);

        if (err)
            HWC_LOGE("%s err(%x), (handle=%p)", __func__, err, hnd);

        if (getPrevOriginalHandle() != hnd ||
            m_prev_original_hnd_alloc_id != m_priv_hnd.alloc_id || err)
        {
            setBufferChanged(true);
        }

        m_original_hnd = hnd;
        m_original_hnd_alloc_id = m_priv_hnd.alloc_id;
        if (m_hnd != nullptr)
        {
            freeDuppedBufferHandle(m_hnd);
            m_hnd = nullptr;
        }
        if (hnd != nullptr)
        {
            dupBufferHandle(hnd, &m_hnd);
        }
        else
        {
            m_hnd = nullptr;
        }
    }

    buffer_handle_t getPrevOriginalHandle() const { return m_prev_original_hnd; }
    void recordPrevOriginalInfo()
    {
        m_prev_original_hnd = m_original_hnd;
        m_prev_original_hnd_alloc_id = m_original_hnd_alloc_id;
    }

    buffer_handle_t getHandle() const { return m_hnd; }

    void setReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected);
    int32_t getReleaseFenceFd() { return m_release_fence_fd; }

    void setPrevReleaseFenceFd(const int32_t& fence_fd, const bool& is_disp_connected);
    int32_t getPrevReleaseFenceFd() { return m_prev_release_fence_fd; }

    void setAcquireFenceFd(const int32_t& fence_fd, const bool& is_disp_connected);
    int32_t getAcquireFenceFd() { return m_acquire_fence_fd; }

    const PrivateHandle& getPrivateHandle() { return m_priv_hnd; }
    PrivateHandle& getEditablePrivateHandle() { return m_priv_hnd; }

    int32_t afterPresent(const bool& is_disp_connected, const bool& is_ct = false);

    void setBufferChanged(const bool& buf_changed) { m_buffer_changed = buf_changed; }
    bool isBufferChanged() const { return m_buffer_changed; }

    bool isPrexformChanged() const { return m_prexform_changed; }

    void setupPrivateHandle()
    {
        if (m_hnd != nullptr)
        {
            uint32_t prevPrexForm = m_priv_hnd.prexform;
            // todo: should not pass nullptr to getPrivateHandle
            (m_is_ct)? getPrivateHandleFBT(m_hnd, &m_priv_hnd) : (::getPrivateHandle(m_hnd, &m_priv_hnd));

            if (prevPrexForm != m_priv_hnd.prexform)
            {
                m_prexform_changed = true;
            }
            else
            {
                m_prexform_changed = false;
            }
        }
        else
        {
            // it's a dim layer?
            m_priv_hnd.format = HAL_PIXEL_FORMAT_RGBA_8888;
        }
    }
private:
    buffer_handle_t m_hnd;
    buffer_handle_t m_original_hnd;
    buffer_handle_t m_prev_original_hnd;
    uint64_t m_original_hnd_alloc_id;
    uint64_t m_prev_original_hnd_alloc_id;
    int32_t m_release_fence_fd;
    int32_t m_prev_release_fence_fd;
    int32_t m_acquire_fence_fd;
    bool m_is_ct;
    PrivateHandle m_priv_hnd;
    uint64_t m_disp_id;
    int32_t m_layer_id;
    bool m_buffer_changed;
    bool m_prexform_changed;
};

#endif
