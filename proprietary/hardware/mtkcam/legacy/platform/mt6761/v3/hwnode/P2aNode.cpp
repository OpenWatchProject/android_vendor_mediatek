/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkCam/P2aNode"
//
#include <mtkcam/Log.h>
#include "BaseNode.h"
#include "hwnode_utilities.h"
#include <mtkcam/v3/hwnode/P2aNode.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/v3/hal/IHal3A.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <mtkcam/iopipe/PostProc/IPortEnum.h>
#include <mtkcam/iopipe/SImager/IImageTransform.h>

//
#include <vector>
#include <list>
#include <utils/KeyedVector.h>

//
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/featureio/eis_hal.h>
//
#include <mtkcam/Trace.h>
//
#include <cutils/properties.h>
//
// #include "nr3d.h"
#include <mtkcam/featureio/3dnr_hal_base.h>

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;

using namespace std;
using namespace NSIoPipe;

using namespace NSIoPipe::NSPostProc;
using namespace NS3Av3;

/******************************************************************************
 *
 ******************************************************************************/
#define P2THREAD_NAME_ENQUE ("Cam@P2Enq")
#define P2THREAD_NAME_COPY  ("Cam@P2Copy")
#define P2THREAD_POLICY     (SCHED_OTHER)
#define P2THREAD_PRIORITY   (0)
//
#define WAITBUFFER_TIMEOUT (1000000000L)
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define NR3D_ENABLE 1

//
#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif
/******************************************************************************
 *
 ******************************************************************************/
#define SUPPORT_3A               (1)
#define FD_PORT_SUPPORT          (1)
#define FORCE_EIS_ON             (0)

#define DEBUG_LOG                (0)
/******************************************************************************
 *
 ******************************************************************************/
static const PortID PORT_IMGI  (EPortType_Memory, EPipePortIndex_IMGI , 0);
static const PortID PORT_WDMAO (EPortType_Memory, EPipePortIndex_WDMAO, 1);
static const PortID PORT_WROTO (EPortType_Memory, EPipePortIndex_WROTO, 1);
#if FD_PORT_SUPPORT
static const PortID PORT_IMG2O (EPortType_Memory, EPipePortIndex_IMG2O, 1);
#endif

static const PortID PORT_IMG3O (EPortType_Memory, EPipePortIndex_IMG3O, 1);
static const PortID PORT_VIPI (EPortType_Memory, EPipePortIndex_VIPI, 0);

inline
MBOOL isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId ) {
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }

    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MVOID
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


/******************************************************************************
 *
 ******************************************************************************/
class StreamControl
{
    public:
        typedef enum
        {
            eStreamStatus_NOT_USED = (0x00000000UL),
            eStreamStatus_FILLED   = (0x00000001UL),
            eStreamStatus_ERROR    = (0x00000001UL << 1),
        } eStreamStatus_t;

    public:

        virtual                         ~StreamControl() {};

    public:

        virtual MERROR                  getInfoIOMapSet(
                                            sp<IPipelineFrame> const& pFrame,
                                            IPipelineFrame::InfoIOMapSet& rIOMapSet
                                        ) const                                   = 0;

        // query in/out stream function
        virtual MBOOL                   isInImageStream(
                                            StreamId_T const streamId
                                        ) const                                   = 0;

        virtual MBOOL                   isInMetaStream(
                                            StreamId_T const streamId
                                        ) const                                   = 0;

        // image stream related
        virtual MERROR                  acquireImageStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IImageStreamBuffer>& rpStreamBuffer
                                        )                                         = 0;

        virtual MVOID                   releaseImageStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            sp<IImageStreamBuffer> const pStreamBuffer,
                                            MUINT32 const status
                                        ) const                                   = 0;

        virtual MERROR                  acquireImageBuffer(
                                            StreamId_T const streamId,
                                            sp<IImageStreamBuffer> const pStreamBuffer,
                                            sp<IImageBuffer>& rpImageBuffer
                                        ) const                                   = 0;

        virtual MVOID                   releaseImageBuffer(
                                            sp<IImageStreamBuffer> const pStreamBuffer,
                                            sp<IImageBuffer> const pImageBuffer
                                        ) const                                   = 0;

        // meta stream related
        virtual MERROR                  acquireMetaStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IMetaStreamBuffer>& rpStreamBuffer
                                        )                                         = 0;

        virtual MVOID                   releaseMetaStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            MUINT32 const status
                                        ) const                                   = 0;

        virtual MERROR                  acquireMetadata(
                                            StreamId_T const streamId,
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            IMetadata*& rpMetadata
                                        ) const                                   = 0;

        virtual MVOID                   releaseMetadata(
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            IMetadata* const pMetadata
                                        ) const                                   = 0;

        // frame control related
        virtual MVOID                   onPartialFrameDone(
                                            sp<IPipelineFrame> const& pFrame
                                        )                                         = 0;

        virtual MVOID                   onFrameDone(
                                            sp<IPipelineFrame> const& pFrame
                                        )                                         = 0;

};

//TODO: reuse P2Node MetaHandle
class MetaHandleA
    : public VirtualLightRefBase
{
    public:
        typedef enum
        {
            STATE_NOT_USED,
            STATE_READABLE,
            STATE_WRITABLE,
            STATE_WRITE_OK = STATE_READABLE,
            STATE_WRITE_FAIL,
        } BufferState_t;

    public:
        static sp<MetaHandleA>           create(
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId
                                        );
                                        ~MetaHandleA();
    protected:
                                        MetaHandleA(
                                            StreamControl* pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            BufferState_t const init_state,
                                            IMetadata * const pMeta
                                        )
                                            : mpStreamCtrl(pCtrl)
                                            , mpFrame(pFrame)
                                            , mStreamId(streamId)
                                            , mpStreamBuffer(pStreamBuffer)
                                            , mpMetadata(pMeta)
                                            , muState(init_state)
                                        {}

    public:
        IMetadata*                      getMetadata() { return mpMetadata; }

#if 0
        virtual MERROR                  waitState(
                                            BufferState_t const state,
                                            nsecs_t const nsTimeout = WAITBUFFER_TIMEOUT
                                        )                                                   = 0;
#endif
        MVOID                           updateState(
                                            BufferState_t const state
                                        );
    private:
        Mutex                           mLock;
        //Condition                       mCond;
        StreamControl* const            mpStreamCtrl;
        sp<IPipelineFrame> const        mpFrame;
        StreamId_T const                mStreamId;
        sp<IMetaStreamBuffer> const     mpStreamBuffer;
        IMetadata* const                mpMetadata;
        MUINT32                         muState;
};


class BufferHandle
    : public VirtualLightRefBase
{
    public:
        typedef enum
        {
            STATE_NOT_USED,
            STATE_READABLE,
            STATE_WRITABLE,
            STATE_WRITE_OK = STATE_READABLE,
            STATE_WRITE_FAIL,
        } BufferState_t;

    public:
        virtual                         ~BufferHandle() {}

    public:
        virtual IImageBuffer*           getBuffer()                                         = 0;

        virtual MERROR                  waitState(
                                            BufferState_t const state,
                                            nsecs_t const nsTimeout = WAITBUFFER_TIMEOUT
                                        )                                                   = 0;
        virtual MVOID                   updateState(
                                            BufferState_t const state
                                        )                                                   = 0;
};


class StreamBufferHandleA
    : public BufferHandle
{
    public:
        static sp<BufferHandle>         create(
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId
                                        );
                                        ~StreamBufferHandleA();
    protected:
                                        StreamBufferHandleA(
                                            StreamControl* pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IImageStreamBuffer> const pStreamBuffer
                                        )
                                            : mpStreamCtrl(pCtrl)
                                            , mpFrame(pFrame)
                                            , mStreamId(streamId)
                                            , mpStreamBuffer(pStreamBuffer)
                                            , muState(STATE_NOT_USED)
                                            , mpImageBuffer(NULL)
                                        {}

    public:
        IImageBuffer*                   getBuffer() { return mpImageBuffer.get(); }
        MERROR                          waitState(
                                            BufferState_t const state,
                                            nsecs_t const nsTimeout
                                        );
        MVOID                           updateState(
                                            BufferState_t const state
                                        );

    private:
        Mutex                           mLock;
        Condition                       mCond;
        StreamControl* const            mpStreamCtrl;
        sp<IPipelineFrame> const        mpFrame;
        StreamId_T const                mStreamId;
        sp<IImageStreamBuffer> const    mpStreamBuffer;
        MUINT32                         muState;
        sp<IImageBuffer>                mpImageBuffer;
};



/******************************************************************************
 *
 ******************************************************************************/
