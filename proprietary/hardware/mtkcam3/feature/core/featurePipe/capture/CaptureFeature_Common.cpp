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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

// Standard C header file
#include <mutex>
#include <algorithm>
#include <sstream>
#include <chrono>
#include <iomanip>
#include <dlfcn.h>
// Android system/core header file

// mtkcam custom header file

// mtkcam global header file
#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>
#include <power/include/mtkperf_resource.h>

// Module header file
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
// eis
#include <camera_custom_eis.h>
#include <mtkcam3/feature/eis/EisInfo.h>
// nvram tuning
#include <camera_custom_nvram.h>
#include <mtkcam/aaa/INvBufUtil.h>
#if MTK_CAM_NEW_NVRAM_SUPPORT
#   include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
//
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
//
#include <featurePipe/core/include/DebugUtil.h>
//
// isp tuning
#include <isp_tuning/isp_tuning.h>
//
// stereo
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
//
// Local header file
#include "CaptureFeatureRequest.h"
#include "CaptureFeature_Common.h"

// Logging
#include "DebugControl.h"
#define PIPE_CLASS_TAG "Util"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_COMMON
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_CAPTURE);

#define ENABLE_DUAL_FOV_CROP MTRUE

/*******************************************************************************
* MACRO Utilities Define.
********************************************************************************/
namespace { // anonymous namespace for MACRO function

using AutoObject = NSCam::NSCamFeature::NSFeaturePipe::NSCapture::UniquePtr<const char>;
//
auto
createAutoScoper(const char* funcName) -> AutoObject
{
    CAM_ULOGMD("[%s] +", funcName);
    return AutoObject(funcName, [](const char* p)
    {
        CAM_ULOGMD("[%s] -", p);
    });
}
#define SCOPED_TRACER() auto scoped_tracer = ::createAutoScoper(__FUNCTION__)
//
auto
createAutoTimer(const char* funcName, const char* fmt, ...) -> AutoObject
{
    using Timing = std::chrono::time_point<std::chrono::high_resolution_clock>;
    using DuationTime = std::chrono::duration<float, std::milli>;
    //
    static const MINT32 LENGTH = 512;

    char* pBuf = new char[LENGTH];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(pBuf, LENGTH, fmt, ap);
    va_end(ap);

    Timing startTime = std::chrono::high_resolution_clock::now();
    return AutoObject(pBuf, [funcName, startTime](const char* p)
    {
        Timing endTime = std::chrono::high_resolution_clock::now();
        DuationTime duationTime = endTime - startTime;
        CAM_ULOGMD("[%s] %s, elapsed(ms):%.4f", funcName, p, duationTime.count());
        delete[] p;
    });
}
#define AUTO_TIMER(FMT, arg...) auto auto_timer = ::createAutoTimer(__FUNCTION__, FMT, ##arg);
//
#define UNREFERENCED_PARAMETER(param) (param)
//
} // end anonymous namespace for MACRO function


