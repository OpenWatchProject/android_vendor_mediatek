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

#define LOG_TAG "MtkCam/P2Node_P2"

#include "../P2Common.h"
#include "P2Procedure.h"

#include <fstream>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/utils/hw/HwTransform.h>
#include <mtkcam/feature/eis/EisInfo.h>
#include <camera_custom_eis.h>
// clear zoom, dre
#include "DpDataType.h"
// query mdp capbility
#include "DpIspStream.h"
#include <mtkcam/utils/exif/DebugExifUtils.h>
#include <debug_exif/cam/dbg_cam_param.h>
//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
// isp5.0 srz setting
#include <mtkcam/drv/def/Dip_Notify_datatype.h>

using namespace NSCamHW;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;
using namespace NS3Av3;
using namespace NSCam;
using namespace plugin;
using namespace NSCam::EIS;
using namespace NSCam::TuningUtils;

#define DEBUG_PORT_IN_IMGO          (0x1)
#define DEBUG_PORT_IN_RRZO          (0x2)
#define DEBUG_PORT_IN_LCSO          (0x4)
#define DEBUG_PORT_OUT_WDMAO        (0x1)
#define DEBUG_PORT_OUT_WROTO        (0x2)
#define DEBUG_PORT_OUT_IMG2O        (0x4)
#define DEBUG_PORT_OUT_IMG3O        (0x8)
#define DEBUG_PORT_OUT_MFBO        (0x10)
#define DEBUG_DRAWLINE_PORT_WDMAO   (0x1)
#define DEBUG_DRAWLINE_PORT_WROTO   (0x2)
#define DEBUG_DRAWLINE_PORT_IMG2O   (0x4)

#define CHECK_ERROR(_err_)                                    \
        do {                                                  \
            MERROR const err = (_err_);                       \
            if( err != OK ) {                                 \
                MY_LOGE("err:%d(%s)", err, ::strerror(-err)); \
                return err;                                   \
            }                                                 \
        } while(0)

#define CHECK_NULL(_parm_)                                    \
        do {                                                  \
            if( _parm_ == NULL ) {                                \
                MY_LOGE("Null %s Pointer", _parm_); \
                return BAD_VALUE;                                 \
            }                                                 \
        } while(0)

P2Procedure::
P2Procedure(CreateParams const &params)
        : mbEnableLog(params.bEnableLog),
          muOpenId(params.uOpenId),
          mCreateParams(params),
          mpPipe(params.pPipe),
          mnStreamTag(-1),
          mpIsp(params.pIsp),
          mConfigVencStream(MFALSE),
          muRequestCnt(0),
          muEnqueCnt(0),
          muDequeCnt(0),
          mDebugScanLineMask(0),
          mpDebugScanLine(NULL),
          mpTuning(NULL)
{
    mpMultiFrameHandler = new MultiFrameHandler(
            params.pPipe, params.bEnableLog);

    muDumpBuffer    = ::property_get_int32("vendor.debug.camera.dump.p2", 0);
    muDumpCondIn    = ::property_get_int32("vendor.debug.camera.dump.p2.cond.in", 0xFF);
    muDumpPortIn    = ::property_get_int32("vendor.debug.camera.dump.p2.in", 0xFF);
    muDumpPortOut   = ::property_get_int32("vendor.debug.camera.dump.p2.out", 0xFF);
    muDumpPortImg3o = ::property_get_int32("vendor.debug.camera.dump.p2.ext.img3o", 0);
    muDumpPortMfbo  = ::property_get_int32("vendor.debug.camera.dump.p2.ext.mfbo", 0);
    muSensorFormatOrder = SENSOR_FORMAT_ORDER_NONE;

    if (muDumpBuffer) {
        IHalSensorList *const pIHalSensorList = MAKE_HalSensorList();
        if (pIHalSensorList) {
            MUINT32 sensorDev = (MUINT32) pIHalSensorList->querySensorDevIdx(muOpenId);

            NSCam::SensorStaticInfo sensorStaticInfo;
            memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
            pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);
            muSensorFormatOrder = sensorStaticInfo.sensorFormatOrder;
        }
    }

    mDebugScanLineMask = ::property_get_int32("debug.camera.scanline.p2", 0);
    if (mDebugScanLineMask != 0) {
        mpDebugScanLine = DebugScanLine::createInstance();
    }

    mnStreamTag = (params.type == P2Node::PASS2_TIMESHARING)
                  ? ENormalStreamTag_Vss
                  : ENormalStreamTag_Normal;

    unsigned int tuningsize = (mpPipe != NULL) ? mpPipe->getRegTableSize() : 0;;
    mpTuning = ::malloc(tuningsize);
    if (mpTuning == NULL) {
        MY_LOGE("alloc tuning buffer fail, tuningsize:%d", tuningsize);
    }

}


