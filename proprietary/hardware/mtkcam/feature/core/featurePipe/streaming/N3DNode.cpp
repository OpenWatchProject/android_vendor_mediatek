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

#include <mtkcam/feature/stereo/hal/stereo_setting_provider.h>
#include <mtkcam/feature/stereo/hal/stereo_size_provider.h>

#include "N3DNode.h"
#include "N3DConfig.h"
#include <mtkcam/feature/DualCam/DualCam.Common.h>

#define PIPE_CLASS_TAG "N3DNode"
#define PIPE_TRACE TRACE_N3D_NODE
#define SCENEINFO_CNT 15
#include <featurePipe/core/include/PipeLog.h>

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

N3DNode::N3DNode(const char *name)
    : StreamingFeatureNode(name)
{
    TRACE_FUNC_ENTER();
    this->addWaitQueue(&mN3DDatas);
    mShotModeInfo = ::property_get_int32("vendor.debug.shotmodeinfo.log", 0);
    TRACE_FUNC_EXIT();
}

N3DNode::~N3DNode()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
}

MBOOL N3DNode::onInit()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3DNode::onUninit()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3DNode::onThreadStart()
{
    TRACE_FUNC_ENTER();
    CAM_TRACE_NAME("N3DNode::onThreadStart");
    StereoSizeProvider * pSizeProvder = StereoSizeProvider::getInstance();
    //  get size
    MSize szSceneInfo = MSize(StereoSettingProvider::getMaxSceneInfoBufferSizeInBytes(), 1);
    MSize szN3DImg = pSizeProvder->getBufferSize(E_MV_Y, eSTEREO_SCENARIO_PREVIEW);
    MSize szN3DMask = pSizeProvder->getBufferSize(E_MASK_M_Y, eSTEREO_SCENARIO_PREVIEW);
    // create pool
    mN3DSceneinfoBufPool = ImageBufferPool::create("mN3DSceneinfoBufPool", szSceneInfo.w,
                                                   szSceneInfo.h, eImgFmt_Y8, ImageBufferPool::USAGE_SW);
    mN3DImgBufPool = ImageBufferPool::create("N3DImgBufPool", szN3DImg.w,
                                        szN3DImg.h, eImgFmt_YV12, ImageBufferPool::USAGE_SW);
    mN3DMaskBufPool = ImageBufferPool::create("N3DMaskBufPool", szN3DMask.w,
                                        szN3DMask.h, eImgFmt_Y8, ImageBufferPool::USAGE_SW);

    mN3DSceneinfoBufPool->allocate(DENOISE_WORKING_BUF_SET);
    mN3DImgBufPool->allocate(2);
    mN3DMaskBufPool->allocate(2);

    // create N3DHAL instance for scenarios
    N3D_HAL_INIT_PARAM initParam_VRPV;
    initParam_VRPV.eScenario = eSTEREO_SCENARIO_RECORD;
    initParam_VRPV.fefmRound = DENOISE_CONST_FE_EXEC_TIMES;
    mpN3DHAL_PV = N3D_HAL::createInstance(initParam_VRPV);
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3DNode::onThreadStop()
{
    TRACE_FUNC_ENTER();
    if(mpN3DHAL_PV != nullptr)
    {
        delete mpN3DHAL_PV;
        mpN3DHAL_PV = nullptr;
    }
    mN3DDatas.clear();
    //
    ImageBufferPool::destroy(mN3DSceneinfoBufPool);
    ImageBufferPool::destroy(mN3DImgBufPool);
    ImageBufferPool::destroy(mN3DMaskBufPool);
    TRACE_FUNC_EXIT();
    return MTRUE;

}

MBOOL N3DNode::onData(DataID id, const N3DData &data)
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Frame %d: %s arrived", data.mRequest->mRequestNo, ID2Name(id));
    MBOOL ret = MFALSE;
    if( id == ID_N3DP2_TO_N3D )
    {
        mN3DDatas.enque(data);
        ret = MTRUE;
    }
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL N3DNode::onThreadLoop()
{
    TRACE_FUNC_ENTER();
    TRACE_FUNC("Waitloop");
    N3DData n3dData;
    RequestPtr request;

    if( !waitAllQueue() )
    {
        return MFALSE;
    }

    if( !mN3DDatas.deque(n3dData) )
    {
        MY_LOGE("Request deque out of sync");
        return MFALSE;
    }

    request = n3dData.mRequest;
    TRACE_FUNC("Frame %d in N3DNode needN3D=%d", request->mRequestNo, request->needN3D());
    CAM_TRACE_NAME("N3DNode::onThreadLoop");
    MBOOL ret;
    if(request->needN3D())
    {
        ret = performN3DALGO_PV(n3dData);
    }
    else
    {
        selectProperShotMode(MFALSE, request, nullptr);
        ret = this->handleDataToNext(request);
    }
    TRACE_FUNC("Frame %d in N3DNode, success=%d", request->mRequestNo, ret);
    TRACE_FUNC_EXIT();
    return ret;
}

MBOOL N3DNode::handleDataToNext(RequestPtr& pRequest)
{
    if(mPipeUsage.supportVendor())
    {
        return handleData(ID_N3D_TO_VMDP, pRequest);
    }
    else
    {
        return handleData(ID_N3D_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_FRAME_DONE, pRequest));
    }
}

