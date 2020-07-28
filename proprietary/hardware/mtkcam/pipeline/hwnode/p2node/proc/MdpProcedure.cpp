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

#define PROCESSOR_NAME_MDP  "MDP"
#define DEFINE_OPEN_ID      (muOpenId)

#define LOG_TAG "MtkCam/P2Node_MDP"

#include "MdpProcedure.h"
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>

#if MTK_CAM_NEW_NVRAM_SUPPORT
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#endif
//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
// clear zoom, dre
#include "DpDataType.h"
#include <mtkcam/utils/exif/DebugExifUtils.h>
// fd
#include <mtkcam/utils/hw/IFDContainer.h>
using namespace NSIoPipe;
using namespace NSCam;
using namespace plugin;
using namespace NSCam::TuningUtils;
/******************************************************************************
 *
 ******************************************************************************/
sp<Processor>
MdpProcedure::
createProcessor(CreateParams &params) {
    return new ProcessorBase<MdpProcedure>(
            params.uOpenId, params, PROCESSOR_NAME_MDP);
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
MdpProcedure::
onMdpPullParams(
        sp<Request> pRequest,
        FrameParams &param_mdp)
{
    FUNC_START;
#if SUPPORT_3RD_PARTY
    if (! (pRequest->context.processorMask & MTK_P2_MDP_PROCESSOR) ) {
        MY_LOGD_IF(1, "not excute mdp processor");
        return NOT_ENOUGH_DATA;
    }
#endif
    if (!pRequest->context.in_mdp_buffer.get() && !pRequest->context.crop_info.get()) {
        return NOT_ENOUGH_DATA;
    }
    param_mdp.uUniqueKey = pRequest->getUniqueKey();
    param_mdp.uRequestNo = pRequest->getRequestNo();
    param_mdp.uFrameNo = pRequest->getFrameNo();
    param_mdp.in.mHandle = pRequest->context.in_mdp_buffer;
    param_mdp.pCropInfo = pRequest->context.crop_info;
    param_mdp.capStreamId = pRequest->context.capture_stream_id;
    param_mdp.iso = pRequest->context.iso;
    // input&output buffer
    int maxOutCount =  2;
    MBOOL remain = MFALSE;
    vector<sp<BufferHandle>>::iterator iter = pRequest->context.out_buffers.begin();
    while (iter != pRequest->context.out_buffers.end() ) {
        sp<BufferHandle> pOutBuffer = *iter;
        if (pOutBuffer.get() && pOutBuffer->getState() == BufferHandle::STATE_NOT_USED) {
            if(maxOutCount == 0)
            {
                remain = MTRUE;
                break;
            }
            MdpProcedure::FrameOutput out;
            out.mHandle = pOutBuffer;
            out.mTransform = pOutBuffer->getTransform();
            param_mdp.vOut.push_back(out);
            (*iter).clear();
            maxOutCount--;
        }
        iter++;
    }

    if(!remain) {
        pRequest->context.in_mdp_buffer.clear();
        pRequest->context.work_buffer.clear();
    }
    else {
        pRequest->setReentry(MTRUE);
    }

    FUNC_END;
    return (param_mdp.vOut.size() > 0) ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MdpProcedure::
onMdpExecute(
        sp<Request> const pRequest,
        FrameParams const &params)
{
    CAM_TRACE_NAME("P2:onMdpExecute");
    FUNC_START;
    MERROR ret = OK;

    IImageBuffer *pSrc = NULL;
    MSize srcSize;
    vector<IImageBuffer *> vDst;
    vector<MUINT32> vTransform;
    vector<MRect> vCrop;

    MINT32 magicNo = 0;

    if(pRequest->context.in_hal_meta.get() && pRequest->context.in_hal_meta->getMetadata())
    {
        tryGetMetadata<MINT32>(pRequest->context.in_hal_meta->getMetadata(), MTK_P1NODE_PROCESSOR_MAGICNUM, magicNo);
    }

    // input
    if (params.in.mHandle.get()) {
        if (OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE))) {
            MY_LOGWO("src buffer err = %d", ret);
            return ret;
        }
        pSrc = params.in.mHandle->getBuffer();
        srcSize = pSrc->getImgSize();
    }
    else {
        MY_LOGWO("no src");
        return BAD_VALUE;
    }
    MINT32 isCapReq[2] = {-1};// 2 mdp port
    // output
    String8 str;
    str += String8::format("Resized(%d) F/R/M:%d/%d/%d", pRequest->context.resized,
                            pRequest->getFrameNo(),
                            pRequest->getRequestNo(),
                            magicNo);
    for (size_t i = 0; i < params.vOut.size(); i++) {
        if (params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE))) {
            MY_LOGWO("dst buffer err = %d", ret);
            continue;
        }
        StreamId_T const streamId = params.vOut[i].mHandle->getStreamId();
        isCapReq[i] = streamId==params.capStreamId ? i : -1;
        IImageBuffer *pDst = params.vOut[i].mHandle->getBuffer();

        if (pDst != NULL) {

            MINT32 const transform = params.vOut[i].mTransform;
            MSize dstSize = (transform & eTransform_ROT_90)
                            ? MSize(pDst->getImgSize().h, pDst->getImgSize().w)
                            : pDst->getImgSize();
            MRect crop;
            if (pRequest->context.in_mdp_crop) {
                MCropRect cropRect;
                Cropper::calcViewAngle(mbEnableLog, *params.pCropInfo, dstSize, cropRect);
                crop.p = cropRect.p_integral;
                crop.s = cropRect.s;
                vCrop.push_back(crop);
            } else {
                Cropper::calcBufferCrop(srcSize, dstSize, crop);
                vCrop.push_back(crop);
            }

            vDst.push_back(pDst);
            vTransform.push_back(params.vOut[i].mHandle->getTransform());

            str += String8::format(" Out(%zu/%zu) Id(%#" PRIx64 ") VA/PA :%#" PRIxPTR "/%#" PRIxPTR " Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d) isCapReq(%d)",
                        (i + 1), params.vOut.size(),
                        streamId,
                        pDst->getBufVA(0), pDst->getBufPA(0),
                        transform,
                        crop.p.x, crop.p.y, crop.s.w, crop.s.h,
                        pDst->getImgSize().w, pDst->getImgSize().h,
                        isCapReq[i]);
        }
        else
            MY_LOGWO("mdp req:%d empty buffer", pRequest->getFrameNo());
    }
    MY_LOGD("%s", str.string());
    if (pSrc == NULL || vDst.size() == 0) {
        MY_LOGEO("wrong mdp in/out: src %p, dst count %zu", pSrc, vDst.size());
        return BAD_VALUE;
    }
    MBOOL success = MFALSE;
    MDPSetting* mdpSet = NULL;
    MINT64 p1timestamp = 0;
    vector<FD_DATATYPE*> fdData;
    {
#ifdef USING_MTK_LDVT
        success = MTRUE;
#else
        NSSImager::IImageTransform *pTrans = NSSImager::IImageTransform::createInstance();
        if (!pTrans) {
            MY_LOGEO("!pTrans");
            return UNKNOWN_ERROR;
        }
        //
        for(int i=0;i<vDst.size();i++) {
            str = String8();
            NSSImager::IImageTransform::PQParam config;
            config.frameNo = magicNo;
            config.iso = params.iso;
            config.sensorId = muOpenId;
            if(pRequest->context.in_hal_meta.get() && pRequest->context.in_hal_meta->getMetadata())
            {
                MINT32 TimeStamp = 0;
                MINT32 lv_value  = 0;
                tryGetMetadata<MINT32>(pRequest->context.in_hal_meta->getMetadata(), MTK_PIPELINE_UNIQUE_KEY, TimeStamp);
                tryGetMetadata<MINT32>(pRequest->context.in_hal_meta->getMetadata(), MTK_REAL_LV,             lv_value);
                config.timestamp = TimeStamp;
                config.lv_value  = lv_value;

            }
            config.requestNo = pRequest->getRequestNo();
            config.mode = isCapReq[i]>=0 ? NSSImager::IImageTransform::Mode::Capture_Single : NSSImager::IImageTransform::Mode::Preview;
            config.enable = FALSE;
            // just one mdp port can support clearzoom in the same time
            MBOOL supportCZ = pRequest->context.customOption & NSCam::v3::P2Node::CUSTOM_OPTION_CLEAR_ZOOM_SUPPORT && isCapReq[i]>=0;
            if(supportCZ) {
                config.type = NSSImager::IImageTransform::PQType::ClearZoom;
                config.enable = MTRUE;
                config.portIdx = i; // 0:DST_BUF_0 1:DST_BUF_1
                // set CZConfig
                MUINT32 idx = 0;
                config.cz.p_customSetting = (void*)getTuningFromNvram(muOpenId, idx, magicNo, NVRAM_TYPE_CZ, mbEnableLog);
                str += String8::format("CZ mode:%d nvram:%p idx:%d ",
                                        config.mode, config.cz.p_customSetting, idx);
            }
            // all mdp port can enable dre in the same time
            MBOOL supportDRE = SUPPORT_DRE && pRequest->context.captureFrame
                               && isCapReq[i]>=0; //only capture stream apply dre
            if(supportDRE) {
                config.type |= NSSImager::IImageTransform::PQType::DRE;
                config.enable = MTRUE;
                config.portIdx = i; // 0:DST_BUF_0 1:DST_BUF_1
                //
                config.dre.cmd  = NSSImager::IImageTransform::DREParam::CMD::DRE_Apply;
                MBOOL last = (i==vDst.size()-1);
                config.dre.type = last ?
                        NSSImager::IImageTransform::DREParam::HisType::His_One_Time :
                        NSSImager::IImageTransform::DREParam::HisType::His_Conti
                        ;
                config.dre.userId = config.frameNo;
                config.dre.pBuffer = NULL;
                MUINT32 idx = 0;
                config.dre.p_customSetting = (void*)getTuningFromNvram(muOpenId, idx, magicNo, NVRAM_TYPE_DRE, mbEnableLog);
                config.dre.customIdx = idx;
                str += String8::format("DRE cmd:0x%x type:0x%x userId:%lld buffer:%p nvram:%p idx:%d ",
                                        config.dre.cmd, config.dre.type,
                                        config.dre.userId,
                                        config.dre.pBuffer,
                                        config.dre.p_customSetting,
                                        idx);

            }

            config.p_mdpSetting = NULL;
            if(supportCZ || supportDRE) {
                if(mdpSet == NULL) {
                    mdpSet = new MDPSetting();
                    mdpSet->size = MDPSETTING_MAX_SIZE;
                    mdpSet->buffer = ::malloc(MDPSETTING_MAX_SIZE);
                    if(mdpSet->buffer==NULL) {
                        MY_LOGE("alloc mdp dbg buffer fail");
                    }
                    else {
                       ::memset((unsigned char*)(mdpSet->buffer), 0xFF, MDPSETTING_MAX_SIZE);
                       config.p_mdpSetting = (void*) mdpSet;
                       str += String8::format("MdpSetting:%p ", config.p_mdpSetting);
                    }
                }
                int64_t dumpFd = ::property_get_int64("camera.debug.fd.dump",  0);
                if( !tryGetMetadata<MINT64>(pRequest->context.in_hal_meta->getMetadata(), MTK_P1NODE_FRAME_START_TIMESTAMP, p1timestamp) )
                    MY_LOGD("cant get p1timestamp meta");
                auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
                MINT64 tolerence = dumpFd>100 ? dumpFd : 600000000; // 600ms
                fdData = fdReader->queryLock(p1timestamp-tolerence, p1timestamp);
                for(int i=((int)fdData.size()-1);i>=0;i--){
                    if(fdData[i]!=nullptr) {
                      config.p_faceInfor = (void*)(&fdData[i]->facedata);
                      if(dumpFd==1)
                          fdReader->dumpInfo();
                      break;
                    }
                }
                if(config.p_faceInfor == nullptr) {
                    MY_LOGW("can't query facedata ts:%" PRId64 "", p1timestamp);
                    fdReader->dumpInfo();
                }
            
                str += String8::format("PQtype:0x%x enable:%d portIdx:%d/%d iso:%d sensorId:%d Timestamp:%d",
                        config.type,
                        config.enable,
                        config.portIdx, vDst.size(),
                        config.iso,
                        config.sensorId,
                        config.timestamp
                        );
                MY_LOGDO("%s", str.string());
                pTrans->setPQParameter(config);
            }
        }
        CAM_TRACE_BEGIN("P2:Mdp:execute");
        success =
                pTrans->execute(
                        pSrc,
                        vDst[0],
                        (vDst.size() > 1) ? vDst[1] : 0L,
                        vCrop[0],
                        (vCrop.size() > 1) ? vCrop[1] : 0L,
                        vTransform[0],
                        (vTransform.size() > 1) ? vTransform[1] : 0L,
                        0xFFFFFFFF);
        CAM_TRACE_END();
        // fd info
        {
            auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);
            fdReader->queryUnlock(fdData);
        }
        auto clearCZandUpdateMeta = [&](MDPSetting* p)
        {
            if(p != nullptr)
            {
                MUINT32* buffer = static_cast<MUINT32*>(p->buffer);
                MUINT32 size = p->size;
                if(buffer)
                {
                    MY_LOGD("Update Mdp debug info: addr %p, size %zu %c %c", buffer, size, *buffer, *(buffer+1));
                    if(mbEnableLog>2) {
                    char filename[256] = {0};
                      if(!NSCam::Utils::makePath("sdcard/camera_dump", 0660))
                          MY_LOGI("makePath[%s] fails", "sdcard/camera_dump");
                      sprintf(filename, "sdcard/camera_dump/mdp_dump");
                       NSCam::Utils::saveBufToFile(filename,
                               (unsigned char*)buffer, size);
                    }
                    IMetadata* outHal = pRequest->context.out_hal_meta.get() ? pRequest->context.out_hal_meta->getMetadata() : nullptr;
                    IMetadata exifMeta;
                    if(outHal)
                    {
                        MY_LOGDO_IF(mbEnableLog, "setDebugExif mdp +");
                        tryGetMetadata<IMetadata>(outHal, MTK_3A_EXIF_METADATA, exifMeta);
                        if (DebugExifUtils::setDebugExif(
                            DebugExifUtils::DebugExifType::DEBUG_EXIF_RESERVE3,
                            static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_KEY),
                            static_cast<MUINT32>(MTK_RESVC_EXIF_DBGINFO_DATA),
                            size,
                            buffer,
                            &exifMeta) == nullptr)
                        {
                            MY_LOGW("set debug exif to metadata fail");
                        }
                        else
                        {
                            trySetMetadata<IMetadata>(outHal, MTK_3A_EXIF_METADATA, exifMeta);
                        }
                        MY_LOGDO_IF(mbEnableLog, "setDebugExif mdp -");
                    }
                    free(buffer);
                }
                pRequest->context.out_hal_meta.clear();
                delete p;
                p = NULL;
            }
        };
        clearCZandUpdateMeta(mdpSet);
        pTrans->destroyInstance();
        pTrans = NULL;
