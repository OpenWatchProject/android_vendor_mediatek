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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include <mtkcam3/feature/utils/p2/DIPStream_NormalStream.h>

#include <mtkcam3/feature/utils/p2/P2Trace.h>

#define ILOG_MODULE_TAG DIPStream_NormalStream
#include <mtkcam3/feature/utils/log/ILogHeader.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_COMMON);

#define DIP_MEDIA_DEVICE_NAME "MTK-ISP-DIP-V4L2"
#define DIP_BATCH_MODE_SDEV_NAME "MTK-ISP-DIP-V4L2"
#define DIP_BATCH_MODE_VDEV_NAME "MTK-ISP-DIP-V4L2 Raw Input"

#define NORMAL_STREAM_NAME "StreamingFeature"

namespace NSCam {
namespace Feature {
namespace P2Util {

DIPStream_NormalStream::DIPStream_NormalStream(MUINT32 sensorIndex)
    : DIPStream(sensorIndex)
{
}

DIPStream_NormalStream::~DIPStream_NormalStream()
{
}

MBOOL DIPStream_NormalStream::init(char const* szCallerName, NSCam::NSIoPipe::EStreamPipeID StreamPipeID, MUINT32 secTag)
{
    MBOOL ret = MFALSE;
    //init NormalStream
    if( ((mNormalStream = NSIoPipe::NSPostProc::INormalStream::createInstance(mSensorIndex)) != NULL) &&
        (mNormalStream->init(szCallerName, StreamPipeID, secTag)) != MFALSE )
    {
        ret = MTRUE;
    }
    else
    {
        MY_LOGE("NormalStream(%s : %p) sensor(%u) secTag(%u) init failed", szCallerName, mNormalStream, mSensorIndex, secTag);
        ret = MFALSE;
        uninit(szCallerName);
    }
    return ret;
}

MBOOL DIPStream_NormalStream::uninit(char const* szCallerName)
{
    //uninit NormalStream
    MBOOL ret = MFALSE;
    if( mNormalStream )
    {
        ret = mNormalStream->uninit(szCallerName);
        mNormalStream->destroyInstance();
        mNormalStream = NULL;
    }
    return ret;
}

MBOOL DIPStream_NormalStream::enque(const DIPParams &dipParams)
{
    //enque NormalStream
    MBOOL ret = MFALSE;
    if( mNormalStream )
    {
        ret = mNormalStream->enque(convertToQParams(dipParams));
    }
    return ret;
}

MVOID DIPStream_NormalStream::normalStreamCB(QParams &qParams)
{
    DIPParams dipParams = convertToDIPParams(qParams);
    if(dipParams.mpfnDIPCallback != NULL)
    {
        dipParams.mpfnDIPCallback(dipParams);
    }
}

MVOID DIPStream_NormalStream::normalStreamFailCB(QParams &qParams)
{
    DIPParams dipParams = convertToDIPParams(qParams);
    if(dipParams.mpfnDIPEnQFailCallback != NULL)
    {
        dipParams.mpfnDIPEnQFailCallback(dipParams);
    }
}

MVOID DIPStream_NormalStream::normalStreamBlockCB(QParams &qParams)
{
    DIPParams dipParams = convertToDIPParams(qParams);
    if(dipParams.mpfnDIPEnQBlockCallback != NULL)
    {
        dipParams.mpfnDIPEnQBlockCallback(dipParams);
    }
}

QParams DIPStream_NormalStream::convertToQParams(const DIPParams &dipParams)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_NormalStream::convertToQParams");
    QParams qParams;
    qParams.mpfnCallback = normalStreamCB;
    qParams.mpfnEnQFailCallback = normalStreamFailCB;
    qParams.mpfnEnQBlockCallback = normalStreamBlockCB;
    // Cookie extension
    struct CookieExt *pCookieExt = new CookieExt();
    pCookieExt->mpOriCookie = dipParams.mpCookie;
    pCookieExt->mpfnDIPCallback = dipParams.mpfnDIPCallback;
    pCookieExt->mpfnDIPEnQFailCallback = dipParams.mpfnDIPEnQFailCallback;
    pCookieExt->mpfnDIPEnQBlockCallback = dipParams.mpfnDIPEnQBlockCallback;
    qParams.mpCookie = (MVOID *)pCookieExt;

