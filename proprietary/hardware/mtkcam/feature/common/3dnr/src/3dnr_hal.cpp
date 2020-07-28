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

#define LOG_TAG "3dnr_hal"
//
#include <mtkcam/utils/std/Log.h>
#include <3dnr_hal.h>
#include "hal/inc/camera_custom_3dnr.h"
#include <sys/resource.h>
#include <cutils/properties.h>  // For property_get().
#include <cutils/atomic.h>
#include <mtkcam/drv/def/ispio_sw_scenario.h>
#include <mtkcam/feature/eis/eis_ext.h>
#include <mtkcam/aaa/IIspMgr.h>
#include <mtkcam/aaa/IHal3A.h>
#include <mtkcam/utils/std/Misc.h>

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;

using namespace NSImageio;
using namespace NSImageio::NSIspio;
using namespace NS3Av3;
using NSCam::Utils::SENSOR_TYPE_GYRO;

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

#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
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


#define NR3D_FORCE_GMV_ZERO     0
#define NR3D_NO_HW_POWER_OFF    0

#define MAKE_HAL3DNR_OBJ(id) \
    template <> Hal3dnrObj<id>* Hal3dnrObj<id>::spInstance = 0; \
    template <> Mutex Hal3dnrObj<id>::s_instMutex(::Mutex::PRIVATE);

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))
#define UNUSED(var) (void)(var)

/*******************************************************************************
*
********************************************************************************/
static hal3dnrBase *pHal3dnr = NULL;
static MINT32 clientCnt = 0;

MAKE_HAL3DNR_OBJ(0);
MAKE_HAL3DNR_OBJ(1);
MAKE_HAL3DNR_OBJ(2);
MAKE_HAL3DNR_OBJ(3);

/*******************************************************************************
*
********************************************************************************/

typedef enum
{
    NR3D_PATH_NOT_DEF           = 0x00,     // invalid path
    NR3D_PATH_RRZO              = 0x01,     // rrzo path
    NR3D_PATH_RRZO_CRZ          = 0x02,     // rrzo + EIS1.2 apply CMV crop
    NR3D_PATH_IMGO              = 0x03,     // ZSD preview IMGO path
} NR3D_PATH_ENUM;

struct NR3DAlignParam
{
public:
    MUINT32 onOff_onOfStX;
    MUINT32 onOff_onOfStY;
    MUINT32 onSiz_onWd;
    MUINT32 onSiz_onHt;
    MUINT32 u4VipiOffset_X;
    MUINT32 u4VipiOffset_Y;
    MUINT32 vipi_readW;     //in pixel
    MUINT32 vipi_readH;     //in pixel

   NR3DAlignParam()
       : onOff_onOfStX(0x0)
       , onOff_onOfStY(0x0)
       , onSiz_onWd(0x0)
       , onSiz_onHt(0x0)
       , u4VipiOffset_X(0x0)
       , u4VipiOffset_Y(0x0)
       , vipi_readW(0x0)
       , vipi_readH(0x0)
   {
   }
};

struct hal3dnrDebugParam {
    MINT32  mLogLevel;
    MINT32  mForce3DNR;// hal force support 3DNR
    MBOOL   mSupportZoom3DNR;
};

struct hal3dnrPolicyTable {
    MUINT32 (*policyFunc)(const NR3DHALParam &nr3dHalParam, const hal3dnrDebugParam &debugParam, const hal3dnrSavedFrameInfo &preSavedFrameInfo); // executable policy function
};

static void print_NR3DHALParam(const NR3DHALParam& nr3dHalParam, MINT32 mLogLevel)
{
    MY_LOGD_IF(mLogLevel >= 1, "=== mkdbg: print_NR3DHALParam: start ===");

    if (nr3dHalParam.pTuningData)
    {
        MY_LOGD_IF(mLogLevel >= 1, "\t pTuningData = %p", nr3dHalParam.pTuningData);
    }
    if (nr3dHalParam.p3A)
    {
        MY_LOGD_IF(mLogLevel >= 1, "\t p3A = %p", nr3dHalParam.p3A);
    }
    // frame generic
    MY_LOGD_IF(mLogLevel >= 1, "\t frameNo = %d", nr3dHalParam.frameNo);
    MY_LOGD_IF(mLogLevel >= 1, "\t iso = %d", nr3dHalParam.iso);
    MY_LOGD_IF(mLogLevel >= 1, "\t isoThreshold = %d", nr3dHalParam.isoThreshold);

    // imgi related
//    MY_LOGD_IF(mLogLevel >= 1, "\t imgiType= %d", nr3dHalParam.imgiType);
    MY_LOGD_IF(mLogLevel >= 1, "\t isCRZUsed = %d", nr3dHalParam.isCRZUsed);
    MY_LOGD_IF(mLogLevel >= 1, "\t isIMGO = %d", nr3dHalParam.isIMGO);

    // lmv related info
    MY_LOGD_IF(mLogLevel >= 1, "\t gmvX= %d => %d pixel", nr3dHalParam.GMVInfo.gmvX, nr3dHalParam.GMVInfo.gmvX/LMV_GMV_VALUE_TO_PIXEL_UNIT);
    MY_LOGD_IF(mLogLevel >= 1, "\t gmvY= %d => %d pixel", nr3dHalParam.GMVInfo.gmvY, nr3dHalParam.GMVInfo.gmvY/LMV_GMV_VALUE_TO_PIXEL_UNIT);
    MY_LOGD_IF(mLogLevel >= 1, "\t (confX,confY)=(%d,%d)", nr3dHalParam.GMVInfo.confX, nr3dHalParam.GMVInfo.confY);
    MY_LOGD_IF(mLogLevel >= 1, "\t x_int= %d", nr3dHalParam.GMVInfo.x_int);
    MY_LOGD_IF(mLogLevel >= 1, "\t y_int= %d", nr3dHalParam.GMVInfo.y_int);

    // vipi related
    if (nr3dHalParam.pIMGBufferVIPI == NULL)
    {
        MY_LOGD_IF(mLogLevel >= 1, "\t pIMGBufferVIPI == NULL");
    }
    else
    {
        MY_LOGD_IF(mLogLevel >= 1, "\t pIMGBufferVIPI: %p", nr3dHalParam.pIMGBufferVIPI);
        MY_LOGD_IF(mLogLevel >= 1, "\t\t vipi_image.w = %d", nr3dHalParam.pIMGBufferVIPI->getImgSize().w);
        MY_LOGD_IF(mLogLevel >= 1, "\t\t vipi_image.h = %d", nr3dHalParam.pIMGBufferVIPI->getImgSize().h);
        MY_LOGD_IF(mLogLevel >= 1, "\t\t vipi_format = %d, eImgFmt_YUY2: %d, eImgFmt_YV12: %d)", nr3dHalParam.pIMGBufferVIPI->getImgFormat(), eImgFmt_YUY2, eImgFmt_YV12);
        MY_LOGD_IF(mLogLevel >= 1, "\t\t vipi_strides = %zu", nr3dHalParam.pIMGBufferVIPI->getBufStridesInBytes(0));
    }

    // output related, ex: img3o
    MRect dst_resizer_rect;
    MY_LOGD_IF(mLogLevel >= 1, "\t destRect.w = %d", nr3dHalParam.dst_resizer_rect.s.w);
    MY_LOGD_IF(mLogLevel >= 1, "\t destRect.w = %d", nr3dHalParam.dst_resizer_rect.s.h);

    MY_LOGD_IF(mLogLevel >= 1, "=== mkdbg: print_NR3DHALParam: end ===");

}

static void print_NR3DParam(const NR3DParam &nr3dParam, MINT32 mLogLevel)
{
    MY_LOGD_IF(mLogLevel >= 1, "=== mkdbg: print_NR3DParam: start ===");

    MY_LOGD_IF(mLogLevel >= 1, "\t ctrl_onEn = %d", nr3dParam.ctrl_onEn);
    MY_LOGD_IF(mLogLevel >= 1, "\t onOff_onOfStX = %d", nr3dParam.onOff_onOfStX);
    MY_LOGD_IF(mLogLevel >= 1, "\t onOff_onOfStY = %d", nr3dParam.onOff_onOfStY);
    MY_LOGD_IF(mLogLevel >= 1, "\t onSiz_onWd = %d", nr3dParam.onSiz_onWd);
    MY_LOGD_IF(mLogLevel >= 1, "\t onSiz_onWd = %d", nr3dParam.onSiz_onHt);
    MY_LOGD_IF(mLogLevel >= 1, "\t vipi_offst = %d", nr3dParam.vipi_offst);
    MY_LOGD_IF(mLogLevel >= 1, "\t vipi_readW = %d", nr3dParam.vipi_readW);
    MY_LOGD_IF(mLogLevel >= 1, "\t vipi_readH = %d", nr3dParam.vipi_readH);

    MY_LOGD_IF(mLogLevel >= 1, "=== mkdbg: print_NR3DParam: end ===");
}

static NR3D_PATH_ENUM determine3DNRPath(const NR3DHALParam &nr3dHalParam)
{
    if (nr3dHalParam.isIMGO == MFALSE && nr3dHalParam.isCRZUsed == MTRUE)
    {
        // RRZ + CRZ
        return NR3D_PATH_RRZO_CRZ;
    }
    else if (nr3dHalParam.isIMGO == MTRUE)
    {
        // IMGO crop
        return NR3D_PATH_IMGO;
    }
    else
    {
        // RRZ only
        return NR3D_PATH_RRZO;
    }
}

static MUINT32 checkIso(const NR3DHALParam &nr3dHalParam, const hal3dnrDebugParam &debugParam, const hal3dnrSavedFrameInfo &preSavedFrameInfo)
{
    UNUSED(preSavedFrameInfo);

    MUINT32 errorStatus = NR3D_ERROR_NONE;
    MINT32 i4IsoThreshold = nr3dHalParam.isoThreshold;

    MY_LOGD_IF(debugParam.mLogLevel >= 1, "iso=%d, Poweroff threshold=%d, frame:%d",
        nr3dHalParam.iso, i4IsoThreshold, nr3dHalParam.frameNo);

    if (nr3dHalParam.iso < i4IsoThreshold)
    {
        errorStatus |= NR3D_ERROR_UNDER_ISO_THRESHOLD;
        return errorStatus;
    }

    return errorStatus;
}