#endif
    }
    FUNC_END;
    return success ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MdpProcedure::
onMdpFinish(
        FrameParams const &params,
        MBOOL const success)
{
    CAM_TRACE_NAME("P2:onMdpFinish");
    //params.in.mHandle->updateState(BufferHandle::Buffer_ReadDone);
    if (muDumpBuffer) {
        MY_LOGDO("[yuv] frame(%d) size(%zu)", params.uFrameNo, params.vOut.size());

        sp<IImageBuffer> pImgBuf = NULL;
        if (!NSCam::Utils::makePath(P2_DEBUG_DUMP_PATH, 0660))
            MY_LOGWO("makePath[%s] fails", P2_DEBUG_DUMP_PATH);

        // ouput
        char filename[256] = {0};
        FILE_DUMP_NAMING_HINT hint;
        hint.UniqueKey          = params.uUniqueKey;
        hint.FrameNo            = params.uFrameNo;
        hint.RequestNo          = params.uRequestNo;
#define GENERATE_HINT(pBuf) do {                        \
            extract(&hint, pBuf.get());                 \
            extract_by_SensorOpenId(&hint, muOpenId);   \
} while(0)
        for (size_t i = 0; i < params.vOut.size(); i++) {
            pImgBuf = params.vOut[i].mHandle->getBuffer();
            GENERATE_HINT(pImgBuf);
            genFileName_YUV(filename, sizeof(filename), &hint, YUV_PORT_UNDEFINED, "mdp");

            MY_LOGDO("[yuv][out] filename = %s",
                    filename);

            pImgBuf->saveToFile(filename);
        }
    }

    for (size_t i = 0; i < params.vOut.size(); i++)
        if (params.vOut[i].mHandle.get())
            params.vOut[i].mHandle->updateState(
                    success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
            );
    return OK;
}
