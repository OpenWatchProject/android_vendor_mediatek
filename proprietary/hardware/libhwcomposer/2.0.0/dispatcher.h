#ifndef HWC_DISPATCHER_H_
#define HWC_DISPATCHER_H_

#include <vector>
#include <utils/threads.h>
#include <utils/SortedVector.h>
#include <utils/BitSet.h>
#include <utils/Singleton.h>

#include "hwc_priv.h"

#include "utils/tools.h"

#include "display.h"
#include "worker.h"
#include "composer.h"
#include "sync.h"
#include "dev_interface.h"
#include "color.h"

using namespace android;

struct dump_buff;
class SyncFence;
class DispatchThread;
class HWCDisplay;
class OverlayEngine;

// ---------------------------------------------------------------------------

#define FILL_BLACK_JOB_SIZE 2

// HWLayer::type values
enum {
    HWC_LAYER_TYPE_INVALID      = 0,
    HWC_LAYER_TYPE_FBT          = 1,
    HWC_LAYER_TYPE_UI           = 2,
    HWC_LAYER_TYPE_MM           = 3,
    HWC_LAYER_TYPE_DIM          = 4,
    HWC_LAYER_TYPE_CURSOR       = 5,
    HWC_LAYER_TYPE_MM_FBT       = 7,
    HWC_LAYER_TYPE_NONE         = 8,
    HWC_LAYER_TYPE_WORMHOLE     = 9,
    HWC_LAYER_TYPE_UIPQ_DEBUG   = 10,
    HWC_LAYER_TYPE_UIPQ         = 11,
    HWC_LAYER_TYPE_HBM          = 12,
    HWC_LAYER_TYPE_AOD          = 13,
};

inline const char* getHWLayerString(const int32_t& hwlayer_type)
{
    switch(hwlayer_type)
    {
        case HWC_LAYER_TYPE_INVALID:
            return "INV";

        case HWC_LAYER_TYPE_FBT:
            return "FBT";

        case HWC_LAYER_TYPE_UI:
            return "UI";

        case HWC_LAYER_TYPE_MM:
            return "MM";

        case HWC_LAYER_TYPE_DIM:
            return "DIM";

        case HWC_LAYER_TYPE_CURSOR:
            return "CUR";

        case HWC_LAYER_TYPE_MM_FBT:
            return "MMF";

        case HWC_LAYER_TYPE_NONE:
            return "NON";

        case HWC_LAYER_TYPE_WORMHOLE:
            return "HOLE";
        case HWC_LAYER_TYPE_UIPQ_DEBUG:
            return "DEBUG";

        case HWC_LAYER_TYPE_UIPQ:
            return "UIPQ";

        case HWC_LAYER_TYPE_HBM:
            return "HBM";

        case HWC_LAYER_TYPE_AOD:
            return "AOD";

        default:
            HWC_LOGE("unknown getHwlayerType:%d", hwlayer_type);
            return "UNKNOWN";
    }
}

// HWLayer::dirty values
enum {
    HWC_LAYER_DIRTY_NONE   = 0x0,
    HWC_LAYER_DIRTY_BUFFER = 0x1,
    HWC_LAYER_DIRTY_PARAM  = 0x2,
    HWC_LAYER_DIRTY_PORT   = 0x4,
    HWC_LAYER_DIRTY_CAMERA = 0x8,
};

// DispatcherJob::post_state values
enum HWC_POST_STATE
{
    HWC_POST_INVALID        = 0x0000,
    HWC_POST_OUTBUF_DISABLE = 0x0010,
    HWC_POST_OUTBUF_ENABLE  = 0x0011,
    HWC_POST_INPUT_NOTDIRTY = 0x0100,
    HWC_POST_INPUT_DIRTY    = 0x0101,
    HWC_POST_MIRROR         = 0x1001,

    HWC_POST_CONTINUE_MASK  = 0x0001,
};

enum {
    HWC_MIRROR_SOURCE_INVALID = -1,
};

enum {
    HWC_SEQUENCE_INVALID = 0,
};

