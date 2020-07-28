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

#define PROCESSOR_NAME_P2   ("P2")
#define DEFINE_OPEN_ID      (muOpenId)

#include "../P2Common.h"
#include "P2Procedure.h"

#include <fstream>
#include <mtkcam/v3/hal/IHal3A.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IPortEnum.h>
#include <mtkcam/featureio/eis_hal_base.h>


using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc_FrmB;
using namespace NS3Av3;

static const PortID PORT_IMGI  (EPortType_Memory, EPipePortIndex_IMGI, 0);
static const PortID PORT_WDMAO (EPortType_Memory, EPipePortIndex_WDMAO, 1);
static const PortID PORT_WROTO (EPortType_Memory, EPipePortIndex_WROTO, 1);
#if SUPPORT_FD_PORT
static const PortID PORT_IMG2O (EPortType_Memory, EPipePortIndex_IMG2O, 1);
#endif

#define DEBUG_PORT_IN_IMGO          (0x1)
#define DEBUG_PORT_IN_RRZO          (0x2)
#define DEBUG_PORT_OUT_WDMAO        (0x1)
#define DEBUG_PORT_OUT_WROTO        (0x2)
#define DEBUG_PORT_OUT_IMG2O        (0x4)

#define DEBUG_DRAWLINE_PORT_WDMAO   (0x1)
#define DEBUG_DRAWLINE_PORT_WROTO   (0x2)
#define DEBUG_DRAWLINE_PORT_IMG2O   (0x4)

#define DUMP_P2_BUF_PREVIEW     1
#define DUMP_P2_BUF_CAPTURE     2
#define DUMP_P2_BUF_ALL         3


/******************************************************************************
 *
 ******************************************************************************/
P2Procedure::
P2Procedure(CreateParams const &params)
        : mCreateParams(params),
          mpPipe(params.pPipe),
          mp3A(params.p3A),
          muEnqueCnt(0),
          muDequeCnt(0),
          muRequestCnt(0),
          mbEnableLog(params.bEnableLog),
          muOpenId(params.uOpenId),
          mpPoolAllocatorMap(params.pPoolAllocatorMap)
{

    mSupportPlugin = ::property_get_int32("vendor.debug.camera.support.plugin", 0);

    mnStreamTag = (params.type == P2Node::PASS2_TIMESHARING)
                  ? ENormalStreamTag_Vss
                  : ENormalStreamTag_Stream;

}


