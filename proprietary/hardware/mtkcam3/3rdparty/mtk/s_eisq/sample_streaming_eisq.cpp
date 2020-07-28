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

#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>
#include <mtkcam3/3rdparty/core/buffer_helper.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <cutils/properties.h>
#include <queue>

using NSCam::NSPipelinePlugin::Interceptor;
using NSCam::NSPipelinePlugin::PipelinePlugin;
using NSCam::NSPipelinePlugin::PluginRegister;
using NSCam::NSPipelinePlugin::Join;
using NSCam::NSPipelinePlugin::JoinPlugin;

using namespace NSCam::NSPipelinePlugin;
using NSCam::MSize;

using NSCam::MERROR;
using NSCam::IImageBuffer;
using NSCam::IMetadata;
using NSCam::MRect;
using NSCam::MRectF;

#ifdef LOG_TAG
#undef LOG_TAG
#endif // LOG_TAG
#define LOG_TAG "MtkCam/TPI_S_EISQ"
#include <log/log.h>
#include <android/log.h>
#include <mtkcam/utils/std/ULog.h>

#define MY_LOGI(fmt, arg...)  CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)  CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)  CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)  CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define FUNCTION_IN   MY_LOGD("%s +", __FUNCTION__)
#define FUNCTION_OUT  MY_LOGD("%s -", __FUNCTION__)

CAM_ULOG_DECLARE_MODULE_ID(MOD_STREAMING_TPI_PLUGIN);

using NSCam::EImageFormat;
using NSCam::eImgFmt_NV21;
using NSCam::eImgFmt_NV12;
using NSCam::eImgFmt_YV12;
using NSCam::eImgFmt_YUY2;
using NSCam::eImgFmt_UNKNOWN;

class S_EISQ_Plugin : public JoinPlugin::IProvider
{
public:
    typedef JoinPlugin::Property Property;
    typedef JoinPlugin::Selection Selection;
    typedef JoinPlugin::Request::Ptr RequestPtr;
    typedef JoinPlugin::RequestCallback::Ptr RequestCallbackPtr;

    class QData
    {
    public:
        BufferHandle::Ptr   mInputHandle = NULL;
        IImageBuffer        *mInputBuffer = NULL;
        MRectF              mInputCrop;
        MRectF              mTargetCrop;
        bool                mChangeRoi = false;

        QData(){}
        QData(const BufferHandle::Ptr &handle, IImageBuffer *buf, const MRectF &inputCrop, const MRectF &targetCrop, bool changeRoi)
        : mInputHandle(handle)
        , mInputBuffer(buf)
        , mInputCrop(inputCrop)
        , mTargetCrop(targetCrop)
        , mChangeRoi(changeRoi)
        {}

        MVOID releaseInput()
        {
            release(mInputHandle);
            mInputBuffer = NULL;
            mInputHandle = NULL;
        }
    };

public:
    S_EISQ_Plugin();
    ~S_EISQ_Plugin();
    const Property& property();
    void set(MINT32 openID1, MINT32 openID2);
    void init();
    void uninit();
    MERROR negotiate(Selection& sel);
    MERROR process(RequestPtr pRequest, RequestCallbackPtr pCallback);
    void abort(std::vector<RequestPtr> &pRequests);

private:
    MERROR getConfigSetting(Selection &sel);
    MERROR getP1Setting(Selection &sel);
    MERROR getP2Setting(Selection &sel);
    bool checkConfig(const IImageBuffer *in) const;
    bool matchFormat(EImageFormat fmt) const;
    void copy(const IImageBuffer *in, IImageBuffer *out);
    void copyCrop(const IImageBuffer *in, IImageBuffer *out, const MRect &crop);
    void drawMask(IImageBuffer *buffer, float fx, float fy, float fw, float fh);

    void processDisplay(const QData &newData, IImageBuffer *disp, JoinImageInfo &outDispInfo);
    void processRecord(const QData &qData, IImageBuffer *rec, JoinImageInfo &outRecInfo);
    void moveDstRoi(const QData &inData, JoinImageInfo &outInfo);
    void printLog(const QData &data, IImageBuffer *out, const MRectF &outCrop, const char* str);
    void clearQueue();

private:
    static IImageBuffer* acquire(const BufferHandle::Ptr &handle);
    static void release(const BufferHandle::Ptr &handle);
    static IMetadata* acquire(const MetadataHandle::Ptr &handle);
    static void release(const MetadataHandle::Ptr &handle);

private:
    bool  mEISPreview = false;
    int   mMargin = 0;
    int   mQueueCount = 0;
    int   mRecordCount = 0;
    int   mOpenID1 = 0;
    int   mOpenID2 = 0;
    float mShrinkRatio = 0.0f;
    bool  mUseNV21 = false;
    bool  mUseNV12 = false;
    bool  mUseYV12 = false;
    bool  mUseYUY2 = false;
    std::queue<QData> mQueue;
};