// to store information about cache layer is hit or not.
// These infomation are usefull in cache algorithm.
struct HitLayerInfo
{
    HitLayerInfo()
        : is_hit(false)
        , hit_layer_head(-1)
        , hit_layer_tail(-1)
    { }

    // Current layer list will apply cache algorithm.
    bool is_hit;

    // The first layer is applied cache algorithm.
    // HWC knows a range of layer list appling cache algorithm between head and tail,
    // and it needs to release fences of these layers.
    int hit_layer_head;

    // The last layer is applied cache algorithm.
    int hit_layer_tail;
};

// To store the information abort hrt
// These information will pass to display driver
struct HrtLayerConfig
{
    HrtLayerConfig()
        : ovl_id(0)
        , ext_sel_layer(-1)
    { }

    uint32_t ovl_id;

    int ext_sel_layer;
};

// to store information about cache layer is hit or not.
// These infomation are usefull in cache algorithm.
struct LayerListInfo
{
    LayerListInfo()
        : disp_dirty(true)
        , hwc_gles_head(-1)
        , hwc_gles_tail(-1)
        , gles_head(-1)
        , gles_tail(-1)
        , max_overlap_layer_num(-1)
        , max_overlap_layer_num_w(-1)
        , layer_info_list(NULL)
        , hrt_config_list(NULL)
        , hrt_weight(0)
        , hrt_idx(0)
    { }

    bool disp_dirty;

    // GLES composition range judged by HWC
    int hwc_gles_head;
    int hwc_gles_tail;

    // gles layer start
    int gles_head;

    // gles layer end
    int gles_tail;

    // overlapping layer num for HRT
    int max_overlap_layer_num;

    // weight value of overlapping layer num for HRT
    int max_overlap_layer_num_w;

    // store the query result of layer
    // size of LayerInfo* is equal to size of layer list
    LayerInfo* layer_info_list;

    // store the query info of layer from driver
    // size of HrtLayerConfig* is equal to size of layer list
    HrtLayerConfig* hrt_config_list;

    // for intraframe DVFS
    uint32_t hrt_weight;

    // for intraframe DVFS
    uint32_t hrt_idx;
};

// HWLayer is used to store information of layers selected
// to be processed by hardware composer
struct HWLayer
{
    // used to determine if this layer is enabled
    bool enable;

    // hwc_layer index in hwc_display_contents_1
    int index;

    // identify if layer should be handled by UI or MM path
    int type;

    // identify if layer has dirty pixels
    bool dirty;

    union
    {
        // information of UI layer
        struct
        {
#ifdef MTK_HWC_PROFILING
            // amount of layers handled by GLES
            int fbt_input_layers;

            // bytes of layers handled by GLES
            int fbt_input_bytes;
#endif

            // used by fbt of fake display
            // TODO: add implementation of this debug feature
            int fake_ovl_id;
        };

        // information of MM layer
        struct
        {
            // used to queue back to display buffer queue at MM thread processing
            // TODO: used by where?
            int disp_fence;

            // used to identify queued frame
            int sync_marker;

            // used to judge the job id for async mdp
            uint32_t mdp_job_id;

            // used to judge if a ultra mdp layer
            bool is_ultra_mdp;

            // used to specify the destination ROI written by MDP
            Rect mdp_dst_roi;
        };
    };

    // index of release fence from display driver
    unsigned int fence_index;

    // smart layer id, this number is queryed from driver
    // need to pass this id when hwc set layeys
    int ext_sel_layer;

    // hwc_layer_1 from SurfaceFlinger
    hwc_layer_1 layer;

    hwc_rect_t surface_damage_rect[MAX_DIRTY_RECT_CNT];

    // private handle information
    PrivateHandle priv_handle;

    // layer id from HWCLayer
    int64_t hwc2_layer_id;

    int32_t layer_caps;

    // solid color
    uint32_t layer_color;

    int32_t dataspace;

    bool need_pq;
};

