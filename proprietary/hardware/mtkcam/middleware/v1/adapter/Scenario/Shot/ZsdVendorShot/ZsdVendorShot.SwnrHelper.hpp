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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

// SWNR
#include <mtkcam/aaa/ICaptureNR.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h> // IImageTransform

// AOSP
#include <cutils/compiler.h> // CC_UNLIKELY
#include <cutils/properties.h> // property_get

// STL
#include <memory>

// POSIX
#include <sys/resource.h>

namespace SwnrHelper
{

// Create a SWNR instance
inline std::shared_ptr<ISwNR>
createSwnrInstance(MINT32 openId)
{
    CAM_TRACE_CALL();

    return std::shared_ptr<ISwNR>( MAKE_SwNR(openId) );
}


// Check if it's necessary to do SWNR
//  @param openId               The current sensorID.
//  @param iso                  The current ISO.
//  @param isMfll               Tells if doing MFLL.
//  @return                     True for need, otherwise no need.
inline bool
isNeedToDoSwnr(
        MINT32              openId,
        MINT32              iso,
        bool                isMfll = false
)
{

    CAM_TRACE_CALL();

    static int f = ::property_get_int32("debug.shot.forcenr",-1);
    if (CC_UNLIKELY( f >= 0 )) {
        switch (f) {
        case 0: CAM_LOGD("force disable NR"); return false;
        case 2: CAM_LOGD("force SWNR"); return true;
        default:;
        }
    }

    int _sw_threshold = -1;
    int _hw_threshold = -1;
    //
    NSCam::IHalSensorList* pHalSensorList = MAKE_HalSensorList();
    MUINT const sensorDev = pHalSensorList->querySensorDevIdx(openId);
    // get threshold from custom folder
    get_capture_nr_th(
            sensorDev, eShotMode_NormalShot, isMfll,
            &_hw_threshold, &_sw_threshold);
    CAM_LOGD("threshold(H:%d/S:%d)", _hw_threshold, _sw_threshold);
    //
    return iso >= _sw_threshold;
}


// To perform SwNR. SwNR takes a in-place buffer with format I420.
//  @param openId               The current sensor ID.
//  @param iso                  The ISO value of this image.
//  @param isMfll               Indicates to if MFLL or not.
//  @param pBuf                 Input / output buffer.
//  @param pSWNR                The ISWNR instance.
//  @note Caller has no need to invoke IImageBuffer::syncCache because SWNR class
//        has invoked it.
inline MERROR
doSwnr(
        MINT32              openId,
        MINT32              iso,
        bool                isMfll,
        IImageBuffer*       pBuf,
        ISwNR*              pSWNR
)
{
    CAM_TRACE_CALL();

    MERROR err = OK;

    // check image type
    if (CC_UNLIKELY( pBuf->getImgFormat() != eImgFmt_I420 )) {
        CAM_LOGE("SWNR only accepts image format I420");
        return BAD_VALUE;
    }

    ISwNR::SWNRParam param;
    param.iso       = iso;
    param.isMfll    = isMfll;
    param.perfLevel = 240;

    int perfLevel_debug = ::property_get_int32("debug.camera.nr.perfLevel", -1);
    if(perfLevel_debug != -1) {
        param.perfLevel = perfLevel_debug;
    }
    CAM_LOGD("swnr param:iso=%d, ismfll=%d, perfLevel=%d",
        param.iso, param.isMfll, param.perfLevel);

    static int _prior = ::property_get_int32("debug.camera.nr.priority", -11);

    int _ori   = ::getpriority(PRIO_PROCESS, 0);
    int _err   = ::setpriority(PRIO_PROCESS, 0, _prior);
    if (__builtin_expect( _err == 0, true )) {
        CAM_LOGD("set SWNR thread priority to %d", _prior);
    }
    else {
        CAM_LOGW("set priority to SWNR thread failed,err=%d", _err);
    }

    CAM_TRACE_BEGIN("alg::doSWNR");
    if (CC_UNLIKELY( ! pSWNR->doSwNR(param, pBuf) )) {
        CAM_TRACE_END();
        CAM_LOGE("SWNR failed");
        err = UNKNOWN_ERROR;
    }
    else {
        CAM_TRACE_END();
#if 0
        if (CC_LIKELY( halMeta )) {
            CAM_TRACE_NAME("setDbgInfo");
            pSWNR->getDebugInfo(*halMeta);
        }
        else {
            CAM_LOGD("no hal metadata for saving debug info");
        }
#endif
    }
    // ::setpriority returns OK, hence restore priority back
    if (__builtin_expect( _err == 0, true )) {
        _err = ::setpriority(PRIO_PROCESS, 0, _ori);
        if (__builtin_expect( _err != 0, false )) {
            CAM_LOGE("weird, setpriority back to %d but failed,err=%d",
                    _prior, _err);
        }
    }

    return err;
}


// Convert image (format, CRZ...etc) using MDP.
//  @param pSrc                 Source image.
//  @param pDst                 Destination image.
//  @param roi                  Can be zero, use full size of source image.
//  @param transfrom            E.g.: eTransform_ROT_90 ... etc
inline MERROR
convertImage(
        IImageBuffer*   pSrc,
        IImageBuffer*   pDst,
        MRect           roi = MRect(MPoint(0, 0), MSize(0, 0)),
        MINT32          transfrom = 0
)
{
    using NSCam::NSIoPipe::NSSImager::IImageTransform;

    auto _d = [](IImageTransform* p){ if (CC_LIKELY(p)) p->destroyInstance(); };
    std::unique_ptr<IImageTransform, decltype(_d)> t
        (
            IImageTransform::createInstance(),
            _d
        );
    if (CC_UNLIKELY( t.get() == nullptr )) {
        CAM_LOGE("create IImageTransform failed");
        return UNKNOWN_ERROR;
    }

    if (roi.size().w <= 0 || roi.size().h <=0) {
        roi = MRect(MPoint(0, 0), pSrc->getImgSize());
    }
    CAM_LOGD("convertImage transfrom %d", transfrom);

    CAM_TRACE_BEGIN("IImageTransform::execute");
    auto _result = t->execute(
            pSrc,
            pDst,
            nullptr,
            roi,
            transfrom,
            3000 // time out in ms
            );
    CAM_TRACE_END();

    if (CC_UNLIKELY( _result != MTRUE )) {
        CAM_LOGE("IImageTransform::execute returns failed");
        return UNKNOWN_ERROR;
    }

    return OK;
}

}; // namespace swnr