/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
P2Procedure::
createProcessor(CreateParams &params) {
    INormalStream *pPipe = NULL;
    IHal3A *p3A = NULL;

    ENormalStreamTag streamtag = ENormalStreamTag_Stream;
    if( params.type == P2Node::PASS2_STREAM ) {
        streamtag = ENormalStreamTag_Stream;
    }
    else if ( params.type == P2Node::PASS2_TIMESHARING ) {
        streamtag = ENormalStreamTag_Vss;
    }
    else {
        MY_LOGE("not supported type %d", params.type);
        goto lbExit;
    }

    CAM_TRACE_BEGIN("P2:NormalStream:create");
    pPipe = INormalStream::createInstance(LOG_TAG, streamtag, params.uOpenId, true);
    CAM_TRACE_END();
    if (pPipe == NULL) {
        MY_LOGE("create pipe failed");
        goto lbExit;
    }

    CAM_TRACE_BEGIN("P2:NormalStream:init");
    if (!pPipe->init()) {
        CAM_TRACE_END();
        MY_LOGE("pipe init failed");
        goto lbExit;
    }
    CAM_TRACE_END();

#if SUPPORT_3A
    CAM_TRACE_BEGIN("P2:3A:create");
    p3A = IHal3A::createInstance(IHal3A::E_Camera_3, params.uOpenId, LOG_TAG);
    CAM_TRACE_END();
#endif
    if (p3A == NULL) {
        MY_LOGE("create 3A failed");
        goto lbExit;
    }
    MY_LOGD("create processor type %d: pipe %p, 3A %p",
            params.type, pPipe, p3A);

    lbExit:
    if (!pPipe || !p3A) {
        if (pPipe) {
            pPipe->uninit();
            pPipe->destroyInstance(LOG_TAG);
            pPipe = NULL;
        }
        if (p3A) {
            p3A->destroyInstance(LOG_TAG);
            p3A = NULL;
        }
    }

    params.pPipe = pPipe;
    params.p3A = p3A;
    return pPipe ? new ProcessorBase<P2Procedure>(
            params.uOpenId, params, PROCESSOR_NAME_P2) : NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
mapPortId(
        StreamId_T const streamId,  // [in]
        MUINT32 const transform,    // [in]
        MBOOL const isFdStream,     // [in]
        MUINT8 &rOccupied,          // [in/out]
        PortID &rPortId             // [out]
) const
{
    MERROR ret = OK;
#define PORT_WDMAO_USED  (0x1)
#define PORT_WROTO_USED  (0x2)
#define PORT_IMG2O_USED  (0x4)
    if (transform != 0) {
        if( !(rOccupied & PORT_WROTO_USED) )
            rOccupied |= PORT_WROTO_USED;
        else
            ret = INVALID_OPERATION;
    }
    else {
#if SUPPORT_FD_PORT
        if (SUPPORT_FD_PORT && isFdStream) {
            if (rOccupied & PORT_IMG2O_USED) {
                MY_LOGWO("should not be occupied");
                ret = INVALID_OPERATION;
            } else {
                rOccupied |= PORT_IMG2O_USED;
                rPortId = PORT_IMG2O;
            }
        } else
#endif
        if (!(rOccupied & PORT_WDMAO_USED)) {
            rOccupied |= PORT_WDMAO_USED;
            rPortId = PORT_WDMAO;
        } else if (!(rOccupied & PORT_WROTO_USED)) {
            rOccupied |= PORT_WROTO_USED;
            rPortId = PORT_WROTO;
        } else
            ret = INVALID_OPERATION;
    }
    MY_LOGDO_IF(0, "stream id %#" PRIx64 ", occupied 0x%x",
               streamId, rOccupied);
    return ret;
#undef PORT_WDMAO_USED
#undef PORT_WROTO_USED
#undef PORT_IMG2O_USED
}


/******************************************************************************
 *
 ******************************************************************************/
P2Procedure::
~P2Procedure() {
    MY_LOGDO_IF(mbEnableLog, "destroy processor %d: %p", mCreateParams.type, mpPipe);

    if (mpPipe) {
        if (!mpPipe->uninit()) {
            MY_LOGEO("pipe uninit failed");
        }
        mpPipe->destroyInstance(LOG_TAG);
    }

    if (mp3A) {
        mp3A->destroyInstance(LOG_TAG);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
onP2Execute(
        sp<Request> const pRequest,
        FrameParams const &params)
{
    CAM_TRACE_NAME("P2:onP2Execute");
    FUNC_START;

    MERROR ret = OK;
    if (OK != (ret = checkParams(params)))
        return ret;
    // prepare metadata
    IMetadata *pMeta_InApp = params.inApp->getMetadata();
    IMetadata *pMeta_InHal = params.inHal->getMetadata();
    IMetadata *pMeta_OutApp = params.outApp.get() ? params.outApp->getMetadata() : NULL;
    IMetadata *pMeta_OutHal = params.outHal.get() ? params.outHal->getMetadata() : NULL;

    if (pMeta_InApp == NULL || (!params.bYuvReproc && pMeta_InHal == NULL)) {
        MY_LOGEO("meta: in app %p, in hal %p", pMeta_InApp, pMeta_InHal);
        return BAD_VALUE;
    }

    sp<Cropper::CropInfo> pCropInfo = new Cropper::CropInfo;
    if (OK != (ret = getCropInfo(pMeta_InApp, pMeta_InHal, params.bResized, *pCropInfo))) {
        MY_LOGEO("getCropInfo failed");
        return ret;
    }
    pRequest->context.crop_info = pCropInfo;

    String8 strEnqueLog;
    QParams enqueParams;
    //frame tag
    //enqueParams.mvStreamTag.push_back(mnStreamTag);

    // input
    {
        if (OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE))) {
            MY_LOGWO("src buffer err = %d", ret);
            return BAD_VALUE;
        }
        IImageBuffer *pSrc = params.in.mHandle->getBuffer();

        Input src;
        src.mPortID = params.in.mPortId;
        src.mPortID.group = 0;
        src.mBuffer = pSrc;
        // update src size
#if SUPPORT_PLUGIN
        if(mSupportPlugin == 1)
        {
            if (params.bRunWorkBuffer) {
                // do nothing if run on working buffer
            }
            else
            {
                pSrc->setExtParam(pCropInfo->dstsize_resizer);
            }
        }
        else
        {
            pSrc->setExtParam(pCropInfo->dstsize_resizer);
        }
#else
        pSrc->setExtParam(pCropInfo->dstsize_resizer);
#endif

        enqueParams.mvIn.push_back(src);
        strEnqueLog += String8::format("EnQ: Src Port(%d) Fmt(0x%x) Size(%dx%d) => ",
                   src.mPortID.index, src.mBuffer->getImgFormat(),
                   src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h);
    }

    // output
    for (size_t i = 0; i < params.vOut.size(); i++) {
        if (params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE))) {
            MY_LOGWO("dst buffer err = %d", ret);
            continue;
        }
        IImageBuffer *pDst = params.vOut[i].mHandle->getBuffer();

        Output dst;
        dst.mPortID = params.vOut[i].mPortId;
        dst.mPortID.group = 0;
        dst.mBuffer = pDst;

        enqueParams.mvOut.push_back(dst);
    }

    if (enqueParams.mvOut.size() == 0) {
        //MY_LOGWO("no dst buffer");
        return BAD_VALUE;
    }

    {
        void* pTuning = NULL;
        unsigned int tuningsize;
        if( !mpPipe->deTuningQue(tuningsize, pTuning) ) {
            MY_LOGW("cannot get tunning buffer");
            return BAD_VALUE;
        }

        MetaSet_T inMetaSet;
        MetaSet_T outMetaSet;

        inMetaSet.appMeta = *pMeta_InApp;
        inMetaSet.halMeta = *pMeta_InHal;

        MBOOL const bGetResult = (pMeta_OutApp || pMeta_OutHal);

        if( pMeta_OutHal ) {
            outMetaSet.halMeta = *pMeta_InHal;
        }

        if (mp3A) {
            MY_LOGDO_IF(mbEnableLog, "P2 setIsp %p : %d", pTuning, tuningsize);
            mp3A->setIsp(0, inMetaSet, pTuning, bGetResult ? &outMetaSet : NULL);
            {
                enqueParams.mpTuningData.push_back(pTuning);
            }
        } else {
            MY_LOGDO_IF(mbEnableLog, "P2 setIsp clear tuning %p : %d", pTuning, tuningsize);
            ::memset((unsigned char *) (pTuning), 0, tuningsize);
        }

        if (pMeta_OutApp) {
            *pMeta_OutApp = outMetaSet.appMeta;

            MRect cropRegion = pCropInfo->crop_a;
            if (pCropInfo->isEisEabled) {
                cropRegion.p.x += pCropInfo->eis_mv_a.p.x;
                cropRegion.p.y += pCropInfo->eis_mv_a.p.y;
            }
            updateCropRegion(cropRegion, pMeta_OutApp);
        }

        if (pMeta_OutHal) {
            *pMeta_OutHal = outMetaSet.halMeta;
            (*pMeta_OutHal) = inMetaSet.halMeta + outMetaSet.halMeta;
        }
    }
    // for output group crop
    {
        MCrpRsInfo crop1;
        crop1.mGroupID    = 1;
        crop1.mCropRect.s = pCropInfo->dstsize_resizer;
        crop1.mResizeDst  = pCropInfo->dstsize_resizer;
        MCrpRsInfo crop2;
        crop2.mGroupID = 2;
        //crop2.mCropRect = calCrop(info, dstSize); //TODO
        //crop2.mResizeDst  = MSize(0,0);
        Vector<Output>::const_iterator iter = enqueParams.mvOut.begin();
        while (iter != enqueParams.mvOut.end()) {
            if( iter->mPortID == PORT_WDMAO
                || iter->mPortID == PORT_WROTO ) {

#if SUPPORT_PLUGIN
                if(mSupportPlugin == 1) {
                    if (iter->mPortID == PORT_WDMAO && params.bRunWorkBuffer) {
                        crop2.mCropRect.p_fractional = {0, 0};
                        crop2.mCropRect.p_integral = {0, 0};
                        crop2.mCropRect.s = iter->mBuffer->getImgSize();
                    } else {
                        Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop2.mCropRect);
                    }
                } else {
                    Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop2.mCropRect);
                }
#else
                Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop2.mCropRect);