static MUINT32 checkVipiImgiFrameSize(const NR3DHALParam &nr3dHalParam, const hal3dnrDebugParam &debugParam, const hal3dnrSavedFrameInfo &preSavedFrameInfo)
{
    MUINT32 errorStatus = NR3D_ERROR_NONE;

    if (nr3dHalParam.pIMGBufferVIPI == NULL)
    {
        errorStatus |= NR3D_ERROR_INVALID_PARAM;
        return errorStatus;
    }

    MSize vipiFrameSize = nr3dHalParam.pIMGBufferVIPI->getImgSize();
    const MRect &pImg3oFrameRect = nr3dHalParam.dst_resizer_rect;

    // W/H of buffer (i.e. Current frame size) is determined, so check previous vs. current frame size for 3DNR.
    if (pImg3oFrameRect.s == vipiFrameSize)
    {
        return errorStatus;
    }
    else
    {
        if (debugParam.mSupportZoom3DNR)
        {
            print_NR3DHALParam(nr3dHalParam, debugParam.mLogLevel);

            // Zoom case
            if (vipiFrameSize.w > nr3dHalParam.dst_resizer_rect.s.w)
            {
                MY_LOGD_IF(debugParam.mLogLevel >= 1, "!!WARN: mkdbg_zoom: VIPI > IMGI: (%d, %d) -> (%d, %d)",
                    nr3dHalParam.pIMGBufferVIPI->getImgSize().w,
                    nr3dHalParam.pIMGBufferVIPI->getImgSize().h,
                    nr3dHalParam.dst_resizer_rect.s.w,
                    nr3dHalParam.dst_resizer_rect.s.h
                    );
            }
            else if (vipiFrameSize.w < nr3dHalParam.dst_resizer_rect.s.w)
            {
                MY_LOGD_IF(debugParam.mLogLevel >= 1, "!!WARN: mkdbg_zoom: IMGI > VIPI: (%d, %d) -> (%d, %d)",
                    nr3dHalParam.pIMGBufferVIPI->getImgSize().w,
                    nr3dHalParam.pIMGBufferVIPI->getImgSize().h,
                    nr3dHalParam.dst_resizer_rect.s.w,
                    nr3dHalParam.dst_resizer_rect.s.h
                    );
            }

            MINT32 nr3dPathID = determine3DNRPath(nr3dHalParam);
            switch(nr3dPathID)
            {
                case NR3D_PATH_RRZO: // === Rule: IMGO --> support IMGO-only, NOT support RRZO/IMGO switch ===
                    break;
                case NR3D_PATH_RRZO_CRZ: // === Rule:  RRZ + CRZ --> 3DNR OFF ===
                    errorStatus |= NR3D_ERROR_NOT_SUPPORT;
                    return errorStatus;
                case NR3D_PATH_IMGO: // === Rule: IMGO --> support IMGO-only, NOT support RRZO/IMGO switch ===
                    break;
                default:
                    MY_LOGD("invalid path ID(%d)",nr3dPathID);
                    errorStatus |= NR3D_ERROR_NOT_SUPPORT;
                    return errorStatus;
            }

            if (preSavedFrameInfo.isCRZUsed != nr3dHalParam.isCRZUsed || preSavedFrameInfo.isIMGO != nr3dHalParam.isIMGO)
            {
                // 2017/03/29: Rule: IMGO/RRZO input switch: 3DNR default on by Algo's request
                MBOOL isInputChg3DNROn = ::property_get_int32("vendor.debug.3dnr.inputchg.on", 1);
                if (isInputChg3DNROn)
                {
                    MY_LOGD("RRZO/IMGO input change: nr3dPathID: %d, CRZUsed=%d -> %d, isIMGO=%d->%d --> 3DNR on",
                        nr3dPathID, preSavedFrameInfo.isCRZUsed, nr3dHalParam.isCRZUsed, preSavedFrameInfo.isIMGO, nr3dHalParam.isIMGO);
                }
                else
                {
                    MY_LOGD("RRZO/IMGO input change: nr3dPathID: %d, CRZUsed=%d -> %d, isIMGO=%d->%d --> 3DNR off",
                        nr3dPathID, preSavedFrameInfo.isCRZUsed, nr3dHalParam.isCRZUsed, preSavedFrameInfo.isIMGO, nr3dHalParam.isIMGO);
                    errorStatus |= NR3D_ERROR_INPUT_SRC_CHANGE;
                    return errorStatus;
                }
            }

            // === Binning: Sensor / Frontal ===
            if (nr3dHalParam.isBinning)
            {
                // TODO:
            }
        }
        else
        {
            // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
            errorStatus |= NR3D_ERROR_FRAME_SIZE_CHANGED;
            return errorStatus;
        }
    }

    return errorStatus;
}

static const hal3dnrPolicyTable _ghal3dnrPolicyTable[] = {
    { checkIso },
    { checkVipiImgiFrameSize }
};

static MUINT32 check3DNRPolicy(const NR3DHALParam &nr3dHalParam, const hal3dnrDebugParam &debugParam, const  hal3dnrSavedFrameInfo &preSavedFrameInfo)
{
    MUINT32 errorStatus = NR3D_ERROR_NONE;
    MINT32 table_size = sizeof(_ghal3dnrPolicyTable)/sizeof(hal3dnrPolicyTable);

    // check policy
    for (MINT32 i = 0; i < table_size; i++)
    {
        errorStatus = _ghal3dnrPolicyTable[i].policyFunc(nr3dHalParam, debugParam,  preSavedFrameInfo);

        if (NR3D_ERROR_NONE != errorStatus)
        {
            break;
        }
    }

    return errorStatus;
}

static void calCMV(const hal3dnrSavedFrameInfo &preSavedFrameInfo, NR3DGMVInfo &GMVInfo)
{
    // For EIS 1.2 (use CMV). gmv_crp (t) = gmv(t) - ( cmv(t) - cmv(t-1) )

    // Use GMV and CMV
    GMVInfo.gmvX = (GMVInfo.gmvX - (GMVInfo.x_int - preSavedFrameInfo.CmvX));
    GMVInfo.gmvY = (GMVInfo.gmvY - (GMVInfo.y_int - preSavedFrameInfo.CmvY));
}

static void calGMV(const NR3DHALParam &nr3dHalParam, MINT32 force3DNR, const hal3dnrSavedFrameInfo &preSavedFrameInfo,
    NR3DGMVInfo &GMVInfo)
{
    UNUSED(force3DNR);

    // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_w must be even, so make mNmvX even. Discussed with James Liang.
    // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_h must be even when image format is 420, so make mNmvX even. Discussed with TC & Christopher.
    GMVInfo.gmvX = (-(GMVInfo.gmvX) / LMV_GMV_VALUE_TO_PIXEL_UNIT);
    GMVInfo.gmvY = (-(GMVInfo.gmvY) / LMV_GMV_VALUE_TO_PIXEL_UNIT);

    MINT32 nr3dPathID = determine3DNRPath(nr3dHalParam);

    switch(nr3dPathID)
    {
        case NR3D_PATH_RRZO:
        case NR3D_PATH_IMGO:
            // Use GMV only.
            break;
        case NR3D_PATH_RRZO_CRZ:
            calCMV(preSavedFrameInfo, GMVInfo);
            break;
        default:
            MY_LOGE("invalid path ID(%d)", nr3dPathID);
            break;
    }

    GMVInfo.gmvX = GMVInfo.gmvX & ~1;    // Make it even.
    GMVInfo.gmvY = GMVInfo.gmvY & ~1;    // Make it even.
}

MUINT32 handleState(MUINT32 errorStatus, MINT32 force3DNR, NR3D_STATE_ENUM &stateMachine)
{
    MUINT32 result = errorStatus;

    if (NR3D_ERROR_NONE == result)
    {
        if (stateMachine == NR3D_STATE_PREPARING) // Last frame is NR3D_STATE_PREPARING.
        {
            stateMachine = NR3D_STATE_WORKING;   // NR3D, IMG3O, VIPI all enabled.
        }
        else if (stateMachine == NR3D_STATE_STOP)
        {
            stateMachine = NR3D_STATE_PREPARING;
        }

        if (force3DNR)
        {
            if (::property_get_int32("vendor.camera.3dnr.forceskip", 0))
            {
                // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
                result |= NR3D_ERROR_FORCE_SKIP;
                if (stateMachine == NR3D_STATE_WORKING) stateMachine = NR3D_STATE_PREPARING;
            }
        }
    }
    else
    {
        if (stateMachine == NR3D_STATE_WORKING) stateMachine = NR3D_STATE_PREPARING;
    }

    return result;
}