/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
P2Procedure::
createProcessor(CreateParams &params) {
    INormalStream *pPipe = NULL;
    IHalISP_T *pIsp = NULL;
    if (params.type != P2Node::PASS2_STREAM &&
        params.type != P2Node::PASS2_TIMESHARING) {
        MY_LOGE("not supported type %d", params.type);
        goto lbExit;
    }

    CAM_TRACE_BEGIN("P2:NormalStream:create");
    pPipe = INormalStream::createInstance(params.uOpenId);
    CAM_TRACE_END();
    if (pPipe == NULL) {
        MY_LOGE("create pipe failed");
        goto lbExit;
    }

    CAM_TRACE_BEGIN("P2:NormalStream:init");
    if (!pPipe->init(LOG_TAG)) {
        CAM_TRACE_END();
        MY_LOGE("pipe init failed");
        goto lbExit;
    }
    CAM_TRACE_END();

#if SUPPORT_ISP
    CAM_TRACE_BEGIN("P2:ISP:create");
    pIsp = MAKE_HalISP(params.uOpenId, LOG_TAG);
    CAM_TRACE_END();
#endif
    if (pIsp == NULL) {
        MY_LOGE("create 3A failed");
        goto lbExit;
    }
    MY_LOGD("create processor type %d: pipe %p, 3A %p",
            params.type, pPipe, pIsp);

    lbExit:
    if (!pPipe || !pIsp) {
        if (pPipe) {
            pPipe->uninit(LOG_TAG);
            pPipe->destroyInstance();
            pPipe = NULL;
        }
        if (pIsp) {
            pIsp->destroyInstance(LOG_TAG);
            pIsp = NULL;
        }
    }

    params.pPipe = pPipe;
    params.pIsp = pIsp;
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
        if (!(rOccupied & PORT_WROTO_USED)) {
            rPortId = PORT_WROTO;
            rOccupied |= PORT_WROTO_USED;
        }
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
    MY_LOGDO_IF(mbEnableLog, "stream id %#" PRIx64 ", occupied %u",
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
        if (mConfigVencStream) {
            if (mpPipe->sendCommand(ESDCmd_RELEASE_VENC_DIRLK)) {
                MY_LOGEO("release venc stream failed");
            }
        }
        CAM_TRACE_BEGIN("P2:NormalStream:uninit");
        if (!mpPipe->uninit(LOG_TAG)) {
            MY_LOGEO("pipe uninit failed");
        }
        mpPipe->destroyInstance();
        CAM_TRACE_END();
    }

    if (mpIsp) {
        CAM_TRACE_BEGIN("P2:ISP:destroy");
        mpIsp->destroyInstance(LOG_TAG);
        CAM_TRACE_END();
    }

    if (mpMultiFrameHandler) {
        delete mpMultiFrameHandler;
    }

    if (mpDebugScanLine != NULL) {
        mpDebugScanLine->destroyInstance();
        mpDebugScanLine = NULL;
    }

    if(mpTuning){
        free(mpTuning);
        mpTuning = NULL;
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


    map<EDIPInfoEnum, MUINT32> mDipInfo;
    MBOOL r = NSCam::NSIoPipe::NSPostProc::INormalStream::queryDIPInfo(mDipInfo);
    if(!r) MY_LOGE("queryDIPInfo fail!");
    map<DP_ISP_FEATURE_ENUM, bool>  mMdpFeatureSupport;
    DpIspStream::queryISPFeatureSupport(mMdpFeatureSupport);

    MERROR ret = OK;
    if (OK != (ret = checkParams(params)))
        return ret;
    // prepare metadata
    IMetadata *pMeta_InApp = params.inApp->getMetadata();
    IMetadata *pMeta_InHal = params.inHal->getMetadata();
    IMetadata *pMeta_OutApp = params.outApp.get() ? params.outApp->getMetadata() : NULL;
    IMetadata *pMeta_OutHal = params.outHal.get() ? params.outHal->getMetadata() : NULL;
    //
    MINT32 magicNo   = 0;
    if(pMeta_InHal != NULL)
    {
        tryGetMetadata<MINT32>(pMeta_InHal, MTK_P1NODE_PROCESSOR_MAGICNUM, magicNo);
    }
    //
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
    QParams qEnqueParams;
    NSCam::NSIoPipe::FrameParams    frameParams;
    //frame tag
    frameParams.mStreamTag = mnStreamTag;

    // input
    {
        CAM_TRACE_BEGIN("P2:waitState:IN");
        if (OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE))) {
            MY_LOGWO("src buffer err = %d", ret);
            CAM_TRACE_END();
            return BAD_VALUE;
        }
        CAM_TRACE_END();
        IImageBuffer *pSrc = params.in.mHandle->getBuffer();

        Input src;
        src.mPortID = params.in.mPortId;
        src.mPortID.group = 0;
        src.mBuffer = pSrc;

        FileReadRule rule; //on-line tuning read raw from sdcard
        if(!params.bResized){
            rule.getFile_RAW(params.uRequestNo, "single_capture", pSrc, "P2Node", muOpenId);
        }
        // update src size
#if SUPPORT_PLUGIN
        if (params.bRunWorkBuffer) {
            // do nothing if run on working buffer
        }
        else
#endif
        if (params.bResized)
            pSrc->setExtParam(pCropInfo->dstsize_resizer);

        frameParams.mvIn.push_back(src);
        strEnqueLog += String8::format("Resized(%d) F/R/M:%d/%d/%d 2Run/Reentry/P2Count:%d/%d/%d, EnQ: Src Port(%d) Fmt(0x%x) Size(%dx%d) Id(%#" PRIx64 ") Heap(%p) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ")=> ",
                   params.bResized,
                   params.uFrameNo,
                   params.uRequestNo,
                   magicNo,
                   pRequest->context.is2run, pRequest->isReentry(),
                   params.uPass2Count,
                   src.mPortID.index, src.mBuffer->getImgFormat(),
                   src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h,
                   params.in.mHandle->getStreamId(),
                   pSrc->getImageBufferHeap(),
                   pSrc->getBufVA(0),
                   pSrc->getBufPA(0));
    }

    // input LCEI
    if(params.in_lcso.mHandle != NULL){
        CAM_TRACE_BEGIN("P2:waitState:LCSO");
        if( OK != (ret = params.in_lcso.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW(" (%d) Lcso handle not null but waitState failed! ", params.uFrameNo);
            CAM_TRACE_END();
            return BAD_VALUE;
        }
        CAM_TRACE_END();
        //

    }
    MRect preview_crop_a;
    // output
    for (size_t i = 0; i < params.vOut.size(); i++) {
        CAM_TRACE_FMT_BEGIN("P2:waitState:OUT %zu", i);
        if (params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE))) {
            MY_LOGWO("dst buffer err = %d", ret);
            CAM_TRACE_END();
            continue;
        }
        CAM_TRACE_END();
        IImageBuffer *pDst = params.vOut[i].mHandle->getBuffer();
        MBOOL isCapStream = (params.capStreamId==params.vOut[i].mHandle->getStreamId()) ? true : false;
        MY_LOGDO_IF(P2_DEBUG_LOG, "isCapStream: %d", isCapStream);
        Output dst;
        dst.mPortID = params.vOut[i].mPortId;
        dst.mPortID.group = 0;
        MUINT32 const uUsage = params.vOut[i].mUsage;
        dst.mPortID.capbility = (NSIoPipe::EPortCapbility)(
                (uUsage & GRALLOC_USAGE_HW_COMPOSER) ? EPortCapbility_Disp :
                (uUsage & GRALLOC_USAGE_HW_VIDEO_ENCODER) ? EPortCapbility_Rcrd :
                (isCapStream) ? EPortCapbility_Cap :
                EPortCapbility_None);
        dst.mBuffer = pDst;
        dst.mTransform = params.vOut[i].mTransform;
        frameParams.mvOut.push_back(dst);
        if(dst.mPortID.capbility == EPortCapbility_Disp)
        {
            sp<Cropper::CropInfo> _pCropInfo = new Cropper::CropInfo;
            if (OK != (ret = getCropInfo(pMeta_InApp, pMeta_InHal, params.bResized, *_pCropInfo, false))) {
                MY_LOGEO("getCropInfo failed");
                return ret;
            }

            MCropRect cropRect;
            Cropper::calcViewAngle(mbEnableLog, *_pCropInfo, dst.mBuffer->getImgSize(), cropRect);
            MRect preview_crop_t;
            preview_crop_t.p  = cropRect.p_integral;
            preview_crop_t.s = cropRect.s;
            if(pRequest->context.resized)
            {
                preview_crop_t.p =  inv_transform(_pCropInfo->tranSensor2Resized, cropRect.p_integral);
                preview_crop_t.s = inv_transform(_pCropInfo->tranSensor2Resized, cropRect.s);
            }
            {
                _pCropInfo->matSensor2Active.transform(preview_crop_t.p, preview_crop_a.p);
                _pCropInfo->matSensor2Active.transform(preview_crop_t.s, preview_crop_a.s);
            }
            MY_LOGD_IF(mbEnableLog, "Preview Crop(%d,%d)(%dx%d) --> rrz Crop(%d,%d)(%dx%d) Size(%dx%d) -> AA Crop(%d,%d)(%dx%d)",
                        cropRect.p_integral.x,
                        cropRect.p_integral.y,
                        cropRect.s.w,
                        cropRect.s.h,
                        dst.mBuffer->getImgSize().w,
                        dst.mBuffer->getImgSize().h,
                        preview_crop_t.p.x,
                        preview_crop_t.p.y,
                        preview_crop_t.s.w,
                        preview_crop_t.s.h,
                        preview_crop_a.p.x,
                        preview_crop_a.p.y,
                        preview_crop_a.s.w,
                        preview_crop_a.s.h
                        );

        }
    }
    if (frameParams.mvOut.size() == 0) {
        //MY_LOGWO("no dst buffer");
        return BAD_VALUE;
    }

    {
        CAM_TRACE_BEGIN("P2:SetTuning");
        TuningParam rTuningParam;
        void *pTuning = NULL;
        unsigned int tuningsize = (mpPipe != NULL) ? mpPipe->getRegTableSize() : 0;//sizeof(dip_x_reg_t);
        if (tuningsize == 0) {
            MY_LOGW("getRegTableSize is 0 (%p)", mpPipe);
        }

        pTuning = ::malloc(tuningsize);
        if (pTuning == NULL) {
            MY_LOGEO("alloc tuning buffer fail");
            return NO_MEMORY;
        }
        ::memset((unsigned char *) (pTuning), 0, tuningsize);

        /*For SMVR performance optimize*/
        MBOOL needIsp = MFALSE;
        MY_LOGDO_IF(mbEnableLog, "P2 setIsp burst(%d)bEnq(%d)", pRequest->context.burst_num, params.bBurstEnque);
        if(!(pRequest->context.burst_num > 1) ||
            (pRequest->context.burst_num > 1 && muRequestCnt % pRequest->context.burst_num == 1)){
            needIsp = MTRUE;
        }
        if(!needIsp && mpTuning != NULL){
            ::memcpy(pTuning, mpTuning, tuningsize);
            MY_LOGDO_IF(mbEnableLog, "reuse tuning buff");
        }

        rTuningParam.pRegBuf = pTuning;

        if(params.in_lcso.mHandle != NULL) {
            rTuningParam.pLcsBuf = params.in_lcso.mHandle->getBuffer();
            FileReadRule rule; //on-line tuning read LCSO from sdcard
            rule.getFile_LCSO(params.uRequestNo, "single_capture", params.in_lcso.mHandle->getBuffer(), "P2Node", muOpenId);
        }
        MY_LOGDO_IF(mbEnableLog, "pass2 setIsp malloc %p : %d, LCSO exist(%d)", pTuning, tuningsize, (rTuningParam.pLcsBuf != NULL));

        MetaSet_T inMetaSet;
        MetaSet_T outMetaSet;

        inMetaSet.appMeta = *pMeta_InApp;
        inMetaSet.halMeta = *pMeta_InHal;

        MBOOL const bGetResult = (pMeta_OutApp || pMeta_OutHal);

        MUINT8 profile = 0;
        if(tryGetMetadata<MUINT8>(&(inMetaSet.halMeta), MTK_VHDR_IMGO_3A_ISP_PROFILE, profile))
        {
            //for IMGO ISP profile rewrite
            if( !params.bResized )
            {
                MY_LOGD("IMGO, rewrite ISP profile to MTK_VHDR_IMGO_3A_ISP_PROFILE(%d)",profile);
                trySetMetadata<MUINT8>(&(inMetaSet.halMeta), MTK_3A_ISP_PROFILE, profile);
            }
        }

        if(!tryGetMetadata<MRect>(&(inMetaSet.halMeta), MTK_3A_PRV_CROP_REGION, preview_crop_a))
            trySetMetadata<MRect>(&(inMetaSet.halMeta), MTK_3A_PRV_CROP_REGION, preview_crop_a);
        if (params.bResized) {
            trySetMetadata<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 0);
        } else {
            trySetMetadata<MUINT8>(&(inMetaSet.halMeta), MTK_3A_PGN_ENABLE, 1);
        }
        if (pMeta_OutHal) {
            // FIX ME: getDebugInfo() @ setIsp() should be modified
            outMetaSet.halMeta = *pMeta_InHal;
        }
        CAM_TRACE_END();
        MBOOL isY2Y = pRequest->context.is2run && !pRequest->isReentry();
        if (mpIsp) {
            trySetMetadata<MINT32>(&inMetaSet.halMeta, MTK_PIPELINE_FRAME_NUMBER, params.uFrameNo);
            trySetMetadata<MINT32>(&inMetaSet.halMeta, MTK_PIPELINE_REQUEST_NUMBER, params.uRequestNo);

            if (params.bYuvReproc) {
                trySetMetadata<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, EIspProfile_YUV_Reprocess);
            }
            //
            if(isY2Y) {
                trySetMetadata<MUINT8>(&(inMetaSet.halMeta), MTK_3A_ISP_PROFILE, EIspProfile_Capture_MultiPass_HWNR );
                IImageBuffer *pSrc = params.in.mHandle->getBuffer();
                MINT32 resolution = pSrc->getImgSize().w | ( pSrc->getImgSize().h << 16 );
                trySetMetadata<MINT32>(&(inMetaSet.halMeta), MTK_ISP_P2_IN_IMG_RES_REVISED, resolution);
                trySetMetadata<MINT32>(&(inMetaSet.halMeta), MTK_ISP_P2_IN_IMG_FMT, 1); // 0 or not exist: RAW->YUV, 1: YUV->YUV
                MY_LOGDO_IF(1, "pass2 run2 P2 revised res=0x%x", resolution);
                rTuningParam.pLcsBuf = nullptr;
            }
            MY_LOGDO_IF(mbEnableLog, "P2 setIsp %p : %d", pTuning, tuningsize);

            CAM_TRACE_BEGIN("P2:SetIsp");

            if (needIsp &&
                0 > mpIsp->setP2Isp(0, inMetaSet, &rTuningParam,
                                 (bGetResult ? &outMetaSet : NULL))) {
                MY_LOGWO("P2 setIsp - skip tuning pushing");
                if (pTuning != NULL) {
                    MY_LOGDO_IF(mbEnableLog, "P2 setIsp free %p : %d", pTuning, tuningsize);
                    ::free(pTuning);
                }
            } else {
                if(needIsp){
                    Mutex::Autolock _l(mMetaLock);
                    mMetaSetISP = outMetaSet;
                    ::memcpy(mpTuning, pTuning, tuningsize);
                    MY_LOGD_IF(mbEnableLog,"needIsp: %d, save outMetaSet.", needIsp);
                }
                else{
                    //mpTuning already copied before.
                    Mutex::Autolock _l(mMetaLock);
                    outMetaSet = mMetaSetISP;
                    MY_LOGD_IF(mbEnableLog,"needIsp: %d, reuse mMetaSetISP.", needIsp);
                }

                // check HLR open or not
                if(rTuningParam.pLcsBuf) {
                    IImageBuffer* pSrc = params.in_lcso.mHandle->getBuffer();
                    //
                    Input src;
                    src.mPortID       = params.in_lcso.mPortId; // LCEI
                    src.mPortID.group = 0;
                    src.mBuffer       = pSrc;

                    //
                    if(!isY2Y) {
                        frameParams.mvIn.push_back(src);
                        MY_LOGD_IF(mbEnableLog, "EnQ Src lcso mPortID.index(%d) Fmt(0x%x) "
                            "Size(%dx%d) (%p)",
                            src.mPortID.index, src.mBuffer->getImgFormat(),
                            src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h, pSrc->getImageBufferHeap());
                    }
                    if( !isY2Y &&  mDipInfo[EDIPINFO_DIPVERSION] == EDIPHWVersion_50 )
                    {
                        src.mPortID  = PORT_DEPI;
                        frameParams.mvIn.push_back(src);
                        MY_LOGD_IF(mbEnableLog, "depi mPortID.index(%d) Fmt(0x%x) "
                                "Size(%dx%d) (%p)",
                                src.mPortID.index, src.mBuffer->getImgFormat(),
                                src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h, pSrc->getImageBufferHeap());

                        //srz4 config
                        ModuleInfo srz4_module;
                        srz4_module.moduleTag = EDipModule_SRZ4;
                        srz4_module.frameGroup=0;
                        _SRZ_SIZE_INFO_    *srzParam = new _SRZ_SIZE_INFO_;
                        srzParam->in_w = src.mBuffer->getImgSize().w;
                        srzParam->in_h = src.mBuffer->getImgSize().h;
                        srzParam->crop_floatX = 0;
                        srzParam->crop_floatY = 0;
                        srzParam->crop_x = 0;
                        srzParam->crop_y = 0;
                        srzParam->crop_w = src.mBuffer->getImgSize().w;
                        srzParam->crop_h = src.mBuffer->getImgSize().h;
                        srzParam->out_w = params.in.mHandle->getBuffer()->getImgSize().w;//p2 input raw width
                        srzParam->out_h = params.in.mHandle->getBuffer()->getImgSize().h;//p2 input raw height
                        srz4_module.moduleStruct   = reinterpret_cast<MVOID*> (srzParam);
                        frameParams.mvModuleData.push_back(srz4_module);
                    }
                }
                mpIsp->dumpIsp(0, inMetaSet, &rTuningParam, &outMetaSet);
                frameParams.mTuningData = pTuning;
#if SUPPORT_IMG3O_PORT
                // dump tuning data
                if (((!params.bResized && muDumpCondIn & DEBUG_PORT_IN_IMGO) ||
                     (params.bResized && muDumpCondIn & DEBUG_PORT_IN_RRZO)) &&
                    muDumpPortImg3o && muDumpPortOut & DEBUG_PORT_OUT_IMG3O) {
                    char filename[100];
                    if (!NSCam::Utils::makePath(P2_DEBUG_DUMP_PATH, 0660))
                        MY_LOGW("makePath[%s] fails", P2_DEBUG_DUMP_PATH);
                    sprintf(filename, P2_DEBUG_DUMP_PATH "/%09d-%04d-%04d-p2-tuning.data",
                            params.uUniqueKey, params.uFrameNo, params.uRequestNo);
                    std::ofstream out(filename);
                    out.write(reinterpret_cast<char *>(pTuning), tuningsize);
                }
#endif

                IImageBuffer *pSrc = !isY2Y ? static_cast<IImageBuffer *>(rTuningParam.pLsc2Buf) : NULL;

                if ( pSrc != NULL) {
                    Input src;
                    src.mPortID = mDipInfo[EDIPINFO_DIPVERSION]==EDIPHWVersion_50 ? PORT_IMGCI : PORT_DEPI;
                    src.mPortID.group = 0;
                    src.mBuffer = pSrc;

                    frameParams.mvIn.push_back(src);
                    MY_LOGDO_IF(mbEnableLog, "EnQ Src mPortID.index(%d) Fmt(0x%x) "
                            "Size(%dx%d)", src.mPortID.index, src.mBuffer->getImgFormat(),
                               src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h);
                }

                IImageBuffer *pSrc2 = !isY2Y ? static_cast<IImageBuffer *>(rTuningParam.pBpc2Buf) : NULL;
                if (pSrc2 != NULL) {
                    Input src2;
                    src2.mPortID = mDipInfo[EDIPINFO_DIPVERSION]==EDIPHWVersion_50 ? PORT_IMGBI : PORT_DMGI;
                    src2.mPortID.group = 0;
                    src2.mBuffer = pSrc2;

                    frameParams.mvIn.push_back(src2);
                    MY_LOGDO_IF(mbEnableLog, "EnQ Src mPortID.index(%d) Fmt(0x%x) "
                            "Size(%dx%d)", src2.mPortID.index, src2.mBuffer->getImgFormat(),
                               src2.mBuffer->getImgSize().w, src2.mBuffer->getImgSize().h);
                }
            }
            CAM_TRACE_END();
        } else {
            MY_LOGDO_IF(mbEnableLog, "P2 setIsp clear tuning %p : %d", pTuning, tuningsize);
            ::memset((unsigned char *) (pTuning), 0, tuningsize);
        }

        if (pMeta_OutApp) {
            if (params.bYuvReproc) {
                *pMeta_OutApp = *pMeta_InApp;
                *pMeta_OutApp += outMetaSet.appMeta;
                pMeta_OutApp->remove(MTK_SENSOR_TIMESTAMP);
                pMeta_OutApp->remove(MTK_JPEG_THUMBNAIL_SIZE);
                pMeta_OutApp->remove(MTK_JPEG_ORIENTATION);
            }
            else {
                *pMeta_OutApp = outMetaSet.appMeta;
                // workaround (for YUV Reprocessing)
                pMeta_OutApp->remove(MTK_EDGE_MODE);
                pMeta_OutApp->remove(MTK_NOISE_REDUCTION_MODE);
                pMeta_OutApp->remove(MTK_JPEG_THUMBNAIL_SIZE);
                pMeta_OutApp->remove(MTK_JPEG_ORIENTATION);
            }
            MRect cropRegion = pCropInfo->crop_a;
            if (pCropInfo->isEisEabled) {
                cropRegion.p.x += pCropInfo->eis_mv_a.p.x;
                cropRegion.p.y += pCropInfo->eis_mv_a.p.y;
            }
            updateCropRegion(cropRegion, pMeta_OutApp);
        }

        if (params.uPass2Count == 1 && pMeta_OutHal) {
            if (!pMeta_OutHal->count())
                *pMeta_OutHal = *pMeta_InHal;
            pMeta_OutHal->remove(MTK_3A_EXIF_METADATA);
            IMetadata exifMeta1;
            tryGetMetadata<IMetadata>(pMeta_OutHal, MTK_3A_EXIF_METADATA, exifMeta1);
            *pMeta_OutHal += outMetaSet.halMeta;
            trySetMetadata<MINT32>(pMeta_OutHal, MTK_PIPELINE_FRAME_NUMBER, pRequest->getFrameNo());
            trySetMetadata<MINT32>(pMeta_OutHal, MTK_PIPELINE_REQUEST_NUMBER, pRequest->getRequestNo());
            pMeta_OutHal->remove(MTK_ISP_P2_IN_IMG_RES_REVISED);
        }
    }
    // for output group crop
    {
        Vector<Output>::const_iterator iter = frameParams.mvOut.begin();
        while (iter != frameParams.mvOut.end()) {
            MCrpRsInfo crop;

            MUINT32 const uPortIndex = iter->mPortID.index;
            if (uPortIndex == PORT_WDMAO.index) {
                crop.mGroupID = 2;
#if SUPPORT_PLUGIN || SUPPORT_3RD_PARTY
                if (params.bRunWorkBuffer) {
                    crop.mCropRect.p_fractional = {0, 0};
                    crop.mCropRect.p_integral = {0, 0};
                    IImageBuffer *src = params.in.mHandle->getBuffer();
                    crop.mCropRect.s = src!=NULL ? src->getImgSize() : iter->mBuffer->getImgSize();
                }
                else
#endif
                    Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop.mCropRect);
            } else if (uPortIndex == PORT_WROTO.index) {
                crop.mGroupID = 3;
                IImageBuffer *pBuf = iter->mBuffer;
                MINT32 const transform = iter->mTransform;
                MSize dstSize = (transform & eTransform_ROT_90)
                                ? MSize(pBuf->getImgSize().h, pBuf->getImgSize().w)
                                : pBuf->getImgSize();
                Cropper::calcViewAngle(mbEnableLog, *pCropInfo, dstSize, crop.mCropRect);
            } else if (uPortIndex == PORT_VENC_STREAMO.index) {
                crop.mGroupID = 2;
                Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop.mCropRect);
#if SUPPORT_FD_PORT
            } else if (uPortIndex == PORT_IMG2O.index) {
                crop.mGroupID = 1;
                Cropper::calcViewAngle(mbEnableLog, *pCropInfo, iter->mBuffer->getImgSize(), crop.mCropRect);
#endif
#if SUPPORT_IMG3O_PORT
            } else if (uPortIndex == PORT_IMG3O.index) {
                crop.mGroupID = 4;
                crop.mCropRect.p_fractional = {0, 0};
                crop.mCropRect.p_integral = {0, 0};
                crop.mCropRect.s = iter->mBuffer->getImgSize();
#endif
#if SUPPORT_MFBO_PORT
            } else if (uPortIndex == PORT_MFBO.index) {
                //crop.mGroupID = 4;
                crop.mCropRect.p_fractional = {0, 0};
                crop.mCropRect.p_integral = {0, 0};
                crop.mCropRect.s = iter->mBuffer->getImgSize();
#endif

            } else {
                MY_LOGEO("not supported output port %d", iter->mPortID.index);
                return BAD_VALUE;
            }
            CAM_TRACE_FMT_BEGIN("P2:OUT GropId %d", crop.mGroupID);
            crop.mResizeDst = iter->mBuffer->getImgSize();

            if(iter != frameParams.mvOut.begin())
                strEnqueLog += ", ";
            strEnqueLog += String8::format("Dst Grp(%d) Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d) Heap(%p) VA/PA(%#" PRIxPTR "/%#" PRIxPTR ")",
                    crop.mGroupID, iter->mTransform,
                    crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                    crop.mCropRect.s.w, crop.mCropRect.s.h,
                    crop.mResizeDst.w, crop.mResizeDst.h,
                    iter->mBuffer->getImageBufferHeap(),
                    iter->mBuffer->getBufVA(0),
                    iter->mBuffer->getBufPA(0)
                    );

            frameParams.mvCropRsInfo.push_back(crop);
            iter++;
            CAM_TRACE_FMT_END();
        }
    }
    // for isp-mdp param
    {
        DpPqParam *MDP_WDMA_PQParam = nullptr, *MDP_WROT_PQParam = nullptr;
        Vector<Output>::const_iterator iter = frameParams.mvOut.begin();
        while (iter != frameParams.mvOut.end()) {
            ClearZoomParam*  CZConfig = nullptr;
            MUINT32 const _uPortIndex = iter->mPortID.index;
            EPortCapbility _capbility = iter->mPortID.capbility;
            DpPqParam* currPQParam = NULL;
            if (MDP_WDMA_PQParam == nullptr && _uPortIndex == PORT_WDMAO.index) {
                MDP_WDMA_PQParam = new DpPqParam();
                MDP_WDMA_PQParam->scenario = _capbility==EPortCapbility_Cap ? MEDIA_ISP_CAPTURE : MEDIA_ISP_PREVIEW;
                MDP_WDMA_PQParam->enable = false;
                currPQParam = MDP_WDMA_PQParam;
            }
            else if (MDP_WROT_PQParam == nullptr && _uPortIndex == PORT_WROTO.index) {
                MDP_WROT_PQParam = new DpPqParam();
                MDP_WROT_PQParam->scenario = _capbility==EPortCapbility_Cap ? MEDIA_ISP_CAPTURE : MEDIA_ISP_PREVIEW;
                MDP_WROT_PQParam->enable = false;
                currPQParam = MDP_WROT_PQParam;
            }
            else{
                iter++;
                continue;
            }
            DpIspParam& ispParam = currPQParam->u.isp;
            // common info
            if(pMeta_InHal)
            {
                MINT32 czTimeStamp = 0, czFrameNo = 0, czRequestNo = 0, lv_value = 0;
                tryGetMetadata<MINT32>(pMeta_InHal, MTK_PIPELINE_UNIQUE_KEY, czTimeStamp);
                ispParam.timestamp = czTimeStamp;
                tryGetMetadata<MINT32>(pMeta_InHal, MTK_REAL_LV, lv_value);
                ispParam.LV = lv_value;
            }
            ispParam.frameNo    = magicNo;
            ispParam.requestNo  = pRequest->getRequestNo();
            ispParam.iso        = pRequest->context.iso;
            ispParam.lensId     = muOpenId;
            if(!params.bReentry) strncpy(ispParam.userString, "caprun2", sizeof("caprun2") );
            // Clear Zoom
            MBOOL supportClearZoom = (pRequest->context.customOption&NSCam::v3::P2Node::CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT)
                                     && mMdpFeatureSupport[ISP_FEATURE_CLEARZOOM];
            if(supportClearZoom && ((_capbility == EPortCapbility_Disp) || (_capbility == EPortCapbility_Cap)))
            {
                if (currPQParam->scenario == MEDIA_ISP_PREVIEW)
                  currPQParam->enable = (PQ_COLOR_EN | PQ_ULTRARES_EN);
                else if (currPQParam->scenario == MEDIA_ISP_CAPTURE)
                  currPQParam->enable = (PQ_ULTRARES_EN);
                //
                ClearZoomParam& CZConfig = ispParam.clearZoomParam;
                pRequest->context.out_hal_meta = params.outHal; //NOTICE
                CZConfig.captureShot    = CAPTURE_SINGLE;//MDP doesn't refer captureShot when preview
                MUINT32 idx = 0;
                CZConfig.p_customSetting = (void*)getTuningFromNvram(muOpenId, idx, magicNo, NVRAM_TYPE_CZ, mbEnableLog);
                strEnqueLog += String8::format(" CZ Port(%d) lengthId(%d) captureShot(%d) idx(%d)",
                    _uPortIndex,
                    ispParam.lensId,
                    CZConfig.captureShot,
                    idx);
            }

            // DRE: 2run mdp
            MBOOL supportDRE = pRequest->context.captureFrame &&
                               SUPPORT_DRE && mMdpFeatureSupport[ISP_FEATURE_DRE] &&
                               ((pRequest->context.is2run && pRequest->isReentry()) || !pRequest->context.is2run);
            if(supportDRE)
            {
                currPQParam->enable |= (PQ_DRE_EN);
                currPQParam->scenario = MEDIA_ISP_CAPTURE;
                DpDREParam& DREParam = ispParam.dpDREParam;
                DREParam.cmd         = DpDREParam::Cmd::Initialize|DpDREParam::Cmd::Generate;
                DREParam.userId      = ispParam.frameNo;
                DREParam.buffer      = nullptr;
                MUINT32 idx = 0;
                DREParam.p_customSetting = (void*)getTuningFromNvram(muOpenId, idx, magicNo, NVRAM_TYPE_DRE, mbEnableLog);
                DREParam.customIndex     = idx;
                strEnqueLog += String8::format(" DRE(%lld) cmd(0x%x) cusSetting(%p) idx(%d)",
                                DREParam.userId, DREParam.cmd, DREParam.p_customSetting, idx);
            }
            // dbg info: only for last run mdp when dre open
            if(pRequest->context.in_mdp_buffer == nullptr && !pRequest->isReentry())
            {
                ispParam.p_mdpSetting =  new MDPSetting();
                MDPSetting* mdpSet = ispParam.p_mdpSetting;
                mdpSet->size = MDPSETTING_MAX_SIZE;
                mdpSet->buffer = ::malloc(MDPSETTING_MAX_SIZE);
                if(mdpSet->buffer==NULL) {
                    MY_LOGE("alloc mdp dbg buffer fail");
                }
                else
                   ::memset((unsigned char*)(mdpSet->buffer), 0, MDPSETTING_MAX_SIZE);
            }
            if(params.bRunWorkBuffer)
                currPQParam->scenario = MEDIA_ISP_CAPTURE;
            strEnqueLog += String8::format(" PQ TS(%d) enable(0x%x) scenario(%d) MdpSetting(%p)",
                    ispParam.timestamp,
                    currPQParam->enable,
                    currPQParam->scenario,
                    ispParam.p_mdpSetting);
            iter++;
        }
        PQParam* framePQParam = new PQParam();
        framePQParam->WDMAPQParam = static_cast<void*>(MDP_WDMA_PQParam);
        framePQParam->WROTPQParam = static_cast<void*>(MDP_WROT_PQParam);
        ExtraParam _extraParam;
        _extraParam.CmdIdx = EPIPE_MDP_PQPARAM_CMD;
        _extraParam.moduleStruct = static_cast<void*>(framePQParam);
        frameParams.mvExtraParam.push_back(_extraParam);
    }

    MY_LOGDO("%s", strEnqueLog.string());
    // push FrameParams into QParams
    qEnqueParams.mvFrameParams.push_back(frameParams);

    if (pRequest->context.burst_num > 1) {
        if (mpMultiFrameHandler)
            return mpMultiFrameHandler->collect(pRequest, qEnqueParams, params.bBurstEnque);
        else
            MY_LOGWO_IF(mbEnableLog, "no burst handler");
    }
    // callback
    qEnqueParams.mpfnCallback = pass2CbFunc;
    qEnqueParams.mpCookie = this;

