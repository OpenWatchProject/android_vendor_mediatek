#ifndef HWC_OVERLAY_H_
#define HWC_OVERLAY_H_

#include <ui/Rect.h>
#include <utils/Singleton.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>

#include <ui/GraphicBuffer.h>

#include <hwc_common/pool.h>

#include "hwc2_defs.h"
#include "queue.h"
#include "post_processing.h"
#include "dispatcher.h"

#define HWLAYER_ID_NONE -1
#define HWLAYER_ID_DBQ  -20

#define POWER_MODE_CHANGED_DO_VALIDATE_NUM 1

using namespace android;

class DisplayBufferQueue;
struct DispatcherJob;
struct HWBuffer;
struct dump_buff;
class SyncFence;
class PostProcessingEngine;

// ---------------------------------------------------------------------------

struct OverlayPrepareParam
{
    OverlayPrepareParam()
        : id(-1)
        , ion_fd(-1)
        , is_need_flush(0)
        , fence_index(0)
        , fence_fd(-1)
        , if_fence_index(0)
        , if_fence_fd(-1)
    { }

    int id;
    int ion_fd;
    unsigned int is_need_flush;

    unsigned int fence_index;
    int fence_fd;
    // in decoupled mode,
    // interface fence of frame N would be released when
    // RDMA starts to read frame (N+1)
    unsigned int if_fence_index;
    int if_fence_fd;
};

enum INPUT_PARAM_STATE
{
    OVL_IN_PARAM_DISABLE = 0,
    OVL_IN_PARAM_ENABLE  = 1,
};

struct OverlayPortParam
{
    OverlayPortParam()
        : state(OVL_IN_PARAM_DISABLE)
        , va(NULL)
        , mva(NULL)
        , pitch(0)
        , v_pitch(0)
        , format(0)
        , color_range(0)
        , is_sharpen(0)
        , fence_index(0)
        , if_fence_index(0)
        , identity(HWLAYER_ID_NONE)
        , connected_type(0)
        , protect(false)
        , secure(false)
        , alpha_enable(0)
        , alpha(0xFF)
        , blending(0)
        , dim(false)
        , sequence(0)
        , ion_fd(-1)
        , mir_rel_fence_fd(-1)
        , is_s3d_layer(false)
        , s3d_type(0)
        , ovl_dirty_rect_cnt(0)
        , ext_sel_layer(-1)
        , fence(-1)
        , layer_color(0)
        , dataspace(HAL_DATASPACE_UNKNOWN)
        , compress(false)
    {
        memset(&(ovl_dirty_rect[0]), 0, sizeof(hwc_rect_t) * MAX_DIRTY_RECT_CNT);
    }

    int state;
    void* va;
    void* mva;
    unsigned int pitch;
    unsigned int v_pitch;
    unsigned int format;
    unsigned int color_range;
    Rect src_crop;
    Rect dst_crop;
    unsigned int is_sharpen;
    unsigned int fence_index;
    unsigned int if_fence_index;
    int identity;
    int connected_type;
    bool protect;
    bool secure;
    unsigned int alpha_enable;
    unsigned char alpha;
    int blending;
    bool dim;
    unsigned int sequence;
#ifdef MTK_HWC_PROFILING
    int fbt_input_layers;
    int fbt_input_bytes;
#endif

    int ion_fd;

    int mir_rel_fence_fd;
    bool is_s3d_layer;
    int s3d_type;

    // dirty rect info
    int ovl_dirty_rect_cnt;
    hwc_rect_t ovl_dirty_rect[MAX_DIRTY_RECT_CNT];

    int ext_sel_layer;
    int fence;

    // solid color
    unsigned int layer_color;

    int dataspace;

    bool compress;
};

struct LayerFence
{
    LayerFence()
        : state(OVL_IN_PARAM_DISABLE)
        , fence(-1)
    { }
    int state;
    int fence;
};

class FrameOverlayInfo
{
public:
    FrameOverlayInfo();
    ~FrameOverlayInfo();
    void initData();

    bool ovl_valid;
    int num_layers;
    Vector<OverlayPortParam*> input;

    bool enable_output;
    OverlayPortParam output;
    sp<ColorTransform> color_transform;
private:
    void resetOverlayPortParam(OverlayPortParam* param);
};

class FrameInfo : public LightPoolBase<FrameInfo>
{
public:
    void initData();

    int present_fence_idx;
    int ovlp_layer_num;
    int prev_present_fence;
    bool av_grouping;
    FrameOverlayInfo overlay_info;
    uint32_t hrt_weight;
    uint32_t hrt_idx;
    hwc2_config_t active_config;
    // for LED HBM (High Backlight Mode) control
    bool is_HBM;
};

// OverlayEngine is used for UILayerComposer and MMLayerComposer
// to config overlay input layers
class OverlayEngine : public HWCThread, public HWCDispatcher::VSyncListener
{
public:
    // prepareInput() is used to preconfig a buffer with specific input port
    status_t prepareInput(OverlayPrepareParam& param);