/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {
namespace NSCapture {


/*******************************************************************************
* Alias.
********************************************************************************/
using namespace vendor::mediatek::hardware::power::V2_0;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NSCam::NSIoPipe;

/*******************************************************************************
* Global Function.
*******************************************************************************/

MBOOL   dumpToFile(IImageBuffer *buffer, const char *fmt, ...);

MUINT32 align(MUINT32 val, MUINT32 bits);

MUINT   getSensorRawFmt(MINT32 sensorId);


/*******************************************************************************
* Class Define
*******************************************************************************/
/**
 * @brief slim wrapper of Powerhal, and it is a singleton
 */
class PowerHalWrapper final
{
public:
    using Handle      = MINT32;

    static inline PowerHalWrapper* getInstance();

public:
    ~PowerHalWrapper();

public:
    Handle createHandle(const std::string& userName);

    MVOID destroyhandle(const std::string& userName, Handle handle);

    MVOID enableBoost(const std::string& userName, Handle handle);

    MVOID disableBoost(const std::string& userName, Handle handle);

private:
    PowerHalWrapper();

private:
    using Handles     = std::vector<Handle>;
    using HandlesItor = Handles::iterator;

private:
    MVOID initLock();

    MVOID uninitLock();

    MVOID dumpLock();

    inline MBOOL getIsValidLock(Handle handle);

    inline MBOOL getIsValidLock(Handle handle, HandlesItor& itor);

private:
    std::mutex           sLocker;
    sp<IPower>           sPowerPtr;
    std::vector<Handle>  sHandles;
    //
    static constexpr MINT32      INVALID_HANDLE = -1;
    static constexpr MINT        TIMEOUT        = (10*1000);
};
/**
 * @brief tool class for performance client vendor
 */
class PerfClientVendor final {
public:
    static PerfClientVendor* getInstance();

    MBOOL isPerfLibEnable();
    int enablePerf(const std::string& userName, int handle);
    int disablePerf(const std::string& userName, int handle);

public:
    ~PerfClientVendor();
private:
    PerfClientVendor();

private:
    MBOOL loadPerfAPI();
    MVOID initPerfLib();
    MVOID closePerfLib();

private:
    using perfLockAcqFunc = int (*)(int, int, int[], int);
    using perfLockRelFunc = int (*)(int);

    /* function pointer to perfserv client */
    perfLockAcqFunc perfLockAcq = NULL;
    perfLockRelFunc perfLockRel = NULL;

    void *libHandle = NULL;
    MBOOL libEnabled = MFALSE;

    const char *perfLib = "libmtkperf_client_vendor.so";
};
/**
 * @brief empty boost for dufaule use
 */
class EmptyBoost : public IBooster
{
public:
    EmptyBoost(const std::string& name);

    ~EmptyBoost();

public:
    const std::string& getName() override;

public:
    MVOID enable() override;

    MVOID disable() override;

private:
    const std::string       mName;
};
/**
 * @brief boost implementation of CUP
 */
class CPUBooster : public IBooster
{
public:
    CPUBooster(const std::string& name);

    ~CPUBooster();

public:
    const std::string& getName() override;

public:
    MVOID enable() override;

    MVOID disable() override;

private:
    const std::string       mName;
    PowerHalWrapper::Handle mHandle;
};
/**
 * @brief boost implementation of PerfBooster
 */
class PerfBooster : public IBooster
{
public:
    PerfBooster(const std::string& name);

    ~PerfBooster();

public:
    const std::string& getName() override;

public:
    MVOID enable() override;

    MVOID disable() override;

private:
    const std::string   mName;
    int                 mHandle;
};

/**
 * @brief buffer resizer implementation
 */
class BufResizer final : public IHolder
{
public:
    BufResizer(IImageBuffer* pBuf, const MSize& size)
        : mpBuf(pBuf)
    {
        mOriSize = mpBuf->getImgSize();
        mpBuf->setExtParam(size);
        MY_LOGD("ctor:%p change output size from (%d, %d) to (%d, %d)",
                this, mOriSize.w, mOriSize.h, mpBuf->getImgSize().w, mpBuf->getImgSize().h);
    };

    ~BufResizer()
    {
        MY_LOGD("dtor:%p reset output size from (%d, %d) to (%d, %d)",
                this, mpBuf->getImgSize().w, mpBuf->getImgSize().h, mOriSize.w, mOriSize.h);
        mpBuf->setExtParam(mOriSize);
    };

private:
    IImageBuffer* mpBuf     = NULL;
    MSize         mOriSize  = MSize(0,0);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Global Function Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
formatConverter(MINT32 sensorIdx, const IImageBuffer *pSrcImg, IImageBuffer *pDstImg)
{
    SCOPED_TRACER();

    AUTO_TIMER("format transfer, srcSize:%dx%d, srcFmt:%s, dstSize:%dx%d, dstFmt:%s",
        pSrcImg->getImgSize().w,  pSrcImg->getImgSize().h, ImgFmt2Name(pSrcImg->getImgFormat()),
        pDstImg->getImgSize().w,  pDstImg->getImgSize().h, ImgFmt2Name(pDstImg->getImgFormat()));

    using NSIoPipe::NSSImager::ISImager;
    auto pSImager = UniquePtr<ISImager>(ISImager::createInstance(pSrcImg, sensorIdx), [](ISImager* p)
    {
        if(p != nullptr)
            p->destroyInstance();
    });

    if(pSImager == nullptr) {
        MY_LOGE("ISImager::createInstance() failed!!!");
        return MFALSE;
    }

    if (!pSImager->setTargetImgBuffer(pDstImg)) {
        MY_LOGE("setTargetImgBuffer failed!!!");
        return MFALSE;
    }
    struct timeval current;
    gettimeofday(&current, NULL);
    if (!pSImager->execute(&current)) {
        MY_LOGE("execute failed!!!");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL
dumpToFile(IImageBuffer *buffer, const char *fmt, ...)
{
    MBOOL ret = MFALSE;
    if( buffer && fmt )
    {
        char filename[256];
        va_list arg;
        va_start(arg, fmt);
        vsprintf(filename, fmt, arg);
        va_end(arg);
        buffer->saveToFile(filename);
        ret = MTRUE;
    }
    return ret;
}

MUINT32
align(MUINT32 val, MUINT32 bits)
{
    // example: align 5 bits => align 32
    MUINT32 mask = (0x01 << bits) - 1;
    return (val + mask) & (~mask);
}

MUINT
getSensorRawFmt(MINT32 sensorId)
{
    MUINT ret = SENSOR_RAW_FMT_NONE;
    IHalSensorList *sensorList = MAKE_HalSensorList();
    if (NULL == sensorList) {
        MY_LOGE("cannot get sensor list");
        return ret;
    }

    int32_t sensorCount = sensorList->queryNumberOfSensors();
    if(sensorId >= sensorCount) {
        MY_LOGW("sensor index should be lower than %d", sensorCount);
        return ret;
    }

    SensorStaticInfo sensorStaticInfo;
    memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
    MINT32 sendorDevIndex = sensorList->querySensorDevIdx(sensorId);
    sensorList->querySensorStaticInfo(sendorDevIndex, &sensorStaticInfo);

    ret = sensorStaticInfo.rawFmtType;
    MY_LOGD("get sensor raw Fmt, sensorId:%d, sendorIndex:%d, rawFmt:%u", sensorId, sendorDevIndex, ret);
    return ret;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Global Function and Variable Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static const NodeID_T sPathMap[NUM_OF_PATH][2] =
{
    {NID_ROOT,      NID_ROOT},          // PID_ENQUE
    {NID_ROOT,      NID_RAW},           // PID_ROOT_TO_RAW
    {NID_ROOT,      NID_P2A},           // PID_ROOT_TO_P2A
    {NID_ROOT,      NID_P2B},           // PID_ROOT_TO_P2B
    {NID_ROOT,      NID_MULTIRAW},      // PID_ROOT_TO_MULTIRAW
    {NID_RAW,       NID_P2A},           // PID_RAW_TO_P2A
    {NID_MULTIRAW,  NID_P2A},           // PID_MULTIRAW_TO_P2A
    {NID_P2A,       NID_DEPTH},         // PID_P2A_TO_DEPTH
    {NID_P2A,       NID_FUSION},        // PID_P2A_TO_FUSION
    {NID_P2A,       NID_MULTIYUV},      // PID_P2A_TO_MULTIYUV
    {NID_P2A,       NID_YUV},           // PID_P2A_TO_YUV
    {NID_P2A,       NID_YUV2},          // PID_P2A_TO_YUV2
    {NID_P2A,       NID_MDP},           // PID_P2A_TO_MDP
    {NID_P2A,       NID_FD} ,           // PID_P2A_TO_FD
    {NID_P2B,       NID_YUV},           // PID_P2B_TO_YUV
    {NID_P2B,       NID_P2A},           // PID_P2B_TO_P2A
    {NID_FD,        NID_DEPTH},         // PID_FD_TO_DEPTH
    {NID_FD,        NID_FUSION},        // PID_FD_TO_FUSION
    {NID_FD,        NID_MULTIYUV},      // PID_FD_TO_MULTIFRAME
    {NID_FD,        NID_YUV},           // PID_FD_TO_YUV
    {NID_FD,        NID_YUV2},          // PID_FD_TO_YUV2
    {NID_MULTIYUV,  NID_YUV},           // PID_MULTIFRAME_TO_YUV
    {NID_MULTIYUV,  NID_YUV2},          // PID_MULTIFRAME_TO_YUV2
    {NID_MULTIYUV,  NID_MDP_C},         // PID_MULTIFRAME_TO_MDP_C
    {NID_MULTIYUV,  NID_BOKEH},         // PID_MULTIFRAME_TO_BOKEH
    {NID_MULTIYUV,  NID_MDP},           // PID_MULTIFRAME_TO_MDP
    {NID_FUSION,    NID_YUV},           // PID_FUSION_TO_YUV
    {NID_FUSION,    NID_YUV2},          // PID_FUSION_TO_YUV2
    {NID_FUSION,    NID_MDP},           // PID_FUSION_TO_MDP
    {NID_DEPTH,     NID_BOKEH},         // PID_DEPTH_TO_BOKEH
    {NID_DEPTH,     NID_YUV2},          // PID_DEPTH_TO_YUV2
    {NID_YUV,       NID_MDP_C},         // PID_YUV_TO_MDP_C
    {NID_YUV,       NID_BOKEH},         // PID_YUV_TO_BOKEH
    {NID_YUV,       NID_YUV2},          // PID_YUV_TO_YUV2
    {NID_YUV,       NID_MDP},           // PID_YUV_TO_MDP
    {NID_BOKEH,     NID_MDP_B},         // PID_BOKEH_TO_MDP_B
    {NID_BOKEH,     NID_YUV2},          // PID_BOKEH_TO_YUV2
    {NID_BOKEH,     NID_MDP},           // PID_BOKEH_TO_MDP
    {NID_YUV2,      NID_MDP},           // PID_YUV2_TO_MDP
};

const char*
PathID2Name(PathID_T pid)
{
    switch(pid)
    {
    case PID_ENQUE:                 return "enque";
    case PID_ROOT_TO_RAW:           return "root_to_raw";
    case PID_ROOT_TO_P2A:           return "root_to_p2a";
    case PID_ROOT_TO_P2B:           return "root_to_p2b";
    case PID_ROOT_TO_MULTIRAW:      return "root_to_multiraw";
    case PID_RAW_TO_P2A:            return "raw_to_p2a";
    case PID_MULTIRAW_TO_P2A:       return "multiraw_to_p2a";
    case PID_P2A_TO_DEPTH:          return "p2a_to_depth";
    case PID_P2A_TO_FUSION:         return "p2a_to_fusion";
    case PID_P2A_TO_MULTIYUV:       return "p2a_to_multiyuv";
    case PID_P2A_TO_YUV:            return "p2a_to_yuv";
    case PID_P2A_TO_YUV2:           return "p2a_to_yuv2";
    case PID_P2A_TO_MDP:            return "p2a_to_mdp";
    case PID_P2A_TO_FD:             return "p2a_to_fd";
    case PID_P2B_TO_YUV:            return "p2b_to_yuv";
    case PID_P2B_TO_P2A:            return "p2b_to_p2a";
    case PID_FD_TO_DEPTH:           return "fd_to_depth";
    case PID_FD_TO_FUSION:          return "fd_to_fusion";
    case PID_FD_TO_MULTIFRAME:      return "fd_to_multiframe";
    case PID_FD_TO_YUV:             return "fd_to_yuv";
    case PID_FD_TO_YUV2:            return "fd_to_yuv2";
    case PID_MULTIFRAME_TO_YUV:     return "multiframe_to_yuv";
    case PID_MULTIFRAME_TO_YUV2:    return "multiframe_to_yuv2";
    case PID_MULTIFRAME_TO_MDP_C:   return "multiframe_to_mdpc";
    case PID_MULTIFRAME_TO_BOKEH:   return "multiframe_to_bokeh";
    case PID_MULTIFRAME_TO_MDP:     return "multiframe_to_mdp";
    case PID_FUSION_TO_YUV:         return "fusion_to_yuv";
    case PID_FUSION_TO_YUV2:        return "fusion_to_yuv2";
    case PID_FUSION_TO_MDP:         return "fusion_to_mdp";
    case PID_DEPTH_TO_BOKEH:        return "depth_to_bokeh";
    case PID_DEPTH_TO_YUV2:         return "depth_to_yuv2";
    case PID_YUV_TO_MDP_C:          return "yuv_to_mdpc";
    case PID_YUV_TO_BOKEH:          return "yuv_to_bokeh";
    case PID_YUV_TO_YUV2:           return "yuv_to_yuv2";
    case PID_YUV_TO_MDP:            return "yuv_to_mdp";
    case PID_BOKEH_TO_MDP_B:        return "bokeh_to_mdpb";
    case PID_BOKEH_TO_YUV2:         return "bokeh_to_yuv2";
    case PID_BOKEH_TO_MDP:          return "bokeh_to_mdp";
    case PID_YUV2_TO_MDP:           return "yuv2_to_mdp";
    case PID_DEQUE:                 return "deque";

    default:                        return "unknown";
    };
}

const char*
NodeID2Name(NodeID_T nid)
{
    switch(nid)
    {
    case NID_ROOT:                  return "root";
    case NID_RAW:                   return "raw";
    case NID_MULTIRAW:              return "multiraw";
    case NID_P2A:                   return "p2a";
    case NID_P2B:                   return "p2b";
    case NID_FD:                    return "fd";
    case NID_MULTIYUV:              return "multiyuv";
    case NID_FUSION:                return "fusion";
    case NID_DEPTH:                 return "depth";
    case NID_YUV:                   return "yuv";
    case NID_YUV_R1:                return "yuv_r1";
    case NID_YUV_R2:                return "yuv_r2";
    case NID_YUV_R3:                return "yuv_r3";
    case NID_YUV_R4:                return "yuv_r4";
    case NID_YUV_R5:                return "yuv_r5";
    case NID_YUV2:                  return "yuv2";
    case NID_YUV2_R1:               return "yuv2_r1";
    case NID_YUV2_R2:               return "yuv2_r2";
    case NID_YUV2_R3:               return "yuv2_r3";
    case NID_YUV2_R4:               return "yuv2_r4";
    case NID_YUV2_R5:               return "yuv2_r5";
    case NID_BOKEH:                 return "bokeh";
    case NID_MDP:                   return "mdp";
    case NID_MDP_B:                 return "mdp_b";
    case NID_MDP_C:                 return "mdp_c";

    default:                        return "unknown";
    };
}

const char*
TypeID2Name(TypeID_T tid)
{
    switch(tid)
    {
    case TID_MAN_FULL_RAW:          return "man_full_raw";
    case TID_MAN_FULL_YUV:          return "man_full_yuv";
    case TID_MAN_FULL_PURE_YUV:     return "man_full_pure_yuv";
    case TID_MAN_RSZ_RAW:           return "man_rsz_raw";
    case TID_MAN_RSZ_YUV:           return "man_rsz_yuv";
    case TID_MAN_CROP1_YUV:         return "man_crop1_yuv";
    case TID_MAN_CROP2_YUV:         return "man_crop2_yuv";
    case TID_MAN_CROP3_YUV:         return "man_crop3_yuv";
    case TID_MAN_SPEC_YUV:          return "man_spec_yuv";
    case TID_MAN_DEPTH:             return "man_depth";
    case TID_MAN_LCS:               return "man_lcs";
    case TID_MAN_DCES:              return "man_dces";
    case TID_MAN_FD_YUV:            return "man_fd_yuv";
    case TID_MAN_FD:                return "man_fd";
    case TID_SUB_FULL_RAW:          return "sub_full_raw";
    case TID_SUB_FULL_YUV:          return "sub_full_yuv";
    case TID_SUB_RSZ_RAW:           return "sub_rsz_raw";
    case TID_SUB_RSZ_YUV:           return "sub_rsz_yuv";
    case TID_SUB_LCS:               return "sub_lcs";
    case TID_POSTVIEW:              return "postview";
    case TID_JPEG:                  return "jpeg";
    case TID_THUMBNAIL:             return "thumbnail";
    case TID_MAN_CLEAN:             return "clean";
    case TID_MAN_BOKEH:             return "bokeh";

    case NULL_TYPE:                 return "";

    default:                        return "unknown";
    };
}

const char* FeatID2Name(FeatureID_T fid)
{
    switch(fid)
    {
    case FID_REMOSAIC:              return "remosaic";
    case FID_ABF:                   return "abf";
    case FID_NR:                    return "nr";
    case FID_AINR:                  return "ainr";
    case FID_MFNR:                  return "mfnr";
    case FID_FB:                    return "fb";
    case FID_HDR:                   return "hdr";
    case FID_DEPTH:                 return "depth";
    case FID_BOKEH:                 return "bokeh";
    case FID_FUSION:                return "fusion";
    case FID_CZ:                    return "cz";
    case FID_DRE:                   return "dre";
    case FID_HFG:                   return "hfg";
    case FID_DCE:                   return "dce";
    case FID_DSDN:                  return "dsdn";
    case FID_FB_3RD_PARTY:          return "fb_3rd_party";
    case FID_HDR_3RD_PARTY:         return "hdr_3rd_party";
    case FID_HDR2_3RD_PARTY:        return "hdr2_3rd_party";
    case FID_MFNR_3RD_PARTY:        return "mfnr_3rd_party";
    case FID_BOKEH_3RD_PARTY:       return "bokeh_3rd_party";
    case FID_DEPTH_3RD_PARTY:       return "depth_3rd_party";
    case FID_FUSION_3RD_PARTY:      return "fusion_3rd_party";
    case FID_PUREBOKEH_3RD_PARTY:   return "purebokeh_3rd_party";
    case FID_AINR_YUV:              return "ainr yuv";
    case FID_RELIGHTING_3RD_PARTY:  return "relighting_3rd_party";
    case FID_AINR_YHDR:             return "ainr_yhdr";
    case FID_AIHDR:                 return "aihdr";

    default:                        return "unknown";
    };
}

const char*
SizeID2Name(MUINT8 sizeId)
{
    switch(sizeId)
    {
    case SID_FULL:                  return "full";
    case SID_RESIZED:               return "resized";
    case SID_QUARTER:               return "quarter";
    case SID_ARBITRARY:             return "arbitrary";
    case SID_SPECIFIED:             return "specified";
    case NULL_SIZE:                 return "";

    default:                        return "unknown";
    };
}

PathID_T
FindPath(NodeID_T src, NodeID_T dst)
{
    for (PathID_T pid = PID_ENQUE + 1; pid < NUM_OF_PATH; pid++) {
        if (sPathMap[pid][0] == src && sPathMap[pid][1] == dst) {
            return pid;
        }
    }
    return NULL_PATH;
}

MBOOL GetPath(PathID_T pid, NodeID_T& src, NodeID_T& dst)
{
    if (pid >= NUM_OF_PATH)
        return MFALSE;

    src = sPathMap[pid][0];
    dst = sPathMap[pid][1];

    return MTRUE;
}

const void*
getTuningFromNvram(
    MUINT32 openId,
    MUINT32& idx,
    MINT32 magicNo,
    MINT32 type,
    const IMetadata* pInHalMeta,
    MBOOL enableLog,
    MBOOL isISPHIDL,
    MINT32 useIspProfile
)
{
#if MTK_CAM_NEW_NVRAM_SUPPORT
    CAM_TRACE_BEGIN("getNvram");
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    const void *pNRNvram = nullptr;

    if (idx >= EISO_NUM) {
        CAM_ULOGME("wrong nvram idx %d", idx);
        return NULL;
    }

    // load some setting from nvram
    MUINT sensorDev = MAKE_HalSensorList()->querySensorDevIdx(openId);

    auto pNvBufUtil = MAKE_NvBufUtil();
    if (pNvBufUtil == NULL) {
        CAM_ULOGME("pNvBufUtil==0");
        return NULL;
    }

    auto result = pNvBufUtil->getBufAndRead(
        CAMERA_NVRAM_DATA_FEATURE,
        sensorDev, (void*&)pNvram);
    if (result != 0) {
        CAM_ULOGME("read buffer chunk fail");
        return NULL;
    }

    IdxMgr* pMgr = IdxMgr::createInstance(static_cast<ESensorDev_T>(sensorDev));

    MBOOL bIspHidl = MFALSE;
    IMetadata::Memory mapInfo;
    CAM_IDX_QRY_COMB rMapping_Info;
    CAM_IDX_QRY_COMB *pMapInfo = nullptr;
    if (isISPHIDL){
        if(!tryGetMetadata<IMetadata::Memory>(pInHalMeta, MTK_ISP_ATMS_MAPPING_INFO, mapInfo))
        {
            MY_LOGE("Failed to get MTK_ISP_ATMS_MAPPING_INFO metadata in ISP HIDL device.");
            return nullptr;
        }
        else
        {
            pMapInfo = (CAM_IDX_QRY_COMB *)mapInfo.array();
        }
    } else {
        pMgr->getMappingInfo(static_cast<ESensorDev_T>(sensorDev), rMapping_Info, magicNo);
        pMapInfo = &rMapping_Info;
    }
    // overwrite the isp profile if needed
    if(useIspProfile != -1)
    {
        pMapInfo->eIspProfile = (EIspProfile_T)useIspProfile;
    }

    switch (type)
    {
        case NVRAM_TYPE_DRE:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_CA_LTM, *pMapInfo, __FUNCTION__);
            pNRNvram = &(pNvram->CA_LTM[idx]);
            break;
        }
        case NVRAM_TYPE_CZ:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_ClearZoom, *pMapInfo, __FUNCTION__);
            pNRNvram = &(pNvram->ClearZoom[idx]);
            break;
        }
        case NVRAM_TYPE_SWNR_THRES:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_SWNR_THRES, *pMapInfo, __FUNCTION__);
            pNRNvram = &(pNvram->SWNR_THRES[idx]);
            break;
        }
#ifdef SUPPORT_HFG
        case NVRAM_TYPE_HFG:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_HFG, *pMapInfo, __FUNCTION__);
            pNRNvram = &(pNvram->HFG[idx]);
            break;
        }