static MBOOL getNR3DParam(const NR3DHALParam &nr3dHalParam, NR3DAlignParam &nr3dAlignParam,
    NR3DParam &outNr3dParam)
{
    MUINT32 u4PixelToBytes = 0;
    MINT imgFormat = nr3dHalParam.pIMGBufferVIPI->getImgFormat();
    size_t stride = nr3dHalParam.pIMGBufferVIPI->getBufStridesInBytes(0);

    // Calculate u4PixelToBytes.
    if (imgFormat == eImgFmt_YUY2) u4PixelToBytes = 2;
    else if (imgFormat == eImgFmt_YV12) u4PixelToBytes = 1;
    // Calculate VIPI start addr offset.
    MUINT32 u4VipiOffset_X = nr3dAlignParam.u4VipiOffset_X;
    MUINT32 u4VipiOffset_Y = nr3dAlignParam.u4VipiOffset_Y;
    // u4VipiStartAddrOffset = u4VipiOffset_Y * minput.mBuffer->getBufStridesInBytes(0) + u4VipiOffset_X * u4PixelToBytes;
    MUINT32 vipi_offst = u4VipiOffset_Y * stride + u4VipiOffset_X * u4PixelToBytes;    //in byte

    // save NR3D setting into NR3DParam
    // handle 2 bytes alignment in isp_mgr_nr3d
#if 0 // TODO: remove this
    outNr3dParam.vipi_offst = vipi_offst & ~1;
    outNr3dParam.vipi_readW = nr3dAlignParam.vipi_readW & ~1;
    outNr3dParam.vipi_readH = nr3dAlignParam.vipi_readH & ~1;
    outNr3dParam.onSiz_onWd = nr3dAlignParam.onSiz_onWd & ~1;
    outNr3dParam.onSiz_onHt = nr3dAlignParam.onSiz_onHt & ~1;
    outNr3dParam.onOff_onOfStX = nr3dAlignParam.onOff_onOfStX & ~1;
    outNr3dParam.onOff_onOfStY = nr3dAlignParam.onOff_onOfStY & ~1;
#else
    outNr3dParam.vipi_offst = vipi_offst;
    outNr3dParam.vipi_readW = nr3dAlignParam.vipi_readW;
    outNr3dParam.vipi_readH = nr3dAlignParam.vipi_readH;
    outNr3dParam.onSiz_onWd = nr3dAlignParam.onSiz_onWd;
    outNr3dParam.onSiz_onHt = nr3dAlignParam.onSiz_onHt;
    outNr3dParam.onOff_onOfStX = nr3dAlignParam.onOff_onOfStX;
    outNr3dParam.onOff_onOfStY = nr3dAlignParam.onOff_onOfStY;
#endif
    outNr3dParam.ctrl_onEn = 1;

    return MTRUE;
}