    // setInputQueue() is used to config a buffer queue with a specific input port
    status_t setInputQueue(int id, sp<DisplayBufferQueue> queue);

    // setInputDirect() is used to set a input port as direct type
    status_t setInputDirect(int id);

    // setInputs() is used to update configs of multiple overlay input layers to driver
    status_t setInputs(int num);

    // disableInput() is used to disable specific input port
    status_t disableInput(int id);

    // disableOutput() is used to disable output port
    //   in regard to performance, the overlay output buffers would not be released.
    //   please use releaseOutputQueue() to release overlay output buffers
    status_t disableOutput();

    // prepareOutput() is used to preconfig a buffer with output port
    status_t prepareOutput(OverlayPrepareParam& param);

    // setOutput() is used to set output port
    status_t setOutput(OverlayPortParam* param, bool mirrored = false);

    // createOutputQueue() is used to allocate overlay output buffers
    status_t createOutputQueue(int format, bool secure);

    // createOutputQueue() is used to release overlay output buffers
    status_t releaseOutputQueue();

    // preparePresentFence() is used to get present fence
    // in order to know when screen is updated
    status_t preparePresentFence(OverlayPrepareParam& param);

    // configMirrorOutput() is used to configure output buffer of mirror source
    //   if virtial display is used as mirror source,
    //   nothing is done and retun with NO_ERROR
    status_t configMirrorOutput(DispatcherJob* job);

    // setOverlaySessionMode() sets the overlay session mode
    status_t setOverlaySessionMode(HWC_DISP_MODE mode);

    // setOverlaySessionMode() gets the overlay session mode
    HWC_DISP_MODE getOverlaySessionMode();

    // trigger() is used to nofity engine to start doing composition
    void trigger(const bool& ovl_valid, const int& num_layers, const int& present_fence_idx,
                 const int& prev_present_fence, const bool& av_grouping, sp<ColorTransform> color_transform,
                 const bool& do_nothing = false);

    // stop() is used to stop engine to process job
    void stop();

    // getInputParams() is used for client to get input params for configuration
    OverlayPortParam* const* getInputParams();

    void setPowerMode(int mode);

    // getInputQueue() returns the buffer queue of a specific input port
    sp<DisplayBufferQueue> getInputQueue(int id) const;

    // getMaxInputNum() is used for getting max amount of overlay inputs
    int getMaxInputNum() { return m_max_inputs; }

    // getAvailableInputNum() is used for
    // getting current available overlay inputs
    int getAvailableInputNum();

    // waitUntilAvailable() is used for waiting until OVL resource is available
    //   during overlay session mode transition,
    //   it is possible for HWC getting transition result from display driver;
    //   this function is used to wait untill
    //   the overlay session mode transition is finisned
    bool waitUntilAvailable();

    // flip() is used to notify OverlayEngine to do its job after notify framework
    void flip();

    // dump() is used to dump each input data to OverlayEngine
    void dump(String8* dump_str);

    // wakeup av grouping job
    void wakeup();

    // wakeup av grouping job and ignore this request
    void ignoreAvGrouping(const bool& ignore);

    bool isEnable()
    {
        AutoMutex l(m_lock);
        return (m_engine_state == OVL_ENGINE_ENABLED);
    }

    inline void setHandlingJob(DispatcherJob* job) { m_handling_job = job; }

    OverlayEngine(const uint64_t& dpy);
    ~OverlayEngine();

    // this function should be used by PostProcessingEngine
    status_t acquireBuffer(DisplayBufferQueue::DisplayBuffer* buffer, int id);

    // this function should be used by PostProcessingEngine
    status_t releaseBuffer(DisplayBufferQueue::DisplayBuffer* buffer, int id, int fence = -1);

    // updateInput() would notify OverlayEngine to consume new queued buffer
    void updateInput(int id);

    // updateInput() would notify OverlayEngine to consume new queued buffer
    void updateSettingOfInput(const int32_t& id, const int32_t& ext_sel_layer);

    // PostProcessingEngine can use this function to show a GB, so the original buffer of DBQ
    // does not be show after call this function
    void updateInput(int id, sp<GraphicBuffer>& buf, int *releaseFd, int sequence);

    void setPostProcessingEngine(sp<PostProcessingEngine> ppe);

    void removePostProcessingEngine();

    void onVSync(void);

    // PostProcessingEngine can send fake vsync to these listener
    void registerVSyncListener(const sp<VSyncListener>& listener);

    // For skip validate
    void decPowerModeChanged()
    {
        AutoMutex l(m_lock);
        if (m_power_mode_changed > 0)
            --m_power_mode_changed;
    }