#if 0
    // FIXME: need this?
    enqueParams.mvPrivaData.push_back(NULL);

    // for crop
    enqueParams.mvP1SrcCrop.push_back(pCropInfo->crop_p1_sensor);
    enqueParams.mvP1Dst.push_back(pCropInfo->dstsize_resizer);
    enqueParams.mvP1DstCrop.push_back(pCropInfo->crop_dma);
    #endif
    String8 strLog;
    strLog += String8::format("p2 enque count:%d, size[in/out]:%zu/%zu",
               muEnqueCnt, frameParams.mvIn.size(), frameParams.mvOut.size());
    // add request to queue
    {
        Mutex::Autolock _l(mLock);
        mvRunning.push_back(pRequest);
#if P2_DEBUG_DEQUE
        mvParams.push_back(qEnqueParams);
#endif
        muEnqueCnt++;
    }

    {
        MY_LOGDO_IF(mbEnableLog, "p2 enque +");
        CAM_TRACE_FMT_BEGIN("P2:Driver:enque %d", muEnqueCnt-1);
        if (!mpPipe->enque(qEnqueParams)) {
            CAM_TRACE_FMT_END();
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
        CAM_TRACE_FMT_END();
        MY_LOGDO_IF(mbEnableLog, "p2 enque -");
    }
    MY_LOGDO("%s", strLog.string());

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

    if (muDumpBuffer) {
        MY_LOGDO("[YUV] frame(%d) size(%zu)", params.uFrameNo, params.vOut.size());
        sp<IImageBuffer> pImgBuf = NULL;
        char filename[256] = {0};
        FILE_DUMP_NAMING_HINT hint;
        if (params.outHal.get()) {
            extract(&hint, params.outHal->getMetadata());
        } else {
            hint.UniqueKey          = params.uUniqueKey;
            hint.FrameNo            = params.uFrameNo;
            hint.RequestNo          = params.uRequestNo;
        }
#define GENERATE_HINT(pBuf) do {                \
    extract(&hint, pBuf.get());                 \
    extract_by_SensorOpenId(&hint, muOpenId);   \
} while(0)
        // dump condition
        MBOOL cond = (!params.bResized && muDumpCondIn & DEBUG_PORT_IN_IMGO) ||
                     (params.bResized && muDumpCondIn & DEBUG_PORT_IN_RRZO)  ||
                     (muDumpCondIn & DEBUG_PORT_IN_LCSO) ;

        // select output buffer if condition matched
        if (cond) {
            //input
            if ((!params.bResized && muDumpPortIn & DEBUG_PORT_IN_IMGO) ||
                (params.bResized && muDumpPortIn & DEBUG_PORT_IN_RRZO)) {
                pImgBuf = params.in.mHandle->getBuffer();
                MBOOL isRaw = (pImgBuf->getImgFormat() > eImgFmt_RAW_START) && 
                              (pImgBuf->getImgFormat() < eImgFmt_BLOB_START) ;
                GENERATE_HINT(pImgBuf);
                if(isRaw) {
                    genFileName_RAW(filename, sizeof(filename), &hint, params.bResized ? RAW_PORT_RRZO : RAW_PORT_IMGO);
                    MY_LOGDO("[raw][in] filename = %s", filename);
                }
                else {
                    genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_UNDEFINED, "run2in");
                    MY_LOGDO("[yuv][in] filename = %s", filename);
                }
                pImgBuf->saveToFile(filename);
            }
            if ((muDumpPortIn & DEBUG_PORT_IN_LCSO)) {
                MY_LOGD("dump lcso");
                if(params.in_lcso.mHandle.get())
                {
                    pImgBuf = params.in_lcso.mHandle->getBuffer();
                    GENERATE_HINT(pImgBuf);
                    genFileName_LCSO(filename, sizeof(filename), &hint);
                    MY_LOGDO("[raw][in] filename = %s", filename);

                    pImgBuf->saveToFile(filename);
                }
            }
            for (size_t i = 0; i < params.vOut.size(); i++) {
                YUV_PORT port = YUV_PORT_UNDEFINED;
                RAW_PORT raw_port = RAW_PORT_UNDEFINED;
                if (muDumpPortOut & DEBUG_PORT_OUT_WDMAO &&
                    params.vOut[i].mPortId == PORT_WDMAO)
                    port = YUV_PORT_WDMAO;
                else if (muDumpPortOut & DEBUG_PORT_OUT_WROTO &&
                         params.vOut[i].mPortId == PORT_WROTO)
                    port = YUV_PORT_WROTO;
                else if (muDumpPortOut & DEBUG_PORT_OUT_IMG2O &&
                         params.vOut[i].mPortId == PORT_IMG2O)
                    port = YUV_PORT_IMG2O;
#if SUPPORT_IMG3O_PORT
                else if (muDumpPortOut & DEBUG_PORT_OUT_IMG3O &&
                         params.vOut[i].mPortId == PORT_IMG3O)
                    port = YUV_PORT_IMG3O;
#endif
#if SUPPORT_MFBO_PORT
                else if (muDumpPortOut & DEBUG_PORT_OUT_MFBO &&
                         params.vOut[i].mPortId == PORT_MFBO)
                {
                    raw_port = RAW_PORT_MFBO;
                }
#endif

                else
                    continue;

                pImgBuf = params.vOut[i].mHandle->getBuffer();
                GENERATE_HINT(pImgBuf);
                if (raw_port == RAW_PORT_UNDEFINED) {
                    if(params.uPass2Count <=1)
                        genFileName_YUV(filename, sizeof(filename), &hint, port);
                    else {
                        String8 name = String8::format("run%dout",params.uPass2Count);
                        genFileName_YUV(filename, sizeof(filename), &hint, port, name.string());
                    }
                    FileReadRule rule; //on-line tuning read yuv name from config
                    rule.getFile_YUV(params.uRequestNo, "single_capture", pImgBuf.get(), "P2Node", port, muOpenId);
                } else {
                    genFileName_RAW(filename, sizeof(filename), &hint, raw_port);
                }

                pImgBuf->saveToFile(filename);
                MY_LOGDO("[yuv][out] filename = %s", filename);
            }
#undef  IMAGE_FORMAT_TO_STRING
        }
    }

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
    if (mpMultiFrameHandler) {
        mpMultiFrameHandler->flush();
    }
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
    switch (event) {
        case eP2_START_VENC_STREAM:
            if (mpPipe->sendCommand(
                    ESDCmd_CONFIG_VENC_DIRLK,
                    arg1, arg2, arg3))
                mConfigVencStream = MTRUE;
            else
                MY_LOGEO("Start venc stream failed");
            break;
        case eP2_STOP_VENC_STREAM:
            if (mpPipe->sendCommand(
                    ESDCmd_RELEASE_VENC_DIRLK))
                mConfigVencStream = MFALSE;
            else
                MY_LOGEO("Stop venc stream failed");
        break;
    }
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::
pass2CbFunc(QParams &rParams) {
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc +++");
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
    CAM_TRACE_FMT_BEGIN("P2:handleDeque %d", muDequeCnt);
    Mutex::Autolock _l(mLock);
    sp<Request> pRequest = NULL;
    {
        MY_LOGDO("p2 deque count:%d, result:%d", muDequeCnt, rParams.mDequeSuccess);
        pRequest = mvRunning.front();
        mvRunning.erase(mvRunning.begin());
        muDequeCnt++;

        if (rParams.mvFrameParams.size() <= 0) {
            MY_LOGE("p2 dequed but mvFrameParams size <= 0");
            // TODO: error handling
            //
            return;
        }

        if (mDebugScanLineMask != 0 && mpDebugScanLine != NULL) {
            for (size_t i = 0; i < rParams.mvFrameParams[0].mvOut.size(); i++) {
                if ((rParams.mvFrameParams[0].mvOut[i].mPortID.index == PORT_WDMAO.index &&
                     mDebugScanLineMask & DEBUG_DRAWLINE_PORT_WDMAO) ||
                    (rParams.mvFrameParams[0].mvOut[i].mPortID.index == PORT_WROTO.index &&
                     mDebugScanLineMask & DEBUG_DRAWLINE_PORT_WROTO) ||
                    (rParams.mvFrameParams[0].mvOut[i].mPortID.index == PORT_IMG2O.index &&
                     mDebugScanLineMask & DEBUG_DRAWLINE_PORT_IMG2O))
                {
                    mpDebugScanLine->drawScanLine(
                            rParams.mvFrameParams[0].mvOut[i].mBuffer->getImgSize().w,
                            rParams.mvFrameParams[0].mvOut[i].mBuffer->getImgSize().h,
                            (void *) (rParams.mvFrameParams[0].mvOut[i].mBuffer->getBufVA(0)),
                            rParams.mvFrameParams[0].mvOut[i].mBuffer->getBufSizeInBytes(0),
                            rParams.mvFrameParams[0].mvOut[i].mBuffer->getBufStridesInBytes(0));
                }
            }
        }

#if P2_DEBUG_DEQUE
        if (mvParams.size()) {
            QParams checkParam;
            checkParam = mvParams.front();
            mvParams.erase(mvParams.begin());

            // make sure params are correct
#define ERROR_IF_NOT_MATCHED(item, i, expected, actual) do{             \
                if( expected != actual)                                             \
                    MY_LOGEO("%s %d: expected %p != %p", item, i, expected, actual); \
            } while(0)

            for (size_t i = 0; i < checkParam.mvFrameParams[0].mvIn.size(); i++) {
                if (i > rParams.mvIn.size()) {
                    MY_LOGEO("no src in dequed Params");
                    break;
                }

                ERROR_IF_NOT_MATCHED("src pa of in", i,
                                     checkParam.mvFrameParams[0].mvIn[i].mBuffer->getBufPA(0),
                                     rParams.mvIn[i].mBuffer->getBufPA(0)
                );
                ERROR_IF_NOT_MATCHED("src va of in", i,
                                     checkParam.mvFrameParams[0].mvIn[i].mBuffer->getBufVA(0),
                                     rParams.mvIn[i].mBuffer->getBufVA(0)
                );
            }

            for (size_t i = 0; i < checkParam.mvFrameParams[0].mvOut.size(); i++) {
                if (i > rParams.mvOut.size()) {
                    MY_LOGEO("no enough dst in dequed Params, %d", i);
                    break;
                }

                ERROR_IF_NOT_MATCHED("dst pa of out", i,
                                     checkParam.mvFrameParams[0].mvOut[i].mBuffer->getBufPA(0),
                                     rParams.mvOut[i].mBuffer->getBufPA(0)
                );
                ERROR_IF_NOT_MATCHED("dst va of out", i,
                                     checkParam.mvFrameParams[0].mvOut[i].mBuffer->getBufVA(0),
                                     rParams.mvOut[i].mBuffer->getBufVA(0)
                );
            }

#undef ERROR_IF_NOT_MATCHED
        }
        else {
            MY_LOGWO("params size not matched");
        }
#endif
    }
    // updateDbgInfo
    IMetadata* outHal = pRequest->context.out_hal_meta.get() ? pRequest->context.out_hal_meta->getMetadata() : nullptr;
    auto updateDbgInfo = [&](MUINT32 tagKey, MUINT32 tagData, MUINT32 size, void* dbgList)
    {
        IMetadata exifMeta;
        tryGetMetadata<IMetadata>(outHal, MTK_3A_EXIF_METADATA, exifMeta);
        if (size>0 && DebugExifUtils::setDebugExif(
                DebugExifUtils::DebugExifType::DEBUG_EXIF_RESERVE3,
                tagKey,
                tagData,
                size,
                dbgList,
                &exifMeta) == nullptr)
        {
            MY_LOGW("set debug exif to metadata fail: reserve3");
        }
        // update cam dbg info
        std::map<MUINT32, MUINT32> debugInfoList;
        {
            using namespace dbg_cam_common_param_1;
            debugInfoList[CMN_TAG_DOWNSCALE_DENOISE_THRES] = pRequest->context.downscaleThres;
            debugInfoList[CMN_TAG_DOWNSCALE_DENOISE_RATIO] = pRequest->context.downscaleRatio;
            debugInfoList[CMN_TAG_DOWNSCALE_DENOISE_WIDTH] = pRequest->context.downscaleWidth;
            debugInfoList[CMN_TAG_DOWNSCALE_DENOISE_HIGHT] = pRequest->context.downscaleHeight;
            debugInfoList[CMN_TAG_SWNR_THRESHOLD] = pRequest->context.swnr_thres_temp;
        }
        if (DebugExifUtils::setDebugExif(
                DebugExifUtils::DebugExifType::DEBUG_EXIF_CAM,
                static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_KEY),
                static_cast<MUINT32>(MTK_CMN_EXIF_DBGINFO_DATA),
                debugInfoList,
                &exifMeta) == nullptr )
        {
            MY_LOGW("set debug exif to metadata fail: cmn");
        }
        {
            trySetMetadata<IMetadata>(outHal, MTK_3A_EXIF_METADATA, exifMeta);
        }
   };
    for (size_t i = 0; i < rParams.mvFrameParams.size(); i++) {
        NSCam::NSIoPipe::FrameParams& param = rParams.mvFrameParams.editItemAt(i);
        if(param.mTuningData) {
            void *pTuning = param.mTuningData;
#if SUPPORT_IMG3O_PORT
            unsigned int tuningsize = mpPipe->getRegTableSize();
            // dump tuning data
            if (((!pRequest->context.resized && muDumpCondIn & DEBUG_PORT_IN_IMGO) ||
                 (pRequest->context.resized && muDumpCondIn & DEBUG_PORT_IN_RRZO)) &&
                  muDumpPortImg3o && muDumpPortOut & DEBUG_PORT_OUT_IMG3O) {
                     char filename[100];
                     sprintf(filename, P2_DEBUG_DUMP_PATH "/%09d-%04d-%04d-p2-tuning-done.data",
                             pRequest->getUniqueKey(), pRequest->getFrameNo(), pRequest->getRequestNo());
                     std::ofstream out(filename);
                     out.write(reinterpret_cast<char *>(pTuning), tuningsize);
            }
#endif
            if(pTuning)
                free(pTuning);
            //
        }
        // delete src
        for(size_t j = 0; j < param.mvModuleData.size(); j++)
        {
            ModuleInfo m = param.mvModuleData.editItemAt(i);
            if(m.moduleStruct != nullptr)
            {
                _SRZ_SIZE_INFO_* p = static_cast<_SRZ_SIZE_INFO_*>(m.moduleStruct);
                delete p;
             }
        }
        for (size_t j = 0; j < param.mvExtraParam.size(); j++)
        {
            MUINT cmdIdx = param.mvExtraParam[j].CmdIdx;
            switch (cmdIdx) {
                case EPIPE_MDP_PQPARAM_CMD:
                {
                    PQParam* extraParam = static_cast<PQParam*>(param.mvExtraParam[j].moduleStruct);
                    if(extraParam == NULL) break;
                    auto clearCZandUpdateMeta = [&](MDPSetting* p)
                    {
                        if(p != nullptr)
                        {
                            MUINT32* buffer = static_cast<MUINT32*>(p->buffer);
                            MUINT32 size = p->size;
                            if(buffer)
                            {
                                MY_LOGD("Update Mdp debug info: addr %p, size %zu", buffer, size);
                                MY_LOGDO_IF(mbEnableLog, "setDebugExif mdp +");
                                updateDbgInfo(static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY), static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA), size, buffer);
                                free(buffer);
                                MY_LOGDO_IF(mbEnableLog, "setDebugExif mdp -");
                            }
                            delete p;
                        }
                    };

                    auto clearMem = [&clearCZandUpdateMeta](DpPqParam* _dpParam)
                                    {
                                        if(_dpParam == nullptr) return;
                                        // delete clearzoom param
                                        clearCZandUpdateMeta(_dpParam->u.isp.p_mdpSetting);
                                        // delete DpPqParam
                                        delete _dpParam;
                                    };
                    clearMem((DpPqParam*)extraParam->WDMAPQParam);
                    clearMem((DpPqParam*)extraParam->WROTPQParam);
                    delete extraParam;
                }
                default:
                {
                }
                break;
            }
        }// for
    }
    if(!pRequest->isReentry()) {
        updateDbgInfo(static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY), static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA), 0, nullptr);
        pRequest->context.out_app_meta.clear();
    }
    pRequest->responseDone(rParams.mDequeSuccess ? OK : UNKNOWN_ERROR);
    CAM_TRACE_FMT_END();
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
        Cropper::CropInfo &cropInfo,
        MBOOL const refEIS
        ) const
{
    if (!tryGetMetadata<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, cropInfo.sensor_size)) {
        MY_LOGEO("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        return BAD_VALUE;
    }

    MSize const sensor = cropInfo.sensor_size;
    MSize const active = mCreateParams.activeArray.s;

    cropInfo.isResized = isResized;
    // get current p1 buffer crop status
    if (!(tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, cropInfo.crop_p1_sensor) &&
          tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE, cropInfo.dstsize_resizer) &&
          tryGetMetadata<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION, cropInfo.crop_dma)))
    {
        MY_LOGWO_IF(1, "[FIXME] should sync with p1 for rrz setting");

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

    MINT32 sensorMode;
    if (!tryGetMetadata<MINT32>(inHal, MTK_P1NODE_SENSOR_MODE, sensorMode)) {
        MY_LOGEO("cannot get MTK_P1NODE_SENSOR_MODE");
        return BAD_VALUE;
    }
    cropInfo.sensorMode = sensorMode;
    cropInfo.uOpenId    = muOpenId;
    HwTransHelper hwTransHelper(muOpenId);
    HwMatrix matToActive;
    if (!hwTransHelper.getMatrixToActive(sensorMode, cropInfo.matSensor2Active) ||
        !hwTransHelper.getMatrixFromActive(sensorMode, cropInfo.matActive2Sensor)) {
        MY_LOGEO("get matrix fail");
        return UNKNOWN_ERROR;
    }

    cropInfo.tranSensor2Resized = simpleTransform(
            cropInfo.crop_p1_sensor.p,
            cropInfo.crop_p1_sensor.s,
            cropInfo.dstsize_resizer
    );

    MBOOL const isEisOn = refEIS ? isEISOn(inApp) : false;

    MRect cropRegion; //active array domain
    queryCropRegion(inApp, inHal, isEisOn, cropRegion);
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
            MINT64 eisPackedInfo = 0;
            if (!tryGetMetadata<MINT64>(inHal, MTK_EIS_INFO, eisPackedInfo)) {
                MY_LOGD("cannot get MTK_EIS_INFO, current eisPackedInfo = %" PRIi64 " ", eisPackedInfo);
            }
            MUINT32 eis_mode = EisInfo::getMode(eisPackedInfo);
            MUINT32 eis_factor = EisInfo::getFactor(eisPackedInfo);
            MY_LOGDO_IF(P2_DEBUG_LOG, "EIS is ON, eisMode=0x%x, eisFactor=%d", eis_mode, eis_factor);
#if 0
            //eis in sensor domain
            isResizedDomain = MFALSE;
            pMv_s->p.x  = eisInfo.x_int - (sensor.w * (eis_factor-100)/2/eis_factor);
            pMv_s->pf.x = eisInfo.x_float;
            pMv_s->p.y  = eisInfo.y_int - (sensor.h * (eis_factor-100)/2/eis_factor);
            pMv_s->pf.y = eisInfo.y_float;

            cropInfo.eis_mv_r = transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_s);