#endif
#ifdef SUPPORT_DSDN_20
        case NVRAM_TYPE_DSDN:
        {
            idx = pMgr->query(static_cast<ESensorDev_T>(sensorDev), NSIspTuning::EModule_DSDN, *pMapInfo, __FUNCTION__);
            MY_LOGD("DSDN: idx %d", idx);
            pNRNvram = &(pNvram->DSDN[idx]);
            break;
        }
#endif
        default:
        {
            CAM_ULOGME("not support nvram type: %d", type);
            break;
        }
    }

    CAM_LOGD_IF(enableLog, "[%s] query nvram type(%d) magic(%d) mappingInfo index: %d",__FUNCTION__, type, magicNo, idx);
    CAM_TRACE_END();
    return pNRNvram;
#else
    UNREFERENCED_PARAMETER(openId);
    UNREFERENCED_PARAMETER(idx);
    UNREFERENCED_PARAMETER(magicNo);
    UNREFERENCED_PARAMETER(type);
    UNREFERENCED_PARAMETER(enableLog);
    UNREFERENCED_PARAMETER(useIspProfile);
    return NULL;
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CropCalculator Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CropCalculator::
CropCalculator(const std::vector<MINT32> sensorList, MUINT32 uLogLevel, MUINT32 uDualMode)
    : mLogLevel(uLogLevel)
    , mDualMode(uDualMode)
{
    for(size_t i = 0; i < sensorList.size(); i++) {
        sp<Transformer> pTrans = new Transformer();
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(sensorList[i]);
        if (pMetadataProvider != NULL) {
            const IMetadata& mrStaticMetadata = pMetadataProvider->getMtkStaticCharacteristics();

            if (tryGetMetadata<MRect>(&mrStaticMetadata, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, pTrans->mActiveArray)) {
                MY_LOGD("Active Array(%d,%d)(%dx%d)",
                        pTrans->mActiveArray.p.x, pTrans->mActiveArray.p.y,
                        pTrans->mActiveArray.s.w, pTrans->mActiveArray.s.h);
                mTransformers[sensorList[i]] = pTrans;
            } else {
                MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            }
        } else {
            MY_LOGD("no metadata provider, sensor:%d", sensorList[i]);
        }
    }
}

