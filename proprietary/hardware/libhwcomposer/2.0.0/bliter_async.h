#ifndef HWC_BLITER_ASYNC_H_
#define HWC_BLITER_ASYNC_H_

#include "DpAsyncBlitStream2.h"

#include <utils/KeyedVector.h>
#include <utils/threads.h>

#include <ui/Rect.h>
#include "composer.h"
#include "dispatcher.h"

using namespace android;

struct HWLayer;
struct PrivateHandle;
class DisplayBufferQueue;
class BliterNode;

// ---------------------------------------------------------------------------

class AsyncBliterHandler : public LayerHandler
{
public:
    struct BufferConfig
    {
        // identify if this config is valid
        bool is_valid;

        // check if need to do interlacing
        bool deinterlace;

        // buffer information in native handle
        unsigned int gralloc_width;
        unsigned int gralloc_height;
        unsigned int gralloc_format;
        unsigned int gralloc_stride;
        unsigned int gralloc_vertical_stride;
        unsigned int gralloc_cbcr_align;
        unsigned int gralloc_color_range;
        int gralloc_private_format;
        int gralloc_ufo_align_type;
        uint32_t gralloc_prexform;

        // src buffer setting for DpFramework
        DP_COLOR_ENUM src_dpformat;
        unsigned int  src_pitch;
        unsigned int  src_pitch_uv;
        unsigned int  src_width;
        unsigned int  src_height;
        unsigned int  src_plane;
        unsigned int  src_size[3];
        int32_t  src_dataspace;

        // dst buffer setting for DpFramework
        DP_COLOR_ENUM dst_dpformat;
        unsigned int  dst_width;
        unsigned int  dst_height;
        unsigned int  dst_pitch;
        unsigned int  dst_pitch_uv;
        unsigned int  dst_plane;
        unsigned int  dst_size[3];
        unsigned int  dst_ovl_id;
        int32_t  dst_dataspace;

        int32_t assigned_output_format;
    };

    AsyncBliterHandler(int dpy, const sp<OverlayEngine>& ovl_engine);
    virtual ~AsyncBliterHandler();

    // set() in AsyncBliterHandler is used to create release fence
    // in order to notify upper producers that layers are already consumed
    virtual void set(const sp<HWCDisplay>& display, DispatcherJob* job);

    // process() in AsyncBliterHandler is used to utilize DpFramework
    // to rescale and rotate input layers and pass the results to display driver
    virtual void process(DispatcherJob* job);

    // release fence by specified MDP job ID
    // This is useful when a mirror job is dropped, and
    // the blit processing needs to be skipped.
    // If the job_id is 0, it will release all fence of MDP job_id
    //
    // NOTE: Although AsyncBliterHandler is inherited from LayerHandler
    // LayerHandler does NOT have this function
    void nullop();
    void nullop(const int32_t& job_id);

    virtual void boost();

    // dump() in AsyncBliterHandler is used to dump debug data
    virtual int dump(char* buff, int buff_len, int dump_level);

    // cancelLayers in AsyncBliterHandler is used to cancel mm layers of job
    virtual void cancelLayers(DispatcherJob* job);

private:
    // setMirror() is used to create release fence for mirror source buffer
    // in order to notify previous engine know if it is consumed
    void setMirror(const sp<HWCDisplay>& display, DispatcherJob* job);

    // setBlack() is used to create mdp job for fill black content
    void setBlack(const sp<HWCDisplay>& display, DispatcherJob* job);

    // cancelMirror() is used to cancel mirror dst buffer of dropped job
    void cancelMirror(DispatcherJob* job);

    void calculateMirRoiXform(uint32_t* xform, Rect* src_roi, Rect* dst_roi, DispatcherJob* job);

    // processPhyMirror() is used to utilize DpFramework
    // to rescale and rotate mirror source buffer
    // in order for post engine to do composition
    void processPhyMirror(DispatcherJob* job);

    // processPhyMirror() is used to utilize DpFramework
    // to rescale and rotate mirror source buffer
    // in order for codec to video encoding
    void processVirMirror(DispatcherJob* job);

    // processVirBlack() is used to utilize DpFramework
    // to fill black content into virtual output buffer
    void processVirBlack(DispatcherJob* job);

    // bypassBlit() is used to check if source buffer is dirty, or
    // there has any updated source buffer could be used
    bool bypassBlit(HWLayer* hw_layer, int ovl_in);

    // getDisplayBufferQueue() is used to get workable display buffer queue
    sp<DisplayBufferQueue> getDisplayBufferQueue(PrivateHandle* priv_handle, BufferConfig* config,
        int ovl_in, const DisplayData* disp_data, const int& assigned_output_format = -1) const;

    // setDpConfig() is used to prepare configuration for DpFramwork
    status_t setDpConfig(PrivateHandle* priv_handle, BufferConfig* config, int ovl_in, const int& assigned_format = -1);

    // setDstDpConfig() is used to prepare destination configuration for DpFramwork
    status_t setDstDpConfig(PrivateHandle& dst_priv_handle, BufferConfig* config);

    struct InvalidateParam
    {
        int src_acq_fence_fd;

        buffer_handle_t src_handle;

        PrivateHandle* src_priv_handle;

        uint32_t transform;

        DpRect src_dp_roi;

        DpRect dst_dp_roi;

        Rect dst_crop;

        Rect src_base;

        Rect dst_base;

        int dst_rel_fence_fd;

        int dst_ion_fd;

        SECHAND dst_sec_handle;

        // used to profile latency
        bool is_mirror;

        unsigned int sequence;

        bool is_enhance;
    };

    // processFillBlack() is used to clear destination buffer by scaling a small black buffer
    void processFillBlack(PrivateHandle* priv_handle, int* fence, MdpJob &job);

    // processFillBlack() is used to clear destination buffer by scaling a small black buffer
    void processFillWhite(PrivateHandle* priv_handle, int* fence, MdpJob &job);

    // fillBkack() is used to clear destination buffer including secure buffer
    void fillBlack(buffer_handle_t handle, PrivateHandle* priv_handle, int* fence, MdpJob* job);

    // fillBkack() is used to clear destination buffer including secure buffer
    void fillWhite(buffer_handle_t handle, PrivateHandle* priv_handle, int* fence, MdpJob* job);

    // clearBackground() is used to check the orientation and clear buffer if needed
    void clearBackground(buffer_handle_t handle, const Rect* current_dst_roi, int* fence, MdpJob* job);

    // clearMdpJob is used to close unused fence of fill black job
    void clearMdpJob(MdpJob* job);

    // m_dp_configs stores src/dst buffer and overlay input information
    BufferConfig* m_dp_configs;

    // m_blit_stream is a bit blit stream
    //DpBlitStream m_blit_stream;
    DpAsyncBlitStream2 m_blit_stream;

    BliterNode* m_bliter_node;
};

#endif // HWC_BLITER_ASYNC_H_