static MBOOL handleFrameAlign(const NR3DHALParam &nr3dHalParam, const NR3DGMVInfo &GMVInfo,
    NR3DAlignParam &outNr3dAlignParam)
{
    // Config VIPI for 3DNR previous frame input.
    MUINT32 u4VipiOffset_X = 0, u4VipiOffset_Y = 0;
    MUINT32 u4Nr3dOffset_X = 0, u4Nr3dOffset_Y = 0;
    MSize imgSize;
    MINT32 mvX = GMVInfo.gmvX;
    MINT32 mvY = GMVInfo.gmvY;

    // Calculate VIPI Offset X/Y according to NMV X/Y.
    u4VipiOffset_X = ((mvX >= 0) ? (mvX): (0));
    u4VipiOffset_Y = ((mvY >= 0) ? (mvY): (0));
    // Calculate NR3D Offset X/Y according to NMV X/Y.
    u4Nr3dOffset_X = ((mvX >= 0) ? (0) : (-mvX));
    u4Nr3dOffset_Y = ((mvY >= 0) ? (0) : (-mvY));

    MUINT32 vipiW = nr3dHalParam.pIMGBufferVIPI->getImgSize().w;
    MUINT32 vipiH = nr3dHalParam.pIMGBufferVIPI->getImgSize().h;

    // Calculate VIPI valid region w/h.
    imgSize.w = vipiW - abs(mvX);  // valid region w
    imgSize.h = vipiH - abs(mvY);  // valid region h

    // save align info into NR3DAlignParam
    outNr3dAlignParam.onOff_onOfStX = u4Nr3dOffset_X;
    outNr3dAlignParam.onOff_onOfStY = u4Nr3dOffset_Y;

    outNr3dAlignParam.onSiz_onWd = imgSize.w;
    outNr3dAlignParam.onSiz_onHt = imgSize.h;

    outNr3dAlignParam.u4VipiOffset_X = u4VipiOffset_X;
    outNr3dAlignParam.u4VipiOffset_Y = u4VipiOffset_Y;

    outNr3dAlignParam.vipi_readW = imgSize.w;    //in pixel
    outNr3dAlignParam.vipi_readH = imgSize.h;    //in pixel

    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
static void dumpVIPIBuffer(NSCam::IImageBuffer *pIMGBufferVIPI, MUINT32 requestNo)
{
    static MSize s_con_write_size;
    static MINT32 s_con_write_countdown = 0;
    if (!s_con_write_countdown && (s_con_write_size != pIMGBufferVIPI->getImgSize()))
    {
        s_con_write_size = pIMGBufferVIPI->getImgSize();
        s_con_write_countdown = 5;
    }

    if (s_con_write_countdown)
    {
        MINT imgFormat = pIMGBufferVIPI->getImgFormat();
        MUINT32 u4PixelToBytes = 0;

        if (imgFormat == eImgFmt_YUY2) u4PixelToBytes = 2;
        if (imgFormat == eImgFmt_YV12) u4PixelToBytes = 1;

        if (NSCam::Utils::makePath("/sdcard/vipi_dump/",0660) == false)
        {
            MY_LOGW("makePath(\"/sdcard/vipi_dump\") error");
        }
        char filename[256] = {0};
        sprintf(filename, "/sdcard/vipi_dump/vipi_%dx%d_S%zu_p2b_%d_N%d.yuv",
            pIMGBufferVIPI->getImgSize().w,
            pIMGBufferVIPI->getImgSize().h,
            pIMGBufferVIPI->getBufStridesInBytes(0),
            u4PixelToBytes,
            requestNo);
        pIMGBufferVIPI->saveToFile(filename);
        s_con_write_countdown--;
    }
}

/*******************************************************************************
*
********************************************************************************/

hal3dnrBase*
Hal3dnr::
getInstance()
{
    clientCnt++;
    MY_LOGD("clientCnt:%d \n", clientCnt);

    if (pHal3dnr == NULL) {
        pHal3dnr = new Hal3dnr();
    }
    return pHal3dnr;
}

/*******************************************************************************
*
********************************************************************************/
hal3dnrBase*
Hal3dnr::
getInstance(char const *userName, const MUINT32 sensorIdx)
{
    MY_LOGD("%s sensorIdx %d", userName, sensorIdx);

    switch(sensorIdx)
    {
        case 0 : return Hal3dnrObj<0>::GetInstance(userName);
        case 1 : return Hal3dnrObj<1>::GetInstance(userName);
        case 2 : return Hal3dnrObj<2>::GetInstance(userName);
        case 3 : return Hal3dnrObj<3>::GetInstance(userName);
        default :
            MY_LOGW("Current limit is 4 sensors, use 0");
            return Hal3dnrObj<0>::GetInstance(userName);
    }
}


/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3dnr::destroyInstance(char const *userName, const MUINT32 sensorIdx)
{
    Mutex::Autolock lock(mLock);
    MY_LOGD("%s sensorIdx %d", userName, sensorIdx);

    //====== Check Reference Count ======
    if(mUsers <= 0)
    {
        MY_LOGD_IF(mLogLevel >= 1, "mSensorIdx(%u) has 0 user", mSensorIdx);

        switch(mSensorIdx)
        {
             case 0 :
                Hal3dnrObj<0>::destroyInstance(userName);
                break;
             case 1 :
                Hal3dnrObj<1>::destroyInstance(userName);
                break;
             case 2 :
                Hal3dnrObj<2>::destroyInstance(userName);
                break;
             case 3 :
                Hal3dnrObj<3>::destroyInstance(userName);
                break;
             default :
                 MY_LOGW(" %d is not exist", mSensorIdx);
        }
    }

}

/*******************************************************************************
*
********************************************************************************/
Hal3dnr::Hal3dnr()
: mSensorIdx(0)
{
    mMaxInputImageSize = MSize(0,0);
    mPrevFrameWidth = 0;
    mPrevFrameHeight = 0;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;
    m3dnrGainZeroCount = 0;
    m3dnrErrorStatus = 0;
    m3dnrStateMachine = NR3D_STATE_STOP;
    // TODO: use static instead of new
    mpNr3dParam = new NR3DParam;
    mIsCMVMode = MFALSE;
    mLogLevel = 0;
    mForce3DNR = 0;
    mSupportZoom3DNR = MFALSE;
    mUsers = 0;
}

/*******************************************************************************
*
********************************************************************************/
Hal3dnr::Hal3dnr(const MUINT32 sensorIdx)
: mSensorIdx(sensorIdx)
{
    mMaxInputImageSize = MSize(0,0);
    mPrevFrameWidth = 0;
    mPrevFrameHeight = 0;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;
    m3dnrGainZeroCount = 0;
    m3dnrErrorStatus = 0;
    m3dnrStateMachine = NR3D_STATE_STOP;
    mpNr3dParam = new NR3DParam;
    mIsCMVMode = MFALSE;
    mLogLevel = 0;
    mForce3DNR = 0;
    mSupportZoom3DNR = MFALSE;
    mUsers = 0;
}



/*******************************************************************************
*
********************************************************************************/
Hal3dnr::~Hal3dnr()
{
    delete mpNr3dParam; // Return allocated memory.
}

MVOID Hal3dnr::setCMVMode(MBOOL useCMV)
{
    mIsCMVMode = useCMV;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Hal3dnr::destroyInstance()
{
    MY_LOGD("destroyInstance \n");
    if (clientCnt == 0 && pHal3dnr) {
        delete pHal3dnr;
        pHal3dnr = NULL;
    }
    clientCnt--;

    MY_LOGD("pHal3dnr=%p, clientCnt:%d \n", pHal3dnr, clientCnt);

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::init(MINT32 force3DNR)
{
    Mutex::Autolock lock(mLock);

    FUNC_START;

    MY_LOGD("m3dnrStateMachine=%d->NR3D_STATE_PREPARING", m3dnrStateMachine);

    //====== Check Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        MY_LOGD("snesorIdx(%u) has %d users", mSensorIdx, mUsers);
        return MTRUE;
    }

    //mpNr3dParam = new NR3D;
    memset(mpNr3dParam, 0, sizeof(NR3DParam));

    mPrevFrameWidth         = 0;
    mPrevFrameHeight        = 0;
    m3dnrGainZeroCount      = 0;
    m3dnrErrorStatus        = NR3D_ERROR_NONE;
    m3dnrStateMachine       = NR3D_STATE_PREPARING;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;

    mLogLevel = ::property_get_int32("vendor.camera.3dnr.log.level", 0);
    mForce3DNR = force3DNR;
    mSupportZoom3DNR = ::property_get_int32("vendor.debug.3dnr.zoom", 1); // set zoom_3dnr default ON

#if 0 // if do3dnrFlow needs Gyro (e.g. old platform/VSDOF)
    if (mpSensorProvider == NULL)
    {
        mpSensorProvider = NSCam::Utils::SensorProvider::createInstance("Hal3dnr");
        if (!mpSensorProvider->enableSensor(SENSOR_TYPE_GYRO, 10))
            mpSensorProvider = NULL;
    }
#endif

    //====== Increase User Count ======

    android_atomic_inc(&mUsers);

    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::uninit()
{
    Mutex::Autolock lock(mLock);

    FUNC_START;

    MY_LOGD("m3dnrStateMachine=%d->NR3D_STATE_PREPARING", m3dnrStateMachine);
    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        MY_LOGD("mSensorIdx(%u) has 0 user",mSensorIdx);
        return MTRUE;
    }
    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    //delete mpNr3dParam; // Return allocated memory.

    mPrevFrameWidth         = 0;
    mPrevFrameHeight        = 0;
    m3dnrGainZeroCount      = 0;
    m3dnrErrorStatus        = NR3D_ERROR_NONE;
    m3dnrStateMachine       = NR3D_STATE_PREPARING;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;

    if (mpSensorProvider != NULL)
    {
        mpSensorProvider->disableSensor(SENSOR_TYPE_GYRO);
        mpSensorProvider = NULL;
    }

    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::prepare(MUINT32 frameNo, MINT32 iso)
{
    FUNC_START;

    char aInputValue[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32  i4TempInputValue = 0;
    MUINT32 u4Img3oOffset_X  = 0;
    MUINT32 u4Img3oOffset_Y  = 0;
    MUINT32 eisFeBlock      = 0;
    MUINT32 u4Img3oWidth    = 0;
    MUINT32 u4Img3oHeight   = 0;
    MUINT32 u4VipiWidth     = 0;
    MUINT32 u4VipiHeight    = 0;
    MUINT32 u4VipiStartAddrOffset = 0;
    MSize imgSize, TempImgSize;

    /************************************************************************/
    /*                          Preprocessing                               */
    /************************************************************************/

    //*****************STEP 1
    //////////////////////////////////////////////////////////////////////////
    // 3DNR State Machine operation                                         //
    //////////////////////////////////////////////////////////////////////////
    NR3D_STATE_ENUM e3dnrStateMachine = m3dnrStateMachine;
    if (e3dnrStateMachine == NR3D_STATE_PREPARING) // Last frame is NR3D_STATE_PREPARING.
    {
        MY_LOGD_IF(mLogLevel >= 1, "m3dnrStateMachine=(NR3D_STATE_PREPARING->NR3D_STATE_WORKING)");
        m3dnrStateMachine = NR3D_STATE_WORKING;   // NR3D, IMG3O, VIPI all enabled.
    }
    MY_LOGD_IF(mLogLevel >= 1, "STEP 1,2: m3dnrStateMachine=(%d->%d), frame:%d", e3dnrStateMachine,
        m3dnrStateMachine, frameNo);
    //*****************STEP 2
    // Reset m3dnrErrorStatus.
    m3dnrErrorStatus = NR3D_ERROR_NONE;

#if 0 //deprecated
    //*****************STEP 3
    //////////////////////////////////////////////////////////////////////////
    //  3DNR HW module on/off according Nr3dGain                            //
    //////////////////////////////////////////////////////////////////////////
    MUINT32 u43dnrHwPowerOffThreshold   = get_3dnr_hw_power_off_threshold();
    MUINT32 u43dnrHwPowerReopenDelay    = get_3dnr_hw_power_reopen_delay();
    MINT32 i4Nr3dGain = 0;
    MY_LOGD_IF(mLogLevel >= 2, "u43dnrHwPowerOffThreshold=%d, u43dnrHwPowerReopenDelay=%d, m3dnrGainZeroCount=%d frame:%d",
        u43dnrHwPowerOffThreshold, u43dnrHwPowerReopenDelay, m3dnrGainZeroCount, frameNo);

    MINT32 i4IsoThreshold = get_3dnr_off_iso_threshold(mForce3DNR);

    if (iso > i4IsoThreshold)
    {
        if (m3dnrGainZeroCount > 0)       // limit at 0.
            m3dnrGainZeroCount--;
    }
    else    // i4Nr3dGain0 is 0.
    {
        m3dnrGainZeroCount++;
        if (m3dnrGainZeroCount > u43dnrHwPowerOffThreshold)       // Cap at u43dnrHwPowerOffThreshold.
            m3dnrGainZeroCount = u43dnrHwPowerOffThreshold;
    }

    #if NR3D_NO_HW_POWER_OFF
    m3dnrGainZeroCount = 0; // For m3dnrGainZeroCount to be 0, so it won't change m3dnrStateMachine.
    #endif  // NR3D_NO_HW_POWER_OFF

    if (m3dnrGainZeroCount >= u43dnrHwPowerOffThreshold)
    {
        m3dnrStateMachine = NR3D_STATE_STOP;
    }
    else if (m3dnrGainZeroCount > (u43dnrHwPowerOffThreshold - u43dnrHwPowerReopenDelay))
    {
        // StateMachine stays the same.
    }
    else    // (u43dnrHwPowerOffThreshold - u43dnrHwPowerReopenDelay) > m3dnrGainZeroCount > 0
    {
        if (m3dnrStateMachine == NR3D_STATE_STOP)
        {
            m3dnrStateMachine = NR3D_STATE_PREPARING;
            m3dnrGainZeroCount = 0; // Reset m3dnrGainZeroCount.
        }
    }
    MY_LOGD_IF(mLogLevel >= 2, "STEP 3: StateMachine=%d, i4Nr3dGain=%d, Zero gain count=%d, Poweroff threshold=%d, frame:%d",
        m3dnrStateMachine, i4Nr3dGain, m3dnrGainZeroCount, u43dnrHwPowerOffThreshold, frameNo);
#else

    MINT32 i4IsoThreshold = NR3DCustom::get_3dnr_off_iso_threshold(mForce3DNR);

    if (iso < i4IsoThreshold)
    {
        m3dnrStateMachine = NR3D_STATE_STOP;
    }
    else
    {
        if (m3dnrStateMachine == NR3D_STATE_STOP)
        {
            m3dnrStateMachine = NR3D_STATE_PREPARING;
            m3dnrGainZeroCount = 0; // Reset m3dnrGainZeroCount.
        }
    }
    MY_LOGD_IF(mLogLevel >= 1, "STEP 3: StateMachine=%d, iso=%d, Poweroff threshold=%d, frame:%d",
        m3dnrStateMachine, iso, i4IsoThreshold, frameNo);
#endif

    FUNC_END;
    return MTRUE;
}


MBOOL
Hal3dnr::setGMV(MUINT32 frameNo, MINT32 gmvX, MINT32 gmvY, MINT32 cmvX_Int, MINT32 cmvY_Int)
{
    FUNC_START;

    //*****************STEP 4
    //////////////////////////////////////////////////////////////////////////
    //  3DNR GMV Calculation                                                //
    //////////////////////////////////////////////////////////////////////////
    MINT32 i4TempNmvXFromQueue = 0, i4TempNmvYFromQueue = 0;
    MINT32 i4TempX = 0, i4TempY = 0;
    //MINT32 i4GmvThreshold = get_3dnr_gmv_threshold(mForce3DNR);

    i4TempNmvXFromQueue = (-(gmvX) / LMV_GMV_VALUE_TO_PIXEL_UNIT);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_w must be even, so make mNmvX even. Discussed with James Liang.
    i4TempNmvYFromQueue = (-(gmvY) / LMV_GMV_VALUE_TO_PIXEL_UNIT);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_h must be even when image format is 420, so make mNmvX even. Discussed with TC & Christopher.

    if (mForce3DNR)
    {
        MINT32 value = ::property_get_int32("vendor.camera.3dnr.forcegmv.enable", 0);
        if (value)
        {
            i4TempNmvXFromQueue = ::property_get_int32("vendor.camera.3dnr.forcegmv.x", 0);

            i4TempNmvYFromQueue = ::property_get_int32("vendor.camera.3dnr.forcegmv.y", 0);

            MY_LOGD_IF(mLogLevel >= 1, "Force GMV X/Y (%d, %d)", i4TempNmvXFromQueue, i4TempNmvYFromQueue);
        }
    }

    #if (NR3D_FORCE_GMV_ZERO)   // Force GMV to be 0.
    mNmvX = 0;
    mNmvY = 0;
    #else   // Normal flow.

//    ::property_get_int32("vendor.camera.3dnr.usecmv.enable", 1);

//    property_get("vendor.camera.3dnr.usecmv.enable", aInputValue, "1");
//    if (aInputValue[0] == '1')   // For EIS 1.2 (use CMV). gmv_crp (t) = gmv(t) - ( cmv(t) - cmv(t-1) )
    if (mIsCMVMode)   // For EIS 1.2 (use CMV). gmv_crp (t) = gmv(t) - ( cmv(t) - cmv(t-1) )
    {
        // Use GMV and CMV
        mCmvX = cmvX_Int;    // Curr frame CMV X. Make it even.
        mCmvY = cmvY_Int;    // Curr frame CMV Y. Make it even.
        mNmvX = (i4TempNmvXFromQueue - (mCmvX - mPrevCmvX)) & ~1;    // Make it even.
        mNmvY = (i4TempNmvYFromQueue - (mCmvY - mPrevCmvY)) & ~1;    // Make it even.
        i4TempX = mCmvX - mPrevCmvX;
        i4TempY = mCmvY - mPrevCmvY;

        mPrevCmvX = mCmvX;                   // Recore last frame CMV X.
        mPrevCmvY = mCmvY;                   // Recore last frame CMV Y.

        // Use GMV only. (david 2015.07.03)
        //mNmvX = i4TempNmvXFromQueue & ~1;    // Make it even.
        //mNmvY = i4TempNmvYFromQueue & ~1;    // Make it even.
    }
    else   // For EIS 2.0 (use GMV)
    {
        // Use GMV only.
        mNmvX = i4TempNmvXFromQueue & ~1;    // Make it even.
        mNmvY = i4TempNmvYFromQueue & ~1;    // Make it even.

        mCmvX = 0;
        mCmvY = 0;
        mPrevCmvX = 0;                   // Recore last frame CMV X.
        mPrevCmvY = 0;                   // Recore last frame CMV Y.
    }
    #endif  // NR3D_FORCE_GMV_ZERO

#if 0
    if ( (abs(mNmvX) >= i4GmvThreshold) || (abs(mNmvY) >= i4GmvThreshold))
    {
        m3dnrErrorStatus |= NR3D_ERROR_GMV_TOO_LARGE;
        if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
    }
#endif

    MY_LOGD_IF(mLogLevel >= 0, "STEP 4: mSensorIdx=%u gmv cal,ST=%d, gmv(x,y)=(%5d,%5d),CmvX/Y(%5d,%5d),NmvX/Y(%5d,%5d), (cmv diff %5d,%5d), frame:%d",
        mSensorIdx, m3dnrStateMachine, gmvX, gmvY, cmvX_Int, cmvY_Int, mNmvX, mNmvY, i4TempX, i4TempY, frameNo);

    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::checkIMG3OSize(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH)
{
    FUNC_START;

    //*****************STEP 5
    //////////////////////////////////////////////////////////////////////////
    //  Calculate target width/height to set IMG3O                          //
    //////////////////////////////////////////////////////////////////////////
    /* Calculate P2A output width and height */

    //david: in hal3 img3o size is the same to imgi in

    //*****************STEP 6
    // W/H of buffer (i.e. Current frame size) is determined, so check previous vs. current frame size for 3DNR.
    if ( (mPrevFrameWidth  != imgiW) || (mPrevFrameHeight != imgiH) )
    {
        MY_LOGD_IF(mLogLevel >= 1, "PrevFrameW/H(%d,%d),imgiW/H(%d,%d), frame:%d, m3dnrStateMachine=%d",
        mPrevFrameWidth, mPrevFrameHeight, imgiW, imgiH, frameNo, m3dnrStateMachine);
        m3dnrErrorStatus |= NR3D_ERROR_FRAME_SIZE_CHANGED;
        // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
        if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;
    }

    //*****************STEP 7
    //resize to real output size
    //david 3dnr only can skip this step
    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::setVipiParams(MBOOL isVIPIIn, MUINT32 vipiW, MUINT32 vipiH, MINT imgFormat, size_t stride)
{
    FUNC_START;
    char aInputValue[PROPERTY_VALUE_MAX] = {'\0'};

    if (isVIPIIn)
    {
        // Config VIPI for 3DNR previous frame input.
        MUINT32 u4VipiOffset_X = 0;
        MUINT32 u4VipiOffset_Y = 0;
        MUINT32 u4PixelToBytes = 0;
        MSize imgSize, tempImgSize;

        // Calculate VIPI Start Address = nmv_x + nmv_y * vipi_stride. Unit: bytes.
        //     Calculate Offset X/Y according to NMV X/Y.
        u4VipiOffset_X = (mNmvX >= 0) ? (mNmvX): (0);
        u4VipiOffset_Y = (mNmvY >= 0) ? (mNmvY): (0);
        //     Calculate u4PixelToBytes.
        if (imgFormat == eImgFmt_YUY2) u4PixelToBytes = 2;
        if (imgFormat == eImgFmt_YV12) u4PixelToBytes = 1;
        //     Calculate VIPI start addr offset.
        // u4VipiStartAddrOffset = u4VipiOffset_Y * minput.mBuffer->getBufStridesInBytes(0) + u4VipiOffset_X * u4PixelToBytes;
        mpNr3dParam->vipi_offst = u4VipiOffset_Y * stride + u4VipiOffset_X * u4PixelToBytes;    //in byte
        MY_LOGD_IF(mLogLevel >= 2, "vipi offset=%d,(xy=%d,%d), stride=%zu, u4PixelToBytes=%d",mpNr3dParam->vipi_offst, u4VipiOffset_X, u4VipiOffset_Y, stride, u4PixelToBytes);

        //     Calculate VIPI valid region w/h.
        tempImgSize.w = vipiW - abs(mNmvX);  // valid region w
        tempImgSize.h = vipiH - abs(mNmvY);  // valid region h
        imgSize.w = tempImgSize.w & ~1;  // valid region w
        imgSize.h = tempImgSize.h & ~1;  // valid region h

        mpNr3dParam->vipi_readW = imgSize.w;    //in pixel
        mpNr3dParam->vipi_readH = imgSize.h;    //in pixel

        //david for test, force set stateMachine to Wokring
        //m3dnrStateMachine = NR3D_STATE_WORKING;

        // Check whether current frame size is equal to last frame size.
        if (mForce3DNR)
        {
            MINT32 value = ::property_get_int32("vendor.camera.3dnr.forceskip", 0);
            if (value)
            {
                m3dnrErrorStatus |= NR3D_ERROR_FORCE_SKIP;
                if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
            }
        }

        if (m3dnrStateMachine == NR3D_STATE_WORKING)
        {
            //enqueParams.mvIn.push_back(minput);    // Only push_back VIPI when N3RD state machine is NR3D_STATE_WORKING.
            //LOG_DBG("push_back VIPI buffer.");

            MY_LOGD_IF(mLogLevel >= 2, "[P2A sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn)",
                    m3dnrStateMachine,
                    m3dnrErrorStatus
            );
            mpNr3dParam->onSiz_onWd = imgSize.w  & ~1;   // Must be even.
            mpNr3dParam->onSiz_onHt = imgSize.h & ~1;
        }
        else    // Not NR3D_STATE_WORKING.
        {
            MY_LOGD_IF(mLogLevel >= 2, "[P2A not sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn)",
                    m3dnrStateMachine,
                    m3dnrErrorStatus
            );
            mpNr3dParam->onSiz_onWd = 0;
            mpNr3dParam->onSiz_onHt = 0;
            return MFALSE;
        }
    }
    else
    {
        LOG_DBG("[P2A not sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn). m3dnrPrvFrmQueue is empty", m3dnrStateMachine, m3dnrErrorStatus); // m3dnrPrvFrmQueue is empty => maybe first run.
        mpNr3dParam->onSiz_onWd = 0;
        mpNr3dParam->onSiz_onHt = 0;
        if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;
        return MFALSE;
    }

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3DParam* &pNr3dParam)
{
    UNUSED(frameNo);

    FUNC_START;
    MBOOL ret = MTRUE;
    //update mpNr3dParam info
    if (m3dnrStateMachine == NR3D_STATE_WORKING)    // Only set NR3D register when N3RD state machine is NR3D_STATE_WORKING.
    {
        mpNr3dParam->ctrl_onEn = 1;
        //david modified, u4Img3oOffset_X can skip because tile driver doesn't have limitation about crop region.
        mpNr3dParam->onOff_onOfStX = ((mNmvX >= 0) ? (0) : (-mNmvX));   // Must be even.
        mpNr3dParam->onOff_onOfStY = ((mNmvY >= 0) ? (0) : (-mNmvY));
    }
    else
    {
        mpNr3dParam->ctrl_onEn      = 0;
        mpNr3dParam->onOff_onOfStX  = 0;   // Must be even.
        mpNr3dParam->onOff_onOfStY  = 0;
        mpNr3dParam->onSiz_onWd     = 0;   // Must be even.
        mpNr3dParam->onSiz_onHt     = 0;
        mpNr3dParam->vipi_offst     = 0;    //in byte
        mpNr3dParam->vipi_readW     = 0;    //in pixel
        mpNr3dParam->vipi_readH     = 0;    //in pixel
        ret = MFALSE;
    }
    MY_LOGD_IF(mLogLevel >= 2, "3dnrSM2(%d S0P1W2),ES(0x%02X FsSzDfLrIn),NmvX/Y(%d, %d),onOfX/Y(%d, %d).onW/H(%d, %d).VipiOff/W/H(%d, %d, %d).MaxIsoInc(%d)",
        m3dnrStateMachine,
        m3dnrErrorStatus,
        mNmvX, mNmvY,
        mpNr3dParam->onOff_onOfStX, mpNr3dParam->onOff_onOfStY,
        mpNr3dParam->onSiz_onWd, mpNr3dParam->onSiz_onHt,
        mpNr3dParam->vipi_offst, mpNr3dParam->vipi_readW, mpNr3dParam->vipi_readH,
        get_3dnr_max_iso_increase_percentage()
    );

    pNr3dParam = mpNr3dParam;
    // Recordng mPrevFrameWidth/mPrevFrameHeight for next frame.
    mPrevFrameWidth  = imgiW;
    mPrevFrameHeight = imgiH;
    FUNC_END;
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Hal3dnr::get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3DParam &nr3dParam)
{
    UNUSED(frameNo);

    FUNC_START;
    MBOOL ret = MTRUE;
    //update mpNr3dParam info
    if (m3dnrStateMachine == NR3D_STATE_WORKING)    // Only set NR3D register when N3RD state machine is NR3D_STATE_WORKING.
    {
        mpNr3dParam->ctrl_onEn = 1;
        //david modified, u4Img3oOffset_X can skip because tile driver doesn't have limitation about crop region.
        mpNr3dParam->onOff_onOfStX = ((mNmvX >= 0) ? (0) : (-mNmvX));   // Must be even.
        mpNr3dParam->onOff_onOfStY = ((mNmvY >= 0) ? (0) : (-mNmvY));
    }
    else
    {
        mpNr3dParam->ctrl_onEn      = 0;
        mpNr3dParam->onOff_onOfStX  = 0;   // Must be even.
        mpNr3dParam->onOff_onOfStY  = 0;
        mpNr3dParam->onSiz_onWd     = 0;   // Must be even.
        mpNr3dParam->onSiz_onHt     = 0;
        mpNr3dParam->vipi_offst     = 0;    //in byte
        mpNr3dParam->vipi_readW     = 0;    //in pixel
        mpNr3dParam->vipi_readH     = 0;    //in pixel
        ret = MFALSE;
    }
    MY_LOGD_IF(mLogLevel >= 2, "3dnrSM2(%d S0P1W2),ES(0x%02X FsSzDfLrIn),NmvX/Y(%d, %d),onOfX/Y(%d, %d).onW/H(%d, %d).VipiOff/W/H(%d, %d, %d).MaxIsoInc(%d)",
        m3dnrStateMachine,
        m3dnrErrorStatus,
        mNmvX, mNmvY,
        mpNr3dParam->onOff_onOfStX, mpNr3dParam->onOff_onOfStY,
        mpNr3dParam->onSiz_onWd, mpNr3dParam->onSiz_onHt,
        mpNr3dParam->vipi_offst, mpNr3dParam->vipi_readW, mpNr3dParam->vipi_readH,
        get_3dnr_max_iso_increase_percentage()
    );

    nr3dParam = (*mpNr3dParam);

    // Recordng mPrevFrameWidth/mPrevFrameHeight for next frame.
    mPrevFrameWidth  = imgiW;
    mPrevFrameHeight = imgiH;
    FUNC_END;
    return ret;
}

MBOOL
Hal3dnr::checkStateMachine(NR3D_STATE_ENUM status)
{
    return (status == m3dnrStateMachine);
}

MBOOL
Hal3dnr::do3dnrFlow(
            void *pTuningData,
            MBOOL useCMV,
            MRect const &dst_resizer_rect,
            NR3DGMVInfo const &GMVInfo,
            NSCam::IImageBuffer *pIMGBufferVIPI,
            MINT32 iso,
            MUINT32 requestNo,
            NS3Av3::IHal3A* p3A)
{
    MBOOL ret = MFALSE;
    MBOOL bDrvNR3DEnabled = 1;

    if (mForce3DNR)
    {
        bDrvNR3DEnabled = ::property_get_int32("vendor.camera.3dnr.drv.nr3d.enable", 1);
    }

    if (MTRUE != prepare(requestNo, iso))
    {
        MY_LOGD_IF(mLogLevel >= 2, "3dnr prepare err");
    }

    setCMVMode(useCMV);

    NR3DCustom::AdjustmentInput adjInput;
    adjInput.force3DNR = mForce3DNR ? true : false;
    adjInput.setGmv(GMVInfo.confX, GMVInfo.confY, GMVInfo.gmvX, GMVInfo.gmvY);
    fillGyroForAdjustment(&adjInput);

    NR3DCustom::AdjustmentOutput adjOutput;
    NR3DCustom::adjust_parameters(adjInput, adjOutput, NULL);
    MINT32 adjustGMVX = GMVInfo.gmvX;
    MINT32 adjustGMVY = GMVInfo.gmvY;
    if (adjOutput.isGmvOverwritten)
    {
        adjustGMVX = adjOutput.gmvX;
        adjustGMVY = adjOutput.gmvY;
        MY_LOGW_IF(mLogLevel >= 1, "AfterAdjusting: (confX,confY)=(%d,%d), gmvX(%d->%d), gmvY(%d->%d)",
            GMVInfo.confX, GMVInfo.confY, GMVInfo.gmvX, adjustGMVX, GMVInfo.gmvY, adjustGMVY);
    }

    if (MTRUE != setGMV(requestNo, adjustGMVX, adjustGMVY, GMVInfo.x_int, GMVInfo.y_int))
    {
        MY_LOGD_IF(mLogLevel >= 2, "3dnr getGMV err");
    }

    // TODO: need to check IMG3O x,y also for IMGO->IMGI path
    if (MTRUE != checkIMG3OSize(requestNo, dst_resizer_rect.s.w, dst_resizer_rect.s.h))
    {
        MY_LOGD_IF(mLogLevel >= 2, "3dnr checkIMG3OSize err");
    }

    if (pIMGBufferVIPI != NULL)
    {
        if (MTRUE != setVipiParams(MTRUE,
              pIMGBufferVIPI->getImgSize().w, pIMGBufferVIPI->getImgSize().h,
              pIMGBufferVIPI->getImgFormat(), pIMGBufferVIPI->getBufStridesInBytes(0))
            )
        {
            MY_LOGD_IF(mLogLevel >= 2, "skip configVipi flow");
        }
        else
        {
            if (mForce3DNR &&
                ::property_get_int32("vendor.debug.3dnr.vipi.dump", 0))
            {
                // dump vipi img
                dumpVIPIBuffer(pIMGBufferVIPI, requestNo);
            }

            MY_LOGD_IF(mLogLevel >= 2, "configVipi: address:%p, W/H(%d,%d)", pIMGBufferVIPI,
            pIMGBufferVIPI->getImgSize().w, pIMGBufferVIPI->getImgSize().h);
            /* config Input for VIPI: this part is done in prepareIO(..) */
        }
    }
    else
    {
        if (MTRUE != setVipiParams(MFALSE/* vipi is NULL */, 0, 0, 0, 0))
        {
            MY_LOGD_IF(mLogLevel >= 2, "3dnr configVipi err");
        }
    }

    NR3DParam Nr3dParam;

    if (MTRUE != get3dnrParams(requestNo,
          dst_resizer_rect.s.w, dst_resizer_rect.s.h, Nr3dParam))
    {
        MY_LOGD_IF(mLogLevel >= 2, "skip config3dnrParams flow");
    }

    MY_LOGD_IF(mLogLevel >= 2, "Nr3dParam: onOff_onOfStX/Y(%d, %d), onSiz_onW/H(%d, %d), vipi_readW/H(%d, %d)",
        Nr3dParam.onOff_onOfStX, Nr3dParam.onOff_onOfStY,
        Nr3dParam.onSiz_onWd, Nr3dParam.onSiz_onHt,
        Nr3dParam.vipi_readW, Nr3dParam.vipi_readH);

    NS3Av3::NR3D_Config_Param param;

    if ((MTRUE == checkStateMachine(NR3D_STATE_WORKING)) && bDrvNR3DEnabled)
    {
        param.enable = bDrvNR3DEnabled;
        param.onRegion.p.x = Nr3dParam.onOff_onOfStX;
        param.onRegion.p.y = Nr3dParam.onOff_onOfStY;
        param.onRegion.s.w = Nr3dParam.onSiz_onWd;
        param.onRegion.s.h = Nr3dParam.onSiz_onHt;
        param.fullImg.p.x = dst_resizer_rect.p.x & ~1;
        param.fullImg.p.y = dst_resizer_rect.p.y & ~1;
        param.fullImg.s.w = dst_resizer_rect.s.w & ~1;
        param.fullImg.s.h = dst_resizer_rect.s.h & ~1;

        param.vipiOffst = Nr3dParam.vipi_offst;
        param.vipiReadSize.w = Nr3dParam.vipi_readW;
        param.vipiReadSize.h = Nr3dParam.vipi_readH;

        if (p3A)
        {
            // turn ON 'pull up ISO value to gain FPS'

            NS3Av3::AE_Pline_Limitation_T params;
            params.bEnable = MTRUE;
            params.bEquivalent = MTRUE;
            // use property "camera.3dnr.forceisolimit" to control max_iso_increase_percentage
            // ex: setprop camera.3dnr.forceisolimit 200
            params.u4IncreaseISO_x100 = get_3dnr_max_iso_increase_percentage();
            params.u4IncreaseShutter_x100 = 100;
            p3A->send3ACtrl(E3ACtrl_SetAEPlineLimitation, (MINTPTR)&params, 0);

            MY_LOGD_IF(mLogLevel >= 2, "turn ON 'pull up ISO value to gain FPS': max: %d %%", get_3dnr_max_iso_increase_percentage());
        }

        ret = MTRUE;
    }
    else
    {
        if (p3A)
        {
            // turn OFF 'pull up ISO value to gain FPS'

            //mp3A->modifyPlineTableLimitation(MTRUE, MTRUE,  100, 100);
            NS3Av3::AE_Pline_Limitation_T params;
            params.bEnable = MFALSE; // disable
            params.bEquivalent = MTRUE;
            params.u4IncreaseISO_x100 = 100;
            params.u4IncreaseShutter_x100 = 100;
            p3A->send3ACtrl(E3ACtrl_SetAEPlineLimitation, (MINTPTR)&params, 0);

            MY_LOGD_IF(mLogLevel >= 2, "turn OFF  'pull up ISO value to gain FPS'");
        }
    }

    if (pTuningData)
    {
        void *pIspPhyReg = pTuningData;

        // log keyword for auto test
        MY_LOGD_IF(mForce3DNR, "postProcessNR3D: EN(%d)", param.enable);
        if  ( auto pIspMgr = MAKE_IspMgr() ) {
            pIspMgr->postProcessNR3D(mSensorIdx, param, pIspPhyReg);
        }
    }

    return ret;
}

MBOOL
Hal3dnr::do3dnrFlow_v2(
            const NR3DHALParam& nr3dHalParam)
{
    FUNC_START;

    MBOOL ret = MTRUE;
    NR3DParam Nr3dParam;//default off

    if (SavedFrameInfo(nr3dHalParam) != MTRUE)
    {
        MY_LOGD_IF(mLogLevel >= 1, "3DNR off: SavedFrameInfo failed");
        // set NR3D off
        ret = MFALSE;
        goto configNR3DOnOff;
    }

    if (handle3DNROnOffPolicy(nr3dHalParam) != MTRUE)
    {
        MY_LOGD_IF(mLogLevel >= 1, "3DNR off: handle3DNROnOffPolicy failed");
        // set NR3D off
        ret = MFALSE;
        goto configNR3DOnOff;
    }

    if (handleAlignVipiIMGI(nr3dHalParam, Nr3dParam) != MTRUE)
    {
        MY_LOGD("3DNR off: handleAlignVipiIMGI failed");
        // set NR3D off
        ret = MFALSE;
        goto configNR3DOnOff;
    }

configNR3DOnOff:

    if (configNR3D(nr3dHalParam, Nr3dParam) != MTRUE)
    {
        MY_LOGD("3DNR off: configNR3D failed");
        ret = MFALSE;
    }

    FUNC_END;

    return ret;
}

MBOOL Hal3dnr::SavedFrameInfo(const NR3DHALParam &nr3dHalParam)
{
    // save data from current frame to previous frame
    mPreSavedFrameInfo = mCurSavedFrameInfo;

    mCurSavedFrameInfo.CmvX = nr3dHalParam.GMVInfo.x_int;
    mCurSavedFrameInfo.CmvY = nr3dHalParam.GMVInfo.y_int;
    mCurSavedFrameInfo.isCRZUsed = nr3dHalParam.isCRZUsed;
    mCurSavedFrameInfo.isIMGO = nr3dHalParam.isIMGO;
    mCurSavedFrameInfo.isBinning = nr3dHalParam.isBinning;

    return MTRUE;
}

MBOOL Hal3dnr::handle3DNROnOffPolicy(const NR3DHALParam &nr3dHalParam)
{
    hal3dnrDebugParam debugParam;
    debugParam.mLogLevel = mLogLevel;
    debugParam.mForce3DNR = mForce3DNR;
    debugParam.mSupportZoom3DNR = mSupportZoom3DNR;

    MUINT32 errorStatus = NR3D_ERROR_NONE;

    // check policy
    errorStatus = check3DNRPolicy(nr3dHalParam, debugParam, mPreSavedFrameInfo);

    // handle state
    NR3D_STATE_ENUM preStateMachine = m3dnrStateMachine;
    m3dnrErrorStatus = handleState(errorStatus, mForce3DNR, m3dnrStateMachine);

    if(NR3D_ERROR_NONE != m3dnrErrorStatus)
    {
        MY_LOGD("SensorIdx(%u), 3dnr state=(%d->%d), status(0x%x)",
            mSensorIdx, preStateMachine, m3dnrStateMachine, m3dnrErrorStatus);
    }

    return (NR3D_ERROR_NONE == m3dnrErrorStatus) ? MTRUE : MFALSE;
}

MBOOL Hal3dnr::handleAlignVipiIMGI(const NR3DHALParam &nr3dHalParam, NR3DParam &outNr3dParam)
{
    if (nr3dHalParam.pIMGBufferVIPI == NULL)
    {
        MY_LOGD_IF(mLogLevel >= 1, "Invalid pIMGBufferVIPI");
        return MFALSE;
    }

    MSize vipiFrameSize = nr3dHalParam.pIMGBufferVIPI->getImgSize();
    const MRect &pImg3oFrameRect = nr3dHalParam.dst_resizer_rect;

    NR3DGMVInfo GMVInfo = nr3dHalParam.GMVInfo;
    NR3DCustom::AdjustmentInput adjInput;
    adjInput.force3DNR = mForce3DNR ? true : false;
    adjInput.setGmv(GMVInfo.confX, GMVInfo.confY, GMVInfo.gmvX, GMVInfo.gmvY);
    const NSCam::NR3D::GyroData &gyroData = nr3dHalParam.gyroData;
    adjInput.setGyro(gyroData.isValid, gyroData.x, gyroData.y, gyroData.z);
    const NR3DRSCInfo &RSCInfo = nr3dHalParam.RSCInfo;
    MY_LOGD_IF(mLogLevel >= 1, "RSCData: MV(%p), BV(%p), rrzo(%d,%d), rsso(%d,%d), rsc_sta(%x) valid(%d)", RSCInfo.pMV, RSCInfo.pBV,
            RSCInfo.rrzoSize.w, RSCInfo.rrzoSize.h, RSCInfo.rssoSize.w, RSCInfo.rssoSize.h, RSCInfo.staGMV, RSCInfo.isValid);
    adjInput.setRsc(RSCInfo.isValid, RSCInfo.pMV, RSCInfo.pBV, RSCInfo.rrzoSize.w, RSCInfo.rrzoSize.h,
                    RSCInfo.rssoSize.w, RSCInfo.rssoSize.h, RSCInfo.staGMV);
    MY_LOGD_IF(mLogLevel >= 2, "Gyro isValid(%d), value(%f,%f,%f)",
        (gyroData.isValid ? 1 : 0), gyroData.x, gyroData.y, gyroData.z);

    NR3DCustom::AdjustmentOutput adjOutput;
    NR3DCustom::adjust_parameters(adjInput, adjOutput, NULL);
    if (adjOutput.isGmvOverwritten)
    {
        MY_LOGW_IF(mLogLevel >= 1, "AfterAdjusting: (confX,confY)=(%d,%d), gmvX(%d->%d), gmvY(%d->%d)",
            GMVInfo.confX, GMVInfo.confY, GMVInfo.gmvX, adjOutput.gmvX, GMVInfo.gmvY, adjOutput.gmvY);
        GMVInfo.gmvX = adjOutput.gmvX;
        GMVInfo.gmvY = adjOutput.gmvY;
    }

    if (pImg3oFrameRect.s == vipiFrameSize)
    {
        calGMV(nr3dHalParam, mForce3DNR, mPreSavedFrameInfo, GMVInfo);

        // === Algo code: start: align vipi/imgi ===
        NR3DAlignParam nr3dAlignParam;
        // TODO: integrate zoom flow here
        if (!handleFrameAlign(nr3dHalParam, GMVInfo, nr3dAlignParam))
        {
            MY_LOGD_IF(mLogLevel >= 0, "handleFrameAlign failed");
            return MFALSE;
        }

        getNR3DParam(nr3dHalParam, nr3dAlignParam, outNr3dParam);
        MY_LOGD_IF(mLogLevel >= 2, "vipi offset=%d,(w,h=%d,%d), on region(%d,%d,%d,%d)",
            outNr3dParam.vipi_offst, outNr3dParam.vipi_readW, outNr3dParam.vipi_readH,
            outNr3dParam.onOff_onOfStX, outNr3dParam.onOff_onOfStY, outNr3dParam.onSiz_onWd, outNr3dParam.onSiz_onHt);
    }
    else
    {

#if 0   // TODO: integrate zoom into basic flow
        calGMV(nr3dHalParam, mPreSavedFrameInfo, GMVInfo);
        // === Algo code: start: align vipi/imgi ===
        handleFrameZoomAlign(nr3dHalParam, GMVInfo, nr3dAlignParam)
        getNR3DParam(nr3dHalParam, nr3dAlignParam, outNr3dParam);
#else
        MINT32 nr3dPathID = determine3DNRPath(nr3dHalParam);

        // === Algo code: start: align vipi/imgi ===
        MINT32 adjustGMVX = GMVInfo.gmvX;
        MINT32 adjustGMVY = GMVInfo.gmvY;

        MINT32 i4GMVX = adjustGMVX /LMV_GMV_VALUE_TO_PIXEL_UNIT;
        MINT32 i4GMVY  = adjustGMVY /LMV_GMV_VALUE_TO_PIXEL_UNIT;

        MINT32 i4PVOfstX1 = 0;
        MINT32 i4PVOfstY1 = 0;
        MINT32 i4CUOfstX1 = 0;
        MINT32 i4CUOfstY1 = 0;
        MINT32 i4FrmWidthCU = nr3dHalParam.dst_resizer_rect.s.w;
        MINT32 i4FrmHeightCU = nr3dHalParam.dst_resizer_rect.s.h;
        MINT32 i4FrmWidthPV = nr3dHalParam.pIMGBufferVIPI->getImgSize().w;
        MINT32 i4FrmHeightPV = nr3dHalParam.pIMGBufferVIPI->getImgSize().h;
        MINT32 i4CUOfstX2 = 0;
        MINT32 i4CUOfstY2 = 0;
        MINT32 i4PVOfstX2 = 0;
        MINT32 i4PVOfstY2 = 0;
        MINT32 i4OvlpWD = 0;
        MINT32 i4OvlpHT = 0;
        MINT32 NR3D_WD = 0;
        MINT32 NR3D_HT = 0;
        MINT32 VIPI_OFST_X = 0;
        MINT32 VIPI_OFST_Y = 0;
        MINT32 VIPI_WD = 0;
        MINT32 VIPI_HT = 0;
        MINT32 NR3D_ON_EN = 0;
        MINT32 NR3D_ON_OFST_X = 0;
        MINT32 NR3D_ON_OFST_Y = 0;
        MINT32 NR3D_ON_WD = 0;
        MINT32 NR3D_ON_HT = 0;
        MINT32 nmvX = 0;
        MINT32 nmvY = 0;

        switch(nr3dPathID)
        {
            case NR3D_PATH_RRZO: // RRZO only
                break;
            case NR3D_PATH_RRZO_CRZ: // CRZ case
                mCmvX = nr3dHalParam.GMVInfo.x_int;
                mCmvY = nr3dHalParam.GMVInfo.y_int;
                nmvX = (-i4GMVX - (mCmvX - mPrevCmvX)) & ~1;
                nmvY = (-i4GMVY - (mCmvY - mPrevCmvY)) & ~1;
                i4GMVX = -nmvX;
                i4GMVY = -nmvY;
                mPrevCmvX = mCmvX;  // Recore last frame CMV X.
                mPrevCmvY = mCmvY;  // Recore last frame CMV Y.
            case NR3D_PATH_IMGO: // IMGO only
                break;
            default:
                MY_LOGE("!!err: should not happen");
                break;
        }

        if (i4GMVX<=0) { i4PVOfstX1 = -i4GMVX; i4CUOfstX1=0; }
        else { i4PVOfstX1 = 0; i4CUOfstX1=i4GMVX; }
        if (i4GMVY<=0) { i4PVOfstY1 = -i4GMVY; i4CUOfstY1=0; }
        else { i4PVOfstY1 = 0; i4CUOfstY1=i4GMVY; }

        if ((i4FrmWidthCU<=i4FrmWidthPV)&&(i4FrmHeightCU<=i4FrmHeightPV)) {   // case: vipi >= imgi
            i4CUOfstX2 = 0;
            i4CUOfstY2 = 0;
            i4PVOfstX2 = (i4FrmWidthPV-i4FrmWidthCU)/2;
            i4PVOfstY2 = (i4FrmHeightPV-i4FrmHeightCU)/2;
        }

        if ((i4FrmWidthCU >= i4FrmWidthPV)&&(i4FrmHeightCU>=i4FrmHeightPV)) {  // case: vipi <= imgi
            i4CUOfstX2 = (i4FrmWidthCU-i4FrmWidthPV)/2;
            i4CUOfstY2 = (i4FrmHeightCU-i4FrmHeightPV)/2;
            i4PVOfstX2 = 0;
            i4PVOfstY2 = 0;
        }

        i4OvlpWD = MIN(i4FrmWidthCU,i4FrmWidthPV)-abs(i4GMVX);
        i4OvlpHT = MIN(i4FrmHeightCU,i4FrmHeightPV)-abs(i4GMVY);

        NR3D_WD=i4FrmWidthCU;
        NR3D_HT=i4FrmHeightCU;

        VIPI_OFST_X=i4PVOfstX1+i4PVOfstX2;
        VIPI_OFST_Y=i4PVOfstY1+i4PVOfstY2;
        VIPI_WD=i4FrmWidthCU;
        VIPI_HT=i4FrmHeightCU;

        NR3D_ON_EN=1;
        NR3D_ON_OFST_X=i4CUOfstX1+i4CUOfstX2;
        NR3D_ON_OFST_Y=i4CUOfstY1+i4CUOfstY2;
        NR3D_ON_WD=i4OvlpWD;
        NR3D_ON_HT=i4OvlpHT;
        // === Algo code: end: align vipi/imgi ===

        // === save the vipi/imgi align info
        MUINT32 u4PixelToBytes = 0;
        MINT imgFormat = nr3dHalParam.pIMGBufferVIPI->getImgFormat();
        if (imgFormat== eImgFmt_YUY2) u4PixelToBytes = 2;
        if (imgFormat == eImgFmt_YV12) u4PixelToBytes = 1;

        outNr3dParam.vipi_offst = VIPI_OFST_Y * nr3dHalParam.pIMGBufferVIPI->getBufStridesInBytes(0) + VIPI_OFST_X * u4PixelToBytes;
        outNr3dParam.vipi_offst &= ~1;
        outNr3dParam.vipi_readW = i4FrmWidthCU &~1;
        outNr3dParam.vipi_readH = i4FrmHeightCU &~1;
        outNr3dParam.onSiz_onWd = i4OvlpWD &~1;
        outNr3dParam.onSiz_onHt = i4OvlpHT &~1;
        outNr3dParam.onOff_onOfStX = NR3D_ON_OFST_X &~1;
        outNr3dParam.onOff_onOfStY = NR3D_ON_OFST_Y &~1;
        outNr3dParam.ctrl_onEn = NR3D_ON_EN;

        print_NR3DParam(outNr3dParam, mLogLevel);
#endif
    }

    MY_LOGD("3dnr: SIdx(%u), ST=%d, path(%d), gmvX/Y(%5d,%5d), int_x/y=(%5d,%5d), confX/Y(%d, %d), "
        "f:%d, isResized(%d) offst(%d) (%d,%d)->(%d,%d,%d,%d) ",
        mSensorIdx, m3dnrStateMachine, (MINT32)determine3DNRPath(nr3dHalParam), GMVInfo.gmvX, GMVInfo.gmvY, GMVInfo.x_int, GMVInfo.y_int, GMVInfo.confX, GMVInfo.confY,
        nr3dHalParam.frameNo, (pImg3oFrameRect.s == vipiFrameSize), outNr3dParam.vipi_offst,
        outNr3dParam.vipi_readW, outNr3dParam.vipi_readH,
        outNr3dParam.onOff_onOfStX, outNr3dParam.onOff_onOfStY, outNr3dParam.onSiz_onWd, outNr3dParam.onSiz_onHt);

    return MTRUE;
}

MBOOL Hal3dnr::configNR3D(const NR3DHALParam& nr3dHalParam, const NR3DParam &Nr3dParam)
{
    MBOOL ret = MTRUE;
    NS3Av3::NR3D_Config_Param param;

    MBOOL bDrvNR3DEnabled = MTRUE;

    if (mForce3DNR)
    {
        bDrvNR3DEnabled = ::property_get_int32("vendor.camera.3dnr.drv.nr3d.enable", 1);
    }

    if ((MTRUE == checkStateMachine(NR3D_STATE_WORKING)) && bDrvNR3DEnabled)
    {
        param.enable = bDrvNR3DEnabled;
        param.onRegion.p.x = Nr3dParam.onOff_onOfStX & ~1;
        param.onRegion.p.y = Nr3dParam.onOff_onOfStY & ~1;
        param.onRegion.s.w = Nr3dParam.onSiz_onWd & ~1;
        param.onRegion.s.h = Nr3dParam.onSiz_onHt & ~1;

        param.fullImg.p.x = nr3dHalParam.dst_resizer_rect.p.x & ~1;
        param.fullImg.p.y = nr3dHalParam.dst_resizer_rect.p.y & ~1;
        param.fullImg.s.w = nr3dHalParam.dst_resizer_rect.s.w & ~1;
        param.fullImg.s.h = nr3dHalParam.dst_resizer_rect.s.h & ~1;

        param.vipiOffst = Nr3dParam.vipi_offst & ~1;
        param.vipiReadSize.w = Nr3dParam.vipi_readW & ~1;
        param.vipiReadSize.h = Nr3dParam.vipi_readH & ~1;

        if (nr3dHalParam.p3A)
        {
            // turn ON 'pull up ISO value to gain FPS'

            NS3Av3::AE_Pline_Limitation_T params;
            params.bEnable = MTRUE;
            params.bEquivalent= MTRUE;
            // use property "camera.3dnr.forceisolimit" to control max_iso_increase_percentage
            // ex: setprop camera.3dnr.forceisolimit 200
            params.u4IncreaseISO_x100= get_3dnr_max_iso_increase_percentage();
            params.u4IncreaseShutter_x100= 100;
            nr3dHalParam.p3A->send3ACtrl(E3ACtrl_SetAEPlineLimitation, (MINTPTR)&params, 0);

            MY_LOGD_IF(mLogLevel >= 2, "turn ON 'pull up ISO value to gain FPS': max: %d %%", get_3dnr_max_iso_increase_percentage());
        }

        if (mForce3DNR && ::property_get_int32("vendor.debug.3dnr.vipi.dump", 0))
        {
            // dump vipi img
            dumpVIPIBuffer(nr3dHalParam.pIMGBufferVIPI, nr3dHalParam.frameNo);
        }
    }
    else
    {
        param.enable = MFALSE;

        if (nr3dHalParam.p3A)
        {
            // turn OFF 'pull up ISO value to gain FPS'

            //mp3A->modifyPlineTableLimitation(MTRUE, MTRUE,  100, 100);
            NS3Av3::AE_Pline_Limitation_T params;
            params.bEnable = MFALSE; // disable
            params.bEquivalent= MTRUE;
            params.u4IncreaseISO_x100= 100;
            params.u4IncreaseShutter_x100= 100;
            nr3dHalParam.p3A ->send3ACtrl(E3ACtrl_SetAEPlineLimitation, (MINTPTR)&params, 0);

            MY_LOGD_IF(mLogLevel >= 2, "turn OFF  'pull up ISO value to gain FPS'");
        }
    }

    if (nr3dHalParam.pTuningData)
    {
        void *pIspPhyReg = nr3dHalParam.pTuningData;

        // log keyword for auto test
        MY_LOGD_IF(mForce3DNR, "postProcessNR3D: EN(%d)", param.enable);
        if  ( auto pIspMgr = MAKE_IspMgr() ) {
            pIspMgr->postProcessNR3D(mSensorIdx, param, pIspPhyReg);
        }
    }

    FUNC_END;
    return ret;
}


MBOOL Hal3dnr::fillGyroForAdjustment(void *__adjInput)
{
    NR3DCustom::AdjustmentInput *pAdjInput = static_cast<NR3DCustom::AdjustmentInput*>(__adjInput);

    NSCam::Utils::SensorData sensorData;
    if (mpSensorProvider != NULL &&
        mpSensorProvider->getLatestSensorData(SENSOR_TYPE_GYRO, sensorData))
    {
        pAdjInput->setGyro(true, sensorData.gyro[0], sensorData.gyro[1], sensorData.gyro[2]);
    }
    else
    {
        pAdjInput->isGyroValid = false;
    }

    MY_LOGD_IF(mLogLevel >= 2, "Gyro isValid(%d), value(%f,%f,%f)",
        (pAdjInput->isGyroValid ? 1 : 0), pAdjInput->gyroX, pAdjInput->gyroY, pAdjInput->gyroZ);

    return (pAdjInput->isGyroValid ? MTRUE : MFALSE);
}