S_EISQ_Plugin::S_EISQ_Plugin()
{
    MY_LOGI("create S_EISQ plugin");
}

S_EISQ_Plugin::~S_EISQ_Plugin()
{
    MY_LOGI("destroy S_EISQ plugin");
}

void S_EISQ_Plugin::set(MINT32 openID1, MINT32 openID2)
{
    MY_LOGD("set openID1:%d openID2:%d", openID1, openID2);
    mOpenID1 = openID1;
    mOpenID2 = openID2;
}

const S_EISQ_Plugin::Property& S_EISQ_Plugin::property()
{
    static Property prop;
    static bool inited;

    if (!inited) {
        prop.mName = "MTK EISQ";
        prop.mFeatures = MTK_FEATURE_EIS;
        inited = true;
    }
    return prop;
}

MERROR S_EISQ_Plugin::negotiate(Selection &sel)
{
    MERROR ret = OK;

    if( sel.mSelStage == eSelStage_CFG )
    {
        ret = getConfigSetting(sel);
    }
    else if( sel.mSelStage == eSelStage_P1 )
    {
        ret = getP1Setting(sel);
    }
    else if( sel.mSelStage == eSelStage_P2 )
    {
        ret = getP2Setting(sel);
    }

    return ret;
}

MERROR S_EISQ_Plugin::getConfigSetting(Selection &sel)
{
    MY_LOGI("max out size(%dx%d)",
            sel.mCfgInfo.mMaxOutSize.w, sel.mCfgInfo.mMaxOutSize.h);

    mEISPreview = property_get_bool("vendor.debug.tpi.s.eisq.preview", 0);
    mMargin = property_get_int32("vendor.debug.tpi.s.eisq.margin", 20);
    mQueueCount = property_get_int32("vendor.debug.tpi.s.eisq.qcount", 12);
    mShrinkRatio = 1.0f / (1.0f + ((float)mMargin / 100.0f)); // out * (1 + margin) = input
    mUseNV21 = property_get_bool("vendor.debug.tpi.s.eisq.nv21", 0);
    mUseNV12 = property_get_bool("vendor.debug.tpi.s.eisq.nv12", 0);
    mUseYV12 = property_get_bool("vendor.debug.tpi.s.eisq.yv12", 0);
    mUseYUY2 = property_get_bool("vendor.debug.tpi.s.eisq.yuy2", 0);

    sel.mCfgOrder = 20;
    sel.mCfgJoinEntry = eJoinEntry_S_DIV_2;
    //sel.mCfgEnableFD = MTRUE;
    sel.mCfgRun = property_get_bool("vendor.debug.tpi.s.eisq", 0);
    sel.mCfgQueueCount = mQueueCount;
    sel.mCfgMarginRatio = mMargin;
    sel.mIBufferMain1.setRequired(MTRUE);
    if( mUseNV21 ) sel.mIBufferMain1.addAcceptedFormat(eImgFmt_NV21);
    if( mUseNV12 ) sel.mIBufferMain1.addAcceptedFormat(eImgFmt_NV12);
    if( mUseYV12 ) sel.mIBufferMain1.addAcceptedFormat(eImgFmt_YV12);
    if( mUseYUY2 ) sel.mIBufferMain1.addAcceptedFormat(eImgFmt_YUY2);
    sel.mOBufferRecord.setRequired(MTRUE);

    if(mEISPreview)
    {
        sel.mOBufferDisplay.setRequired(MTRUE);
    }

    IMetadata *meta = sel.mIMetadataApp.getControl().get();
    MY_LOGD("sessionMeta=%p", meta);

    return OK;
}

MERROR S_EISQ_Plugin::getP1Setting(Selection &sel)
{
    (void)sel;
    return OK;
}

MERROR S_EISQ_Plugin::getP2Setting(Selection &sel)
{
    MBOOL run = MTRUE;
    sel.mP2Run = run;
    return OK;
}