// HWBuffer is used to store buffer information of
// 1. virtual display
// 2. mirror source
struct HWBuffer
{
    HWBuffer()
        : handle(NULL)
    {
        // fd used for phyical display
        phy_present_fence_fd = -1;

        // fd used by virtual display
        out_acquire_fence_fd = -1;
        out_retire_fence_fd = -1;

        // fd used by mirror producer (overlay engine)
        mir_out_rel_fence_fd = -1;
        mir_out_acq_fence_fd = -1;
        mir_out_if_fence_fd = -1;

        // fd used by mirror consumer (DpFramework)
        mir_in_acq_fence_fd = -1;
        mir_in_rel_fence_fd = -1;

        phy_present_fence_idx = 0;
        out_retire_fence_idx = 0;
        mir_out_sec_handle = 0;
        mir_out_acq_fence_idx = 0;
        mir_out_if_fence_idx = 0;
        mir_in_sync_marker = 0;

        dataspace = 0;
    }

    // struct used for phyical display
    // phy_present_fence_fd is used for present fence to notify hw vsync
    int phy_present_fence_fd;

    // phy_present_fence_idx is present fence index from display driver
    unsigned int phy_present_fence_idx;

    // struct used by virtual display
    // out_acq_fence_fd is used for hwc to know if outbuf could be read
    int out_acquire_fence_fd;

    // out_retire_fence_fd is used to notify producer
    // if output buffer is ready to be written
    int out_retire_fence_fd;

    // out_retire_fence_idx is retire fence index from display driver
    unsigned int out_retire_fence_idx;


    // struct used by mirror producer (overlay engine)
    // mir_out_sec_handle is secure handle for mirror output if needed
    SECHAND mir_out_sec_handle;

    // mir_out_rel_fence_fd is fence to notify
    // all producer display in previous round are finished
    // and mirror buffer is ready to be written
    int mir_out_rel_fence_fd;

    // mir_out_acq_fence_fd is fence to notify
    // when mirror buffer's contents are available
    int mir_out_acq_fence_fd;

    // mir_out_acq_fence_idx is fence index from display driver
    unsigned int mir_out_acq_fence_idx;

    // mir_out_if_fence_fd is fence to notify
    // all producer display in previous round are finished
    // and mirror buffer is ready to be written
    int mir_out_if_fence_fd;

    // mir_out_if_fence_idx is fence index from display driver (decouple mode)
    unsigned int mir_out_if_fence_idx;

    // struct used by mirror consumer (DpFramework)
    // mir_in_acq_fence_fd is used to notify
    // when mirror buffer's contents are available
    int mir_in_acq_fence_fd;

    // mir_in_rel_fence_fd is used to notify
    // mirror source that buffer is ready to be written
    int mir_in_rel_fence_fd;

    // mir_in_sync_marker is used for identifying timeline marker
    int mir_in_sync_marker;

    // outbuf native handle
    buffer_handle_t handle;

    // private handle information
    PrivateHandle priv_handle;

    int32_t dataspace;
};

struct MdpJob
{
    MdpJob()
        : id(0)
        , fence(-1)
    { }
    uint32_t id;
    int fence;
};

// DispatcherJob is a display job unit and is used by DispatchThread
struct DispatcherJob
{
    DispatcherJob()
        : enable(false)
        , protect(false)
        , secure(false)
        , mirrored(false)
        , need_output_buffer(false)
        , dropped(false)
        , disp_ori_id(0)
        , disp_mir_id(HWC_MIRROR_SOURCE_INVALID)
        , disp_ori_rot(0)
        , disp_mir_rot(0)
        , num_layers(0)
        , ovl_valid(false)
        , fbt_exist(false)
        , need_flush(false)
        , need_sync(false)
        , triggered(false)
        , force_wait(false)
        , uipq_index(-1)
        , has_s3d_layer(false)
        , s3d_type(0)
        , num_ui_layers(0)
        , num_mm_layers(0)
        , post_state(HWC_POST_INVALID)
        , sequence(HWC_SEQUENCE_INVALID)
        , timestamp(0)
        , hw_layers(NULL)
        , prev_present_fence_fd(-1)
        , is_full_invalidate(true)
        , mdp_job_output_buffer(0)
        , mm_fbt(false)
        , need_av_grouping(false)
        , num_processed_mm_layers(0)
        , is_black_job(false)
        , color_transform(nullptr)
        , is_HBM(false)
        , active_config(0)
        , disp_data(nullptr)
        , mir_disp_data(nullptr)
    { }