#endif

                crop2.mResizeDst = iter->mBuffer->getImgSize();
                if (crop2.mResizeDst.w / crop2.mCropRect.s.w > 32
                        || crop2.mResizeDst.h / crop2.mCropRect.s.h > 32) {
                    int logw = crop2.mCropRect.s.w;
                    int logh = crop2.mCropRect.s.h;
                    crop2.mCropRect.s.w = crop2.mResizeDst.w / 32;
                    crop2.mCropRect.s.h = crop2.mResizeDst.h / 32;
                    MY_LOGWO("resize ratio > 32: s(%dx%d) -> s(%dx%d)",
                            logw, logh, crop2.mCropRect.s.w, crop2.mCropRect.s.h);
                }
                break;
            }
            iter++;
        }
        enqueParams.mvCropRsInfo.push_back(crop1);
        enqueParams.mvCropRsInfo.push_back(crop2);
    }

    MY_LOGDO("%s", strEnqueLog.string());

    // callback
    enqueParams.mpfnCallback = pass2CbFunc;
    enqueParams.mpCookie = this;
    enqueParams.mFrameNo = pRequest->getFrameNo();
    MUINT8 capture_intent = -1;
    if (!tryGetMetadata<MUINT8>(pMeta_InApp, MTK_CONTROL_CAPTURE_INTENT, capture_intent)) {
        MY_LOGD("Get capture intent fail");
    }
    enqueParams.mpPrivaData = reinterpret_cast<void*>(capture_intent);
    enqueParams.mvPrivaData.push_back(NULL);

    // for crop
    enqueParams.mvP1SrcCrop.push_back(pCropInfo->crop_p1_sensor);
    enqueParams.mvP1Dst.push_back(pCropInfo->dstsize_resizer);
    enqueParams.mvP1DstCrop.push_back(pCropInfo->crop_dma);

    MY_LOGDO("p2 enque count:%d, size[in/out]:%d/%d",
               muEnqueCnt, enqueParams.mvIn.size(), enqueParams.mvOut.size());
    // add request to queue
    {
        Mutex::Autolock _l(mLock);
        mvRunning.push_back(pRequest);
        muEnqueCnt++;
    }

    {
        MY_LOGDO_IF(mbEnableLog, "p2 enque +");
        CAM_TRACE_NAME("P2:Driver:enque");
        if (!mpPipe->enque(enqueParams)) {
            MY_LOGEO("p2 enque failed");
            // remove job from queue
            {
                Mutex::Autolock _l(mLock);
                vector<sp<Request>>::iterator iter = mvRunning.end();
                while (iter != mvRunning.begin()) {
                    iter--;
                    if (*iter == pRequest) {
                        mvRunning.erase(iter);
                        break;
                    }
                }

                MY_LOGEO("p2 deque count:%d, enque failed", muDequeCnt);
                muDequeCnt++;
                AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam/P2Node:ISP pass2 deque fail");
            }
            return UNKNOWN_ERROR;
        }
        MY_LOGDO_IF(mbEnableLog, "p2 enque -");
    }

    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
