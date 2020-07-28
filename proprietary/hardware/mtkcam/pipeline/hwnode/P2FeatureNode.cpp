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

#define LOG_TAG "MtkCam/P2FeatureNode"
//
#include <mtkcam/utils/std/Log.h>
#include "BaseNode.h"
#include "hwnode_utilities.h"
#include <mtkcam/pipeline/hwnode/P2FeatureNode.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
//
#include <vector>
#include <list>
#include <fstream>
//
#include <mtkcam/utils/std/DebugScanLine.h>
#include <mtkcam/utils/std/DebugDrawID.h>
#include <mtkcam/utils/std/DebugTimer.h>
//
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/hw/IFDContainer.h>
//
#include <camera_custom_nvram.h>
#include <isp_tuning_cam_info.h>
//
#if 0/*[EP_TEMP]*/
#include <featureio/eis_hal.h>
#endif
//
#include <mtkcam/utils/std/Trace.h>
//
#include <cutils/properties.h>
//
using namespace NSCam;
#include <mtkcam/utils/hw/HwTransform.h>
#include "P2_utils.h"
//
// clear zoom
#include "DpDataType.h"
#include <DpIspStream.h>
#include <camera_custom_dualzoom.h>
#include <mtkcam/feature/DualCam/DualCam.Common.h>

#include <mtkcam/feature/lcenr/lcenr.h>

#ifdef FEATURE_MODIFY
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <mtkcam/custom/ExifFactory.h>
#include <camera_custom_3dnr.h>
#include <mtkcam/feature/3dnr/3dnr_defs.h>
#include <mtkcam/feature/fsc/fsc_defs.h>
#include <mtkcam/feature/fsc/fsc_ext.h>
#include <camera_custom_eis.h>
#include <mtkcam/feature/featurePipe/IStreamingFeaturePipe.h>
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/feature/eis/eis_type.h>
#include <mtkcam/feature/3dnr/util_3dnr.h>

#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1) || (MTK_CAM_STEREO_DENOISE_SUPPORT == 1)
// DualCam support
#include <mtkcam/feature/DualCam/IDualCamStreamingFeaturePipe.h>
#define DUAL_CAM_SUPPORT 1
#else
#define DUAL_CAM_SUPPORT 0
#endif

#include <cstddef>
#include <cstdint>
#include <sys/mman.h>

#include <camera_custom_nvram.h>
#include <mtkcam/aaa/INvBufUtil.h>

#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif

#include <unordered_map>
#include <tuple>

using NSCam::NSCamFeature::NSFeaturePipe::IStreamingFeaturePipe;
using NSCam::NSCamFeature::NSFeaturePipe::FeaturePipeParam;
using NSCam::NSCamFeature::VarMap;
using NSCam::Utils::DebugTimer;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
#define VAR_P2_REQUEST "p2request"
#endif // FEATURE_MODIFY

using namespace android;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NSCamHW;

using namespace std;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;
using namespace NS3Av3;
using namespace NSCam::FSC;

/******************************************************************************
 *
 ******************************************************************************/
#define PROCESSOR_NAME_P2   ("P2_Feature")
#define PROCESSOR_NAME_MDP  ("MDP_P2_Feature")
#define P2THREAD_POLICY     (SCHED_OTHER)
#define P2THREAD_PRIORITY   (-2)
//
#define WAITBUFFER_TIMEOUT (1000000000L)
/******************************************************************************
 *
 ******************************************************************************/

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF
#undef MY_LOGV_IF
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF
#undef MY_LOGA_IF
#undef MY_LOGF_IF

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

//
#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif
#define P2_DEQUE_DEBUG (1)
/******************************************************************************
 *
 ******************************************************************************/
#define SUPPORT_3A               (1)
#define SUPPORT_EIS              (1)
#define SUPPORT_EIS_MV           (0)
#define FD_PORT_SUPPORT          (1)
#define FORCE_EIS_ON             (SUPPORT_EIS && (0))
#define FORCE_BURST_ON           (0)
#define DEBUG_LOG                (0)
#define VALUE_UNDEFINED          (-1)

typedef IHal3A IHal3A_T;

/******************************************************************************
 *
 ******************************************************************************/
static auto
getDebugExif()
{
    static auto const inst = MAKE_DebugExif();
    return inst;
}

MBOOL is4K2K(const MSize &size)
{
    #define UHD_VR_WIDTH  (3840)
    #define UHD_VR_HEIGHT (2160)
    MBOOL is4K = (size.w >= UHD_VR_WIDTH && size.h >= UHD_VR_HEIGHT);
    #undef UHD_VR_WIDTH
    #undef UHD_VR_HEIGHT
    return is4K;
}

/******************************************************************************
 *
 ******************************************************************************/
namespace P2Feature {
/******************************************************************************
 *
 ******************************************************************************/


inline
MBOOL isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId ) {
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}

class DebugControl
{
public:
    enum DUMP_BUFFER_MODE {
        DUMP_BUFFER_DEBUG_MODE,
        DUMP_BUFFER_NORMAL_MODE,
    };

    enum DUMP_IN {
        DUMP_IN_RRZO,
        DUMP_IN_IMGO,
        DUMP_IN_LCSO,
    };

    enum DUMP_OUT {
        DUMP_OUT_DISPLAY,
        DUMP_OUT_RECORD,
        DUMP_OUT_FD,
        DUMP_OUT_PREVIEWCB,
    };

    DebugControl()
        : mbEnableDumpBuffer(MFALSE)
        , mDumpBufferMode(DUMP_BUFFER_DEBUG_MODE)
        , mDebugDumpStart(0)
        , mDebugDumpCount(0)
        , mbEnableMemCheck(MFALSE)
        , mDebugDumpIn(0)
        , mDebugDumpOut(0)
    {
        mbEnableDumpBuffer = ::property_get_int32("vendor.debug.p2f.dump.enable", 0) > 0 ? MTRUE : MFALSE;
        if ( mbEnableDumpBuffer )
        {
            mDumpBufferMode = (DUMP_BUFFER_MODE)::property_get_int32("vendor.debug.p2f.dump.mode", DUMP_BUFFER_DEBUG_MODE);
            mDebugDumpStart = ::property_get_int32("vendor.debug.p2f.dump.start", 0);
            mDebugDumpCount = ::property_get_int32("vendor.debug.p2f.dump.count", 0);
            mDebugDumpIn = ::property_get_int32("vendor.debug.p2f.dump.in", 0xFFFFFFFF);
            mDebugDumpOut = ::property_get_int32("vendor.debug.p2f.dump.out", 0xFFFFFFFF);
        }

        mbEnableMemCheck = ::property_get_int32("vendor.debug.p2f.memcheck.enable", 0) > 0 ? MTRUE : MFALSE;

        MY_LOGD("mbEnableDumpBuffer(%d) mDumpBufferMode(%d) mDebugDumpStart(%d) mDebugDumpCount(%d) mbEnableMemCheck(%d)",
            mbEnableDumpBuffer, mDumpBufferMode, mDebugDumpStart, mDebugDumpCount, mbEnableMemCheck);

    }
    ~DebugControl(){}

    MBOOL isNddMode() const { return mDumpBufferMode == DUMP_BUFFER_NORMAL_MODE; }
    MBOOL isDebugMode() const { return mDumpBufferMode == DUMP_BUFFER_DEBUG_MODE; }
    MBOOL needMemCheck()  const { return mbEnableMemCheck == MTRUE; }
    MBOOL needDumpFrame(MINT32 const frameNo) const
    {
        MBOOL debugDumpOn = MFALSE;

        if( mbEnableDumpBuffer )
        {
            if( mDumpBufferMode == DUMP_BUFFER_NORMAL_MODE )
            {
                MUINT32 previewDump = ::property_get_int32("vendor.debug.camera.preview.dump", 0);
                if( previewDump > 0 )
                {
                    debugDumpOn = MTRUE;
                }
            }
            else
            {
                debugDumpOn = (mDebugDumpStart < 0) ||
                              ((frameNo >= mDebugDumpStart) && ((MUINT32)(frameNo - mDebugDumpStart) < mDebugDumpCount));
            }
        }

        return debugDumpOn;
    }
    MBOOL needDumpIn(DUMP_IN mask) const
    {
        return (mDebugDumpIn & (1<<mask)) ? MTRUE : MFALSE;
    }

    MBOOL needDumpOut(DUMP_OUT mask) const
    {
        return (mDebugDumpOut & (1<<mask)) ? MTRUE : MFALSE;
    }

private:
    MBOOL                           mbEnableDumpBuffer;
    DUMP_BUFFER_MODE                mDumpBufferMode;
    MINT32                          mDebugDumpStart;
    MUINT32                         mDebugDumpCount;
    MBOOL                           mbEnableMemCheck;
    MUINT32                         mDebugDumpIn;
    MUINT32                         mDebugDumpOut;
};

class PostRedZone
{
private:
    // can query via "getconf PAGE_SIZE" or sysconf()
    static constexpr intptr_t __PAGE_SIZE = 4096;
public:
    static void* mynew(std::size_t count)
    {
        intptr_t __DATA_PAGES = (sizeof(std::size_t) + sizeof(char*) + count + __PAGE_SIZE - 1) / __PAGE_SIZE;
        char *spaceAddr = new char[__PAGE_SIZE * (__DATA_PAGES + 2)];
        intptr_t redZone = reinterpret_cast<intptr_t>(spaceAddr) + (__PAGE_SIZE * (__DATA_PAGES + 1));
        redZone = (redZone & ~(__PAGE_SIZE - 1));
        mprotect(reinterpret_cast<void*>(redZone), __PAGE_SIZE, PROT_READ);
        intptr_t objAddr = redZone - count;
        *(reinterpret_cast<char**>(objAddr - sizeof(char*))) = spaceAddr;
        *(reinterpret_cast<std::size_t*>(objAddr - sizeof(char*) - sizeof(std::size_t))) = count;
        MY_LOGD("spaceAddr(%p) objAddr(%" PRIiPTR ") count(%zu)", spaceAddr, objAddr, count);
        return reinterpret_cast<void*>(objAddr);
    }

    static void mydelete(void *objAddrP) noexcept
    {
        intptr_t objAddr = reinterpret_cast<intptr_t>(objAddrP);
        std::size_t count = *(reinterpret_cast<std::size_t*>(objAddr - sizeof(char*) - sizeof(std::size_t)));
        intptr_t redZone = objAddr + count;
        mprotect(reinterpret_cast<void*>(redZone), __PAGE_SIZE, PROT_READ | PROT_WRITE);
        char *spaceAddr = *(reinterpret_cast<char**>(objAddr - sizeof(char*)));
        MY_LOGD("spaceAddr(%p) objAddr(%" PRIiPTR ") count(%zu)", spaceAddr, objAddr, count);
        delete [] spaceAddr;
    }
};

MBOOL concurrencyControlAcqurie(sp<IResourceConcurrency> &ctrl, IResourceConcurrency::CLIENT_HANDLER &client)
{
    MBOOL ret = MTRUE;
    if( ctrl != NULL )
    {
        client = ctrl->requestClient();
        if( client != IResourceConcurrency::CLIENT_HANDLER_NULL )
        {
            MY_LOGI("[ResCon][%p-%d] resource acquiring", ctrl.get(), (MUINT32)client);
            CAM_TRACE_FMT_BEGIN("P2F:Res-Acquire[%p-%d]", ctrl.get(), (MUINT32)client);

            MERROR res = NO_ERROR;
            if ( (res = ctrl->acquireResource(client)) && res != NO_ERROR )
            {
               MY_LOGI("[ResCon][%p-%d] cannot acquire (%d)", ctrl.get(), (MUINT32)client, res);
               ret = MFALSE;
            }
            else
            {
               MY_LOGI("[ResCon][%p-%d] resource acquired (%d)", ctrl.get(), (MUINT32)client, res);
               ret = MTRUE;
            }

            CAM_TRACE_FMT_END();
        }
        else
        {
           MY_LOGI("[ResCon][%p-%d] cannot request", ctrl.get(), (MUINT32)client);
           ret = MFALSE;
        }
    }

    return ret;
}

MBOOL concurrencyControlRelease(sp<IResourceConcurrency> &ctrl, IResourceConcurrency::CLIENT_HANDLER &client)
{
    MBOOL ret = MTRUE;
    if( ctrl != NULL )
    {
        if( client != IResourceConcurrency::CLIENT_HANDLER_NULL )
        {
            MY_LOGI("[ResCon][%p-%d] resource releasing", ctrl.get(), (MUINT32)client);
            CAM_TRACE_FMT_BEGIN("P2F:Res-Release[%p-%d]", ctrl.get(), (MUINT32)client);

            MERROR res = NO_ERROR;
            if( (res = ctrl->releaseResource(client)) && res != NO_ERROR )
            {
                MY_LOGI("[ResCon][%p-%d] cannot release (%d)", ctrl.get(), (MUINT32)client, res);
                ret = MFALSE;
            }
            else if( (res = ctrl->returnClient(client)) && res != NO_ERROR )
            {
                MY_LOGI("[ResCon][%p-%d] cannot return (%d)", ctrl.get(), (MUINT32)client, res);
                ret = MFALSE;
            }
            else
            {
                MY_LOGI("[ResCon][%p-%d] resource released and returned (%d)", ctrl.get(), (MUINT32)client, res);
                ret = MTRUE;
                client = IResourceConcurrency::CLIENT_HANDLER_NULL;
            }

            CAM_TRACE_FMT_END();
        }
    }
    return ret;
}

class SecureBufferControl
{
public:
    SecureBufferControl()
    :mcleared(MFALSE){}

    ~SecureBufferControl()
    {
        if(!mcleared)
        {
            MY_LOGW("should call SecureBufferControl destroy before release object!");
            std::lock_guard<std::mutex> _m(mMapLock);
            mMap.clear();
        }
    }
    MUINT32 registerAndGetSecHandle(MUINT32 type, IImageBuffer* buf);
    MVOID destroy();

private:
    sp<IImageBuffer> allocSecureBuffer(size_t size, MUINT32 type);
    using secHandleTbl = std::unordered_map<MUINT32, std::tuple<MUINT32, sp<IImageBuffer>>>;
    std::mutex mMapLock;
    secHandleTbl mMap;
    MBOOL mcleared;

};

sp<IImageBuffer> SecureBufferControl::allocSecureBuffer(size_t size, MUINT32 type)
{
    IImageBufferAllocator::ImgParam imgParam(size, 0);

    SecType secType = (type == EDIPSecureEnum::EDIPSecure_SECURE) ?
        SecType::mem_secure : SecType::mem_protected;
    sp<ISecureImageBufferHeap> blobHeap =
        ISecureImageBufferHeap::create(LOG_TAG,
                imgParam,
                ISecureImageBufferHeap::AllocExtraParam(0, 1, 0, MTRUE, secType));
    if (blobHeap == NULL) {
        MY_LOGE("Allocate secure tuning buffer failed!");
        return NULL;
    }

    return blobHeap->createImageBuffer();
}

MUINT32 SecureBufferControl::registerAndGetSecHandle(MUINT32 type, IImageBuffer* buf)
{
    MUINT32 sec_handle = 0;
    if (buf == NULL) return 0;
    std::lock_guard<std::mutex> _m(mMapLock);
    MUINT32 key = buf->getFD(0);
    auto search = mMap.find(key);
    if (search != mMap.end()) {
        sec_handle = std::get<0>(search->second);
    } else {
        sp<IImageBuffer> sec_buf =
            allocSecureBuffer(buf->getBufSizeInBytes(0), type);
        if (sec_buf == NULL) return 0;
        MUINT const groupUsage =
            (eBUFFER_USAGE_HW_CAMERA_READWRITE |
             eBUFFER_USAGE_SW_READ_OFTEN |
             eBUFFER_USAGE_SW_WRITE_OFTEN);
        sec_buf->lockBuf(LOG_TAG, groupUsage);
        mMap.emplace(key, std::make_tuple(sec_buf->getBufVA(0), sec_buf));
        sec_handle = sec_buf->getBufVA(0);
        sec_buf->unlockBuf(LOG_TAG);
    }
    return sec_handle;
}

MVOID SecureBufferControl::destroy()
{
    // need to clean the secure buffer handle
    mcleared = MTRUE;
    std::lock_guard<std::mutex> _m(mMapLock);
    mMap.clear();
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


class MetaHandle
    : public VirtualLightRefBase
{
    public:
        typedef enum
        {
            STATE_NOT_USED,
            STATE_READABLE,
            STATE_WRITE_OK = STATE_READABLE,
            STATE_WRITABLE,
            STATE_WRITE_FAIL,
        } BufferState_t;

    public:
        static sp<MetaHandle>           create(
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId
                                        );
                                        ~MetaHandle();
    protected:
                                        MetaHandle(
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
            STATE_WRITE_OK = STATE_READABLE,
            STATE_WRITABLE,
            STATE_WRITE_FAIL,
            STATE_RELEASED,
        } BufferState_t;

        enum ReleaseFlag
        {
            FORCE = 1,
        };

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
        virtual MUINT32                 getState()                                          = 0;

        virtual MUINT32                 getUsage() { return 0; };

        virtual MUINT32                 getTransform() { return 0; };

        virtual StreamId_T              getStreamId() { return 0; };

    #ifdef FEATURE_MODIFY
        virtual MBOOL                   isValid() { return MTRUE; }
        virtual MBOOL                   earlyRelease(MBOOL /*force*/=MFALSE) { return MTRUE; }
        virtual MVOID                   setBorrowed() { }
    #endif // FEATURE_MODIFY

};

class Request;

class StreamBufferHandle
    : public BufferHandle
{
    public:
        static sp<BufferHandle>         create(
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            wp<Request> wpRequest
                                        );
        virtual                         ~StreamBufferHandle();
    protected:
                                        StreamBufferHandle(
                                            StreamControl* pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IImageStreamBuffer> const pStreamBuffer,
                                            MUINT32 const uTransform,
                                            MUINT32 const uUsage,
                                            wp<Request> wpRequest
                                        );
    public:
        IImageBuffer*                   getBuffer() { return mpImageBuffer.get(); }
        MERROR                          waitState(
                                            BufferState_t const state,
                                            nsecs_t const nsTimeout
                                        );
        MVOID                           updateState(
                                            BufferState_t const state
                                        );

        virtual MUINT32                 getState() { return muState; };

        virtual MUINT32                 getTransform() { return muTransform; };

        virtual MUINT32                 getUsage() { return muUsage; };

        virtual StreamId_T              getStreamId() { return mStreamId; };

    #ifdef FEATURE_MODIFY
        virtual MBOOL                   isValid() {  return mBorrowed || !mEarlyReleased; }
        virtual MBOOL                   earlyRelease(MBOOL force=MFALSE);
        virtual MVOID                   setBorrowed();
    #endif // FEATURE_MODIFY


    private:
        MVOID                           notifyState();

        Mutex                           mLock;
        Condition                       mCond;
        StreamControl* const            mpStreamCtrl;
        sp<IPipelineFrame> const        mpFrame;
        StreamId_T const                mStreamId;
        sp<IImageStreamBuffer> const    mpStreamBuffer;
        MUINT32                         muState;
        MUINT32                         muTransform;
        MUINT32                         muUsage;
        sp<IImageBuffer>                mpImageBuffer;
    #ifdef FEATURE_MODIFY
        MBOOL                           mEarlyReleased;
        MBOOL                           mBorrowed;
    #endif // FEATURE_MODIFY
        wp<Request>                     mwpRequest;
};

#define ENABLE_TRACE_RSS 0

#if ENABLE_TRACE_RSS
#define TRACE_RSS(fmt, arg...) CAM_LOGD("RSS_LOG[%s] " fmt, __FUNCTION__, ##arg)
#else
#define TRACE_RSS(fmt, arg...)
#endif // ENABLE_TRACE_RSS

class BufferLifeHolder : public RefBase
{
    public:
        enum User {
            NONE   = 0,
            FIRST  = 1 << 0,
            SECOND = 1 << 1,
        };

        BufferLifeHolder(StreamControl *streamControl, const sp<IPipelineFrame> &frame);
        virtual ~BufferLifeHolder();

        void notifyFrame();
        void tryReleaseFrame();
        bool borrowRSSO(const sp<BufferHandle> &rsso);
        bool returnRSSO(User user);
        IImageBuffer* getRSSOBuffer();

    private:
        void doNotifyFrame();
        void doReleaseFrame();
        bool doReleaseRSSO();

    private:
        Mutex mMutex;
        StreamControl *mStreamControl;
        sp<IPipelineFrame> mFrame;
        int mFrameNo;
        sp<BufferHandle> mRSSO;
        int mRSSOUser;
};

BufferLifeHolder::BufferLifeHolder(StreamControl *streamControl, const sp<IPipelineFrame> &frame)
    : mStreamControl(streamControl)
    , mFrame(frame)
    , mFrameNo(frame == NULL ? -1 : mFrame->getFrameNo())
    , mRSSO(NULL)
    , mRSSOUser(NONE)
{
    TRACE_RSS("%d", mFrameNo);
}

BufferLifeHolder::~BufferLifeHolder()
{
    TRACE_RSS("%d", mFrameNo);
    Mutex::Autolock lock(mMutex);
    if( doReleaseRSSO() )
    {
        doNotifyFrame();
    }
    doReleaseFrame();
}

void BufferLifeHolder::notifyFrame()
{
    TRACE_RSS("%d", mFrameNo);
    Mutex::Autolock lock(mMutex);
    doNotifyFrame();
}

void BufferLifeHolder::tryReleaseFrame()
{
    TRACE_RSS("%d", mFrameNo);
    Mutex::Autolock lock(mMutex);
    if( mRSSO == NULL )
    {
        doReleaseFrame();
    }
}

bool BufferLifeHolder::borrowRSSO(const sp<BufferHandle> &rsso)
{
    TRACE_RSS("%d", mFrameNo);
    Mutex::Autolock lock(mMutex);
    bool ret = true;
    if( mRSSO != NULL )
    {
        MY_LOGE("cannot borrow RSSO more than once");
        ret = false;
    }
    else
    {
        mRSSO = rsso;
        mRSSOUser = NONE;
        if( mRSSO != NULL )
        {
            mRSSO->setBorrowed();
            mRSSOUser = FIRST | SECOND;
        }
    }
    return ret;
}

bool BufferLifeHolder::returnRSSO(User user)
{
    CAM_TRACE_CALL();

    TRACE_RSS("%d", mFrameNo);
    Mutex::Autolock lock(mMutex);
    bool ret = false;
    mRSSOUser &= ~user;
    if( !mRSSOUser )
    {
        ret = doReleaseRSSO();
    }
    return ret;
}

IImageBuffer* BufferLifeHolder::getRSSOBuffer()
{
    TRACE_RSS("%d", mFrameNo);
    Mutex::Autolock lock(mMutex);
    return (mRSSO == NULL) ? NULL : mRSSO->getBuffer();
}

void BufferLifeHolder::doNotifyFrame()
{
    TRACE_RSS("%d", mFrameNo);
    if( mStreamControl != NULL && mFrame != NULL )
    {
        mStreamControl->onPartialFrameDone(mFrame);
    }
}

void BufferLifeHolder::doReleaseFrame()
{
    TRACE_RSS("%d", mFrameNo);
    if( mFrame != NULL )
    {
        CAM_TRACE_NAME("P2F:Release Pipeline Frame");
        mFrame = NULL;
    }
}

bool BufferLifeHolder::doReleaseRSSO()
{
    TRACE_RSS("%d", mFrameNo);
    bool ret = false;
    if( mRSSO != NULL )
    {
        mRSSO->earlyRelease(BufferHandle::FORCE);
        mRSSO = NULL;
        mRSSOUser = NONE;
        ret = true;
    }
    return ret;
}

class FrameLifeHolder : public VirtualLightRefBase
{
    public:
                                        FrameLifeHolder(
                                            MINT32 const openId,
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            MBOOL const enableLog
                                            )
                                            : mbEnableLog(enableLog)
                                            , mOpenId(openId)
                                            , mpStreamControl(pCtrl)
                                            , mpFrame(pFrame)
                                        {
                                            if  ( ATRACE_ENABLED() ) {
                                                mTraceName = String8::format("Cam:%d:IspP2|%d|request:%d frame:%u", mOpenId, mpFrame->getRequestNo(), mpFrame->getRequestNo(), mpFrame->getFrameNo());
                                                CAM_TRACE_ASYNC_BEGIN(mTraceName.string(), 0);
                                            }
                                            CAM_TRACE_ASYNC_BEGIN("P2F:FrameLifeHolder", mpFrame->getFrameNo());
                                            MY_LOGD_IF(mbEnableLog, "frame %u +", mpFrame->getFrameNo());
                                        }

                                        ~FrameLifeHolder() {
                                            if  ( ! mTraceName.isEmpty() ) {
                                                CAM_TRACE_ASYNC_END(mTraceName.string(), 0);
                                            }
                                            if( mpStreamControl )
                                                mpStreamControl->onFrameDone(mpFrame);
                                            MY_LOGD_IF(mbEnableLog, "frame %u -", mpFrame->getFrameNo());
                                            CAM_TRACE_ASYNC_END("P2F:FrameLifeHolder", mpFrame->getFrameNo());
                                        }

    public:
        MVOID                           onPartialFrameDone() {
                                            if( mpStreamControl )
                                                mpStreamControl->onPartialFrameDone(mpFrame);
                                        }

        StreamControl*                  getStreamControl() {
                                            return mpStreamControl;
                                        }

        sp<IPipelineFrame>              getPipelineFrame() {
                                            return mpFrame;
                                        }

    private:
        MBOOL const                     mbEnableLog;
        MINT32 const                    mOpenId;
        String8                         mTraceName;
        StreamControl* const            mpStreamControl; //stream control & dispatch
        sp<IPipelineFrame> const        mpFrame;
};

class Processor: virtual public RefBase
{
public:
    virtual MERROR   queueRequest(sp<Request>)           = 0;
    virtual MERROR   handleRequest(sp<Request>)          = 0;
    virtual MVOID    flushRequests()                     = 0;
    virtual MVOID    waitForIdle()                       = 0;
    virtual MVOID    setNextProcessor(wp<Processor>)     = 0;
    virtual MBOOL    isAsyncProcessor()                  = 0;
    virtual MVOID    callbackAsync(sp<Request>, MERROR)  = 0;
    virtual MBOOL    close()                             = 0;
};

class P2RequestTimer
{
public:
    ADD_DEBUGTIMER(SetIsp);
    ADD_DEBUGTIMER(OnExecute);
};

class Request
    : virtual public RefBase
{
public:
    struct Context
    {
        MUINT8                      burst_num;
        MUINT32                     custom_option;
        MBOOL                       resized;
        StreamId_T                  fd_stream_id;
        sp<BufferHandle>            in_buffer;
        sp<BufferHandle>            in_lcso_buffer;
        sp<BufferHandle>            in_rsso_buffer;
        sp<Cropper::crop_info_t>    crop_info;
        vector<sp<BufferHandle> >   out_buffers;
        sp<BufferHandle>            in_mdp_buffer;
        sp<MetaHandle>              in_app_meta;
        map<StreamId_T, MUINT32>    buffer_status_map;
#ifdef FEATURE_MODIFY
        MINT32                      iso;
        MUINT8                      isp_profile;
        MINT32                      magic3A;
        MINT32                      lv_value;
        MINT32                      ctrl_caltm_enable;
#endif // FEATURE_MODIFY
        sp<MetaHandle>              in_hal_meta;
        sp<MetaHandle>              out_app_meta;
        sp<MetaHandle>              out_hal_meta;
        _SRZ_SIZE_INFO_             srz4Param;
        TuningUtils::FILE_DUMP_NAMING_HINT hint;
        P2RequestTimer              timer;
    };

    struct Context context;

    typedef enum
    {
        STATE_WAIT,
        STATE_SUSPEND,
        STATE_RUNNING,
        STATE_DONE,
    } RequestState_t;

    RequestState_t          uState;

    Request(sp<FrameLifeHolder> pFrameLifeHolder)
        : uState(STATE_WAIT)
        , index(0)
        , frameNo(0)
        , mpFrameLifeHolder(pFrameLifeHolder)
    {
    }

    MUINT32                    index;
    MUINT32                    frameNo;
    MINT64                     timestamp = -1;
    MUINT8                     afstate = 0;
    MUINT8                     lensstate = 0;
    MBOOL                      skipOutCheck = 0;

    MVOID updateBufferHandleStatus(StreamId_T id, MUINT32 status)
    {
        context.buffer_status_map[id] = status;
    }

    String8 getBufferHandleStatusString()
    {
        String8 statusString;

        for( auto iter : context.buffer_status_map )
        {
            StreamId_T streamId = iter.first;
            statusString += String8::format("Id(%#" PRIx64 "):%u ", streamId, iter.second);
        }
        return statusString;
    }

    MVOID setCurrentOwner(wp<Processor> pProcessor)
    {
        mwpProcessor = pProcessor;
    }

    MVOID responseDone(MERROR status)
    {
        sp<Processor> spProcessor = mwpProcessor.promote();
        if(spProcessor.get())
        {
            MY_LOGD_IF(DEBUG_LOG, "perform callback %d[%d]",frameNo ,index);
            spProcessor->callbackAsync(this, status);
        }
    }

    MVOID onPartialRequestDone()
    {
        if(mpFrameLifeHolder.get())
        {
            MY_LOGD_IF(1, "ReqNo %u frame %u applyRelease %s", getPipelineFrame()->getRequestNo(), getPipelineFrame()->getFrameNo(), getBufferHandleStatusString().string());
            mpFrameLifeHolder->onPartialFrameDone();
        }
    }

    virtual ~Request()
    {
        CAM_TRACE_CALL();

        if(context.in_buffer.get())
        {
            context.in_buffer.clear();
            MY_LOGW("context.in_buffer not released");
        }
        if(context.in_lcso_buffer.get())
        {
            context.in_lcso_buffer.clear();
            MY_LOGW("context.in_lcso_buffer not released");
        }
        if(context.in_rsso_buffer.get())
        {
            context.in_rsso_buffer.clear();
            MY_LOGW("context.in_rsso_buffer not released");
        }
        if(context.in_mdp_buffer.get())
        {
            context.in_mdp_buffer.clear();
            MY_LOGW("context.in_mdp_buffer not released");
        }

        vector<sp<BufferHandle> >::iterator iter = context.out_buffers.begin();
        while(iter != context.out_buffers.end())
        {
            if((*iter).get())
            {
                MY_LOGW("context.out_buffers[%#" PRIx64 "] not released", (*iter)->getStreamId());
                (*iter).clear();
            }
            iter++;
        }

        if(context.in_app_meta.get())
        {
            context.in_app_meta.clear();
            MY_LOGW("context.in_app_meta not released");
        }
        if(context.in_hal_meta.get())
        {
            context.in_hal_meta.clear();
            MY_LOGW("context.in_hal_meta not released");
        }
        if(context.out_app_meta.get())
        {
            context.out_app_meta.clear();
            MY_LOGD("context.out_app_meta not released");
        }
        if(context.out_hal_meta.get())
        {
            context.out_hal_meta.clear();
            MY_LOGD("context.out_hal_meta not released");
        }

        onPartialRequestDone();
    }

    StreamControl* getStreamControl()
    {
        return (mpFrameLifeHolder == NULL) ? NULL : mpFrameLifeHolder->getStreamControl();
    }

    sp<IPipelineFrame> getPipelineFrame()
    {
        return (mpFrameLifeHolder == NULL) ? NULL : mpFrameLifeHolder->getPipelineFrame();
    }

private:
    sp<FrameLifeHolder>     mpFrameLifeHolder;
    wp<Processor>           mwpProcessor;

};


template<typename TProcedure>
struct ProcedureTraits {
    typedef typename TProcedure::InitParams  TInitParams;
    typedef typename TProcedure::FrameParams TProcParams;

    typedef MERROR (TProcedure::*TOnExtract)(Request*, TProcParams&);
    typedef MERROR (TProcedure::*TOnExecute)(sp<Request>,TProcParams const&);
    typedef MERROR (TProcedure::*TOnFinish)(TProcParams const&, MBOOL const);
    typedef MVOID  (TProcedure::*TOnFlush)();
    typedef MVOID  (TProcedure::*TOnSync)();

    static constexpr TOnExtract   fnOnExtract     = &TProcedure::onExtractParams;
    static constexpr TOnExecute   fnOnExecute     = &TProcedure::onExecute;
    static constexpr TOnFinish    fnOnFinish      = &TProcedure::onFinish;
    static constexpr TOnFlush     fnOnFlush       = &TProcedure::onFlush;
    static constexpr TOnSync      fnOnSync        = &TProcedure::onSync;

    static constexpr MBOOL  isAsync               = TProcedure::isAsync;
};


template<typename TProcedure, typename TProcedureTraits = ProcedureTraits<TProcedure>>
class ProcessorBase : virtual public Processor, virtual public Thread
{
    typedef typename TProcedureTraits::TInitParams          TInitParams;
    typedef typename TProcedureTraits::TProcParams          TProcParams;

    typedef typename TProcedureTraits::TOnExtract           TOnExtract;
    typedef typename TProcedureTraits::TOnExecute           TOnExecute;
    typedef typename TProcedureTraits::TOnFinish            TOnFinish;
    typedef typename TProcedureTraits::TOnFlush             TOnFlush;
    typedef typename TProcedureTraits::TOnSync              TOnSync;


protected:
    TProcedure         mProcedure;

public:
    ProcessorBase(TInitParams const& initParams, char const* name)
        : Thread(false)
        , mProcedure(initParams)
        , mbRequestDrained(MTRUE)
        , mbExit(MFALSE)
        , mName(name)
    {
       run(name);
    }
    #define PROC_TAG(str) "[%s] " str, mName.string()
    virtual MBOOL close()
    {
        MY_LOGD_IF(DEBUG_LOG, PROC_TAG("close processor"));

        requestExit();
        join();
        waitForIdle();
        return MTRUE;
    }

    ~ProcessorBase()
    {
        MY_LOGD_IF(DEBUG_LOG, PROC_TAG("destroy processor"));
    }

    virtual MERROR queueRequest(sp<Request> pRequest)
    {
        Mutex::Autolock _l(mRequestLock);
        pRequest->setCurrentOwner(this);
        //  Make sure the request with a smaller frame number has a higher priority.
        vector<sp<Request> >::iterator it = mvPending.end();
        for (; it != mvPending.begin();)
        {
            --it;
            if (0 <= (MINT32)(pRequest->frameNo - (*it)->frameNo))
            {
                ++it; //insert(): insert before the current node
                break;
            }
        }
        mvPending.insert(it, pRequest);
        MY_LOGD_IF(DEBUG_LOG, PROC_TAG("after request[%d-%d] queued, pending:%zu, running:%zu"),
            pRequest->frameNo, pRequest->index, mvPending.size(), mvRunning.size());

        mRequestQueueCond.signal();
        return OK;
    }


    virtual MERROR handleRequest(sp<Request> pRequest)
    {
        TProcParams params;
        TOnExtract fnOnExtract = TProcedureTraits::fnOnExtract;
        TOnExecute fnOnExecute = TProcedureTraits::fnOnExecute;
        TOnFinish fnOnFinish = TProcedureTraits::fnOnFinish;

        if (OK == (mProcedure.*fnOnExtract)(pRequest.get(), params))
        {
            MERROR ret = (mProcedure.*fnOnExecute)(pRequest, params);
            if(isAsyncProcessor() && ret == OK)
            {
                // do aync processing
                Mutex::Autolock _l(mCallbackLock);
                mvRunning.push_back(make_pair(pRequest,params));
                MY_LOGD_IF(DEBUG_LOG, PROC_TAG("request[%d-%d], pending:%zu, running:%zu"),
                    pRequest->frameNo, pRequest->index, mvPending.size(), mvRunning.size());
                mCallbackCond.signal();
            }
            else
            {
                (mProcedure.*fnOnFinish)(params, ret == OK);
                // trigger to release buffer
                params = TProcParams();
                pRequest->onPartialRequestDone();

                sp<Processor> spProcessor = mwpNextProcessor.promote();
                if (spProcessor.get())
                {
                    spProcessor->queueRequest(pRequest);
                }
            }
            return ret;
        }
        return OK;
    }

    virtual MVOID callbackAsync(sp<Request> pRequest, MERROR status)
    {
        if (isAsyncProcessor())
        {
            Mutex::Autolock _l1(mAsyncLock);
            Mutex::Autolock _l2(mCallbackLock);
            MY_LOGD_IF(DEBUG_LOG, PROC_TAG("request[%d-%d], pending:%zu, running:%zu"),
                pRequest->frameNo, pRequest->index, mvPending.size(), mvRunning.size());
            TOnFinish fnOnFinish = &TProcedure::onFinish;

            MBOOL isFound = MFALSE;

            do
            {
                typename vector<pair<sp<Request>,TProcParams> >::iterator iter = mvRunning.begin();
                for(; iter != mvRunning.end(); iter++)
                   if((*iter).first == pRequest)
                   {
                       (mProcedure.*fnOnFinish)((*iter).second, status == OK);
                       CAM_TRACE_BEGIN("P2F:Release FrameParams");
                       mvRunning.erase(iter);
                       CAM_TRACE_END();
                       isFound = MTRUE;
                       break;
                   }

                if(!isFound)
                {
                     MY_LOGD_IF(1 ,PROC_TAG("request[%d-%d] callback faster than execution finished"),
                        pRequest->frameNo, pRequest->index);
                     #if 0 // July28,2017 - Rynn Wu <rynn.wu@mediatek.com>
                           // Unknown deadlock may happened here, for some reasons,
                           // we're not able to fix this problem, but invoke a
                           // null pointer dereference exception to avoid deadlock
                           // problem.

                     mCallbackCond.wait(mCallbackLock);
                     #else
                     const nsecs_t TIMEOUTNS = 3000000000LL; // nanosecond
                     auto _result = mCallbackCond.waitRelative(mCallbackLock, TIMEOUTNS);
                     if (__builtin_expect( _result != OK , false )) {
                         MY_LOGE("%s: mCallbackCond wait timed out(%" PRId64 "), err=%#x",
                                 __FUNCTION__, TIMEOUTNS, _result);
                         *((volatile int*)(0x0)) = 0xDEADFEED; // invoke NE
                     }
                     #endif
                }
            } while (!isFound);

            MY_LOGD_IF(DEBUG_LOG, PROC_TAG("request callback async, status:%d"), status);
            pRequest->onPartialRequestDone();

            mAsyncCond.signal();
            sp<Processor> spProcessor = mwpNextProcessor.promote();
            if (spProcessor.get())
            {
                spProcessor->queueRequest(pRequest);
            }
        }
        return;
    }

    virtual MVOID setNextProcessor(wp<Processor> pProcessor)
    {
        mwpNextProcessor = pProcessor;
    }

    virtual MBOOL isAsyncProcessor()
    {
        return TProcedureTraits::isAsync;
    }

public:
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void requestExit()
    {
        Mutex::Autolock _l(mRequestLock);
        mbExit = MTRUE;
        mRequestQueueCond.signal();
    }

    // Good place to do one-time initializations
    virtual status_t readyToRun()
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

        MY_LOGD_IF(1 ,PROC_TAG("tid(%d) policy(%d) priority(%d)"), ::gettid(), P2THREAD_POLICY, P2THREAD_PRIORITY);

        return OK;

    }

private:

    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool threadLoop()
    {
        while(!exitPending() && OK == onWaitRequest())
        {
            sp<Request> pRequest = NULL;
            {
                Mutex::Autolock _l(mRequestLock);

                if( mvPending.size() == 0 )
                {
                    MY_LOGW_IF(1 ,PROC_TAG("no request"));
                    return true;
                }

                pRequest = mvPending.front();
                mvPending.erase( mvPending.begin());
            }

            MY_LOGD_IF(handleRequest(pRequest) != OK, "request execute != OK");

            return true;
        }
        MY_LOGD_IF(DEBUG_LOG, PROC_TAG("exit thread"));
        return  false;
    }

public:

    virtual MVOID flushRequests()
    {
        FUNC_START;

        Mutex::Autolock _l(mRequestLock);

        mvPending.clear();

        if (!mbRequestDrained)
        {
            MY_LOGD_IF(1 ,PROC_TAG("wait for request drained"));
            mRequestDrainedCond.wait(mRequestLock);
        }

        TOnFlush fnOnFlush = TProcedureTraits::fnOnFlush;
        (mProcedure.*fnOnFlush)();

        FUNC_END;
        return;
    }

    virtual MVOID waitForIdle()
    {
        TOnSync fnOnSync = TProcedureTraits::fnOnSync;
        (mProcedure.*fnOnSync)();

        if(isAsyncProcessor())
            return;

        Mutex::Autolock _l(mAsyncLock);
        while(mvRunning.size())
        {
            MY_LOGD_IF(1 ,PROC_TAG("wait request done %zu"), mvRunning.size());
            mAsyncCond.wait(mAsyncLock);
        }

        return;
    }

    virtual MERROR onWaitRequest()
    {
        Mutex::Autolock _l(mRequestLock);
        while (!mvPending.size() && !mbExit)
        {
            // set drained flag
            mbRequestDrained = MTRUE;
            mRequestDrainedCond.signal();

            status_t status = mRequestQueueCond.wait(mRequestLock);
            if (OK != status)
            {
                MY_LOGW_IF(1 ,PROC_TAG("wait status:%d:%s, request size:%zu, exit:%d"),
                    status, ::strerror(-status), mvPending.size(), mbExit
                );
            }
        }

        if (mbExit)
        {
            MY_LOGW_IF(mvPending.size(), PROC_TAG("existed mvPending.size:%zu"), mvPending.size());
            return DEAD_OBJECT;
        }

        mbRequestDrained = MFALSE;
        return OK;
    }

protected:
    vector<sp<Request> >     mvPending;
    // for async request
    vector<pair<sp<Request>,TProcParams> >
                                    mvRunning;
    wp<Processor>                   mwpNextProcessor;
    mutable Mutex                   mRequestLock;
    mutable Condition               mRequestQueueCond;
    mutable Condition               mRequestDrainedCond;
    MBOOL                           mbRequestDrained;
    MBOOL                           mbExit;
    mutable Mutex                   mAsyncLock;
    mutable Condition               mAsyncCond;
    mutable Mutex                   mCallbackLock;
    mutable Condition               mCallbackCond;
    String8                         mName;

};