#else
            MSize const resizer = cropInfo.dstsize_resizer;

#if SUPPORT_EIS_MV
            if (eisInfo.is_from_zzr)
            {
                pMv_r->p.x  = eisInfo.x_mv_int;
                pMv_r->pf.x = 0;
                pMv_r->p.y  = eisInfo.y_mv_int;
                pMv_r->pf.y = 0;
                cropInfo.eis_mv_s = inv_transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_r);
            }
            else
            {
                isResizedDomain = MFALSE;
                pMv_s->p.x  = eisInfo.x_mv_int;
                pMv_s->pf.x = 0;
                pMv_s->p.y  = eisInfo.y_mv_int;
                pMv_s->pf.y = 0;
                cropInfo.eis_mv_r = transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_s);
            }
#else
            //eis in resized domain
            if (EIS_MODE_IS_EIS_12_ENABLED(eis_mode))
            {
                pMv_r->p.x = eisInfo.x_int - (resizer.w * (eis_factor - 100) / 2 / eis_factor);
                pMv_r->pf.x = 0;
                pMv_r->p.y = eisInfo.y_int - (resizer.h * (eis_factor - 100) / 2 / eis_factor);
                pMv_r->pf.y = 0;
                cropInfo.eis_mv_s = inv_transform(cropInfo.tranSensor2Resized, cropInfo.eis_mv_r);
            }
            else
            {
                pMv_r->p.x = 0;
                pMv_r->pf.x = 0.0f;
                pMv_r->p.y = 0;
                pMv_r->pf.y = 0.0f;
            }