onP2Finish(
        FrameParams const &params,
        MBOOL const success)
{
    CAM_TRACE_NAME("P2:onP2Finish");
    //params.in.mHandle->updateState(BufferHandle::Buffer_ReadDone);
    for (size_t i = 0; i < params.vOut.size(); i++)
        if (params.vOut[i].mHandle.get())
            params.vOut[i].mHandle->updateState(
                    success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
            );
    if (params.outApp.get())
        params.outApp->updateState(success ? MetaHandle::STATE_WRITE_OK : MetaHandle::STATE_WRITE_FAIL);
    if (params.outHal.get())
        params.outHal->updateState(success ? MetaHandle::STATE_WRITE_OK : MetaHandle::STATE_WRITE_FAIL);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
onP2Flush() {
    CAM_TRACE_NAME("P2:onP2Flush");
    return;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
onP2Notify(
        MUINT32 const event,
        MINTPTR const arg1,
        MINTPTR const arg2,
        MINTPTR const arg3)
{
    return;
}

static int enableP2RawDump() {
    char value[PROPERTY_VALUE_MAX];
    memset(value, 0x0, sizeof(value));
    property_get("vendor.debug.camera.dump_p2_buf", value, "0");
    return atoi(value);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
pass2CbFunc(QParams &rParams) {
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc +++");
    if (enableP2RawDump() > 0) {
        if (enableP2RawDump() == DUMP_P2_BUF_ALL
                || enableP2RawDump() == reinterpret_cast<int>(rParams.mpPrivaData)) {
            for(int i = 0; i < rParams.mvOut.size(); ++i) {
                char filename[256];
                sprintf(filename, "/data/vendor/camera_dump/P2_%d_%dx%d.yuv", rParams.mFrameNo,
                        rParams.mvOut[i].mBuffer->getImgSize().w,
                        rParams.mvOut[i].mBuffer->getImgSize().h);
                NSCam::Utils::saveBufToFile(filename,
                        (unsigned char*)rParams.mvOut[i].mBuffer->getBufVA(0),
                        rParams.mvOut[i].mBuffer->getBufSizeInBytes(0));
                MY_LOGD("Save P2 Buf to Files %s", filename);
            }
        }
    }
    P2Procedure *pProcedure = reinterpret_cast<P2Procedure *>(rParams.mpCookie);
    pProcedure->handleDeque(rParams);
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc ---");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
handleDeque(QParams &rParams) {
    CAM_TRACE_NAME("P2:handleDeque");
    Mutex::Autolock _l(mLock);
    sp<Request> pRequest = NULL;
    {
        MY_LOGDO("p2 deque count:%d, result:%d", muDequeCnt, rParams.mDequeSuccess);
        pRequest = mvRunning.front();
        mvRunning.erase(mvRunning.begin());
        muDequeCnt++;
    }
    //
    if (rParams.mpTuningData.size() > 0) {
        void* pTuning = rParams.mpTuningData[0];
        if( pTuning ) mpPipe->enTuningQue(pTuning);
    }

    pRequest->responseDone(rParams.mDequeSuccess ? OK : UNKNOWN_ERROR);

    mCondJob.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
checkParams(FrameParams const params) const {
#define CHECK(val, fail_ret, ...) \
    do{                           \
        if( !(val) )              \
        {                         \
            MY_LOGEO(__VA_ARGS__); \
            return fail_ret;      \
        }                         \
    } while(0)

    CHECK(params.in.mHandle.get(), BAD_VALUE, "no src handle");
    CHECK(params.vOut.size(), BAD_VALUE, "no dst");
    CHECK(params.inApp.get(), BAD_VALUE, "no in app meta");
    CHECK(params.inHal.get(), BAD_VALUE, "no in hal meta");
#undef CHECK
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
getCropInfo(
        IMetadata *const inApp,
        IMetadata *const inHal,
        MBOOL const isResized,
        Cropper::CropInfo &cropInfo) const
{
    if (!tryGetMetadata<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, cropInfo.sensor_size)) {
        MY_LOGEO("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        return BAD_VALUE;
    }

    MSize const sensor = cropInfo.sensor_size;
    MSize const active = mCreateParams.activeArray.s;

    cropInfo.isResized = isResized;
    // get current p1 buffer crop status
    if (!isResized ||
	    !(tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, cropInfo.crop_p1_sensor) &&
          tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE, cropInfo.dstsize_resizer) &&
          tryGetMetadata<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION, cropInfo.crop_dma)))
    {
        MY_LOGW_IF(isResized, "cannot find tag, use it as full-size");

        cropInfo.crop_p1_sensor = MRect(MPoint(0, 0), sensor);
        cropInfo.dstsize_resizer = sensor;
        cropInfo.crop_dma = MRect(MPoint(0, 0), sensor);
    }

    MY_LOGDO_IF(P2_DEBUG_LOG, "SCALAR_CROP_REGION:(%d,%d)(%dx%d) RESIZER_SIZE:(%dx%d) DMA_CROP_REGION:(%d,%d)(%dx%d)",
               cropInfo.crop_p1_sensor.p.x, cropInfo.crop_p1_sensor.p.y,
               cropInfo.crop_p1_sensor.s.w, cropInfo.crop_p1_sensor.s.h,
               cropInfo.dstsize_resizer.w, cropInfo.dstsize_resizer.h,
               cropInfo.crop_dma.p.x, cropInfo.crop_dma.p.y,
               cropInfo.crop_dma.s.w, cropInfo.crop_dma.s.h);

    cropInfo.tranActive2Sensor = simpleTransform(
                MPoint(0,0),
                active,
                sensor
            );

    cropInfo.tranSensor2Resized = simpleTransform(
            cropInfo.crop_p1_sensor.p,
            cropInfo.crop_p1_sensor.s,
            cropInfo.dstsize_resizer
    );

    MBOOL const isEisOn = isEISOn(inApp);

    MRect cropRegion; //active array domain
    queryCropRegion(inApp, isEisOn, cropRegion);
    cropInfo.crop_a = cropRegion;

    // query EIS result
    {
        eis_region eisInfo;
        if (isEisOn && queryEisRegion(inHal, eisInfo)) {
            cropInfo.isEisEabled = MTRUE;
            // calculate mv
            vector_f *pMv_s = &cropInfo.eis_mv_s;
            vector_f *pMv_r = &cropInfo.eis_mv_r;
            MBOOL isResizedDomain = MTRUE;
#if 1
            //eis in sensor domain
            isResizedDomain = MFALSE;
            pMv_s->p.x  = eisInfo.x_int - (sensor.w * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_s->pf.x = eisInfo.x_float;
            pMv_s->p.y  = eisInfo.y_int - (sensor.h * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_s->pf.y = eisInfo.y_float;

            cropInfo.eis_mv_r = transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_s);
#else
            MSize const resizer = cropInfo.dstsize_resizer;
            //eis in resized domain
            pMv_r->p.x = eisInfo.x_int - (resizer.w * (EIS_FACTOR - 100) / 2 / EIS_FACTOR);
            pMv_r->pf.x = eisInfo.x_float;
            pMv_r->p.y = eisInfo.y_int - (resizer.h * (EIS_FACTOR - 100) / 2 / EIS_FACTOR);
            pMv_r->pf.y = eisInfo.y_float;
            //
            cropInfo.eis_mv_s = inv_transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_r);
            //
            MY_LOGDO_IF(P2_DEBUG_LOG, "mv (%s): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
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
            cropInfo.eis_mv_a = inv_transform(cropInfo.tranActive2Sensor, cropInfo.eis_mv_s);

            MY_LOGDO_IF(P2_DEBUG_LOG, "mv in active %d/%d, %d/%d",
                       cropInfo.eis_mv_a.p.x,
                       cropInfo.eis_mv_a.pf.x,
                       cropInfo.eis_mv_a.p.y,
                       cropInfo.eis_mv_a.pf.y
            );
        }
        else {
            cropInfo.isEisEabled = MFALSE;
            // no need to set 0
            //memset(&cropInfo.eis_mv_a, 0, sizeof(vector_f));
            //memset(&cropInfo.eis_mv_s, 0, sizeof(vector_f));
            //memset(&cropInfo.eis_mv_r, 0, sizeof(vector_f));
        }
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
queryCropRegion(
        IMetadata *const meta_request,
        MBOOL const isEisOn,
        MRect &cropRegion) const
{
    if (!tryGetMetadata<MRect>(meta_request, MTK_SCALER_CROP_REGION, cropRegion)) {
        cropRegion.p = MPoint(0, 0);
        cropRegion.s = mCreateParams.activeArray.s;
        MY_LOGWO_IF(mbEnableLog, "no MTK_SCALER_CROP_REGION, crop full size %dx%d",
                   cropRegion.s.w, cropRegion.s.h);
    }
    MY_LOGDO_IF(mbEnableLog, "control: cropRegion(%d, %d, %dx%d)",
               cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

    if (isEisOn) {
        cropRegion.p.x += (cropRegion.s.w * (EIS_FACTOR - 100) / 2 / EIS_FACTOR);
        cropRegion.p.y += (cropRegion.s.h * (EIS_FACTOR - 100) / 2 / EIS_FACTOR);
        cropRegion.s = cropRegion.s * 100 / EIS_FACTOR;
        MY_LOGDO_IF(mbEnableLog, "EIS: factor %d, cropRegion(%d, %d, %dx%d)",
                   EIS_FACTOR, cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
updateCropRegion(
        MRect const crop,
        IMetadata *meta_result) const
{
    trySetMetadata<MRect>(meta_result, MTK_SCALER_CROP_REGION, crop);
    MY_LOGDO_IF(P2_DEBUG_LOG && mbEnableLog, "result: cropRegion (%d, %d, %dx%d)",
               crop.p.x, crop.p.y, crop.s.w, crop.s.h);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2Procedure::
isEISOn(IMetadata *const inApp) const {
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if (!tryGetMetadata<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode)) {
        MY_LOGWO_IF(mbEnableLog, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
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
        IMetadata *const inHal,
        eis_region &region
) const {
    IMetadata::IEntry entry = inHal->entryFor(MTK_EIS_REGION);

    // get EIS's region
    if (entry.count() > 5) {
        region.x_int   = entry.itemAt(0, Type2Type<MINT32>());
        region.x_float = entry.itemAt(1, Type2Type<MINT32>());
        region.y_int   = entry.itemAt(2, Type2Type<MINT32>());
        region.y_float = entry.itemAt(3, Type2Type<MINT32>());
        region.s.w     = entry.itemAt(4, Type2Type<MINT32>());
        region.s.h     = entry.itemAt(5, Type2Type<MINT32>());

        MY_LOGDO_IF(mbEnableLog, "EIS Region: %d, %d, %d, %d, %dx%d",
                   region.x_int,
                   region.x_float,
                   region.y_int,
                   region.y_float,
                   region.s.w,
                   region.s.h);
        return MTRUE;
    }

    MY_LOGWO("wrong eis region count %zu", entry.count());
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::
onP2PullParams(
        sp<Request> pRequest,
        FrameParams &param_p2)
{
    MBOOL bOutWorkingBuffer;
#if SUPPORT_PLUGIN
    if (mSupportPlugin == 1) {
        bOutWorkingBuffer = MFALSE;
    }
#endif

    param_p2.uUniqueKey = pRequest->getUniqueKey();
    param_p2.uRequestNo = pRequest->getRequestNo();
    param_p2.uFrameNo = pRequest->getFrameNo();

    // input buffer
    param_p2.in.mPortId = PORT_IMGI;
    param_p2.bResized = pRequest->context.resized;

#if SUPPORT_PLUGIN
    if (mSupportPlugin == 1) {
        bOutWorkingBuffer |= pRequest->context.run_plugin_yuv;
    }
#endif
    {
        param_p2.in.mHandle = pRequest->context.in_buffer;
        pRequest->context.in_buffer.clear();
    }

    // output buffer
    MUINT8 occupied = 0;
    MBOOL remains = MFALSE;

    sp<BufferHandle> pCopyCandidate = NULL;

#if SUPPORT_PLUGIN
    if (mSupportPlugin == 1) {
        param_p2.bRunWorkBuffer = bOutWorkingBuffer;
    }

    if (mSupportPlugin == 1 && bOutWorkingBuffer) {
        MY_LOGDO_IF(mbEnableLog, "output to working buffer");
        MERROR ret = OK;
        if (OK != (ret = param_p2.in.mHandle->waitState(BufferHandle::STATE_READABLE))) {
            MY_LOGWO("input buffer err = %d", ret);
            return ret;
        }

        IImageBuffer *pInImageBuffer = param_p2.in.mHandle->getBuffer();
        if (pInImageBuffer == NULL) {
            MY_LOGEO("no input buffer");
            return UNKNOWN_ERROR;
        }

        sp<BufferHandle> pBufferHandle = NULL;
        if (pRequest->context.work_buffer_format == NULL) {
            pBufferHandle = WorkingBufferHandle::create(
                    (pRequest->context.nr_type == Request::NR_TYPE_SWNR) ? "SWNR" : "MNR",
                    (pRequest->context.nr_type == Request::NR_TYPE_SWNR) ? eImgFmt_I420 : eImgFmt_YUY2,
                    pInImageBuffer->getImgSize());

        }
		else {
            MINT32 imgFormat = pRequest->context.work_buffer_format;
            MSize imgSize = pInImageBuffer->getImgSize();
            String8 bufferName = String8::format("pool:s%dx%d:f%d",
                    imgSize.w, imgSize.h, imgFormat);

            sp<PoolBufferAllocator> poolAllocator;
            if (mpPoolAllocatorMap->indexOfKey(bufferName) >= 0) {
                poolAllocator = mpPoolAllocatorMap->valueFor(bufferName);
            }
            else {
                poolAllocator = PoolBufferAllocator::create(imgFormat, imgSize);
                mpPoolAllocatorMap->add(bufferName,poolAllocator);
            }
            pBufferHandle = PoolBufferHandle::create(bufferName.string(), poolAllocator);
        }


        if (pBufferHandle.get()) {
            // occupied WDMA0 and WROTO, except IMGO2
            occupied |= 0x3;

            pRequest->context.work_buffer = pBufferHandle;
            pRequest->context.in_mdp_crop = MTRUE;
            pCopyCandidate = pBufferHandle;
            P2Procedure::FrameOutput out;
            out.mPortId = PORT_WDMAO;
            out.mHandle = pBufferHandle;
            out.mTransform = 0;
            param_p2.vOut.push_back(out);
        }
    }
#endif


    vector<sp<BufferHandle>>::iterator iter = pRequest->context.out_buffers.begin();
    for (; iter != pRequest->context.out_buffers.end(); iter++) {
        sp<BufferHandle> pOutBuffer = *iter;
        if (!pOutBuffer.get())
            continue;

        StreamId_T const streamId = pOutBuffer->getStreamId();
        MUINT32 const transform = pOutBuffer->getTransform();
        MUINT32 const usage = pOutBuffer->getUsage();

        PortID port_p2;
        MBOOL isFdStream = streamId == pRequest->context.fd_stream_id;
        if (OK == mapPortId(streamId, transform, isFdStream, occupied, port_p2)) {
            P2Procedure::FrameOutput out;
            {
                out.mPortId = port_p2;
            }
            if (pCopyCandidate == NULL && transform == 0) {
                pCopyCandidate = pOutBuffer;
            }
            out.mHandle = pOutBuffer;
            out.mTransform = transform;
            out.mUsage = usage;

            param_p2.vOut.push_back(out);
            (*iter).clear();
        }
        else
            remains = MTRUE;
    }

    if (param_p2.vOut.size() == 0) {
        MY_LOGW("param_p2 vOut size 0");
        return UNKNOWN_ERROR;
    }
    else if (pRequest->isReentry()) {
        // skip to determine mdp's input buffer if need to re-entry
    }
    else if (remains) {
        if (pCopyCandidate == NULL) {
            pCopyCandidate = param_p2.vOut[param_p2.vOut.size() - 1].mHandle;
            MY_LOGW("no candidate buffer for copying");
        }
        pRequest->context.in_mdp_buffer = pCopyCandidate;
    }

    param_p2.inApp = pRequest->context.in_app_meta;
    param_p2.inHal = pRequest->context.in_hal_meta;
    param_p2.outApp = pRequest->context.out_app_meta;
    param_p2.outHal = pRequest->context.out_hal_meta;

    // pull meta buffers if not re-entry
    if (!pRequest->isReentry()) {
        // let FrameLifeHolder release the buffer
        //pRequest->context.in_app_meta.clear();
        //pRequest->context.in_hal_meta.clear();
        pRequest->context.out_app_meta.clear();
        pRequest->context.out_hal_meta.clear();
    }

    return OK;
}

