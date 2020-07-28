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
 * MediaTek Inc. (C) 2016. All rights reserved.
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
//
#include "../bokeh_common.h"
#include <mtkcam/feature/stereo/pipe/vsdof_common.h>
#include <mtkcam/feature/stereo/pipe/vsdof_data_define.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>
//
#define PIPE_MODULE_TAG "BokehPipe"
#define PIPE_CLASS_TAG "VendorFilterNode"
//
#include "VendorFilterNode.h"
//
#include <PipeLog.h>
#include <string>
//
#include <chrono>
//
#include <DpBlitStream.h>
#include <vsdof/util/vsdof_util.h>
//
#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <stereo_crop_util.h>
// FD
#include <mtkcam/utils/hw/IFDContainer.h>
#include <cmath>
//

#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
using namespace std;
using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace VSDOF::util;

//************************************************************************
//
//************************************************************************
VendorFilterNode::
VendorFilterNode(
    const char *name,
    Graph_T *graph,
    MINT8 mode  __attribute__((UNUSED)))
    : BokehPipeNode(name, graph)
{
    MY_LOGD("ctor(0x%x)", this);
    mVendorProcess     = ::property_get_int32("vendor.debug.vsdof.3rdflow", 1);
    // Get FD info timestamp tolerence = 600ms (Default)
    mVendorFdTolerence = ::property_get_int64("vendor.debug.vsdof.3rdflow.fd.tolerence",  600000000);
    this->addWaitQueue(&mRequests);
}
//************************************************************************
//
//************************************************************************
VendorFilterNode::
~VendorFilterNode()
{
    MY_LOGD("dctor(%p)", this);
}
//************************************************************************
//
//************************************************************************
MBOOL
VendorFilterNode::
onInit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    BokehPipeNode::onInit();
    //
    mpDpStream = new DpBlitStream();

    switch (StereoSettingProvider::getModuleRotation()) {
    case eRotate_90:  mTargeRotation = eRotate_270; break;
    case eRotate_270: mTargeRotation = eRotate_90;  break;
    case eRotate_180: mTargeRotation = eRotate_180; break;
    case eRotate_0:
    default:
        break;
    }

    if (eRotate_0 != mTargeRotation) {
        MSize targetSize = StereoSizeProvider::getInstance()->getBufferSize(E_DEPTH_MAP).
                                                                rotatedByModule();
        if (mTargeRotation != eRotate_180)
            std::swap(targetSize.w, targetSize.h);
        // Create mpVendorDepthMapBufPool
        if(createBufferPool(mpVendorDepthMapBufPool, targetSize.w, targetSize.h,
                            eImgFmt_Y8, 1, "VENDOR_DEPTHMAP_BUF",
                            ImageBufferPool::USAGE_HW_AND_SW))
        {
            mpVendorDepthMapBufPool->allocate(1);
            mRotatedDepthmap = mpVendorDepthMapBufPool->request()->mImageBuffer.get();
        }
    }

    ret = MTRUE;
    FUNC_END;
    return ret;
}
//************************************************************************
//
//************************************************************************
MBOOL
VendorFilterNode::
onUninit()
{
    FUNC_START;
    cleanUp();
    FUNC_END;
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MVOID
VendorFilterNode::
cleanUp()
{
    FUNC_START;
    mRequests.clear();
    // dump all queue size
    MY_LOGD("mRequests.size(%d)", mRequests.size());
    // delete mdp instance
    if(mpDpStream!= nullptr) {
        delete mpDpStream;
        mpDpStream = nullptr;
    }

    if (mTargeRotation != eRotate_0) {
        if (mpVendorDepthMapBufPool.get())
            ImageBufferPool::destroy(mpVendorDepthMapBufPool);
    }

    FUNC_END;
}
//************************************************************************
//
//************************************************************************
MBOOL
VendorFilterNode::
onThreadStart()
{

    int main1Id, main2Id;
    StereoSettingProvider::getStereoSensorIndex(main1Id, main2Id);

    sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(main1Id);
    if( ! pMetadataProvider.get() ) {
        MY_LOGE("MetadataProvider is NULL");
        return false;
    }

    IMetadata static_meta = pMetadataProvider->getMtkStaticCharacteristics();
    //Mapping to active domain by ratio (preview size)
    if( tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveCropSift) ) {
        CropUtil::cropRectByImageRatio(mActiveCropSift, StereoSettingProvider::imageRatio());
    }
    else
    {
        MY_LOGE("tryGetMetadata mActiveArrayCrop is NULL");
    }

    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
VendorFilterNode::
onThreadStop()
{
    return MTRUE;
}
//************************************************************************
//
//************************************************************************
MBOOL
VendorFilterNode::
onData(
    DataID data,
    EffectRequestPtr &request)
{
    TRACE_FUNC_ENTER();
    VSDOF_PRFLOG("reqID=%d +", request->getRequestNo());
    MBOOL ret = MFALSE;
    //
    if(request->vInputFrameInfo.size() == 0)
    {
        MY_LOGE("vInputFrameInfo.size() is 0");
        return ret;
    }
    //
    if(request->vOutputFrameInfo.size() == 0)
    {
        MY_LOGE("vOutputFrameInfo.size() is 0");
        return ret;
    }
    //
    switch(data)
    {
        case ID_ROOT_ENQUE:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            break;
    }
    //
    VSDOF_PRFLOG("-");
    TRACE_FUNC_EXIT();
    return ret;
}
//************************************************************************
//
//************************************************************************
MBOOL
VendorFilterNode::
onThreadLoop()
{
    EffectRequestPtr request;
    //
    if( !waitAllQueue() )
    {
        // mJobs.abort() called
        return MFALSE;
    }
    //
    if( !mRequests.deque(request) )
    {
        return MFALSE;
    }
    CAM_TRACE_NAME("VendorFilterNode::onThreadLoop");
    //
    VendorInputBuffer inputBuffers;
    VendorOutputBuffer outputBuffers;
    VendorInputParameter inputParameters;
    prepareVendorInOutData(request, &inputBuffers, &outputBuffers, &inputParameters);
    //
    if(mVendorProcess)
    {
        if(!processVendor(inputParameters, inputBuffers, outputBuffers))
        {
            return MFALSE;
        }
        // if preview callback is exist, copy output result to preview callback buffer.
        {
            sp<IImageBuffer> pOutPrvCbImage = nullptr;
            if(getOutputBufferFromRequest(request, BOKEH_ER_BUF_PRVCB, pOutPrvCbImage))
            {
                copy(outputBuffers.pOutBokehImage, pOutPrvCbImage);
            }
        }
    }
    //Dump Buffers
    dumpBuffers(request, inputBuffers, outputBuffers);
    //
    if (request->vOutputFrameInfo.indexOfKey(BOKEH_ER_BUF_PRVCB) >= 0)
    {
        MY_LOGD_IF(miPipeLogEnable, "reqID=%d, exist preview CB, Pass to MDPnode",
                   request->getRequestNo());
        handleData(VENDOR_TO_MDP, request);
    }
    else
    {
        MY_LOGD_IF(miPipeLogEnable, "reqID=%d, NO preview CB, notify pipe",
                   request->getRequestNo());
        handleData(VENDOR_OUT, request);
    }
    return MTRUE;
}

void VendorFilterNode:: dumpBuffers(
            const EffectRequestPtr&   request,
            const VendorInputBuffer&  inBuffers,
            const VendorOutputBuffer& outBuffers)
{    // dump out in/output image
    if (mbDumpImageBuffer && checkReqIndexToDump(request))
    {
        MY_LOGD("Vendor Dump image(%d)+", request->getRequestNo());
        IMetadata* pMeta_InHal= nullptr;
        if (!getInputMetadataFromRequest(request, BOKEH_ER_IN_HAL_META_MAIN1, pMeta_InHal))
        {
            MY_LOGE("Get HAL metadata fail.");
        }
        else
        {
            const size_t PATH_SIZE = 1024;
            char writepath[PATH_SIZE];
            extract(&mDumpHint, pMeta_InHal);
            mDumpHint.RequestNo = request->getRequestNo();
            //
            IImageBuffer* pInClearImage = inBuffers.pInClearImage.get();
            extract(&mDumpHint, pInClearImage);
            genFileName_YUV(writepath, PATH_SIZE, &mDumpHint,
                            TuningUtils::YUV_PORT_UNDEFINED, "VENDOR_IN_MAIN1");
            pInClearImage->saveToFile(writepath);

            IImageBuffer* pInDepthmap = inBuffers.pInDepthmap.get();
            extract(&mDumpHint, pInDepthmap);
            genFileName_YUV(writepath, PATH_SIZE, &mDumpHint,
                            TuningUtils::YUV_PORT_UNDEFINED, "VENDOR_IN_BLUR");
            pInDepthmap->saveToFile(writepath);

            IImageBuffer* pOutBokehImage = outBuffers.pOutBokehImage.get();
            extract(&mDumpHint, pOutBokehImage);
            genFileName_YUV(writepath, PATH_SIZE, &mDumpHint,
                            TuningUtils::YUV_PORT_UNDEFINED, "VENDOR_OUT_BOEKH");
            pOutBokehImage->saveToFile(writepath);
        }
    }
}

MBOOL
VendorFilterNode::
prepareVendorInOutData(
    const EffectRequestPtr& request,
    VendorInputBuffer* inputBuffers,
    VendorOutputBuffer* outputBuffers,
    VendorInputParameter* inputParameters
)
{
    // ================================================
    //             Prepare Input/Output buffer
    // ================================================
    if(!getInputBufferFromRequest(request, BOKEH_ER_BUF_MAIN1, inputBuffers->pInClearImage))
    {
        MY_LOGE("Get input buffer for ClearImage fail");
    }
    if(!getInputBufferFromRequest(request, BOKEH_ER_OUTPUT_DEPTHMAP, inputBuffers->pInDepthmap))
    {
        MY_LOGE("Get input buffer for PackedDepthmap fail");
    } else if(mRotatedDepthmap) {
        if (mTargeRotation != eRotate_180)
        {
            if (inputBuffers->pInDepthmap->getImgSize().w ==
                inputBuffers->pInDepthmap->getImgSize().h)
            {
                inputBuffers->pInDepthmapRotate = mRotatedDepthmap;
                rotateBuffer((MUINT8*)inputBuffers->pInDepthmap->getBufVA(0),
                             inputBuffers->pInDepthmap->getImgSize(),
                             (MUINT8*)mRotatedDepthmap->getBufVA(0), mTargeRotation);
            }
            else
                MY_LOGE("Buffer Width != Buffer Height");
        }
    }
    if(!getOutputBufferFromRequest(request, BOKEH_ER_BUF_DISPLAY, outputBuffers->pOutBokehImage))
    {
        MY_LOGE("Get output buffer for BokehImage fail");
    }

    // =============================================================
    //             Prepare Input Parameter (Bokeh level & AF roi)
    // =============================================================
    // get EffectRequest parameter
    const sp<EffectParameter> pEffectParameter = request->getRequestParameter();
    // get bokeh level
    inputParameters->bokehLevel = pEffectParameter->getInt(VSDOF_FRAME_BOKEH_LEVEL);
    // get distance
    inputParameters->iDistance = pEffectParameter->getFloat(VSDOF_FRAME_DISTANCE);
    // get af state
    if(!tryGetMetadataInRequest<MUINT8>(request, FRAME_INPUT, BOKEH_ER_IN_P1_RETURN_META, MTK_CONTROL_AF_STATE, inputParameters->afState))
    {
        MY_LOGE("Get AF State fail");
    }
    // get p1 return metadata
    IMetadata* pMeta_InApp  = nullptr;
    if(!getInputMetadataFromRequest(request, BOKEH_ER_IN_P1_RETURN_META, pMeta_InApp))
    {
        MY_LOGE("Get Meta_InApp fail");
    }
    // get af roi
    IMetadata::IEntry entry = pMeta_InApp->entryFor(MTK_3A_FEATURE_AF_ROI);
    MINT32 afTopLeftX     = 0;
    MINT32 afTopLeftY     = 0;
    MINT32 afBottomRightX = 0;
    MINT32 afBottomRightY = 0;
    MINT32 afROINum       = 0;
    MINT32 afROITagStart  = 0;
    MINT32 centerPointX   = 0;
    MINT32 centerPointY   = 0;

    if(!entry.isEmpty())
    {
        afROINum         = entry.itemAt(1, Type2Type<MINT32>());
        afROITagStart    = (afROINum-1)*5+2; //Special rule to map to 3A metadata tag order
        afTopLeftX       = entry.itemAt(afROITagStart,   Type2Type<MINT32>());
        afTopLeftY       = entry.itemAt(afROITagStart+1, Type2Type<MINT32>());
        afBottomRightX   = entry.itemAt(afROITagStart+2, Type2Type<MINT32>());
        afBottomRightY   = entry.itemAt(afROITagStart+3, Type2Type<MINT32>());
        MY_LOGD_IF(miPipeLogEnable,"Get Meta_InApp done");
    }
    else
    {
        MY_LOGE("Get AF roi fail");
    }

    //Map source buffer(sensor domain) to 4:3 or 16:9 buffer to make ratio align
    MINT32 width  = inputBuffers->pInClearImage.get()->getImgSize().w;
    MINT32 height = inputBuffers->pInClearImage.get()->getImgSize().h;
    centerPointX = (afTopLeftX + afBottomRightX) / 2;
    centerPointY = (afTopLeftY + afBottomRightY) / 2;

    float ratioX = (float)(centerPointX - mActiveCropSift.p.x) / (float) mActiveCropSift.s.w;
    float ratioY = (float)(centerPointY - mActiveCropSift.p.y) / (float) mActiveCropSift.s.h;
    inputParameters->afRoi.x = width * ratioX;
    inputParameters->afRoi.y = height * ratioY;

    // ===================================================================
    //             Prepare Input paramter (FD information)
    // ===================================================================
    // 1. create IFDContainer instance
    auto fdReader = IFDContainer::createInstance(LOG_TAG,  IFDContainer::eFDContainer_Opt_Read);

    // 2. get timestamp
    MINT64 p1timestamp = 0;
    IMetadata* pMeta_inHal = nullptr;
    if(!getInputMetadataFromRequest(request, BOKEH_ER_IN_HAL_META_MAIN1, pMeta_inHal))
    {
        MY_LOGE("Get HAL metadata fail.");
    }
    if( !tryGetMetadata<MINT64>(pMeta_inHal, MTK_P1NODE_FRAME_START_TIMESTAMP, p1timestamp) )
    {
        MY_LOGD("Cant get p1 timestamp meta.");
    }
    // 3. query fd info by timestamps, fdData must be return after use
    auto fdData = fdReader->queryLock(p1timestamp - mVendorFdTolerence, p1timestamp);

    // 4. fd info sameple
    {
        // fdData is the container of face data,
        if (fdData.size() > 0)
        {
            auto fdChunk = fdData.back();

            if (CC_LIKELY( fdChunk != nullptr )) {
                MY_LOGD_IF(miPipeLogEnable, "Number_of_faces:%d",fdChunk->facedata.number_of_faces);
                // Check if exceed the maxium faces count
                MINT32 facesNum = (fdChunk->facedata.number_of_faces >
                                   VENDOR_NODE_MAX_FD_FACES_NUM) ?
                                    VENDOR_NODE_MAX_FD_FACES_NUM :
                                    fdChunk->facedata.number_of_faces;
                inputParameters->faceCounts = facesNum;
                // face,  15 is the max number of faces
                for (size_t i = 0 ; i < facesNum; i++) {
                    // Origianl FD ROI set is from -1000 to 1000, need to map to preview size domain
                    MINT32 width  = inputBuffers->pInClearImage.get()->getImgSize().w;
                    MINT32 height = inputBuffers->pInClearImage.get()->getImgSize().h;
                    MPoint TopLeft(0,0);
                    MPoint BotRight(0,0);
                    float ratioTopX = (float)(fdChunk->facedata.faces[i].rect[0] + 1000 ) / (float) 2000;
                    float ratioTopY = (float)(fdChunk->facedata.faces[i].rect[1] + 1000 ) / (float) 2000;
                    float ratioBotX = (float)(fdChunk->facedata.faces[i].rect[2] + 1000 ) / (float) 2000;
                    float ratioBotY = (float)(fdChunk->facedata.faces[i].rect[3] + 1000 ) / (float) 2000;
                    TopLeft.x = ratioTopX * width;
                    TopLeft.y = ratioTopY * height;
                    BotRight.x= ratioBotX * width;
                    BotRight.y= ratioBotY * height;
                    MINT32 rotationDeg = fdChunk->facedata.fld_rip[i];
                    // Map to ArcSoft face rotate rule
                    if(rotationDeg < 0)
                    {
                        rotationDeg = abs(rotationDeg);
                    }
                    else
                    {
                        rotationDeg = 360 - rotationDeg;
                    }
                    MY_LOGD_IF(miPipeLogEnable, "FD[%d] ROI %d %d %d %d, rotationDeg: %d",
                               i, TopLeft.x, TopLeft.y, BotRight.x, BotRight.y, rotationDeg);
                    inputParameters->fdInfo[i].fdRoiTopLeft.x  = TopLeft.x;
                    inputParameters->fdInfo[i].fdRoiTopLeft.y  = TopLeft.y;
                    inputParameters->fdInfo[i].fdRoiBotRight.x = BotRight.x;
                    inputParameters->fdInfo[i].fdRoiBotRight.y = BotRight.y;
                    inputParameters->fdInfo[i].fdRotationDeg   = rotationDeg;
                }
            }
            else {
                MY_LOGW("fdData is not found");
            }
        }
    }
    // 4. fdData must be return after use
    fdReader->queryUnlock(fdData);

    // ===================================================================
    //             Debug section (log & dump buffer)
    // ===================================================================
    MY_LOGD_IF(miPipeLogEnable, "mActiveCropSift %d %d %d %d", mActiveCropSift.p.x, mActiveCropSift.p.y, mActiveCropSift.s.w, mActiveCropSift.s.h);
    MY_LOGD_IF(miPipeLogEnable, "af roi %d %d %d %d", afTopLeftX, afTopLeftY, afBottomRightX, afBottomRightY);
    MY_LOGD_IF(miPipeLogEnable, "centerPoint %d %d %d %d", centerPointX, centerPointY, inputParameters->afRoi.x, inputParameters->afRoi.y);
    MY_LOGD_IF(miPipeLogEnable,"bokehLevel(%d), afState(%d), inputParameters->afRoi.x(%d),"
                               "inputParameters->afRoi.y(%d), distance(%d)",
                               inputParameters->bokehLevel, inputParameters->afState,
                               inputParameters->afRoi.x, inputParameters->afRoi.y,
                               inputParameters->iDistance);

    return MTRUE;
}

//************************************************************************
//
//************************************************************************
MBOOL
VendorFilterNode::
processVendor(
    const VendorInputParameter& inputParameters,
    VendorInputBuffer inputBuffers,
    VendorOutputBuffer outputBuffers
)
{
    CAM_TRACE_BEGIN("VendorFilterNode::processVendor");
    MY_LOGD_IF(miPipeLogEnable, "+");

    MBOOL ret = MTRUE;
    // ---------------------------
    // Get input parameter example
    // ---------------------------
    // Get Bokeh level
    MINT32 bokehLevel = inputParameters.bokehLevel;
    // Get AF State
    MINT32 afState = inputParameters.afState;
    // Get AF ROI
    MPoint afRoi = inputParameters.afRoi;
    // Get FD info
    MINT32 fdFaceNumber = inputParameters.faceCounts; // Number of faces
    for (size_t i = 0 ; i < inputParameters.faceCounts; i++)
    {
        MPoint fdRoiTopLeft  = inputParameters.fdInfo[i].fdRoiTopLeft;  // FD ROI top left point
        MPoint fdRoiBotRight = inputParameters.fdInfo[i].fdRoiBotRight; // FD ROI bottom right point
        MINT32 fdRotationDeg = inputParameters.fdInfo[i].fdRotationDeg; // FD rotation information
        MY_LOGD_IF(miPipeLogEnable, "faceId:%d, fdRoiTopLeft(%d.%d), fdRoiBotRight(%d.%d), fdRotationDeg(%d)",
                                     i, fdRoiTopLeft.x, fdRoiTopLeft.y, fdRoiBotRight.x, fdRoiBotRight.y, fdRotationDeg);
    }
    // --------------------------------
    // Add IN AND OUT vendor code here
    // and return process result
    // --------------------------------
    if(!copy(inputBuffers.pInClearImage, outputBuffers.pOutBokehImage)) //this is example PLEASE replace it
    {
        ret = MFALSE;
    }
    //
    MY_LOGD_IF(miPipeLogEnable, "-");
    CAM_TRACE_END();
    return ret;
}

MBOOL
VendorFilterNode::
copy(
    sp<IImageBuffer> source,
    sp<IImageBuffer> target
)
{
    if(source != nullptr)
    {
        if(mpDpStream == nullptr)
        {
            MY_LOGE("mpDpStream is null.");
            return MFALSE;
        }
        sMDP_Config config;
        config.pDpStream = mpDpStream;
        config.pSrcBuffer = source.get();
        config.pDstBuffer = target.get();
        if(!excuteMDP(config))
        {
            MY_LOGE("excuteMDP fail.");
            return MFALSE;
        }
    }
    return MTRUE;
}

bool
VendorFilterNode::rotateBuffer(
    const MUINT8 *SRC_BUFFER,
    const MSize SRC_SIZE,
    MUINT8 *dstBuffer,
    ENUM_ROTATION targetRotation
)
{
    if(NULL == SRC_BUFFER ||
       NULL == dstBuffer ||
       0 == SRC_SIZE.w ||
       0 == SRC_SIZE.h)
    {
        return false;
    }

    // AutoProfileUtil profile(TAG, "Rotate Depthmap");

    const MINT32 BUFFER_LEN = SRC_SIZE.w * SRC_SIZE.h;
    if(eRotate_0 == targetRotation) {
        ::memcpy(dstBuffer, SRC_BUFFER, BUFFER_LEN);
        return true;
    }

    //Rotate by CPU
    MINT32 writeRow = 0;
    MINT32 writeCol = 0;
    MINT32 writePos = 0;

    if(eRotate_270 == targetRotation) {
        writeRow = 0;
        writeCol = SRC_SIZE.h - 1;
        writePos = writeCol;
        for(int i = BUFFER_LEN-1; i >= 0; --i) {
            *(dstBuffer + writePos) = *(SRC_BUFFER + i);

            writePos += SRC_SIZE.h;
            ++writeRow;
            if(writeRow >= SRC_SIZE.w) {
                writeRow = 0;
                --writeCol;
                writePos = writeCol;
            }
        }
    } else if(eRotate_90 == targetRotation) {
        writeRow = SRC_SIZE.w - 1;
        writeCol = 0;
        writePos = BUFFER_LEN - SRC_SIZE.h;
        for(int i = BUFFER_LEN-1; i >= 0; --i) {
            *(dstBuffer + writePos) = *(SRC_BUFFER + i);

            writePos -= SRC_SIZE.h;
            --writeRow;
            if(writeRow < 0) {
                writeRow = SRC_SIZE.w - 1;
                ++writeCol;
                writePos = BUFFER_LEN - SRC_SIZE.h + writeCol;
            }
        }
    }

    return true;
}