    // check if job should be processed
    bool enable;

    // check if any protect layer exist
    bool protect;

    // check if job should use secure composition
    bool secure;

    // check if job acts as a mirror source
    // if yes, it needs to provide a mirror output buffer to one another
    bool mirrored;

    // check if display use decouple mirror mode
    // if yes, it need to provide a decouple output buffer to display driver
    bool need_output_buffer;

    // check if job should be dropped
    bool dropped;

    // display id
    int disp_ori_id;

    // display id of mirror source
    int disp_mir_id;

    // orientation of display
    unsigned int disp_ori_rot;

    // orientation of mirror source
    unsigned int disp_mir_rot;

    // amount of the current available overlay inputs
    int num_layers;

    // check if ovl engine has availale inputs
    bool ovl_valid;

    // check if fbt is used
    bool fbt_exist;

    // check if need to wait for previous jobs
    bool need_flush;

    // check if UI and MM composers should be synced
    bool need_sync;

    // check if the current job is triggered
    bool triggered;

    // [WORKAROUND]
    // force to wait until job is finished
    bool force_wait;

    // the index of the uipq layer
    int uipq_index;

    // determine this job need to process in S3D flow or not
    bool has_s3d_layer;
    int s3d_type;

    // amount of layers for UI composer
    int num_ui_layers;

    // amount of layers for MM composer
    int num_mm_layers;

    // used to determine if need to trigger UI/MM composers
    int post_state;

    // used as a sequence number for profiling latency purpose
    unsigned int sequence;

    // used for video frame
    unsigned int timestamp;

    // input layers for compositing
    HWLayer* hw_layers;

    // mirror source buffer for bitblit
    HWBuffer hw_mirbuf;

    // the output buffer of
    // 1. virtual display
    // 2. mirror source
    HWBuffer hw_outbuf;

    // store present fence fd
    int prev_present_fence_fd;

    // cache hit information
    HitLayerInfo hit_info;

    LayerListInfo layer_info;

    // set true to invalidate full screen
    bool is_full_invalidate;

    // store the MDP job id for output buffer
    uint32_t mdp_job_output_buffer;

    // store the MDP job id and fence for clear background
    // index 0 is for normal buffer
    // index 1 is for secure buffer
    MdpJob fill_black[FILL_BLACK_JOB_SIZE];

    // set true when FBT will be processed by MM
    bool mm_fbt;

    // determine the job whether it need to group mdp and overlay
    bool need_av_grouping;

    // amount of layers for MM composer which do not bypass
    int num_processed_mm_layers;

    // determine the job is need to fill black to output buffer or not
    bool is_black_job;

    sp<ColorTransform> color_transform;

    // for LED HBM (High Backlight Mode) control
    bool is_HBM;

    // active config
    hwc2_config_t active_config;

    // the display data active config
    const DisplayData* disp_data;

    // the mirror source display data active config
    const DisplayData* mir_disp_data;
};

// HWCDispatcher is used to dispatch layers to DispatchThreads
class HWCDispatcher : public Singleton<HWCDispatcher>
{
public:
    // onPlugIn() is used to notify if a new display is added
    void onPlugIn(int dpy);

    // onPlugOut() is used to notify if a new display is removed
    void onPlugOut(int dpy);

    // setPowerMode() is used to wait display thread idle when display changes power mode
    void setPowerMode(int dpy, int mode);

    // onVSync() is used to receive vsync signal
    void onVSync(int dpy);

    // decideCtDirty() is used to verify if fbt layer type is changed
    bool decideCtDirtyAndFlush(int dpy, int idx);