MBOOL N3DNode::performN3DALGO_PV(N3DData &data)
{
    TRACE_FUNC_ENTER();
    CAM_TRACE_NAME("N3DNode::performN3DALGO_PV");
    RequestPtr pRequest = data.mRequest;
    N3D_HAL_PARAM n3dParams;
    N3D_HAL_OUTPUT n3dOutput;
    // prepare input params
    MBOOL bRet = prepareN3DInputParam(data, n3dParams);
    bRet &= prepareN3DOutputParam(n3dOutput);

    if(!bRet)
    {
        MY_LOGE("reqID=%d, failed to prepare PV N3D params to run N3D Hal.", pRequest->mRequestNo);
        return MFALSE;
    }
    // debugN3DParams(n3dParams, n3dOutput);
    // start timer
    pRequest->mTimer.startN3D();
    CAM_TRACE_BEGIN("N3DNode::N3DHALRun");
    bool ret = mpN3DHAL_PV->N3DHALRun(n3dParams, n3dOutput);
    pRequest->mTimer.stopN3D();
    CAM_TRACE_END();
    if(ret)
    {
        TRACE_FUNC("Frame: %d, finished N3D PV ALGO, exec-time=%d msec",
                    pRequest->mRequestNo,pRequest->mTimer.getElapsedN3D());
        selectProperShotMode(MTRUE, pRequest, n3dOutput.sceneInfo);
    }
    else
        MY_LOGE("Frame: %d, N3D execute failed.", pRequest->mRequestNo);
    //
    this->handleDataToNext(pRequest);
    return ret;
    TRACE_FUNC_EXIT();
}

MVOID N3DNode::selectProperShotMode(bool isExecuted, const RequestPtr &pRequest, int* pSceneInfo)
{
    if(mShotModeInfo)
    {
        if(pSceneInfo == nullptr)
        {
            MY_LOGD("mRequestNo=%d, no SceneInfo", pRequest->mRequestNo);
        }
        else
        {
            for(int i=0;i<SCENEINFO_CNT;i++)
            {
                MY_LOGD("mRequestNo=%d, SceneInfo[%d]=%d", pRequest->mRequestNo, i , pSceneInfo[i]);
            }
        }
    }

    MINT32 iso = 200;
    iso = pRequest->getVar<MUINT32>("3dnr.iso", iso);
    BMDeNoiseType deNoiseType = mSelectShotMode.decideDenoiseType(pSceneInfo, iso, (int)isExecuted);
    DENOISE_SCENE_RESULT result = DENOISE_SCENE_RESULT::DENOISE_SCENE_NOT_SUPPORT;
    if(deNoiseType == DN_TYPE_TBD )
    {
        result = DENOISE_SCENE_RESULT::DENOISE_SCENE_NOT_SUPPORT;
    }
    else if(deNoiseType == DN_TYPE_BMNR)
    {
        result = DENOISE_SCENE_RESULT::DENOISE_SCENE_DENOISE;
    }
    else if(deNoiseType == DN_TYPE_MFHR)
    {
        result = DENOISE_SCENE_RESULT::DENOISE_SCENE_HIGH_RES;
    }
    pRequest->setVar<MUINT8>(VAR_N3D_SHOTMODE, (MUINT8)result);
    if(mShotModeInfo)
    {
        MY_LOGD("generate shotMode: %d mRequestNo=%d, iso =%d", (MUINT8)result, pRequest->mRequestNo, iso);
    }
    handleData(ID_N3D_TO_HELPER, CBMsgData(FeaturePipeParam::MSG_N3D_SET_SHOTMODE, pRequest));
}