#endif
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
            // cropInfo.eis_mv_a = inv_transform(cropInfo.tranActive2Sensor, cropInfo.eis_mv_s);
            cropInfo.matSensor2Active.transform(cropInfo.eis_mv_s.p, cropInfo.eis_mv_a.p);
            // FIXME: float
            //cropInfo.matSensor2Active.transform(cropInfo.eis_mv_s.pf,cropInfo.eis_mv_a.pf);

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
        IMetadata *const inApp,
        IMetadata *const inHal,
        MBOOL const isEisOn,
        MRect &cropRegion) const
{
    if (!tryGetMetadata<MRect>(inApp, MTK_SCALER_CROP_REGION, cropRegion)) {
        cropRegion.p = MPoint(0, 0);
        cropRegion.s = mCreateParams.activeArray.s;
        MY_LOGWO_IF(mbEnableLog, "no MTK_SCALER_CROP_REGION, crop full size %dx%d",
                   cropRegion.s.w, cropRegion.s.h);
    }
    MY_LOGDO_IF(mbEnableLog, "control: cropRegion(%d, %d, %dx%d)",
               cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

#if SUPPORT_EIS
    if (isEisOn) {
        MINT64 eisPackedInfo = 0;
        if (!tryGetMetadata<MINT64>(inHal, MTK_EIS_INFO, eisPackedInfo)) {
            MY_LOGD("cannot get MTK_EIS_INFO, current eisPackedInfo = %" PRIi64 " ", eisPackedInfo);
        }
        MUINT32 eis_factor = EisInfo::getFactor(eisPackedInfo);
        cropRegion.p.x += (cropRegion.s.w * (eis_factor - 100) / 2 / eis_factor);
        cropRegion.p.y += (cropRegion.s.h * (eis_factor - 100) / 2 / eis_factor);
        cropRegion.s = cropRegion.s * 100 / eis_factor;
        MY_LOGDO_IF(mbEnableLog, "EIS: factor %d, cropRegion(%d, %d, %dx%d)",
                   eis_factor, cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
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

        MY_LOGDO_IF(mbEnableLog, "EIS MV:%d, %d, %d",
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

        MY_LOGDO_IF(mbEnableLog, "EIS Region: %d, %d, %d, %d, %dx%d",
                   region.x_int,
                   region.x_float,
                   region.y_int,
                   region.y_float,
                   region.s.w,
                   region.s.h);
        return MTRUE;
    }

    MY_LOGWO("wrong eis region count %d", entry.count());
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
    FUNC_START;
#if SUPPORT_3RD_PARTY
    if (! (pRequest->context.processorMask & MTK_P2_ISP_PROCESSOR) ) {
        MY_LOGE("cannot enter P2Procedure");
        return UNKNOWN_ERROR;
    }
#endif
    param_p2.bBurstEnque = MFALSE;
    if (pRequest->context.burst_num > 1) {
        muRequestCnt++;
        if (pRequest->context.burst_num > 1 && muRequestCnt % pRequest->context.burst_num == 0) {
            param_p2.bBurstEnque = MTRUE;
            MY_LOGD("[burst] counter: %d - meet the condition of enque package", muRequestCnt);
        } else {
            MY_LOGD("[burst] counter: %d", muRequestCnt);
        }
    }

    param_p2.uUniqueKey = pRequest->getUniqueKey();
    param_p2.uRequestNo = pRequest->getRequestNo();
    param_p2.uFrameNo   = pRequest->getFrameNo();
    param_p2.bResized   = pRequest->context.resized;
    param_p2.bYuvReproc = pRequest->context.is_yuv_reproc;
    param_p2.uPass2Count = ++pRequest->context.uPass2Count;
    //
    param_p2.bReentry = false;
    const MBOOL isLastP2run = (pRequest->context.is2run && pRequest->context.work_buffer.get()) || (!pRequest->context.is2run) ;
    if(!isLastP2run) {
        param_p2.bReentry = true;
        pRequest->setReentry(MTRUE);
    }

    // input buffer
    param_p2.in.mPortId = PORT_IMGI;
    param_p2.in_lcso.mPortId = PORT_LCEI;
    param_p2.in_lcso.mHandle = pRequest->context.in_lcso_buffer;
    pRequest->context.in_lcso_buffer.clear();
    {
        MY_LOGDO_IF(mbEnableLog, "in_buffer size: %d %p", pRequest->context.in_buffer.size(), pRequest->context.in_buffer[0].get());
        if (pRequest->context.in_buffer.size())
        {
            if(!pRequest->context.in_buffer[0].get())
                MY_LOGE("input buffer is null");
            param_p2.in.mHandle = pRequest->context.in_buffer[0];
            pRequest->context.in_buffer.clear();
        }
        else {
            param_p2.in.mHandle = pRequest->context.work_buffer;
            pRequest->context.work_buffer.clear();
        }
    }
    param_p2.capStreamId = pRequest->context.capture_stream_id;
    // output buffer
    MUINT8 occupied = 0;
    MBOOL remains = MFALSE;

#if SUPPORT_3RD_PARTY
    if(isLastP2run) {

        MBOOL queryWorkBufFromVendor = ((pRequest->context.processorMask & MTK_P2_YUV_PROCESSOR)
                                  &&
                                  ((pRequest->context.resized && (pRequest->context.processorMask & MTK_P2_PREVIEW_REQUEST))
                                   ||
                                   (!pRequest->context.resized && (pRequest->context.processorMask & MTK_P2_CAPTURE_REQUEST))
                                  ))
                                  ;

        if ( queryWorkBufFromVendor ) {
            // prepare params
            plugin::MetaItem meta;
            //
            meta.setting.appMeta = *pRequest->context.in_app_meta->getMetadata();
            meta.setting.halMeta = *pRequest->context.in_hal_meta->getMetadata();
            IMetadata tempmeta;
            trySetMetadata<MINT32>(&tempmeta, MTK_SENSOR_SENSITIVITY, pRequest->context.iso);
            if(pRequest->context.in_app_ret_meta.get())
            {
                meta.others.push_back(*pRequest->context.in_app_ret_meta->getMetadata());
            } else {
                MY_LOGE("no p1 dynamic result meta.");
            }

            // out buffer
            sp<BufferHandle> pBufferHandle = NULL;
            plugin::BufferItem bufItem;
            bufItem.bufferStatus = BUFFER_IN;
            sp<IVendorManager> pVMgr = pRequest->getVendorMgr();
            if(pVMgr.get()) {
                CAM_TRACE_BEGIN("P2:AcquireVendorWB");
                pVMgr->acquireWorkingBuffer(
                                        pRequest->context.userId,
                                        meta,
                                        bufItem
                                      );
                CAM_TRACE_END();
            }
            if(!bufItem.heap.get())
            {
                pRequest->context.mbSkipNextProcessor = MTRUE;
                MY_LOGW("can't get vendor working buffer");
            }
            else {
                pBufferHandle = WorkingBufferHandle::create(
                                    "VendorWB",
                                    bufItem,
                                    pRequest->getVendorMgr(),
                                    meta,
                                    pRequest->context.userId
                                );

                if( bufItem.bufferStatus & BUFFER_DST ) {
                    pRequest->context.processorMask = pRequest->context.processorMask & ~MTK_P2_MDP_PROCESSOR;
                }

                MY_LOGD_IF(mbEnableLog,"working buffer format 0x%x, wxh: %dx%d, heap: %p, status: 0x%x",
                               bufItem.heap->getImgFormat(), bufItem.heap->getImgSize().w,
                               bufItem.heap->getImgSize().h, bufItem.heap.get(), bufItem.bufferStatus);

                if (pBufferHandle.get()) {
                    pRequest->context.work_buffer = pBufferHandle;
                    // occupied WDMA0 and WROTO, except IMGO2
                    occupied |= 0x3;
                    P2Procedure::FrameOutput out;
                    out.mPortId = PORT_WDMAO;
                    out.mHandle = pBufferHandle;
                    out.mTransform = 0;
                    param_p2.vOut.push_back(out);
                }
            }
        } // if ( queryWorkBufFromVendor )
        else {
            pRequest->context.mbSkipNextProcessor = MTRUE;
        }
        MY_LOGDO_IF(P2_DEBUG_LOG, "request:%d[%02d] Resized(%d), skip next: %d",
              pRequest->getFrameNo(),
              pRequest->getFrameSubNo(),
              pRequest->context.resized,
              pRequest->context.mbSkipNextProcessor);
    } // if(isLastP2run)

#endif

    sp<BufferHandle> pCopyCandidate = pRequest->context.work_buffer; // for candidate
#if SUPPORT_DRE
    // allocWorkBufLocal
    if ( pCopyCandidate==nullptr &&
        (pRequest->context.captureFrame||pRequest->context.is2run) ) {
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
        MSize size = pInImageBuffer->getImgSize();
        MINT32 r = pRequest->context.downscaleRatio;
        if(!isLastP2run) {
            size = MSize(size.w/r, size.h/r);
            pRequest->context.downscaleWidth = size.w;
            pRequest->context.downscaleHeight = size.h;
            pRequest->setReentry(MTRUE);
        } else {
            if(pRequest->context.is2run)
                size = MSize(size.w*r, size.h*r);
        }

        sp<BufferHandle> pBufferHandle = NULL;
        pBufferHandle = WorkingBufferHandle::create(
                    "DRE_Wb",
                    eImgFmt_YUY2 ,
                    size);
        pCopyCandidate = pBufferHandle;
        if (pBufferHandle.get()) {
            // occupied WDMA0 and WROTO, except IMGO2
            occupied |= 0x3;

            pRequest->context.work_buffer = pBufferHandle;
            pRequest->context.in_mdp_crop = MTRUE;
            P2Procedure::FrameOutput out;
            out.mPortId = PORT_WDMAO;
            out.mHandle = pBufferHandle;
            out.mTransform = 0;
            param_p2.vOut.push_back(out);
        }
        MY_LOGDO_IF(mbEnableLog, "dre flow : wbsize(%dx%d) downscale(%d) ratio(%d) handle(%p)",
              size.w,size.h,
              !isLastP2run, r,
              pBufferHandle.get()
        );

   }
#endif

    if(pRequest->context.work_buffer.get())
        param_p2.bRunWorkBuffer = MTRUE;

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
        if(pRequest->context.enable_venc_stream) occupied |= 0x1;
        if (OK == mapPortId(streamId, transform, isFdStream, occupied, port_p2)) {
            P2Procedure::FrameOutput out;
            // assign to port of VENC direct link
            if (pRequest->context.enable_venc_stream &&
                usage & GRALLOC_USAGE_HW_VIDEO_ENCODER)
                out.mPortId = PORT_VENC_STREAMO;
            else
                out.mPortId = port_p2;

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
        if (param_p2.bBurstEnque) {
            mpMultiFrameHandler->enque();
            MY_LOGD("no-buffer frame triggers multi-frame enque");
        }
        MY_LOGE("no output buffer.");
        pRequest->context.out_app_meta->updateState(MetaHandle::STATE_WRITE_FAIL);
        pRequest->context.out_hal_meta->updateState(MetaHandle::STATE_WRITE_FAIL);
        return UNKNOWN_ERROR;
    }
    else if (pRequest->isReentry()) {
        // skip to determine mdp's input buffer if need to re-entry
    }
    else if (remains) {
        if (pCopyCandidate == NULL) {
            MY_LOGW("no candidate buffer for copying");
            pRequest->context.in_mdp_buffer = param_p2.vOut[param_p2.vOut.size() - 1].mHandle;
        } else {
            pRequest->context.in_mdp_buffer = pCopyCandidate;
        }

    }

#if SUPPORT_IMG3O_PORT
    if (muDumpPortImg3o) {
        MY_LOGDO("output img3o");
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

        // format: YUY2
        MUINT32 stridesInBytes[3] = {static_cast<MUINT32>(pInImageBuffer->getImgSize().w << 1), 0, 0};

        sp<BufferHandle> pBufferHandle = WorkingBufferHandle::create(
                "IMG3O_WB",
                eImgFmt_YUY2,
                pInImageBuffer->getImgSize());

        if (pBufferHandle.get()) {
            P2Procedure::FrameOutput out;
            out.mPortId = PORT_IMG3O;
            out.mHandle = pBufferHandle;
            out.mTransform = 0;
            param_p2.vOut.push_back(out);
        }
    }
#endif
#if SUPPORT_MFBO_PORT
    if (muDumpPortMfbo) {
        MY_LOGDO("output mfbo");
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

        sp<BufferHandle> pBufferHandle = WorkingBufferHandle::create(
                "MFBO_WB",
                eImgFmt_BAYER10,
                pInImageBuffer->getImgSize());

        if (pBufferHandle.get()) {
            P2Procedure::FrameOutput out;
            out.mPortId = PORT_MFBO;
            out.mHandle = pBufferHandle;
            out.mTransform = 0;
            param_p2.vOut.push_back(out);
        }
    }
#endif

    param_p2.inApp = pRequest->context.in_app_meta;
    param_p2.inHal = pRequest->context.in_hal_meta;
    param_p2.outApp = pRequest->context.out_app_meta;
    param_p2.outHal = pRequest->context.out_hal_meta;
#if SUPPORT_3RD_PARTY
    if ((pRequest->context.processorMask&MTK_P2_YUV_PROCESSOR)
        &&
        (!pRequest->context.mbSkipNextProcessor) //if decide run yuv procedure, return here(not to release out meta)
       )
        return OK;
#endif
    if(pRequest->context.captureFrame)// DRE
        return OK;
    // pull meta buffers if not re-entry
    if (!pRequest->isReentry()) {
        // let FrameLifeHolder release the buffer
        //pRequest->context.in_app_meta.clear();
        //pRequest->context.in_hal_meta.clear();
        pRequest->context.out_app_meta.clear();
#if SUPPORT_SWNR
        // postpone to release output hal metadata
        if (pRequest->context.nr_type != Request::NR_TYPE_SWNR)
#endif
        pRequest->context.out_hal_meta.clear();
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::MultiFrameHandler::
collect(sp<Request> pRequest, QParams &params, MBOOL bForceEnque) {
    mParamCollecting.mvFrameParams.appendVector(params.mvFrameParams);

    mvReqCollecting.push_back(pRequest);

    if (bForceEnque || mvReqCollecting.size() >= pRequest->context.burst_num) {
        enque();
    }
    return OK;
}
/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Procedure::MultiFrameHandler::
enque() {
    if (!mvReqCollecting.size()) {
        return OK;
    }

    QParams enqueParams = mParamCollecting;
    const auto& frameParams = enqueParams.mvFrameParams;

    if (frameParams.size() <= 0) {
        MY_LOGE("unexpected result of FrameParams, size of it is <= 0");
        return UNKNOWN_ERROR;
    }

    const auto& params = frameParams.itemAt(0);

    // callback
    enqueParams.mpfnCallback = callback;
    enqueParams.mpCookie = this;

    {
        // push the collected requests
        {
            Mutex::Autolock _l(mLock);
            mvRunning.push_back(mvReqCollecting);
        }
        MY_LOGD("[burst] p2 enque + count:%d, size[I/O]:%zu/%zu",
                 muMfEnqueCnt, params.mvIn.size(), params.mvOut.size());
        muMfEnqueCnt++;
        CAM_TRACE_FMT_BEGIN("P2:burst:enque %d", muMfEnqueCnt);
        if (!mpPipe->enque(enqueParams)) {
            CAM_TRACE_FMT_END();
            MY_LOGE("[burst] p2 enque failed");
            // remove the collected requests from queue
            {
                Mutex::Autolock _l(mLock);
                mvRunning.erase(mvRunning.end() -1);
            }
            // response error
            vector<sp<Request>>::iterator iter = mvReqCollecting.begin();
            while (iter != mvReqCollecting.end()) {
                (*iter)->responseDone(UNKNOWN_ERROR);
                iter++;
            }

            MY_LOGE("[burst] p2 deque count:%d, enque failed", muMfDequeCnt);
            muMfDequeCnt++;
            AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam/P2Node:ISP pass2 deque fail");
        }
        CAM_TRACE_FMT_END();
        MY_LOGD("[burst] p2 enque -");
    }
    // clear the collected request
    mParamCollecting.mvFrameParams.clear();
    mvReqCollecting.clear();
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::MultiFrameHandler::
deque(QParams &rParams) {
    CAM_TRACE_FMT_BEGIN("P2:Burst:deque %d",muMfDequeCnt);
    vector<sp<Request>> vpRequest;
    {
        Mutex::Autolock _l(mLock);
        MY_LOGD("[burst] p2 deque count:%d, result:%d", muMfDequeCnt, rParams.mDequeSuccess);
        if (mvRunning.size() == 0) {
            MY_LOGE("[burst] there is no running request");
            AEE_ASSERT("\nCRDISPATCH_KEY:MtkCam/P2Node:ISP pass2 enque/deque unmatched");
        }
        vpRequest = mvRunning.front();
        mvRunning.erase(mvRunning.begin());
        muMfDequeCnt++;
    }
    vector<sp<Request>>::iterator iter = vpRequest.begin();
    // updateDbgInfo
    auto updateDbgInfo = [&](IMetadata* pMeta, MUINT32 tagKey, MUINT32 tagData, MUINT32 size, void* buffer)
    {
        IMetadata exifMeta;
        tryGetMetadata<IMetadata>(pMeta, MTK_3A_EXIF_METADATA, exifMeta);
        if (DebugExifUtils::setDebugExif(
            DebugExifUtils::DebugExifType::DEBUG_EXIF_RESERVE3,
            tagKey,
            tagData,
            size,
            buffer,
            &exifMeta) == nullptr)
        {
            MY_LOGW("set debug exif to metadata fail");
        }
        else
        {
            trySetMetadata<IMetadata>(pMeta, MTK_3A_EXIF_METADATA, exifMeta);
        }
    };

    for (size_t i = 0; i < rParams.mvFrameParams.size(); i++)
    {
        NSCam::NSIoPipe::FrameParams& param = rParams.mvFrameParams.editItemAt(i);
        if (param.mTuningData)
        {
            void* pTuning = param.mTuningData;
            if (pTuning)
            {
                free(pTuning);
            }
        }

        for (size_t j = 0; j < param.mvExtraParam.size(); j++)
        {
            MUINT cmdIdx = param.mvExtraParam[j].CmdIdx;
            switch (cmdIdx) {
                case EPIPE_MDP_PQPARAM_CMD:
                {
                    PQParam* extraParam = static_cast<PQParam*>(param.mvExtraParam[j].moduleStruct);
                    if(extraParam == NULL) break;
                    auto clearCZandUpdateMeta = [&](MDPSetting* p)
                    {
                        if(p != nullptr)
                        {
                            MUINT32* buffer = static_cast<MUINT32*>(p->buffer);
                            MUINT32 size = p->size;
                            if(buffer)
                            {
                                MY_LOGD("Update Mdp debug info: addr %p, size %zu", buffer, size);
                                IMetadata* outHal = (*iter)->context.out_hal_meta.get() ? (*iter)->context.out_hal_meta->getMetadata() : nullptr;
                                IMetadata exifMeta;
                                if(outHal)
                                    updateDbgInfo(outHal, static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY), static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA), size, buffer);
                                delete buffer;
                                free(p);
                            }
                            (*iter)->context.out_hal_meta.clear();
                        }
                    };

                    auto clearMem = [&clearCZandUpdateMeta](DpPqParam* _dpParam)
                    {
                        if(_dpParam == nullptr) return;
                        // delete clearzoom param
                        clearCZandUpdateMeta(_dpParam->u.isp.p_mdpSetting);
                        // delete DpPqParam
                        delete _dpParam;
                    };
                    clearMem((DpPqParam*)extraParam->WDMAPQParam);
                    clearMem((DpPqParam*)extraParam->WROTPQParam);
                    delete extraParam;
                    break;
                }
                default:
                {
                }
                break;
            }
        }
    }

    while (iter != vpRequest.end()) {
        (*iter)->responseDone(rParams.mDequeSuccess ? OK : UNKNOWN_ERROR);
        (*iter).clear();
        iter++;
    }
    CAM_TRACE_FMT_END();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Procedure::MultiFrameHandler::
flush() {
    CAM_TRACE_NAME("P2:Burst:flush");
    FUNC_START;

    mParamCollecting = QParams();
    vector<sp<Request>>::iterator iter = mvReqCollecting.begin();
    while (iter != mvReqCollecting.end()) {
        (*iter)->responseDone(UNKNOWN_ERROR);
        iter = mvReqCollecting.erase(iter);
    }

    FUNC_END;
    return;
}