sp<CropCalculator::Factor>
CropCalculator::
getFactor(const IMetadata* pInAppMeta, const IMetadata* pInHalMeta, const MINT32 sensorId)
{
    sp<Factor> pFactor = new Factor();
    Factor& rFactor = *pFactor.get();
    sp<Transformer> pTrans = mTransformers[sensorId];

    if (!tryGetMetadata<MSize>(pInHalMeta, MTK_HAL_REQUEST_SENSOR_SIZE, rFactor.mSensorSize)) {
        MY_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        return NULL;
    }

    const MSize& sensor = rFactor.mSensorSize;

    // 1. Get current p1 buffer crop status
    if (!(tryGetMetadata<MRect>(pInHalMeta, MTK_P1NODE_SCALAR_CROP_REGION, rFactor.mP1SensorCrop) &&
          tryGetMetadata<MSize>(pInHalMeta, MTK_P1NODE_RESIZER_SIZE, rFactor.mP1ResizerSize) &&
          tryGetMetadata<MRect>(pInHalMeta, MTK_P1NODE_DMA_CROP_REGION, rFactor.mP1DmaCrop)))
    {
        MY_LOGW_IF(mLogLevel, "[FIXME] should sync with p1 for rFactor setting");
        rFactor.mP1SensorCrop = MRect(MPoint(0, 0), sensor);
        rFactor.mP1ResizerSize = sensor;
        rFactor.mP1DmaCrop = MRect(MPoint(0, 0), sensor);
    }

    // 2. Get sensor mode
    if (!tryGetMetadata<MINT32>(pInHalMeta, MTK_P1NODE_SENSOR_MODE, rFactor.mSensorMode)) {
        MY_LOGE("cannot get MTK_P1NODE_SENSOR_MODE");
        return NULL;
    }

    // 3. Query crop region (in active array domain)
    // check which sensor is used
    MBOOL hasTGCrop = MFALSE;
    MRect& cropRegion = rFactor.mActiveCrop;
    MRect& cropRegionTG = rFactor.mSensorCrop;
    if (!tryGetMetadata<MRect>(pInAppMeta, MTK_SCALER_CROP_REGION, cropRegion)) {
        cropRegion.p = MPoint(0, 0);
        cropRegion.s = pTrans->mActiveArray.s;
        rFactor.mbExistActiveCrop = MFALSE;
        MY_LOGW("no MTK_SCALER_CROP_REGION: using full crop size %dx%d",
                cropRegion.s.w, cropRegion.s.h);
    }
    if (!tryGetMetadata<MRect>(pInHalMeta, MTK_DUALZOOM_CAP_CROP, cropRegionTG)) {
        MY_LOGD("cannot get MTK_DUALZOOM_CAP_CROP, using active crop");
        hasTGCrop = MFALSE;
    } else {
        MY_LOGD("use Hal meta TG crop");
        hasTGCrop = MTRUE;
    }

    // 4. Query EIS Mode
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    MINT32 advEisMode = MTK_EIS_FEATURE_EIS_MODE_OFF;
    if (!tryGetMetadata<MUINT8>(pInAppMeta, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode)) {
        MY_LOGW_IF(mLogLevel, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }
    if (!tryGetMetadata<MINT32>(pInAppMeta, MTK_EIS_FEATURE_EIS_MODE, advEisMode)) {
        MY_LOGW_IF(mLogLevel, "no MTK_EIS_FEATURE_EIS_MODE");
    }

    rFactor.mEnableEis = (eisMode    == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON ||
                          advEisMode == MTK_EIS_FEATURE_EIS_MODE_ON);

  { // enlarge mutex scope to avoid racing
    // 5. Use last rFactor if equal
    Mutex::Autolock _l(pTrans->mLock);
    if (pTrans->mpLastFactor != NULL && pTrans->mpLastFactor->isEqual(pFactor)) {
        return pTrans->mpLastFactor;
    } else {
        pTrans->mpLastFactor = pFactor;
    }

    // 6. Transform Matrix
    NSCamHW::HwTransHelper hwTransHelper(sensorId);
    if (!hwTransHelper.getMatrixToActive(rFactor.mSensorMode, rFactor.mSensor2Active) ||
        !hwTransHelper.getMatrixFromActive(rFactor.mSensorMode, rFactor.mActive2Sensor))
    {
        MY_LOGE("fail to get HW transform matrix!");
        return NULL;
    }

    rFactor.mSensor2Resizer = simpleTransform(
            rFactor.mP1SensorCrop.p,
            rFactor.mP1SensorCrop.s,
            rFactor.mP1ResizerSize);

    MRect& cropP2Region = rFactor.mP2RegionCrop;
    if (!tryGetMetadata<MRect>(pInHalMeta, MTK_P2NODE_CROP_REGION, cropP2Region)) {
        cropP2Region.p = MPoint(0, 0);
        cropP2Region.s = MSize(0, 0);
    }

    if (!hasTGCrop) {
        rFactor.mActive2Sensor.transform(rFactor.mActiveCrop, rFactor.mSensorCrop);

        MY_LOGD("Active:(%d,%d)(%dx%d) TG:(%d,%d)(%dx%d) Sensor:(%d,%d)(%dx%d) Resizer:(%dx%d) DMA:(%d,%d)(%dx%d) ValidRegion:(%d,%d)(%dx%d)",
            rFactor.mActiveCrop.p.x, rFactor.mActiveCrop.p.y,
            rFactor.mActiveCrop.s.w, rFactor.mActiveCrop.s.h,
            rFactor.mSensorCrop.p.x, rFactor.mSensorCrop.p.y,
            rFactor.mSensorCrop.s.w, rFactor.mSensorCrop.s.h,
            rFactor.mP1SensorCrop.p.x, rFactor.mP1SensorCrop.p.y,
            rFactor.mP1SensorCrop.s.w, rFactor.mP1SensorCrop.s.h,
            rFactor.mP1ResizerSize.w, rFactor.mP1ResizerSize.h,
            rFactor.mP1DmaCrop.p.x, rFactor.mP1DmaCrop.p.y,
            rFactor.mP1DmaCrop.s.w, rFactor.mP1DmaCrop.s.h,
            rFactor.mP2RegionCrop.p.x, rFactor.mP2RegionCrop.p.y,
            rFactor.mP2RegionCrop.s.w, rFactor.mP2RegionCrop.s.h);
    } else {
        MY_LOGD("TG:(%d,%d)(%dx%d) Sensor:(%d,%d)(%dx%d) Resizer:(%dx%d) DMA:(%d,%d)(%dx%d) ValidRegion:(%d,%d)(%dx%d)",
            rFactor.mSensorCrop.p.x, rFactor.mSensorCrop.p.y,
            rFactor.mSensorCrop.s.w, rFactor.mSensorCrop.s.h,
            rFactor.mP1SensorCrop.p.x, rFactor.mP1SensorCrop.p.y,
            rFactor.mP1SensorCrop.s.w, rFactor.mP1SensorCrop.s.h,
            rFactor.mP1ResizerSize.w, rFactor.mP1ResizerSize.h,
            rFactor.mP1DmaCrop.p.x, rFactor.mP1DmaCrop.p.y,
            rFactor.mP1DmaCrop.s.w, rFactor.mP1DmaCrop.s.h,
            rFactor.mP2RegionCrop.p.x, rFactor.mP2RegionCrop.p.y,
            rFactor.mP2RegionCrop.s.w, rFactor.mP2RegionCrop.s.h);
    }

    // 7. Query EIS
    {

        // EIS Info
        MINT64 iEisInfo = 0;
        MUINT32 uEisFactor = 0;
        MUINT32 uEisMode = 0;
        if (rFactor.mEnableEis) {
            if (!tryGetMetadata<MINT64>(pInHalMeta, MTK_EIS_INFO, iEisInfo)) {
                rFactor.mEnableEis = MFALSE;
                MY_LOGD("cannot get MTK_EIS_INFO, current EIS_INFO = %" PRIi64 " ", iEisInfo);
            } else {
                uEisFactor = EIS::EisInfo::getFactor(iEisInfo);
                uEisMode = EIS::EisInfo::getMode(iEisInfo);
                cropRegion.p.x += (cropRegion.s.w * (uEisFactor - 100) / 2 / uEisFactor);
                cropRegion.p.y += (cropRegion.s.h * (uEisFactor - 100) / 2 / uEisFactor);
                cropRegion.s = cropRegion.s * 100 / uEisFactor;
                MY_LOGD_IF(mLogLevel, "EIS: Factor %d, Mode 0x%x , Crop Region(%d, %d, %dx%d)",
                           uEisFactor, uEisMode,
                           cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
            }
        }

        // EIS Region
        eis_region eisRegion;
        if (rFactor.mEnableEis && queryEisRegion(pInHalMeta, eisRegion)) {
            // calculate mv
            vector_f& rSensorMv = rFactor.mSensorEisMv;
            vector_f& rResizerMv = rFactor.mResizerEisMv;
            MBOOL isResizedDomain = MTRUE;

#if SUPPORT_EIS_MV
            if (eisRegion.is_from_zzr)
            {
                rResizerMv.p.x  = eisRegion.x_mv_int;
                rResizerMv.pf.x = eisRegion.x_mv_float;
                rResizerMv.p.y  = eisRegion.y_mv_int;
                rResizerMv.pf.y = eisRegion.y_mv_float;
                rSensorMv = inv_transform(rFactor.mSensor2Resizer, rResizerMv);
            }
            else
            {
                isResizedDomain = MFALSE;
                rSensorMv.p.x  = eisRegion.x_mv_int;
                rSensorMv.pf.x = eisRegion.x_mv_float;
                rSensorMv.p.y  = eisRegion.y_mv_int;
                rSensorMv.pf.y = eisRegion.y_mv_float;
                rResizerMv = transform(rFactor.mSensor2Resizer, rSensorMv);
            }
#else
            //eis in resized domain
            if (EIS_MODE_IS_EIS_12_ENABLED(uEisMode))
            {
                const MSize& resizer = rFactor.mP1ResizerSize;
                rResizerMv.p.x = eisRegion.x_int - (resizer.w * (uEisFactor - 100) / 2 / uEisFactor);
                rResizerMv.pf.x = eisRegion.x_float;
                rResizerMv.p.y = eisRegion.y_int - (resizer.h * (uEisFactor - 100) / 2 / uEisFactor);
                rResizerMv.pf.y = eisRegion.y_float;
                rSensorMv = inv_transform(rFactor.mSensor2Resizer, rResizerMv);
            }
            else
            {
                rResizerMv.p.x = 0;
                rResizerMv.pf.x = 0.0f;
                rResizerMv.p.y = 0;
                rResizerMv.pf.y = 0.0f;
            }
#endif
            MY_LOGD_IF(mLogLevel > 1, "mv (%s): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
                       isResizedDomain ? "r->s" : "s->r",
                       rResizerMv.p.x,
                       rResizerMv.pf.x,
                       rResizerMv.p.y,
                       rResizerMv.pf.y,
                       rSensorMv.p.x,
                       rSensorMv.pf.x,
                       rSensorMv.p.y,
                       rSensorMv.pf.y
            );
            // rFactor.mActiveEisMv = inv_transform(rFactor.tranActive2Sensor, rFactor.mSensorEisMv);
            rFactor.mSensor2Active.transform(rFactor.mSensorEisMv.p, rFactor.mActiveEisMv.p);
            // FIXME: ignore float?
            // rFactor.mSensor2Active.transform(rFactor.mSensorEisMv.pf, rFactor.mActiveEisMv.pf);
            MY_LOGD_IF(mLogLevel > 1, "mv in active %d/%d, %d/%d",
                       rFactor.mActiveEisMv.p.x,
                       rFactor.mActiveEisMv.pf.x,
                       rFactor.mActiveEisMv.p.y,
                       rFactor.mActiveEisMv.pf.y
            );
        } else {
            rFactor.mEnableEis = MFALSE;
            memset(&rFactor.mActiveEisMv, 0, sizeof(vector_f));
            memset(&rFactor.mSensorEisMv, 0, sizeof(vector_f));
            memset(&rFactor.mResizerEisMv, 0, sizeof(vector_f));
        }
    }

    return pFactor;
  }
}

MBOOL
CropCalculator::
queryEisRegion(const IMetadata* pInHalMeta, eis_region& region) const
{
    IMetadata::IEntry entry = pInHalMeta->entryFor(MTK_EIS_REGION);
#if SUPPORT_EIS_MV
    // get EIS's motion vector
    if (entry.count() > 8)
    {
        MINT32 x_mv         = entry.itemAt(6, Type2Type<MINT32>());
        MINT32 y_mv         = entry.itemAt(7, Type2Type<MINT32>());
        region.is_from_zzr  = entry.itemAt(8, Type2Type<MINT32>());
        MBOOL x_mv_negative = x_mv >> 31;
        MBOOL y_mv_negative = y_mv >> 31;
        // convert to positive for getting parts of int and float if negative
        if (x_mv_negative) x_mv = ~x_mv + 1;
        if (y_mv_negative) y_mv = ~y_mv + 1;
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
        MY_LOGD_IF(mLogLevel, "EIS MV:%d, %d, %d",
                        region.s.w,
                        region.s.h,
                        region.is_from_zzr);
     }
#endif
    // get EIS's region
    if (entry.count() > 5) {
        region.x_int = entry.itemAt(0, Type2Type<MINT32>());
        region.x_float = entry.itemAt(1, Type2Type<MINT32>());
        region.y_int = entry.itemAt(2, Type2Type<MINT32>());
        region.y_float = entry.itemAt(3, Type2Type<MINT32>());
        region.s.w = entry.itemAt(4, Type2Type<MINT32>());
        region.s.h = entry.itemAt(5, Type2Type<MINT32>());
        MY_LOGD_IF(mLogLevel, "EIS Region: %d, %d, %d, %d, %dx%d",
                   region.x_int,
                   region.x_float,
                   region.y_int,
                   region.y_float,
                   region.s.w,
                   region.s.h);
        return MTRUE;
    }
    MY_LOGW("wrong eis region count %d", entry.count());
    return MFALSE;
}

MVOID
CropCalculator::
evaluate(MSize const &srcSize, MSize const &dstSize, MRect &srcCrop)
{
    // pillarbox
    if (srcSize.w * dstSize.h > srcSize.h * dstSize.w) {
        srcCrop.s.w = div_round(srcSize.h * dstSize.w, dstSize.h);
        srcCrop.s.h = srcSize.h;
        srcCrop.p.x = ((srcSize.w - srcCrop.s.w) >> 1);
        srcCrop.p.y = 0;
    }
    // letterbox
    else {
        srcCrop.s.w = srcSize.w;
        srcCrop.s.h = div_round(srcSize.w * dstSize.h, dstSize.w);
        srcCrop.p.x = 0;
        srcCrop.p.y = ((srcSize.h - srcCrop.s.h) >> 1);
    }
}

/**
 * use source buffer as coordinate system to crop region while YUV reprocessing
 *
 * @param[in]  srcSize: the size of source buffer
 * @param[in]  dstSize: the size of target buffer
 * @param[in]  srcCropRegion: the rectangle to crop on source buffer from app's metadata
 * @param[out] srcCrop:       the rectangle to crop on source buffer after evaluating
 *
 */
MVOID
CropCalculator::
evaluate(MSize const &srcSize, MSize const &dstSize, MRect const &srcCropRegion, MRect &srcCrop)
{
    srcCrop = MRect(srcCropRegion.p, srcCropRegion.s);
    // correct the boundary
    if (srcCrop.p.x < 0) srcCrop.p.x = 0;
    if (srcCrop.p.y < 0) srcCrop.p.y = 0;
    if (srcCrop.p.x + srcCrop.s.w > srcSize.w) srcCrop.s.w = srcSize.w - srcCrop.p.x;
    if (srcCrop.p.y + srcCrop.s.h > srcSize.h) srcCrop.s.h = srcSize.h - srcCrop.p.y;

    // pillarbox
    MINT32 tmp;
    if (srcCrop.s.w * dstSize.h > srcCrop.s.h * dstSize.w) {
        tmp = srcCrop.s.w;
        srcCrop.s.w = div_round(srcCrop.s.h * dstSize.w, dstSize.h);
        srcCrop.p.x += ((tmp - srcCrop.s.w) >> 1);
    }
    // letterbox
    else {
        tmp = srcCrop.s.h;
        srcCrop.s.h = div_round(srcCrop.s.w * dstSize.h, dstSize.w);
        srcCrop.p.y += ((tmp - srcCrop.s.h) >> 1);
    }

    srcCrop.s.w &= ~(0x1);
    srcCrop.s.h &= ~(0x1);

    MY_LOGD("srcSize:(%dx%d) dstSize:(%dx%d) srcCropRegion:(%d,%d)(%dx%d) srcCrop:(%d,%d)(%dx%d)",
            srcSize.w, srcSize.h,
            dstSize.w, dstSize.h,
            srcCropRegion.p.x, srcCropRegion.p.y,
            srcCropRegion.s.w, srcCropRegion.s.h,
            srcCrop.p.x, srcCrop.p.y,
            srcCrop.s.w, srcCrop.s.h);
}

MVOID
CropCalculator::
evaluate(sp<Factor> pFactor, MSize const &dstSize, MRect &srcCrop, MBOOL const bResized, MBOOL const bCheckCropRegion) const
{
    Factor& rFactor = *pFactor.get();
    MRect& rSensorCrop = rFactor.mSensorCrop;
#define abs(x,y) ((x)>(y)?(x)-(y):(y)-(x))
#define FOV_DIFF_TOLERANCE (3)
    MRect s_viewcrop;
    // pillarbox
    if (rSensorCrop.s.w * dstSize.h > rSensorCrop.s.h * dstSize.w) {
        s_viewcrop.s.w = div_round(rSensorCrop.s.h * dstSize.w, dstSize.h);
        s_viewcrop.s.h = rSensorCrop.s.h;
        s_viewcrop.p.x = rSensorCrop.p.x + ((rSensorCrop.s.w - s_viewcrop.s.w) >> 1);
        if( s_viewcrop.p.x < 0 && abs(s_viewcrop.p.x, 0) < FOV_DIFF_TOLERANCE )
           s_viewcrop.p.x = 0;
        s_viewcrop.p.y = rSensorCrop.p.y;
    }
    // letterbox
    else {
        s_viewcrop.s.w = rSensorCrop.s.w;
        s_viewcrop.s.h = div_round(rSensorCrop.s.w * dstSize.h, dstSize.w);
        s_viewcrop.p.x = rSensorCrop.p.x;
        s_viewcrop.p.y = rSensorCrop.p.y + ((rSensorCrop.s.h - s_viewcrop.s.h) >> 1);
        if( s_viewcrop.p.y < 0 && abs(s_viewcrop.p.y, 0) < FOV_DIFF_TOLERANCE ) {
           s_viewcrop.p.y = 0;
        }
    }
    MY_LOGD_IF(mLogLevel > 1, "s_cropRegion(%d, %d, %dx%d), dst %dx%d, view crop(%d, %d, %dx%d)",
               rSensorCrop.p.x, rSensorCrop.p.y,
               rSensorCrop.s.w, rSensorCrop.s.h,
               dstSize.w, dstSize.h,
               s_viewcrop.p.x, s_viewcrop.p.y,
               s_viewcrop.s.w, s_viewcrop.s.h
    );
#undef FOV_DIFF_TOLERANCE

    float ratio_s = (float)rFactor.mP1SensorCrop.s.w / (float)rFactor.mP1SensorCrop.s.h;
    float ratio_d = (float)s_viewcrop.s.w / (float)s_viewcrop.s.h;
    MY_LOGD_IF(mLogLevel > 1, "ratio_s:%f ratio_d:%f", ratio_s, ratio_d);

    // handle HAL3 sensor mode 16:9 FOV
    if ((s_viewcrop.p.x < 0 || s_viewcrop.p.y < 0) && abs(ratio_s, ratio_d) < 0.1f) {
        MRect refined = s_viewcrop;
        float ratio = (float)rFactor.mP1SensorCrop.s.h / (float)s_viewcrop.s.h;
        refined.s.w = s_viewcrop.s.w * ratio;
        refined.s.h = s_viewcrop.s.h * ratio;
        refined.p.x = s_viewcrop.p.x + ((s_viewcrop.s.w - refined.s.w) >> 1);
        refined.p.y = s_viewcrop.p.y + ((s_viewcrop.s.h - refined.s.h) >> 1);
        //
        s_viewcrop = refined;
        MY_LOGD_IF(mLogLevel > 1, "refine negative crop ratio %f r.s(%dx%d) r.p(%d, %d)",ratio, refined.s.w,refined.s.h, refined.p.x, refined.p.y );
    }
    //
    MY_LOGD_IF(mLogLevel > 1, "p1 sensor crop(%d, %d,%dx%d), %d, %d",
                    rFactor.mP1SensorCrop.p.x, rFactor.mP1SensorCrop.p.y,
                    rFactor.mP1SensorCrop.s.w, rFactor.mP1SensorCrop.s.h,
                    s_viewcrop.s.w*rFactor.mP1SensorCrop.s.h,  s_viewcrop.s.h*rFactor.mP1SensorCrop.s.w);
#undef abs
    if (bResized) {
        MRect r_viewcrop = transform(rFactor.mSensor2Resizer, s_viewcrop);
        srcCrop.s = r_viewcrop.s;
        srcCrop.p = r_viewcrop.p + rFactor.mResizerEisMv.p;

        // make sure hw limitation
        srcCrop.s.w &= ~(0x1);
        srcCrop.s.h &= ~(0x1);

        // check boundary
        if (refineBoundary(rFactor.mP1ResizerSize, srcCrop)) {
            MY_LOGW_IF(mLogLevel, "[FIXME] need to check crop!");
            rFactor.dump();
        }

    } else {
        srcCrop.s = s_viewcrop.s;
        srcCrop.p = s_viewcrop.p + rFactor.mSensorEisMv.p;

         // crop valid region
         if (bCheckCropRegion && rFactor.mP2RegionCrop.s.w > 0) {
             MRect validCrop = rFactor.mP2RegionCrop;
             //1. move to valid region
             if (validCrop.p.x < srcCrop.p.x) {
                 validCrop.p.x = srcCrop.p.x;
             }
             if (validCrop.p.y < srcCrop.p.y) {
                 validCrop.p.y = srcCrop.p.y;
             }
             if (validCrop.p.x + validCrop.s.w > srcCrop.p.x + srcCrop.s.w) {
                validCrop.s.w = srcCrop.p.x + srcCrop.s.w - validCrop.p.x;
             }
             if (validCrop.p.y + validCrop.s.h > srcCrop.p.y + srcCrop.s.h) {
                validCrop.s.h = srcCrop.p.y + srcCrop.s.h - validCrop.p.y;
             }

             // correct aspect ratio of crop region
             if (srcCrop.s.w * validCrop.s.h >= validCrop.s.w * srcCrop.s.h) { //pillar box
                 validCrop.s.h    = validCrop.s.w*srcCrop.s.h/srcCrop.s.w;
                 validCrop.p.y += (srcCrop.s.h - validCrop.s.h)/2;
             } else {  // letter box
                 validCrop.s.w    = validCrop.s.h*srcCrop.s.w/srcCrop.s.h;
                 validCrop.p.x += (srcCrop.s.w - validCrop.s.w)/2;
             }
             MY_LOGD("change crop region from (%d,%d)(%dx%d) to (%d,%d)(%dx%d)",
                     srcCrop.p.x, srcCrop.p.y,
                     srcCrop.s.w, srcCrop.s.h,
                     validCrop.p.x, validCrop.p.y,
                     validCrop.s.w, validCrop.s.h);

             srcCrop = validCrop;
         }


        // make sure hw limitation
        srcCrop.s.w &= ~(0x1);
        srcCrop.s.h &= ~(0x1);

        // check boundary
        if (refineBoundary(rFactor.mSensorSize, srcCrop)) {
            MY_LOGW_IF(mLogLevel, "[FIXME] need to check crop!");
            rFactor.dump();
        }
    }

    MY_LOGD_IF(mLogLevel > 1, "resized %d, crop (%d.%d)(%dx%d)",
           bResized,
           srcCrop.p.x, srcCrop.p.y,
           srcCrop.s.w, srcCrop.s.h);
}

MBOOL
CropCalculator::
refineBoundary(MSize const &bufSize, MRect &crop) const
{
    // tolerance
    if (crop.p.x == -1)
        crop.p.x = 0;

    if (crop.p.y == -1)
        crop.p.y = 0;

    MBOOL isRefined = MFALSE;
    MRect refined = crop;
    if (crop.p.x < 0) {
        refined.p.x = 0;
        isRefined = MTRUE;
    }
    if (crop.p.y < 0) {
        refined.p.y = 0;
        isRefined = MTRUE;
    }

    if ((refined.p.x + crop.s.w) > bufSize.w) {
        refined.s.w = bufSize.w - refined.p.x;
        isRefined = MTRUE;
    }
    if ((refined.p.y + crop.s.h) > bufSize.h) {
        refined.s.h = bufSize.h - refined.p.y;
        isRefined = MTRUE;
    }

    if (isRefined) {
        // make sure hw limitation
        refined.s.w &= ~(0x1);
        refined.s.h &= ~(0x1);

        MY_LOGW_IF(mLogLevel, "buffer size:%dx%d, crop(%d,%d)(%dx%d) -> refined crop(%d,%d)(%dx%d)",
                bufSize.w, bufSize.h,
                crop.p.x,
                crop.p.y,
                crop.s.w,
                crop.s.h,
                refined.p.x,
                refined.p.y,
                refined.s.w,
                refined.s.h
        );
        crop = refined;
    }
    return isRefined;
}

MBOOL
CropCalculator::Factor::
isEqual(sp<Factor> pFactor) const
{
    if (pFactor->mEnableEis == MTRUE)
        return MFALSE;

    Factor& rFactor = *pFactor;
    if (mP1SensorCrop == rFactor.mP1SensorCrop &&
        mP1ResizerSize == rFactor.mP1ResizerSize &&
        mP1DmaCrop == rFactor.mP1DmaCrop &&
        mActiveCrop == rFactor.mActiveCrop &&
        mSensorMode == rFactor.mSensorMode &&
        mP2RegionCrop == rFactor.mP2RegionCrop)
    {
        return MTRUE;
    }

    return MFALSE;
}

MVOID
CropCalculator::Factor::
dump() const
{
    MY_LOGD("p1 sensro crop(%d,%d,%dx%d), resizer size(%dx%d), crop dma(%d,%d,%dx%d)",
            mP1SensorCrop.p.x,
            mP1SensorCrop.p.y,
            mP1SensorCrop.s.w,
            mP1SensorCrop.s.h,
            mP1ResizerSize.w,
            mP1ResizerSize.h,
            mP1DmaCrop.p.x,
            mP1DmaCrop.p.y,
            mP1DmaCrop.s.w,
            mP1DmaCrop.s.h
    );

    mActive2Sensor.dump("tran active to sensor");

    MY_LOGD("tran sensor to resized o %d, %d, s %dx%d -> %dx%d",
            mSensor2Resizer.tarOrigin.x,
            mSensor2Resizer.tarOrigin.y,
            mSensor2Resizer.oldScale.w,
            mSensor2Resizer.oldScale.h,
            mSensor2Resizer.newScale.w,
            mSensor2Resizer.newScale.h
    );
    MY_LOGD("modified active crop %d, %d, %dx%d",
            mActiveCrop.p.x,
            mActiveCrop.p.y,
            mActiveCrop.s.w,
            mActiveCrop.s.h
    );
    MY_LOGD("isEisOn %d", mEnableEis);
    MY_LOGD("mv in active %d/%d, %d/%d",
            mActiveEisMv.p.x, mActiveEisMv.pf.x,
            mActiveEisMv.p.y, mActiveEisMv.pf.y
    );
    MY_LOGD("mv in sensor %d/%d, %d/%d",
            mSensorEisMv.p.x, mSensorEisMv.pf.x,
            mSensorEisMv.p.y, mSensorEisMv.pf.y
    );
    MY_LOGD("mv in resized %d/%d, %d/%d",
            mResizerEisMv.p.x, mResizerEisMv.pf.x,
            mResizerEisMv.p.y, mResizerEisMv.pf.y
    );
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IBoosterPtr Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IBoosterPtr
IBooster::createInstance(const std::string& name)
{
    auto getIsForceDisableBoost = []() -> MBOOL
    {
        MINT32 ret = ::property_get_int32("vendor.debug.camera.capture.boost.disable", 0);
        MY_LOGD("vendor.debug.camera.capture.boost.disable = %d", ret);
        return (ret > 0);
    };
    static MBOOL isForceDisable = getIsForceDisableBoost();
    static MBOOL isPerfLibEnable = PerfClientVendor::getInstance()->isPerfLibEnable();
    //
    IBooster* pBoost = NULL;
    if (isForceDisable)
        pBoost = new EmptyBoost(name);
    else if (isPerfLibEnable)
        pBoost = new PerfBooster(name);
    else
        pBoost = new CPUBooster(name);
    return IBoosterPtr(pBoost, [](IBooster* p)
        {
            delete p;
        });
}

IBooster::~IBooster()
{

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PowerHalWrapper Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PowerHalWrapper*
PowerHalWrapper::
getInstance()
{
    static UniquePtr<PowerHalWrapper> singleton(new PowerHalWrapper(), [](PowerHalWrapper* p)
        {
            delete p;
        });
    return singleton.get();
}

PowerHalWrapper::
PowerHalWrapper()
{
    TRACE_FUNC("ctor:%p", this);
}

PowerHalWrapper::
~PowerHalWrapper()
{
    TRACE_FUNC("dtor:%p", this);
}

PowerHalWrapper::Handle
PowerHalWrapper::
createHandle(const std::string& userName)
{
    AUTO_TIMER("createPowerHalHandle, userName:%s", userName.c_str());

    Handle ret = INVALID_HANDLE;
    {
        std::lock_guard<std::mutex> guard(sLocker);
        //
        const size_t oldCount = sHandles.size();
        if(oldCount == 0) {
            initLock();
        }
        if(sPowerPtr != nullptr)
        {
            ret = sPowerPtr->scnReg();
            sHandles.push_back(ret);

            const size_t newCount = sHandles.size();
            MY_LOGD("create handle,  userName:%s, powerPtr:%p, handle:%d, handleCount:%zu -> %zu",
                 userName.c_str(), sPowerPtr.get(), ret, oldCount, newCount);
        }
        else
        {
            MY_LOGW("failed to create handle, userName:%s", userName.c_str());
        }
    }
    return ret;
}

MVOID
PowerHalWrapper::
destroyhandle(const std::string& userName, Handle handle)
{
    AUTO_TIMER("destroyPowerHalHandle, userName:%s", userName.c_str());

    std::lock_guard<std::mutex> guard(sLocker);
    //
    HandlesItor itor = sHandles.end();
    if(!getIsValidLock(handle, itor)) {
        MY_LOGE("invalid handle, powerPtr:%p, handle:%d", sPowerPtr.get(), handle);
        return;
    }
    //
    const size_t oldCount = sHandles.size();
    sHandles.erase(itor);
    sPowerPtr->scnDisable(handle);
    sPowerPtr->scnUnreg(handle);

    const size_t newCount = sHandles.size();
    MY_LOGD("destory handle, powerPtr:%p, handle:%d, handleCount:%zu -> %zu",
        sPowerPtr.get(), handle, oldCount, newCount);

    if(newCount == 0) {
        uninitLock();
    }
}

MVOID
PowerHalWrapper::
enableBoost(const std::string& userName, Handle handle)
{
    AUTO_TIMER("boostPowerHal, userName:%s", userName.c_str());

    std::lock_guard<std::mutex> guard(sLocker);
    //
    if(sPowerPtr == NULL) {
         MY_LOGW("failed to enable boost, invalid powerPtr");
         return;
    }

    if(!getIsValidLock(handle)) {
        MY_LOGW("invalid handle, powerPtr:%p, handle:%d", sPowerPtr.get(), handle);
        return;
    }
    // frenquency
    const MINT32 clusterCount = sPowerPtr->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_NUM, 0);
    for(MINT32 i = 0; i < clusterCount; ++i) {
        // query the maximum frenquency of the current cluster
        MINT32 maxFreq = sPowerPtr->querySysInfo(MtkQueryCmd::CMD_GET_CLUSTER_CPU_FREQ_MAX, i);
        // set both the min and max frequency of current cluster to the maximum frequency
        sPowerPtr->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MIN, i, maxFreq, 0, 0);
        sPowerPtr->scnConfig(handle, MtkPowerCmd::CMD_SET_CLUSTER_CPU_FREQ_MAX, i, maxFreq, 0, 0);
        TRACE_FUNC("modify CPU frequency, powerPtr:%p, handle:%d, clusterNum:%d/%d, maxFreq:%d",
            sPowerPtr.get(), handle, i, clusterCount, maxFreq);
    }
    // DRAM
    const MINT32 maxDramOPPLevel = 0;
    sPowerPtr->scnConfig(handle, MtkPowerCmd::CMD_SET_OPP_DDR, maxDramOPPLevel, 0, 0, 0);
    //
    sPowerPtr->scnEnable(handle, TIMEOUT);
    MY_LOGD("enable boost, powerPtr:%p, handle:%d", sPowerPtr.get(), handle);
}

MVOID
PowerHalWrapper::
disableBoost(const std::string& userName, Handle handle)
{
    AUTO_TIMER("disboostPowerHal, userName:%s", userName.c_str());

    std::lock_guard<std::mutex> guard(sLocker);
    //
    if(sPowerPtr == NULL) {
         MY_LOGW("failed to enable boost, invalid powerPtr");
         return;
    }

    if(!getIsValidLock(handle)) {
        MY_LOGW("invalid handle, powerPtr:%p, handle:%d", sPowerPtr.get(), handle);
        return;
    }
    sPowerPtr->scnDisable(handle);
    MY_LOGD("disable boost, powerPtr:%p, handle:%d", sPowerPtr.get(), handle);
}

MVOID
PowerHalWrapper::
initLock()
{
    if((sPowerPtr != NULL) || (sHandles.size() != 0)) {
        MY_LOGE("non-expected value, powerPtr:%p, handleCount:%zu", sPowerPtr.get(), sHandles.size());
        dumpLock();
    }
    //
    sPowerPtr = IPower::getService();
    if(sPowerPtr != NULL) {
        MY_LOGD("get powerService, powerPtr:%p", sPowerPtr.get());
    }
    else {
        MY_LOGE("failed to get powerService");
    }
}

MVOID
PowerHalWrapper::
uninitLock()
{
    if((sPowerPtr == NULL) || (sHandles.size() != 0)) {
        MY_LOGE("non-expected value, powerPtr:%p, handleCount:%zu", sPowerPtr.get(), sHandles.size());
        dumpLock();
    }
    //
    MY_LOGD("release powerService, powerPtr:%p", sPowerPtr.get());
    sPowerPtr = NULL;
    sHandles.clear();
}

MVOID
PowerHalWrapper::
dumpLock()
{
    const ssize_t handleCount = sHandles.size();
    std::stringstream ss;
    ss << "["
       << "pp: " << sPowerPtr.get() << ", "
       << "hs:[" ;
    const MINT32 lastIndex = (handleCount - 1);
    for(MINT32 i = 0; i < handleCount; ++i) {
        ss << i << ": " << sHandles[i];

        if(i != lastIndex)
            ss << ", ";
    }
    ss << "]"
       << "]" << std::endl;
    MY_LOGD("dump info:%s", ss.str().c_str());
}

MBOOL
PowerHalWrapper::
getIsValidLock(Handle handle)
{
    return (std::find(sHandles.begin(), sHandles.end(), handle) != sHandles.end());
}

MBOOL
PowerHalWrapper::
getIsValidLock(Handle handle, HandlesItor& itor)
{
    itor = std::find(sHandles.begin(), sHandles.end(), handle);
    return (itor != sHandles.end());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// PerfClientVendor Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PerfClientVendor*
PerfClientVendor::getInstance()
{
    static UniquePtr<PerfClientVendor> singleton(new PerfClientVendor(), [](PerfClientVendor* p)
    {
        delete p;
    });
    return singleton.get();
}

PerfClientVendor::
PerfClientVendor()
{
    initPerfLib();
    TRACE_FUNC("ctor:%p", this);
}

PerfClientVendor::
~PerfClientVendor()
{
    closePerfLib();
    TRACE_FUNC("dtor:%p", this);
}


MBOOL PerfClientVendor::loadPerfAPI()
{
    void *func;

    libHandle = dlopen(perfLib, RTLD_NOW);

    if (libHandle == NULL) {
        MY_LOGW("dlopen fail: %s\n", dlerror());
        libEnabled = MFALSE;
        return MFALSE;
    }

    func = dlsym(libHandle, "perf_lock_acq");
    perfLockAcq = reinterpret_cast<perfLockAcqFunc>(func);

    if (perfLockAcq == NULL) {
        MY_LOGW("perfLockAcq error: %s\n", dlerror());
        dlclose(libHandle);
        libEnabled = MFALSE;
        return MFALSE;
    }

    func = dlsym(libHandle, "perf_lock_rel");
    perfLockRel = reinterpret_cast<perfLockRelFunc>(func);

    if (perfLockRel == NULL) {
        MY_LOGW("perfLockRel error: %s\n", dlerror());
        dlclose(libHandle);
        libEnabled = MFALSE;
        return MFALSE;
    }

    libEnabled = MTRUE;
    return MTRUE;
}

MVOID PerfClientVendor::initPerfLib()
{
    if (!libEnabled || (perfLockAcq == NULL) || (perfLockRel == NULL)) {
        if (!loadPerfAPI()) {
            MY_LOGW("dlopen failed");
            return;
        }
        MY_LOGI("dlopen mtkperf_client_vendor success");
    }
    return;
}

MVOID PerfClientVendor::closePerfLib()
{
    if (libHandle != NULL) {
        perfLockAcq = NULL;
        perfLockRel = NULL;
        dlclose(libHandle);
        libHandle = NULL;
        libEnabled = MFALSE;
        MY_LOGI("dlclose mtkperf_client_vendor");
    }
    return;
}

MBOOL PerfClientVendor::isPerfLibEnable()
{
    return libEnabled;
}

int PerfClientVendor::enablePerf(const std::string& userName, int handle)
{
    AUTO_TIMER("enalbePerf, userName:%s", userName.c_str());

    if(handle != -1) {
         MY_LOGW("already enabled, handle:%d", handle);
         return handle;
    }

    if(!libEnabled) {
        if(loadPerfAPI())
            MY_LOGD("reload mtkperf_client_vendor");
        else {
            MY_LOGW("cannot reload mtkperf_client_vendor");
            return -1;
        }
    }

    int perfLockSrc[] = {PERF_RES_CPUFREQ_PERF_MODE, 1, PERF_RES_DRAM_OPP_MIN, 0};
    int newHandle = perfLockAcq(0, 10000, perfLockSrc, 4);
    if(newHandle == -1) {
         MY_LOGW("failed to enable boost, invalid handle:%d", newHandle);
         return -1;
    }
    MY_LOGD("enable boost, userName:%s handle:%d", userName.c_str(), newHandle);
    return newHandle;
}

int PerfClientVendor::disablePerf(const std::string& userName, int handle)
{
    AUTO_TIMER("disalbePerf, userName:%s", userName.c_str());

    if(handle == -1) {
         MY_LOGW("failed to disable boost, invalid handle:%d", handle);
         return -1;
    }

    if(!libEnabled) {
        if(loadPerfAPI())
            MY_LOGD("reload mtkperf_client_vendor");
        else {
            MY_LOGW("cannot reload mtkperf_client_vendor");
            return -1;
        }
    }

    perfLockRel(handle);
    MY_LOGD("disable boost, userName:%s handle:%d", userName.c_str(), handle);
    return -1;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  EmptyBooster Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
EmptyBoost::
EmptyBoost(const std::string& name)
: mName(name)
{
    MY_LOGD("ctor:%p, name:%s", this, mName.c_str());
}

EmptyBoost::
~EmptyBoost()
{
    MY_LOGD("dtor:%p, name:%s", this, mName.c_str());
}

const std::string&
EmptyBoost::
getName()
{
    return mName;
}

MVOID
EmptyBoost::
enable()
{
    MY_LOGD("enable EmptyBoost, addr:%p, name:%s", this, mName.c_str());
}

MVOID
EmptyBoost::
disable()
{
    MY_LOGD("enable EmptyBoost, addr:%p, name:%s", this, mName.c_str());
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  CPUBooster Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
CPUBooster::
CPUBooster(const std::string& name)
: mName(name)
{
    mHandle = PowerHalWrapper::getInstance()->createHandle(getName());
    TRACE_FUNC("ctor:%p, name:%s, handle:%d", this, mName.c_str(), mHandle);
}

CPUBooster::
~CPUBooster()
{
    TRACE_FUNC("dtor:%p, name:%s, handle:%d", this, mName.c_str(), mHandle);
    try {
        PowerHalWrapper::getInstance()->destroyhandle(getName(), mHandle);
    }
    catch (std::exception&) {
        MY_LOGW("PowerHalWrapper's destroyhandle throws exception");
    }
}

const std::string&
CPUBooster::
getName()
{
    return mName;
}

MVOID
CPUBooster::
enable()
{
    PowerHalWrapper::getInstance()->enableBoost(getName(), mHandle);
}

MVOID
CPUBooster::
disable()
{
    PowerHalWrapper::getInstance()->disableBoost(getName(), mHandle);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PerfBooster Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
PerfBooster::
PerfBooster(const std::string& name)
: mName(name)
, mHandle(-1)
{
    TRACE_FUNC("ctor:%p, name:%s, handle:%d", this, mName.c_str(), mHandle);
}

PerfBooster::
~PerfBooster()
{
    mHandle = -1;
    TRACE_FUNC("dtor:%p, name:%s, handle:%d", this, mName.c_str(), mHandle);
}

const std::string&
PerfBooster::
getName()
{
    return mName;
}

MVOID
PerfBooster::
enable()
{
    mHandle = PerfClientVendor::getInstance()->enablePerf(getName(), mHandle);
}

MVOID
PerfBooster::
disable()
{
    mHandle = PerfClientVendor::getInstance()->disablePerf(getName(), mHandle);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IspProfileManager Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
using IspProfileInfoTable = std::map<IspProfileHint, IspProfileInfo, IspProfileHint::Compare>;
inline static
const IspProfileInfoTable&
getIspProfileInfoTable()
{
    static const IspProfileInfoTable table = []()
    {
        #if (MTK_CAPTURE_ISP_VERSION == 6)
            const IspProfileInfoTable ret =
            {
                // BayerBayer
                { {eSAN_Master, eSCT_BayerBayer, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture) },
                { {eSAN_Master, eSCT_BayerBayer, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture_Depth) },
                { {eSAN_Sub_01, eSCT_BayerBayer, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture_Full) },
                { {eSAN_Sub_01, eSCT_BayerBayer, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture_Depth) },
                // BayerMono
                { {eSAN_Master, eSCT_BayerMono, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture) },
                { {eSAN_Master, eSCT_BayerMono, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture_Depth_toW) },
                { {eSAN_Sub_01, eSCT_BayerMono, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture) },
                { {eSAN_Sub_01, eSCT_BayerMono, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture_Depth) },
            };
        #else
            const IspProfileInfoTable ret =
            {
                // BayerBayer
                { {eSAN_Master, eSCT_BayerBayer, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture) },
                { {eSAN_Master, eSCT_BayerBayer, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Preview) },
                { {eSAN_Sub_01, eSCT_BayerBayer, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture) },
                { {eSAN_Sub_01, eSCT_BayerBayer, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Preview) },
                // BayerMono
                { {eSAN_Master, eSCT_BayerMono, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture) },
                { {eSAN_Master, eSCT_BayerMono, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Preview) },
                { {eSAN_Sub_01, eSCT_BayerMono, eRIT_Imgo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture) },
                { {eSAN_Sub_01, eSCT_BayerMono, eRIT_Rrzo}, MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Preview) },
            };
        #endif
        //
        {
            std::stringstream ss;
            ss << "dualcam ispprofile table" << std::endl;
            for(const auto& item : ret)
            {
                ss << "key:0x" << std::hex << std::setw(10) << std::setfill('0') << item.first.mValue << ", "
                   << "profileName:" << item.second.mName << std::endl;
            }
            MY_LOGD("%s", ss.str().c_str());
        }
        return ret;
    }();
    return table;
}

const IspProfileInfo&
IspProfileManager::
get(const IspProfileHint& hint)
{
    const auto& table = getIspProfileInfoTable();
    auto found = table.find(hint);
    if(found != table.end())
    {
        MY_LOGD("found ispprofile, hint:%#012" PRIx64 ", name:%s", hint.mValue, found->second.mName);
        return found->second;
    }
    else
    {
        static const IspProfileInfo ret = MAKE_ISP_PROFILE_INFO(EIspProfile_N3D_Capture);
        MY_LOGD("not existing hint and use the default profile. hint:%#012" PRIx64 ", name:%s", hint.mValue, ret.mName);
        return ret;
    }
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  FovCalculator Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FovCalculator::
FovCalculator(const std::vector<MINT32>& sensorIndexes, const MBOOL isDualCam)
: mEnable(MTRUE)
, mIsDualCam(isDualCam)
{
    mEnable = (property_get_int32("vendor.debug.camera.p2.enable.dualfov", ENABLE_DUAL_FOV_CROP) > 0);
    MY_LOGD("ctor:%p, isDualCam:%d, enable:%d", this, mIsDualCam, mEnable);
    init(sensorIndexes);
}

auto
FovCalculator::
getIsEnable() const -> MBOOL
{
    return mIsDualCam && mEnable;
}

auto
FovCalculator::
getFovInfo(MINT32 sensorIndex, FovInfo& fovInfo) const -> MBOOL
{
    MBOOL ret = MFALSE;
    if ( mIsDualCam && mEnable )
    {
        if ( mFOVInfos.count(sensorIndex) > 0 )
        {
            fovInfo = mFOVInfos.at(sensorIndex);
            ret = MTRUE;
        }
        else
        {
            MY_LOGW("failed to get fov info, invalid sensorIndex:%d", sensorIndex);
        }
    }
    return ret;
}

auto
FovCalculator::
transform(MINT32 sensorIndex, const MPoint& inPoint, MPoint& outPoint) const -> MBOOL
{
    MBOOL ret = MFALSE;
    if ( mIsDualCam && mEnable )
    {
        FovInfo fovInfo;
        if ( getFovInfo(sensorIndex, fovInfo) )
        {
            auto trans = [](float in, float offset, float oldLength, float newLength) -> float
            {
                return (in - offset) * (newLength/oldLength);
            };
            outPoint.x = trans(inPoint.x, fovInfo.mFOVCropRegion.p.x, fovInfo.mFOVCropRegion.s.w, fovInfo.mDestinationSize.w);
            outPoint.y = trans(inPoint.y, fovInfo.mFOVCropRegion.p.y, fovInfo.mFOVCropRegion.s.h, fovInfo.mDestinationSize.h);
            ret = MTRUE;
        }
        else
        {
            MY_LOGW("failed to get fov info, sensorIndex:%d, inSize:(%dx%d)", sensorIndex, inPoint.x, inPoint.y);
        }
    }
    return ret;
}


auto
FovCalculator::
transform(MINT32 sensorIndex, const MPoint& inLeftTop, const MPoint& inRightBottom, MPoint& outLeftTop, MPoint& outRightBottom) const -> MBOOL
{
    MBOOL ret = MFALSE;
    if ( mIsDualCam && mEnable )
    {
        FovInfo fovInfo;
        if ( getFovInfo(sensorIndex, fovInfo) )
        {
            auto &fovRect = fovInfo.mFOVCropRegion;
            auto &dstSize = fovInfo.mDestinationSize;

            auto intersect = [](MRect const& inRect, MRect const& fovRect) -> MRect
            {
                MINT32 x1 = std::max(inRect.p.x, fovRect.p.x);
                MINT32 x2 = std::min(inRect.p.x + inRect.s.w, fovRect.p.x + fovRect.s.w);
                MINT32 y1 = std::max(inRect.p.y, fovRect.p.y);
                MINT32 y2 = std::min(inRect.p.y + inRect.s.h, fovRect.p.y + fovRect.s.h);

                if (x2 >= x1 && y2 >= y1) {
                    return MRect(MPoint(x1, y1), MSize(x2 - x1, y2 - y1));
                }

                // face rectangle is out of FOVCropRegion
                return MRect(fovRect.p, MSize(0, 0));
            };

            auto trans = [](float in, float offset, float oldLength, float newLength) -> float
            {
                return (in - offset)*(newLength/oldLength);
            };

            auto adjustRect  = intersect(MRect(inLeftTop, inRightBottom), fovRect);

            outLeftTop.x     = trans(adjustRect.p.x, fovRect.p.x, fovRect.s.w, dstSize.w);
            outLeftTop.y     = trans(adjustRect.p.y, fovRect.p.y, fovRect.s.h, dstSize.h);
            outRightBottom.x = trans(adjustRect.p.x + adjustRect.s.w, fovRect.p.x, fovRect.s.w, dstSize.w);
            outRightBottom.y = trans(adjustRect.p.y + adjustRect.s.h, fovRect.p.y, fovRect.s.h, dstSize.h);

            ret = MTRUE;
        }
        else
        {
            MY_LOGW("failed to get fov info, sensorIndex:%d, inSize:(%dx%d)(%dx%d)", sensorIndex, inLeftTop.x, inLeftTop.y, inRightBottom.x, inRightBottom.y);
        }
    }
    return ret;
}

auto
FovCalculator::
init(const std::vector<MINT32>& sensorIndexes) -> MVOID
{
    SCOPED_TRACER();

    if ( mIsDualCam && mEnable )
    {
        for ( MINT32 sensorIndex : sensorIndexes )
        {
            FovInfo fovInfo;
            if ( StereoSizeProvider::getInstance()->getDualcamP2IMGOYuvCropResizeInfo(sensorIndex, fovInfo.mFOVCropRegion, fovInfo.mDestinationSize) )
            {
                mFOVInfos.insert({sensorIndex, fovInfo});
                MY_LOGD("add fovInfo, sensorIndex:%d, region(x, y, w, h):(%d, %d, %d, %d), dstSize:(%d, %d)",
                    sensorIndex,
                    fovInfo.mFOVCropRegion.p.x, fovInfo.mFOVCropRegion.p.y,
                    fovInfo.mFOVCropRegion.s.w, fovInfo.mFOVCropRegion.s.h,
                    fovInfo.mDestinationSize.w, fovInfo.mDestinationSize.h);
            }
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IHolder Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
std::unique_ptr<IHolder>
IHolder::createResizerInstance(IImageBuffer* pBuf, const MSize& size)
{
    auto pHolder = std::make_unique<BufResizer>(pBuf, size) ;
    return pHolder;
}

IHolder::~IHolder()
{

}

// calculate gcd/lcm
int gcd(int num1, int num2)
{
    for (;;)
    {
        if (num1 == 0) return num2;
        num2 %= num1;
        if (num2 == 0) return num1;
        num1 %= num2;
    }
}

int lcm(int num1, int num2)
{
    int temp = gcd(num1, num2);

    return temp ? (num1 / temp * num2) : 0;
}

// HW input has the alignment limiation = 32 bit align
#define HW_INPUT_STRIDE_LIMITATION 32
// only the following bits per pixel needs to handle explicitly
std::map<HwStrideAlignment::BitPerPixel, HwStrideAlignment::WidthAlign>
HwStrideAlignment::ALIGNMENT_WIDTH =
{
    // ONLY for 10bits per pixel fmt, need to handle its alignment
    {10, lcm(10, HW_INPUT_STRIDE_LIMITATION)/10}
};
// height alignment
std::map<HwStrideAlignment::BitPerPixel, HwStrideAlignment::HeightAlign>
HwStrideAlignment::ALIGNMENT_HEIGHT =
{
    // 10bit yuv has the 4 pixel hight align for mdp
    {10, lcm(10,8)/10}
};

MSize
HwStrideAlignment::
queryFormatAlignment(
    const Format_T& fmt,
    const MSize& currentAlignment
)
{
    MSize resultAlignment = currentAlignment;
    // alignment check
    int bitSize = Utils::Format::queryPlaneBitsPerPixel(fmt, 0);
    if(HwStrideAlignment::ALIGNMENT_WIDTH.count(bitSize))
    {
        int alignment_w = HwStrideAlignment::ALIGNMENT_WIDTH[bitSize];
        int alignment_h = HwStrideAlignment::ALIGNMENT_HEIGHT[bitSize];
        if(currentAlignment.w == 0)
            resultAlignment.w = alignment_w;
        else
        {
            int lcm_align = lcm(currentAlignment.w, alignment_w);
            resultAlignment.w = lcm_align;
        }
        if(currentAlignment.h == 0)
            resultAlignment.h = alignment_h;
        else
        {
            int lcm_align = lcm(currentAlignment.h, alignment_h);
            resultAlignment.h = lcm_align;
        }
    }
    return resultAlignment;
}

MVOID dumpQParams(int logLevel, QParams& rInputQParam)
{
    if(logLevel<3)
        return;

    MY_LOGD("Frame size = %zu", rInputQParam.mvFrameParams.size());
    for(size_t index = 0; index < rInputQParam.mvFrameParams.size(); ++index)
    {
        auto& frameParam = rInputQParam.mvFrameParams.itemAt(index);
        MY_LOGD("=================================================");
        MY_LOGD("Frame index = %zu", index);
        MY_LOGD("mStreamTag=%d mSensorIdx=%d", frameParam.mStreamTag, frameParam.mSensorIdx);
        MY_LOGD("FrameNo=%d RequestNo=%d Timestamp=%d IspProfile=%d SensorDev=%d NeedDump=%d",
                frameParam.FrameNo, frameParam.RequestNo, frameParam.Timestamp, frameParam.IspProfile, frameParam.SensorDev, frameParam.NeedDump);
        MY_LOGD("=== mvIn section ===");
        for(size_t index2=0;index2<frameParam.mvIn.size();++index2)
        {
            Input data = frameParam.mvIn[index2];
            MY_LOGD("Index = %zu", index2);
            MY_LOGD("mvIn.PortID.index = %d", data.mPortID.index);
            MY_LOGD("mvIn.PortID.type = %d", data.mPortID.type);
            MY_LOGD("mvIn.PortID.inout = %d", data.mPortID.inout);
            MY_LOGD("mvIn.mBuffer=%p", data.mBuffer);
            if(data.mBuffer !=NULL)
            {
                MY_LOGD("mvIn.mBuffer->getImgSize = %dx%d", data.mBuffer->getImgSize().w,
                                                    data.mBuffer->getImgSize().h);
                MY_LOGD("mvIn.mBuffer->getImgFormat = %x", data.mBuffer->getImgFormat());
                MY_LOGD("mvIn.mBuffer->getPlaneCount = %zu", data.mBuffer->getPlaneCount());
                for(int j=0;j<data.mBuffer->getPlaneCount();j++)
                {
                    MY_LOGD("mvIn.mBuffer->getBufVA(%d) = %#" PRIxPTR "", j, data.mBuffer->getBufVA(j));
                    MY_LOGD("mvIn.mBuffer->getBufStridesInBytes(%d) = %zu", j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else
            {
                MY_LOGD("mvIn.mBuffer is NULL!!");
            }
            MY_LOGD("mvIn.mTransform = %d", data.mTransform);
        }
        MY_LOGD("=== mvOut section ===");
        for(size_t index2=0;index2<frameParam.mvOut.size(); index2++)
        {
            Output data = frameParam.mvOut[index2];
            MY_LOGD("Index = %zu", index2);
            MY_LOGD("mvOut.PortID.index = %d", data.mPortID.index);
            MY_LOGD("mvOut.PortID.type = %d", data.mPortID.type);
            MY_LOGD("mvOut.PortID.inout = %d", data.mPortID.inout);
            MY_LOGD("mvOut.mBuffer=%p", data.mBuffer);
            if(data.mBuffer != NULL)
            {
                MY_LOGD("mvOut.mBuffer->getImgSize = %dx%d", data.mBuffer->getImgSize().w,
                                                    data.mBuffer->getImgSize().h);
                MY_LOGD("mvOut.mBuffer->getImgFormat = %x", data.mBuffer->getImgFormat());
                MY_LOGD("mvOut.mBuffer->getPlaneCount = %zu", data.mBuffer->getPlaneCount());
                for(size_t j=0;j<data.mBuffer->getPlaneCount();j++)
                {
                    MY_LOGD("mvOut.mBuffer->getBufVA(%zu) = %#" PRIxPTR "", j, data.mBuffer->getBufVA(j));
                    MY_LOGD("mvOut.mBuffer->getBufStridesInBytes(%zu) = %zu", j, data.mBuffer->getBufStridesInBytes(j));
                }
            }
            else
            {
                MY_LOGD("mvOut.mBuffer is NULL!!");
            }
            MY_LOGD("mvOut.mTransform = %d", data.mTransform);
        }
        MY_LOGD("=== mvCropRsInfo section ===");
        for(size_t i=0;i<frameParam.mvCropRsInfo.size(); i++)
        {
            MCrpRsInfo data = frameParam.mvCropRsInfo[i];
            MY_LOGD("Index = %zu", i);
            MY_LOGD("CropRsInfo.mGroupID=%d", data.mGroupID);
            MY_LOGD("CropRsInfo.mMdpGroup=%d", data.mMdpGroup);
            MY_LOGD("CropRsInfo.mResizeDst=%dx%d", data.mResizeDst.w, data.mResizeDst.h);
            MY_LOGD("CropRsInfo.mCropRect.p_fractional=(%d,%d) ", data.mCropRect.p_fractional.x, data.mCropRect.p_fractional.y);
            MY_LOGD("CropRsInfo.mCropRect.p_integral=(%d,%d) ", data.mCropRect.p_integral.x, data.mCropRect.p_integral.y);
            MY_LOGD("CropRsInfo.mCropRect.s=%dx%d ", data.mCropRect.s.w, data.mCropRect.s.h);
        }
        MY_LOGD("=== mvModuleData section ===");
        for(size_t i=0;i<frameParam.mvModuleData.size(); i++)
        {
            ModuleInfo data = frameParam.mvModuleData[i];
            MY_LOGD("Index = %zu", i);
            MY_LOGD("ModuleData.moduleTag=%d", data.moduleTag);
            _SRZ_SIZE_INFO_ *SrzInfo = (_SRZ_SIZE_INFO_ *) data.moduleStruct;
            MY_LOGD("SrzInfo->in_w=%d", SrzInfo->in_w);
            MY_LOGD("SrzInfo->in_h=%d", SrzInfo->in_h);
            MY_LOGD("SrzInfo->crop_w=%lu", SrzInfo->crop_w);
            MY_LOGD("SrzInfo->crop_h=%lu", SrzInfo->crop_h);
            MY_LOGD("SrzInfo->crop_x=%d", SrzInfo->crop_x);
            MY_LOGD("SrzInfo->crop_y=%d", SrzInfo->crop_y);
            MY_LOGD("SrzInfo->crop_floatX=%d", SrzInfo->crop_floatX);
            MY_LOGD("SrzInfo->crop_floatY=%d", SrzInfo->crop_floatY);
            MY_LOGD("SrzInfo->out_w=%d", SrzInfo->out_w);
            MY_LOGD("SrzInfo->out_h=%d", SrzInfo->out_h);
        }
        MY_LOGD("TuningData=%p", frameParam.mTuningData);
        MY_LOGD("=== mvExtraData section ===");
        for(size_t i=0;i<frameParam.mvExtraParam.size(); i++)
        {
            auto extraParam = frameParam.mvExtraParam[i];
            MY_LOGD("ExtraParam CmdIdx:%d", extraParam.CmdIdx);
            if(extraParam.CmdIdx == EPIPE_MDP_PQPARAM_CMD)
            {
                PQParam* param = reinterpret_cast<PQParam*>(extraParam.moduleStruct);
                if(param->WDMAPQParam != nullptr)
                {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WDMAPQParam;
                    DpIspParam& ispParam = dpPqParam->u.isp;
                    MY_LOGD("WDMAPQParam %p enable = %d, scenario=%d", dpPqParam, dpPqParam->enable, dpPqParam->scenario);
                    MY_LOGD("WDMAPQParam iso = %d, frameNo=%d requestNo=%d", ispParam.iso , ispParam.frameNo, ispParam.requestNo);
                }
                if(param->WROTPQParam != nullptr)
                {
                    DpPqParam* dpPqParam = (DpPqParam*)param->WROTPQParam;
                    DpIspParam&ispParam = dpPqParam->u.isp;
                    MY_LOGD("WROTPQParam %p enable = %d, scenario=%d", dpPqParam, dpPqParam->enable, dpPqParam->scenario);
                    MY_LOGD("WROTPQParam iso = %d, frameNo=%d requestNo=%d", ispParam.iso , ispParam.frameNo, ispParam.requestNo);
                }
            }
        }
    }
}

} // NSCapture
} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