class P2aNodeImp
    : public BaseNode
    , public P2aNode
    , public StreamControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:

    class UnpackThread
        : public Thread
    {

    public:

                                    UnpackThread(P2aNodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~UnpackThread()
                                    {}

    public:

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
                    // Ask this object's thread to exit. This function is asynchronous, when the
                    // function returns the thread might still be running. Of course, this
                    // function can be called from a different thread.
                    virtual void        requestExit();

                    // Good place to do one-time initializations
                    virtual status_t    readyToRun();

    private:
                    // Derived class must implement threadLoop(). The thread starts its life
                    // here. There are two ways of using the Thread object:
                    // 1) loop: if threadLoop() returns true, it will be called again if
                    //          requestExit() wasn't called.
                    // 2) once: if threadLoop() returns false, the thread will exit upon return.
                    virtual bool        threadLoop();

    private:

                    P2aNodeImp*       mpNodeImp;

    };

//from P2Processor struct
    public:
                    struct FrameInput
                    {
                        PortID                          mPortId;
                        sp<BufferHandle>                mHandle;
                    };

                    struct FrameOutput
                    {
                        PortID                          mPortId;
                        sp<BufferHandle>                mHandle;
                        MINT32                          mTransform;
                    };

                    struct FrameParams : public LightRefBase<FrameParams>
                    {
                        FrameInput                      in;
                        Vector<FrameOutput>             vOut;
                        FrameOutput                     pendingOut;
                        //
                        MBOOL                           bResized;
                        //
                        sp<MetaHandleA>                  inApp;
                        sp<MetaHandleA>                  inRetApp;
                        sp<MetaHandleA>                  inHal;
                        sp<MetaHandleA>                  outApp;
                        sp<MetaHandleA>                  outHal;
                    };

    private:    //private use structures
                    struct eis_region
                    {
                        MUINT32 x_int;
                        MUINT32 x_float;
                        MUINT32 y_int;
                        MUINT32 y_float;
                        MSize   s;
                        MINT32  gmvX;
                        MINT32  gmvY;
                    };

                    struct crop_infos
                    {
                        // port
                        MBOOL                isResized;
                        //
                        MSize                sensor_size;
                        // p1 crop infos
                        MRect                crop_p1_sensor;
                        MSize                dstsize_resizer;
                        MRect                crop_dma;
                        //
                        simpleTransform      tranActive2Sensor;
                        simpleTransform      tranSensor2Resized;
                        //
                        // target crop: cropRegion
                        // not applied eis's mv yet, but the crop area is already reduced by
                        // EIS ratio.
                        // _a: active array coordinates
                        // _s: sensor coordinates
                        // active array coordinates
                        MRect                crop_a;
                        //MPoint               crop_a_p;
                        //MSize                crop_a_size;
                        // sensor coordinates
                        //MPoint               crop_s_p;
                        //MSize                crop_s_size;
                        // resized coordinates
                        //
                        MBOOL                isEisEabled;
                        vector_f             eis_mv_a; //active array coor.
                        vector_f             eis_mv_s; //sensor coor.
                        vector_f             eis_mv_r; //resized coor.

                        MVOID                       dump() const {
                                                        MY_LOGD("isResized %d", isResized);
                                                        MY_LOGD("p1 info (%d,%d,%dx%d), (%dx%d), (%d,%d,%dx%d)",
                                                                crop_p1_sensor.p.x,
                                                                crop_p1_sensor.p.y,
                                                                crop_p1_sensor.s.w,
                                                                crop_p1_sensor.s.h,
                                                                dstsize_resizer.w,
                                                                dstsize_resizer.h,
                                                                crop_dma.p.x,
                                                                crop_dma.p.y,
                                                                crop_dma.s.w,
                                                                crop_dma.s.h
                                                               );
                                                        MY_LOGD("tran active to sensor o %d, %d, s %dx%d -> %dx%d",
                                                                tranActive2Sensor.tarOrigin.x,
                                                                tranActive2Sensor.tarOrigin.y,
                                                                tranActive2Sensor.oldScale.w,
                                                                tranActive2Sensor.oldScale.h,
                                                                tranActive2Sensor.newScale.w,
                                                                tranActive2Sensor.newScale.h
                                                               );
                                                        MY_LOGD("tran sensor to resized o %d, %d, s %dx%d -> %dx%d",
                                                                tranSensor2Resized.tarOrigin.x,
                                                                tranSensor2Resized.tarOrigin.y,
                                                                tranSensor2Resized.oldScale.w,
                                                                tranSensor2Resized.oldScale.h,
                                                                tranSensor2Resized.newScale.w,
                                                                tranSensor2Resized.newScale.h
                                                               );
                                                        MY_LOGD("modified active crop %d, %d, %dx%d",
                                                                crop_a.p.x,
                                                                crop_a.p.y,
                                                                crop_a.s.w,
                                                                crop_a.s.h
                                                               );
                                                        MY_LOGD("isEisOn %d", isEisEabled);
                                                        MY_LOGD("mv in active %d/%d, %d/%d",
                                                                eis_mv_a.p.x, eis_mv_a.pf.x,
                                                                eis_mv_a.p.y, eis_mv_a.pf.y
                                                                );
                                                        MY_LOGD("mv in sensor %d/%d, %d/%d",
                                                                eis_mv_s.p.x, eis_mv_s.pf.x,
                                                                eis_mv_s.p.y, eis_mv_s.pf.y
                                                                );
                                                        MY_LOGD("mv in resized %d/%d, %d/%d",
                                                                eis_mv_r.p.x, eis_mv_r.pf.x,
                                                                eis_mv_r.p.y, eis_mv_r.pf.y
                                                                );
                                                    }
                    };


                    struct FrameInfo : public LightRefBase<FrameInfo>
                    {
                        sp<FrameParams>                 mpParams;
                        sp<IPipelineFrame>              mpFrame;
                        FrameInfo(
                            sp<FrameParams>                 _param = NULL,
                            sp<IPipelineFrame>              _frame = NULL
                        )
                            : mpParams(_param)
                            , mpFrame(_frame)
                        {
                        }
                    };



//from P2Processor end

    //
public:     ////                    Operations.

                                    P2aNodeImp(ePass2Type const type);

                                    ~P2aNodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  freeAllStreams();

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

//david add stream control
public:     ////                    StreamControl

    MERROR                          getInfoIOMapSet(
                                        sp<IPipelineFrame> const& pFrame,
                                        IPipelineFrame::InfoIOMapSet& rIOMapSet
                                    ) const;

    MBOOL                           isInImageStream(
                                        StreamId_T const streamId
                                    ) const;

    MBOOL                           isInMetaStream(
                                        StreamId_T const streamId
                                    ) const;

    MERROR                          acquireImageStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer
                                    );

    MVOID                           releaseImageStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        sp<IImageStreamBuffer> const pStreamBuffer,
                                        MUINT32 const status
                                    ) const;

    MERROR                          acquireImageBuffer(
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer> const pStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer
                                    ) const;

    MVOID                           releaseImageBuffer(
                                        sp<IImageStreamBuffer> const rpStreamBuffer,
                                        sp<IImageBuffer> const pImageBuffer
                                    ) const;

    MERROR                          acquireMetaStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer>& rpStreamBuffer
                                    );

    MVOID                           releaseMetaStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        sp<IMetaStreamBuffer> const pStreamBuffer,
                                        MUINT32 const status
                                    ) const;

    MERROR                          acquireMetadata(
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer> const pStreamBuffer,
                                        IMetadata*& rpMetadata
                                    ) const;

    MVOID                           releaseMetadata(
                                        sp<IMetaStreamBuffer> const pStreamBuffer,
                                        IMetadata* const pMetadata
                                    ) const;

    MVOID                           onPartialFrameDone(
                                        sp<IPipelineFrame> const& pFrame
                                    );

    MVOID                           onFrameDone(
                                        sp<IPipelineFrame> const& pFrame
                                    );


public:
    MERROR                          mapPortId(
                                        StreamId_T const streamId, // [in]
                                        MUINT32 const transform,   // [in]
                                        MUINT32& rOccupied,        // [in/out]
                                        PortID&  rPortId           // [out]
                                    ) const;


inline MBOOL                        isFullRawLocked(StreamId_T const streamId) const {
                                        for( size_t i = 0; i < mpvInFullRaw.size(); i++ ) {
                                            if( isStream(mpvInFullRaw[i], streamId) )
                                                return MTRUE;
                                        }
                                        return MFALSE;
                                    }

inline MBOOL                        isResizeRawLocked(StreamId_T const streamId) const {
                                        return isStream(mpInResizedRaw, streamId);
                                    }