MBOOL N3DNode::prepareN3DInputParam(N3DData &data, N3D_HAL_PARAM& rN3dParam)
{
    TRACE_FUNC_ENTER();
    CAM_TRACE_NAME("prepareN3DInputParam");
    // prepare FEFM input
    N3DResult n3dRes = data.mData;
    prepareFEFMData(data, rN3dParam.hwfefmData);

    // prepare Rect/CC input
    IImageBuffer* pImgBuf_CCIn1 = n3dRes.mCCin_Master->getImageBufferPtr();
    IImageBuffer* pImgBuf_CCIn2 = n3dRes.mCCin_Slave->getImageBufferPtr();
    IImageBuffer* pImgBuf_RectIn1 = n3dRes.mRectin_Master->getImageBufferPtr();
    IImageBuffer* pImgBuf_RectIn2 = n3dRes.mRectin_Slave->getImageBufferPtr();

    pImgBuf_RectIn1->syncCache(eCACHECTRL_INVALID);
    pImgBuf_RectIn2->syncCache(eCACHECTRL_INVALID);
    pImgBuf_CCIn1->syncCache(eCACHECTRL_INVALID);
    pImgBuf_CCIn2->syncCache(eCACHECTRL_INVALID);

    rN3dParam.rectifyImgMain1 = pImgBuf_RectIn1;
    rN3dParam.rectifyImgMain2 = pImgBuf_RectIn2;
    rN3dParam.ccImage[0] = pImgBuf_CCIn1;
    rN3dParam.ccImage[1] = pImgBuf_CCIn2;

    RequestPtr pRequest = data.mRequest;
    MBOOL bRet = MFALSE;
    //
    MINT32 depthAFON = 0;
    MINT32 disMeasureFON = 0;
    MUINT8 isAFTrigger;
    MINT32 magicNum = 0;

    isAFTrigger = pRequest->getVar<MUINT8>(VAR_N3D_ISAFTRIGGER, 0);
    //meta
    rN3dParam.requestNumber = data.mRequest->mRequestNo;
    rN3dParam.isAFTrigger = isAFTrigger;
    rN3dParam.isDepthAFON = depthAFON;
    rN3dParam.isDistanceMeasurementON = disMeasureFON;
    TRACE_FUNC_EXIT();
    return MTRUE;
}

MBOOL N3DNode::prepareN3DOutputParam(N3D_HAL_OUTPUT& rN3dParam)
{
    SmartImageBuffer smImgBuf_MVY = mN3DImgBufPool->request();
    SmartImageBuffer smImgBuf_SVY = mN3DImgBufPool->request();
    SmartImageBuffer smImgBuf_MASK_S = mN3DMaskBufPool->request();
    SmartImageBuffer smImgBuf_MASK_M = mN3DMaskBufPool->request();

    rN3dParam.rectifyImgMain1 = smImgBuf_MVY->mImageBuffer.get();
    rN3dParam.rectifyImgMain2 = smImgBuf_SVY->mImageBuffer.get();
    rN3dParam.maskMain1 = (MUINT8*) smImgBuf_MASK_M->mImageBuffer->getBufVA(0);
    rN3dParam.maskMain2 = (MUINT8*) smImgBuf_MASK_S->mImageBuffer->getBufVA(0);

    SmartImageBuffer smImgBuf_Sceneinfo =  mN3DSceneinfoBufPool->request();
    rN3dParam.sceneInfo = (MINT32*) smImgBuf_Sceneinfo->mImageBuffer->getBufVA(0);
    return MTRUE;
}