    // decideDirtyAndFlush() is used to verify if layer type is changed then set as dirty
    bool decideDirtyAndFlush(int dpy, const PrivateHandle* priv_handle,
        int idx, int dirty, int type, const int64_t& hwc2_layer_id, const int32_t& layer_caps, const bool& need_pq);

    // setSessionMode() is used to set display session mode
    void setSessionMode(int dpy, bool mirrored);
    HWC_DISP_MODE getSessionMode(const int& dpy);

    // configMirrorJob() is used to config job as mirror source
    void configMirrorJob(DispatcherJob* job);
    void configMirrorOutput(DispatcherJob* job, const int& display_color_mode);

    // getJob() is used for HWCMediator to get a new job for filling in
    DispatcherJob* getJob(int dpy);

    // getExistJob() is used for HWCMediator to get an exist job for re-filling
    DispatcherJob* getExistJob(int dpy);

    // setJob() is used to update jobs in hwc::set()
    void setJob(const sp<HWCDisplay>& display);

    // trigger() is used to queue job and trigger dispatchers to work
    void trigger(const hwc2_display_t& dpy);

    struct VSyncListener : public virtual RefBase {
        // onVSync() is used to receive the vsync signal
        virtual void onVSync() = 0;
    };

    // registerVSyncListener() is used to register a VSyncListener to HWCDispatcher
    void registerVSyncListener(int dpy, const sp<VSyncListener>& listener);

    // removeVSyncListener() is used to remove a VSyncListener to HWCDispatcher
    void removeVSyncListener(int dpy, const sp<VSyncListener>& listener);

    // dump() is used for debugging purpose
    void dump(String8* dump_str);

    // saveFbt() is used to save present FBT handle
    // return true, if present FBT is the different with previous
    bool saveFbtHandle(int dpy, buffer_handle_t handle);

    // ignoreJob() is used to notify that the display is removed
    void ignoreJob(int dpy, bool ignore);

    void clearJob(const int32_t& dpy);
//------------------------------------------------------------------------------------------
// For skip validate
    void incSessionModeChanged()
    {
        AutoMutex l(m_session_mode_changed_mutex);
        ++m_session_mode_changed;
    }
    void decSessionModeChanged()
    {
        AutoMutex l(m_session_mode_changed_mutex);
        if (m_session_mode_changed > 0)
            --m_session_mode_changed;
    }
    int getSessionModeChanged()
    {
        AutoMutex l(m_session_mode_changed_mutex);
        return m_session_mode_changed;
    }

    int getOvlEnginePowerModeChanged(const int32_t& dpy) const;
    void decOvlEnginePowerModeChanged(const int32_t& dpy) const;
//-------------------------------------------------------------------------------------------
public:
    void addBufToBufRecorder(buffer_handle_t val)
    {
        AutoMutex l(m_dup_recorder_mutex);
        m_dup_recorder.push_back(val);

        if (m_dup_recorder.size() > 10)
        {
            HWC_LOGE("Buf recorder size too big %d", m_dup_recorder.size());
        }
    }

    void removeBufFromBufRecorder(buffer_handle_t val)
    {
        AutoMutex l(m_dup_recorder_mutex);
        size_t i = 0;
        for (; i < m_dup_recorder.size(); i++)
        {
            if (m_dup_recorder[i] == val)
                break;
        }

        if (i == m_dup_recorder.size())
        {
            HWC_LOGE("can't find handle buf:%p", val);
        }
        else
        {
            m_dup_recorder.erase(m_dup_recorder.begin() + i);
        }
    }

    void dupMMBufferHandle(DispatcherJob* job);
    void freeMMDuppedBufferHandle(DispatcherJob* job);

    void prepareMirror(const std::vector<sp<HWCDisplay> >& displays);
private:
    mutable Mutex m_dup_recorder_mutex;
    Vector<buffer_handle_t> m_dup_recorder;
private:
    friend class Singleton<HWCDispatcher>;
    friend class DispatchThread;

    HWCDispatcher();
    ~HWCDispatcher();