bool S_EISQ_Plugin::checkConfig(const IImageBuffer *in) const
{
    bool ret = false;
    if( in )
    {
        ret = true;
        EImageFormat inFmt = (EImageFormat)in->getImgFormat();
        if( !matchFormat(inFmt) )
        {
            MY_LOGW("EISQ_PLUGIN: fmt not match: useNV21(0x%x)=%d useNV12(0x%x)=%d useYV12(0x%x)=%d useYUY2(0x%x)=%d inFmt(0x%x)",
                    eImgFmt_NV21, mUseNV21,
                    eImgFmt_NV12, mUseNV12,
                    eImgFmt_YV12, mUseYV12,
                    eImgFmt_YUY2, mUseYUY2,
                    inFmt);
            ret = false;
        }
    }
    return ret;
}

bool S_EISQ_Plugin::matchFormat(EImageFormat fmt) const
{
    if( mUseNV21 || mUseNV12 || mUseYV12 || mUseYUY2 )
    {
        return (mUseNV21 && fmt == eImgFmt_NV21) ||
               (mUseNV12 && fmt == eImgFmt_NV12) ||
               (mUseYV12 && fmt == eImgFmt_YV12) ||
               (mUseYUY2 && fmt == eImgFmt_YUY2);
    }
    return true;
}

void S_EISQ_Plugin::init()
{
    MY_LOGI("init S_EISQ plugin");
}

void S_EISQ_Plugin::uninit()
{
    MY_LOGI("uninit S_EISQ plugin, clear queue");
    clearQueue();
}

void S_EISQ_Plugin::abort(std::vector<RequestPtr> &pRequests)
{
    (void)pRequests;
    MY_LOGD("uninit S_EISQ plugin");
};

MERROR S_EISQ_Plugin::process(RequestPtr pRequest, RequestCallbackPtr pCallback)
{
    (void)pCallback;
    MERROR ret = -EINVAL;
    MBOOL needRun = MFALSE;
    IImageBuffer *in = NULL, *disp = NULL, *rec = NULL;

    MRectF &srcROI = pRequest->mIBufferMain1_Info.mISrcZoomROI;
    MBOOL cropValid = (srcROI.s.w > 0 && srcROI.s.h > 0 && srcROI.p.x >= 0.0f && srcROI.p.y >= 0.0f);

    needRun = MTRUE;
    if( !cropValid )
    {
        MY_LOGW("src zoom ROI is invalid !! (%f,%f, %fx%f)", srcROI.p.x, srcROI.p.y, srcROI.s.w, srcROI.s.h);
    }
    MY_LOGD("enter EISQ_PLUGIN needRun=%d, cropValid=%d", needRun, cropValid);

    if( needRun && pRequest->mIBufferMain1 != NULL && cropValid)
    {
        bool changeRoi = property_get_bool("vendor.debug.tpi.s.eisq.changeRoi", 0);
        in = acquire(pRequest->mIBufferMain1);
        disp = acquire(pRequest->mOBufferDisplay);
        rec = acquire(pRequest->mOBufferRecord);

        QData newData(pRequest->mIBufferMain1, in, pRequest->mIBufferMain1_Info.mISensorClip, pRequest->mIBufferMain1_Info.mISrcZoomROI, changeRoi);

        if( mEISPreview && disp )
        {
            processDisplay(newData, disp, pRequest->mOBufferDisplay_Info);
        }

        if( rec == NULL )
        {
            // Just do eis display, clear queue
            pRequest->mOQueueCmd = eJoinQueueCmd_POP_ALL;
            mRecordCount = 0;
            clearQueue();
            newData.releaseInput();
        }
        else if( mRecordCount < mQueueCount )
        {
            // Just do eis display, start queue
            pRequest->mOQueueCmd = eJoinQueueCmd_PUSH;
            mRecordCount += 1;
            mQueue.push(newData);
        }
        else
        {
            // Start Output record. The oldest QData need to be pop and release in this frame.
            pRequest->mOQueueCmd = eJoinQueueCmd_PUSH_POP;
            //mRecordCount += 1; // be careful record count over flow
            mQueue.push(newData);
            QData qData = mQueue.front();
            mQueue.pop();
            processRecord(qData, rec, pRequest->mOBufferRecord_Info);
            qData.releaseInput();
        }

        release(pRequest->mOBufferDisplay);
        release(pRequest->mOBufferRecord);
        // release(pRequest->mIBufferMain1) by qData.releaseInput()

        ret = OK;
    }
    MY_LOGD("exit EISQ_PLUGIN ret=%d", ret);
    return ret;
}