    qParams.mDequeSuccess = dipParams.mDequeSuccess;
    qParams.mvFrameParams.reserve(dipParams.mvDIPFrameParams.size());
    for( const auto &dipFrameParam : dipParams.mvDIPFrameParams )
    {
        FrameParams frameParam;
        frameParam.FrameNo = dipFrameParam.FrameNo;
        frameParam.RequestNo = dipFrameParam.RequestNo;
        frameParam.Timestamp = dipFrameParam.Timestamp;
        frameParam.UniqueKey = dipFrameParam.UniqueKey;
        frameParam.IspProfile = dipFrameParam.IspProfile;
        frameParam.SensorDev = dipFrameParam.SensorDev;
        frameParam.FrameIdentify = dipFrameParam.FrameIdentify;
        frameParam.mRunIdx = dipFrameParam.mRunIdx;
        frameParam.NeedDump = dipFrameParam.NeedDump;
        frameParam.mStreamTag = dipFrameParam.mStreamTag;
        frameParam.mSensorIdx = dipFrameParam.mSensorIdx;
        frameParam.mSecureFra = dipFrameParam.mSecureFra;
        frameParam.mTuningData = dipFrameParam.mTuningData;
        frameParam.mpCookie = dipFrameParam.mpCookie;
        frameParam.ExpectedEndTime = dipFrameParam.ExpectedEndTime;
        frameParam.mvIn.reserve(dipFrameParam.mvIn.size());
        for( const auto &it : dipFrameParam.mvIn )
        {
            frameParam.mvIn.push_back(it);
        }
        frameParam.mvOut.reserve(dipFrameParam.mvOut.size());
        for( const auto &it : dipFrameParam.mvOut )
        {
            frameParam.mvOut.push_back(it);
        }
        frameParam.mvCropRsInfo.reserve(dipFrameParam.mvCropRsInfo.size());
        for( const auto &it : dipFrameParam.mvCropRsInfo )
        {
            frameParam.mvCropRsInfo.push_back(it);
        }
        frameParam.mvModuleData.reserve(dipFrameParam.mvModuleData.size());
        for( const auto &it : dipFrameParam.mvModuleData )
        {
            frameParam.mvModuleData.push_back(it);
        }
        frameParam.mvExtraParam.reserve(dipFrameParam.mvExtraParam.size());
        for( const auto &it : dipFrameParam.mvExtraParam )
        {
            frameParam.mvExtraParam.push_back(it);
        }
        frameParam.mpfnCallback = dipFrameParam.mpfnCallback;
        qParams.mvFrameParams.push_back(frameParam);
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();

    return qParams;
}

DIPParams DIPStream_NormalStream::convertToDIPParams(const QParams &qParams)
{
    TRACE_FUNC_ENTER();
    P2_CAM_TRACE_BEGIN(TRACE_ADVANCED, "DIPStream_NormalStream::convertToDIPParams");

    DIPParams dipParams;
    // Cookie convert
    struct CookieExt *pCookieExt = (CookieExt *)qParams.mpCookie;
    if(pCookieExt != NULL && pCookieExt->mMagicCheck == MAGIC_PASS_NUM)
    {
        dipParams.mpCookie = pCookieExt->mpOriCookie;
        dipParams.mpfnDIPCallback = pCookieExt->mpfnDIPCallback;
        dipParams.mpfnDIPEnQFailCallback = pCookieExt->mpfnDIPEnQFailCallback;
        dipParams.mpfnDIPEnQBlockCallback = pCookieExt->mpfnDIPEnQBlockCallback;
        pCookieExt->mMagicCheck = 0x0;
        delete pCookieExt;
        pCookieExt = NULL;
    }
    else
    {
        MY_LOGW("QParams cookie return error, pCookieExt = %p", pCookieExt);
        if(pCookieExt != NULL)
            MY_LOGW("MagicCheck = 0x%X", pCookieExt->mMagicCheck);
    }

    dipParams.mDequeSuccess = qParams.mDequeSuccess;
    dipParams.mvDIPFrameParams.reserve(qParams.mvFrameParams.size());
    for( const auto &frameParam : qParams.mvFrameParams )
    {
        DIPFrameParams dipFrameParam;
        dipFrameParam.FrameNo = frameParam.FrameNo;
        dipFrameParam.RequestNo = frameParam.RequestNo;
        dipFrameParam.Timestamp = frameParam.Timestamp;
        dipFrameParam.UniqueKey = frameParam.UniqueKey;
        dipFrameParam.IspProfile = frameParam.IspProfile;
        dipFrameParam.FrameIdentify = frameParam.FrameIdentify;
        dipFrameParam.mRunIdx = frameParam.mRunIdx;
        dipFrameParam.NeedDump = frameParam.NeedDump;
        dipFrameParam.mStreamTag = frameParam.mStreamTag;
        dipFrameParam.mSensorIdx = frameParam.mSensorIdx;
        dipFrameParam.mSecureFra = frameParam.mSecureFra;
        dipFrameParam.mTuningData = frameParam.mTuningData;
        dipFrameParam.mpCookie = frameParam.mpCookie;
        dipFrameParam.ExpectedEndTime = frameParam.ExpectedEndTime;
        dipFrameParam.mvIn.reserve(frameParam.mvIn.size());
        for( const auto &it : frameParam.mvIn )
        {
            dipFrameParam.mvIn.push_back(it);
        }
        dipFrameParam.mvOut.reserve(frameParam.mvOut.size());
        for( const auto &it : frameParam.mvOut )
        {
            dipFrameParam.mvOut.push_back(it);
        }
        dipFrameParam.mvCropRsInfo.reserve(frameParam.mvCropRsInfo.size());
        for( const auto &it : frameParam.mvCropRsInfo )
        {
            dipFrameParam.mvCropRsInfo.push_back(it);
        }
        dipFrameParam.mvModuleData.reserve(frameParam.mvModuleData.size());
        for( const auto &it : frameParam.mvModuleData )
        {
            dipFrameParam.mvModuleData.push_back(it);
        }
        dipFrameParam.mvExtraParam.reserve(frameParam.mvExtraParam.size());
        for( const auto &it : frameParam.mvExtraParam )
        {
            dipFrameParam.mvExtraParam.push_back(it);
        }
        dipFrameParam.mpfnCallback = frameParam.mpfnCallback;
        dipParams.mvDIPFrameParams.push_back(dipFrameParam);
    }

    P2_CAM_TRACE_END(TRACE_ADVANCED);
    TRACE_FUNC_EXIT();
    return dipParams;
}


MBOOL DIPStream_NormalStream::sendCommand(NSIoPipe::NSPostProc::ESDCmd cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MBOOL ret = MFALSE;
    if( mNormalStream )
    {
        ret = mNormalStream->sendCommand(cmd, arg1, arg2, arg3);
    }
    return ret;
}

MBOOL DIPStream_NormalStream::setJpegParam(NSIoPipe::NSPostProc::EJpgCmd jpgCmd,int arg1,int arg2)
{
    MBOOL ret = MFALSE;
    if( mNormalStream )
    {
        ret = mNormalStream->setJpegParam(jpgCmd, arg1, arg2);
    }
    return ret;
}

MBOOL DIPStream_NormalStream::setFps(MINT32 fps)
{
    MBOOL ret = MFALSE;
    if( mNormalStream )
    {
        ret = mNormalStream->setFps(fps);
    }
    return ret;
}

MUINT32 DIPStream_NormalStream::getRegTableSize()
{
    return NSCam::NSIoPipe::NSPostProc::INormalStream::getRegTableSize();
}

} // namespace P2Util
} // namespace Feature
} // namespace NSCam