    void disableMirror(const sp<HWCDisplay>& display, DispatcherJob* job);
    // releaseResourceLocked() is used to release resources in display's WorkerCluster
    void releaseResourceLocked(int dpy);

    // access must be protected by m_vsync_lock
    mutable Mutex m_vsync_lock;

    // m_vsync_callbacks is a queue of VSyncListener registered by DispatchThread
    Vector<SortedVector< sp<VSyncListener> > > m_vsync_callbacks;

    // m_alloc_disp_ids is a bit set of displays
    // each bit index with a 1 corresponds to an valid display session
    BitSet32 m_alloc_disp_ids;

    // m_curr_jobs holds DispatcherJob of all displays
    // and is used between prepare() and set().
    DispatcherJob* m_curr_jobs[DisplayManager::MAX_DISPLAYS];

    class PostHandler : public LightRefBase<PostHandler>
    {
    public:
        PostHandler(HWCDispatcher* dispatcher,
            int dpy, const sp<OverlayEngine>& ovl_engine);

        virtual ~PostHandler();

        // set() is used to get retired fence from display driver
        virtual void set(const sp<HWCDisplay>& display, DispatcherJob* job) = 0;

        // setMirror() is used to fill dst_job->hw_mirbuf
        // from src_job->hw_outbuf
        virtual void setMirror(DispatcherJob* src_job, DispatcherJob* dst_job) = 0;

        // process() is used to wait outbuf is ready to use
        // and sets output buffer to display driver
        virtual void process(DispatcherJob* job) = 0;

    protected:
        // set overlay input
        void setOverlayInput(DispatcherJob* job);

        // m_dispatcher is used for callback usage
        HWCDispatcher* m_dispatcher;

        // m_disp_id is used to identify which display
        int m_disp_id;

        // m_ovl_engine is used for config overlay engine
        sp<OverlayEngine> m_ovl_engine;

        // m_sync_fence is used to create or wait fence
        sp<SyncFence> m_sync_fence;

        // store the presentfence fd
        int m_curr_present_fence_fd;
    };

    class PhyPostHandler : public PostHandler
    {
    public:
        PhyPostHandler(HWCDispatcher* dispatcher,
            int dpy, const sp<OverlayEngine>& ovl_engine)
            : PostHandler(dispatcher, dpy, ovl_engine)
        { }

        virtual void set(const sp<HWCDisplay>& display, DispatcherJob* job);

        virtual void setMirror(DispatcherJob* src_job, DispatcherJob* dst_job);

        virtual void process(DispatcherJob* job);
    };

    class VirPostHandler : public PostHandler
    {
    public:
        VirPostHandler(HWCDispatcher* dispatcher,
            int dpy, const sp<OverlayEngine>& ovl_engine)
            : PostHandler(dispatcher, dpy, ovl_engine)
        { }

        virtual void set(const sp<HWCDisplay>& display, DispatcherJob* job);

        virtual void setMirror(DispatcherJob* src_job, DispatcherJob* dst_job);

        virtual void process(DispatcherJob* job);

    private:
        void setError(DispatcherJob* job);
    };

    class PostProcessingHandler : public PostHandler
    {
    public:
        PostProcessingHandler(HWCDispatcher* dispatcher,
            int dpy, const sp<OverlayEngine>& ovl_engine);


        virtual void set(const sp<HWCDisplay>& display, DispatcherJob* job);

        virtual void setMirror(DispatcherJob* src_job, DispatcherJob* dst_job);

        virtual void process(DispatcherJob* job);
    };

    // WorkerCluster is used for processing composition of single display.
    // One WokerCluster would creates
    // 1. one thread to handle a job list and
    // 2. two threads to handle UI or MM layers.
    // Different display would use different WorkCluster to handle composition.
    struct WorkerCluster
    {
        WorkerCluster()
            : enable(false)
            , force_wait(false)
            , ignore_job(false)
            , ovl_engine(NULL)
            , dp_thread(NULL)
            , composer(NULL)
            , post_handler(NULL)
        { }