void S_EISQ_Plugin::printLog(const QData &data, IImageBuffer *out, const MRectF &outCrop, const char* str)
{
    if( out )
    {
        MY_LOGD("%s. In(%p) (%dx%d/%" PRId64 ") InCrop(%.3f,%.3f, %.3fx%.3f) TargetCrop(%.3f,%.3f, %.3fx%.3f), out(%dx%d/%" PRId64 "), outCrop(%.3f,%.3f,%.3fx%.3f)",
            str, data.mInputBuffer, data.mInputBuffer->getImgSize().w, data.mInputBuffer->getImgSize().h, data.mInputBuffer->getTimestamp(),
            data.mInputCrop.p.x, data.mInputCrop.p.y, data.mInputCrop.s.w, data.mInputCrop.s.h,
            data.mTargetCrop.p.x, data.mTargetCrop.p.y, data.mTargetCrop.s.w, data.mTargetCrop.s.h,
            out->getImgSize().w, out->getImgSize().h, out->getTimestamp(),
            outCrop.p.x, outCrop.p.y, outCrop.s.w, outCrop.s.h);
    }
    else
    {
        MY_LOGD("%s. In(%p) (%dx%d/%" PRId64 ") InCrop(%.3f,%.3f, %.3fx%.3f) TargetCrop(%.3f,%.3f, %.3fx%.3f), out=NULL, outCrop(%.3f,%.3f,%.3fx%.3f)",
            str, data.mInputBuffer, data.mInputBuffer->getImgSize().w, data.mInputBuffer->getImgSize().h, data.mInputBuffer->getTimestamp(),
            data.mInputCrop.p.x, data.mInputCrop.p.y, data.mInputCrop.s.w, data.mInputCrop.s.h,
            data.mTargetCrop.p.x, data.mTargetCrop.p.y, data.mTargetCrop.s.w, data.mTargetCrop.s.h,
            outCrop.p.x, outCrop.p.y, outCrop.s.w, outCrop.s.h);
    }
}

void S_EISQ_Plugin::processDisplay(const QData &inData, IImageBuffer *disp, JoinImageInfo &outDispInfo)
{
    IImageBuffer *in = inData.mInputBuffer;
    // Get AHardwareBuffer Example
    AHardwareBuffer* aIn = NSCam::BufferHelper::getAHWBuffer(in);
    AHardwareBuffer* aDisp = NSCam::BufferHelper::getAHWBuffer(disp);

    MSize inSize, outSize;
    MRect crop;

    if( in && disp )
    {
        inSize = in->getImgSize();
        outSize = MSize(inSize.w * mShrinkRatio, inSize.h * mShrinkRatio);
        outSize.w &= ~1; // 2-byte align test
        outSize.h &= ~1; // 2-byte align test

        // center crop and 2-byte align for Display test
        crop.p.x = ((inSize.w - outSize.w) / 2) & ~1;
        crop.p.y = ((inSize.h - outSize.h) / 2) & ~1;
        crop.s = outSize;
        if( checkConfig(in) )
        {
            copyCrop(in, disp, crop);
            drawMask(disp, 0.48, 0, 0.04, 1);
        }
    }
    MY_LOGD("display(%dx%d) = in(%dx%d/%dx%d)@(%d,%d) disp=%p/%p in=%p/%p",
            outSize.w, outSize.h, crop.s.w, crop.s.h, inSize.w, inSize.h,
            crop.p.x, crop.p.y, disp, aDisp, in, aIn);

    // set output buffer info
    outDispInfo.mOSrcImageClip = crop;
    outDispInfo.mODstImageClip.s = outSize;

    if( inData.mChangeRoi )
    {
        moveDstRoi(inData, outDispInfo);
    }
    printLog(inData, disp, outDispInfo.mOSrcImageClip, "EISQ Disp");
}

