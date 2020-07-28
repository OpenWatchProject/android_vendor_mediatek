#ifndef HWC_BLITER_ULTRA_H_
#define HWC_BLITER_ULTRA_H_

#include "queue.h"
#include "bliter_async.h"
#include "utils/tools.h"

#define IS_MASTER(dpy) (dpy == HWC_DISPLAY_PRIMARY)
#define ID(ISMASTER) (ISMASTER ? ID_MASTER : ID_SLAVE)

using namespace android;

typedef struct AsyncBliterHandler::BufferConfig BufferConfig;
typedef struct DisplayBufferQueue::DisplayBuffer DisplayBuffer;

struct HWLayer;

class BliterNode
{
public:
    enum
    {
        MAX_OUT_CNT = 4,
    };

    struct BufferInfo
    {
        BufferInfo()
            : ion_fd(-1)
            , sec_handle(0)
            , handle(nullptr)
            , fence_fd(-1)
            , dpformat(DP_COLOR_YUYV)
            , pitch(0)
            , pitch_uv(0)
            , plane(1)
            , pq_enable(0)
            , pq_pos(0)
            , pq_orientation(0)
        { memset(size, 0, sizeof(size)); }
        int ion_fd;
        SECHAND sec_handle;
        buffer_handle_t handle;
        Rect rect;
        int fence_fd;

        // dst buffer setting for DpFramework
        DP_COLOR_ENUM dpformat;
        unsigned int  pitch;
        unsigned int  pitch_uv;
        unsigned int  plane;
        unsigned int  size[3];
        uint32_t pq_enable;
        uint32_t pq_pos;
        uint32_t pq_orientation;
        uint32_t pq_table_idx;
    };

    struct SrcInvalidateParam
    {
        SrcInvalidateParam()
            : gralloc_color_range(0)
            , deinterlace(false)
            , is_secure(false)
            , is_flush(false)
            , is_uipq(false)
            , is_game(false)
            , pool_id(0)
            , time_stamp(0)
            , dataspace(0)
        {}

        BufferInfo bufInfo;

        unsigned int gralloc_color_range;
        bool deinterlace;
        bool is_secure;
        bool is_flush;
        bool is_uipq;
        bool is_game;

        int32_t pool_id;
        uint32_t time_stamp;
        int32_t dataspace;
    };

    struct DstInvalidateParam
    {
        DstInvalidateParam()
            : enable(false)
            , xform(0)
            , is_enhance(0)
        {}
        bool enable;
        BufferInfo bufInfo;
        uint32_t xform;
        Rect src_crop;
        Rect dst_crop;
        uint32_t is_enhance;
        int32_t dataspace;
    };

    struct Parameter
    {
        Rect            src_roi;
        Rect            dst_roi;
        BufferConfig*   config;
        uint32_t        xform;
        uint32_t        pq_enhance;
    };

    BliterNode(DpAsyncBlitStream2* blit_stream, uint32_t dpy);

    ~BliterNode();

    SrcInvalidateParam& getSrcInvalidateParam() { return m_src_param; }

    DstInvalidateParam& getDstInvalidateParam(uint32_t i) { return m_dst_param[i >= MAX_OUT_CNT ? 0 : i]; }

    void setSrc(BufferConfig* config, PrivateHandle& src_priv_handle, int* src_fence_fd = NULL, const bool& is_game = false);

    void setDst(uint32_t idx, Parameter* param, int ion_fd, SECHAND sec_handle, int* dst_fence_fd = NULL);

    void cancelJob(const int32_t& job_id);

    status_t invalidate(uint32_t job_id, Rect* cal_dst_roi = nullptr, const uint32_t& dispatch_job_id = 0);

    status_t getHWCExpectMDPFinishedTime(timeval* mdpFinishTime, const uint32_t& dispatch_job_id);
private:
    static status_t calculateROI(DpRect* src_roi, DpRect* dst_roi, DpRect* output_size,
        const SrcInvalidateParam& src_param, const DstInvalidateParam& dst_param,
        const BufferInfo& src_buf, const BufferInfo& dst_buf);
    static status_t calculateContentROI(Rect* cnt_roi, const DpRect& dst_roi,
        const DpRect& output_roi, int32_t padding);

    /*unsigned int mapDpOrientation(const uint32_t transform);

    DP_PROFILE_ENUM mapDpColorRange(const uint32_t range, const bool& is_input);*/

    DP_PROFILE_ENUM mapDataspace2DpColorRange(const int32_t ds, const bool& is_input);

    void mapP3PqParam(DpPqParam* dppq_param, const int32_t out_ds);

    status_t errorCheck();

    DpAsyncBlitStream2* m_blit_stream;

    SrcInvalidateParam m_src_param;

    DstInvalidateParam m_dst_param[MAX_OUT_CNT];

    DbgLogger* m_config_logger;

    DbgLogger* m_geometry_logger;

    DbgLogger* m_buffer_logger;

    uint32_t m_dpy;

    bool m_bypass_mdp_for_debug;
};

class UltraBliter : public Singleton<UltraBliter>
{
public:
    enum INDEX
    {
        ID_MASTER,
        ID_SLAVE,
    };

    enum SyncState
    {
        SyncState_init,
        SyncState_start,
        SyncState_set_buf_done,
        SyncState_trig_done,
    };

    enum TrigAction
    {
        DoJob,
        CancelJob,
    };

    UltraBliter();

    ~UltraBliter();

    void setBliter(BliterNode* blit_node);

    void masterSetSrcBuf(HWLayer* buf, uint32_t job_seq);

    void setDstBuf(bool is_master, DisplayBuffer* buf);

    void config(int is_master, BliterNode::Parameter* param);

    status_t trig(bool is_master, TrigAction action, int* mdp_fence_fd = NULL);

    inline void debug(bool on_off) { m_is_debug_on = on_off; }

private:
    status_t processJob();

    void barrier(bool is_master, SyncState sync_state);

    BliterNode* m_bliter_node;

    HWLayer* m_src_buf;
    DisplayBuffer* m_dst_buf[2];
    BliterNode::Parameter* m_param[2];

    uint32_t m_job_seq;

    mutable Mutex m_lock;
    Condition m_condition;
    SyncState m_sync_state;

    mutable Mutex m_lock_inverse;
    Condition m_condition_inverse;

    status_t m_status;
    TrigAction m_action[2];

    bool m_is_debug_on;
};
#endif