protected:  ////                    Operations.

    MERROR                          onDequeRequest( //TODO: check frameNo
                                        android::sp<IPipelineFrame>& rpFrame
                                    );

    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    //david
    MERROR                          verifyConfigParams(
                                        ConfigParams const & rParams
                                    ) const;

    MERROR                          onProcessFrameReturnRet(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MVOID                           waitForRequestDrained();


    MERROR                          getImageBufferAndLock(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer,
                                        MBOOL const isInStream
                                    );


    //
    MVOID                           doProcess(
                                        android::sp<IPipelineFrame> const& pFrame,
                                        sp<FrameParams> &param_p2
                                    );

    MERROR                          do3dnrFlow(android::sp<IPipelineFrame> const& pFrame,
                                        sp<FrameParams> &param_p2, crop_infos &cropInfos,
                                        QParams &enqueParams, Input &input,
                                        Output &img3oOut, ModuleInfo &moduleinfo
                                    );


//from P2Processor
protected:
        MERROR                          checkParams(FrameParams const params) const;

        MERROR                          getCropInfos(
                                            IMetadata* const inApp,
                                            IMetadata* const inHal,
                                            MBOOL const isResized,
                                            crop_infos & cropInfos
                                        ) const;

        MVOID                           queryCropRegion(
                                            IMetadata* const meta_request,
                                            MBOOL const isEisOn,
                                            MRect& targetCrop
                                        ) const;

        MVOID                           updateCropRegion(
                                            MRect const crop,
                                            IMetadata* meta_result
                                        ) const;

        MVOID                           calcCrop_viewangle(
                                            crop_infos const& cropInfos,
                                            MSize const& dstSize,
                                            MCropRect& result
                                        ) const;

        MBOOL                           isEISOn(
                                            IMetadata* const inApp
                                        ) const;

        MBOOL                           queryEisRegion(
                                            IMetadata* const inHal,
                                            eis_region& region
                                        ) const;

        MBOOL                           refineBoundary(
                                            MSize const& bufSize,
                                            MCropRect& crop
                                        ) const;

        static MVOID                    pass2CbFunc(QParams& rParams);
        MVOID                           handleDeque(QParams& rParams);

//from P2Processor end
protected:

    MERROR                          threadSetting();

protected:  ////                    LOGE & LOGI on/off
    MINT32                          mLogLevel;

protected:  ////                    Data Members. (Config)
    ePass2Type const                mType;
    mutable RWLock                  mConfigRWLock;
    // meta
    sp<IMetaStreamInfo>             mpInAppMeta_Request;
    sp<IMetaStreamInfo>             mpInAppRetMeta_Request;
    sp<IMetaStreamInfo>             mpInHalMeta_P1;
    sp<IMetaStreamInfo>             mpOutAppMeta_Result;
    sp<IMetaStreamInfo>             mpOutHalMeta_Result;

    // image
    Vector<sp<IImageStreamInfo> >   mpvInFullRaw;
    sp<IImageStreamInfo>            mpInResizedRaw;
    ImageStreamInfoSetT             mvOutImages;
    sp<IImageStreamInfo>            mpOutFd;
    sp<IImageStreamInfo>            mpOut3dnr;

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained;
    Condition                       mbRequestDrainedCond;
    MBOOL                           mbRequestExit;

private:   ////                     Threads
    sp<UnpackThread>                mpUnpackThread;


    //from P2Processor
    Mutex                           mLock;
    INormalStream*                  mpPipe;
    IFeatureStream*                 mpIFeatureStream;
    // sp<nr3d>                        mpNr3d;
    hal3dnrBase*                    mp3dnr;
    //sp<IImageBuffer>                mpPervious;
    vector<sp<IPipelineFrame> >     mvFrames;
    vector<sp<BufferHandle> >       mvPervious;
    sp<BufferHandle>                mpPervious;
    IHal3A*                         mp3A;
    //
    MUINT32                         muEnqueCnt;
    MUINT32                         muDequeCnt;
    vector<sp<FrameInfo> >          mvRunning;

    MRect                           mActiveArray;//from P2Processor::InitParams
    //muProcessCnt
    DefaultKeyedVector<MUINT32, MUINT32>  mvProcessCnt;

    //MUINT32                         muProcessCnt;
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<P2aNode>
P2aNode::
createInstance(ePass2Type const type)
{
    if( type < 0 ||
        type >= PASS2_TYPE_TOTAL )
    {
        MY_LOGE("not supported p2 type %d", type);
        return NULL;
    }
    //
    return new P2aNodeImp(type);
}


/******************************************************************************
 *
 ******************************************************************************/
P2aNodeImp::
P2aNodeImp(ePass2Type const type)
    : BaseNode()
    , P2aNode()
    //david start
    , mType(type)
    , mConfigRWLock()
    //
    , mpInAppMeta_Request()
    , mpInAppRetMeta_Request()
    , mpInHalMeta_P1()
    , mpOutAppMeta_Result()
    , mpOutHalMeta_Result()
    //
    , mpvInFullRaw()
    , mpInResizedRaw()
    , mvOutImages()
    , mpOutFd()
    , mpOut3dnr()
    //david end
    , mbRequestDrained(MFALSE)
    , mbRequestExit(MFALSE)
    //
    , mpIFeatureStream(NULL)
    , mp3dnr(NULL)
    , mvFrames(NULL)
    , mvPervious(NULL)
    , mpPervious(NULL)
    , mpUnpackThread(NULL)
    , mpPipe(NULL)
    , mp3A(NULL)
    , muEnqueCnt(0)
    , muDequeCnt(0)
    //
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = atoi(cLogLevel);
    if ( mLogLevel == 0 ) {
        ::property_get("debug.camera.log.P2Node", cLogLevel, "0");
        mLogLevel = atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
P2aNodeImp::
~P2aNodeImp()
{

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    Mutex::Autolock _l(mLock);
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;
    //
    MY_LOGD("OpenId %d, nodeId %d, name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    mpUnpackThread = new UnpackThread(this);
    if( mpUnpackThread->run(P2THREAD_NAME_ENQUE) != OK ) {
        return UNKNOWN_ERROR;
    }

    //
    MRect activeArray;
    {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }
        IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
        if( tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray) ) {
            MY_LOGD_IF(mLogLevel>=1,"active array(%d, %d, %dx%d)",
                    activeArray.p.x, activeArray.p.y, activeArray.s.w, activeArray.s.h);
        } else {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }
    }
    mActiveArray = activeArray;


    //INormalStream* mpPipe = NULL;
    //IHal3A*        mp3A   = NULL;
    ENormalStreamTag streamtag = ENormalStreamTag_Prv;
    if( mType == P2aNode::PASS2_STREAM ) {
        streamtag = ENormalStreamTag_Prv;
    }
    else if ( mType == P2aNode::PASS2_TIMESHARING ) {
        streamtag = ENormalStreamTag_Vss;
    }
    else {
        MY_LOGE("not supported type %d", mType);
        goto lbExit;
    }

    MY_LOGD("NR3D_ENABLE= %d",NR3D_ENABLE);

    //NR3D flow(init)
    if (NR3D_ENABLE)
    {
        /* Create IFeatureStream */
        mpIFeatureStream = IFeatureStream::createInstance(LOG_TAG, EFeatureStreamTag_Stream, mOpenId, true);
        if( mpIFeatureStream == NULL ) {
            MY_LOGE("create IFeatureStream failed");
            goto lbExit;
        }
        //
        if( ! mpIFeatureStream->init() )
        {
            MY_LOGE("IFeatureStream init failed");
            goto lbExit;
        }
        // mpNr3d = new nr3d();
        // mpNr3d->init();
        mp3dnr = hal3dnrBase::createInstance();
        mp3dnr->init();
        MY_LOGD("create FeatureStream:%p, 3dnr=%p", mpIFeatureStream, mp3dnr);
        mpPipe = NULL;
    }
    else
    {
        mpPipe = INormalStream::createInstance(LOG_TAG, streamtag, mOpenId, true);
        if( mpPipe == NULL ) {
            MY_LOGE("create pipe failed");
            goto lbExit;
        }
        //
        if( ! mpPipe->init() )
        {
            MY_LOGE("pipe init failed");
            goto lbExit;
        }
        MY_LOGD("create INormalStream:%p", mpPipe);
    }
    //
    mp3A = IHal3A::createInstance(IHal3A::E_Camera_3, mOpenId, LOG_TAG);
    if( mp3A == NULL ) {
        MY_LOGE("create 3A failed");
        goto lbExit;
    }
    //
lbExit:
    // if( ! mpPipe || !mp3A || !mpIFeatureStream) {
    if( ! (mpPipe || mpIFeatureStream) || !mp3A) {
        if( mpPipe ) {
            mpPipe->uninit();
            mpPipe->destroyInstance(LOG_TAG);
            mpPipe = NULL;
        }
        if( mp3A ) {
            mp3A->destroyInstance(LOG_TAG);
            mp3A = NULL;
        }
        if( mpIFeatureStream ) {
            mpIFeatureStream->uninit();
            mpIFeatureStream->destroyInstance(LOG_TAG);
            mpIFeatureStream = NULL;
        }
    }

    MY_LOGD("create processor type %d: pipe %p, 3A %p, IFeatureStream=%p",
            mType, mpPipe, mp3A, mpIFeatureStream);


    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
config(ConfigParams const& rParams)
{
    FUNC_START;
    //
    {
        MERROR const err = verifyConfigParams(rParams);
        if( err != OK ) {
            MY_LOGE("verifyConfigParams failed, err = %d", err);
            return err;
        }
    }
    //
    flush();
    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta_Request  = rParams.pInAppMeta;
        mpInAppRetMeta_Request = rParams.pInAppRetMeta;
        mpInHalMeta_P1       = rParams.pInHalMeta;
        mpOutAppMeta_Result  = rParams.pOutAppMeta;
        mpOutHalMeta_Result  = rParams.pOutHalMeta;
        // image
        mpvInFullRaw         = rParams.pvInFullRaw;
        mpInResizedRaw       = rParams.pInResizedRaw;
        mvOutImages          = rParams.vOutImage;
        mpOutFd              = rParams.pOutFDImage;
        mpOut3dnr            = rParams.pOutNR3DImage;
        //
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");

    if( mpPipe ) {
        if( ! mpPipe->uninit() ) {
            MY_LOGE("pipe uninit failed");
        }
        mpPipe->destroyInstance(LOG_TAG);
    }

    if( mp3A ) {
        mp3A->destroyInstance(LOG_TAG);
    }


    if( mpIFeatureStream ) {
        mpIFeatureStream->uninit();
        mpIFeatureStream->destroyInstance(LOG_TAG);
        mpIFeatureStream = NULL;

        if(mp3dnr){
            mp3dnr->destroyInstance();
            mp3dnr = NULL;
        }
    }


    // exit threads
    mpUnpackThread->requestExit();
    // join
    mpUnpackThread->join();
    //
    mpUnpackThread = NULL;
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
flush()
{
    FUNC_START;
    //
    // 1. clear requests
    {
        Mutex::Autolock _l(mRequestQueueLock);
        //
        Que_T::iterator it = mRequestQueue.begin();
        while ( it != mRequestQueue.end() ) {
            BaseNode::flush(*it);
            it = mRequestQueue.erase(it);
        }
#warning "Hotdog should review for UNHANDLED but PROCESSED streams."
/*        MY_LOGD("previous request:%d", mvPervious.size());
        while ( !mvPervious.empty())
        {
            //free the previous steambuffer and do apply relase
            {
                MBOOL const success = true;
                mvPervious.front()->updateState(
                    success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
                mvPervious.erase(mvPervious.begin());
            }
            sp<IPipelineFrame> pFrameOld = mvFrames.front();
            mvFrames.erase(mvFrames.begin());
            IStreamBufferSet& rStreamBufferSet = pFrameOld->getStreamBufferSet();
            rStreamBufferSet.applyRelease(getNodeId());
            MY_LOGD_IF(mLogLevel >= 1, "Free old img3o handle, mvPervious=%d, mvFrames=%d",
                        mvPervious.size(), mvFrames.size());
        }*/
    }
    //
    // 2. wait enque thread
    waitForRequestDrained();
    //
    {
        Mutex::Autolock _l(mLock);
        mvPervious.clear();
        mvFrames.clear();
    }
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
freeAllStreams()
{
    FUNC_START;
    // use lock to protect request queue
    Mutex::Autolock _l(mRequestQueueLock);

    // mkdbg: lock onDequeRequest(..) to prevent further request coming in'
    //     before all enque-ed QParam is called back, we should not process freeAllStreams() for now
    int wait_count = 0;
    int max_wait_count = ((muEnqueCnt - muDequeCnt) > 0) ? (muEnqueCnt - muDequeCnt) * 2: 0; // wait for twice the callback time
    int MAX_ENQUE_QPARAM_MSEC = 300;
    while(wait_count < max_wait_count)
    {
        MY_LOGD("mkdbg: wait ongoing frame done, (encCnt, decCnt)=(%d, %d) -- round %d", muEnqueCnt, muDequeCnt, wait_count);
        usleep(MAX_ENQUE_QPARAM_MSEC);
        wait_count++;
    }
    if (max_wait_count != 0 && wait_count >= max_wait_count)
    {
        MY_LOGW("mkdbg: !!WARNING!! wait %d frames for TOO LONG: %d.%d sec",
            max_wait_count,
            (max_wait_count*MAX_ENQUE_QPARAM_MSEC)/1000,
            ((max_wait_count*MAX_ENQUE_QPARAM_MSEC)%1000)/100
            );
    }

    // mkdbg: should use mLock to protect mvPervious, mvFrames
    {
        Mutex::Autolock _l(mLock);

        //free the previous steambuffer and do apply relase
        MBOOL const success = true;
        MY_LOGD("free all streams...");
        while ( !mvPervious.empty())
        {
            //free the previous steambuffer and do apply relase
            {
                MBOOL const success = true;
                mvPervious.front()->updateState(
                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
                mvPervious.erase(mvPervious.begin());
            }
            sp<IPipelineFrame> pFrameOld = mvFrames.front();
            mvFrames.erase(mvFrames.begin());
            IStreamBufferSet& rStreamBufferSet = pFrameOld->getStreamBufferSet();
            rStreamBufferSet.applyRelease(getNodeId());
            MY_LOGD_IF(mLogLevel >= 1, "Free old img3o handle, mvPervious=%d, mvFrames=%d",
                    mvPervious.size(), mvFrames.size());
        }

        mvPervious.clear();
        mvFrames.clear();
    }
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mRequestQueueLock);

    //  Make sure the request with a smaller frame number has a higher priority.
    Que_T::iterator it = mRequestQueue.end();
    for (; it != mRequestQueue.begin(); ) {
        --it;
        if  ( 0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()) ) {
            ++it;   //insert(): insert before the current node
            break;
        }
    }

    mRequestQueue.insert(it, pFrame);

    mRequestQueueCond.signal();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
onDequeRequest(
    android::sp<IPipelineFrame>& rpFrame
)
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    //
    //  Wait until the queue is not empty or not going exit
    while ( mRequestQueue.empty() && ! mbRequestExit )
    {
        // set dained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.signal();
        //
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu",
                status, ::strerror(-status), mRequestQueue.size()
            );
        }
    }
    //
    if  ( mbRequestExit ) {
        MY_LOGW_IF(!mRequestQueue.empty(), "[flush] mRequestQueue.size:%zu", mRequestQueue.size());
//        MY_LOGW_IF(1, "mkdbg: [flush] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }
    //
    //  Here the queue is not empty, take the first request from the queue.
    mbRequestDrained = MFALSE;
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    //
    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
onProcessFrameReturnRet(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNC_START;
    MERROR ret = OK;
    MY_LOGD_IF(mLogLevel >= 1, "frame %zu +", pFrame->getFrameNo());

    // 1. get IOMap
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if( OK != getInfoIOMapSet(pFrame, IOMapSet) )
    {
        MY_LOGE("queryInfoIOMap failed");
        return BAD_VALUE;
    }

    // 2. create metadata handle (based on IOMap)
    sp<MetaHandleA> pMeta_InA  = mpInAppMeta_Request.get() ?
        MetaHandleA::create(this, pFrame, mpInAppMeta_Request->getStreamId()) : NULL;
    sp<MetaHandleA> pMeta_InRetA  = mpInAppRetMeta_Request.get() ?
        MetaHandleA::create(this, pFrame, mpInAppRetMeta_Request->getStreamId()) : NULL;
    sp<MetaHandleA> pMeta_InH  = mpInHalMeta_P1.get() ?
        MetaHandleA::create(this, pFrame, mpInHalMeta_P1->getStreamId()) : NULL;
    sp<MetaHandleA> pMeta_OutA = mpOutAppMeta_Result.get() ?
        MetaHandleA::create(this, pFrame, mpOutAppMeta_Result->getStreamId()) : NULL;
    sp<MetaHandleA> pMeta_OutH = mpOutHalMeta_Result.get() ?
        MetaHandleA::create(this, pFrame, mpOutHalMeta_Result->getStreamId()) : NULL;
    //
    if( pMeta_InA  == NULL || pMeta_InH  == NULL )
    {
        MY_LOGW("meta check failed");
        return BAD_VALUE;
    }

    // 4. process image IO
    IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = IOMapSet.mImageInfoIOMapSet;

    { // add process cnt
        Mutex::Autolock _l(mLock);
        //muProcessCnt = imageIOMapSet.size();
        mvProcessCnt.add(pFrame->getFrameNo(), imageIOMapSet.size());
        MY_LOGD_IF(mLogLevel >= 1, "frame %d, ProcessCnt=%d", pFrame->getFrameNo(), imageIOMapSet.size());
    }

    //in p2a case, the imageIOMapSet size always equal 1.(only rrzo in)
    for( size_t run_idx = 0 ; run_idx < imageIOMapSet.size(); run_idx++ )
    {
        sp<FrameParams> param_p2 = new FrameParams();
        //FrameParams param_p2;

        IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[run_idx];
        //

        MY_LOGD_IF(mLogLevel >= 1, "run_idx =%d, vIn streamId=%#" PRIxPTR "", run_idx, imageIOMap.vIn.keyAt(0));

        // source
        {
            StreamId_T const streamId = imageIOMap.vIn.keyAt(0);
            param_p2->in.mPortId = PORT_IMGI;
            param_p2->in.mHandle = StreamBufferHandleA::create(this, pFrame, streamId);
            //
            {
                RWLock::AutoRLock _l(mConfigRWLock);
                param_p2->bResized = isResizeRawLocked(streamId);
            }
        }
        // destination
        MUINT32 occupied = 0;
        for( size_t i = 0; i < imageIOMap.vOut.size(); i++ )
        {
            MY_LOGD_IF(mLogLevel >= 1, "run_idx =%d, vOut streamId=%#" PRIxPTR "", run_idx, imageIOMap.vOut.keyAt(i));

            StreamId_T const streamId = imageIOMap.vOut.keyAt(i);
            MUINT32 const transform = imageIOMap.vOut.valueAt(i)->getTransform();
            PortID port_p2;
            if( OK == mapPortId(streamId, transform, occupied, port_p2) ) {
                FrameOutput out;
                out.mPortId = port_p2;
                out.mHandle = StreamBufferHandleA::create(this, pFrame, streamId);
                out.mTransform = transform;
                //
                param_p2->vOut.push_back(out);
            }
            else
            {
                param_p2->pendingOut.mPortId = PORT_IMG3O;
                param_p2->pendingOut.mHandle = StreamBufferHandleA::create(this, pFrame, streamId);
                param_p2->pendingOut.mTransform = transform;
            }
        }
        MY_LOGD_IF(mLogLevel >= 1, "frame %zu, job: p2 out %d", pFrame->getFrameNo(), param_p2->vOut.size() );
        //
        param_p2->inApp = pMeta_InA;
        param_p2->inRetApp = pMeta_InRetA;
        param_p2->inHal = pMeta_InH;
        if( run_idx == 0 ) {
            param_p2->outApp = pMeta_OutA;
            param_p2->outHal = pMeta_OutH;
        }

        doProcess(pFrame, param_p2);
    } //end imageIOMapSet.size()



    FUNC_END;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    FUNC_START;

    if( OK != onProcessFrameReturnRet(pFrame) ) {
        MY_LOGW("onProcessFrame failed");
        BaseNode::flush(pFrame);
        return;
    }

    FUNC_END;
    return;
}



/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
doProcess(android::sp<IPipelineFrame> const& pFrame,
        sp<FrameParams> &param_p2)
{
    MERROR ret = OK;
    // prepare metadata
    IMetadata* pMeta_InApp  = param_p2->inApp->getMetadata();
    IMetadata* pMeta_InHal  = param_p2->inHal->getMetadata();
    IMetadata* pMeta_OutApp = param_p2->outApp.get() ? param_p2->outApp->getMetadata() : NULL;
    IMetadata* pMeta_OutHal = param_p2->outHal.get() ? param_p2->outHal->getMetadata() : NULL;

    if( pMeta_InApp == NULL || pMeta_InHal == NULL ) {
        MY_LOGE("meta: in app %p, in hal %p", pMeta_InApp, pMeta_InHal);
        return;
    }
    crop_infos cropInfos;
    if( OK != (ret = getCropInfos(pMeta_InApp, pMeta_InHal, param_p2->bResized, cropInfos)) ) {
        MY_LOGE("getCropInfos failed");
        return;
    }

    //
    QParams enqueParams;
    IImageBuffer* pSrc;
    // input
    {
        if( OK != (ret = param_p2->in.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW("src buffer err = %d", ret);
            return;
        }
        pSrc = param_p2->in.mHandle->getBuffer();
        //
        Input src;
        src.mPortID       = param_p2->in.mPortId;
        src.mPortID.group = 0;
        src.mBuffer       = pSrc;
        // update src size
        if( param_p2->bResized )
            pSrc->setExtParam(cropInfos.dstsize_resizer);

        MY_LOGD_IF(mLogLevel >= 1, "pSrc W/H(%d,%d)", cropInfos.dstsize_resizer.w, cropInfos.dstsize_resizer.h);
        //
        enqueParams.mvIn.push_back(src);
    }
    //
    // output
    for( size_t i = 0; i < param_p2->vOut.size(); i++ )
    {
        if( param_p2->vOut[i].mHandle == NULL ||
            OK != (ret = param_p2->vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE)) ) {
            MY_LOGW("dst buffer err = %d", ret);
            continue;
        }
        IImageBuffer* pDst = param_p2->vOut[i].mHandle->getBuffer();
        //
        Output dst;
        dst.mPortID       = param_p2->vOut[i].mPortId;
        dst.mPortID.group = 0;
        dst.mBuffer       = pDst;
        dst.mTransform    = param_p2->vOut[i].mTransform;
        MY_LOGD_IF(mLogLevel >= 1, "get out buffer, mPortID=%d", param_p2->vOut[i].mPortId);
        //
        enqueParams.mvOut.push_back(dst);
    }

    if( enqueParams.mvOut.size() == 0 ) {
        MY_LOGW("no dst buffer");
        return;
    }

    MY_LOGD_IF(mLogLevel >= 1, "pipe %p, 3A %p, IFeatureStream=%p, mp3dnr=%p, frameNo=%d",
        mpPipe, mp3A, mpIFeatureStream, mp3dnr, pFrame->getFrameNo());


    //deTuringQue and setIsp turning
    {
        void* pTuning = NULL;
        unsigned int tuningsize;
        if( mpPipe && (!mpPipe->deTuningQue(tuningsize, pTuning)) ) {
            MY_LOGE("cannot get tunning buffer");
            return;
        }
        else {
            MBOOL ret = mpIFeatureStream->deTuningQue(tuningsize, pTuning);
            if (pTuning == NULL){
                MY_LOGE("IFeatureStream cannot get tunning buffer, ret=%d, tuningsize=%d, pTuning=%p",
                    ret, tuningsize, pTuning);
                return;
            }
        }
        MY_LOGD_IF(mLogLevel >= 1, "deTuningQue tuningsize=%d, pTuning=%p", tuningsize, pTuning);
        //
        MetaSet_T inMetaSet;
        MetaSet_T outMetaSet;
        //
        inMetaSet.appMeta = *pMeta_InApp;
        inMetaSet.halMeta = *pMeta_InHal;
        //
        MBOOL const bGetResult = (pMeta_OutApp || pMeta_OutHal);
        //
        if( param_p2->bResized ) {
            updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
        } else {
            updateEntry<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
        }
        if( pMeta_OutHal ) {
            // FIX ME: getDebugInfo() @ setIsp() should be modified
            //outMetaSet.halMeta = *pMeta_InHal;
        }
        //
        mp3A->setIsp(0, inMetaSet, pTuning, bGetResult ? &outMetaSet : NULL);
        //
        if( pMeta_OutApp ) {
            *pMeta_OutApp = outMetaSet.appMeta;
            //
            MRect cropRegion = cropInfos.crop_a;
            if( cropInfos.isEisEabled ) {
                cropRegion.p.x += cropInfos.eis_mv_a.p.x;
                cropRegion.p.y += cropInfos.eis_mv_a.p.y;
            }
            //
            updateCropRegion(cropRegion, pMeta_OutApp);
        }
        //
        if( pMeta_OutHal ) {
            *pMeta_OutHal = outMetaSet.halMeta;
            *pMeta_OutHal += *pMeta_InHal;
        }
        //
        enqueParams.mvTuningData.push_back(pTuning);
    }

    //cal viewangle of all output buffers
    {
        Vector<Output>::const_iterator iter = enqueParams.mvOut.begin();
        while( iter != enqueParams.mvOut.end() ) {
            MCrpRsInfo crop;
            if( iter->mPortID == PORT_WDMAO ) {
                crop.mGroupID = 2;
                calcCrop_viewangle(cropInfos, iter->mBuffer->getImgSize(), crop.mCropRect);

                //add for align mdp crop
                if (param_p2->bResized)
                {
                    if( param_p2->pendingOut.mHandle == NULL ||
                        OK != param_p2->pendingOut.mHandle->waitState(BufferHandle::STATE_WRITABLE) ) {
                        MY_LOGE("3dnr buffer err");
                    }
                    else
                    {
                        sp<IImageBuffer> pImg3oBuf = param_p2->pendingOut.mHandle->getBuffer();

                        MY_LOGD_IF(mLogLevel >= 1,"pImg3oBuf address=%p, pImg3oBuf W/H(%d,%d), IMGI size(%d,%d), frameNo=%d",
                                pImg3oBuf.get(), pImg3oBuf->getImgSize().w, pImg3oBuf->getImgSize().h,
                                cropInfos.dstsize_resizer.w, cropInfos.dstsize_resizer.h, pFrame->getFrameNo());
                        pImg3oBuf->setExtParam(crop.mCropRect.s);
                    }
                }
            } else if ( iter->mPortID == PORT_WROTO ) {
                crop.mGroupID = 3;
                IImageBuffer* pBuf      = iter->mBuffer;
                MINT32 const transform  = iter->mTransform;
                MSize dstSize = ( transform == eTransform_ROT_90 || transform == eTransform_ROT_270 )
                                ? MSize(pBuf->getImgSize().h, pBuf->getImgSize().w)
                                : pBuf->getImgSize();
                calcCrop_viewangle(cropInfos, dstSize, crop.mCropRect);
#if FD_PORT_SUPPORT
            } else if ( iter->mPortID == PORT_IMG2O ) {
                crop.mGroupID = 1;
                calcCrop_viewangle(cropInfos, iter->mBuffer->getImgSize(), crop.mCropRect);
#endif
            } else {
                MY_LOGE("not supported port %p", iter->mPortID);
                return;
            }
            crop.mResizeDst = iter->mBuffer->getImgSize();
            enqueParams.mvCropRsInfo.push_back(crop);
            iter++;
        }
    }
    //
    // callback
    enqueParams.mpfnCallback = pass2CbFunc;
    enqueParams.mpCookie     = this;

    // FIXME: need this?
    enqueParams.mvPrivaData.push_back(NULL);

    // for crop
    enqueParams.mvP1SrcCrop.push_back(cropInfos.crop_p1_sensor);
    enqueParams.mvP1Dst.push_back(cropInfos.dstsize_resizer);
    enqueParams.mvP1DstCrop.push_back(cropInfos.crop_dma);

    //
    MY_LOGD_IF(mLogLevel >= 1, "cnt %d, in %d, out %d",
            muEnqueCnt, enqueParams.mvIn.size(), enqueParams.mvOut.size() );
    //
    { // add job to queue
        Mutex::Autolock _l(mLock);
        mvRunning.push_back(new FrameInfo(param_p2, pFrame));
        muEnqueCnt++;
    }


    //check IMGO input first
    if (!param_p2->bResized)
    {
        CAM_TRACE_NAME("drv_enq");
        if (mpPipe){
            ret = mpPipe->enque(enqueParams);
        }
        else{
            ret = mpIFeatureStream->enque(enqueParams);
        }
        if( !ret)
        {
            MY_LOGE("enque pass2 failed, mpPipe:%p, mpIFeatureStream:%p", mpPipe, mpIFeatureStream);
            {
                MY_LOGW("cnt %d execute failed", muDequeCnt);
                muDequeCnt++;
            }
        }
        return;
    }

    //RRZO
    if (NR3D_ENABLE)
    {
        Input input;
        Output img3oOut;
        ModuleInfo moduleinfo;
        ret = do3dnrFlow(pFrame, param_p2, cropInfos, enqueParams, input, img3oOut, moduleinfo);

        //mp3dnr->processFrame(enqueParams, pImg3oBuf, pPervious);
        CAM_TRACE_NAME("drv_enq");
        if( !mpIFeatureStream->enque(enqueParams) )
        {
            Mutex::Autolock _l(mLock);
            MY_LOGE("IFeatureStream enque pass2 failed");
            //
            {
                MY_LOGW("cnt %d execute failed", muDequeCnt);
                muDequeCnt++;
            }
            return;
        }

        // update the vipi image for next frame used.
        {
            Mutex::Autolock _l(mLock);
            if (!mvPervious.empty())
            {
                //david: print img3o output
                // char str[32];
                // sprintf(str, "/sdcard/IMG3O-%3d.yuv", pFrame->getFrameNo());
                // mvPervious.front()->getBuffer()->saveToFile(str);

                //free the previous steambuffer and do apply relase
                {
                    MBOOL const success = true;
                    mvPervious.front()->updateState(
                        success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
                    mvPervious.erase(mvPervious.begin());
                }
                sp<IPipelineFrame> pFrameOld = mvFrames.front();
                mvFrames.erase(mvFrames.begin());
                IStreamBufferSet& rStreamBufferSet = pFrameOld->getStreamBufferSet();
                rStreamBufferSet.applyRelease(getNodeId());
                MY_LOGD_IF(mLogLevel >= 1, "Free old img3o handle, mvPervious=%d, mvFrames=%d",
                            mvPervious.size(), mvFrames.size());
            }
            mvPervious.push_back(param_p2->pendingOut.mHandle);
            mvFrames.push_back(pFrame);
            MY_LOGD_IF(mLogLevel >= 1, "Add img3o handle, mvPervious=%d", mvPervious.size());
        }

    }
    else
    {
        Mutex::Autolock _l(mLock);
        //RRZO and use NormalStream
        CAM_TRACE_NAME("drv_enq");
        if( !mpPipe->enque(enqueParams) )
        {
            MY_LOGE("enque pass2 failed");
            //
            {
                MY_LOGW("cnt %d execute failed", muDequeCnt);
                muDequeCnt++;
            }
            return;
        }
    }
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
do3dnrFlow(android::sp<IPipelineFrame> const& pFrame,
        sp<FrameParams> &param_p2, crop_infos &cropInfos, QParams &enqueParams,
        Input &input, Output &img3oOut, ModuleInfo &moduleinfo)
{
    MERROR ret = OK;
    sp<BufferHandle> pPervious = NULL;
    sp<IImageBuffer> pImg3oBuf = NULL;
    NR3D* pNr3dParam;

    MY_LOGD_IF(mLogLevel >= 1, "3dnr pendingOut:%p", param_p2->pendingOut.mHandle.get());
    if( param_p2->pendingOut.mHandle == NULL ||
        OK != (ret = param_p2->pendingOut.mHandle->waitState(BufferHandle::STATE_WRITABLE)) ) {
        MY_LOGE("3dnr buffer err = %d", ret);
        return BAD_VALUE;
    }
    pImg3oBuf = param_p2->pendingOut.mHandle->getBuffer();
    //reset the IMG3O H/W
    pImg3oBuf->setExtParam(cropInfos.dstsize_resizer);

    MY_LOGD_IF(mLogLevel >= 1,"pImg3oBuf=%p, pImg3oBuf W/H(%d,%d), frameNo=%d",
            pImg3oBuf.get(), pImg3oBuf->getImgSize().w, pImg3oBuf->getImgSize().h, pFrame->getFrameNo());

    //get ISO from app in metadata
    MINT32 iso = 0;
    tryGetMetadata<MINT32>(param_p2->inRetApp->getMetadata(), MTK_SENSOR_SENSITIVITY, iso);
    if (MTRUE != mp3dnr->prepare(pFrame->getFrameNo(), iso))
    {
        MY_LOGW("3dnr prepare err");
    }
    //get GMV from metadata
    eis_region eisInfo;
    IMetadata* pMeta_InHal  = param_p2->inHal->getMetadata();
    queryEisRegion(pMeta_InHal, eisInfo);

    // if (MTRUE != mp3dnr->setGMV(pFrame->getFrameNo(), 0, 0,0,0))
    if (MTRUE != mp3dnr->setGMV(pFrame->getFrameNo(), eisInfo.gmvX, eisInfo.gmvY,
                            eisInfo.x_int, eisInfo.y_int))

    {
        MY_LOGW("3dnr getGMV err");
    }
    if (MTRUE != mp3dnr->checkIMG3OSize(pFrame->getFrameNo(), cropInfos.dstsize_resizer.w, cropInfos.dstsize_resizer.h))
    {
        MY_LOGW("3dnr checkIMG3OSize err");
    }

    if (mvPervious.size() > 0) pPervious = mvPervious.front();
    if (pPervious == NULL)
    {
        if (MTRUE != mp3dnr->setVipiParams(pPervious != NULL, 0, 0, 0, 0))
        {
            MY_LOGW("3dnr setVipiParams err");
        }
    }
    else
    {
        if (MTRUE != mp3dnr->setVipiParams(pPervious != NULL,
            pPervious->getBuffer()->getImgSize().w, pPervious->getBuffer()->getImgSize().h,
            pImg3oBuf->getImgFormat(), pPervious->getBuffer()->getBufStridesInBytes(0)))
        {
            MY_LOGD_IF(mLogLevel >= 1, "skip setVipiParams flow");
        }
        else
        {
            MY_LOGD_IF(mLogLevel >= 1,"setVipiParams: address:%p, W/H(%d,%d)", pPervious->getBuffer(),
                pPervious->getBuffer()->getImgSize().w, pPervious->getBuffer()->getImgSize().h);
            /* VIPI */
            input.mPortID = PORT_VIPI;
            input.mBuffer = mvPervious.front()->getBuffer();
            enqueParams.mvIn.push_back(input);
        }
    }

    if (MTRUE != mp3dnr->get3dnrParams(pFrame->getFrameNo(),
                pImg3oBuf->getImgSize().w, pImg3oBuf->getImgSize().h, pNr3dParam))
    {
        MY_LOGD_IF(mLogLevel >= 1, "skip get3dnrParams flow");
    }
    else
    {
        //set for nr3d module
        moduleinfo.moduleTag = EFeatureModule_NR3D;
        moduleinfo.moduleStruct   = reinterpret_cast<MVOID*> (pNr3dParam);
        enqueParams.mvModuleData.push_back(moduleinfo);

    }
    MY_LOGD_IF(mLogLevel >= 1,"pNr3dParam: onOff_onOfStX/Y(%d, %d), onSiz_onW/H(%d, %d), vipi_readW/H(%d, %d)",
        pNr3dParam->onOff_onOfStX, pNr3dParam->onOff_onOfStY,
        pNr3dParam->onSiz_onWd, pNr3dParam->onSiz_onHt,
        pNr3dParam->vipi_readW, pNr3dParam->vipi_readH);

    if (MTRUE == mp3dnr->checkStateMachine(NR3D_STATE_WORKING))
    {
        //Config IMG3O
        img3oOut.mPortID = PORT_IMG3O;
        img3oOut.mBuffer = param_p2->pendingOut.mHandle->getBuffer();
        enqueParams.mvOut.push_back(img3oOut);
        MY_LOGD_IF(mLogLevel >= 1, "config img3o, mvOut=%d", enqueParams.mvOut.size());
    }
    return OK;
}



/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
waitForRequestDrained()
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    if( !mbRequestDrained) {
        if (! mRequestQueue.empty()) // mkdbg: no need to wait if queue is empty
        {
            MY_LOGD("mkdbg: bbb_wait for request drained: mbRequestExit: %d", mbRequestExit);
            mbRequestDrainedCond.wait(mRequestQueueLock);
            MY_LOGD("mkdbg: aaa_wait for request drained: mbRequestExit: %d", mbRequestExit);
        }
    }
    //
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
getImageBufferAndLock(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer,
    MBOOL const isInStream
)
{
    IStreamBufferSet&      rStreamBufferSet = pFrame->getStreamBufferSet();
    sp<IImageBufferHeap>   pImageBufferHeap = NULL;
    MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            rpStreamBuffer
            );

    if( err != OK )
        return err;
    //
    //  Query the group usage.
    MUINT const groupUsage = rpStreamBuffer->queryGroupUsage(getNodeId());

    if(isInStream){
        pImageBufferHeap = rpStreamBuffer->tryReadLock(getNodeName());
    }
    else{
        pImageBufferHeap = rpStreamBuffer->tryWriteLock(getNodeName());
    }

    if (pImageBufferHeap == NULL) {
        MY_LOGE("pImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    MY_LOGD("@pImageBufferHeap->getBufSizeInBytes(0) = %d", pImageBufferHeap->getBufSizeInBytes(0));
    rpImageBuffer = pImageBufferHeap->createImageBuffer();

    if (rpImageBuffer == NULL) {
        MY_LOGE("rpImageBuffer == NULL");
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    //MY_LOGD("stream buffer: (%p) %p, heap: %p, buffer: %p, usage: %p",
    //    streamId, rpStreamBuffer.get(), pImageBufferHeap.get(), rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
threadSetting()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    MINT tid;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (P2THREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, P2THREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, P2THREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = P2THREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, P2THREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), P2THREAD_POLICY, P2THREAD_PRIORITY);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
void
P2aNodeImp::UnpackThread::
requestExit()
{
    //TODO: refine this
    Mutex::Autolock _l(mpNodeImp->mRequestQueueLock);
    mpNodeImp->mbRequestExit = MTRUE;
    mpNodeImp->mRequestQueueCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
P2aNodeImp::UnpackThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
P2aNodeImp::UnpackThread::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            !exitPending()
        &&  OK == mpNodeImp->onDequeRequest(pFrame)
        &&  pFrame != 0
        )
    {
        mpNodeImp->onProcessFrame(pFrame);

        return true;
    }

    MY_LOGD("exit unpack thread");
    return  false;

}


MERROR
P2aNodeImp::
verifyConfigParams(
    ConfigParams const & rParams
) const
{
    if  ( ! rParams.pInAppMeta.get() ) {
        MY_LOGE("no in app meta");
        return BAD_VALUE;
    }
    if  ( ! rParams.pInHalMeta.get() ) {
        MY_LOGE("no in hal meta");
        return BAD_VALUE;
    }
    //if  ( ! rParams.pOutAppMeta.get() ) {
    //    return BAD_VALUE;
    //}
    //if  ( ! rParams.pOutHalMeta.get() ) {
    //    return BAD_VALUE;
    //}
    if  (  rParams.pvInFullRaw.size() == 0
            && ! rParams.pInResizedRaw.get() ) {
        MY_LOGE("no in image fullraw or resized raw");
        return BAD_VALUE;
    }
    if  (  0 == rParams.vOutImage.size() && !rParams.pOutFDImage.get() ) {
        MY_LOGE("no out yuv image");
        return BAD_VALUE;
    }
    //
#define dumpStreamIfExist(str, stream)                         \
    do {                                                       \
        MY_LOGD_IF(stream.get(), "%s: id %#" PRIxPTR ", %s",     \
                str,                                           \
                stream->getStreamId(), stream->getStreamName() \
               );                                              \
    } while(0)
    //
    dumpStreamIfExist("[meta] in app", rParams.pInAppMeta);
    dumpStreamIfExist("[meta] in app ret", rParams.pInAppRetMeta);
    dumpStreamIfExist("[meta] in hal", rParams.pInHalMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);
    for( size_t i = 0; i < rParams.pvInFullRaw.size(); i++ ) {
        dumpStreamIfExist("[img] in full", rParams.pvInFullRaw[i]);
    }
    dumpStreamIfExist("[img] in resized", rParams.pInResizedRaw);
    for( size_t i = 0; i < rParams.vOutImage.size(); i++ ) {
        dumpStreamIfExist("[img] out yuv", rParams.vOutImage[i]);
    }
    dumpStreamIfExist("[img] out fd", rParams.pOutFDImage);
#undef dumpStreamIfExist
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
getInfoIOMapSet(
    sp<IPipelineFrame> const& pFrame,
    IPipelineFrame::InfoIOMapSet& rIOMapSet
) const
{
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), rIOMapSet ) ) {
        MY_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }
    //
    // do some check
    IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = rIOMapSet.mImageInfoIOMapSet;
    if( ! imageIOMapSet.size() ) {
        MY_LOGW("no imageIOMap in frame");
        return BAD_VALUE;
    }
    //
    for( size_t i = 0; i < imageIOMapSet.size(); i++ ) {
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[i];
        if( imageIOMap.vIn.size() != 1 || imageIOMap.vOut.size() == 0) {
            MY_LOGE("[img] #%d wrong size vIn %d, vOut %d",
                    i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
            return BAD_VALUE;
        }
        MY_LOGD_IF(mLogLevel >= 1, "frame %zu:[img] #%zu, in %d, out %d",
                pFrame->getFrameNo(), i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
    }
    //
    IPipelineFrame::MetaInfoIOMapSet& metaIOMapSet = rIOMapSet.mMetaInfoIOMapSet;
    if( ! metaIOMapSet.size() ) {
        MY_LOGW("no metaIOMap in frame");
        return BAD_VALUE;
    }
    //
    for( size_t i = 0; i < metaIOMapSet.size(); i++ ) {
        IPipelineFrame::MetaInfoIOMap const& metaIOMap = metaIOMapSet[i];
        if( !mpInAppMeta_Request.get() ||
                0 > metaIOMap.vIn.indexOfKey(mpInAppMeta_Request->getStreamId()) ) {
            MY_LOGE("[meta] no in app");
            return BAD_VALUE;
        }
        if( !mpInHalMeta_P1.get() ||
                0 > metaIOMap.vIn.indexOfKey(mpInHalMeta_P1->getStreamId()) ) {
            MY_LOGE("[meta] no in hal");
            return BAD_VALUE;
        }
        MY_LOGD_IF(1, "frame %zu:[meta] #%zu: in %d, out %d",
                pFrame->getFrameNo(), i, metaIOMap.vIn.size(), metaIOMap.vOut.size());
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2aNodeImp::
isInImageStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    //
    if( isFullRawLocked(streamId) || isResizeRawLocked(streamId) )
        return MTRUE;
    //
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2aNodeImp::
isInMetaStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    return isStream(mpInAppMeta_Request, streamId) ||
            isStream(mpInAppRetMeta_Request, streamId) ||
            isStream(mpInHalMeta_P1, streamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
acquireImageStream(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer
)
{
    return ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            pFrame->getStreamBufferSet(),
            rpStreamBuffer
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
releaseImageStream(
    sp<IPipelineFrame> const& pFrame,
    sp<IImageStreamBuffer> const pStreamBuffer,
    MUINT32 const status
) const
{
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();
    //
    if( pStreamBuffer == NULL ) {
        MY_LOGE("pStreamBuffer == NULL");
        return;
    }
    //
    if( isInImageStream(streamId) ) {
        pStreamBuffer->markStatus(
                (status&eStreamStatus_ERROR) ?
                STREAM_BUFFER_STATUS::WRITE_ERROR :
                STREAM_BUFFER_STATUS::WRITE_OK
                );
    }
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    streamBufferSet.markUserStatus(
            streamId,
            getNodeId(),
            ((status != eStreamStatus_NOT_USED) ? IUsersManager::UserStatus::USED : 0) |
            IUsersManager::UserStatus::RELEASE
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
acquireImageBuffer(
    StreamId_T const streamId,
    sp<IImageStreamBuffer> const pStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer
) const
{
    if( pStreamBuffer == NULL ) {
        MY_LOGE("pStreamBuffer == NULL");
        return BAD_VALUE;
    }
    //  Query the group usage.
    MUINT const groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
    sp<IImageBufferHeap>   pImageBufferHeap =
        isInImageStream(streamId) ?
        pStreamBuffer->tryReadLock(getNodeName()) :
        pStreamBuffer->tryWriteLock(getNodeName());

    if (pImageBufferHeap == NULL) {
        MY_LOGE("[node:%d][stream buffer:%s] cannot get ImageBufferHeap",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (rpImageBuffer == NULL) {
        MY_LOGE("[node:%d][stream buffer:%s] cannot create ImageBuffer",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }
    //david hark first, have to set groupUsage on UserManager
    // if (!groupUsage)
    //     groupUsage = 0x20003;
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGD_IF(mLogLevel >= 1, "stream %#" PRIxPTR ": buffer: %p, usage: %p",
        streamId, rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
releaseImageBuffer(
    sp<IImageStreamBuffer> const pStreamBuffer,
    sp<IImageBuffer> const pImageBuffer
) const
{
    if( pStreamBuffer == NULL || pImageBuffer == NULL )
    {
        MY_LOGE("pStreamBuffer || pImageBuffer should not be NULL");
        return;
    }
    //
    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
acquireMetaStream(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>& rpStreamBuffer
)
{
    return ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            pFrame->getStreamBufferSet(),
            rpStreamBuffer
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
releaseMetaStream(
    android::sp<IPipelineFrame> const& pFrame,
    sp<IMetaStreamBuffer> const pStreamBuffer,
    MUINT32 const status
) const
{
    IStreamBufferSet&     rStreamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();
    //
    if( pStreamBuffer.get() == NULL ) {
        MY_LOGE("StreamId %d: pStreamBuffer == NULL",
                streamId);
        return;
    }
    //
    //Buffer Producer must set this status.
    if( !isInMetaStream(streamId) ) {
        pStreamBuffer->markStatus(
                (status&eStreamStatus_ERROR) ?
                STREAM_BUFFER_STATUS::WRITE_ERROR :
                STREAM_BUFFER_STATUS::WRITE_OK
                );
    }
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
            streamId, getNodeId(),
            ((status != eStreamStatus_NOT_USED) ? IUsersManager::UserStatus::USED : 0) |
            IUsersManager::UserStatus::RELEASE
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
acquireMetadata(
    StreamId_T const streamId,
    sp<IMetaStreamBuffer> const pStreamBuffer,
    IMetadata*& rpMetadata
) const
{
    rpMetadata = isInMetaStream(streamId) ?
        pStreamBuffer->tryReadLock(getNodeName()) :
        pStreamBuffer->tryWriteLock(getNodeName());

    if( rpMetadata == NULL ) {
        MY_LOGE("[node:%d][stream buffer:%s] cannot get metadata",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGD_IF(0,"stream %#" PRIxPTR ": stream buffer %p, metadata: %p",
        streamId, pStreamBuffer.get(), rpMetadata);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
releaseMetadata(
    sp<IMetaStreamBuffer> const pStreamBuffer,
    IMetadata* const pMetadata
) const
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    pStreamBuffer->unlock(getNodeName(), pMetadata);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
onPartialFrameDone(
    sp<IPipelineFrame> const& pFrame
)
{
    CAM_TRACE_CALL();
    //FUNC_START;
    IStreamBufferSet&     rStreamBufferSet = pFrame->getStreamBufferSet();
    rStreamBufferSet.applyRelease(getNodeId());
    //FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
onFrameDone(
    sp<IPipelineFrame> const& pFrame
)
{
    CAM_TRACE_CALL();
    //MY_LOGD("frame %u done", pFrame->getFrameNo());
    onDispatchFrame(pFrame);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
pass2CbFunc(QParams& rParams)
{
    P2aNodeImp* pP2aImplObj = reinterpret_cast<P2aNodeImp*>(rParams.mpCookie);
    pP2aImplObj->handleDeque(rParams);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
handleDeque(QParams& rParams)
{
    CAM_TRACE_NAME("p2 deque");
    sp<FrameInfo> pFrameInfo = NULL;
    MUINT processCnt = 0;

    {
        Mutex::Autolock _l(mLock);
        MY_LOGD_IF(mLogLevel >= 1, "p2 done %d, success %d", muDequeCnt, rParams.mDequeSuccess);
        pFrameInfo = mvRunning.front();
        processCnt = mvProcessCnt.valueFor(pFrameInfo->mpFrame->getFrameNo());
        MY_LOGD_IF(1 >= 1, "frame %zu, p2 done %d, success %d, processCnt=%d",
            pFrameInfo->mpFrame->getFrameNo(), muDequeCnt, rParams.mDequeSuccess, processCnt);
        mvRunning.erase(mvRunning.begin());
        muDequeCnt++;
    }


    // MY_LOGD("mvPervious size=%d, img3o=%p, frameNo=%d", mvPervious.size(),
    //     pFrameInfo->mpParams->pendingOut.mHandle.get(), pFrameInfo->mpFrame->getFrameNo());
    // {
    //     Mutex::Autolock _l(mLock);
    //     if (mvPervious.size() > 1)
    //     {
    //         //david: print img3o output
    //         // char str[32];
    //         // sprintf(str, "/sdcard/IMG3O-%3d.yuv", pFrame->getFrameNo());
    //         // mvPervious.front()->getBuffer()->saveToFile(str);

    //         //free the previous steambuffer and do apply relase
    //         {
    //             MBOOL const success = true;
    //             mvPervious.front()->updateState(
    //                 success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL);
    //             mvPervious.erase(mvPervious.begin());
    //         }
    //         sp<IPipelineFrame> pFrameOld = mvFrames.front();
    //         mvFrames.erase(mvFrames.begin());
    //         IStreamBufferSet& rStreamBufferSet = pFrameOld->getStreamBufferSet();
    //         rStreamBufferSet.applyRelease(getNodeId());
    //         MY_LOGD("Free old img3o handle, mvPervious=%d, mvFrames=%d",
    //             mvPervious.size(), mvFrames.size());
    //     }
    //     // mvPervious.push_back(pFrameInfo->mpParams->pendingOut.mHandle);
    //     // mvFrames.push_back(pFrameInfo->mpFrame);
    //     // MY_LOGD("Add img3o handle, mvPervious=%d, frameNo=%d", mvPervious.size(), pFrameInfo->mpFrame->getFrameNo());
    // }



    //
    if( rParams.mvTuningData.size() > 0 )
    {
        void* pTuning = rParams.mvTuningData[0];
        if( pTuning )
        {
            if(mpPipe) mpPipe->enTuningQue(pTuning);
            else {
                mpIFeatureStream->enTuningQue(pTuning);
            }
        }
    }

    //try to destruct StreamBufferHandleA
    //for markStatus to WRITE_OK or WRITE_ERROR and markUserStatus to USED and RELEASE
    //also for unlockBuf IImageStreamBuffer and unlock IImageBuffer

    //onFinish //~ProcessorJob
    MBOOL const success = true;
    for( size_t i = 0; i < pFrameInfo->mpParams->vOut.size(); i++ )
        pFrameInfo->mpParams->vOut[i].mHandle->updateState(
                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
                );
    if( pFrameInfo->mpParams->outApp.get() )
        pFrameInfo->mpParams->outApp->updateState(success ? MetaHandleA::STATE_WRITE_OK : MetaHandleA::STATE_WRITE_FAIL);
    if( pFrameInfo->mpParams->outHal.get() )
        pFrameInfo->mpParams->outHal->updateState(success ? MetaHandleA::STATE_WRITE_OK : MetaHandleA::STATE_WRITE_FAIL);

    // to release any sp<>
    pFrameInfo->mpParams = NULL;
    //~ProcessorJob  //do applyRelease
    onPartialFrameDone(pFrameInfo->mpFrame);

    { // add process cnt
        Mutex::Autolock _l(mLock);
        processCnt--;
        mvProcessCnt.replaceValueFor(pFrameInfo->mpFrame->getFrameNo(), processCnt);
        //muProcessCnt--;
        if (!processCnt)
        {
            MY_LOGD_IF(mLogLevel >= 1, "frame %zu onFrameDone start", pFrameInfo->mpFrame->getFrameNo());
            ssize_t ret = mvProcessCnt.removeItem(pFrameInfo->mpFrame->getFrameNo());
            //~FrameLifeControl //do onDispatchFrame
            onFrameDone(pFrameInfo->mpFrame);
        }
    }

    MY_LOGD_IF(mLogLevel >= 1, "frame %zu handleDeque -", pFrameInfo->mpFrame->getFrameNo());
}



/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
mapPortId(
    StreamId_T const streamId, // [in]
    MUINT32 const transform,   // [in]
    MUINT32& rOccupied,        // [in/out]
    PortID&  rPortId           // [out]
) const
{
    MERROR ret = OK;
#define PORT_WDMAO_USED  (0x1)
#define PORT_WROTO_USED  (0x2)
#define PORT_IMG2O_USED  (0x4)
    if( transform != 0 ) {
        if( !(rOccupied & PORT_WROTO_USED) ) {
            rPortId = PORT_WROTO;
            rOccupied |= PORT_WROTO_USED;
        }
        else
            ret = INVALID_OPERATION;
    }
    else {

#if NR3D_ENABLE
        if( NR3D_ENABLE && isStream(mpOut3dnr, streamId) ) {
            MY_LOGD_IF(mLogLevel >= 1, "stream id %#" PRIxPTR ", occupied %p",
            streamId, rOccupied);
            return INVALID_OPERATION;
        }
#endif
#if FD_PORT_SUPPORT
        if( FD_PORT_SUPPORT && isStream(mpOutFd, streamId) ) {
            if( rOccupied & PORT_IMG2O_USED ) {
                MY_LOGW("should not be occupied");
                ret = INVALID_OPERATION;
            } else
                rOccupied |= PORT_IMG2O_USED;
                rPortId = PORT_IMG2O;
        } else
#endif
        if( !(rOccupied & PORT_WDMAO_USED) ) {
            rOccupied |= PORT_WDMAO_USED;
            rPortId = PORT_WDMAO;
        } else if( !(rOccupied & PORT_WROTO_USED) ) {
            rOccupied |= PORT_WROTO_USED;
            rPortId = PORT_WROTO;
        } else
            ret = INVALID_OPERATION;
    }
    // MY_LOGD_IF(1, "stream id %#" PRIxPTR ", occupied %p",
    //         streamId, rOccupied);
    return ret;
#undef PORT_WDMAO_USED
#undef PORT_WROTO_USED
#undef PORT_IMG2O_USED
}



/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2aNodeImp::
getCropInfos(
    IMetadata* const inApp,
    IMetadata* const inHal,
    MBOOL const isResized,
    crop_infos & cropInfos
) const
{
    if( ! tryGetMetadata<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, cropInfos.sensor_size) ) {
        MY_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        return BAD_VALUE;
    }
    //
    MSize const sensor = cropInfos.sensor_size;
    MSize const active = mActiveArray.s;
    //
    cropInfos.isResized = isResized;
    // get current p1 buffer crop status
    if(
            !( tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, cropInfos.crop_p1_sensor) &&
               tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE      , cropInfos.dstsize_resizer) &&
               tryGetMetadata<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION   , cropInfos.crop_dma)
             )
      ) {
        MY_LOGW_IF(1, "[FIXME] should sync with p1 for rrz setting");
        //
        cropInfos.crop_p1_sensor  = MRect( MPoint(0,0), sensor );
        cropInfos.dstsize_resizer = sensor;
        cropInfos.crop_dma        = MRect( MPoint(0,0), sensor );
    }

    MY_LOGD_IF(mLogLevel >= 1, "size check: active:(%dx%d), p1 sensor:start(%d,%d),(%dx%d), dstsize(%dx%d)",
            mActiveArray.s.w, mActiveArray.s.h,
            cropInfos.crop_p1_sensor.p.x, cropInfos.crop_p1_sensor.p.y,
            cropInfos.crop_p1_sensor.s.w, cropInfos.crop_p1_sensor.s.h,
            cropInfos.dstsize_resizer.w, cropInfos.dstsize_resizer.h);
    //
    // setup transform
    cropInfos.tranActive2Sensor = simpleTransform(
                MPoint(0,0),
                active,
                sensor
            );
    //
    cropInfos.tranSensor2Resized = simpleTransform(
                cropInfos.crop_p1_sensor.p,
                cropInfos.crop_p1_sensor.s,
                cropInfos.dstsize_resizer
            );
    //
    MBOOL const isEisOn = isEISOn(inApp);
    //
    MRect cropRegion; //active array domain
    queryCropRegion(inApp, isEisOn, cropRegion);
    cropInfos.crop_a = cropRegion;
    //
    // query EIS result
    {
        eis_region eisInfo;
        if( isEisOn && queryEisRegion(inHal, eisInfo)) {
            cropInfos.isEisEabled = MTRUE;
            // calculate mv
            vector_f* pMv_s = &cropInfos.eis_mv_s;
            vector_f* pMv_r = &cropInfos.eis_mv_r;
#if 0
            //eis in sensor domain
            pMv_s->p.x  = eisInfo.x_int - (sensor.w * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_s->pf.x = eisInfo.x_float;
            pMv_s->p.y  = eisInfo.y_int - (sensor.h * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_s->pf.y = eisInfo.y_float;
            //
            cropInfos.eis_mv_r = transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_s);
            //
            MY_LOGD_IF(1, "mv (s->r): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
                    pMv_s->p.x,
                    pMv_s->pf.x,
                    pMv_s->p.y,
                    pMv_s->pf.y,
                    pMv_r->p.x,
                    pMv_r->pf.x,
                    pMv_r->p.y,
                    pMv_r->pf.y
                    );
#else
            MSize const resizer = cropInfos.dstsize_resizer;
            //eis in resized domain
            pMv_r->p.x  = eisInfo.x_int - (resizer.w * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_r->pf.x = eisInfo.x_float;
            pMv_r->p.y  = eisInfo.y_int - (resizer.h * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_r->pf.y = eisInfo.y_float;
            //
            cropInfos.eis_mv_s = inv_transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_r);
            //
            MY_LOGD_IF(1, "mv (r->s): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
                    pMv_r->p.x,
                    pMv_r->pf.x,
                    pMv_r->p.y,
                    pMv_r->pf.y,
                    pMv_s->p.x,
                    pMv_s->pf.x,
                    pMv_s->p.y,
                    pMv_s->pf.y
                    );
#endif
            cropInfos.eis_mv_a = inv_transform(cropInfos.tranActive2Sensor, cropInfos.eis_mv_s);
            MY_LOGD_IF(1, "mv in active %d/%d, %d/%d",
                    cropInfos.eis_mv_a.p.x,
                    cropInfos.eis_mv_a.pf.x,
                    cropInfos.eis_mv_a.p.y,
                    cropInfos.eis_mv_a.pf.y
                    );
        }
        else {
            cropInfos.isEisEabled = MFALSE;
            //
            // no need to set 0
            //memset(&cropInfos.eis_mv_a, 0, sizeof(vector_f));
            //memset(&cropInfos.eis_mv_s, 0, sizeof(vector_f));
            //memset(&cropInfos.eis_mv_r, 0, sizeof(vector_f));
        }
    }
    // debug
    //cropInfos.dump();
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
queryCropRegion(
    IMetadata* const meta_request,
    MBOOL const isEisOn,
    MRect& cropRegion
) const
{
    if( !tryGetMetadata<MRect>(meta_request, MTK_SCALER_CROP_REGION, cropRegion) ) {
        cropRegion.p = MPoint(0,0);
        cropRegion.s = mActiveArray.s;
        MY_LOGW_IF(mLogLevel >= 1, "no MTK_SCALER_CROP_REGION, crop full size %dx%d",
                cropRegion.s.w, cropRegion.s.h);
    }
    MY_LOGD_IF(mLogLevel >= 1, "control: cropRegion(%d, %d, %dx%d)",
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
    //
    if( isEisOn ) {
        cropRegion.p.x += (cropRegion.s.w * (EIS_FACTOR-100)/2/EIS_FACTOR);
        cropRegion.p.y += (cropRegion.s.h * (EIS_FACTOR-100)/2/EIS_FACTOR);
        cropRegion.s   = cropRegion.s * 100 / EIS_FACTOR;
        MY_LOGD_IF(mLogLevel >= 1, "EIS: factor %d, cropRegion(%d, %d, %dx%d)",
                EIS_FACTOR,
                cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
updateCropRegion(
    MRect const crop,
    IMetadata* meta_result
) const
{
    updateEntry<MRect>(meta_result, MTK_SCALER_CROP_REGION, crop);
    //
    MY_LOGD_IF( DEBUG_LOG && mLogLevel >= 1, "result: cropRegion (%d, %d, %dx%d)",
            crop.p.x, crop.p.y, crop.s.w, crop.s.h);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2aNodeImp::
calcCrop_viewangle(
    crop_infos const& cropInfos,
    MSize const& dstSize,
    MCropRect& result
) const
{
    MBOOL const isResized = cropInfos.isResized;
    // coordinates: s_: sensor
    MRect const s_crop = transform(cropInfos.tranActive2Sensor, cropInfos.crop_a);
    MRect s_viewcrop;
    //
    if( s_crop.s.w * dstSize.h > s_crop.s.h * dstSize.w ) { // pillarbox
        s_viewcrop.s.w = div_round(s_crop.s.h * dstSize.w, dstSize.h);
        s_viewcrop.s.h = s_crop.s.h;
        s_viewcrop.p.x = s_crop.p.x + ((s_crop.s.w - s_viewcrop.s.w) >> 1);
        s_viewcrop.p.y = s_crop.p.y;
    }
    else { // letterbox
        s_viewcrop.s.w = s_crop.s.w;
        s_viewcrop.s.h = div_round(s_crop.s.w * dstSize.h, dstSize.w);
        s_viewcrop.p.x = s_crop.p.x;
        s_viewcrop.p.y = s_crop.p.y + ((s_crop.s.h - s_viewcrop.s.h) >> 1);
    }
    MY_LOGD_IF(0, "s_cropRegion(%d, %d, %dx%d), dst %dx%d, view crop(%d, %d, %dx%d)",
            s_crop.p.x     , s_crop.p.y     ,
            s_crop.s.w     , s_crop.s.h     ,
            dstSize.w      , dstSize.h      ,
            s_viewcrop.p.x , s_viewcrop.p.y ,
            s_viewcrop.s.w , s_viewcrop.s.h
           );
    //
    if( isResized ) {
        MRect r_viewcrop = transform(cropInfos.tranSensor2Resized, s_viewcrop);
        result.s            = r_viewcrop.s;
        result.p_integral   = r_viewcrop.p + cropInfos.eis_mv_r.p;
        result.p_fractional = cropInfos.eis_mv_r.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if( refineBoundary(cropInfos.dstsize_resizer, result) ) {
            MY_LOGE("[FIXME] need to check crop!");
            cropInfos.dump();
        }
    }
    else {
        result.s            = s_viewcrop.s;
        result.p_integral   = s_viewcrop.p + cropInfos.eis_mv_s.p;
        result.p_fractional = cropInfos.eis_mv_s.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if( refineBoundary(cropInfos.sensor_size, result) ) {
            MY_LOGE("[FIXME] need to check crop!");
            cropInfos.dump();
        }
    }
    //
    MY_LOGD_IF(mLogLevel >= 1, "resized %d, crop %d/%d, %d/%d, %dx%d",
            isResized,
            result.p_integral.x,
            result.p_integral.y,
            result.p_fractional.x,
            result.p_fractional.y,
            result.s.w,
            result.s.h
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2aNodeImp::
isEISOn(
    IMetadata* const inApp
) const
{
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if( !tryGetMetadata<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode) ) {
        MY_LOGW_IF(mLogLevel >= 1, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }
#if FORCE_EIS_ON
    eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
#endif
    return eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2aNodeImp::
queryEisRegion(
    IMetadata* const inHal,
    eis_region& region
) const
{
    IMetadata::IEntry entry = inHal->entryFor(MTK_EIS_REGION);
    if( entry.count() == 8 ) {
        region.x_int   = entry.itemAt(0, Type2Type<MINT32>());
        region.x_float = entry.itemAt(1, Type2Type<MINT32>());
        region.y_int   = entry.itemAt(2, Type2Type<MINT32>());
        region.y_float = entry.itemAt(3, Type2Type<MINT32>());
        region.s.w     = entry.itemAt(4, Type2Type<MINT32>());
        region.s.h     = entry.itemAt(5, Type2Type<MINT32>());
        region.gmvX   = entry.itemAt(6, Type2Type<MINT32>());
        region.gmvY   = entry.itemAt(7, Type2Type<MINT32>());
        MY_LOGD_IF(mLogLevel >= 1, "%d, %d, %d, %d, %dx%d, gmv:%dx%d",
                region.x_int,
                region.x_float,
                region.y_int,
                region.y_float,
                region.s.w,
                region.s.h,
                region.gmvX,
                region.gmvY);
        return MTRUE;
    }
    //
    MY_LOGW("wrong eis region count %zu", entry.count());
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
inline
MBOOL
P2aNodeImp::
refineBoundary(
    MSize const& bufSize,
    MCropRect& crop
) const
{
    MBOOL isRefined = MFALSE;
    MCropRect refined = crop;
    if( crop.p_integral.x < 0 ) {
        refined.p_integral.x = 0;
        isRefined = MTRUE;
    }
    if( crop.p_integral.y < 0 ) {
        refined.p_integral.y = 0;
        isRefined = MTRUE;
    }
    //
    int const carry_x = (crop.p_fractional.x != 0) ? 1 : 0;
    if( (refined.p_integral.x + crop.s.w + carry_x) > bufSize.w ) {
        refined.s.w = bufSize.w - refined.p_integral.x - carry_x;
        isRefined = MTRUE;
    }
    int const carry_y = (crop.p_fractional.y != 0) ? 1 : 0;
    if( (refined.p_integral.y + crop.s.h + carry_y) > bufSize.h ) {
        refined.s.h = bufSize.h - refined.p_integral.y - carry_y;
        isRefined = MTRUE;
    }
    //
    if( isRefined ) {
        MY_LOGE("buf size %dx%d, crop(%d/%d, %d/%d, %dx%d) -> crop(%d/%d, %d/%d, %dx%d)",
                bufSize.w, bufSize.h,
                crop.p_integral.x,
                crop.p_integral.y,
                crop.p_fractional.x,
                crop.p_fractional.y,
                crop.s.w,
                crop.s.h,
                refined.p_integral.x,
                refined.p_integral.y,
                refined.p_fractional.x,
                refined.p_fractional.y,
                refined.s.w,
                refined.s.h
                );
        crop = refined;
    }
    return isRefined;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<MetaHandleA>
MetaHandleA::
create(
    StreamControl* const pCtrl,
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId
)
{
    //MY_LOGD_IF(1, "metadata handle, streamId=%#" PRIxPTR "", streamId);

    // check StreamBuffer here
    sp<IMetaStreamBuffer> pStreamBuffer = NULL;
    if( pCtrl && OK == pCtrl->acquireMetaStream(
                pFrame,
                streamId,
                pStreamBuffer) )
    {
        IMetadata* pMeta = NULL;
        if( OK == pCtrl->acquireMetadata(
                    streamId,
                    pStreamBuffer,
                    pMeta
                    ) )
        {
            BufferState_t const init_state =
                pCtrl->isInMetaStream(streamId) ? STATE_READABLE : STATE_WRITABLE;
            return new MetaHandleA(
                    pCtrl,
                    pFrame,
                    streamId,
                    pStreamBuffer,
                    init_state,
                    pMeta
                    );
        }
        else {
            pCtrl->releaseMetaStream(pFrame, pStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
        }
    }
    MY_LOGE("create fail, streamId=%#" PRIxPTR "", streamId);
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MetaHandleA::
~MetaHandleA()
{
    // MY_LOGD_IF(1, "metadata handle release, frame %zu, streamId=%#" PRIxPTR ", muState=%d",
    //                mpFrame->getFrameNo(),  mStreamId, muState);
    if( muState != STATE_NOT_USED )
    {
        const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
            StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
        //
        mpStreamCtrl->releaseMetadata(mpStreamBuffer, mpMetadata);
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, status);
    }
    else
    {
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
    }

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MetaHandleA::
updateState(BufferState_t const state)
{

    Mutex::Autolock _l(mLock);
    // MY_LOGD_IF(1, "streamId %#" PRIxPTR " set status, state %d -> %d",
    //     mStreamId, muState, state);
    if( muState == STATE_NOT_USED ) {
        MY_LOGW("streamId %#" PRIxPTR " state %d -> %d",
            mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIxPTR " set fail, state %d -> %d",
                mStreamId, muState, state);
        muState = state;
    }
    //mCond.broadcast();
}

/******************************************************************************
 *
 ******************************************************************************/
sp<BufferHandle>
StreamBufferHandleA::
create(
    StreamControl* const pCtrl,
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId
)
{

    //MY_LOGD_IF(1, "image handle, streamId=%#" PRIxPTR "", streamId);

    // check StreamBuffer here
    sp<IImageStreamBuffer> pStreamBuffer = NULL;
    if( OK == pCtrl->acquireImageStream(
                pFrame,
                streamId,
                pStreamBuffer) )
    {
        //MY_LOGD("create successlly, pStreamBuffer=%p", pStreamBuffer.get());

        return new StreamBufferHandleA(
                pCtrl,
                pFrame,
                streamId,
                pStreamBuffer
                );
    }
    MY_LOGE("create failed, streamId=%#" PRIxPTR "", streamId);

    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
StreamBufferHandleA::
~StreamBufferHandleA()
{
    // MY_LOGD_IF(1, "iamge handle release, frame %zu, streamId=%#" PRIxPTR ", muState=%d",
    //                mpFrame->getFrameNo(),  mStreamId, muState);
    if( muState != STATE_NOT_USED )
    {
        const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
            StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
        //
        mpStreamCtrl->releaseImageBuffer(mpStreamBuffer, mpImageBuffer);
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, status);
    }
    else
    {
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferHandleA::
waitState(
    BufferState_t const state,
    nsecs_t const nsTimeout
)
{
    Mutex::Autolock _l(mLock);
    if( mpImageBuffer == NULL ) {

        // get buffer from streambuffer
        const MERROR ret = mpStreamCtrl->acquireImageBuffer(mStreamId, mpStreamBuffer, mpImageBuffer);

        // MY_LOGD_IF(1, "mpImageBuffer is null, do acquire, ret=%d, streamId=%#" PRIxPTR ", target status=%d",
        //            ret, mStreamId, state);
        // update initial state
        if( ret == OK )
            muState = mpStreamCtrl->isInImageStream(mStreamId) ? STATE_READABLE : STATE_WRITABLE;
        //return ret;
    }
    //
    if( muState != state ) {
        mCond.waitRelative(mLock, nsTimeout);
    }
    return (muState == state) ? OK : TIMED_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferHandleA::
updateState(BufferState_t const state)
{
    Mutex::Autolock _l(mLock);
    // MY_LOGD_IF(1, "streamId %#" PRIxPTR " set status, state %d -> %d",
    //     mStreamId, muState, state);

    if( muState == STATE_NOT_USED ) {
        MY_LOGW("streamId %#" PRIxPTR " state %d -> %d",
            mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIxPTR " set fail: state %d -> %d",
                mStreamId, muState, state);
        muState = state;
    }
    mCond.broadcast();
}