void S_EISQ_Plugin::processRecord(const QData &qData, IImageBuffer *rec, JoinImageInfo &outRecInfo)
{
    IImageBuffer *in = qData.mInputBuffer;
    // Get AHardwareBuffer Example
    AHardwareBuffer* aIn = NSCam::BufferHelper::getAHWBuffer(in);
    AHardwareBuffer* aRec = NSCam::BufferHelper::getAHWBuffer(rec);

    MSize inSize, outSize;
    MRect crop;

    // Record crop left top for test
    if( in && rec )
    {
        inSize = in->getImgSize();
        outSize = MSize(inSize.w * mShrinkRatio, inSize.h * mShrinkRatio);
        outSize.w &= ~1; // 2-byte align t est
        outSize.h &= ~1; // 2-byte align test
        crop.p.x = 0;
        crop.p.y = 0;
        crop.s = outSize;
        if( checkConfig(in) )
        {
            copy(in, rec);
            drawMask(rec, 0, 0.48, 1, 0.04);
         }
    }
    MY_LOGD("record(%dx%d) = in(%dx%d/%dx%d)@(%d,%d) rec=%p/%p in=%p/%p",
            outSize.w, outSize.h, crop.s.w, crop.s.h, inSize.w, inSize.h,
            crop.p.x, crop.p.y, rec, aRec, in, aIn);

    // set output buffer info
    outRecInfo.mOSrcImageClip = crop;
    outRecInfo.mODstImageClip.s = outSize;
    if( qData.mChangeRoi )
    {
        moveDstRoi(qData, outRecInfo);
    }
    printLog(qData, rec, outRecInfo.mOSrcImageClip, "EISQ Rec");
}

void S_EISQ_Plugin::moveDstRoi(const QData &inData, JoinImageInfo &outInfo)
{
    outInfo.mODstZoomROI.s.w = (inData.mTargetCrop.s.w <= outInfo.mODstImageClip.s.w)
                                    ? inData.mTargetCrop.s.w
                                    : outInfo.mODstImageClip.s.w;
    outInfo.mODstZoomROI.s.h = outInfo.mODstZoomROI.s.w * inData.mTargetCrop.s.h / inData.mTargetCrop.s.w;
    outInfo.mODstZoomROI.p.x = 0.0f;
    outInfo.mODstZoomROI.p.y = 0.0f;
}

void S_EISQ_Plugin::clearQueue()
{
    while( !mQueue.empty() )
    {
        QData data = mQueue.front();
        data.releaseInput();
        mQueue.pop();
    }
}

void S_EISQ_Plugin::copy(const IImageBuffer *in, IImageBuffer *out)
{
    if( !in || !out )
    {
        return;
    }

    unsigned inPlane = in->getPlaneCount();
    unsigned outPlane = out->getPlaneCount();

    for( unsigned i = 0; i < inPlane && i < outPlane; ++i )
    {
        char *inPtr = (char*)in->getBufVA(i);
        char *outPtr = (char*)out->getBufVA(i);
        unsigned inStride = in->getBufStridesInBytes(i);
        unsigned outStride = out->getBufStridesInBytes(i);
        unsigned inBytes = in->getBufSizeInBytes(i);
        unsigned outBytes = out->getBufSizeInBytes(i);

        if( !inPtr || !outPtr || !inStride || !outStride )
        {
            continue;
        }

        if( inStride == outStride )
        {
            memcpy(outPtr, inPtr, std::min(inBytes, outBytes));
        }
        else
        {
            unsigned stride = std::min(inStride, outStride);
            unsigned height = std::min(inBytes/inStride, outBytes/outStride);
            for( unsigned y = 0; y < height; ++y )
            {
                memcpy(outPtr+y*outStride, inPtr+y*inStride, stride);
            }
        }
    }
}