    int getPowerModeChanged() const
    {
        AutoMutex l(m_lock);
        return m_power_mode_changed;
    }

private:
    enum INPUT_TYPE
    {
        OVL_INPUT_NONE    = 0,
        OVL_INPUT_UNKNOWN = 1,
        OVL_INPUT_DIRECT  = 2,
        OVL_INPUT_QUEUE   = 3,
    };

    enum PORT_STATE
    {
        OVL_PORT_DISABLE = 0,
        OVL_PORT_ENABLE  = 1,
    };

    struct OverlayInput
    {
        OverlayInput();

        // connected_state points the input port usage state
        int connected_state;

        // connected_type is connecting type of input port
        int connected_type;

        // param is used for configure input parameters
        OverlayPortParam param;

        // queue is used to acquire and release buffer from client
        sp<DisplayBufferQueue> queue;
    };

    struct OverlayOutput
    {
        OverlayOutput();

        // connected_state points the output port usage state
        int connected_state;

        // param is used for configure output parameters
        OverlayPortParam param;

        // queue is used to acquire and release buffer from client
        sp<DisplayBufferQueue> queue;
    };

    // createOutputQueueLocked() is used to preallocate overlay output buffers
    status_t createOutputQueueLocked(int format, bool secure);

    // disableInputLocked is used to clear status of m_inputs
    void disableInputLocked(int id);

    // disableOutputLocked is used to clear status of m_output
    void disableOutputLocked();

    // threadLoop() is used to process related job, e.g. wait fence and trigger overlay
    virtual bool threadLoop();

    // loopHandler() is used to trigger overlay
    status_t loopHandler(sp<FrameInfo>& info);

    // waitAllFence() is used to wait layer fence, present fence and output buffer fence
    void waitAllFence(sp<FrameInfo>& info);

    // waitOverlayFence is used to wait layer fence and output buffer fence
    void waitOverlayFence(FrameOverlayInfo* info);

    // closeOverlayFenceFd is used to close input and output fence
    void closeOverlayFenceFd(FrameOverlayInfo* info);

    // closeAllFenceFd is used to close input, output and present fence
    void closeAllFenceFd(const sp<FrameInfo>& info);

    // onFirstRef() is used to init OverlayEngine thread
    virtual void onFirstRef();

    // packageFrameInfo() is used to backup the overlay info for queued frame
    void packageFrameInfo(sp<FrameInfo>& info, const bool& ovl_valid, const int& num_layers,
                          const int& present_fence_idx, const int& prev_present_fence,
                          const bool& av_grouping, sp<ColorTransform> color_transform);

    // setInputsAndOutput() is used to update configs of input layers and output buffer to driver
    void setInputsAndOutput(FrameOverlayInfo* info);

    // m_condition is used to wait (block) for a certain condition to become true
    mutable Condition m_cond;

    // m_disp_id is display id
    uint64_t m_disp_id;

    enum
    {
        OVL_ENGINE_DISABLED = 0,
        OVL_ENGINE_ENABLED  = 1,
        OVL_ENGINE_PAUSED   = 2,
    };
    // m_state used to verify if OverlayEngine could be use
    int m_engine_state;

    // m_max_inputs is overlay engine max amount of input layer
    // it is platform dependent
    int m_max_inputs;

    // m_inputs is input information array
    // it needs to be initialized with runtime information
    Vector<OverlayInput*> m_inputs;

    // m_input_params is an array which
    // points to all input configurations
    // which would be set to display driver
    Vector<OverlayPortParam*> m_input_params;

    OverlayOutput m_output;

    DispatcherJob* m_handling_job;

    // m_sync_fence is used to wait fence
    sp<SyncFence> m_sync_fence;

    // m_stop is used to stop OverlayEngine thread
    bool m_stop;

    // m_frame_queue is an array which store the parameters of queued frame
    Vector< sp<FrameInfo> > m_frame_queue;

    // m_pool keep all pointer of FrameInfo and maintain their life cycle
    ObjectPool<FrameInfo>* m_pool;

    // m_post_processing use to processing the input buffers
    sp<PostProcessingEngine> m_post_processing;

    // m_lock_av_grouping is used to protect the parameter of av grouping
    mutable Mutex m_lock_av_grouping;

    // m_cond_threadloop is used to control trigger timeing when need av grouping
    Condition m_cond_threadloop;

    // m_need_wakeup is used to tag overlay engine whether it need wakeup or not
    bool m_need_wakeup;

    // when m_ignore_av_grouping is true, ignore all av grouping require
    bool m_ignore_av_grouping;

    // keep the last frameinfo for layer dump
    sp<FrameInfo> m_last_frame_info;

    std::vector<int32_t> m_rel_fence_fds;
    std::vector<DisplayBufferQueue::DisplayBuffer> m_last_acquired_bufs;

    // For skip validate
    int m_prev_power_mode;
    int m_power_mode_changed;
};

#endif // HWC_OVERLAY_H_
