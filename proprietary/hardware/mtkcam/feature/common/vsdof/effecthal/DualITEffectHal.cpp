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
#define LOG_TAG "DualITEffectHal"
#include <mtkcam/utils/std/Log.h>

#define EFFECT_NAME "DualEffect"
#define MAJOR_VERSION 0
#define MINOR_VERSION 1


#define UNUSED(x) (void)x

#include <cutils/log.h>
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/feature/effectHalBase/IEffectHal.h>
#include <mtkcam/feature/effectHalBase/EffectHalBase.h>
//
#include <mtkcam/feature/stereo/effecthal/DualITEffectHal.h>
//
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
//
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <string>
//
#include <vsdof_util.h>
#include <DpBlitStream.h>
//
#include <chrono>
#include <string>
#include <mtkcam/utils/std/Misc.h>
#include <mtkcam/utils/std/Trace.h>
#include <cutils/properties.h>
//
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START
#define FUNCTION_LOG_END
//
using namespace std;
using namespace NSCam;
using namespace android;
using namespace NSCam::NSIoPipe::NSSImager;
//************************************************************************
//
//************************************************************************
DualITEffectHal::
DualITEffectHal()
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("vendor.debug.vsdof.ditdump", cLogLevel, "0");
    miDump = ::atoi(cLogLevel);
    if(miDump>0)
    {
        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
        msFilename = std::string("/sdcard/vsdof/Cap/ditinput/")+std::to_string(millis);
        NSCam::Utils::makePath(msFilename.c_str(), 0660);
    }
}
//************************************************************************
//
//************************************************************************
DualITEffectHal::
~DualITEffectHal()
{

}
//************************************************************************
//
//************************************************************************
bool
DualITEffectHal::
allParameterConfigured()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return true;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
initImpl()
{
    FUNCTION_LOG_START;
    // init ImageTransform
    mpImgTransform = IImageTransform::createInstance();
    if(!mpImgTransform)
    {
        MY_LOGE("imageTransform create failed.");
    }
    mpDpStream = new DpBlitStream();
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
uninitImpl()
{
    FUNCTION_LOG_START;
    if(mpDpStream!=nullptr)
    {
        delete mpDpStream;
    }
    if( mpImgTransform )
    {
        mpImgTransform->destroyInstance();
        mpImgTransform = NULL;
    }
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
prepareImpl()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
releaseImpl()
{
    FUNCTION_LOG_START;

    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
getNameVersionImpl(
    EffectHalVersion &nameVersion) const
{
    FUNCTION_LOG_START;

    nameVersion.effectName = EFFECT_NAME;
    nameVersion.major = MAJOR_VERSION;
    nameVersion.minor = MINOR_VERSION;

    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
getCaptureRequirementImpl(
    EffectParameter *inputParam,
    Vector<EffectCaptureRequirement> &requirements) const
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
setParameterImpl(
    String8 &key,
    String8 &object)
{
    FUNCTION_LOG_START;
    UNUSED(key);
    UNUSED(object);
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
setParametersImpl(
    sp<EffectParameter> parameter)
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
startImpl(
    uint64_t *uid)
{
    FUNCTION_LOG_START;
    UNUSED(uid);
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
abortImpl(
    EffectResult &result,
    EffectParameter const *parameter)
{
    FUNCTION_LOG_START;
    UNUSED(result);
    UNUSED(parameter);
    //
    onFlush();
    //
    FUNCTION_LOG_END;
    return OK;
}
//************************************************************************
//
//************************************************************************
status_t
DualITEffectHal::
updateEffectRequestImpl(
    const EffectRequestPtr request)
{
    FUNCTION_LOG_START;
    Mutex::Autolock _l(mLock);
    MBOOL ret = MTRUE;
    //
    sp<EffectFrameInfo> pLeftJpsFrame  = nullptr;
    sp<EffectFrameInfo> pRightJpsFrame = nullptr;
    sp<EffectFrameInfo> pOutJpsFrame = nullptr;
    sp<IImageBuffer> pLeftImgBuf = nullptr;
    sp<IImageBuffer> pRightImgBuf = nullptr;
    sp<IImageBuffer> pOutImgBuf = nullptr;
    sp<IImageBuffer> pTargetLeft = nullptr;
    sp<IImageBuffer> pTargetRight = nullptr;
    EffectRequestPtr req = (EffectRequestPtr)request;
    //
    MBOOL bConvertLeftImg = MFALSE;
    MBOOL bConvertRightImg = MFALSE;
    sp<EffectParameter>params = request->getRequestParameter();
    bConvertLeftImg = params->getInt(DIT_CONVERT_LEFT_IMG_FMT);
    bConvertRightImg = params->getInt(DIT_CONVERT_RIGHT_IMG_FMT);
    // get left frame
    ssize_t index = request->vInputFrameInfo.indexOfKey(DIT_BUF_JPS_LEFT);
    if(index >= 0)
    {
        pLeftJpsFrame = request->vInputFrameInfo.valueAt(index);
        pLeftJpsFrame->getFrameBuffer(pLeftImgBuf);
        pTargetLeft = pLeftImgBuf;
    }
    else
    {
        MY_LOGE("Get left image fail.");
        ret = MFALSE;
        goto lbExit;
    }
    // get right frame
    index = request->vInputFrameInfo.indexOfKey(DIT_BUF_JPS_RIGHT);
    if(index >= 0)
    {
        pRightJpsFrame = request->vInputFrameInfo.valueAt(index);
        pRightJpsFrame->getFrameBuffer(pRightImgBuf);
        pTargetRight = pRightImgBuf;
    }
    else
    {
        MY_LOGE("Get right image fail.");
        ret = MFALSE;
        goto lbExit;
    }
    // get output frame
    index = request->vOutputFrameInfo.indexOfKey(DIT_BUF_JPS_OUTPUT);
    if(index >= 0)
    {
        pOutJpsFrame = request->vOutputFrameInfo.valueAt(index);
        pOutJpsFrame->getFrameBuffer(pOutImgBuf);
    }
    else
    {
        MY_LOGE("Get out image fail.");
        ret = MFALSE;
        goto lbExit;
    }
    if(miDump>0)
    {
        std::string saveFileName = msFilename + string("/LEFT_")+
            string(".yuv");
        pLeftImgBuf->saveToFile(saveFileName.c_str());
        saveFileName = msFilename + string("/RIGHT_")+
            string(".yuv");
        pRightImgBuf->saveToFile(saveFileName.c_str());
    }
    // check needs to convert img fmt to RGBA8888
    {
        // image
        MSize IMG_SIZE = StereoSizeProvider::getInstance()->getSBSImageSize();
        MUINT32 IMG_STRIDES[3] = {static_cast<MUINT32>(IMG_SIZE.w<<2), 0, 0};
        MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};
        const IImageBufferAllocator::ImgParam imgParam_Img(
        eImgFmt_RGBA8888, IMG_SIZE, IMG_STRIDES, FULL_RAW_BOUNDARY, 1);
        if(bConvertLeftImg)
        {
            MY_LOGD("Force to convert left image fmt");
            pTargetLeft = VSDOF::util::createEmptyImageBuffer(
                                            imgParam_Img,
                                            "JPS_Left",
                                            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN);
            // copy right image to pTargetLeft
            processImageByMDP(pLeftImgBuf, pTargetLeft, 0);
        }
        if(bConvertRightImg)
        {
            MY_LOGD("Force to convert right image fmt");
            pTargetRight = VSDOF::util::createEmptyImageBuffer(
                                            imgParam_Img,
                                            "JPS_Right",
                                            eBUFFER_USAGE_SW_READ_OFTEN |eBUFFER_USAGE_HW_CAMERA_READWRITE| eBUFFER_USAGE_SW_WRITE_OFTEN);
            // copy right image to pTargetRight
            processImageByMDP(pRightImgBuf, pTargetRight, 0);
        }
    }
    //printf("0x%x 0x%x 0x%x\n", pLeftImgBuf->getBufPA(0), pLeftImgBuf->getBufPA(1), pLeftImgBuf->getBufPA(2));
    //printf("0x%x 0x%x 0x%x\n", pRightImgBuf->getBufPA(0), pRightImgBuf->getBufPA(1), pRightImgBuf->getBufPA(2));
    //printf("0x%x 0x%x 0x%x\n", pOutImgBuf->getBufPA(0), pOutImgBuf->getBufPA(1), pOutImgBuf->getBufPA(2));
    //
    ret = doImageTransform(
                pTargetLeft,
                pOutImgBuf,
                MFALSE) &&
          doImageTransform(
                pTargetRight,
                pOutImgBuf,
                MTRUE);
    //
    if(request->mpOnRequestProcessed!=nullptr)
    {
        request->mpOnRequestProcessed(request->mpTag, String8("Done"), req);
    }
lbExit:
    FUNCTION_LOG_END;
    return ret;
}
//************************************************************************
//
//************************************************************************
MBOOL
DualITEffectHal::
onFlush()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return true;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualITEffectHal::
processImageByMDP(
    sp<IImageBuffer> srcBuf,
    sp<IImageBuffer> dstBuf,
    MINT32 rot)
{
    CAM_TRACE_NAME("DualITEffectHal::processImageByMDP");
    if(mpDpStream == nullptr)
    {
        MY_LOGE("mpDpStream is null.");
        return MFALSE;
    }
    VSDOF::util::sMDP_Config config;
    config.pDpStream = mpDpStream;
    config.pSrcBuffer = srcBuf.get();
    config.pDstBuffer = dstBuf.get();
    config.rotAngle = rot;
    if(!excuteMDP(config))
    {
        MY_LOGE("excuteMDP fail.");
        return MFALSE;
    }
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
DualITEffectHal::
doImageTransform(
    sp<IImageBuffer> srcBuf,
    sp<IImageBuffer> dstBuf,
    MBOOL const isRightBuf
)
{
    MBOOL ret = MTRUE;
    //
    if(dstBuf == NULL)
    {
        MY_LOGE("dstBuf is null.");
        return MFALSE;
    }
    //
    if(srcBuf == NULL)
    {
        MY_LOGE("srcBuf is null.");
        return MFALSE;
    }
    //
    sp<IImageBuffer> pDstBuf_SBS = NULL;

    // this value may get from stereo_hal.
    MUINT32 const trans = 0; // eTransform_None, if this value is not exist, it need to add to ImageFormat.h.
    // set src image size to crop info.
    MRect crop = MRect(MPoint(), srcBuf->getImgSize());
    //
    MY_LOGD("1");
    pDstBuf_SBS = dstBuf->getImageBufferHeap()->createImageBuffer_SideBySide(isRightBuf);
    MY_LOGD("2");
    if(pDstBuf_SBS != NULL)
    {
        printf("%x\n", eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK);
        if(!pDstBuf_SBS->lockBuf(LOG_TAG, eBUFFER_USAGE_SW_READ_MASK | eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_HW_TEXTURE))
        {
            MY_LOGE("lock buffer failed");
            ret = MFALSE;
        }
    MY_LOGD("3");
        pDstBuf_SBS->setTimestamp(srcBuf->getTimestamp());

    MY_LOGD("4");
        // Watch Out! this may cause image size reset!
        // EX: [IonHeap::setExtParam] update imgSize(1344x1984 -> 1920x1080), offset(0->0) @0-plane
        pDstBuf_SBS->setExtParam(crop.s);
    }
    else
    {
        MY_LOGE("pDstBuf_SBS is NULL.");
    }

    //
    MY_LOGD("src:(%p),S(%dx%d),stride(%d),F(%d),TS(%lld)", srcBuf.get(), srcBuf->getImgSize().w, srcBuf->getImgSize().h,
            srcBuf->getBufStridesInBytes(0), srcBuf->getImgFormat(), srcBuf->getTimestamp() );
    MY_LOGD("SBS:(%p),isR(%d),S(%dx%d),stride(%d),F(%d),TS(%lld),crop(%d,%d,%d,%d)", pDstBuf_SBS.get(), isRightBuf,
            pDstBuf_SBS->getImgSize().w, pDstBuf_SBS->getImgSize().h,
            pDstBuf_SBS->getBufStridesInBytes(0), pDstBuf_SBS->getImgFormat(), pDstBuf_SBS->getTimestamp(),
            crop.p.x, crop.p.y, crop.s.w, crop.s.h);
    // log information need query from stereo_hal.
    MY_LOGD("Stereo_Profile: mpImgTransform->execute +");
    mpImgTransform->execute(srcBuf.get(), NULL, pDstBuf_SBS.get(), crop, trans, 0xFFFFFFFF);
    MY_LOGD("Stereo_Profile: mpImgTransform->execute -");

    if(!pDstBuf_SBS->unlockBuf(LOG_TAG))
    {
        MY_LOGE("unlock buffer failed.");
        ret = MFALSE;
    }
    return ret;
}