void S_EISQ_Plugin::copyCrop(const IImageBuffer *in, IImageBuffer *out, const MRect &crop)
{
    if( in && out )
    {
        EImageFormat inFmt = (EImageFormat)in->getImgFormat();
        EImageFormat outFmt = (EImageFormat)out->getImgFormat();
        unsigned inIBPP = in->getImgBitsPerPixel();
        unsigned outIBPP = out->getImgBitsPerPixel();
        unsigned inPlane = in->getPlaneCount();
        unsigned outPlane = out->getPlaneCount();
        MSize inSize = in->getImgSize();
        MSize outSize = out->getImgSize();
        bool check1 = true;

        check1 = (inFmt == outFmt) && (inPlane == outPlane) && (inPlane > 0);
        MY_LOGD("run=%d in:size(%dx%d) fmt(0x%x) IBPP(%d) plane(%d)",
                check1, inSize.w, inSize.h, inFmt, inIBPP, inPlane);
        MY_LOGD("run=%d out:size(%dx%d) fmt(0x%x) IBPP(%d) plane(%d)",
                check1, outSize.w, outSize.h, outFmt, outIBPP, outPlane);
        unsigned imgStrideP = check1 ? in->getBufStridesInPixel(0) : 1;
        unsigned imgLineP = check1 ? in->getBufScanlines(0) : 1;

        for( unsigned i = 0; i < inPlane; ++i )
        {
            char *inPtr = (char*)in->getBufVA(i);
            char *outPtr = (char*)out->getBufVA(i);
            unsigned inBPP = in->getPlaneBitsPerPixel(i);
            unsigned outBPP = out->getPlaneBitsPerPixel(i);
            unsigned inBytes = in->getBufSizeInBytes(i);
            unsigned outBytes = out->getBufSizeInBytes(i);
            unsigned inStrideB = in->getBufStridesInBytes(i);
            unsigned outStrideB = out->getBufStridesInBytes(i);
            unsigned inStrideP = in->getBufStridesInPixel(i);
            unsigned outStrideP = out->getBufStridesInPixel(i);
            unsigned inLineP = in->getBufScanlines(i);
            unsigned outLineP = out->getBufScanlines(i);
            bool check2 = check1 && inPtr && outPtr && inStrideB && outStrideB && inStrideP && outStrideP && (inBPP == outBPP);
            unsigned sampleH = check2 ? imgStrideP / inStrideP : 1;
            unsigned sampleV = check2 ? imgLineP / inLineP : 1;

            MY_LOGD("check2=%d in[%d]: bpp(%d) bytes(%d) stride(%d/%d) line(%d)", check2, i, inBPP, inBytes, inStrideB, inStrideP, inLineP);
            MY_LOGD("check2=%d out[%d]: bpp(%d) bytes(%d) stride(%d/%d) line(%d)", check2, i, outBPP, outBytes, outStrideB, outStrideP, outLineP);

            MRect cropP = crop;
            cropP.p.x /= sampleH;
            cropP.p.y /= sampleV;
            cropP.s.w /= sampleH;
            cropP.s.h /= sampleV;

            unsigned cropWidthByte = cropP.s.w * inBPP / 8;
            unsigned inOffsetX = cropP.p.x * inBPP / 8;
            unsigned inOffsetY = cropP.p.y * inStrideB;
            if( check2 )
            {
                for( unsigned dy = 0; dy < (unsigned)cropP.s.h; ++dy )
                {
                    unsigned inOffset = dy * inStrideB + inOffsetX + inOffsetY;
                    unsigned outOffset = dy * outStrideB;
                    memcpy(outPtr + outOffset, inPtr + inOffset, cropWidthByte);
                }
            }
        }
    }
}

void S_EISQ_Plugin::drawMask(IImageBuffer *buffer, float fx, float fy, float fw, float fh)
{
    // sample: modify output buffer
    if( buffer )
    {
        char *ptr = (char*)buffer->getBufVA(0);
        if( ptr )
        {
            char mask = 128;
            MSize size = buffer->getImgSize();
            int stride = buffer->getBufStridesInBytes(0);
            int y_from = fy * size.h;
            int y_to = (fy + fh) * size.h;
            int x = fx * size.w;
            int width = fw * size.w;

            y_to = std::max(0, std::min(y_to, size.h));
            y_from = std::max(0, std::min(y_from, y_to));
            x = std::max(0, std::min(x, size.w));
            width = std::max(0, std::min(width, size.w));

            for( int y = y_from; y < y_to; ++y )
            {
                memset(ptr+y*stride + x, mask, width);
            }
        }
    }
}

IImageBuffer* S_EISQ_Plugin::acquire(const BufferHandle::Ptr &handle)
{
    return handle != NULL ? handle->acquire() : NULL;
}

void S_EISQ_Plugin::release(const BufferHandle::Ptr &handle)
{
    return handle != NULL ? handle->release() : void();
}

IMetadata* S_EISQ_Plugin::acquire(const MetadataHandle::Ptr &handle)
{
    return handle != NULL ? handle->acquire() : NULL;
}

void S_EISQ_Plugin::release(const MetadataHandle::Ptr &handle)
{
    return handle != NULL ? handle->release() : void();
}

//REGISTER_PLUGIN_PROVIDER_DYNAMIC(Join, S_EISQ_Plugin, MTK_FEATURE_EIS);
REGISTER_PLUGIN_PROVIDER(Join, S_EISQ_Plugin);