MVOID N3DNode::debugN3DParams(N3D_HAL_PARAM& inParam, N3D_HAL_OUTPUT& output)
{
    TRACE_FUNC_ENTER();
    for(int i=0;i<3;i++)
    {
        TRACE_FUNC("inParam.hwfefmData.geoDataMain1[%d]=%x",i, inParam.hwfefmData.geoDataMain1[i]);
    }

    for(int i=0;i<3;i++)
    {
        TRACE_FUNC("inParam.hwfefmData.geoDataMain2[%d]=%x",i, inParam.hwfefmData.geoDataMain2[i]);
    }

    for(int i=0;i<3;i++)
    {
        TRACE_FUNC("inParam.hwfefmData.geoDataLeftToRight[%d]=%x",i, inParam.hwfefmData.geoDataLeftToRight[i]);
    }

    for(int i=0;i<3;i++)
    {
        TRACE_FUNC("inParam.hwfefmData.geoDataRightToLeft[%d]=%x",i, inParam.hwfefmData.geoDataRightToLeft[i]);
    }

    TRACE_FUNC("inParam.rectifyImgMain1=%x",inParam.rectifyImgMain1);
    TRACE_FUNC("inParam.rectifyImgMain2=%x",inParam.rectifyImgMain2);

    TRACE_FUNC("inParam.ccImage[0]=%x",inParam.ccImage[0]);
    TRACE_FUNC("inParam.ccImage[1]=%x", inParam.ccImage[1]);
    TRACE_FUNC("inParam.magicNumber=%d, %d", inParam.magicNumber[0], inParam.magicNumber[1]);
    TRACE_FUNC("inParam.requestNumber=%d", inParam.requestNumber);
    TRACE_FUNC("inParam.isAFTrigger=%d", inParam.isAFTrigger);
    TRACE_FUNC("inParam.isDepthAFON=%d", inParam.isDepthAFON);
    TRACE_FUNC("inParam.isDistanceMeasurementON=%d", inParam.isDistanceMeasurementON);

    TRACE_FUNC_EXIT();
}
MBOOL N3DNode::prepareFEFMData(N3DData &data, HWFEFM_DATA& rFefmData)
{
    CAM_TRACE_NAME("prepareFEFMData");
    TRACE_FUNC("prepareFEFMData +");
    N3DResult n3dRes = data.mData;
    // N3D input FEO/FMO data
    IImageBuffer* pImgBuf_Fe1bo = n3dRes.mFEBO_Master->getImageBufferPtr();
    IImageBuffer* pImgBuf_Fe2bo = n3dRes.mFEBO_Slave->getImageBufferPtr();
    IImageBuffer* pImgBuf_Fe1co = n3dRes.mFECO_Master->getImageBufferPtr();
    IImageBuffer* pImgBuf_Fe2co = n3dRes.mFECO_Slave->getImageBufferPtr();
    pImgBuf_Fe1bo->syncCache(eCACHECTRL_INVALID);
    pImgBuf_Fe2bo->syncCache(eCACHECTRL_INVALID);
    pImgBuf_Fe1co->syncCache(eCACHECTRL_INVALID);
    pImgBuf_Fe2co->syncCache(eCACHECTRL_INVALID);

    rFefmData.geoDataMain1[0] = (MUINT16*)pImgBuf_Fe1bo->getBufVA(0);
    rFefmData.geoDataMain1[1] = (MUINT16*)pImgBuf_Fe1co->getBufVA(0);
    rFefmData.geoDataMain1[2] = NULL;

    rFefmData.geoDataMain2[0] = (MUINT16*)pImgBuf_Fe2bo->getBufVA(0);
    rFefmData.geoDataMain2[1] = (MUINT16*)pImgBuf_Fe2co->getBufVA(0);
    rFefmData.geoDataMain2[2] = NULL;

    IImageBuffer* pImgBuf_FmboLR = n3dRes.mFMBO_MtoS->getImageBufferPtr();
    IImageBuffer* pImgBuf_FmboRL = n3dRes.mFMBO_StoM->getImageBufferPtr();
    IImageBuffer* pImgBuf_FmcoLR = n3dRes.mFMCO_MtoS->getImageBufferPtr();
    IImageBuffer* pImgBuf_FmcoRL = n3dRes.mFMCO_StoM->getImageBufferPtr();
    pImgBuf_FmboLR->syncCache(eCACHECTRL_INVALID);
    pImgBuf_FmboRL->syncCache(eCACHECTRL_INVALID);
    pImgBuf_FmcoLR->syncCache(eCACHECTRL_INVALID);
    pImgBuf_FmcoRL->syncCache(eCACHECTRL_INVALID);

    rFefmData.geoDataLeftToRight[0] = (MUINT16*)pImgBuf_FmboLR->getBufVA(0);
    rFefmData.geoDataLeftToRight[1] = (MUINT16*)pImgBuf_FmcoLR->getBufVA(0);
    rFefmData.geoDataLeftToRight[2] = NULL;

    rFefmData.geoDataRightToLeft[0] = (MUINT16*)pImgBuf_FmboRL->getBufVA(0);
    rFefmData.geoDataRightToLeft[1] = (MUINT16*)pImgBuf_FmcoRL->getBufVA(0);
    rFefmData.geoDataRightToLeft[2] = NULL;

    TRACE_FUNC("prepareFEFMData -");
    return MTRUE;
}

}; //NSFeaturePipe
}; //NSCamFeature
}; //NSCam