class P2Procedure
{
    protected:
        class MultiFrameHandler
        {
            public:
                MultiFrameHandler(IStreamingFeaturePipe* const pPipe, MBOOL bEableLog)
                    : mpPipe(pPipe)
                    , mbEnableLog(bEableLog)
                    , muMfEnqueCnt(0)
                    , muMfDequeCnt(0)
                {
                }
                MERROR                      collect(sp<Request>, FeaturePipeParam&);
                MVOID                       flush();
                static MVOID                callback(QParams& rParams)
                {
                    MultiFrameHandler* pHandler = reinterpret_cast<MultiFrameHandler*>(rParams.mpCookie);
                    pHandler->handleCallback(rParams);
                }
                MVOID                       handleCallback(QParams& rParams);

            private:
                IStreamingFeaturePipe* const mpPipe;
                mutable Mutex               mLock;
                MBOOL const                 mbEnableLog;
                QParams                     mParamCollecting;
                MUINT32                     muMfEnqueCnt;
                MUINT32                     muMfDequeCnt;
                vector<sp<Request> >        mvReqCollecting;
                vector<vector<sp<Request> > > mvRunning;
        };


    public:
        struct InitParams
        {
            MINT32                          openId;
            P2FeatureNode::ePass2Type       type;
            MRect                           activeArray;
            IHal3A_T*                       p3A;
            IStreamingFeaturePipe*          pPipe;
            MBOOL                           enableLog;
            MBOOL                           enableDumpBuffer;
            #ifdef FEATURE_MODIFY
            P2FeatureNode::UsageHint        usageHint;
            #endif // FEATURE_MODIFY
            IResourceConcurrency::CLIENT_HANDLER
                                            resConClient;
        };

        struct FrameInput
        {
            PortID                          mPortId;
            sp<BufferHandle>                mHandle;

            ~FrameInput()
            {
                CAM_TRACE_CALL();
                mHandle = NULL;
            }
        };

        struct FrameOutput
        {
            FrameOutput()
            : mTransform(0)
            , mUsage(0)
            {};
            PortID                          mPortId;
            sp<BufferHandle>                mHandle;
            MINT32                          mTransform;
            MUINT32                         mUsage;

            ~FrameOutput()
            {
                CAM_TRACE_CALL();
                mHandle = NULL;
            }
        };

        struct FrameParams
        {
            FrameInput           in;
            FrameInput           in_lcso;
            FrameInput           in_rsso;
            Vector<FrameOutput>  vOut;
            //
            MBOOL                bResized;
            //
            sp<MetaHandle>       inApp;
            sp<MetaHandle>       inHal;
            sp<MetaHandle>       outApp;
            sp<MetaHandle>       outHal;
        };

    private: //private use structures
        struct eis_region
        {
            MUINT32 x_int;
            MUINT32 x_float;
            MUINT32 y_int;
            MUINT32 y_float;
            MSize   s;
#if SUPPORT_EIS_MV
            MUINT32 x_mv_int;
            MUINT32 x_mv_float;
            MUINT32 y_mv_int;
            MUINT32 y_mv_float;
            MUINT32 is_from_zzr;
#endif
#ifdef FEATURE_MODIFY
            // 3dnr vipi: needs x_int/y_int/gmvX/gmvY
            // ISP smoothNR3D: needs gmvX/gmvY/confX/confY/maxGMV
            MINT32 gmvX;
            MINT32 gmvY;
            MINT32 confX;
            MINT32 confY;
            MINT32 maxGMV;

            eis_region()
                : x_int(0)
                , x_float(0)
                , y_int(0)
                , y_float(0)
                , s(MSize(0,0))
#if SUPPORT_EIS_MV
                , x_mv_int(0)
                , x_mv_float(0)
                , y_mv_int(0)
                , y_mv_float(0)
                , is_from_zzr(0)
#endif
                , gmvX(0)
                , gmvY(0)
                , confX(0)
                , confY(0)
                , maxGMV(LMV_MAX_GMV_DEFAULT)
                {};
#endif // FEATURE_MODIFY
        };

    public:
        static sp<Processor>            createProcessor(InitParams& params);
                                        ~P2Procedure();

                                        P2Procedure(InitParams const& params)
                                            : mInitParams(params)
                                            , mpPipe(params.pPipe)
                                            , mp3A(params.p3A)
                                            , muEnqueCnt(0)
                                            , muDequeCnt(0)
                                            , mDebugScanLineMask(0)
                                            , mDebugDrawRectMask(0)
                                            , mpDebugScanLine(NULL)
                                            , mpDebugDrawID(NULL)
                                            , mbEnableLog(params.enableLog)
                                            , mpHalSensorList(NULL)
                                            , mpNvram(NULL)
                                            , mpFDContainer(NULL)
                                            , mspResConCtrl(params.usageHint.pResourceConcurrency)
                                            , mResConClient(params.resConClient)
                                            , mSecureEnum(EDIPSecureEnum::EDIPSecure_NONE)
                                            , mSecure(MFALSE)
                                            , mOpenId(params.openId)
                                            , mBufferLifeHolder(NULL)
                                        {
                                            mpMultiFrameHandler = new MultiFrameHandler(
                                                params.pPipe, params.enableLog);

                                            char cProperty[PROPERTY_VALUE_MAX];
                                            ::property_get("vendor.debug.camera.scanline.p2", cProperty, "0");
                                            mDebugScanLineMask = ::atoi(cProperty);
                                            if ( mDebugScanLineMask != 0)
                                            {
                                                mpDebugScanLine = DebugScanLine::createInstance();
                                            }

                                            mpDebugDrawID = DebugDrawID::createInstance();
                                            mDebugDrawRectMask = ::property_get_int32("vendor.debug.camera.drawcrop.mask", 0);

#ifdef FEATURE_MODIFY
                                            //disable/enable LCE
                                            mbIsDisabLCE = ::property_get_int32("vendor.isp.lce.disable", 0);
                                            mUtil3dnr = new Util3dnr(mOpenId);
                                            mUtil3dnr->init(mInitParams.usageHint.m3DNRMode &
                                                NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT);
#endif // FEATURE_MODIFY
                                            mpHalSensorList = MAKE_HalSensorList();
                                            if( mpHalSensorList == NULL )
                                            {
                                                MY_LOGE("IHalSensorList::get fail");
                                            }

                                            DpIspStream::queryISPFeatureSupport(mIspFeatureSupport);
                                            initNvramPtr();

                                            mpFDContainer = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
                                            if( mpFDContainer == NULL )
                                            {
                                                MY_LOGE("IFDContainer::createInstance fail");
                                            }

                                            switch (params.usageHint.mSecType) {
                                                case SecType::mem_normal :
                                                    mSecureEnum = EDIPSecureEnum::EDIPSecure_NONE;
                                                    mSecure = MFALSE;
                                                    break;
                                                case SecType::mem_secure :
                                                    mSecureEnum = EDIPSecureEnum::EDIPSecure_SECURE;
                                                    mSecure = MTRUE;
                                                    break;
                                                case SecType::mem_protected :
                                                    mSecureEnum = EDIPSecureEnum::EDIPSecure_PROTECT;
                                                    mSecure = MTRUE;
                                                    break;
                                                default:
                                                    MY_LOGW("Not support secure type (%d), reset to normal type!", params.usageHint.mSecType);
                                                    mSecureEnum = EDIPSecureEnum::EDIPSecure_NONE;
                                                    mSecure = MFALSE;
                                                    break;
                                            }
                                        }

        MERROR                          onExtractParams(Request*, FrameParams&);

    public:
        MERROR                          waitIdle();

    public:                             // used by job
        static const MBOOL              isAsync = MTRUE;

        MERROR                          onExecute(
                                            sp<Request> const pRequest,
                                            FrameParams const& params
                                        );

        MERROR                          onFinish(
                                            FrameParams const& params,
                                            MBOOL const success
                                        );

        MVOID                           onFlush();

        MVOID                           onSync();

    protected:

        MERROR                          mapPortId(
                                            StreamId_T const streamId, // [in]
                                            MUINT32 const transform,   // [in]
                                            MBOOL const isFdStream,    // [in]
                                            MUINT32& rOccupied,        // [in/out]
                                            PortID&  rPortId           // [out]
                                        ) const;

        MERROR                          checkParams(FrameParams const params, MBOOL const skipCheckOutBuffer) const;

        MERROR                          getCropInfos(
                                            IMetadata* const inApp,
                                            IMetadata* const inHal,
                                            MBOOL const isResized,
                                            Cropper::crop_info_t& cropInfos
                                        ) const;

        MVOID                           queryCropRegion(
                                            Cropper::crop_info_t& cropInfos,
                                            IMetadata* const inApp,
                                            IMetadata* const inHal,
                                            MBOOL const isEisOn,
                                            MSize const &fovMargin,
                                            MRect& targetCrop
                                        ) const;

        MVOID                           updateCropRegion(
                                            MRect const crop,
                                            IMetadata* meta_result
                                        ) const;

        MBOOL                           isEISOn(
                                            IMetadata* const inApp
                                        ) const;

        MBOOL                           queryEisRegion(
                                            IMetadata* const inHal,
                                            eis_region& region
                                        ) const;

        MVOID                           getEISMarginPixel(
                                            IMetadata* const inApp,
                                            IMetadata* const inHal,
                                            MSize& eisSensorMargin,
                                            MRectF& eisDomainCrop
                                        ) const;

        MVOID                           getFOVMarginPixel(
                                            IMetadata* const inApp,
                                            IMetadata* const inHal,
                                            MSize& fovSensorMargin,
                                            MSize& fovRRZMargin
                                        ) const;

        MVOID                           prepareFrameParam(
                                            QParams &enqueParams,
                                            const sp<Request> &pRequest,
                                            MINT32 czTimeStamp
                                            );

        MVOID                           prepareCustomSettingPtr(
                                            void* &pCustomSetting,
                                            uint32_t &customIndex,
                                            DP_ISP_FEATURE_ENUM ispFeature,
                                            MINT32 magic3A
                                            );

        MVOID                           prepareFaceInfoPtr(
                                            void* &pFaceInfo
                                            );

        MVOID                           preparePQParam(
                                            DpPqParam &pqParam,
                                            const Output &output,
                                            const sp<Request> &pRequest,
                                            MINT32 czTimeStamp
                                            );

        MVOID                           initNvramPtr();

        MVOID                           appendHandleDebugString(
                                            sp<BufferHandle> handle,
                                            const char *tag,
                                            String8 &str
                                            );

#ifdef FEATURE_MODIFY
        MVOID                           prepareFeaturePipeParam(
                                          FeaturePipeParam &featureEnqueParams,
                                          const QParams &enqueParams,
                                          const sp<Request> &pRequest,
                                          IMetadata *pMeta_InApp,
                                          IMetadata *pMeta_InHal,
                                          IMetadata *pMeta_OutApp,
                                          IMetadata *pMeta_OutHal,
                                          const Cropper::crop_info_t &cropInfos);
        MBOOL                           setP2B3A(FeaturePipeParam &param);
        sp<BufferLifeHolder>            updateBufferLifeHolder(const sp<BufferLifeHolder> &holder);
        MVOID                           flushBufferLifeHolder();
        MVOID                           prepareBufferLifeHolder(
                                          FeaturePipeParam &param,
                                          const sp<Request> &request,
                                          const sp<BufferLifeHolder> &holder,
                                          const sp<BufferHandle> &rsso
                                          );
        MVOID                           drawScanLine(const Output &output);
        MVOID                           drawRect(const IImageBuffer *buffer, const Output *output, const MCropRect &mCropRect, IMetadata *pMeta_InHal);
        MVOID                           drawID(const Output &output, unsigned id);
        static MVOID                    partialRelease(FeaturePipeParam &param);
        static MVOID                    returnRSSO(FeaturePipeParam &param, MBOOL needNotify);
        static MINT32                   getP2ID(const FeaturePipeParam &param);
        static MBOOL                    preparePreviewCallback(FeaturePipeParam &param);
        static MBOOL                    featurePipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &data);
        static MBOOL                    featurePipeCBDebug(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &data);
        static MVOID                    dumpNddBuffer(TuningUtils::FILE_DUMP_NAMING_HINT* hint, IImageBuffer *buffer, MUINT32 portIndex);
        static MVOID                    dumpBuffer(MINT32 frameNo, IImageBuffer *buffer, const char *fmt, ...);

        MVOID prepare3DNR_SL2E_Info(
            IMetadata *pMeta_InApp,
            IMetadata *pMeta_InHal,
            QParams &enqueParams,
            const Cropper::crop_info_t &cropInfos,
            FeaturePipeParam &featureEnqueParams,
            const sp<Request> &pRequest);

        MVOID prepareFeatureMask_EIS(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal);
        MVOID prepareFeatureData_3DNR(
            FeaturePipeParam &featureEnqueParams,
            const QParams &enqueParams,
            const Cropper::crop_info_t &cropInfos,
            const sp<Request> &pRequest, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal);
        MVOID prepareFeatureData_FSC(FeaturePipeParam &pipeParam, IMetadata *pMeta_InHal);
        MVOID prepareFeatureData_VHDR(FeaturePipeParam &pipeParam, IMetadata *pMeta_InHal);
        MVOID prepareFeatureData_VFB(FeaturePipeParam &pipeParam);
        MVOID prepareFeatureData_EIS(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal);
        MVOID prepareFeatureData_Common(FeaturePipeParam &pipeParam, const sp<Request> &pRequest, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal, const Cropper::crop_info_t &cropInfos);
        MVOID prepareFeatureData_DualCam(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal);
        MVOID prepareFeatureData_N3D(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp);

        MBOOL isCRZApplied(IMetadata* const inApp, IMetadata *pMeta_InHal, const QParams& rParams, const MUINT32 featureMask);

        MINT32 get3DNRIsoThreshold(MUINT8 ispProfile);

#endif // FEATURE_MODIFY
        static MVOID                    pass2CbFunc(QParams& rParams);

        MVOID                           handleDeque(QParams& rParams);

        MBOOL                           packDualCamInHalMetadata(
                                                MINT64 timestamp,
                                                MUINT8 afstate,
                                                MUINT8 lensstate,
                                                IMetadata *inHal);

    private:
        MultiFrameHandler*            mpMultiFrameHandler;
        SecureBufferControl             mSecBufCtrl;
        //
        mutable Mutex                   mLock;
        mutable Condition               mCondJob;
        //
        InitParams const                mInitParams;
        //
        IStreamingFeaturePipe* const    mpPipe;
        IHal3A_T* const                 mp3A;
        //
        MUINT32                         muEnqueCnt;
        MUINT32                         muDequeCnt;
        vector<sp<Request> >            mvRunning;
        //
        #define DRAWLINE_PORT_WDMAO     0x1
        #define DRAWLINE_PORT_WROTO     0x2
        #define DRAWLINE_PORT_IMG2O     0x4
        MUINT32                         mDebugScanLineMask;
        MUINT32                         mDebugDrawRectMask;
        DebugScanLine*                  mpDebugScanLine;
        DebugDrawID*                    mpDebugDrawID;
        //
        MBOOL const                     mbEnableLog;
        DebugControl                    mDebugControl;

        // Sensor Hal
        IHalSensorList                 *mpHalSensorList;
        map<DP_ISP_FEATURE_ENUM, bool>  mIspFeatureSupport;
        NVRAM_CAMERA_FEATURE_STRUCT    *mpNvram;
        sp<IFDContainer>                mpFDContainer;
        sp<IResourceConcurrency>        mspResConCtrl;
        IResourceConcurrency::CLIENT_HANDLER
                                        mResConClient;
        EDIPSecureEnum                  mSecureEnum;
        MBOOL                           mSecure;


#ifdef FEATURE_MODIFY
        MINT                            mOpenId;
        MBOOL                           mbIsDisabLCE;
        Mutex                           mBufferLifeHolderMutex;
        sp<BufferLifeHolder>            mBufferLifeHolder;
        sp<Util3dnr>                    mUtil3dnr;
#endif // FEATURE_MODIFY


#if P2_DEQUE_DEBUG
        vector<QParams>                 mvParams;
#endif
};


class MDPProcedure
{
    public:
        struct InitParams
        {
            MBOOL                          enableLog;
        };

        struct FrameInput
        {
            sp<BufferHandle>                mHandle;
        };

        struct FrameOutput
        {
            sp<BufferHandle>                mHandle;
            MINT32                          mTransform;
        };

        struct FrameParams
        {
            sp<Cropper::crop_info_t>    pCropInfo;
            FrameInput                  in;
            Vector<FrameOutput>         vOut;
        };
    public:
        static sp<Processor>            createProcessor(InitParams& params);
                                        ~MDPProcedure() {}
                                        MDPProcedure(InitParams const& params)
                                        : mbEnableLog(params.enableLog)
                                        {}

        MERROR                          onExtractParams(Request*, FrameParams&);

    protected:

                                        MDPProcedure(MBOOL const enableLog)
                                        : mbEnableLog(enableLog)
                                        {}

    public:
        static const MBOOL              isAsync = MFALSE;

        MERROR                          waitIdle() { return OK; } // since is synchronous

        MERROR                          onExecute(
                                            sp<Request> const pRequest,

                                            FrameParams const& params
                                        );

        MERROR                          onFinish(
                                            FrameParams const& params,
                                            MBOOL const success
                                        );

        MVOID                           onFlush(){};

        MVOID                           onSync(){};

    private:
        MBOOL const                     mbEnableLog;

};


/******************************************************************************
 *
 ******************************************************************************/
class P2NodeImp
    : virtual public BaseNode
    , virtual public P2FeatureNode
    , public StreamControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;
    //
public:     ////                    Operations.

                                    P2NodeImp(ePass2Type const type, const UsageHint &usageHint);

    virtual                         ~P2NodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.


    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    using BaseNode::flush;
    virtual MERROR                  flush();

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Operations.
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MERROR                          verifyConfigParams(
                                        ConfigParams const & rParams
                                    ) const;

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

    MERROR                          mapToRequests(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

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

inline MBOOL                        isLcsoRawLocked(StreamId_T const streamId) const {
                                        return isStream(mpInLcsoRaw, streamId);
                                    }

inline MBOOL                        isRssoRawLocked(StreamId_T const streamId) const {
                                        return isStream(mpInRssoRaw, streamId);
                                    }

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
    sp<IImageStreamInfo>            mpInLcsoRaw;
    sp<IImageStreamInfo>            mpInRssoRaw;
    ImageStreamInfoSetT             mvOutImages;
    sp<IImageStreamInfo>            mpOutFd;
    // feature
    MUINT8 mBurstNum;

    MUINT32 mCustomOption;
#ifdef FEATURE_MODIFY
    UsageHint                       mUsageHint;
#endif // FEATURE_MODIFY

protected:  ////                    Data Members. (Operation)
    mutable Mutex                   mOperationLock;
    sp<IResourceConcurrency>        mspResConCtrl;
    IResourceConcurrency::CLIENT_HANDLER
                                    mResConClient;

private:
    sp<Processor>                   mpP2Processor;
    sp<Processor>                   mpMdpProcessor;
};

/******************************************************************************
 *
 ******************************************************************************/
#ifdef FEATURE_MODIFY
MBOOL isAPEnabled_3DNR(MINT32 force3DNR, IMetadata *appInMeta, IMetadata *halInMeta)
{
    MINT32 e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;

    if( appInMeta == NULL ||
        !tryGetMetadata<MINT32>(appInMeta, MTK_NR_FEATURE_3DNR_MODE, e3DnrMode) )
    {
        MY_LOGW("no MTK_NR_FEATURE_3DNR_MODE: appInMeta: %p", appInMeta);
    }

    MINT32 eHal3DnrMode = MTK_NR_FEATURE_3DNR_MODE_ON;
    if (halInMeta == NULL ||
        !tryGetMetadata<MINT32>(halInMeta, MTK_DUALZOOM_3DNR_MODE, eHal3DnrMode))
    {
        // Only valid for dual cam. On single cam, we don't care HAL meta,
        // and can assume HAL is "ON" on single cam.
        eHal3DnrMode = MTK_NR_FEATURE_3DNR_MODE_ON;
        // We already printed following log in P1Node
        // MY_LOGD("no MTK_DUALZOOM_3DNR_MODE in HAL");
    }

    if (force3DNR)
    {
        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.camera.3dnr.enable", EnableOption, "n");
        if (EnableOption[0] == '1')
        {
            e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_ON;
            eHal3DnrMode = MTK_NR_FEATURE_3DNR_MODE_ON;
        }
        else if (EnableOption[0] == '0')
        {
            e3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
            eHal3DnrMode = MTK_NR_FEATURE_3DNR_MODE_OFF;
        }
    }
    return (e3DnrMode == MTK_NR_FEATURE_3DNR_MODE_ON && eHal3DnrMode == MTK_NR_FEATURE_3DNR_MODE_ON);
}