        // access must be protected by lock (DispatchThread <-> Hotplug thread)
        mutable Mutex plug_lock_loop;
        // access must be protected by lock (SurfaceFlinger <-> VSyncThread)
        mutable Mutex plug_lock_main;
        // access must be protected by lock (Hotplug thread <-> VSyncThread)
        mutable Mutex plug_lock_vsync;

        bool enable;
        bool force_wait;
        bool ignore_job;

        sp<OverlayEngine> ovl_engine;
        sp<DispatchThread> dp_thread;
        sp<LayerComposer> composer;
        sp<PostHandler> post_handler;

        struct PrevHwLayer
        {
            int type;
            int pool_id;
            bool enable;
            int64_t hwc2_layer_id;
            int32_t layer_caps;
            uint32_t pq_enable;
            uint32_t pq_pos;
            uint32_t pq_orientation;
            uint32_t pq_table_idx;
            bool need_pq;

            PrevHwLayer()
                : type(HWC_LAYER_TYPE_INVALID)
                , pool_id(-1)
                , enable(false)
                , hwc2_layer_id(-1)
                , layer_caps(0)
                , pq_enable(0)
                , pq_pos(0)
                , pq_orientation(0)
                , pq_table_idx(0)
                , need_pq(false)
            {
            }
        };
        std::vector<PrevHwLayer> prev_hw_layers;
    };

    // m_workers is the WorkerCluster array used by different display composition.
    WorkerCluster m_workers[DisplayManager::MAX_DISPLAYS];

    // m_sequence is used as a sequence number for profiling latency purpose
    // initialized to 1, (0 is reserved to be an error code)
    unsigned int m_sequence;

    // used to save the handle of previous fbt. When need to skip redundant
    // composition, compare the present fbt.
    buffer_handle_t m_prev_fbt[DisplayManager::MAX_DISPLAYS];

    // For skip validate
    int m_session_mode_changed;
    mutable Mutex m_session_mode_changed_mutex;
public:
    void initPrevHwLayers(const sp<HWCDisplay>& display, DispatcherJob* job);

    void fillPrevHwLayers(const sp<HWCDisplay>& display, DispatcherJob* job);

    // enable/disable skip redundant composition
    bool m_disable_skip_redundant;

    // use to prompt the limitation of bandwidth
    bool m_ultra_scenario;

    // merge ui and mm thread
    bool m_is_worker_all_in_one;
};

// DispatchThread handles DispatcherJobs
// from UILayerComposer and MMLayerComposer
class DispatchThread : public HWCThread,
                       public HWCDispatcher::VSyncListener
{
public:
    DispatchThread(int dpy);

    // trigger() is used to add a dispatch job into a job queue,
    // then triggers DispatchThread
    // async: whether the job will replace the last job or not
    // skip: Do not wake up the threadLoop
    void trigger(DispatcherJob* job, bool async = false, bool skip = false);

    int getQueueSize();

private:
    virtual void onFirstRef();
    virtual bool threadLoop();

    // waitNextVSyncLocked() requests and waits for the next vsync signal
    void waitNextVSyncLocked(int dpy);

    /* ------------------------------------------------------------------------
     * VSyncListener interface
     */
    void onVSync();

    // mark the last job of job queue as droppable
    bool markDroppableJob();

    // implementation of drop job
    bool dropJob();

    void clearUsedJob(DispatcherJob* job);

    // m_disp_id is used to identify which display
    // DispatchThread needs to handle
    int m_disp_id;

    // m_job_queue is a job queue
    // which new job would be queued in set()
    typedef Vector<DispatcherJob*> Fifo;
    Fifo m_job_queue;

    // access must be protected by m_vsync_lock
    mutable Mutex m_vsync_lock;
    Condition m_vsync_cond;
    bool m_vsync_signaled;

    // To record skiping times of trigger() when trigger_by_vsync is enabled
    // This is useful to detecting whether the VSync source is fine or not.
    int32_t m_continue_skip;

    // m_first_trigger is used to identify the display whether it has triggered
    // or not.
    bool m_first_trigger;
};

#endif // HWC_DISPATCHER_H_
