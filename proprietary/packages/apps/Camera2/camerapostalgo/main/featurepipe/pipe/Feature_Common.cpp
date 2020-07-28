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
// Android system/core header file

// mtkcam custom header file


#if 0
// mtkcam global header file
#include <vendor/mediatek/hardware/power/2.0/IPower.h>
#include <vendor/mediatek/hardware/power/2.0/types.h>

// Module header file
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/metadata/client/mtk_metadata_tag.h>
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
//
#include <featurePipe/core/include/DebugUtil.h>
//
// isp tuning
#include <isp_tuning/isp_tuning.h>
//
// stereo
#include <mtkcam3/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#endif

// Local header file
#include <pipe/Feature_Common.h>
#include <pipe/FeatureRequest.h>

// Logging
#include "core/DebugControl.h"
#define PIPE_CLASS_TAG "Util"
#define PIPE_TRACE TRACE_CAPTURE_FEATURE_COMMON
#include <core/PipeLog.h>


/*******************************************************************************
* MACRO Utilities Define.
********************************************************************************/
namespace { // anonymous namespace for MACRO function
using AutoObject = com::mediatek::campostalgo::NSFeaturePipe::UniquePtr<const char>;
//
auto
createAutoScoper(const char* funcName) -> AutoObject
{
    CAM_LOGD("[%s] +", funcName);
    return AutoObject(funcName, [](const char* p)
    {
        CAM_LOGD("[%s] -", p);
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
        CAM_LOGD("[%s] %s, elapsed(ms):%.4f", funcName, p, duationTime.count());
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
namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {


/*******************************************************************************
* Alias.
********************************************************************************/
//using namespace vendor::mediatek::hardware::power::V2_0;


/*******************************************************************************
* Global Function.
*******************************************************************************/

MBOOL   copyImageBuffer(IImageBuffer *src, IImageBuffer *dst);

MBOOL   dumpToFile(IImageBuffer *buffer, const char *fmt, ...);

MUINT32 align(MUINT32 val, MUINT32 bits);

//MUINT   getSensorRawFmt(MINT32 sensorId);


/*******************************************************************************
* Class Define
*******************************************************************************/
/**
 * @brief slim wrapper of Powerhal, and it is a singleton
 */
#if 0
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
    //sp<IPower>           sPowerPtr;
    std::vector<Handle>  sHandles;
    //
    static constexpr MINT32      INVALID_HANDLE = -1;
    static constexpr MINT        TIMEOUT        = (10*1000);
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
    //PowerHalWrapper::Handle mHandle;
};
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Global Function Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
copyImageBuffer(IImageBuffer *src, IImageBuffer *dst)
{
    TRACE_FUNC_ENTER();
    MBOOL ret = MTRUE;

    if( !src || !dst )
    {
        MY_LOGE("Invalid buffers src=%p dst=%p", src, dst);
        ret = MFALSE;
    }
    else if( src->getImgSize() != dst->getImgSize() )
    {
        MY_LOGE("Mismatch buffer size src(%dx%d) dst(%dx%d)",
                src->getImgSize().w, src->getImgSize().h,
                dst->getImgSize().w, dst->getImgSize().h);
        ret = MFALSE;
    }
    else
    {
        unsigned srcPlane = src->getPlaneCount();
        unsigned dstPlane = dst->getPlaneCount();

        if( !srcPlane || !dstPlane ||
            (srcPlane != dstPlane && srcPlane != 1 && dstPlane != 1) )
        {
            MY_LOGE("Mismatch buffer plane src(%d) dst(%d)", srcPlane, dstPlane);
            ret = MFALSE;
        }
        for( unsigned i = 0; i < srcPlane; ++i )
        {
            if( !src->getBufVA(i) )
            {
                MY_LOGE("Invalid src plane[%d] VA", i);
                ret = MFALSE;
            }
        }
        for( unsigned i = 0; i < dstPlane; ++i )
        {
            if( !dst->getBufVA(i) )
            {
                MY_LOGE("Invalid dst plane[%d] VA", i);
                ret = MFALSE;
            }
        }

        if( srcPlane == 1 )
        {
            MY_LOGD("src: plane=1 size=%zu stride=%zu",
                    src->getBufSizeInBytes(0), src->getBufStridesInBytes(0));
            ret = MFALSE;
        }
        if( dstPlane == 1 )
        {
            MY_LOGD("dst: plane=1 size=%zu stride=%zu",
                    dst->getBufSizeInBytes(0), dst->getBufStridesInBytes(0));
            ret = MFALSE;
        }

        if( ret )
        {
            char *srcVA = NULL, *dstVA = NULL;
            size_t srcSize = 0;
            size_t dstSize = 0;
            size_t srcStride = 0;
            size_t dstStride = 0;

            for( unsigned i = 0; i < srcPlane && i < dstPlane; ++i )
            {
                if( i < srcPlane )
                {
                    srcVA = (char*)src->getBufVA(i);
                }
                if( i < dstPlane )
                {
                    dstVA = (char*)dst->getBufVA(i);
                }

                srcSize = src->getBufSizeInBytes(i);
                dstSize = dst->getBufSizeInBytes(i);
                srcStride = src->getBufStridesInBytes(i);
                dstStride = dst->getBufStridesInBytes(i);
                MY_LOGD("plane[%d] memcpy %p(%zu)=>%p(%zu)",
                          i, srcVA, srcSize, dstVA, dstSize);
                if( srcStride == dstStride )
                {
                    memcpy((void*)dstVA, (void*)srcVA, (srcSize <= dstSize) ? srcSize : dstSize );
                }
                else
                {
                    MY_LOGD("Stride: src(%zu) dst(%zu)", srcStride, dstStride);
                    size_t stride = (srcStride < dstStride) ? srcStride : dstStride;
                    unsigned height = dstSize / dstStride;
                    for( unsigned j = 0; j < height; ++j )
                    {
                        memcpy((void*)dstVA, (void*)srcVA, stride);
                        srcVA += srcStride;
                        dstVA += dstStride;
                    }
                }
            }
        }
    }

    TRACE_FUNC_EXIT();
    return ret;
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
#if 0
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
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Global Function and Variable Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
static const NodeID_T sPathMap[NUM_OF_PATH][2] =
{
    {NID_ROOT,      NID_ROOT},          // PID_ENQUE
    {NID_ROOT,      NID_YUV},           // PID_ROOT_TO_YUV
    {NID_ROOT,      NID_MULTIYUV},      // PID_ROOT_TO_MULTIFRAME
    {NID_ROOT,      NID_MDP},           // PID_ROOT_TO_MDP
    {NID_MULTIYUV,  NID_YUV},           // PID_MULTIFRAME_TO_YUV
    {NID_MULTIYUV,  NID_MDP} ,          // PID_MULTIFRAME_TO_MDP
    {NID_YUV,       NID_MDP},           // PID_YUV_TO_MDP
};

const char*
PathID2Name(PathID_T pid)
{
    switch(pid)
    {
    case PID_ENQUE:                 return "enque";
    case PID_ROOT_TO_YUV:           return "root_to_yuv";
    case PID_ROOT_TO_MULTIFRAME:      return "root_to_multiframe";
    case PID_ROOT_TO_MDP:           return "root_to_mdp";
    case PID_MULTIFRAME_TO_YUV:     return "multiframe_to_yuv";
    case PID_MULTIFRAME_TO_MDP:     return "multiframe_to_mdp";
    case PID_YUV_TO_MDP:            return "yuv_to_mdp";
    case PID_DEQUE:                 return "deque";

#if 0
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
    case PID_MULTIFRAME_TO_YUV2:    return "multiframe_to_yuv2";
    case PID_MULTIFRAME_TO_MDP_C:   return "multiframe_to_mdpc";
    case PID_MULTIFRAME_TO_BOKEH:   return "multiframe_to_bokeh";
    case PID_FUSION_TO_YUV:         return "fusion_to_yuv";
    case PID_FUSION_TO_YUV2:        return "fusion_to_yuv2";
    case PID_FUSION_TO_MDP:         return "fusion_to_mdp";
    case PID_DEPTH_TO_BOKEH:        return "depth_to_bokeh";
    case PID_DEPTH_TO_YUV2:         return "depth_to_yuv2";
    case PID_YUV_TO_MDP_C:          return "yuv_to_mdpc";
    case PID_YUV_TO_BOKEH:          return "yuv_to_bokeh";
    case PID_YUV_TO_YUV2:           return "yuv_to_yuv2";
    case PID_BOKEH_TO_MDP_B:        return "bokeh_to_mdpb";
    case PID_BOKEH_TO_YUV2:         return "bokeh_to_yuv2";
    case PID_BOKEH_TO_MDP:          return "bokeh_to_mdp";
    case PID_YUV2_TO_MDP:           return "yuv2_to_mdp";
#endif
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
    case FID_FILTER_PREVIEW:        return "filter_preview";
    case FID_FILTER_CAPTURE:        return "filter_capture";
    case FID_FILTER_MATRIX:         return "filter_matrix";
    case FID_AUTOPANORAMA:          return "auto_panorama";
    case FID_PORTRAIT:              return "portrait";
    case FID_VFB_PREVIEW:           return "vendor_fb_preview";
    case FID_VFB_CAPTURE:           return "vendor_fb_capture";
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

const NodeID_T* GetPath(PathID_T pid)
{
    if (pid < NUM_OF_PATH)
        return sPathMap[pid];
    return nullptr;
}

#if 0
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
    //
    IBooster* pBoost = isForceDisable ? static_cast<IBooster*>(new EmptyBoost(name)) : static_cast<IBooster*>(new CPUBooster(name));
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
        ret = sPowerPtr->scnReg();
        sHandles.push_back(ret);

        const size_t newCount = sHandles.size();
        MY_LOGD("create handle, powerPtr:%p, handle:%d, handleCount:%zu -> %zu",
            sPowerPtr.get(), ret, oldCount, newCount);
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
//  CPUBooster Implementation.
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
    TRACE_FUNC("ctor:%p, name:%s, hande:%d", this, mName.c_str(), mHandle);
}

CPUBooster::
~CPUBooster()
{
    TRACE_FUNC("dtor:%p, name:%s, hande:%d", this, mName.c_str(), mHandle);
    PowerHalWrapper::getInstance()->destroyhandle(getName(), mHandle);
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
#endif

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
HwStrideAlignment::ALIGNMENT =
{
    // ONLY for 10bits per pixel fmt, need to handle its alignment
    {10, lcm(10, HW_INPUT_STRIDE_LIMITATION)/10}
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
    if(HwStrideAlignment::ALIGNMENT.count(bitSize))
    {
        int alignment = HwStrideAlignment::ALIGNMENT[bitSize];
        if(currentAlignment == MSize(0, 0))
        {
            resultAlignment.w = alignment;
        }
        else
        {
            int lcm_align = lcm(currentAlignment.w, alignment);
            resultAlignment.w = lcm_align;
        }
    }
    return resultAlignment;
}


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