MBOOL isAPEnabled_VHDR(IMetadata * /*halInMeta*/)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.isp.lce.disable", value, "0"); // 0: enable, 1: disable
    MBOOL bDisable = atoi(value);

    if(bDisable) {
        return MFALSE;
    } else {
        return MTRUE;
    }
#if 0
    // Because LCEI module will be always on, so this always returns true except VFB on.
    if( isVFBOn)
        return MFALSE;

    IMetadata::IEntry entry = halInMeta->entryFor(MTK_VHDR_LCEI_DATA);
    return ! entry.isEmpty();
#endif
}

MBOOL isHALenabled_VHDR(IMetadata *halInMeta)
{
    MBOOL enabledVHDR = MFALSE;
    MINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
    tryGetMetadata<MINT32>(halInMeta, MTK_P1NODE_SENSOR_VHDR_MODE, vhdrMode);

    if(vhdrMode != SENSOR_VHDR_MODE_NONE)
    {
        enabledVHDR = MTRUE;
    }
    return enabledVHDR;
}

MBOOL isAPEnabled_VFB(IMetadata * /*appInMeta*/)
{
    return MFALSE;
}

MBOOL isAPEnabled_VFB_EX(IMetadata * /*appInMeta*/)
{
    return MFALSE;
}

MBOOL isAPEnabled_EIS(IMetadata *appInMeta)
{
    MBOOL enabledStreamingEIS = MFALSE;
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;

    if (!tryGetMetadata<MUINT8>(appInMeta, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode))
    {
        MY_LOGW("no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
        return MFALSE;

    }

    enabledStreamingEIS = (eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON) ? MTRUE : MFALSE;

    return enabledStreamingEIS;
    //return MFALSE;
}


#ifdef FEATURE_MODIFY
MVOID P2Procedure::prepare3DNR_SL2E_Info(
    IMetadata *pMeta_InApp,
    IMetadata *pMeta_InHal,
    QParams &enqueParams,
    const Cropper::crop_info_t &cropInfos,
    FeaturePipeParam &featureEnqueParams,
    const sp<Request> &pRequest)
{
    CAM_TRACE_NAME("P2F:prepare3DNR_SL2E_Info");

#if 1
    MBOOL canEnable3dnr = (HAS_3DNR(featureEnqueParams.mFeatureMask)) ? MTRUE : MFALSE;

    NR3DMVInfo mvInfo;
    eis_region eisInfo;

    if (canEnable3dnr)
    {
        queryEisRegion(pMeta_InHal, eisInfo);

        MINT32 isLmvValid = 1;
        if (!tryGetMetadata<MINT32>(pMeta_InHal, MTK_LMV_VALIDITY, isLmvValid))
        {
            isLmvValid = 1; // Absent. Default behavior is 1 on single cam
        }

        mvInfo.status = (isLmvValid == 1 ? NR3DMVInfo::VALID : NR3DMVInfo::INVALID);
        mvInfo.x_int = eisInfo.x_int;
        mvInfo.y_int = eisInfo.y_int;
        mvInfo.gmvX = eisInfo.gmvX;
        mvInfo.gmvY = eisInfo.gmvY;
        mvInfo.confX = eisInfo.confX;
        mvInfo.confY = eisInfo.confY;
        mvInfo.maxGMV = eisInfo.maxGMV;
    }

    MBOOL isImgo = (cropInfos.isResized == MFALSE);

    if (pMeta_InApp == NULL)
    {
        MY_LOGE("sensor(%d) no meta inApp: %p",
            mOpenId, pMeta_InApp);
        return;
    }
    MBOOL isCRZMode = isCRZApplied(pMeta_InApp, pMeta_InHal, enqueParams, featureEnqueParams.mFeatureMask);

    MBOOL isSl2eEnable = (mInitParams.usageHint.m3DNRMode & NSCam::NR3D::E3DNR_MODE_MASK_SL2E_EN) ?
        MTRUE : MFALSE;
    IImageBuffer *iBuffer = NULL;

    if (enqueParams.mvFrameParams[0].mvIn.size() > 0)
    {
        iBuffer = enqueParams.mvFrameParams[0].mvIn[0].mBuffer;
    }

    MINT32 isoThreshold = get3DNRIsoThreshold(pRequest->context.isp_profile);

    // must call after enqueParams.mvFrameParams[0].mvCropRsInfo[0].mCropRect is ready!
    mUtil3dnr->prepareISPDataHAL1(canEnable3dnr, mvInfo, isImgo, cropInfos.crop_p1_sensor, pRequest->context.iso, isoThreshold, isCRZMode,
        isSl2eEnable, iBuffer, enqueParams.mvFrameParams[0].mvCropRsInfo[0].mCropRect, pMeta_InHal);
#else
    if (pMeta_InApp == NULL || pMeta_InHal == NULL)
    {
        MY_LOGE("sensor(%d) no meta inApp: %p or no meta inHal: %p",
            mOpenId, pMeta_InApp, pMeta_InHal);
        return;
    }

    if (isAPEnabled_3DNR((mInitParams.usageHint.m3DNRMode &
        NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT), pMeta_InApp))
    {
        if( pMeta_InHal != NULL && enqueParams.mvIn.size() > 0)
        {
            MBOOL isCRZMode = MFALSE;
            MSize sl2eOriSize;
            MRect sl2eCropInfo;
            MSize sl2eRrzSize;

            if (isCRZApplied(pMeta_InApp, pMeta_InHal, enqueParams, featureEnqueParams.mFeatureMask))
            {
                isCRZMode = MTRUE;

                // 1. imgi w/h
                sl2eOriSize.w = (enqueParams.mvIn[0].mBuffer)->getImgSize().w;
                sl2eOriSize.h = (enqueParams.mvIn[0].mBuffer)->getImgSize().h;

                // 2. crop info: x, y, w, h
                sl2eCropInfo.p.x = enqueParams.mvCropRsInfo[0].mCropRect.p_integral.x;
                sl2eCropInfo.p.y = enqueParams.mvCropRsInfo[0].mCropRect.p_integral.y;
                sl2eCropInfo.s.w = enqueParams.mvCropRsInfo[0].mCropRect.s.w;
                sl2eCropInfo.s.h = enqueParams.mvCropRsInfo[0].mCropRect.s.h;

                // 3. img3o w/h
                // IMG3O image size
                // same region as crop rect in getFullImgSize
                sl2eRrzSize.w = enqueParams.mvCropRsInfo[0].mCropRect.s.w;
                sl2eRrzSize.h = enqueParams.mvCropRsInfo[0].mCropRect.s.h;
            }
            else /* 1:1 */
            {
                if (cropInfos.isResized == MFALSE)//IMGO
                {
                   // 1. imgi w/h
                    sl2eOriSize.w = (enqueParams.mvIn[0].mBuffer)->getImgSize().w;
                    sl2eOriSize.h = (enqueParams.mvIn[0].mBuffer)->getImgSize().h;

                    // 2. crop info: x, y, w, h
                    sl2eCropInfo = cropInfos.crop_p1_sensor;

                    // 3. img3o w/h
                    sl2eRrzSize.w = cropInfos.crop_p1_sensor.s.w;
                    sl2eRrzSize.h = cropInfos.crop_p1_sensor.s.h;
                }
                else//RRZO
                {
                    // 1. imgi w/h
                    sl2eOriSize.w = (enqueParams.mvIn[0].mBuffer)->getImgSize().w;
                    sl2eOriSize.h = (enqueParams.mvIn[0].mBuffer)->getImgSize().h;

                    // 2. crop info: x, y, w, h
                    sl2eCropInfo.p.x = 0;
                    sl2eCropInfo.p.y = 0;
                    sl2eCropInfo.s.w = (enqueParams.mvIn[0].mBuffer)->getImgSize().w;
                    sl2eCropInfo.s.h = (enqueParams.mvIn[0].mBuffer)->getImgSize().h;

                    // 3. img3o w/h
                    sl2eRrzSize.w = (enqueParams.mvIn[0].mBuffer)->getImgSize().w;
                    sl2eRrzSize.h = (enqueParams.mvIn[0].mBuffer)->getImgSize().h;
                }
            }
            trySetMetadata<MSize>(pMeta_InHal, MTK_ISP_P2_ORIGINAL_SIZE, sl2eOriSize);
            trySetMetadata<MRect>(pMeta_InHal, MTK_ISP_P2_CROP_REGION, sl2eCropInfo);
            trySetMetadata<MSize>(pMeta_InHal, MTK_ISP_P2_RESIZER_SIZE, sl2eRrzSize);

            MY_LOGD_IF(mbEnableLog, "CRZ(%d) resized(%d), sl2e.imgi(%d,%d), sl2e.crop(%d,%d,%d,%d), sl2e.rrz(%d,%d)",
                isCRZMode, cropInfos.isResized, sl2eOriSize.w, sl2eOriSize.h,
                sl2eCropInfo.p.x,  sl2eCropInfo.p.y, sl2eCropInfo.s.w, sl2eCropInfo.s.h,
                sl2eRrzSize.w, sl2eRrzSize.h);

            featureEnqueParams.setVar<MBOOL>("3dnr.eis.isCRZMode", isCRZMode);
        }
    }
#endif
    return;
}
#endif // FEATURE_MODIFY


MVOID P2Procedure::prepareFeatureData_3DNR(
    FeaturePipeParam &featureEnqueParams,
    const QParams &enqueParams,
    const Cropper::crop_info_t &cropInfos,
    const sp<Request> &pRequest, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal
    )
{
    CAM_TRACE_NAME("P2F:prepareFeatureData_3DNR");

    MY_LOGD_IF(mbEnableLog, "sensor(%d) HAS_3DNR: 0x%x", mOpenId, HAS_3DNR(featureEnqueParams.mFeatureMask) );

    eis_region eisInfo;

#if 1// new util3dnr

    MBOOL isImgo = (cropInfos.isResized == MFALSE);
    MBOOL uiEnable = (HAS_3DNR(featureEnqueParams.mFeatureMask)) ? MTRUE : MFALSE;
    NR3DMVInfo mvInfo;

    if (uiEnable)
    {
        queryEisRegion(pMeta_InHal, eisInfo);

        MINT32 isLmvValid = 1;
        if (!tryGetMetadata<MINT32>(pMeta_InHal, MTK_LMV_VALIDITY, isLmvValid))
        {
            // If LMV validity is not present, maybe old behavior(single cam)
            // For old behavior, LMV is always valid
            isLmvValid = 1;
        }

        mvInfo.status = (isLmvValid == 1 ? NR3DMVInfo::VALID : NR3DMVInfo::INVALID);
        mvInfo.x_int = eisInfo.x_int;
        mvInfo.y_int = eisInfo.y_int;
        mvInfo.gmvX = eisInfo.gmvX;
        mvInfo.gmvY = eisInfo.gmvY;
        mvInfo.confX = eisInfo.confX;
        mvInfo.confY = eisInfo.confY;
        mvInfo.maxGMV = eisInfo.maxGMV;
    }

    MINT32 isoThreshold = get3DNRIsoThreshold(pRequest->context.isp_profile);
    MBOOL canEnable3dnr = mUtil3dnr->canEnable3dnr(uiEnable, pRequest->context.iso, isoThreshold);
    mUtil3dnr->modifyMVInfo(canEnable3dnr, isImgo, cropInfos.crop_p1_sensor, cropInfos.dstsize_resizer,
        mvInfo);

    if (pMeta_InApp == NULL)
    {
        MY_LOGE("sensor(%d) no meta inApp: %p",
            mOpenId, pMeta_InApp);
        return;
    }

    MBOOL isCRZMode = isCRZApplied(pMeta_InApp, pMeta_InHal, enqueParams, featureEnqueParams.mFeatureMask);
    mUtil3dnr->prepareFeatureData(canEnable3dnr, mvInfo, pRequest->context.iso, isoThreshold, isCRZMode, featureEnqueParams);
    mUtil3dnr->prepareGyro(NULL, &featureEnqueParams);
#else
#define INVALID_ISO_VALUE (-1)

    MINT32 force3DNR = (mInitParams.usageHint.m3DNRMode &
                        NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT);
    MINT32 i4IsoThreshold = isoThreshold;

    if (HAS_3DNR(featureEnqueParams.mFeatureMask) )
    {
        if (iso < i4IsoThreshold)
        {
            MY_LOGD_IF(mbEnableLog, "DISABLE 3DNR: due to iso(%d) < %d",
                iso, i4IsoThreshold);
            featureEnqueParams.setFeatureMask(MASK_3DNR, 0);
            // TODO: must reset frame information for smoothNR3D
            return;
        }

        queryEisRegion(pMeta_InHal, eisInfo);

        // Modify eisInfo for IMGO->IMGI
        if (cropInfos.isResized == MFALSE)
        {
            MINT32 x_int = eisInfo.gmvX/256;
            MINT32 y_int = eisInfo.gmvY/256;
            eisInfo.gmvX = (x_int * cropInfos.crop_p1_sensor.s.w / cropInfos.dstsize_resizer.w * 256);
            eisInfo.gmvY = (y_int * cropInfos.crop_p1_sensor.s.h / cropInfos.dstsize_resizer.h * 256);

            MY_LOGD_IF(mbEnableLog, "[IMGO2IMGI] modify gmv(%d,%d)->(%d,%d)", x_int, y_int,
                eisInfo.gmvX/256, eisInfo.gmvY/256);
        }

        // for ZSD digital zoom path
        featureEnqueParams.setVar<MBOOL>(VAR_IMGO_2IMGI_ENABLE, (cropInfos.isResized == MFALSE));
        featureEnqueParams.setVar<MRect>(VAR_IMGO_2IMGI_P1CROP, cropInfos.crop_p1_sensor);

        //MBOOL tmp_isEISOn = isEISOn(pMeta_InApp);

        //featureEnqueParams.setVar<MBOOL>("3dnr.eis.isEisOn", tmp_isEISOn);
        featureEnqueParams.setVar<MUINT32>("3dnr.eis.x_int", eisInfo.x_int);
        featureEnqueParams.setVar<MUINT32>("3dnr.eis.y_int", eisInfo.y_int);
        featureEnqueParams.setVar<MUINT32>("3dnr.eis.gmvX", eisInfo.gmvX);
        featureEnqueParams.setVar<MUINT32>("3dnr.eis.gmvY", eisInfo.gmvY);

        MY_LOGD_IF(mbEnableLog, "sensor(%d) 3dnr.eis: x_int=%d, y_int=%d, gmvX=%d, gmvY=%d, iso(%d)",
            mOpenId,
            eisInfo.x_int,
            eisInfo.y_int,
            eisInfo.gmvX,
            eisInfo.gmvY,
            iso
            );

        // === ISO value ===
        if (iso != INVALID_ISO_VALUE)
        {
            featureEnqueParams.setVar<MUINT32>("3dnr.iso", iso);
        }
    }
#endif
}

MVOID P2Procedure::prepareFeatureData_FSC(FeaturePipeParam &pipeParam, IMetadata * pMeta_InHal)
{
    if (HAS_FSC(pipeParam.mFeatureMask) )
    {
        // set FSC crop data
        IMetadata::IEntry cropEntry = pMeta_InHal->entryFor(MTK_FSC_CROP_DATA);
        if (cropEntry.count())
        {
            IMetadata::Memory metaMemory = cropEntry.itemAt(0, Type2Type< IMetadata::Memory >());
            FSC_CROPPING_DATA_STRUCT *cropData = (FSC_CROPPING_DATA_STRUCT*)metaMemory.array();
            pipeParam.setVar<MRectF>(VAR_FSC_RRZO_CROP_REGION, cropData->RRZOFSCRegion);
            pipeParam.setVar<MRectF>(VAR_FSC_RSSO_CROP_REGION, cropData->RSSOFSCRegion);
        }
        // set FSC warp data
        IMetadata::IEntry warpEntry = pMeta_InHal->entryFor(MTK_FSC_WARP_DATA);
        if (warpEntry.count())
        {
            IMetadata::Memory metaMemory = warpEntry.itemAt(0, Type2Type< IMetadata::Memory >());
            FSC_WARPING_DATA_STRUCT *warpData = (FSC_WARPING_DATA_STRUCT*)metaMemory.array();
            pipeParam.setVar<FSC_WARPING_DATA_STRUCT>(VAR_FSC_RRZO_WARP_DATA, *warpData);
        }
    }
}

MVOID P2Procedure::prepareFeatureData_VHDR(FeaturePipeParam &/*pipeParam*/, IMetadata * /*pMeta_InHal*/)
{
}
MVOID P2Procedure::prepareFeatureData_VFB(FeaturePipeParam &pipeParam)
{
    if( HAS_VFB(pipeParam.mFeatureMask) )
    {
    }
}

MVOID P2Procedure::prepareFeatureData_N3D(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp)
{
   if( HAS_N3D(pipeParam.mFeatureMask) )
   {
        MUINT8 isAFTrigger = 0;
        tryGetMetadata<MUINT8>(pMeta_InApp, MTK_CONTROL_AF_TRIGGER, isAFTrigger);
        pipeParam.setVar<MUINT8>(VAR_N3D_ISAFTRIGGER, isAFTrigger);
   }
}

MVOID P2Procedure::prepareFeatureData_EIS(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal)
{
    CAM_TRACE_NAME("P2F:prepareFeatureData_EIS");

    if( HAS_EIS(pipeParam.mFeatureMask) )
    {
        MSize sensor_size;
        MRect crop_info;
        MSize resizer_size;
        MINT32 GMV_X,GMV_Y,ConfX,ConfY;
        MINT32 LongExpTime = 0,ExpTime = 0,ihwTS = 0,ilwTS = 0;
        MBOOL isBinEnabled = 0;
        MINT64 ts = 0,tmp;
        GMV_X = GMV_Y = ConfX = ConfY = 0;

        // Get EIS data
        IMetadata::IEntry entryRegion = pMeta_InHal->entryFor(MTK_EIS_REGION);
        if (entryRegion.count() > EIS_REGION_INDEX_LWTS)
        {
            GMV_X = entryRegion.itemAt(EIS_REGION_INDEX_GMVX, Type2Type<MINT32>());
            GMV_Y = entryRegion.itemAt(EIS_REGION_INDEX_GMVY, Type2Type<MINT32>());
            ConfX = entryRegion.itemAt(EIS_REGION_INDEX_CONFX, Type2Type<MINT32>());
            ConfY = entryRegion.itemAt(EIS_REGION_INDEX_CONFY, Type2Type<MINT32>());
            ExpTime = LongExpTime = entryRegion.itemAt(EIS_REGION_INDEX_EXPTIME, Type2Type<MINT32>());
            ihwTS = entryRegion.itemAt(EIS_REGION_INDEX_HWTS, Type2Type<MINT32>());
            ilwTS = entryRegion.itemAt(EIS_REGION_INDEX_LWTS, Type2Type<MINT32>());
            isBinEnabled = entryRegion.itemAt(EIS_REGION_INDEX_ISFRONTBIN, Type2Type<MBOOL>());
            tmp = (MINT64)(ihwTS&0xFFFFFFFF);
            ts = (tmp<<32) + ((MINT64)ilwTS&0xFFFFFFFF);
            //MY_LOGD("EisHal TS hw: (%x)  lw: (%x) %lld", ihwTS,ilwTS,ts);

        }

        pipeParam.setVar<MINT32>(VAR_EIS_GMV_X, GMV_X);
        pipeParam.setVar<MINT32>(VAR_EIS_GMV_Y, GMV_Y);
        pipeParam.setVar<MUINT32>(VAR_EIS_CONF_X, ConfX);
        pipeParam.setVar<MUINT32>(VAR_EIS_CONF_Y, ConfY);
        pipeParam.setVar<MINT64>(VAR_EIS_TIMESTAMP, ts);

        IMetadata::IEntry lmvDataEntry = pMeta_InHal->entryFor(MTK_EIS_LMV_DATA);
        if (lmvDataEntry.count() != 0)
        {
            IMetadata::Memory eisStatistic = lmvDataEntry.itemAt(0, Type2Type< IMetadata::Memory >());
            EIS_STATISTIC_STRUCT *lmvData = (EIS_STATISTIC_STRUCT*)eisStatistic.array();
            pipeParam.setVar<EIS_STATISTIC_STRUCT>(VAR_EIS_LMV_DATA, *lmvData);
        }

        if ( !tryGetMetadata<MSize>(pMeta_InHal, MTK_HAL_REQUEST_SENSOR_SIZE, sensor_size))
        {
            MY_LOGW("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        }
        //
        if ( !tryGetMetadata<MRect>(pMeta_InHal, MTK_P1NODE_SCALAR_CROP_REGION, crop_info))
        {
            MY_LOGW("should sync with p1 for rrz setting");
            //
        }
        else
        {
            if( isBinEnabled )
            {
                // scale to rrzo domain
                crop_info.p.x = crop_info.p.x >> 1;
                crop_info.p.y = crop_info.p.y >> 1;
                crop_info.s.w = crop_info.s.w >> 1;
                crop_info.s.h = crop_info.s.h >> 1;
            }
        }

        if ( !tryGetMetadata<MSize>(pMeta_InHal, MTK_P1NODE_RESIZER_SIZE, resizer_size))
        {
            MY_LOGW("should sync with p1 for rrz setting");
            //
        }

        pipeParam.setVar<MSize>(VAR_EIS_SENSOR_SIZE, sensor_size);
        pipeParam.setVar<MRect>(VAR_EIS_SCALER_CROP, crop_info);
        pipeParam.setVar<MSize>(VAR_EIS_SCALER_SIZE, resizer_size);

        // Get Exposure Time
        IMetadata::Memory meta;
        if( tryGetMetadata<IMetadata::Memory>(pMeta_InHal, MTK_PROCESSOR_CAMINFO, meta) )
        {
            RAWIspCamInfo pCamInfo;
            ::memcpy(&pCamInfo, meta.array(), sizeof(NSIspTuning::RAWIspCamInfo));
            if( pCamInfo.rAEInfo.u4EISExpRatio > 0 )
            {
                ExpTime = LongExpTime * 100 / pCamInfo.rAEInfo.u4EISExpRatio;
            }
            else
            {
                MY_LOGW("[prepareFeatureData_EIS] Invalid u4EISExpRatio. Use ExpTime=LongExpTime");
            }
        }
        else
        {
            ExpTime = LongExpTime;
            MY_LOGW("[prepareFeatureData_EIS] Get MTK_PROCESSOR_CAMINFO failed. Use ExpTime=LongExpTime");
        }

        pipeParam.setVar<MINT32>(VAR_EIS_EXP_TIME, ExpTime);
        pipeParam.setVar<MINT32>(VAR_EIS_LONGEXP_TIME, LongExpTime);

        MSize eisSensorMargin;
        MRectF eisDomainCrop;
        getEISMarginPixel(pMeta_InApp, pMeta_InHal, eisSensorMargin, eisDomainCrop);
        pipeParam.setVar<MRectF>(VAR_EIS_RRZO_CROP, eisDomainCrop);

        MY_LOGD_IF(mbEnableLog, "EisHal ExpTime: %d, LongExpTime: %d, ts: %lld, isBinEn: %d, Sensor size(%dx%d) SCALAR_CROP_REGION:(%d,%d)(%dx%d) ==>SCALAR_output(%dx%d) crop(%f,%f)(%fx%f)",
                   ExpTime, LongExpTime, (long long)ts, isBinEnabled,
                   sensor_size.w, sensor_size.h,
                   crop_info.p.x, crop_info.p.y,
                   crop_info.s.w, crop_info.s.h,
                   resizer_size.w, resizer_size.h,
                   eisDomainCrop.p.x, eisDomainCrop.p.y,
                   eisDomainCrop.s.w, eisDomainCrop.s.h);
    }
}

static MINT32 getP2RequestNo(FeaturePipeParam &param);

MVOID P2Procedure::prepareFeatureData_Common(FeaturePipeParam &pipeParam, const sp<Request> &pRequest, IMetadata * pMeta_InApp, IMetadata *pMeta_InHal, const Cropper::crop_info_t &cropInfos)
{
    CAM_TRACE_NAME("P2F:prepareFeatureData_Common");

    (void)pMeta_InApp;

    MINT32 appMode = 0;
    tryGetMetadata<MINT32>(pMeta_InHal, MTK_FEATUREPIPE_APP_MODE, appMode);
    IStreamingFeaturePipe::eAppMode mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
    switch( appMode )
    {
    case MTK_FEATUREPIPE_PHOTO_PREVIEW:
        mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
        break;
    case MTK_FEATUREPIPE_VIDEO_PREVIEW:
        mode = IStreamingFeaturePipe::APP_VIDEO_PREVIEW;
        break;
    case MTK_FEATUREPIPE_VIDEO_RECORD:
        mode = IStreamingFeaturePipe::APP_VIDEO_RECORD;
        break;
    case MTK_FEATUREPIPE_VIDEO_STOP:
        mode = IStreamingFeaturePipe::APP_VIDEO_STOP;
        break;
    default:
        mode = IStreamingFeaturePipe::APP_PHOTO_PREVIEW;
        break;
    }
    pipeParam.setVar<IStreamingFeaturePipe::eAppMode>(VAR_APP_MODE, mode);
    MY_LOGD_IF(mbEnableLog, "FeaturePipe appMode: %d", mode);

    // for debug dump control
    MINT32 frameNo = getP2RequestNo(pipeParam);
    MBOOL debugDump = mDebugControl.needDumpFrame(frameNo);
    pipeParam.setVar<MBOOL>(VAR_DEBUG_DUMP, debugDump);
    MY_LOGD_IF(mbEnableLog, "FeaturePipe debugDump: %d", debugDump);

    if( mDebugControl.isNddMode() )
    {
        pipeParam.setVar<TuningUtils::FILE_DUMP_NAMING_HINT>(VAR_DEBUG_DUMP_HINT, pRequest->context.hint);
    }

    // for ZSD digital zoom path
    MBOOL isImgo = (cropInfos.isResized == MFALSE);
    const MRect cropP1Sensor = cropInfos.crop_p1_sensor;

    pipeParam.setVar<MBOOL>(VAR_IMGO_2IMGI_ENABLE, isImgo);
    pipeParam.setVar<MRect>(VAR_IMGO_2IMGI_P1CROP, cropP1Sensor);
}

MVOID P2Procedure::prepareFeatureData_DualCam(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal)
{
    if( HAS_FOV(pipeParam.mFeatureMask) )
    {
        MSize fovSensorMargin;
        MSize fovRRZMargin;
        getFOVMarginPixel(pMeta_InApp, pMeta_InHal, fovSensorMargin, fovRRZMargin);

        pipeParam.setVar<MSize>(VAR_DUALCAM_FOV_SENSOR_MARGIN, fovSensorMargin);
        pipeParam.setVar<MSize>(VAR_DUALCAM_FOV_RRZO_MARGIN, fovRRZMargin);
    }
}

IStreamingFeaturePipe::UsageHint getPipeUsageHint(const P2FeatureNode::UsageHint &usage)
{
    IStreamingFeaturePipe::UsageHint pipeUsage;
    switch( usage.mUsageMode )
    {
    case P2FeatureNode::USAGE_PREVIEW:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_FEATURE;
        break;

    case P2FeatureNode::USAGE_CAPTURE:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH;
        break;
    case P2FeatureNode::USAGE_TIMESHARE_CAPTURE:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_P2A_PASS_THROUGH_TIME_SHARING;
        break;

    case P2FeatureNode::USAGE_RECORD:
    default:
        pipeUsage.mMode = IStreamingFeaturePipe::USAGE_FULL;
        break;
    }
    pipeUsage.mStreamingSize = usage.mStreamingSize;
    pipeUsage.mVendorCusSize = ((usage.mUsageMode == P2FeatureNode::USAGE_RECORD) && (usage.mVideoSize.size() > usage.mPreviewSize.size())) ? usage.mVideoSize : usage.mPreviewSize;
    pipeUsage.mDualMode = usage.mDualMode;
    pipeUsage.m3DNRMode = usage.m3DNRMode;
    pipeUsage.mFSCMode = usage.mFSCMode;
    pipeUsage.mUseTSQ = MFALSE;
    pipeUsage.mSecType = usage.mSecType;

    //MY_LOGD("%d (%dx%d) (%dx%d) (%dx%d)", usage.mUsageMode, usage.mVideoSize.w, usage.mVideoSize.h, usage.mPreviewSize.w, usage.mPreviewSize.h,
    //    pipeUsage.mVendorCusSize.w, pipeUsage.mVendorCusSize.h);

    // Set EIS pipeUsage
    pipeUsage.mEISInfo = usage.mEisInfo;

    return pipeUsage;
}

MVOID makeDebugExif( const IMetadata *pMeta_InHal, IMetadata *pMeta_OutHal)
{
    MUINT8 isRequestExif = 0;
    tryGetMetadata<MUINT8>(pMeta_InHal, MTK_HAL_REQUEST_REQUIRE_EXIF, isRequestExif);

    if(isRequestExif)
    {
        MINT32 vhdrMode = SENSOR_VHDR_MODE_NONE;
        tryGetMetadata<MINT32>(pMeta_InHal, MTK_P1NODE_SENSOR_VHDR_MODE, vhdrMode);

        if(vhdrMode != SENSOR_VHDR_MODE_NONE)
        {
            // add HDR image flag
            std::map<MUINT32, MUINT32> debugInfoList;
            debugInfoList[getDebugExif()->getTagId_MF_TAG_IMAGE_HDR()] = 1;

            // get debug Exif metadata
            IMetadata exifMetadata;
            tryGetMetadata<IMetadata>(pMeta_OutHal, MTK_3A_EXIF_METADATA, exifMetadata);

            if (DebugExifUtils::setDebugExif(
                DebugExifUtils::DebugExifType::DEBUG_EXIF_MF,
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_KEY),
                static_cast<MUINT32>(MTK_MF_EXIF_DBGINFO_MF_DATA),
                debugInfoList,
                &exifMetadata) != nullptr)
            {
                trySetMetadata<IMetadata>(pMeta_OutHal, MTK_3A_EXIF_METADATA, exifMetadata);
            }
        } // set HDR image flag end
    }
}

MVOID P2Procedure::prepareFeatureMask_EIS(FeaturePipeParam &pipeParam, IMetadata *pMeta_InApp, IMetadata *pMeta_InHal)
{
    if( isAPEnabled_EIS(pMeta_InApp) )
    {
        MINT32 appMode = 0;
        MUINT32 eisMode = 0;
        tryGetMetadata<MINT32>(pMeta_InHal, MTK_FEATUREPIPE_APP_MODE, appMode);
        if( appMode == MTK_FEATUREPIPE_VIDEO_RECORD ||
            appMode == MTK_FEATUREPIPE_VIDEO_STOP )
        {
            IStreamingFeaturePipe::UsageHint usageHint = getPipeUsageHint(mInitParams.usageHint);
            eisMode = usageHint.mEISInfo.mode;
        }
        MY_LOGD_IF(mbEnableLog, "sensor(%d) eisMode: 0x%x", mOpenId, eisMode);

        // Set EIS featureMask
        if( EIS_MODE_IS_EIS_30_ENABLED(eisMode) )
        {
            pipeParam.setFeatureMask(MASK_EIS_30, MTRUE);
            pipeParam.setFeatureMask(MASK_EIS, MTRUE);
        }
        else if( EIS_MODE_IS_EIS_25_ENABLED(eisMode) )
        {
            pipeParam.setFeatureMask(MASK_EIS_25, MTRUE);
            pipeParam.setFeatureMask(MASK_EIS, MTRUE);
        }
        else if( EIS_MODE_IS_EIS_22_ENABLED(eisMode) )
        {
            pipeParam.setFeatureMask(MASK_EIS, MTRUE);
        }
        else
        {
            // Turn on EIS 1.2 only
            DISABLE_EIS(pipeParam.mFeatureMask);
        }
    }
    else
    {
        DISABLE_EIS(pipeParam.mFeatureMask);
    }
}

#endif // FEATURE_MODIFY


/******************************************************************************
 *
 ******************************************************************************/
} // namespace P2Feature

android::sp<P2FeatureNode>
P2FeatureNode::
createHal1Instance(ePass2Type const type, const UsageHint usageHint)
{
    if(!getDebugExif()) {
        MY_LOGE("bad getDebugExif()");
        return NULL;
    }

    if( type < 0 ||
        type >= PASS2_TYPE_TOTAL )
    {
        MY_LOGE("not supported p2 type %d", type);
        return NULL;
    }
    //
    return new P2Feature::P2NodeImp(type, usageHint);
}

namespace P2Feature {

/******************************************************************************
 *
 ******************************************************************************/
P2NodeImp::
P2NodeImp(ePass2Type const type, const UsageHint &usageHint)
    : BaseNode()
    , P2FeatureNode()
    , mType(type)
    , mConfigRWLock()
    , mpInAppMeta_Request()
    , mpInAppRetMeta_Request()
    , mpInHalMeta_P1()
    , mpOutAppMeta_Result()
    , mpOutHalMeta_Result()
    , mpvInFullRaw()
    , mpInResizedRaw()
    , mvOutImages()
    , mpOutFd()
    , mBurstNum(0)
    , mCustomOption(0)
    , mUsageHint(usageHint)
    , mOperationLock()
    //
    , mspResConCtrl(usageHint.pResourceConcurrency)
    , mResConClient(IResourceConcurrency::CLIENT_HANDLER_NULL)
    //
    , mpP2Processor(NULL)
    , mpMdpProcessor(NULL)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.camera.log", cLogLevel, "0");
    mLogLevel = atoi(cLogLevel);
    if ( mLogLevel == 0 ) {
        ::property_get("vendor.debug.camera.log.P2FeatureNode", cLogLevel, "0");
        mLogLevel = atoi(cLogLevel);
    }
#if 0 /*[EP_TEMP]*/ //[FIXME] TempTestOnly
    #warning "[FIXME] force enable P2FeatureNode log"
    if (mLogLevel < 2) {
        mLogLevel = 2;
    }
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
P2NodeImp::
~P2NodeImp()
{
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
init(InitParams const& rParams)
{
    CAM_TRACE_NAME("P2F:init");
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;

    MY_LOGD("OpenId %d, nodeId %#" PRIxPTR ", name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    MRect activeArray;
    {
        sp<IMetadataProvider> pMetadataProvider =
            NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE("sensor(%d) ! pMetadataProvider.get() ", mOpenId);
            return DEAD_OBJECT;
        }
        IMetadata static_meta =
            pMetadataProvider->getMtkStaticCharacteristics();
        if( tryGetMetadata<MRect>(&static_meta,
            MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray) ) {
            MY_LOGD_IF(1,"sensor(%d) active array(%d, %d, %dx%d)",
                    mOpenId,
                    activeArray.p.x, activeArray.p.y,
                    activeArray.s.w, activeArray.s.h);
        } else {
            MY_LOGE("sensor(%d) no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION", mOpenId);
            #ifdef USING_MTK_LDVT /*[EP_TEMP]*/ //[FIXME] TempTestOnly
            activeArray = MRect(1600, 1200);// hard-code sensor size
            MY_LOGD("sensor(%d) set sensor size to active array(%d, %d, %dx%d)",
                mOpenId,
                activeArray.p.x, activeArray.p.y,
                activeArray.s.w, activeArray.s.h);
            #else
            return UNKNOWN_ERROR;
            #endif
        }
/*
        if( mspResConCtrl != NULL && !concurrencyControlAcqurie(mspResConCtrl, mResConClient) )
        {
            MY_LOGE("sensor(%d) ! concurrencyControlAcqurie failed", mOpenId);
            return UNKNOWN_ERROR;
        }
*/
        //
        {
            mpP2Processor  = NULL;
            //
            P2Procedure::InitParams param;
            param.openId      = mOpenId;
            param.type        = mType;
            param.activeArray = activeArray;
            param.enableLog   = mLogLevel >= 1;
            param.enableDumpBuffer = mLogLevel >= 3;
            #ifdef FEATURE_MODIFY
            MUINT32 debugRescon = ::property_get_int32("vendor.debug.camera.p2f.rescon", 0);
            if(debugRescon)
                mUsageHint.pResourceConcurrency = IResourceConcurrency::createInstance("P2F_test", 3000);
            param.usageHint = mUsageHint;
            #endif // FEATURE_MODIFY
            param.resConClient = IResourceConcurrency::CLIENT_HANDLER_NULL;
            //
            mpP2Processor = P2Procedure::createProcessor(param);
            if( mpP2Processor == NULL )
                return BAD_VALUE;
        }
        //
        #ifndef FEATURE_MODIFY
        {
            mpMdpProcessor = NULL;
            //
            MDPProcedure::InitParams param;
            param.enableLog   = mLogLevel >= 1;
            //
            mpMdpProcessor = MDPProcedure::createProcessor(param);
            if( mpMdpProcessor == NULL )
                return BAD_VALUE;
        }
        mpP2Processor->setNextProcessor(mpMdpProcessor);
        #endif
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
config(ConfigParams const& rParams)
{
    CAM_TRACE_NAME("P2F:config");
    //
    {
        MERROR const err = verifyConfigParams(rParams);
        if( err != OK ) {
            MY_LOGE("sensor(%d) verifyConfigParams failed, err = %d", mOpenId, err);
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
        mpInHalMeta_P1       = rParams.pInHalMeta;
        mpInAppRetMeta_Request = rParams.pInAppRetMeta;
        mpOutAppMeta_Result  = rParams.pOutAppMeta;
        mpOutHalMeta_Result  = rParams.pOutHalMeta;
        // image
        mpvInFullRaw         = rParams.pvInFullRaw;
        mpInResizedRaw       = rParams.pInResizedRaw;
        mpInLcsoRaw          = rParams.pInLcsoRaw;
        mpInRssoRaw          = rParams.pInRssoRaw;
        mvOutImages          = rParams.vOutImage;
        mpOutFd              = rParams.pOutFDImage;
        // property
        mBurstNum            = rParams.burstNum;
        mCustomOption        = rParams.customOption;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
uninit()
{
    CAM_TRACE_NAME("P2F:uninit");
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("sensor(%d) flush failed", mOpenId);
    //
    if( mpP2Processor.get() ) {
        mpP2Processor->waitForIdle();
        mpP2Processor->close();
        //MY_LOGD("[P2] reference count:%d",mpP2Processor->getStrongCount());
        mpP2Processor = NULL;
    }
    //
    #ifndef FEATURE_MODIFY
    if( mpMdpProcessor.get() ) {
        mpMdpProcessor->waitForIdle();
        mpMdpProcessor->close();
        //MY_LOGD("MDP] reference count:%d",mpMdpProcessor->getStrongCount());
        mpMdpProcessor = NULL;
    }
    #endif
    //
/*
    if( mspResConCtrl != NULL )
    {
        if( !concurrencyControlRelease(mspResConCtrl, mResConClient) )
        {
            MY_LOGE("sensor(%d) ! concurrencyControlRelease failed", mOpenId);
        }

        mspResConCtrl = NULL;
    }
*/
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
flush()
{
    CAM_TRACE_NAME("P2F:flush");
    FUNC_START;
    //
    Mutex::Autolock _l(mOperationLock);
    // 1. wait for P2 thread
    mpP2Processor->flushRequests();
    #ifndef FEATURE_MODIFY
    // 2. wait for MDP thread
    mpMdpProcessor->flushRequests();
    #endif
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("sensor(%d) Null frame", mOpenId);
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mOperationLock);
    MY_LOGD_IF(mLogLevel >= 2, "sensor(%d) queue pass2 @ frame(%u)", mOpenId, pFrame->getFrameNo());

    onProcessFrame(pFrame);

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    //FUNC_START;
    if( mpP2Processor == NULL ) {
        MY_LOGW("sensor(%d) may not configured yet", mOpenId);
        BaseNode::flush(pFrame);
        return;
    }
    //
    // map IPipelineFrame to requests
    if( OK != mapToRequests(pFrame) ) {
        MY_LOGW("sensor(%d) map to jobs failed", mOpenId);
        BaseNode::flush(pFrame);
        return;
    }
    //
    //FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
verifyConfigParams(
    ConfigParams const & rParams
) const
{
    if  ( ! rParams.pInAppMeta.get() ) {
        MY_LOGE("sensor(%d) no in app meta", mOpenId);
        return BAD_VALUE;
    }
    if  ( ! rParams.pInHalMeta.get() ) {
        MY_LOGE("sensor(%d) no in hal meta", mOpenId);
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
        MY_LOGE("sensor(%d) no in image fullraw or resized raw", mOpenId);
        return BAD_VALUE;
    }
    if  (  0 == rParams.vOutImage.size() && !rParams.pOutFDImage.get() ) {
        MY_LOGE("sensor(%d) no out yuv image", mOpenId);
        return BAD_VALUE;
    }
    //
#define dumpStreamIfExist(str, stream)                         \
    do {                                                       \
        MY_LOGD_IF(stream.get(), "%s: id %#" PRIx64 ", %s",   \
                str,                                           \
                stream->getStreamId(), stream->getStreamName() \
               );                                              \
    } while(0)
    //
    dumpStreamIfExist("[meta] in app", rParams.pInAppMeta);
    dumpStreamIfExist("[meta] in hal", rParams.pInHalMeta);
    dumpStreamIfExist("[meta] in appRet", rParams.pInAppRetMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);
    for( size_t i = 0; i < rParams.pvInFullRaw.size(); i++ ) {
        dumpStreamIfExist("[img] in full", rParams.pvInFullRaw[i]);
    }
    dumpStreamIfExist("[img] in resized", rParams.pInResizedRaw);
    dumpStreamIfExist("[img] in lcso", rParams.pInLcsoRaw);
    dumpStreamIfExist("[img] in rsso", rParams.pInRssoRaw);
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
P2NodeImp::
getInfoIOMapSet(
    sp<IPipelineFrame> const& pFrame,
    IPipelineFrame::InfoIOMapSet& rIOMapSet
) const
{
    CAM_TRACE_NAME("P2F:getInfoIOMapSet");
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), rIOMapSet ) ) {
        MY_LOGE("sensor(%d) queryInfoIOMap failed", mOpenId);
        return NAME_NOT_FOUND;
    }
    //
    // do some check
    IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = rIOMapSet.mImageInfoIOMapSet;
    if( ! imageIOMapSet.size() ) {
        MY_LOGW("sensor(%d) no imageIOMap in frame", mOpenId);
        return BAD_VALUE;
    }
    //
    for( size_t i = 0; i < imageIOMapSet.size(); i++ ) {
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[i];
        if( imageIOMap.vIn.size() == 0 || imageIOMap.vOut.size() == 0) {
            MY_LOGE("sensor(%d) [img] #%zu wrong size vIn %zu, vOut %zu",
                    mOpenId, i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
            return BAD_VALUE;
        }
        MY_LOGD_IF(mLogLevel, "sensor(%d) frame %u:[img] #%zu, in %zu, out %zu",
                mOpenId, pFrame->getFrameNo(), i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
    }
    //
    IPipelineFrame::MetaInfoIOMapSet& metaIOMapSet = rIOMapSet.mMetaInfoIOMapSet;
    if( ! metaIOMapSet.size() ) {
        MY_LOGW("sensor(%d) no metaIOMap in frame", mOpenId);
        return BAD_VALUE;
    }
    //
    for( size_t i = 0; i < metaIOMapSet.size(); i++ ) {
        IPipelineFrame::MetaInfoIOMap const& metaIOMap = metaIOMapSet[i];
        if( !mpInAppMeta_Request.get() ||
                0 > metaIOMap.vIn.indexOfKey(mpInAppMeta_Request->getStreamId()) ) {
            MY_LOGE("sensor(%d) [meta] no in app", mOpenId);
            return BAD_VALUE;
        }
        if( !mpInHalMeta_P1.get() ||
                0 > metaIOMap.vIn.indexOfKey(mpInHalMeta_P1->getStreamId()) ) {
            MY_LOGE("sensor(%d) [meta] no in hal", mOpenId);
            return BAD_VALUE;
        }
        MY_LOGD_IF(mLogLevel, "sensor(%d) frame %u:[meta] #%zu: in %zu, out %zu",
                mOpenId, pFrame->getFrameNo(), i, metaIOMap.vIn.size(), metaIOMap.vOut.size());
    }
    //
    MY_LOGD("sensor(%d) frame %u: [image(%zu)] in/out %zu/%zu, [meta(%zu)] in/out %zu/%zu",
        mOpenId, pFrame->getFrameNo(), imageIOMapSet.size(), imageIOMapSet[0].vIn.size(), imageIOMapSet[0].vOut.size(), metaIOMapSet.size(), metaIOMapSet[0].vIn.size(), metaIOMapSet[0].vOut.size());
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isInImageStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    //
    if( isFullRawLocked(streamId) || isResizeRawLocked(streamId) ||
        isLcsoRawLocked(streamId) || isRssoRawLocked(streamId))
        return MTRUE;
    //
    MY_LOGD_IF(0, "sensor(%d) stream id %#" PRIx64 " is not in-stream", mOpenId, streamId);
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isInMetaStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    return isStream(mpInAppMeta_Request, streamId) ||
           isStream(mpInHalMeta_P1, streamId) ||
           isStream(mpInAppRetMeta_Request, streamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireImageStream(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer
)
{
    CAM_TRACE_FMT_BEGIN("P2F:ensureImgBuf sID%#" PRIxPTR , streamId);
    MERROR ret = ensureImageBufferAvailable_(
                    pFrame->getFrameNo(),
                    streamId,
                    pFrame->getStreamBufferSet(),
                    rpStreamBuffer
                    );
    CAM_TRACE_FMT_END();
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
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
        MY_LOGE("sensor(%d) pStreamBuffer == NULL", mOpenId);
        return;
    }
    //
    if( ! isInImageStream(streamId) ) {
        pStreamBuffer->markStatus(
                (status != eStreamStatus_FILLED) ?
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
P2NodeImp::
acquireImageBuffer(
    StreamId_T const streamId,
    sp<IImageStreamBuffer> const pStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer
) const
{
    if( pStreamBuffer == NULL ) {
        MY_LOGE("sensor(%d) pStreamBuffer == NULL", mOpenId);
        return BAD_VALUE;
    }
    //  Query the group usage.
    MUINT const groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
    sp<IImageBufferHeap>   pImageBufferHeap =
        isInImageStream(streamId) ?
        pStreamBuffer->tryReadLock(getNodeName()) :
        pStreamBuffer->tryWriteLock(getNodeName());

    if (pImageBufferHeap == NULL) {
        MY_LOGE("sensor(%d) [node:%#" PRIxPTR "][stream buffer:%s] cannot get ImageBufferHeap",
                mOpenId, getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (rpImageBuffer == NULL) {
        MY_LOGE("sensor(%d) [node:%#" PRIxPTR "][stream buffer:%s] cannot create ImageBuffer",
                mOpenId, getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGD_IF(mLogLevel >= 1, "sensor(%d) stream %#" PRIx64 ": buffer: %p, usage: 0x%x",
        mOpenId, streamId, rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseImageBuffer(
    sp<IImageStreamBuffer> const pStreamBuffer,
    sp<IImageBuffer> const pImageBuffer
) const
{
    if( pStreamBuffer == NULL || pImageBuffer == NULL ) {
        MY_LOGE("sensor(%d) pStreamBuffer && pImageBuffer should not be NULL", mOpenId);
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
P2NodeImp::
acquireMetaStream(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>& rpStreamBuffer
)
{
    CAM_TRACE_NAME("P2F:acquireMetaStream");
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
P2NodeImp::
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
        MY_LOGE("sensor(%d) StreamId %#" PRIx64 ": pStreamBuffer == NULL",
                mOpenId, streamId);
        return;
    }
    //
    //Buffer Producer must set this status.
    if( ! isInMetaStream(streamId) ) {
        pStreamBuffer->markStatus(
                (status != eStreamStatus_FILLED) ?
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
P2NodeImp::
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
        MY_LOGE("sensor(%d) [node:%#" PRIxPTR "][stream buffer:%s] cannot get metadata",
                mOpenId, getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGD_IF(0,"sensor(%d) stream %#" PRIx64 ": stream buffer %p, metadata: %p",
        mOpenId, streamId, pStreamBuffer.get(), rpMetadata);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseMetadata(
    sp<IMetaStreamBuffer> const pStreamBuffer,
    IMetadata* const pMetadata
) const
{
    if( pMetadata == NULL ) {
        MY_LOGW("sensor(%d) pMetadata == NULL", mOpenId);
        return;
    }
    pStreamBuffer->unlock(getNodeName(), pMetadata);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onPartialFrameDone(
    sp<IPipelineFrame> const& pFrame
)
{
    CAM_TRACE_NAME("P2F:PartialFrameDone");
    //FUNC_START;
    //MY_LOGD_IF(1, "sensor(%d) ReqNo %u frame %u applyRelease", mOpenId, pFrame->getRequestNo(), pFrame->getFrameNo());
    IStreamBufferSet&     rStreamBufferSet = pFrame->getStreamBufferSet();
    CAM_TRACE_FMT_BEGIN("P2F:applyRelease(%u)", pFrame->getFrameNo());
    rStreamBufferSet.applyRelease(getNodeId());
    CAM_TRACE_FMT_END();
    //FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onFrameDone(
    sp<IPipelineFrame> const& pFrame
)
{
    CAM_TRACE_NAME("P2F:FrameDone");
    //MY_LOGD("frame %u done", pFrame->getFrameNo());
    onDispatchFrame(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
mapToRequests(
    android::sp<IPipelineFrame> const& pFrame
)
{
    //
    // 1. get IOMap
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if( OK != getInfoIOMapSet(pFrame, IOMapSet) ) {
        MY_LOGE("sensor(%d) queryInfoIOMap failed", mOpenId);
        return BAD_VALUE;
    }
    //
    // 2. create metadata handle (based on IOMap)
    sp<MetaHandle> pMeta_InApp  = mpInAppMeta_Request.get() ?
        MetaHandle::create(this, pFrame, mpInAppMeta_Request->getStreamId()) : NULL;
    sp<MetaHandle> pMeta_InHal  = mpInHalMeta_P1.get() ?
        MetaHandle::create(this, pFrame, mpInHalMeta_P1->getStreamId()) : NULL;

    if( pMeta_InApp  == NULL ||
        pMeta_InHal  == NULL )
    {
        MY_LOGW("sensor(%d) meta check failed: pMeta_InApp == NULL | pMeta_InHal == NULL", mOpenId);
        return BAD_VALUE;
    }

#ifdef FEATURE_MODIFY
    MINT32 iso = -1;
    MINT64 timestamp = -1;
    MUINT8 afstate = 0;
    MUINT8 lensstate = 0;
    MUINT8 ispProfile = 0;
    MINT32 magic3A = 0;
    MINT32 lv_value = 0;
    MINT32 ctrl_caltm_enable = VALUE_UNDEFINED;
    {
        // get iso from inAppRet
        sp<MetaHandle> pMeta_InAppRet  = mpInAppRetMeta_Request.get() ?
            MetaHandle::create(this, pFrame, mpInAppRetMeta_Request->getStreamId()) : NULL;

        CAM_TRACE_BEGIN("P2F:mapToRequests::GetMetaData");

        if (pMeta_InAppRet.get() &&
            !tryGetMetadata<MINT32>(pMeta_InAppRet->getMetadata(), MTK_SENSOR_SENSITIVITY, iso))
        {
            MY_LOGW("no MTK_SENSOR_SENSITIVITY from AppRetMeta");
        }
        if (iso == -1)
        {
            IMetadata *inApp = pMeta_InApp->getMetadata();
            if (inApp != NULL && !tryGetMetadata<MINT32>(inApp, MTK_SENSOR_SENSITIVITY, iso))
                MY_LOGW("no MTK_SENSOR_SENSITIVITY from AppMeta");
        }

        //
        if (!tryGetMetadata<MUINT8>(pMeta_InHal->getMetadata(), MTK_3A_ISP_PROFILE, ispProfile))
        {
            MY_LOGW_IF(mLogLevel,"no ISPProfile from HalMeta");
        }

        //
        if (!tryGetMetadata<MINT32>(pMeta_InHal->getMetadata(), MTK_REAL_LV, lv_value))
        {
            MY_LOGW_IF(mLogLevel,"no lv_value from HalMeta");
        }

        //
        if (!tryGetMetadata<MBOOL>(pMeta_InHal->getMetadata(), MTK_P2NODE_CTRL_CALTM_ENABLE, ctrl_caltm_enable))
        {
            ctrl_caltm_enable = VALUE_UNDEFINED;
        }

        // ************************ for Dual Cam p2Sync ****************************
        // get timestamp(ISP) for frame sync checking
        // MTK_P1NODE_FRAME_START_TIMESTAMP = MTK_SENSOR_TIMESTAMP + exposure time
        if (!tryGetMetadata<MINT64>(pMeta_InHal->getMetadata(), MTK_P1NODE_FRAME_START_TIMESTAMP, timestamp))
        {
            MY_LOGW("no MTK_P1NODE_FRAME_START_TIMESTAMP from HalMeta");
            if (!tryGetMetadata<MINT64>(pMeta_InAppRet->getMetadata(), MTK_SENSOR_TIMESTAMP, timestamp))
            {
                MY_LOGW("no MTK_SENSOR_TIMESTAMP from AppMeta");
            }
        }
        tryGetMetadata<MINT32>(pMeta_InHal->getMetadata(), MTK_P1NODE_PROCESSOR_MAGICNUM, magic3A);

        // get af state
        if (!tryGetMetadata<MUINT8>(pMeta_InAppRet->getMetadata(), MTK_CONTROL_AF_STATE, afstate))
        {
            MY_LOGW_IF(mLogLevel,"no MTK_CONTROL_AF_STATE from AppMeta");
        }
        // get lens state
        if (!tryGetMetadata<MUINT8>(pMeta_InAppRet->getMetadata(), MTK_LENS_STATE, lensstate))
        {
            MY_LOGW_IF(mLogLevel,"no MTK_LENS_STATE from AppMeta");
        }
        // *************************************************************************
        CAM_TRACE_END();

        CAM_TRACE_BEGIN("P2F:mapToRequests::Destruct MetaHandle");
        // explicitly destruct MetaHandle
        pMeta_InAppRet = NULL;
        CAM_TRACE_END();
    }
#endif // FEATURE_MODIFY
    //
   // 3. create FrameLifeHolder
    sp<FrameLifeHolder> pFrameLife = new FrameLifeHolder(mOpenId, this, pFrame, mLogLevel >= 1);
    // 4. process image IO
    IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = IOMapSet.mImageInfoIOMapSet;
    for( size_t run_idx = 0 ; run_idx < imageIOMapSet.size(); run_idx++ )
    {
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[run_idx];
        sp<Request> pRequest = new Request(pFrameLife);
        pRequest->frameNo = pFrame->getFrameNo();
        pRequest->timestamp = timestamp;
        pRequest->index = run_idx;
        pRequest->context.fd_stream_id = mpOutFd.get() ? mpOutFd->getStreamId() : 0;

        pRequest->context.iso = iso;
        pRequest->context.isp_profile = ispProfile;
        pRequest->afstate = afstate;
        pRequest->lensstate = lensstate;
        pRequest->context.magic3A = magic3A;
        pRequest->context.lv_value = lv_value;
        pRequest->context.ctrl_caltm_enable = ctrl_caltm_enable;
        if (mLogLevel >= 2) {
            MY_LOGD("sensor(%d) [StreamID] run_idx(%zu) vIn.size(%zu) +++",
                mOpenId, run_idx, imageIOMap.vIn.size());
            for (unsigned i = 0; i < imageIOMap.vIn.size(); ++i) {
                StreamId_T const sId = imageIOMap.vIn.keyAt(i);
                MY_LOGD("sensor(%d) [StreamID] In(%u) sId(%#" PRIx64 ") F(%d) R(%d)",
                    mOpenId, i, sId, isFullRawLocked(sId), isResizeRawLocked(sId));
            }
            MY_LOGD("sensor(%d) [StreamID] run_idx(%zu) vIn.size(%zu) ---",
                mOpenId, run_idx, imageIOMap.vIn.size());
        }
        // source
        for(size_t i = 0; i < imageIOMap.vIn.size() ; i++)
        {
            StreamId_T const sId = imageIOMap.vIn.keyAt(i);
            if(isFullRawLocked(sId) || isResizeRawLocked(sId))
            {
                pRequest->context.in_buffer = StreamBufferHandle::create(this, pFrame, sId, pRequest);
            }
            else if(isLcsoRawLocked(sId))
            {
                pRequest->context.in_lcso_buffer = StreamBufferHandle::create(this, pFrame, sId, pRequest);
            }
            else if(isRssoRawLocked(sId))
            {
                pRequest->context.in_rsso_buffer = StreamBufferHandle::create(this, pFrame, sId, pRequest);
            }
        }
        if( pRequest->context.in_buffer == NULL )
        {
            MY_LOGW("sensor(%d) get input buffer failed", mOpenId);
            return BAD_VALUE;
        }
        StreamId_T const streamId = pRequest->context.in_buffer->getStreamId();
        //
        {
            RWLock::AutoRLock _l(mConfigRWLock);
            pRequest->context.resized = isResizeRawLocked(streamId);
        }
        // determine whether burst or not
#if FORCE_BURST_ON
        pRequest->context.burst_num = pRequest->context.resized ? 4 : 0;
#else
        pRequest->context.burst_num = pRequest->context.resized ? mBurstNum : 0;
#endif
        pRequest->context.custom_option = mCustomOption;

        // destination
        MBOOL dropOutBuffer = MFALSE;
        MINT32 dualDropReq = 0;
        pRequest->skipOutCheck = MFALSE;
        if(tryGetMetadata<MINT32>(pMeta_InHal->getMetadata(), MTK_DUALZOOM_DROP_REQ, dualDropReq))
        {
            if (dualDropReq == MTK_DUALZOOM_DROP_NEED_SYNCMGR ||
                dualDropReq == MTK_DUALZOOM_DROP_NEED_SYNCMGR_NEED_STREAM_F_PIPE)
            {
                dropOutBuffer = MTRUE;
                pRequest->skipOutCheck = MTRUE;
            }
        }

        for( size_t i = 0; i < imageIOMap.vOut.size(); i++ )
        {
            StreamId_T const streamId = imageIOMap.vOut.keyAt(i);
            //MUINT32 const transform = imageIOMap.vOut.valueAt(i)->getTransform();
            if(dropOutBuffer)
            {
                IStreamBufferSet& rStreamBufferSet = pFrame->getStreamBufferSet();
                rStreamBufferSet.markUserStatus(
                        streamId, getNodeId(),
                        IUsersManager::UserStatus::RELEASE);
            }
            else
            {
                pRequest->context.out_buffers.push_back(StreamBufferHandle::create(this, pFrame, streamId, pRequest));
            }
        }

        //
        TuningUtils::FILE_DUMP_NAMING_HINT hint;
        extract(&pRequest->context.hint, pMeta_InHal->getMetadata());
        extract_by_SensorOpenId(&pRequest->context.hint, mOpenId);

        //
        pRequest->context.in_app_meta = pMeta_InApp;
        pRequest->context.in_hal_meta = pMeta_InHal;

        if (run_idx == 0)
        {
            pRequest->context.out_app_meta = mpOutAppMeta_Result.get() ?
                MetaHandle::create(this, pFrame, mpOutAppMeta_Result->getStreamId()) : NULL;
            pRequest->context.out_hal_meta= mpOutHalMeta_Result.get() ?
                MetaHandle::create(this, pFrame, mpOutHalMeta_Result->getStreamId()) : NULL;
        }
        // prevent HAL1 that a frame have no buffer from dispatching faster than this scope
        if (run_idx == imageIOMapSet.size() - 1)
        {
            pMeta_InApp.clear();
            pMeta_InHal.clear();
        }

        mpP2Processor->queueRequest(pRequest);
        //
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2Procedure::
packDualCamInHalMetadata(
    MINT64 timestamp,
    MUINT8 afstate,
    MUINT8 lensstate,
    IMetadata *inHal
)
{
    trySetMetadata<MINT64>(inHal, MTK_DUALCAM_TIMESTAMP, timestamp);
    trySetMetadata<MUINT8>(inHal, MTK_DUALCAM_AF_STATE, afstate);
    trySetMetadata<MUINT8>(inHal, MTK_DUALCAM_LENS_STATE, lensstate);
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
mapPortId(
    StreamId_T const streamId, // [in]
    MUINT32 const transform,   // [in]
    MBOOL const isFdStream,    // [in]
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
#if FD_PORT_SUPPORT
        if( FD_PORT_SUPPORT && isFdStream ) {
            if( rOccupied & PORT_IMG2O_USED ) {
                MY_LOGW("sensor(%d) should not be occupied", mOpenId);
                ret = INVALID_OPERATION;
            } else {
                rOccupied |= PORT_IMG2O_USED;
                rPortId = PORT_IMG2O;
            }
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
    MY_LOGD_IF(0, "sensor(%d) stream id %#" PRIx64 ", occupied 0x%x",
            mOpenId, streamId, rOccupied);
    return ret;
#undef PORT_WDMAO_USED
#undef PORT_WROTO_USED
#undef PORT_IMG2O_USED
}

/******************************************************************************
 *
 ******************************************************************************/
sp<MetaHandle>
MetaHandle::
create(
    StreamControl* const pCtrl,
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId
)
{
    CAM_TRACE_NAME("P2F:MetaHandle:create");
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
            return new MetaHandle(
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
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MetaHandle::
~MetaHandle()
{
    CAM_TRACE_NAME("P2F:~MetaHandle");
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
    //MY_LOGD("release meta[%d] streamId[0x%x]",mpFrame->getFrameNo() ,mStreamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MetaHandle::
updateState(BufferState_t const state)
{
    Mutex::Autolock _l(mLock);
    if( muState == STATE_NOT_USED ) {
        MY_LOGW("streamId %#" PRIx64 " state %d -> %d",
            mStreamId, muState, state);
    }
    else {
        MY_LOGD_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIx64 " set, state %d -> %d",
                mStreamId, muState, state);
        muState = state;
    }
    //mCond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
sp<BufferHandle>
StreamBufferHandle::
create(
    StreamControl* const pCtrl,
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    wp<Request> wpRequest
)
{
    CAM_TRACE_NAME("P2F:StreamBufferHandle:create");
    // check StreamBuffer here
    sp<IImageStreamBuffer> pStreamBuffer = NULL;
    if( OK == pCtrl->acquireImageStream(
                pFrame,
                streamId,
                pStreamBuffer) )
    {
        MUINT32 uTransform = pStreamBuffer->getStreamInfo()->getTransform();
        MUINT32 uUsage = pStreamBuffer->getStreamInfo()->getUsageForAllocator();
        MY_LOGD_IF(0, "create buffer handler, stream:%#" PRIx64 ", transform:%d, usage:%d",
            streamId, uTransform, uUsage);
        return new StreamBufferHandle(
                pCtrl, pFrame,
                streamId, pStreamBuffer,
                uTransform, uUsage, wpRequest);
    }
    //
    return NULL;
}

/******************************************************************************
 *
 ******************************************************************************/
StreamBufferHandle::StreamBufferHandle(
    StreamControl* pCtrl,
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer> const pStreamBuffer,
    MUINT32 const uTransform,
    MUINT32 const uUsage,
    wp<Request> wpRequest
)
    : mpStreamCtrl(pCtrl)
    , mpFrame(pFrame)
    , mStreamId(streamId)
    , mpStreamBuffer(pStreamBuffer)
    , muState(STATE_NOT_USED)
    , muTransform(uTransform)
    , muUsage(uUsage)
    , mpImageBuffer(NULL)
    #ifdef FEATURE_MODIFY
    , mEarlyReleased(MFALSE)
    , mBorrowed(MFALSE)
    #endif // FEATURE_MODIFY
    , mwpRequest(wpRequest)
{
    notifyState();
}

/******************************************************************************
 *
 ******************************************************************************/
StreamBufferHandle::
~StreamBufferHandle()
{
    CAM_TRACE_NAME("P2F:~StreamBufferHandle");

    TRACE_RSS("released=%d, borrowed=%d", mEarlyReleased, mBorrowed);
    #ifdef FEATURE_MODIFY
    if( mEarlyReleased )
    {
        return;
    }
    #endif // FEATURE_MODIFY
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

    muState = STATE_RELEASED;
    notifyState();
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferHandle::
waitState(
    BufferState_t const state,
    nsecs_t const nsTimeout
)
{
    Mutex::Autolock _l(mLock);
    CAM_TRACE_FMT_BEGIN("P2F:waitState Id %#"  PRIx64 "", mStreamId);
    if( mpImageBuffer == NULL ) {
        // get buffer from streambuffer
        const MERROR ret = mpStreamCtrl->acquireImageBuffer(mStreamId, mpStreamBuffer, mpImageBuffer);
        // update initial state
        if( ret == OK )
            muState = mpStreamCtrl->isInImageStream(mStreamId) ? STATE_READABLE : STATE_WRITABLE;
        //return ret;
    }
    //
    if( muState != state ) {
        mCond.waitRelative(mLock, nsTimeout);
    }
    CAM_TRACE_END();

    notifyState();

    return (muState == state) ? OK : TIMED_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferHandle::
updateState(BufferState_t const state)
{
    Mutex::Autolock _l(mLock);
    if( muState == STATE_NOT_USED ) {
        MY_LOGW("streamId %#" PRIx64 " state %d -> %d",
            mStreamId, muState, state);
    }
    else {
        MY_LOGD_IF(state == STATE_WRITE_FAIL, "streamId %#" PRIx64 " set : state %d -> %d",
                mStreamId, muState, state);
        muState = state;
    }
    notifyState();
    mCond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
#ifdef FEATURE_MODIFY
MBOOL
StreamBufferHandle::
earlyRelease(MBOOL force)
{
    CAM_TRACE_NAME("P2F:earlyRelease");

    TRACE_RSS("released=%d, borrowed=%d, force=%d", mEarlyReleased, mBorrowed, force);
    if( mEarlyReleased || (mBorrowed && !force) )
    {
        return MFALSE;
    }
    mEarlyReleased = MTRUE;
    if( muState != STATE_NOT_USED )
    {
        MUINT32 status = (muState != STATE_WRITE_FAIL) ?
            StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
        mpStreamCtrl->releaseImageBuffer(mpStreamBuffer, mpImageBuffer);
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, status);
    }
    else
    {
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
    }

    muState = STATE_RELEASED;
    notifyState();
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferHandle::
setBorrowed()
{
    mBorrowed = MTRUE;
}
#endif // FEATURE_MODIFY

/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferHandle::
notifyState()
{
    sp<Request> spRequest = mwpRequest.promote();
    if(spRequest.get())
    {
        spRequest->updateBufferHandleStatus(mStreamId, (MUINT32)muState);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
P2Procedure::
createProcessor(InitParams& params)
{
    CAM_TRACE_NAME("P2F:[Proc]createProcessor");
    IStreamingFeaturePipe* pPipe = NULL;
    IStreamingFeaturePipe::UsageHint usageHint = getPipeUsageHint(params.usageHint);
    IHal3A_T* p3A = NULL;
    ENormalStreamTag streamtag = ENormalStreamTag_Prv;
    if( params.type == P2FeatureNode::PASS2_STREAM ) {
        streamtag = ENormalStreamTag_Prv;
    }
    else if ( params.type == P2FeatureNode::PASS2_TIMESHARING ) {
        streamtag = ENormalStreamTag_Vss;
    }
    else {
        MY_LOGE("sensor(%d) not supported type %d", params.openId, params.type);
        goto lbExit;
    }
    //
    CAM_TRACE_BEGIN("P2F:[Proc]NormalStream create");

    #if DUAL_CAM_SUPPORT
    if (usageHint.mDualMode)
    {
        pPipe = IDualCamStreamingFeaturePipe::createInstance(params.openId, usageHint);
    }
    else
    #endif
    {
        pPipe = IStreamingFeaturePipe::createInstance(params.openId, usageHint);
    }

    if( !concurrencyControlAcqurie(params.usageHint.pResourceConcurrency, params.resConClient) )
    {
        MY_LOGE("sensor(%d) ! concurrencyControlAcqurie failed", params.openId);
        goto lbExit;
    }

    //
    if( pPipe == NULL ) {
        MY_LOGE("sensor(%d) create pipe failed", params.openId);
        CAM_TRACE_END();
        goto lbExit;
    }
    //
    CAM_TRACE_BEGIN("P2F:[Proc]NormalStream init");
    if( ! pPipe->init(LOG_TAG) )
    {
        CAM_TRACE_END();
        MY_LOGE("sensor(%d) pipe init failed", params.openId);
        goto lbExit;
    }
    CAM_TRACE_END();
    //
    #if SUPPORT_3A
    CAM_TRACE_BEGIN("P2F:[Proc]3A create");
    p3A = MAKE_Hal3A(params.openId, LOG_TAG);
    CAM_TRACE_END();
    #endif
    if( p3A == NULL ) {
        MY_LOGE("sensor(%d) create 3A failed", params.openId);
        goto lbExit;
    }
    MY_LOGD("sensor(%d) create processor type %d: pipe %p, 3A %p",
            params.openId, params.type, pPipe, p3A);
    //
lbExit:
    if( ! pPipe || !p3A ) {

        if( params.usageHint.pResourceConcurrency != NULL )
        {
            if( !concurrencyControlRelease(params.usageHint.pResourceConcurrency, params.resConClient) )
            {
                MY_LOGE("sensor(%d) ! concurrencyControlRelease failed", params.openId);
            }
            params.usageHint.pResourceConcurrency = NULL;
        }

        if( pPipe ) {
            pPipe->uninit(LOG_TAG);
            pPipe->destroyInstance();
            pPipe = NULL;
        }

        if( p3A ) {
            p3A->destroyInstance(LOG_TAG);
            p3A = NULL;
        }
    }

    params.pPipe        = pPipe;
    params.p3A          = p3A;
    return pPipe ? new ProcessorBase<P2Procedure>(params, PROCESSOR_NAME_P2) : NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
P2Procedure::
~P2Procedure()
{
    MY_LOGD("sensor(%d) destroy processor %d: %p",
            mOpenId, mInitParams.type, mpPipe);
    //
    if( mpPipe ) {
        if( ! mpPipe->uninit(LOG_TAG) ) {
            MY_LOGE("sensor(%d) pipe uninit failed", mOpenId);
        }

        if( mspResConCtrl != NULL )
        {
            if( !concurrencyControlRelease(mspResConCtrl, mResConClient) )
            {
                MY_LOGE("sensor(%d) ! concurrencyControlRelease failed", mOpenId);
            }

            mspResConCtrl = NULL;
        }

        mpPipe->destroyInstance();
    }

    if( mp3A ) {
        mp3A->destroyInstance(LOG_TAG);
    }

    if( mpMultiFrameHandler ) {
        delete mpMultiFrameHandler;
    }

    if( mpDebugScanLine != NULL )
    {
        mpDebugScanLine->destroyInstance();
        mpDebugScanLine = NULL;
    }

    if( mpDebugDrawID != NULL )
    {
        mpDebugDrawID->destroyInstance();
        mpDebugDrawID = NULL;
    }

    if( mpFDContainer != NULL )
    {
        mpFDContainer = NULL;
    }

    if (mSecureEnum)
    {
        mSecBufCtrl.destroy();
    }

#ifdef FEATURE_MODIFY
    mUtil3dnr = NULL;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
#ifdef FEATURE_MODIFY
static sp<Request> getP2Request(FeaturePipeParam &param)
{
    return param.getVar<sp<Request> >(VAR_P2_REQUEST, NULL);
}

static MINT32 getP2RequestNo(FeaturePipeParam &param)
{
    MINT32 num = -1;
    sp<Request> request = param.getVar<sp<Request> >(VAR_P2_REQUEST, NULL);
    if( request != NULL )
    {
        num = request->frameNo;
    }
    return num;
}

static P2Procedure* getP2Procedure(const FeaturePipeParam &param)
{
    P2Procedure* proc = reinterpret_cast<P2Procedure*>(param.mQParams.mpCookie);
    return proc;
}

static MVOID releaseP2ObjectReference(FeaturePipeParam &param)
{
    // release life cycle
    CAM_TRACE_CALL();
    param.clearVar<sp<Request> >(VAR_P2_REQUEST);
}

MVOID P2Procedure::prepareFrameParam(
    QParams &enqueParams,
    const sp<Request> &pRequest,
    MINT32 czTimeStamp)
{
    CAM_TRACE_NAME("P2F:prepareFrameParam");
    NSCam::NSIoPipe::FrameParams f = NSCam::NSIoPipe::FrameParams();
    f.FrameNo = pRequest->getPipelineFrame()->getFrameNo();
    f.RequestNo = pRequest->getPipelineFrame()->getRequestNo();
    f.Timestamp = czTimeStamp;
    f.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal;
    f.mSecureFra = mSecure;
    enqueParams.mvFrameParams.push_back(f);
}

MVOID P2Procedure::prepareCustomSettingPtr(
    void* &pCustomSetting,
    uint32_t &customIndex,
    DP_ISP_FEATURE_ENUM ispFeature,
    MINT32 magic3A)
{
#if MTK_CAM_NEW_NVRAM_SUPPORT
    if( mpHalSensorList )
    {
        MUINT sensorDev = mpHalSensorList->querySensorDevIdx(mOpenId);
        IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));
        CAM_IDX_QRY_COMB rMapping_Info;
        pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magic3A);

        customIndex = 0;

        if( ispFeature == ISP_FEATURE_CLEARZOOM )
        {
            customIndex = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_ClearZoom, rMapping_Info, __FUNCTION__);
            MY_LOGD_IF(mbEnableLog, "EModule_ClearZoom mappingInfo index: %d", customIndex);
            pCustomSetting = &(mpNvram->ClearZoom[customIndex]);
        }
        else if( ispFeature == ISP_FEATURE_DRE )
        {
            customIndex = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_CA_LTM, rMapping_Info, __FUNCTION__);
            MY_LOGD_IF(mbEnableLog, "EModule_CA_LTM mappingInfo index: %d", customIndex);
            pCustomSetting = &(mpNvram->CA_LTM[customIndex]);
        }
        else
        {
            MY_LOGW("unknown ispFeature %d", ispFeature);
            pCustomSetting = NULL;
        }
    }
#else
    customIndex = 0;
    pCustomSetting = NULL;
#endif
}

MVOID P2Procedure::prepareFaceInfoPtr(
        void* &pFaceInfo
        )
{
    pFaceInfo = NULL;

    if( mpFDContainer != NULL )
    {
        vector<FD_DATATYPE*> fdDatas = mpFDContainer->queryLock();
        if( !fdDatas.empty() )
        {
            FD_DATATYPE* fdData = new FD_DATATYPE();
            fdDatas.back()->clone(*fdData);
            pFaceInfo = (void*)fdData;
        }
        mpFDContainer->queryUnlock(fdDatas);
    }
}

MVOID P2Procedure::preparePQParam(DpPqParam &pqParam,
    const Output &dst,
    const sp<Request> &pRequest,
    MINT32 czTimeStamp)
{
    pqParam.scenario = MEDIA_ISP_PREVIEW;
    pqParam.enable  = false;

    DpIspParam& ispParam = pqParam.u.isp;

    ispParam.iso = pRequest->context.iso;
    ispParam.timestamp = czTimeStamp;
    ispParam.frameNo = pRequest->getPipelineFrame()->getFrameNo();
    ispParam.requestNo = pRequest->getPipelineFrame()->getRequestNo();
    ispParam.lensId = mOpenId;
    ispParam.userString[0] = '\0';
    ispParam.LV = pRequest->context.lv_value;

    //platform support
    bool supportClearZoom = mIspFeatureSupport[ISP_FEATURE_CLEARZOOM];
    bool supportDRE = mIspFeatureSupport[ISP_FEATURE_DRE];

    //custom control
    supportClearZoom &= pRequest->context.custom_option & P2FeatureNode::CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT ? true : false;
    supportDRE &= pRequest->context.custom_option & P2FeatureNode::CUSTOM_OPTION_DRE_SUPPORT ? true : false;

    if(pRequest->context.ctrl_caltm_enable != VALUE_UNDEFINED)
    {
        supportDRE = supportDRE && ( pRequest->context.ctrl_caltm_enable == MTRUE );
    }

    if( dst.mPortID.capbility == EPortCapbility_Disp && supportClearZoom )
    {
        pqParam.enable |= (PQ_COLOR_EN | PQ_ULTRARES_EN);
        ispParam.clearZoomParam.captureShot = CAPTURE_SINGLE;//MDP doesn't refer captureShot when preview
        uint32_t customIndex = 0;

        prepareCustomSettingPtr(ispParam.clearZoomParam.p_customSetting, customIndex, ISP_FEATURE_CLEARZOOM, pRequest->context.magic3A);
    }

    if( supportDRE )
    {
        pqParam.enable |= (PQ_DRE_EN);
        DpDREParam& DREParam = ispParam.dpDREParam;
        DREParam.cmd         = DpDREParam::Cmd::Initialize | DpDREParam::Cmd::Default;
        DREParam.userId      = (((unsigned long long)MEDIA_ISP_PREVIEW)<<32);
        DREParam.buffer      = nullptr;

        prepareCustomSettingPtr(DREParam.p_customSetting, DREParam.customIndex, ISP_FEATURE_DRE, pRequest->context.magic3A);
        prepareFaceInfoPtr(ispParam.p_faceInfor);
    }

    MY_LOGD_IF(mbEnableLog, "{port.index:%d port.cap:%d enable:0x%X iso:%d timestamp:%d frameNo:%d requestNo:%d lensId:%d LV:%d fd:%p CZ.captureShot:%d CZ.cust:%p dre.cust:%p dre.idx:%d addr:%p}",
        dst.mPortID.index,
        dst.mPortID.capbility,
        pqParam.enable,
        ispParam.iso,
        ispParam.timestamp,
        ispParam.frameNo,
        ispParam.requestNo,
        ispParam.lensId,
        ispParam.LV,
        ispParam.p_faceInfor,
        ispParam.clearZoomParam.captureShot,
        ispParam.clearZoomParam.p_customSetting,
        ispParam.dpDREParam.p_customSetting,
        ispParam.dpDREParam.customIndex,
        &ispParam);
}

MVOID P2Procedure::initNvramPtr()
{
    if( mpHalSensorList )
    {
        MUINT sensorDev = mpHalSensorList->querySensorDevIdx(mOpenId);

        auto pNvBufUtil = MAKE_NvBufUtil();
        if( pNvBufUtil != NULL )
        {
            auto result = pNvBufUtil->getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)mpNvram);
            if( result != 0 )
            {
                MY_LOGE("read buffer chunk fail");
                mpNvram = NULL;
            }
        }
        else
        {
            MY_LOGE("pNvBufUtil==0");
        }
    }
    else
    {
        MY_LOGE("pIHalSensorList==0");
    }
}

MVOID P2Procedure::appendHandleDebugString(
    sp<BufferHandle> handle,
    const char *tag,
    String8 &str)
{
    if( handle != NULL )
    {
        str += String8::format("%sId %#"  PRIx64 "%s", str.isEmpty() ? "" : ", ", handle->getStreamId(), tag);
    }
}

MVOID P2Procedure::prepareFeaturePipeParam(FeaturePipeParam &featureEnqueParams,
              const QParams &enqueParams,
              const sp<Request> &pRequest,
              IMetadata *pMeta_InApp,
              IMetadata *pMeta_InHal,
              IMetadata *pMeta_OutApp,
              IMetadata *pMeta_OutHal,
              const Cropper::crop_info_t &cropInfos)
{
    CAM_TRACE_NAME("P2F:prepareFeaturePipeParam");

    (void)pMeta_OutApp;
    (void)pMeta_OutHal;

    featureEnqueParams.setVar<sp<Request> >(VAR_P2_REQUEST, pRequest);

    MBOOL appEn3DNR = isAPEnabled_3DNR((mInitParams.usageHint.m3DNRMode &
        NSCam::NR3D::E3DNR_MODE_MASK_HAL_FORCE_SUPPORT), pMeta_InApp, pMeta_InHal);
    if (appEn3DNR)
    {
        featureEnqueParams.setFeatureMask(MASK_3DNR, appEn3DNR);
        featureEnqueParams.setFeatureMask(MASK_3DNR_RSC, (mInitParams.usageHint.m3DNRMode &
            NSCam::NR3D::E3DNR_MODE_MASK_RSC_EN));
    }
    featureEnqueParams.setFeatureMask(MASK_FSC, EFSC_FSC_ENABLED(mInitParams.usageHint.mFSCMode));
    featureEnqueParams.setFeatureMask(MASK_VHDR, isHALenabled_VHDR(pMeta_InHal));
    featureEnqueParams.setFeatureMask(MASK_VFB, isAPEnabled_VFB(pMeta_InApp));
    featureEnqueParams.setFeatureMask(MASK_VFB_EX, isAPEnabled_VFB_EX(pMeta_InApp));
    prepareFeatureMask_EIS(featureEnqueParams, pMeta_InApp, pMeta_InHal);

    if (HAS_VHDR(featureEnqueParams.mFeatureMask))
    {
        if (HAS_VFB(featureEnqueParams.mFeatureMask) || HAS_VFB_EX(featureEnqueParams.mFeatureMask))
        {
            MY_LOGE("sensor(%d) z/i vHDR could NOT be with vFB&vFB_EX", mOpenId);
            DISABLE_VFB(featureEnqueParams.mFeatureMask);
            DISABLE_VFB_EX(featureEnqueParams.mFeatureMask);
        }
    }

    if (HAS_VFB(featureEnqueParams.mFeatureMask) || HAS_VFB_EX(featureEnqueParams.mFeatureMask))
    {
        //If vFBx is enabled, turn on EIS 1.2 only
        DISABLE_EIS(featureEnqueParams.mFeatureMask);
    }

    //
    #if DUAL_CAM_SUPPORT
    int value = mInitParams.usageHint.mDualMode;
    if (value)
    {
        MetaSet_T inMetaSet;
        inMetaSet.appMeta = *pMeta_InApp;
        inMetaSet.halMeta = *pMeta_InHal;
        packDualCamInHalMetadata(
                            pRequest->timestamp,
                            pRequest->afstate,
                            pRequest->lensstate,
                            &inMetaSet.halMeta);
        IDualCamStreamingFeaturePipe::prepareFeatureData(featureEnqueParams, pRequest->frameNo,
            &inMetaSet.appMeta, &inMetaSet.halMeta, pMeta_OutApp, pMeta_OutHal, value);

        prepareFeatureData_DualCam(featureEnqueParams, pMeta_InApp, pMeta_InHal);
    }
    #endif

    prepareFeatureData_3DNR(
        featureEnqueParams,
        enqueParams,
        cropInfos,
        pRequest, pMeta_InApp, pMeta_InHal
        );
    prepareFeatureData_FSC(featureEnqueParams, pMeta_InHal);
    prepareFeatureData_VHDR(featureEnqueParams, pMeta_InHal);
    prepareFeatureData_VFB(featureEnqueParams);
    prepareFeatureData_EIS(featureEnqueParams, pMeta_InApp, pMeta_InHal);
    prepareFeatureData_N3D(featureEnqueParams, pMeta_InApp);
    prepareFeatureData_Common(featureEnqueParams, pRequest, pMeta_InApp, pMeta_InHal, cropInfos);
}

MBOOL P2Procedure::setP2B3A(FeaturePipeParam &param)
{
    MBOOL ret = MFALSE;
    void *pTuning;
    if( param.tryGetVar<void*>("p2b_3a_tuning", pTuning) )
    {
        sp<Request> request;
        request = getP2Request(param);
        if( request != NULL )
        {
            IMetadata *appInMeta = request->context.in_app_meta->getMetadata();
            IMetadata *halInMeta = request->context.in_hal_meta->getMetadata();
            if( halInMeta && appInMeta )
            {
                TuningParam rTuningParam(pTuning);
                MetaSet_T inMetaSet;
                inMetaSet.appMeta = *halInMeta;
                inMetaSet.halMeta = *appInMeta;
                // trySetMetadata<MUINT8>(halInMeta, MTK_3A_ISP_PROFILE, EIspProfile_VFB_PostProc);
                mp3A->setIsp(0, inMetaSet, &rTuningParam, NULL/*outMetaSet*/);
                ret = MTRUE;
            }
        }
    }
    return ret;
}

sp<BufferLifeHolder> P2Procedure::updateBufferLifeHolder(const sp<BufferLifeHolder> &holder)
{
    Mutex::Autolock lock(mBufferLifeHolderMutex);
    sp<BufferLifeHolder> temp = mBufferLifeHolder;
    mBufferLifeHolder = holder;
    return temp;
}

MVOID P2Procedure::flushBufferLifeHolder()
{
    Mutex::Autolock lock(mBufferLifeHolderMutex);
    mBufferLifeHolder = NULL;
}

MVOID P2Procedure::prepareBufferLifeHolder(FeaturePipeParam &param, const sp<Request> &request, const sp<BufferLifeHolder> &holder, const sp<BufferHandle> &rsso)
{
    if( rsso != NULL )
    {
        if( OK != rsso->waitState(BufferHandle::STATE_READABLE) )
        {
            MY_LOGW("sensor(%d) frame(%d) rsso handle not null but waitState failed!", mOpenId, request->frameNo);
        }
        else if( holder != NULL )
        {
            IImageBuffer* prsso = rsso->getBuffer();
            if (prsso != NULL)
                prsso->setTimestamp(request->timestamp);

            holder->borrowRSSO(rsso);
        }
    }

    sp<BufferLifeHolder> prev = updateBufferLifeHolder(holder);
    param.setVar<sp<BufferLifeHolder> >(VAR_PREV_HOLDER, prev);
    param.setVar<sp<BufferLifeHolder> >(VAR_CURR_HOLDER, holder);
    if( prev != NULL )
    {
        param.setVar<IImageBuffer*>(VAR_PREV_RSSO, prev->getRSSOBuffer() );
    }
    if( holder != NULL )
    {
        param.setVar<IImageBuffer*>(VAR_CURR_RSSO, holder->getRSSOBuffer() );
    }
}

MVOID P2Procedure::partialRelease(FeaturePipeParam &param)
{
    P2Procedure* p2Procedure = getP2Procedure(param);
    sp<Request> p2Request = getP2Request(param);
    MINT32 p2FrameNo = getP2RequestNo(param);

    if( p2Procedure == NULL || p2Request == NULL )
    {
        MY_LOGE("Cannot retrieve p2Procedure(%p)/p2Request(%p)", p2Procedure, p2Request.get());
        return;
    }

    for( size_t i = 0, size = param.mQParams.mvFrameParams[0].mvIn.size(); i < size ; ++i )
    {
        param.mQParams.mvFrameParams.editItemAt(0).mvIn.editItemAt(i).mBuffer = NULL;
    }
    for( size_t i = 0, size = param.mQParams.mvFrameParams[0].mvOut.size(); i < size; ++i )
    {
        if( param.mQParams.mvFrameParams[0].mvOut[i].mPortID.capbility == EPortCapbility_Disp )
        {
            param.mQParams.mvFrameParams.editItemAt(0).mvOut.editItemAt(i).mBuffer = NULL;
        }
    }

    // release p1 buffer
    if( p2Request->context.in_buffer != NULL )
    {
        p2Request->context.in_buffer->earlyRelease();
        p2Request->context.in_buffer = NULL;
    }
    if( p2Request->context.in_lcso_buffer != NULL )
    {
        p2Request->context.in_lcso_buffer->earlyRelease();
        p2Request->context.in_lcso_buffer = NULL;
    }
    if( p2Request->context.in_rsso_buffer != NULL )
    {
        p2Request->context.in_rsso_buffer->earlyRelease();
        p2Request->context.in_rsso_buffer = NULL;
    }
    // release display buffer
    vector<sp<BufferHandle> >::iterator it = p2Request->context.out_buffers.begin();
    vector<sp<BufferHandle> >::iterator end = p2Request->context.out_buffers.end();
    MUINT32 displayCount = 0;
    for( ; it != end; ++it )
    {
        sp<BufferHandle> handle = *it;
        if( handle != NULL )
        {
            MUINT32 usage = handle->getUsage();
            if( usage & GRALLOC_USAGE_HW_COMPOSER )
            {
                ++displayCount;
                if (param.mQParams.mDequeSuccess)
                {
                    handle->updateState(BufferHandle::STATE_WRITE_OK);
                }
                else
                {
                    handle->updateState(BufferHandle::STATE_WRITE_FAIL);
                }
                if ( p2Request->context.in_mdp_buffer != NULL)
                {
                    preparePreviewCallback(param);
                }
                handle->earlyRelease();
                (*it) = NULL;
            }
        }
    }
    if( displayCount != 1 )
    {
        MY_LOGD_IF(DEBUG_LOG, "Found unexpected display count = %d", displayCount);
    }
    p2Request->onPartialRequestDone();
}

MVOID P2Procedure::returnRSSO(FeaturePipeParam &param, MBOOL needNotify)
{
    CAM_TRACE_NAME("P2F:returnRSSO");

    sp<BufferLifeHolder> prev, curr;
    prev = param.getVar<sp<BufferLifeHolder> >(VAR_PREV_HOLDER, NULL);
    curr = param.getVar<sp<BufferLifeHolder> >(VAR_CURR_HOLDER, NULL);

    if( prev != NULL )
    {
        if( prev->returnRSSO(BufferLifeHolder::SECOND) )
        {
            // always notify previous frame change
            prev->notifyFrame();
        }
        prev->tryReleaseFrame();
        param.setVar<IImageBuffer*>(VAR_PREV_RSSO, NULL);
        param.setVar<sp<BufferLifeHolder> >(VAR_PREV_HOLDER, NULL);
    }
    if( curr != NULL )
    {
        if( curr->returnRSSO(BufferLifeHolder::FIRST) )
        {
            if( needNotify )
            {
                curr->notifyFrame();
            }
        }
        curr->tryReleaseFrame();
        param.setVar<IImageBuffer*>(VAR_CURR_RSSO, NULL);
        param.setVar<sp<BufferLifeHolder> >(VAR_CURR_HOLDER, NULL);
    }
}

#define NOTIFY_CHANGE MTRUE
#define NO_NOTIFY_CHANGE MFALSE

MINT32 P2Procedure::getP2ID(const FeaturePipeParam &param)
{
    P2Procedure* proc = getP2Procedure(param);
    return (proc == NULL) ? -1 : proc->mOpenId;
}

MBOOL P2Procedure::preparePreviewCallback(FeaturePipeParam &param)
{
    MY_LOGD("+");

    P2Procedure* p2Procedure = getP2Procedure(param);
    sp<Request> pRequest = getP2Request(param);
    MINT32 p2FrameNo = getP2RequestNo(param);

    if( pRequest != NULL )
    {
        MY_LOGD("%d[%d]", pRequest->frameNo, pRequest->index);

        sp<BufferHandle> inHandle = pRequest->context.in_mdp_buffer;
        pRequest->context.in_mdp_buffer.clear();

        sp<BufferHandle> outHandle;
        vector<sp<BufferHandle> >::iterator iter = pRequest->context.out_buffers.begin();
        while(iter !=  pRequest->context.out_buffers.end())
        {
            sp<BufferHandle> pOutBuffer = *iter;
            if (pOutBuffer.get() && pOutBuffer->getState() == BufferHandle::STATE_NOT_USED)
            {
                outHandle = pOutBuffer;
                (*iter).clear();
                break;
            }
            iter++;
        }

        MUINT32 transform = 0;
        MRect crop;
        IImageBuffer* pSrc = NULL;
        IImageBuffer* pDst = NULL;

        MERROR ret = OK;
        if( inHandle == NULL )
        {
            MY_LOGW("no src");
            ret = NOT_ENOUGH_DATA;
        }
        else if( OK != (ret = inHandle->waitState(BufferHandle::STATE_READABLE)) )
        {
            MY_LOGW("src buffer err = %d", ret);
        }
        else if( NULL == (pSrc = inHandle->getBuffer()) )
        {
            MY_LOGW("src buffer is NULL");
            ret = NOT_ENOUGH_DATA;
        }
        else if( outHandle == NULL )
        {
            MY_LOGW("no dst");
            ret = NOT_ENOUGH_DATA;
        }
        else if( OK != (ret = outHandle->waitState(BufferHandle::STATE_WRITABLE)) )
        {
            MY_LOGW("dst buffer err = %d", ret);
        }
        else if( NULL == (pDst = outHandle->getBuffer()) )
        {
            MY_LOGW("dst buffer is NULL");
            ret = NOT_ENOUGH_DATA;
        }

        if( ret != OK )
        {
            MY_LOGD("-");
            return MFALSE;
        }

        if( inHandle->getTransform() == eTransform_ROT_90 )
        {
            transform = eTransform_ROT_270;
        }
        else if( inHandle->getTransform() == eTransform_ROT_180 )
        {
            transform = eTransform_ROT_180;
        }
        else if( inHandle->getTransform() == eTransform_ROT_270 )
        {
            transform = eTransform_ROT_90;
        }
        else
        {
            MY_LOGW("not support transform");
        }

        MBOOL success = MFALSE;
        {
            NSSImager::IImageTransform* pTrans = NSSImager::IImageTransform::createInstance();
            if( !pTrans ) {
                MY_LOGE("!pTrans");
                MY_LOGD("-");
                return MFALSE;
            }

            success =
                pTrans->execute(
                    pSrc,
                    pDst,
                    NULL,
                    crop,
                    NULL,
                    transform,
                    0,
                    0xFFFFFFFF
                    );

            MY_LOGD("mdp execute - success=%d", success);

            pTrans->destroyInstance();
            pTrans = NULL;
        }


        MBOOL debugDump = param.getVar<MBOOL>(VAR_DEBUG_DUMP, MFALSE);
        if( debugDump && p2Procedure->mDebugControl.needDumpOut(DebugControl::DUMP_OUT_PREVIEWCB) )
        {
            dumpBuffer(p2FrameNo, pDst, "preview_cb");
        }

        outHandle->updateState(success? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL );
        outHandle->earlyRelease();
    }

    MY_LOGD("-");
    return MTRUE;
}

MBOOL P2Procedure::featurePipeCB(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param)
{
    CAM_TRACE_FMT_BEGIN("P2F:featurePipeCB msg=%d", msg);

    MBOOL ret = MFALSE;

    featurePipeCBDebug(msg, param);

    if( msg == FeaturePipeParam::MSG_FRAME_DONE )
    {
        sp<Request> pRequest = getP2Request(param);
        if( pRequest != NULL )
        {
            CAM_TRACE_FMT_BEGIN("Release Out Meta");
            pRequest->context.out_app_meta = NULL;
            pRequest->context.out_hal_meta = NULL;
            CAM_TRACE_FMT_END();
        }

        returnRSSO(param, NO_NOTIFY_CHANGE);
        partialRelease(param);
        releaseP2ObjectReference(param);
        pRequest = NULL;
        param.mQParams.mpfnCallback(param.mQParams);
        ret = MTRUE;
    }
    else if( msg == FeaturePipeParam::MSG_DISPLAY_DONE )
    {
        partialRelease(param);
        ret = MTRUE;
    }
    else if( msg == FeaturePipeParam::MSG_RSSO_DONE)
    {
        returnRSSO(param, NOTIFY_CHANGE);
        ret = MTRUE;
    }
    else if( msg == FeaturePipeParam::MSG_FD_DONE )
    {
    }
    else if( msg == FeaturePipeParam::MSG_P2B_SET_3A )
    {
        P2Procedure *proc = getP2Procedure(param);
        if( proc )
        {
            ret = proc->setP2B3A(param);
        }
    }
    else if( msg == FeaturePipeParam::MSG_N3D_SET_SHOTMODE )
    {
        sp<Request> pRequest = getP2Request(param);
        if(pRequest != NULL && pRequest->context.out_app_meta != NULL)
        {
            IMetadata *pAppOutMeta = pRequest->context.out_app_meta->getMetadata();
            MINT8 shotMode = param.getVar<MUINT8>(VAR_N3D_SHOTMODE, -1);
            trySetMetadata<MUINT8>(pAppOutMeta, MTK_STEREO_FEATURE_SHOT_MODE, (MUINT8)shotMode);
        }
    }

    CAM_TRACE_FMT_END();
    return ret;
}

MBOOL P2Procedure::featurePipeCBDebug(FeaturePipeParam::MSG_TYPE msg, FeaturePipeParam &param)
{
    CAM_TRACE_CALL();

    P2Procedure* p2Procedure = getP2Procedure(param);
    sp<Request> p2Request = getP2Request(param);
    MINT32 p2FrameNo = getP2RequestNo(param);

    if( p2Procedure == NULL || p2Request == NULL )
    {
        MY_LOGE("Cannot retrieve p2Procedure(%p)/p2Request(%p)", p2Procedure, p2Request.get());
        return MFALSE;
    }

    if( msg != FeaturePipeParam::MSG_FRAME_DONE &&  msg != FeaturePipeParam::MSG_DISPLAY_DONE )
    {
        return MTRUE;
    }

    MBOOL debugDump = param.getVar<MBOOL>(VAR_DEBUG_DUMP, MFALSE);

    char fileName[256];
    TuningUtils::FILE_DUMP_NAMING_HINT &hint = p2Request->context.hint;

    for( size_t i = 0, size = param.mQParams.mvFrameParams[0].mvOut.size(); i < size; ++i )
    {
        const Output &output = param.mQParams.mvFrameParams[0].mvOut[i];
        if( output.mBuffer != NULL )
        {
            MBOOL isDisplay = output.mPortID.capbility == EPortCapbility_Disp ? MTRUE : MFALSE;
            MBOOL isRecord = output.mPortID.capbility == EPortCapbility_Rcrd ? MTRUE : MFALSE;
            MBOOL isFD = output.mPortID.index == PORT_IMG2O.index ? MTRUE : MFALSE;
            if(isFD)
            {
                isRecord = MFALSE;
            }
            MBOOL isOther = (!isDisplay && !isRecord && !isFD);

            if( msg == FeaturePipeParam::MSG_DISPLAY_DONE )
            {
                if( !isDisplay )
                    continue;
            }
            else if( msg == FeaturePipeParam::MSG_FRAME_DONE)
            {
            }

            p2Procedure->drawID(output, p2Request->context.magic3A);
            p2Procedure->drawScanLine(output);

            DebugControl &debugControl = p2Procedure->mDebugControl;

            if( debugDump )
            {
                MY_LOGD("dump output (%zu/%zu) M:%d (D:%d R:%d F:%d O:%d)", i, size, p2Request->context.magic3A, isDisplay, isRecord, isFD, isOther);

                if( p2Procedure->mDebugControl.isNddMode() )
                {
                    if( (isDisplay && debugControl.needDumpOut(DebugControl::DUMP_OUT_DISPLAY)) ||
                        (isRecord  && debugControl.needDumpOut(DebugControl::DUMP_OUT_RECORD))
                      )
                    {
                        dumpNddBuffer(&hint, output.mBuffer, output.mPortID.index);
                    }
                }
                else
                {
                    if( isDisplay && debugControl.needDumpOut(DebugControl::DUMP_OUT_DISPLAY) )
                    {
                        dumpBuffer(p2FrameNo, output.mBuffer, "display");
                    }
                    else if( isRecord && debugControl.needDumpOut(DebugControl::DUMP_OUT_RECORD) )
                    {
                        dumpBuffer(p2FrameNo, output.mBuffer, "record");
                    }
                    else if( isFD && debugControl.needDumpOut(DebugControl::DUMP_OUT_FD) )
                    {
                        dumpBuffer(p2FrameNo, output.mBuffer, "fd");
                    }
                    else if( isOther && debugControl.needDumpOut(DebugControl::DUMP_OUT_PREVIEWCB) )
                    {
                        dumpBuffer(p2FrameNo, output.mBuffer, "preview_cb");
                    }
                }
            }
        }
    }

    return MTRUE;
}

MVOID P2Procedure::dumpNddBuffer(TuningUtils::FILE_DUMP_NAMING_HINT* hint, IImageBuffer *buffer, MUINT32 portIndex)
{
    if( hint && buffer )
    {
        char fileName[256] = {0};
        extract(hint, buffer);

        if( portIndex == NSImageio::NSIspio::EPortIndex_IMGO )
        {
            genFileName_RAW(fileName, sizeof(fileName), hint, TuningUtils::RAW_PORT_IMGO);
        }
        else if( portIndex == NSImageio::NSIspio::EPortIndex_RRZO )
        {
            genFileName_RAW(fileName, sizeof(fileName), hint, TuningUtils::RAW_PORT_RRZO);
        }
        else if( portIndex == NSImageio::NSIspio::EPortIndex_LCSO )
        {
            genFileName_LCSO(fileName, sizeof(fileName), hint);
        }
        else if( portIndex == NSImageio::NSIspio::EPortIndex_WDMAO )
        {
            genFileName_YUV(fileName, sizeof(fileName), hint, NSCam::TuningUtils::YUV_PORT_WDMAO);
        }
        else if( portIndex == NSImageio::NSIspio::EPortIndex_WROTO )
        {
            genFileName_YUV(fileName, sizeof(fileName), hint, NSCam::TuningUtils::YUV_PORT_WROTO);
        }

        MY_LOGD("dump to: %s", fileName);
        buffer->saveToFile(fileName);
    }
}

MVOID P2Procedure::dumpBuffer(MINT32 frameNo, IImageBuffer *buffer, const char *fmt, ...)
{
    if( buffer )
    {
        MUINT32 stride, pbpp, ibpp, width, height, size;
        stride = buffer->getBufStridesInBytes(0);
        pbpp = buffer->getPlaneBitsPerPixel(0);
        ibpp = buffer->getImgBitsPerPixel();
        size = buffer->getBufSizeInBytes(0);
        pbpp = pbpp ? pbpp : 8;
        width = stride * 8 / pbpp;
        width = width ? width : 1;
        ibpp = ibpp ? ibpp : 8;
        height = size / width;
        if( buffer->getPlaneCount() == 1 )
        {
          height = height * 8 / ibpp;
        }

        va_list ap;
        va_start(ap, fmt);

        char name[256] = {0};
        char path[256] = {0};
        if( 0 >= vsnprintf(name, sizeof(name), fmt, ap) )
        {
            strncpy(name, "NA", sizeof(name));
            name[sizeof(name)-1] = 0;
        }
        va_end(ap);

        snprintf(path, sizeof(path), "/sdcard/dump/%04d_%s_%dx%d.bin", frameNo, name, width, height);

        MY_LOGD("dump to %s", path);
        buffer->saveToFile(path);
    }
}

MVOID P2Procedure::drawScanLine(const Output &output)
{
    if( mDebugScanLineMask != 0 &&
        mpDebugScanLine != NULL &&
        output.mBuffer != NULL )
    {
        if( (output.mPortID.index == PORT_WDMAO.index &&
             mDebugScanLineMask & DRAWLINE_PORT_WDMAO) ||
            (output.mPortID.index == PORT_WROTO.index &&
             mDebugScanLineMask & DRAWLINE_PORT_WROTO) ||
            (output.mPortID.index == PORT_IMG2O.index &&
             mDebugScanLineMask & DRAWLINE_PORT_IMG2O) )
        {
            mpDebugScanLine->drawScanLine(
                                    output.mBuffer->getImgSize().w,
                                    output.mBuffer->getImgSize().h,
                                    (void*)(output.mBuffer->getBufVA(0)),
                                    output.mBuffer->getBufSizeInBytes(0),
                                    output.mBuffer->getBufStridesInBytes(0));
        }
    }
}

MVOID P2Procedure::drawRect(const IImageBuffer *buffer, const Output *output, const MCropRect &mCropRect, IMetadata *pMeta_InHal)
{
    if( mDebugDrawRectMask != 0)
    {
        if( (output->mPortID.index == PORT_WDMAO.index &&
             mDebugDrawRectMask & DRAWLINE_PORT_WDMAO) ||
            (output->mPortID.index == PORT_WROTO.index &&
             mDebugDrawRectMask & DRAWLINE_PORT_WROTO) ||
            (output->mPortID.index == PORT_IMG2O.index &&
             mDebugDrawRectMask & DRAWLINE_PORT_IMG2O)  )
        {
            //buffer->syncCache(eCACHECTRL_INVALID);
            MINT32 i = 0;
            MINT32 margin = ::property_get_int32("vendor.debug.camera.drawcrop.margin", 20);
            MVOID* virtAddr = (MVOID*)(buffer->getBufVA(0));
            MINT32 value = ::property_get_int32("vendor.debug.camera.drawcrop.value", 255);
            MINT32 fsc_offset = ::property_get_int32("vendor.debug.camera.drawcrop.fsc_offset", 0);
            MINT32 imgStride = buffer->getBufStridesInBytes(0);
            MUINT32 pbpp = buffer->getPlaneBitsPerPixel(0);//bayer8 = 15 bits
            MCropRect dupCropRect = mCropRect;
            if ( fsc_offset )
            {
                MUINT32 fscMode = mInitParams.usageHint.mFSCMode;
                if( EFSC_FSC_ENABLED(fscMode) )
                {
                    MFLOAT fscScaleRatio = 1.0f;
                    IMetadata::IEntry cropEntry = pMeta_InHal->entryFor(MTK_FSC_CROP_DATA);
                    if (cropEntry.count())
                    {
                        IMetadata::Memory metaMemory = cropEntry.itemAt(0, Type2Type< IMetadata::Memory >());
                        FSC_CROPPING_DATA_STRUCT *cropData = (FSC_CROPPING_DATA_STRUCT*)metaMemory.array();
                        fscScaleRatio = cropData->image_scale;
                    }
                    dupCropRect.p_integral.x -= dupCropRect.s.w*(1.0f-fscScaleRatio)/2/fscScaleRatio;
                    dupCropRect.p_integral.y -= dupCropRect.s.h*(1.0f-fscScaleRatio)/2/fscScaleRatio;
                    dupCropRect.p_integral.x = max(dupCropRect.p_integral.x, 0);
                    dupCropRect.p_integral.y = max(dupCropRect.p_integral.y, 0);
                    MINT32 frameNum = 0;
                    tryGetMetadata<MINT32>(pMeta_InHal, MTK_PIPELINE_FRAME_NUMBER, frameNum);
                    MY_LOGD("f(%d) xy offset (%d,%d,%d,%d)->(%d,%d,%d,%d) fscScaleRatio(%f)",
                        frameNum,
                        mCropRect.p_integral.x, mCropRect.p_integral.y, mCropRect.s.w, mCropRect.s.h,
                        dupCropRect.p_integral.x, dupCropRect.p_integral.y, dupCropRect.s.w, dupCropRect.s.h,
                        fscScaleRatio);
                }
            }

            for(i=dupCropRect.p_integral.y; i<dupCropRect.p_integral.y+margin; i++)
            {
                memset((void*)((char*)virtAddr+imgStride*i),
                               value,
                               imgStride);
            }

            for(i=dupCropRect.p_integral.y+dupCropRect.s.h-1; i>=dupCropRect.p_integral.y+dupCropRect.s.h-margin; i--)
            {
                memset((void*)((char*)virtAddr+imgStride*i),
                               value,
                               imgStride);
            }

            for(i=0; i<buffer->getImgSize().h; i++)
            {
                memset((void*)((char*)virtAddr+dupCropRect.p_integral.x*pbpp/8+imgStride*i),
                               value,
                               margin*pbpp/8);
            }

            for(i=0; i<buffer->getImgSize().h; i++)
            {
                memset((void*)((char*)virtAddr+(dupCropRect.p_integral.x+dupCropRect.s.w-1-margin)*pbpp/8+imgStride*i),
                               value,
                               margin*pbpp/8);
            }
            //buffer->syncCache(eCACHECTRL_FLUSH);
        }

    }
}

MVOID P2Procedure::drawID(const Output &output, unsigned id)
{
    if( mpDebugDrawID != NULL &&
        mpDebugDrawID->needDraw() &&
        output.mBuffer != NULL )
    {
        if( output.mPortID.index == PORT_WDMAO.index ||
            output.mPortID.index == PORT_WROTO.index )
        {
            IImageBuffer *buffer = output.mBuffer;
            mpDebugDrawID->draw(id,
                                (char*)buffer->getBufVA(0),
                                buffer->getImgSize().w,
                                buffer->getImgSize().h,
                                buffer->getBufStridesInBytes(0),
                                buffer->getBufSizeInBytes(0));
        }
    }
}
#endif // FEATURE_MODIFY

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
onExecute(
    sp<Request> const pRequest,
    FrameParams const& params
)
{
    CAM_TRACE_NAME("P2F:[Proc]exe");
    pRequest->context.timer.startOnExecute();
    //
    MERROR ret = OK;
    //
    if ( OK != (ret = checkParams(params, pRequest->skipOutCheck)) )
    #ifdef FEATURE_MODIFY
    {
        pRequest->context.in_buffer.clear();
        pRequest->context.in_lcso_buffer.clear();
        pRequest->context.in_rsso_buffer.clear();
        vector<sp<BufferHandle> >::iterator it, end;
        it = pRequest->context.out_buffers.begin();
        end = pRequest->context.out_buffers.end();
        for( ; it != end; ++it )
        {
            (*it).clear();
        }
        return ret;
    }
    #endif // FEATURE_MODIFY
    CAM_TRACE_BEGIN("P2F:prepareMeta");
    // prepare metadata
    IMetadata* pMeta_InApp  = params.inApp->getMetadata();
    IMetadata* pMeta_InHal  = params.inHal->getMetadata();
    IMetadata* pMeta_OutApp = params.outApp.get() ? params.outApp->getMetadata() : NULL;
    IMetadata* pMeta_OutHal = params.outHal.get() ? params.outHal->getMetadata() : NULL;
    //
    if( pMeta_InApp == NULL || pMeta_InHal == NULL ) {
        MY_LOGE("sensor(%d) meta: in app %p, in hal %p", mOpenId, pMeta_InApp, pMeta_InHal);
        return BAD_VALUE;
    }
    //
    sp<Cropper::crop_info_t> pCropInfo = new Cropper::crop_info_t;
    if( OK != (ret = getCropInfos(pMeta_InApp, pMeta_InHal, params.bResized, *pCropInfo)) ) {
        MY_LOGE("sensor(%d) getCropInfos failed", mOpenId);
        return ret;
    }
    pRequest->context.crop_info = pCropInfo;

    MINT32 czTimeStamp = 0;
    if(pMeta_InHal)
    {
        tryGetMetadata<MINT32>(pMeta_InHal, MTK_PIPELINE_UNIQUE_KEY, czTimeStamp);
    }
    CAM_TRACE_END();

    String8 inDebugString, outDebugString;

    //
    QParams enqueParams;
    prepareFrameParam(enqueParams, pRequest, czTimeStamp);
    CAM_TRACE_BEGIN("P2F:prepareInput");
    //
    // input
    {
        if( OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW("sensor(%d) src buffer err = %d", mOpenId, ret);
            return BAD_VALUE;
        }
        IImageBuffer* pSrc = params.in.mHandle->getBuffer();
        //
        Input src;
        src.mPortID       = params.in.mPortId;
        src.mPortID.group = 0;
        src.mBuffer       = pSrc;
        src.mSecureTag    = mSecureEnum;
        // update src size
        if( params.bResized )
            pSrc->setExtParam(pCropInfo->dstsize_resizer);
        //
        enqueParams.mvFrameParams.editItemAt(0).mvIn.push_back(src);

        appendHandleDebugString(params.in.mHandle, params.bResized? "(RRZ)" : "(IMG)", inDebugString);
        MY_LOGD_IF(mbEnableLog, "sensor(%d) P2FeatureNode EnQ Src mPortID.index(%d) Fmt(0x%x) "
            "Size(%dx%d)", mOpenId, src.mPortID.index, src.mBuffer->getImgFormat(),
            src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h);
    }
    // input LCEI
    if(params.in_lcso.mHandle != NULL){
        if( OK != (ret = params.in_lcso.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW(" (%d) Lcso handle not null but waitState failed! ", pRequest->frameNo);
            return BAD_VALUE;
        }
        //
        appendHandleDebugString(params.in_lcso.mHandle, "(LCE)", inDebugString);
    }

    MBOOL debugDump = mDebugControl.needDumpFrame(pRequest->frameNo);
    if( debugDump )
    {
        if( mDebugControl.isNddMode() )
        {
            TuningUtils::FILE_DUMP_NAMING_HINT &hint = pRequest->context.hint;

            char fileName[256];
            if( params.in.mHandle != NULL && mDebugControl.needDumpIn(params.bResized ? DebugControl::DUMP_IN_RRZO : DebugControl::DUMP_IN_IMGO) )
            {
                dumpNddBuffer(&hint, params.in.mHandle->getBuffer(), params.bResized ? NSImageio::NSIspio::EPortIndex_RRZO : NSImageio::NSIspio::EPortIndex_IMGO);
            }

            if( params.in_lcso.mHandle != NULL && mDebugControl.needDumpIn(DebugControl::DUMP_IN_LCSO) )
            {
                dumpNddBuffer(&hint, params.in_lcso.mHandle->getBuffer(), NSImageio::NSIspio::EPortIndex_LCSO);
            }
        }
        else
        {
            if( params.in.mHandle != NULL && mDebugControl.needDumpIn(params.bResized ? DebugControl::DUMP_IN_RRZO : DebugControl::DUMP_IN_IMGO) )
            {
                dumpBuffer(pRequest->frameNo, params.in.mHandle->getBuffer(), params.bResized ? "rrzo" : "imgo");
            }
        }
    }
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("P2F:prepareOutput");
    //
    // Output
    for( size_t i = 0; i < params.vOut.size(); i++ )
    {
        if( params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE)) ) {
            MY_LOGW("sensor(%d) dst buffer err = %d", mOpenId, ret);
            continue;
        }
        IImageBuffer* pDst = params.vOut[i].mHandle->getBuffer();
        //
        Output dst;
        dst.mPortID       = params.vOut[i].mPortId;
        dst.mPortID.group = 0;
        MUINT32 const uUsage = params.vOut[i].mUsage;
        dst.mPortID.capbility   = (NSIoPipe::EPortCapbility)(
            (uUsage & GRALLOC_USAGE_HW_COMPOSER) ? EPortCapbility_Disp :
            (uUsage & GRALLOC_USAGE_HW_VIDEO_ENCODER) ? EPortCapbility_Rcrd :
            EPortCapbility_None);
        dst.mBuffer       = pDst;
        dst.mTransform    = params.vOut[i].mTransform;
        dst.mSecureTag    = mSecureEnum;
        //
        enqueParams.mvFrameParams.editItemAt(0).mvOut.push_back(dst);

        const char* tag = dst.mPortID.capbility == EPortCapbility_Disp ? "(Disp)" :
                          dst.mPortID.index == PORT_IMG2O.index ? "(Fd)" :
                          dst.mPortID.capbility == EPortCapbility_Rcrd ? "(Rcrd)" : "(Other)";
        appendHandleDebugString(params.vOut[i].mHandle, tag, outDebugString);
    }

    if( enqueParams.mvFrameParams[0].mvOut.size() == 0  && !pRequest->skipOutCheck) {
        //MY_LOGW("sensor(%d) no dst buffer", mOpenId);
        return BAD_VALUE;
    }
    CAM_TRACE_END();

#ifdef FEATURE_MODIFY
    FeaturePipeParam featureEnqueParams(featurePipeCB);
    prepareFeaturePipeParam(featureEnqueParams, enqueParams, pRequest,
                            pMeta_InApp, pMeta_InHal,
                            pMeta_OutApp, pMeta_OutHal,
                            *pCropInfo);
    featureEnqueParams.setQParams(enqueParams);
    sp<BufferLifeHolder> bufferLifeHolder;
    if( params.in_rsso.mHandle != NULL )
    {
        if( OK != (ret = params.in_rsso.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW(" (%d) Rsso handle not null but waitState failed! ", pRequest->frameNo);
            return BAD_VALUE;
        }else {
            MSize rssoSize;
            if( ! tryGetMetadata<MSize>(pMeta_InHal, MTK_P1NODE_RSS_SIZE, rssoSize) ) {
               MY_LOGE("sensor(%d) cannot get MTK_P1NODE_RSS_SIZE", mOpenId);
               return BAD_VALUE;
            }
            MY_LOGD_IF(mbEnableLog, "Rsso(%dx%d)", rssoSize.w, rssoSize.h);

            IImageBuffer* pSrc = params.in_rsso.mHandle->getBuffer();
            pSrc->setExtParam(rssoSize);
        }
        bufferLifeHolder = new BufferLifeHolder(pRequest->getStreamControl(), pRequest->getPipelineFrame());
        appendHandleDebugString(params.in_rsso.mHandle, "RSS", inDebugString);
    }
    prepareBufferLifeHolder(featureEnqueParams, pRequest, bufferLifeHolder, params.in_rsso.mHandle);
#endif // FEATURE_MODIFY

    // for output group crop
    {
        FSC_CROPPING_RESULT_STRUCT fsc_cropping_result;

        Vector<Output>::const_iterator iter = enqueParams.mvFrameParams[0].mvOut.begin();
        while( iter != enqueParams.mvFrameParams[0].mvOut.end() ) {
            MCrpRsInfo crop;
            MUINT32 const uPortIndex = iter->mPortID.index;
            if( uPortIndex == PORT_WDMAO.index ) {
                crop.mGroupID = 2;
                if (pCropInfo->isFSCEnabled)
                {
                    MRectF resultf;
                    Cropper::calcViewAngleF(mbEnableLog || EFSC_DEBUG_ENABLED(mInitParams.usageHint.mFSCMode), EFSC_SUBPIXEL_ENABLED(mInitParams.usageHint.mFSCMode),
                        *pCropInfo, iter->mBuffer->getImgSize(), crop.mCropRect, resultf);
                    fsc_cropping_result.croppingGroup[0].groupID = crop.mGroupID;
                    fsc_cropping_result.croppingGroup[0].resultF = resultf;
                }
                else
                {
                    Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop.mCropRect);
                }
            } else if ( uPortIndex == PORT_WROTO.index ) {
                crop.mGroupID = 3;
                IImageBuffer* pBuf      = iter->mBuffer;
                MINT32 const transform  = iter->mTransform;
                MSize dstSize = ( transform & eTransform_ROT_90 )
                                ? MSize(pBuf->getImgSize().h, pBuf->getImgSize().w)
                                : pBuf->getImgSize();
                if (pCropInfo->isFSCEnabled)
                {
                    MRectF resultf;
                    Cropper::calcViewAngleF(mbEnableLog || EFSC_DEBUG_ENABLED(mInitParams.usageHint.mFSCMode), EFSC_SUBPIXEL_ENABLED(mInitParams.usageHint.mFSCMode),
                        *pCropInfo, dstSize, crop.mCropRect, resultf);
                    fsc_cropping_result.croppingGroup[1].groupID = crop.mGroupID;
                    fsc_cropping_result.croppingGroup[1].resultF = resultf;
                }
                else
                {
                    Cropper::calcViewAngle(mbEnableLog, *pCropInfo, dstSize, crop.mCropRect);
                }
#if FD_PORT_SUPPORT
            } else if ( uPortIndex == PORT_IMG2O.index ) {
                crop.mGroupID = 1;
                Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop.mCropRect);
#endif
            } else {
                MY_LOGE("sensor(%d) not supported output port index %d", mOpenId, iter->mPortID.index);
                return BAD_VALUE;
            }
            drawRect(pRequest->context.in_buffer->getBuffer(), iter, crop.mCropRect, pMeta_InHal);

            crop.mResizeDst = iter->mBuffer->getImgSize();
            MY_LOGD_IF(mbEnableLog, "sensor(%d) P2FeatureNode EnQ out 00 Crop G(%d) S(%d,%d)(%d,%d)(%dx%d) D(%dx%d)",
                mOpenId,
                crop.mGroupID,
                crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                crop.mCropRect.p_fractional.x, crop.mCropRect.p_fractional.y,
                crop.mCropRect.s.w, crop.mCropRect.s.h,
                crop.mResizeDst.w, crop.mResizeDst.h
                );
            enqueParams.mvFrameParams.editItemAt(0).mvCropRsInfo.push_back(crop);
            iter++;
        }

        if (pCropInfo->isFSCEnabled)
        {
            featureEnqueParams.setVar<FSC_CROPPING_RESULT_STRUCT >(VAR_FSC_CROP_RESULT, fsc_cropping_result);
        }
    }
    if (pRequest->context.burst_num > 1)
    {
        if(mpMultiFrameHandler)
            return mpMultiFrameHandler->collect(pRequest, featureEnqueParams);
        else
            MY_LOGW_IF(mbEnableLog, "sensor(%d) no burst handler", mOpenId);
    }

    CAM_TRACE_BEGIN("P2F:Tuning");
    {
        TuningParam rTuningParam;
        void* pTuning = NULL;
        unsigned int tuningsize = mpPipe->getRegTableSize();
        pTuning = mDebugControl.needMemCheck() ? PostRedZone::mynew(tuningsize) : malloc(tuningsize);
        if (pTuning == NULL) {
            MY_LOGE("sensor(%d) alloc tuning buffer fail", mOpenId);
            return NO_MEMORY;
        }
        rTuningParam.pRegBuf = pTuning;
        if(params.in_lcso.mHandle != NULL) {
            rTuningParam.pLcsBuf = params.in_lcso.mHandle->getBuffer();
        }
        MY_LOGD_IF(mbEnableLog, "sensor(%d) pass2 setIsp malloc %p : %d, LCSO exist(%d)", mOpenId, pTuning, tuningsize, (rTuningParam.pLcsBuf != NULL));
        //
        MetaSet_T inMetaSet;
        MetaSet_T outMetaSet;
        //
        CAM_TRACE_BEGIN("P2F:Meta clone");
        inMetaSet.appMeta = *pMeta_InApp;
        inMetaSet.halMeta = *pMeta_InHal;
        CAM_TRACE_END();

        CAM_TRACE_BEGIN("P2F:Meta set");
        //
        MBOOL const bGetResult = (pMeta_OutApp || pMeta_OutHal);
        //
        if( params.bResized ) {
            trySetMetadata<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
        } else {
            trySetMetadata<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
        }
        CAM_TRACE_END();
        if( pMeta_OutHal ) {
            // FIX ME: getDebugInfo() @ setIsp() should be modified
            //outMetaSet.halMeta = *pMeta_InHal;
        }

        //
        if (mp3A ) {

#ifdef FEATURE_MODIFY
            //if (mInitParams.usageHint.m3DNRMode & NSCam::NR3D::E3DNR_MODE_MASK_SL2E_EN)
            {
                prepare3DNR_SL2E_Info(&(inMetaSet.appMeta), &(inMetaSet.halMeta), enqueParams,
                    *pCropInfo, featureEnqueParams, pRequest);
            }
#endif // FEATURE_MODIFY

            MY_LOGD_IF(mbEnableLog, "sensor(%d) P2 setIsp %p : %d", mOpenId, pTuning, tuningsize);
            CAM_TRACE_BEGIN("P2F:setIsp");
            pRequest->context.timer.startSetIsp();
            if (0 > mp3A->setIsp(0, inMetaSet, &rTuningParam,
                                    (bGetResult ? &outMetaSet : NULL))) {
                CAM_TRACE_END();
                pRequest->context.timer.stopSetIsp();
                MY_LOGW("sensor(%d) P2 setIsp - skip tuning pushing", mOpenId);
                if (pTuning != NULL) {
                    MY_LOGD_IF(mbEnableLog, "sensor(%d) P2 setIsp free %p : %d", mOpenId, pTuning, tuningsize);
                    //
                    mDebugControl.needMemCheck() ? PostRedZone::mydelete(pTuning) : ::free(pTuning);
                }
            } else {
                CAM_TRACE_END();
                pRequest->context.timer.stopSetIsp();
                //
                map<EDIPInfoEnum, MUINT32> mDipInfo;
                mDipInfo[EDIPINFO_DIPVERSION] = EDIPHWVersion_40;
                MBOOL r = NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(mDipInfo);
                if (!r) {
                    MY_LOGE("queryDIPInfo fail!");
                }

                // check HLR open or not
                if(rTuningParam.pLcsBuf) {
                    IImageBuffer* pSrc = params.in_lcso.mHandle->getBuffer();
                    //
                    Input src;
                    src.mPortID       = params.in_lcso.mPortId;
                    src.mPortID.group = 0;
                    src.mBuffer       = pSrc;
                    if (mSecureEnum) {
                        MUINT32 sec_handle = mSecBufCtrl.registerAndGetSecHandle(mSecureEnum, src.mBuffer);
                        if (!sec_handle) return BAD_VALUE;
                        src.mSecHandle = sec_handle;
                        src.mSecureTag = mSecureEnum;
                    }
                    //
                    enqueParams.mvFrameParams.editItemAt(0).mvIn.push_back(src);
                    MY_LOGD_IF(mbEnableLog, "sensor(%d) P2FeatureNode EnQ Src mPortID.index(%d) Fmt(0x%x) "
                        "Size(%dx%d) sizeInBytes(%zu) bufferfd(%d)", mOpenId, src.mPortID.index, src.mBuffer->getImgFormat(),
                        src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h, src.mBuffer->getBufSizeInBytes(0), src.mBuffer->getFD(0));

                    if( mDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_50 )
                    {
                        src.mPortID  = PORT_DEPI;

                        enqueParams.mvFrameParams.editItemAt(0).mvIn.push_back(src);
                        MY_LOGD_IF(mbEnableLog, "sensor(%d) P2FeatureNode EnQ Src mPortID.index(%d) Fmt(0x%x) "
                            "Size(%dx%d) sizeInBytes(%zu) bufferId(%d)", mOpenId, src.mPortID.index, src.mBuffer->getImgFormat(),
                            src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h, src.mBuffer->getBufSizeInBytes(0), src.mBuffer->getFD(0));

                        //srz4 config
                        ModuleInfo srz4_module;
                        srz4_module.moduleTag = EDipModule_SRZ4;
                        srz4_module.frameGroup = 0;
                        LCENR_IN_PARAMS in;
                        LCENR_OUT_PARAMS out;

                        in.resized = params.bResized;
                        in.p2_in = params.in.mHandle->getBuffer()->getImgSize();
                        in.rrz_in = pCropInfo->bin_size;
                        in.rrz_crop_in = pCropInfo->crop_p1_bin;
                        in.rrz_out = pCropInfo->dstsize_resizer;
                        in.lce_full = src.mBuffer->getImgSize();

                        MY_LOGD_IF(mbEnableLog, "sensor(%d) LCENR in rrz_in(%dx%d) rrz_crop_in(%d,%d)(%dx%d) rrz_out(%dx%d)",
                            mOpenId, in.rrz_in.w, in.rrz_in.h, in.rrz_crop_in.p.x, in.rrz_crop_in.p.y, in.rrz_crop_in.s.w, in.rrz_crop_in.s.h,
                            in.rrz_out.w, in.rrz_out.h);

                        calculateLCENRConfig(in, out);

                        MY_LOGD_IF(mbEnableLog, "sensor(%d) LCENR out reszied(%d) in(%dx%d) crop(%lux%lu) crop_start=(%d.%u, %d.%u) out(%dx%d)",
                            mOpenId, params.bResized, out.srz4Param.in_w, out.srz4Param.in_h, out.srz4Param.crop_w, out.srz4Param.crop_h,
                            out.srz4Param.crop_x, out.srz4Param.crop_floatX, out.srz4Param.crop_y, out.srz4Param.crop_floatY,
                            out.srz4Param.out_w, out.srz4Param.out_h);

                        pRequest->context.srz4Param = out.srz4Param;
                        srz4_module.moduleStruct = reinterpret_cast<MVOID*> (&pRequest->context.srz4Param);
                        enqueParams.mvFrameParams.editItemAt(0).mvModuleData.push_back(srz4_module);
                    }
                }

                enqueParams.mvFrameParams.editItemAt(0).mTuningData = pTuning;

                IImageBuffer* pSrc = static_cast<IImageBuffer*>(rTuningParam.pLsc2Buf);
                if (pSrc != NULL) {
                    Input src;
                    src.mPortID       = mDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_50 ? PORT_IMGCI : PORT_DEPI;
                    src.mPortID.group = 0;
                    src.mBuffer       = pSrc;
                    if (mSecureEnum) {
                        MUINT32 sec_handle = mSecBufCtrl.registerAndGetSecHandle(mSecureEnum, src.mBuffer);
                        if (!sec_handle) return BAD_VALUE;
                        src.mSecHandle = sec_handle;
                        src.mSecureTag = mSecureEnum;
                    }
                    //
                    enqueParams.mvFrameParams.editItemAt(0).mvIn.push_back(src);
                    MY_LOGD_IF(mbEnableLog, "sensor(%d) P2Node EnQ Src mPortID.index(%d) Fmt(0x%x) "
                        "Size(%dx%d) sizeInBytes(%zu) bufferfd(%d)", mOpenId, src.mPortID.index, src.mBuffer->getImgFormat(),
                        src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h, src.mBuffer->getBufSizeInBytes(0), src.mBuffer->getFD(0));
                }

                IImageBuffer *pSrc2 = static_cast<IImageBuffer *>(rTuningParam.pBpc2Buf);
                if (pSrc2 != NULL) {
                    Input src2;
                    src2.mPortID = mDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_50 ? PORT_IMGBI : PORT_DMGI;
                    src2.mPortID.group = 0;
                    src2.mBuffer = pSrc2;
                    if (mSecureEnum) {
                        MUINT32 sec_handle = mSecBufCtrl.registerAndGetSecHandle(mSecureEnum, src2.mBuffer);
                        if (!sec_handle) return BAD_VALUE;
                        src2.mSecHandle = sec_handle;
                        src2.mSecureTag = mSecureEnum;
                    }
                    enqueParams.mvFrameParams.editItemAt(0).mvIn.push_back(src2);
                    MY_LOGD_IF(mbEnableLog, "sensor(%d) P2Node EnQ Src mPortID.index(%d) Fmt(0x%x) "
                        "Size(%dx%d) sizeInBytes(%zu) bufferfd(%d)", mOpenId, src2.mPortID.index, src2.mBuffer->getImgFormat(),
                        src2.mBuffer->getImgSize().w, src2.mBuffer->getImgSize().h, src2.mBuffer->getBufSizeInBytes(0), src2.mBuffer->getFD(0));
                }

            }
        } else {
            MY_LOGD_IF(mbEnableLog, "sensor(%d) P2 setIsp clear tuning %p : %d", mOpenId, pTuning, tuningsize);
            ::memset((unsigned char*)(pTuning), 0, tuningsize);
        }
        //
        if( pMeta_OutApp ) {
            CAM_TRACE_BEGIN("P2F:Meta_OutApp");
            *pMeta_OutApp = outMetaSet.appMeta;
            CAM_TRACE_END();
            //
            MRect cropRegion = pCropInfo->crop_a;
            if( pCropInfo->isFSCEnabled ) {
                cropRegion.p.x = pCropInfo->cropf_a.p.x;
                cropRegion.p.y = pCropInfo->cropf_a.p.y;
                cropRegion.s.w = pCropInfo->cropf_a.s.w;
                cropRegion.s.h = pCropInfo->cropf_a.s.h;
            }
            if( pCropInfo->isEisEabled ) {
                cropRegion.p.x += pCropInfo->eis_mv_a.p.x;
                cropRegion.p.y += pCropInfo->eis_mv_a.p.y;
            }
            //
            updateCropRegion(cropRegion, pMeta_OutApp);
        }
        //
        if( pMeta_OutHal ) {
            CAM_TRACE_BEGIN("P2F:Meta_OutHal");
            *pMeta_OutHal = *pMeta_InHal;
            *pMeta_OutHal += outMetaSet.halMeta;
#ifdef FEATURE_MODIFY
            CAM_TRACE_BEGIN("P2F:makeDebugExif");
            makeDebugExif(pMeta_InHal , pMeta_OutHal);
            CAM_TRACE_END();
#endif
            trySetMetadata<MINT32>(pMeta_OutHal, MTK_PIPELINE_FRAME_NUMBER, pRequest->getPipelineFrame()->getFrameNo());
            trySetMetadata<MINT32>(pMeta_OutHal, MTK_PIPELINE_REQUEST_NUMBER, pRequest->getPipelineFrame()->getRequestNo());
            CAM_TRACE_END();
        }
    }
    CAM_TRACE_END();

    CAM_TRACE_BEGIN("P2F:preparePQParam");
    //
    // ExtraParam
    DpPqParam* wdmaPqParam = NULL;
    DpPqParam* wrotPqParam = NULL;
    for( size_t i = 0; i < enqueParams.mvFrameParams[0].mvOut.size(); i++ )
    {
        const Output &dst = enqueParams.mvFrameParams[0].mvOut[i];

        // Clear Zoom
        if( wdmaPqParam == NULL && dst.mPortID.index == NSImageio::NSIspio::EPortIndex_WDMAO )
        {
            wdmaPqParam = new DpPqParam();
            preparePQParam(*wdmaPqParam, dst, pRequest, czTimeStamp);
        }
        else if( wrotPqParam == NULL && dst.mPortID.index == NSImageio::NSIspio::EPortIndex_WROTO )
        {
            wrotPqParam = new DpPqParam();
            preparePQParam(*wrotPqParam, dst, pRequest, czTimeStamp);
        }
    }

    PQParam* pqParam = new PQParam();
    pqParam->WDMAPQParam = wdmaPqParam;
    pqParam->WROTPQParam = wrotPqParam;

    MY_LOGD_IF(mbEnableLog, "pqParam:%p pqParam.WDMA:%p pqParam.WROT:%p", pqParam, pqParam->WDMAPQParam, pqParam->WROTPQParam);

    ExtraParam extraParam;
    extraParam.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
    extraParam.moduleStruct = static_cast<void*>(pqParam);
    enqueParams.mvFrameParams.editItemAt(0).mvExtraParam.push_back(extraParam);
    CAM_TRACE_END();

    // callback
    enqueParams.mpfnCallback = pass2CbFunc;
    enqueParams.mpCookie     = this;

    //
    #if 0
    // FIXME: need this?
    enqueParams.mvPrivaData.push_back(NULL);

    // for crop
    enqueParams.mvP1SrcCrop.push_back(pCropInfo->crop_p1_sensor);
    enqueParams.mvP1Dst.push_back(pCropInfo->dstsize_resizer);
    enqueParams.mvP1DstCrop.push_back(pCropInfo->crop_dma);
    #endif
    //
    //
    pRequest->context.timer.stopOnExecute();
    MY_LOGD("sensor(%d) frame %u cnt %d, in %zu(%s), out %zu(%s) customOption: 0x%08X Time: [%u/%u]",
            mOpenId, pRequest->frameNo, muEnqueCnt, enqueParams.mvFrameParams[0].mvIn.size(), inDebugString.string(),
            enqueParams.mvFrameParams[0].mvOut.size(), outDebugString.string(), pRequest->context.custom_option,
            pRequest->context.timer.getElapsedSetIsp(), pRequest->context.timer.getElapsedOnExecute());
    //
    { // add request to queue
        Mutex::Autolock _l(mLock);
        mvRunning.push_back(pRequest);
#if P2_DEQUE_DEBUG
        mvParams.push_back(enqueParams);
#endif
        muEnqueCnt++;
    }
    //
    {
        MY_LOGD_IF(mbEnableLog, "sensor(%d) enque pass2 ...", mOpenId);
        CAM_TRACE_NAME("P2F:[Proc]drv enq");
#ifdef FEATURE_MODIFY
        featureEnqueParams.setQParams(enqueParams);
#endif // FEATURE_MODIFY

        if( !mpPipe->enque(featureEnqueParams) )
        {
            MY_LOGE("sensor(%d) enque pass2 failed", mOpenId);
            //
            { // remove job from queue
                Mutex::Autolock _l(mLock);
                vector<sp<Request> >::iterator iter = mvRunning.end();
                while( iter != mvRunning.begin() ) {
                    iter--;
                    if( *iter == pRequest ) {
                        mvRunning.erase(iter);
                        break;
                    }
                }

                MY_LOGW("sensor(%d) cnt %d execute failed", mOpenId, muDequeCnt);
                muDequeCnt++;
            }
            return UNKNOWN_ERROR;
        }
        MY_LOGD_IF(mbEnableLog, "sensor(%d) enque pass2 success", mOpenId);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
onFinish(
    FrameParams const& params,
    MBOOL const success
)
{
    CAM_TRACE_NAME("P2F:[Proc]Finish");
    //params.in.mHandle->updateState(BufferHandle::Buffer_ReadDone);
    for( size_t i = 0; i < params.vOut.size(); i++ )
    #ifdef FEATURE_MODIFY
        if( params.vOut[i].mHandle != NULL && params.vOut[i].mHandle->isValid() )
    #else
        if (params.vOut[i].mHandle.get())
    #endif // FEATURE_MODIFY
            params.vOut[i].mHandle->updateState(
                    success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
                    );
    if( params.outApp.get() )
        params.outApp->updateState(success ? MetaHandle::STATE_WRITE_OK : MetaHandle::STATE_WRITE_FAIL);
    if( params.outHal.get() )
        params.outHal->updateState(success ? MetaHandle::STATE_WRITE_OK : MetaHandle::STATE_WRITE_FAIL);
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
pass2CbFunc(QParams& rParams)
{
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc +++");
    P2Procedure* pProcedure = reinterpret_cast<P2Procedure*>(rParams.mpCookie);
    pProcedure->handleDeque(rParams);
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc ---");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
handleDeque(QParams& rParams)
{
    CAM_TRACE_NAME("P2F:[Proc]deque");
    Mutex::Autolock _l(mLock);
    sp<Request> pRequest = NULL;
    {
        CAM_TRACE_NAME("P2F:deque check");

        MY_LOGD_IF(mbEnableLog, "sensor(%d) p2 done %d, success %d", mOpenId, muDequeCnt, rParams.mDequeSuccess);
        pRequest = mvRunning.front();
        mvRunning.erase(mvRunning.begin());
        muDequeCnt++;
        MY_LOGD("sensor(%d) p2 done muDequeCnt:%d", mOpenId, muDequeCnt);

#if P2_DEQUE_DEBUG
        if( mvParams.size() )
        {
            QParams checkParam;
            checkParam = mvParams.front();
            mvParams.erase(mvParams.begin());
            //
            // make sure params are correct
            //
            #define ERROR_IF_NOT_MATCHED(item, i, expected, actual) do{             \
                if( expected != actual)                                             \
                    MY_LOGE("sensor(%d) %s %zu: expected %#" PRIxPTR " != %#" PRIxPTR "", mOpenId, item, i, expected, actual); \
            } while(0)
            //
            for( size_t i = 0 ; i < checkParam.mvFrameParams[0].mvIn.size() ; i++ )
            {
                if( i >= rParams.mvFrameParams[0].mvIn.size() ) {
                    MY_LOGE("sensor(%d) no src in dequed Params", mOpenId);
                    break;
                }
                #ifdef FEATURE_MODIFY
                if( rParams.mvFrameParams[0].mvIn[i].mBuffer == NULL )
                {
                    continue;
                }
                #endif // FEATURE_MODIFY
                //
                ERROR_IF_NOT_MATCHED("src pa of in", i,
                        checkParam.mvFrameParams[0].mvIn[i].mBuffer->getBufPA(0),
                        rParams.mvFrameParams[0].mvIn[i].mBuffer->getBufPA(0)
                        );
                ERROR_IF_NOT_MATCHED("src va of in", i,
                        checkParam.mvFrameParams[0].mvIn[i].mBuffer->getBufVA(0),
                        rParams.mvFrameParams[0].mvIn[i].mBuffer->getBufVA(0)
                        );
            }
            //
            for( size_t i = 0 ; i < checkParam.mvFrameParams[0].mvOut.size() ; i++ )
            {
                if( i >= rParams.mvFrameParams[0].mvOut.size() ) {
                    MY_LOGE("sensor(%d) no enough dst in dequed Params, %zu", mOpenId, i);
                    break;
                }
                #ifdef FEATURE_MODIFY
                if( rParams.mvFrameParams[0].mvOut[i].mBuffer == NULL )
                {
                    continue;
                }
                #endif // FEATURE_MODIFY
                //
                ERROR_IF_NOT_MATCHED("dst pa of out", i,
                        checkParam.mvFrameParams[0].mvOut[i].mBuffer->getBufPA(0),
                        rParams.mvFrameParams[0].mvOut[i].mBuffer->getBufPA(0)
                        );
                ERROR_IF_NOT_MATCHED("dst va of out", i,
                        checkParam.mvFrameParams[0].mvOut[i].mBuffer->getBufVA(0),
                        rParams.mvFrameParams[0].mvOut[i].mBuffer->getBufVA(0)
                        );
            }
            //
            #undef ERROR_IF_NOT_MATCHED
        }
        else {
            MY_LOGW("sensor(%d) params size not matched", mOpenId);
        }
#endif
    }
    //
    CAM_TRACE_BEGIN("P2F:release tuning");
    if( rParams.mvFrameParams[0].mTuningData ) {
        void* pTuning = rParams.mvFrameParams[0].mTuningData;
        if( pTuning ) {
            mDebugControl.needMemCheck() ? PostRedZone::mydelete(pTuning) : ::free(pTuning);
        }
    }
    CAM_TRACE_END();
    CAM_TRACE_BEGIN("P2F:release PQParam");
    for( size_t i = 0; i < rParams.mvFrameParams[0].mvExtraParam.size(); i++ )
    {
        MUINT cmdIdx = rParams.mvFrameParams[0].mvExtraParam[i].CmdIdx;

        if( cmdIdx == EPIPE_MDP_PQPARAM_CMD )
        {
            PQParam* extraParam = static_cast<PQParam*>(rParams.mvFrameParams[0].mvExtraParam[i].moduleStruct);
            if( extraParam )
            {
                DpPqParam* wdmaPqParam = static_cast<DpPqParam*>(extraParam->WDMAPQParam);
                if( wdmaPqParam )
                {
                    DpIspParam& ispParam = wdmaPqParam->u.isp;
                    if( ispParam.p_faceInfor )
                    {
                        delete (FD_DATATYPE*)ispParam.p_faceInfor;
                        ispParam.p_faceInfor = NULL;
                    }
                    delete wdmaPqParam;
                }

                DpPqParam* wrotPqParam = static_cast<DpPqParam*>(extraParam->WROTPQParam);
                if( wrotPqParam )
                {
                    DpIspParam& ispParam = wrotPqParam->u.isp;
                    if( ispParam.p_faceInfor )
                    {
                        delete (FD_DATATYPE*)ispParam.p_faceInfor;
                        ispParam.p_faceInfor = NULL;
                    }
                    delete wrotPqParam;
                }
                delete extraParam;
                rParams.mvFrameParams.editItemAt(0).mvExtraParam.editItemAt(i).moduleStruct = NULL;
            }
        }
    }
    CAM_TRACE_END();
    //
    pRequest->responseDone(rParams.mDequeSuccess ? OK : UNKNOWN_ERROR);

    CAM_TRACE_BEGIN("P2F:Release Request");
    pRequest = NULL;
    CAM_TRACE_END();
    //
    mCondJob.signal();
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
checkParams(FrameParams const params, MBOOL const skipCheckOutBuffer) const
{
#define CHECK(lv, val, fail_ret, ...) \
    do{                               \
        if( !(val) )                  \
        {                             \
            MY_LOG##lv(__VA_ARGS__);  \
            return fail_ret;          \
        }                             \
    } while(0)
    //
    CHECK( E, params.in.mHandle.get() , BAD_VALUE , "no src handle" );
    if(!skipCheckOutBuffer)
    CHECK( D, params.vOut.size()      , BAD_VALUE , "no dst" );
    CHECK( E, params.inApp.get()      , BAD_VALUE , "no in app meta" );
    CHECK( E, params.inHal.get()      , BAD_VALUE , "no in hal meta" );
    //
#undef CHECK
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
getCropInfos(
    IMetadata* const inApp,
    IMetadata* const inHal,
    MBOOL const isResized,
    Cropper::crop_info_t& cropInfos
) const
{
    if( ! tryGetMetadata<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, cropInfos.sensor_size) ) {
        MY_LOGE("sensor(%d) cannot get MTK_HAL_REQUEST_SENSOR_SIZE", mOpenId);
        return BAD_VALUE;
    }
    //
    MSize const sensor = cropInfos.sensor_size;
    MSize const active = mInitParams.activeArray.s;
    //
    cropInfos.isResized = isResized;
    // get current p1 buffer crop status
    if(
            !( tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, cropInfos.crop_p1_sensor) &&
               tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE      , cropInfos.dstsize_resizer) &&
               tryGetMetadata<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION   , cropInfos.crop_dma)
             )
      ) {
        MY_LOGW_IF(1, "sensor(%d) [FIXME] should sync with p1 for rrz setting", mOpenId);
        //
        cropInfos.crop_p1_sensor  = MRect( MPoint(0,0), sensor );
        cropInfos.dstsize_resizer = sensor;
        cropInfos.crop_dma        = MRect( MPoint(0,0), sensor );
    }

    // get current p1 buffer crop status
    if(
            !( tryGetMetadata<MRect>(inHal, MTK_P1NODE_BIN_CROP_REGION, cropInfos.crop_p1_bin) &&
               tryGetMetadata<MSize>(inHal, MTK_P1NODE_BIN_SIZE       , cropInfos.bin_size)
             )
      ) {
        MY_LOGW_IF(1, "sensor(%d) [FIXME] should sync with p1 for bin setting", mOpenId);
        //
        cropInfos.crop_p1_bin  = MRect( MPoint(0,0), sensor );
        cropInfos.bin_size = sensor;
    }

    MY_LOGD_IF(mbEnableLog, "sensor(%d) SCALAR_CROP_REGION:(%d,%d)(%dx%d) RESIZER_SIZE:(%dx%d) DMA_CROP_REGION:(%d,%d)(%dx%d) BIN_GROP_REGION:(%d,%d)(%dx%d) BIN_SIZE(%dx%d)",
        mOpenId,
        cropInfos.crop_p1_sensor.p.x, cropInfos.crop_p1_sensor.p.y,
        cropInfos.crop_p1_sensor.s.w, cropInfos.crop_p1_sensor.s.h,
        cropInfos.dstsize_resizer.w, cropInfos.dstsize_resizer.h,
        cropInfos.crop_dma.p.x, cropInfos.crop_dma.p.y,
        cropInfos.crop_dma.s.w, cropInfos.crop_dma.s.h,
        cropInfos.crop_p1_bin.p.x, cropInfos.crop_p1_bin.p.y,
        cropInfos.crop_p1_bin.s.w, cropInfos.crop_p1_bin.s.h,
        cropInfos.bin_size.w, cropInfos.bin_size.h
        );
    //
    // setup transform
    MINT32 sensorMode;
    if(!tryGetMetadata<MINT32>(inHal, MTK_P1NODE_SENSOR_MODE, sensorMode)) {
        MY_LOGE("sensor(%d) cannot get MTK_P1NODE_SENSOR_MODE", mOpenId);
        return BAD_VALUE;
    }
    //
    HwTransHelper hwTransHelper(mInitParams.openId);
    HwMatrix matToActive;
    if (!hwTransHelper.getMatrixToActive(sensorMode, cropInfos.matSensor2Active) ||
        !hwTransHelper.getMatrixFromActive(sensorMode, cropInfos.matActive2Sensor)) {
       MY_LOGE("sensor(%d) get matrix fail", mOpenId);
       return UNKNOWN_ERROR;
    }

    cropInfos.tranSensor2Resized = simpleTransform(
                cropInfos.crop_p1_sensor.p,
                cropInfos.crop_p1_sensor.s,
                cropInfos.dstsize_resizer
            );
    MRect appCrop;
    if( tryGetMetadata<MRect>(inApp, MTK_SCALER_CROP_REGION, appCrop) )
    {
        MRect appSensorDomain;
        cropInfos.matActive2Sensor.transform(appCrop, appSensorDomain);
        if( appSensorDomain.s.w > cropInfos.crop_p1_sensor.s.w ||
            appSensorDomain.s.h > cropInfos.crop_p1_sensor.s.h )
        {
            MY_LOGD("AppCrop(%d,%d)(%dx%d) to sensor domain(%d,%d)(%dx%d) will exceed p1Crop(%d,%d)(%dx%d)!",
                    appCrop.p.x, appCrop.p.y, appCrop.s.w, appCrop.s.h,
                    appSensorDomain.p.x, appSensorDomain.p.y, appSensorDomain.s.w, appSensorDomain.s.h,
                    cropInfos.crop_p1_sensor.p.x, cropInfos.crop_p1_sensor.p.y, cropInfos.crop_p1_sensor.s.w, cropInfos.crop_p1_sensor.s.h);
        }
    }
    else
    {
        MY_LOGW_IF(mbEnableLog, "Cannot get MTK_SCALER_CROP_REGION");
    }
    MY_LOGD_IF(mbEnableLog,"Active(%dx%d), Sensor(%dx%d), P1Crop=(%d,%d)(%dx%d), P1Size=(%dx%d), AppCrop=(%d,%d)(%dx%d)",
               mInitParams.activeArray.s.w, mInitParams.activeArray.s.h, sensor.w, sensor.h,
               cropInfos.crop_p1_sensor.p.x, cropInfos.crop_p1_sensor.p.y,
               cropInfos.crop_p1_sensor.s.w, cropInfos.crop_p1_sensor.s.h,
               cropInfos.dstsize_resizer.w, cropInfos.dstsize_resizer.h,
               appCrop.p.x, appCrop.p.y, appCrop.s.w, appCrop.s.h);
    //
    MBOOL const isEisOn = isEISOn(inApp);

    IStreamingFeaturePipe::UsageHint usageHint = getPipeUsageHint(mInitParams.usageHint);
    MUINT32 eisMode   = usageHint.mEISInfo.mode;
    MUINT32 eisFactor = usageHint.mEISInfo.factor;

    MSize fovSensorMargin;
    MSize fovRRZMargin;
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    getFOVMarginPixel(inApp, inHal, fovSensorMargin, fovRRZMargin);

    MY_LOGD_IF(mbEnableLog, "sensor(%d) fovSensorMargin:(%dx%d) fovRRZMargin:(%dx%d)",
        mOpenId, fovSensorMargin.w, fovSensorMargin.h, fovRRZMargin.w, fovRRZMargin.h);
#endif
    //
    MRect cropRegion; //active array domain
    queryCropRegion(cropInfos, inApp, inHal, isEisOn, fovSensorMargin, cropRegion);
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
            MBOOL isResizedDomain = MTRUE;

#if 0
            //eis in sensor domain
            isResizedDomain = MFALSE;
            pMv_s->p.x  = eisInfo.x_int - (sensor.w * (eisFactor-100)/2/eisFactor);
            pMv_s->pf.x = eisInfo.x_float;
            pMv_s->p.y  = eisInfo.y_int - (sensor.h * (eisFactor-100)/2/eisFactor);
            pMv_s->pf.y = eisInfo.y_float;
            //
            cropInfos.eis_mv_r = transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_s);
#else
            MSize const resizer = cropInfos.dstsize_resizer;

#if SUPPORT_EIS_MV

            if (eisInfo.is_from_zzr)
            {
                pMv_r->p.x  = eisInfo.x_mv_int;
                pMv_r->pf.x = 0;
                pMv_r->p.y  = eisInfo.y_mv_int;
                pMv_r->pf.y = 0;
                cropInfos.eis_mv_s = inv_transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_r);
            }
            else
            {
                isResizedDomain = MFALSE;
                pMv_s->p.x  = eisInfo.x_mv_int;
                pMv_s->pf.x = 0;
                pMv_s->p.y  = eisInfo.y_mv_int;
                pMv_s->pf.y = 0;
                cropInfos.eis_mv_r = transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_s);
            }
#else
            if( EIS_MODE_IS_EIS_12_ENABLED(eisMode) )
            {
                pMv_r->p.x  = eisInfo.x_int - (resizer.w * (eisFactor-100)/2/eisFactor);
                pMv_r->pf.x = 0;
                pMv_r->p.y  = eisInfo.y_int - (resizer.h * (eisFactor-100)/2/eisFactor);
                pMv_r->pf.y = 0;
            }
            else
            {
                pMv_r->p.x = 0;
                pMv_r->pf.x = 0.0f;
                pMv_r->p.y = 0;
                pMv_r->pf.y = 0.0f;
            }

            //
            cropInfos.eis_mv_s = inv_transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_r);
#endif
            //
            MY_LOGD_IF(mbEnableLog, "sensor(%d) mv (%s): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
                    mOpenId,
                    isResizedDomain ? "r->s" : "s->r",
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
            // cropInfos.eis_mv_a = inv_transform(cropInfos.tranActive2Sensor, cropInfos.eis_mv_s);
            cropInfos.matSensor2Active.transform(cropInfos.eis_mv_s.p,cropInfos.eis_mv_a.p);
            // FIXME: float
            //cropInfos.matSensor2Active.transform(cropInfos.eis_mv_s.pf,cropInfos.eis_mv_a.pf);

            MY_LOGD_IF(mbEnableLog, "sensor(%d) mv in active %d/%d, %d/%d",
                    mOpenId,
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
P2Procedure::
queryCropRegion(
    Cropper::crop_info_t& cropInfos,
    IMetadata* const inApp,
    IMetadata *const inHal,
    MBOOL const isEisOn,
    MSize const &fovMargin,
    MRect& cropRegion
) const
{
    if( !tryGetMetadata<MRect>(inApp, MTK_SCALER_CROP_REGION, cropRegion) ) {
        cropRegion.p = MPoint(0,0);
        cropRegion.s = mInitParams.activeArray.s;
        MY_LOGW_IF(mbEnableLog, "sensor(%d) no MTK_SCALER_CROP_REGION, crop full size %dx%d",
                mOpenId, cropRegion.s.w, cropRegion.s.h);
    }
    MY_LOGD_IF(mbEnableLog, "sensor(%d) control: cropRegion(%d, %d, %dx%d)",
            mOpenId, cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

    cropRegion.p.x += fovMargin.w;
    cropRegion.p.y += fovMargin.h;
    cropRegion.s   -= fovMargin*2;

    // Get FSC crop data
    MRect fscSensorCropReigon, fscActiveCropReigon;
    MRectF fscOrgCropReigonf, fscActiveCropReigonf;
    MUINT32 fscMode = mInitParams.usageHint.mFSCMode;
    MFLOAT fscScaleRatio = 1.0f;
    if( EFSC_FSC_ENABLED(fscMode) )
    {
        IMetadata::IEntry cropEntry = inHal->entryFor(MTK_FSC_CROP_DATA);
        if (cropEntry.count())
        {
            IMetadata::Memory metaMemory = cropEntry.itemAt(0, Type2Type< IMetadata::Memory >());
            FSC_CROPPING_DATA_STRUCT *cropData = (FSC_CROPPING_DATA_STRUCT*)metaMemory.array();
            fscScaleRatio = cropData->image_scale;
            if (EFSC_DEBUG_ENABLED(fscMode) && ::property_get_int32(FSC_FORCE_MAX_CROP_PROPERTY, 0))
            {
                MFLOAT maxCropValue = ::property_get_int32(FSC_MAX_CROP_VALUE_PROPERTY, 80);
                fscScaleRatio = maxCropValue/100;
            }
        }

        fscOrgCropReigonf.p.x = cropRegion.p.x;
        fscOrgCropReigonf.p.y = cropRegion.p.y;
        fscOrgCropReigonf.s.w = cropRegion.s.w;
        fscOrgCropReigonf.s.h = cropRegion.s.h;

        fscActiveCropReigonf = fscOrgCropReigonf;
        fscActiveCropReigonf.p.x += (fscActiveCropReigonf.s.w * (1.0f - fscScaleRatio) / 2);
        fscActiveCropReigonf.p.y += (fscActiveCropReigonf.s.h * (1.0f - fscScaleRatio) / 2);
        fscActiveCropReigonf.s    = (fscActiveCropReigonf.s * fscScaleRatio);
        cropInfos.cropf_a = fscActiveCropReigonf;
        cropInfos.isFSCEnabled = MTRUE;

        cropRegion.p.x = fscActiveCropReigonf.p.x;
        cropRegion.p.y = fscActiveCropReigonf.p.y;
        cropRegion.s.w = fscActiveCropReigonf.s.w;
        cropRegion.s.h = fscActiveCropReigonf.s.h;
        MY_LOGD_IF(EFSC_DEBUG_ENABLED(fscMode), "sensor(%d) FSC(%f), Active(%.0f,%.0f,%.0f,%.0f)->crop(%f,%f,%f,%f)",
                   mOpenId, fscScaleRatio,
                   fscOrgCropReigonf.p.x, fscOrgCropReigonf.p.y, fscOrgCropReigonf.s.w, fscOrgCropReigonf.s.h,
                   fscActiveCropReigonf.p.x, fscActiveCropReigonf.p.y, fscActiveCropReigonf.s.w, fscActiveCropReigonf.s.h);
    }
    else
    {
        cropInfos.isFSCEnabled = MFALSE;
    }

    #if SUPPORT_EIS
    if( isEisOn ) {
        MUINT32 eis_factor = mInitParams.usageHint.mEisInfo.factor;
        // EIS margin is relative to FSC cropped domain
        if( EFSC_FSC_ENABLED(fscMode) )
        {
            MFLOAT fscEISScaleRatio = fscScaleRatio * 100 / eis_factor;

            fscActiveCropReigonf = fscOrgCropReigonf;
            fscActiveCropReigonf.p.x += (fscActiveCropReigonf.s.w * (1.0f-fscEISScaleRatio) / 2);
            fscActiveCropReigonf.p.y += (fscActiveCropReigonf.s.h * (1.0f-fscEISScaleRatio) / 2);
            fscActiveCropReigonf.s    = (fscActiveCropReigonf.s * fscEISScaleRatio);
            cropInfos.cropf_a = fscActiveCropReigonf;

            cropRegion.p.x = fscActiveCropReigonf.p.x;
            cropRegion.p.y = fscActiveCropReigonf.p.y;
            cropRegion.s.w = fscActiveCropReigonf.s.w;
            cropRegion.s.h = fscActiveCropReigonf.s.h;
            MY_LOGD_IF(EFSC_DEBUG_ENABLED(fscMode), "sensor(%d) FSC(%f) EIS(%d)->(%f), Active(%.0f,%.0f,%.0f,%.0f)->crop(%f,%f,%f,%f)",
                       mOpenId, fscScaleRatio, eis_factor, fscEISScaleRatio,
                       fscOrgCropReigonf.p.x, fscOrgCropReigonf.p.y, fscOrgCropReigonf.s.w, fscOrgCropReigonf.s.h,
                       fscActiveCropReigonf.p.x, fscActiveCropReigonf.p.y, fscActiveCropReigonf.s.w, fscActiveCropReigonf.s.h);
        }
        else
        {
            cropRegion.p.x += (cropRegion.s.w * (eis_factor-100)/2/eis_factor);
            cropRegion.p.y += (cropRegion.s.h * (eis_factor-100)/2/eis_factor);
            cropRegion.s    = (cropRegion.s * 100/eis_factor);
            MY_LOGD_IF(mbEnableLog, "sensor(%d) EIS: factor %d, cropRegion(%d, %d, %dx%d)",
                       mOpenId, eis_factor,
                       cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
        }

    }
    #endif
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
updateCropRegion(
    MRect const crop,
    IMetadata* meta_result
) const
{
    trySetMetadata<MRect>(meta_result, MTK_SCALER_CROP_REGION, crop);
    //
    MY_LOGD_IF( DEBUG_LOG && mbEnableLog, "mSensor(%d) result: cropRegion (%d, %d, %dx%d)",
            mOpenId, crop.p.x, crop.p.y, crop.s.w, crop.s.h);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2Procedure::
isEISOn(
    IMetadata* const inApp
) const
{
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if( !tryGetMetadata<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode) ) {
        MY_LOGW_IF(mbEnableLog, "sensor(%d) no MTK_CONTROL_VIDEO_STABILIZATION_MODE", mOpenId);
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
P2Procedure::
queryEisRegion(
    IMetadata* const inHal,
    eis_region& region
) const
{
    IMetadata::IEntry entry = inHal->entryFor(MTK_EIS_REGION);

#if SUPPORT_EIS_MV
    // get EIS's motion vector
    if (entry.count() > EIS_REGION_INDEX_ISFROMRZ)
    {
        MINT32 x_mv         = entry.itemAt(EIS_REGION_INDEX_MV2CENTERX, Type2Type<MINT32>());
        MINT32 y_mv         = entry.itemAt(EIS_REGION_INDEX_MV2CENTERY, Type2Type<MINT32>());
        region.is_from_zzr  = entry.itemAt(EIS_REGION_INDEX_ISFROMRZ, Type2Type<MINT32>());
        MBOOL x_mv_negative = x_mv >> 31;
        MBOOL y_mv_negative = y_mv >> 31;
        // convert to positive for getting parts of int and float if negative
        if (x_mv_negative) x_mv = ~x_mv + 1;
        if (y_mv_negative) y_mv = ~y_mv + 1;
        //
        region.x_mv_int   = (x_mv & (~0xFF)) >> 8;
        region.x_mv_float = (x_mv & (0xFF)) << 31;
        if(x_mv_negative){
            region.x_mv_int   = ~region.x_mv_int + 1;
            region.x_mv_float = ~region.x_mv_float + 1;
        }
        region.y_mv_int   = (y_mv& (~0xFF)) >> 8;
        region.y_mv_float = (y_mv& (0xFF)) << 31;
        if(y_mv_negative){
            region.y_mv_int   = ~region.y_mv_int + 1;
            region.y_mv_float = ~region.x_mv_float + 1;
        }
        //
        MY_LOGD_IF(mbEnableLog, "sensor(%d) EIS MV:%d, %d, %d",
                        mOpenId
                        region.s.w,
                        region.s.h,
                        region.is_from_zzr);
    }
#endif

#ifdef FEATURE_MODIFY
    if (entry.count() > EIS_REGION_INDEX_GMVY)
    {
        region.gmvX = entry.itemAt(EIS_REGION_INDEX_GMVX, Type2Type<MINT32>());
        region.gmvY = entry.itemAt(EIS_REGION_INDEX_GMVY, Type2Type<MINT32>());
        MY_LOGD_IF(mbEnableLog, "sensor(%d) EIS gmvX:%d, gmvY: %d", mOpenId, region.gmvX, region.gmvY);
    }

    if (entry.count() > EIS_REGION_INDEX_MAX_GMV)
    {
        region.confX = entry.itemAt(EIS_REGION_INDEX_CONFX, Type2Type<MINT32>());
        region.confY = entry.itemAt(EIS_REGION_INDEX_CONFY, Type2Type<MINT32>());
        region.maxGMV = entry.itemAt(EIS_REGION_INDEX_MAX_GMV, Type2Type<MINT32>());
        MY_LOGD_IF(mbEnableLog, "sensor(%d) EIS confX:%d, confY: %d, maxGMV: %d",
            mOpenId, region.confX, region.confY, region.maxGMV);
    }
    else if (entry.count() > EIS_REGION_INDEX_CONFY)
    {
        region.confX = entry.itemAt(EIS_REGION_INDEX_CONFX, Type2Type<MINT32>());
        region.confY = entry.itemAt(EIS_REGION_INDEX_CONFY, Type2Type<MINT32>());
        region.maxGMV = LMV_MAX_GMV_64;//LMV_MAX_GMV_64;
        MY_LOGD_IF(mbEnableLog, "sensor(%d) EIS confX:%d, confY: %d, fix maxGMV: %d",
            mOpenId, region.confX, region.confY, region.maxGMV);
    }

    if (abs(region.gmvX)/LMV_GMV_VALUE_TO_PIXEL_UNIT > region.maxGMV)
    {
        MY_LOGW_IF(mbEnableLog, "warning! gmvX(%d) > maxGMV(%d)", region.gmvX/LMV_GMV_VALUE_TO_PIXEL_UNIT, region.maxGMV);
        region.gmvX = (region.gmvX > 0) ? (region.maxGMV * LMV_GMV_VALUE_TO_PIXEL_UNIT) :
            -(region.maxGMV * LMV_GMV_VALUE_TO_PIXEL_UNIT);
    }

    if (abs(region.gmvY)/LMV_GMV_VALUE_TO_PIXEL_UNIT > region.maxGMV)
    {
        MY_LOGW_IF(mbEnableLog, "warning! gmvY(%d) > maxGMV(%d)", region.gmvY/LMV_GMV_VALUE_TO_PIXEL_UNIT, region.maxGMV);
        region.gmvY = (region.gmvY > 0) ? (region.maxGMV * LMV_GMV_VALUE_TO_PIXEL_UNIT) :
            -(region.maxGMV * LMV_GMV_VALUE_TO_PIXEL_UNIT);
    }
#endif // FEATURE_MODIFY

    // get EIS's region
    if (entry.count() > EIS_REGION_INDEX_HEIGHT)
    {
        region.x_int        = entry.itemAt(EIS_REGION_INDEX_XINT, Type2Type<MINT32>());
        region.x_float      = entry.itemAt(EIS_REGION_INDEX_XFLOAT, Type2Type<MINT32>());
        region.y_int        = entry.itemAt(EIS_REGION_INDEX_YINT, Type2Type<MINT32>());
        region.y_float      = entry.itemAt(EIS_REGION_INDEX_YFLOAT, Type2Type<MINT32>());
        region.s.w          = entry.itemAt(EIS_REGION_INDEX_WIDTH, Type2Type<MINT32>());
        region.s.h          = entry.itemAt(EIS_REGION_INDEX_HEIGHT, Type2Type<MINT32>());
        //
        MY_LOGD_IF(mbEnableLog, "sensor(%d) EIS Region: %d, %d, %d, %d, %dx%d",
                        mOpenId,
                        region.x_int,
                        region.x_float,
                        region.y_int,
                        region.y_float,
                        region.s.w,
                        region.s.h);
       return MTRUE;
    }

    MY_LOGW("sensor(%d) wrong eis region count %u", mOpenId, entry.count());
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
getEISMarginPixel(
    IMetadata* const inApp,
    IMetadata* const inHal,
    MSize& eisSensorMargin,
    MRectF& eisRRZCrop
) const
{
    MBOOL const isEisOn = isEISOn(inApp);

    if( !isEisOn )
    {
        return;
    }

    IStreamingFeaturePipe::UsageHint usageHint = getPipeUsageHint(mInitParams.usageHint);

    MUINT32 eisMode = usageHint.mEISInfo.mode;
    MUINT32 eisFactor = usageHint.mEISInfo.factor;

    MSize fovSensorMargin;
    MSize fovRRZMargin;
#if (MTKCAM_HAVE_DUAL_ZOOM_SUPPORT == 1)
    getFOVMarginPixel(inApp, inHal, fovSensorMargin, fovRRZMargin);
    MY_LOGD_IF(mbEnableLog, "fovSensorMargin(%dx%d) fovRRZMargin(%dx%d)", fovSensorMargin.w, fovSensorMargin.h, fovRRZMargin.w, fovRRZMargin.h);
#endif

    MRect P1Crop;
    MSize DstResizer;
    if( !( tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, P1Crop) &&
           tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE      , DstResizer)) )
    {
        MY_LOGW("should sync with p1 for rrz setting");
    }

    // EIS margin is relative to FSC cropped domain
    if( EFSC_FSC_ENABLED(mInitParams.usageHint.mFSCMode) )
    {
        MFLOAT fscScaleRatio = 1.0f;
        IMetadata::IEntry cropEntry = inHal->entryFor(MTK_FSC_CROP_DATA);
        if( cropEntry.count() )
        {
            IMetadata::Memory metaMemory = cropEntry.itemAt(0, Type2Type< IMetadata::Memory >());
            FSC_CROPPING_DATA_STRUCT *cropData = (FSC_CROPPING_DATA_STRUCT*)metaMemory.array();
            fscScaleRatio = cropData->image_scale;
        }
        MFLOAT fscEISScaleRatio = fscScaleRatio * 100 / eisFactor;

        eisSensorMargin = (P1Crop.s - fovSensorMargin*2 ) * fscEISScaleRatio / 2;

        MPointF eisRRZCropOffset = MPointF( (DstResizer.w - fovRRZMargin.w*2)*(1.0f-fscEISScaleRatio)/2,
                                            (DstResizer.h - fovRRZMargin.h*2)*(1.0f-fscEISScaleRatio)/2 );
        MSizeF eisRRZCropSize;
        eisRRZCropSize.w = (DstResizer.w - fovRRZMargin.w*2 ) * fscEISScaleRatio;
        eisRRZCropSize.h = (DstResizer.h - fovRRZMargin.h*2 ) * fscEISScaleRatio;
        eisRRZCrop       = MRectF(eisRRZCropOffset, eisRRZCropSize);
    }
    else
    {
        eisSensorMargin         = (P1Crop.s - fovSensorMargin*2 ) * (eisFactor - 100) / eisFactor / 2;
        MPointF eisRRZCropOffset = MPointF( (DstResizer.w - fovRRZMargin.w*2 ) * (eisFactor - 100) / eisFactor / 2,
                                            (DstResizer.h - fovRRZMargin.h*2 ) * (eisFactor - 100) / eisFactor / 2 );
        MSizeF eisRRZCropSize;
        eisRRZCropSize.w = (DstResizer.w - fovRRZMargin.w*2 ) * 100 / eisFactor;
        eisRRZCropSize.h = (DstResizer.h - fovRRZMargin.h*2 ) * 100 / eisFactor;
        eisRRZCrop              = MRectF(eisRRZCropOffset, eisRRZCropSize);
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
getFOVMarginPixel(
    IMetadata* const /*inApp*/,
    IMetadata* const inHal,
    MSize& fovSensorMargin,
    MSize& fovRRZMargin
) const
{
    MRect P1Crop;
    MSize DstResizer, FovMargin;
    if(
            !( tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, P1Crop) &&
               tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE      , DstResizer) &&
               tryGetMetadata<MSize>(inHal, MTK_DUALZOOM_FOV_MARGIN_PIXEL, FovMargin)
             ) || (FovMargin.w == 0 || FovMargin.h == 0)
      ) {
        MY_LOGD_IF(1, "sensor(%d) Could not get fov margin", mOpenId);
        fovSensorMargin = MSize(0, 0);
        fovRRZMargin = MSize(0, 0);
        return;
    }

    // update fov info
    /*{
        IMetadata::IEntry entry = inHal->entryFor(MTK_DUALZOOM_FOV_RECT_INFO);
        MINT32 info[sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4];
        if (entry.count() < (sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4))
        {
            MY_LOGW("FOV info not enough, skip FOV");
        }
        else
        {
            for (int i = 0; i < sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4; i++)
            {
                info[i] = entry.itemAt(i, Type2Type<MINT32>());
            }
            if (info[4] != P1Crop.s.w || info[5] != P1Crop.s.h)
            {
                FovMargin.h = FovMargin.w * P1Crop.s.h / P1Crop.s.w;
                info[4] = P1Crop.s.w;
                info[5] = P1Crop.s.h;
                info[6] = P1Crop.s.w - FovMargin.w;
                info[7] = P1Crop.s.h - FovMargin.h;
                MY_LOGD("Update FOV info");
                {
                    IMetadata::IEntry tag(MTK_DUALZOOM_FOV_RECT_INFO);
                    for(int i = 0; i < sizeof(DUAL_ZOOM_FOV_INPUT_INFO) / 4;i++)
                    {
                        tag.push_back(info[i], Type2Type<MINT32>());
                    }
                    inHal->update(MTK_DUALZOOM_FOV_RECT_INFO, tag);
                }
            }
        }
    }*/

    FovMargin.h = FovMargin.w * P1Crop.s.h / P1Crop.s.w;
    fovSensorMargin = FovMargin / 2;
    fovRRZMargin.w = FovMargin.w * DstResizer.w / P1Crop.s.w / 2;
    fovRRZMargin.h = FovMargin.h * DstResizer.h / P1Crop.s.h / 2;

    return;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
MDPProcedure::
createProcessor(InitParams& params)
{
    return new ProcessorBase<MDPProcedure>(params, PROCESSOR_NAME_MDP);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MDPProcedure::
onExecute(
    sp<Request> const pRequest,
    FrameParams const& params
)
{
    CAM_TRACE_NAME("P2F:[MDP]exe");
    FUNC_START;
    //
    MERROR ret = OK;
    //
    IImageBuffer* pSrc = NULL;
    vector<IImageBuffer*> vDst;
    vector<MUINT32> vTransform;
    vector<MRect> vCrop;

    // input
    if( params.in.mHandle.get() )
    {
        if( OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW("src buffer err = %d", ret);
            return ret;
        }
        pSrc = params.in.mHandle->getBuffer();
    }
    else {
        MY_LOGW("no src");
        return BAD_VALUE;
    }
    //
    // output
    for (size_t i = 0; i < params.vOut.size(); i++)
    {
        if (params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE))) {
            MY_LOGW("dst buffer err = %d", ret);
            continue;
        }
        IImageBuffer* pDst = params.vOut[i].mHandle->getBuffer();
        //
        if (pDst != NULL)
        {
            MCropRect cropRect;
#if 0
            MY_LOGD("dump crop info");
            Cropper::dump(*params.pCropInfo);
#endif
            MINT32 const transform  = params.vOut[i].mTransform;
            MSize dstSize = ( transform & eTransform_ROT_90 )
                            ? MSize(pDst->getImgSize().h, pDst->getImgSize().w)
                            : pDst->getImgSize();

            Cropper::calcViewAngle(mbEnableLog, *params.pCropInfo, dstSize, cropRect);
            MRect crop(cropRect.p_integral, cropRect.s);

            vCrop.push_back(crop);
            vDst.push_back(pDst);
            vTransform.push_back(params.vOut[i].mHandle->getTransform());

            MY_LOGD_IF(DEBUG_LOG, "mdp req:%d out:%zu/%zu size:%dx%d crop (%d,%d) %dx%d",
                pRequest->frameNo, i, params.vOut.size(),
                pDst->getImgSize().w, pDst->getImgSize().h,
                crop.p.x, crop.p.y, crop.s.w, crop.s.h);
        }
        else
            MY_LOGW("mdp req:%d empty buffer", pRequest->frameNo);
    }
    //
    if (pSrc == NULL || vDst.size() == 0) {
        MY_LOGE("wrong mdp in/out: src %p, dst count %zu", pSrc, vDst.size());
        return BAD_VALUE;
    }
    //

    MBOOL success = MFALSE;
    {
        //
        #ifdef USING_MTK_LDVT
        success = MTRUE;
        #else
        NSSImager::IImageTransform* pTrans = NSSImager::IImageTransform::createInstance();
        if( !pTrans ) {
            MY_LOGE("!pTrans");
            return UNKNOWN_ERROR;
        }
        //
        success =
            pTrans->execute(
                pSrc,
                vDst[0],
                (vDst.size() > 1 )? vDst[1] : 0L,
                vCrop[0],
                (vCrop.size() > 1 )? vCrop[1] : 0L,
                vTransform[0],
                (vTransform.size() > 1 )? vTransform[1] : 0L,
                0xFFFFFFFF
                );

        pTrans->destroyInstance();
        pTrans = NULL;
        #endif
    }
    //
    FUNC_END;
    return success ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MDPProcedure::
onFinish(
    FrameParams const& params,
    MBOOL const success
)
{
    CAM_TRACE_NAME("P2F:[MDP]Finish");
    //params.in.mHandle->updateState(BufferHandle::Buffer_ReadDone);
    for( size_t i = 0; i < params.vOut.size(); i++ )
        if (params.vOut[i].mHandle.get())
            params.vOut[i].mHandle->updateState(
                    success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
                    );
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
onExtractParams(Request* pRequest, FrameParams& param_p2)
{
    // input buffer
    {
        StreamId_T const streamId = pRequest->context.in_buffer->getStreamId();
        param_p2.in.mPortId = PORT_IMGI;
        param_p2.in.mHandle = pRequest->context.in_buffer;
        param_p2.in_lcso.mPortId = PORT_LCEI;
        param_p2.in_lcso.mHandle = pRequest->context.in_lcso_buffer;
        param_p2.in_rsso.mHandle = pRequest->context.in_rsso_buffer;
        param_p2.bResized = pRequest->context.resized;
        #ifndef FEATURE_MODIFY
        pRequest->context.in_buffer.clear();
        pRequest->context.in_lcso_buffer.clear();
        pRequest->context.in_rsso_buffer.clear();
        #endif // FEATURE_MODIFY
    }

    // output buffer
    MUINT32 occupied = 0;
    MBOOL remains = MFALSE;
    vector<sp<BufferHandle> >::iterator iter = pRequest->context.out_buffers.begin();
    for (; iter !=  pRequest->context.out_buffers.end(); iter++)
    {
        sp<BufferHandle> pOutBuffer = *iter;
        if(!pOutBuffer.get())
            continue;

        StreamId_T const streamId = pOutBuffer->getStreamId();
        MUINT32 const transform = pOutBuffer->getTransform();
        MUINT32 const usage = pOutBuffer->getUsage();

        PortID port_p2;
        MBOOL isFdStream = streamId == pRequest->context.fd_stream_id;
        if (OK == mapPortId(streamId, transform, isFdStream, occupied, port_p2))
        {
            P2Procedure::FrameOutput out;
            out.mPortId = port_p2;
            out.mHandle = pOutBuffer;
            out.mTransform = transform;
            out.mUsage = usage;

            param_p2.vOut.push_back(out);
            #ifdef FEATURE_MODIFY
            if ( !(out.mUsage & GRALLOC_USAGE_HW_COMPOSER) )
            {
                (*iter).clear();
            }
            #else
            (*iter).clear();
            #endif // FEATURE_MODIFY
        }
        else
            remains = MTRUE;
    }

    if (remains && param_p2.vOut.size() > 0)
    {
        for (size_t i = 0, n = param_p2.vOut.size(); i < n; ++i)
        {
            if (param_p2.vOut[i].mUsage & GRALLOC_USAGE_HW_COMPOSER)
            {
                pRequest->context.in_mdp_buffer = param_p2.vOut[i].mHandle;
                break;
            }
        }
    }

    param_p2.inApp = pRequest->context.in_app_meta;
    param_p2.inHal = pRequest->context.in_hal_meta;
    param_p2.outApp = pRequest->context.out_app_meta;
    param_p2.outHal = pRequest->context.out_hal_meta;

    pRequest->context.in_app_meta.clear();
    pRequest->context.in_hal_meta.clear();
    // out meta will be clear later
    // pRequest->context.out_app_meta.clear();
    // pRequest->context.out_hal_meta.clear();

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MDPProcedure::
onExtractParams(Request* pRequest, FrameParams& param_mdp)
{
    if (!pRequest->context.in_mdp_buffer.get())
    {
        return NOT_ENOUGH_DATA;
    }
    param_mdp.in.mHandle = pRequest->context.in_mdp_buffer;
    pRequest->context.in_mdp_buffer.clear();

    // input&output buffer
    vector<sp<BufferHandle> >::iterator iter = pRequest->context.out_buffers.begin();
    while(iter !=  pRequest->context.out_buffers.end())
    {
        sp<BufferHandle> pOutBuffer = *iter;
        if (pOutBuffer.get() && pOutBuffer->getState() == BufferHandle::STATE_NOT_USED)
        {
            MDPProcedure::FrameOutput out;
            out.mHandle = pOutBuffer;
            out.mTransform = pOutBuffer->getTransform();
            param_mdp.vOut.push_back(out);
            (*iter).clear();
        }
        iter++;
    }
    return (param_mdp.vOut.size() > 0) ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::MultiFrameHandler::
collect(sp<Request> /*pRequest*/, FeaturePipeParam& /*featureParams*/)
{
/*
#ifdef FEATURE_MODIFY
    QParams params = featureParams.getQParams();
#endif
    MINT32 index = mvReqCollecting.size();
    // tag
    for( size_t i = 0; i < params.mvStreamTag.size(); i++ )
    {
        mParamCollecting.mvStreamTag.push_back(params.mvStreamTag[i]);
    }
    // truning
    for( size_t i = 0; i < params.mvTuningData.size(); i++ )
    {
        mParamCollecting.mvTuningData.push_back(params.mvTuningData[i]);
    }
    // input
    for( size_t i = 0; i < params.mvIn.size(); i++ )
    {
        params.mvIn.editItemAt(i).mPortID.group = index;
        mParamCollecting.mvIn.push_back(params.mvIn[i]);
    }
    // output
    for( size_t i = 0; i < params.mvOut.size(); i++ )
    {
        params.mvOut.editItemAt(i).mPortID.group = index;
        mParamCollecting.mvOut.push_back(params.mvOut[i]);
    }
    // crop
    for( size_t i = 0; i < params.mvCropRsInfo.size(); i++ )
    {
        params.mvCropRsInfo.editItemAt(i).mFrameGroup = index;
        mParamCollecting.mvCropRsInfo.push_back(params.mvCropRsInfo[i]);
    }
    // module
    for( size_t i = 0; i < params.mvModuleData.size(); i++ )
    {
        params.mvModuleData.editItemAt(i).frameGroup = index;
        mParamCollecting.mvModuleData.push_back(params.mvModuleData[i]);
    }

    mvReqCollecting.push_back(pRequest);
    if(mvReqCollecting.size() >= pRequest->context.burst_num)
    {
        QParams enqueParams = mParamCollecting;
        // callback
        enqueParams.mpfnCallback = callback;
        enqueParams.mpCookie     = this;

        MY_LOGD_IF(mbEnableLog, "[burst] cnt %d, in %d, out %d",
                muMfEnqueCnt, enqueParams.mvIn.size(), enqueParams.mvOut.size() );
        muMfEnqueCnt++;
        {
#ifdef FEATURE_MODIFY
            featureParams.setQParams(enqueParams);
#endif // FEATURE_MODIFY
            MY_LOGD("[burst] Pass2 enque");
            CAM_TRACE_NAME("[burst] drv_enq");
            if (mpPipe->enque(featureParams))
            {
                Mutex::Autolock _l(mLock);
                MY_LOGD("[burst] Pass2 enque success");
                mvRunning.push_back(mvReqCollecting);
            }
            else
            {
                MY_LOGE("[burst] Pass2 enque failed");
                Mutex::Autolock _l(mLock);
                vector<sp<Request> >::iterator iter = mvReqCollecting.begin();
                while (iter != mvReqCollecting.end())
                {
                    (*iter)->responseDone(UNKNOWN_ERROR);
                    iter++;
                }

                MY_LOGW("[burst] cnt %d execute failed", muMfDequeCnt);
                muMfDequeCnt++;
            }
        }
        // clear the collected request
        mParamCollecting.mvStreamTag.clear();
        mParamCollecting.mvTuningData.clear();
        mParamCollecting.mvIn.clear();
        mParamCollecting.mvOut.clear();
        mParamCollecting.mvCropRsInfo.clear();
        mParamCollecting.mvModuleData.clear();
        mvReqCollecting.clear();
    }
    */
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::MultiFrameHandler::
handleCallback(QParams& /*rParams*/)
{
    CAM_TRACE_NAME("[burst] P2 deque");
    Mutex::Autolock _l(mLock);
    /*
    vector<sp<Request> > vpRequest;
    {
        MY_LOGD_IF(mbEnableLog, "[burst] p2 done %d, success %d", muMfDequeCnt, rParams.mDequeSuccess);
        vpRequest = mvRunning.front();
        mvRunning.erase(mvRunning.begin());
        muMfDequeCnt++;
        MY_LOGD_IF(mbEnableLog, "[burst] p2 done muDequeCnt:%d", muMfDequeCnt);
    }
    //
    for (size_t i = 0; i < rParams.mvTuningData.size(); i++ )
    {
        void* pTuning = rParams.mvTuningData[i];
        if( pTuning ) {
            free(pTuning);
        }
    }
    //
    vector<sp<Request> >::iterator iter = vpRequest.begin();
    while (iter != vpRequest.end())
    {
        (*iter)->responseDone(rParams.mDequeSuccess ? OK : UNKNOWN_ERROR);
        iter++;
    }
    */
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::MultiFrameHandler::
flush()
{
    FUNC_START;

    mParamCollecting = QParams();
    //mvReqCollecting.clear();
    vector<sp<Request> >::iterator iter = mvReqCollecting.begin();
    while (iter != mvReqCollecting.end())
    {
        (*iter)->responseDone(UNKNOWN_ERROR);
        iter = mvReqCollecting.erase(iter);
    }

    FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
onFlush()
{
    CAM_TRACE_NAME("P2F:[Proc]Flush");
#ifdef FEATURE_MODIFY
    if(mpPipe)
    {
        mpPipe->flush();
    }
    this->flushBufferLifeHolder();
#endif // FEATURE_MODIFY
    if(mpMultiFrameHandler)
    {
        mpMultiFrameHandler->flush();
    }
    return;
}

MVOID
P2Procedure::
onSync()
{
    CAM_TRACE_NAME("P2F:[Proc]Sync");
    Mutex::Autolock _l(mLock);

    while( !mvRunning.empty() )
    {
        mCondJob.wait(mLock);
    }
}

#ifdef FEATURE_MODIFY

MBOOL P2Procedure::isCRZApplied(IMetadata* const /*inApp*/, IMetadata* /*pMeta_InHal*/, const QParams& /*rParams*/, const MUINT32 /*featureMask*/)
{
    // Get EIS mode
    IStreamingFeaturePipe::UsageHint usageHint = getPipeUsageHint(mInitParams.usageHint);
    MINT32 eisMode = usageHint.mEISInfo.mode;

#ifdef CRZ_NR3D_REPLACED_BY_TILE_SRC_CROP
    if (mInitParams.usageHint.mDualMode)
    {
        MY_LOGD_IF(mbEnableLog, "mkdbg: dualMode: no CRZ needed: %d", mInitParams.usageHint.mDualMode);
        return MFALSE;
    }
    else
    {
        if( EIS_MODE_IS_EIS_12_ENABLED(eisMode) && is4K2K(usageHint.mStreamingSize) )
        {
            // 4k2k + eisv1.2
            MY_LOGD_IF(mbEnableLog, "mkdbg: CRZ used");
            return MTRUE;
        }
    }
    MY_LOGD_IF(mbEnableLog, "mkdbg: no CRZ needed");
    return MFALSE;

#else // ori: CRZ_NR3D
    MSize maxSize = MSize(0, 0);
    MUINT32 max = 0;
    for( unsigned i = 0, n = rParams.mvFrameParams[0].mvOut.size(); i < n; ++i )
    {
        MSize size = rParams.mvFrameParams[0].mvOut[i].mBuffer->getImgSize();
        MUINT32 temp = size.w * size.h;
        if( temp > max )
        {
            maxSize = size;
            max = temp;
        }
    }
    // 4k2k rec + EIS1.2
    if( is4K2K(maxSize) && EIS_MODE_IS_EIS_12_ENABLED(eisMode) )
    {
        return MTRUE;
    }
    return MFALSE;
#endif // CRZ_NR3D_REPLACED_BY_TILE_SRC_CROP
}
#endif // FEATURE_MODIFY

MINT32
P2Procedure::
get3DNRIsoThreshold(MUINT8 ispProfile)
{
    MINT32 isoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(ispProfile, mUtil3dnr->is3dnrDebugMode());
    MY_LOGD_IF(mbEnableLog,"Get isoThreshold : %d",isoThreshold);
    return isoThreshold;
}

} // namespace P2Feature
