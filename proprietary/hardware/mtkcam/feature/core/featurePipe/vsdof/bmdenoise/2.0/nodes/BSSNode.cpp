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
#include "BSSNode.h"

#define PIPE_MODULE_TAG "MFHR"
#define PIPE_CLASS_TAG "BSSNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// MET tags
#define DO_BSS "doBSS"

/**
 *  The confidence threshold of GMV, which means if the confidence is not
 *  enough then the GMV won't be applied.
 */
#define MFC_GMV_CONFX_TH 25
#define MFC_GMV_CONFY_TH 25

#define SKIP_BSS 1

#include <PipeLog.h>

#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <mtkcam/feature/eis/eis_ext.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NS3Av3;
using namespace NSIoPipe;
/*******************************************************************************
 *
 ********************************************************************************/
BSSNode::
BSSNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mImgInfoRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
BSSNode::
~BSSNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BSSNode::
onData(
    DataID id,
    ImgInfoMapPtr &imgInfo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    TRACE_FUNC_ENTER();

    MBOOL ret = MFALSE;
    switch(id)
    {
        case ROOT_HR_TO_BSS:
            mImgInfoRequests.enque(imgInfo);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    TRACE_FUNC_EXIT();
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BSSNode::
onInit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BSSNode::
onUninit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    cleanUp();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BSSNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(mp3AHal_Main1)
    {
        mp3AHal_Main1->destroyInstance("BMDENOISE_3A_MAIN1");
        mp3AHal_Main1 = NULL;
    }
    if(mp3AHal_Main2)
    {
        mp3AHal_Main2->destroyInstance("BMDENOISE_3A_MAIN2");
        mp3AHal_Main2 = NULL;
    }
    mBufPool.uninit();
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BSSNode::
onThreadStart()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x, Main2: %x", mp3AHal_Main1, mp3AHal_Main2);

    initBufferPool();

    mvPendingBSSCandidates.clear();

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BSSNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BSSNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    ImgInfoMapPtr imgInfo = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }
    if( !mImgInfoRequests.deque(imgInfo) )
    {
        MY_LOGD("mImgInfoRequests.deque() failed");
        return MFALSE;
    }

    CAM_TRACE_CALL();

    this->incExtThreadDependency();

    ImgInfoMapPtr imgInfo_BSS = doBSS(imgInfo);

    if(imgInfo_BSS != nullptr){
        handleData(BSS_TO_P2AFM, imgInfo_BSS);
    }

    this->decExtThreadDependency();
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BSSNode::
retrieveGmvInfo(IMetadata* pMetadatadata, int& x, int& y, MSize& size)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL  ret = MTRUE;
    MSize  rzoSize;
    IMetadata::IEntry entry;
    struct __confidence{
        MINT32 x;
        MINT32 y;
        __confidence() : x(0), y(0) {}
    } confidence;

    /* get size first */
    ret = tryGetMetadata<MSize>(pMetadatadata, MTK_P1NODE_RESIZER_SIZE, rzoSize);
    if (ret != MTRUE) {
        MY_LOGE("%s: cannot get rzo size", __FUNCTION__);
        goto lbExit;
    }

    entry = pMetadatadata->entryFor(MTK_EIS_REGION);

    /* check if a valid EIS_REGION */
    if (entry.count() < EIS_REGION_INDEX_SIZE) {
        MY_LOGE("%s: entry is not a valid EIS_REGION, size = %d",
                __FUNCTION__,
                entry.count());
        ret = MFALSE;
        goto lbExit;
    }

    /* read confidence */
    confidence.x = static_cast<MINT32>(entry.itemAt(EIS_REGION_INDEX_CONFX, Type2Type<MINT32>()));
    confidence.y = static_cast<MINT32>((MINT32)entry.itemAt(EIS_REGION_INDEX_CONFY, Type2Type<MINT32>()));

    /* to read GMV if confidence is enough */
    if (confidence.x > MFC_GMV_CONFX_TH) {
        x = entry.itemAt(EIS_REGION_INDEX_GMVX, Type2Type<MINT32>());
    }

    if (confidence.y > MFC_GMV_CONFY_TH) {
        y = entry.itemAt(EIS_REGION_INDEX_GMVY, Type2Type<MINT32>());
    }

    size = rzoSize;

    MY_LOGD("EIS info conf(x,y) = (%d, %d), gmv(x, y) = (%d, %d)",
            confidence.x, confidence.y, x, y);

lbExit:
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
BSSNode::GMV
BSSNode::
calMotionVector(IMetadata* pMetadata, MBOOL isMain)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    BSSNode::GMV        mv;
    MSize               rzoSize;
    MRect               p1ScalarRgn;
    MBOOL               ret = MTRUE;

    /* to get GMV info and the working resolution */
    ret = retrieveGmvInfo(pMetadata, mv.x, mv.y, rzoSize);
    if (ret == MTRUE) {
        ret = tryGetMetadata<MRect>(
                pMetadata,
                MTK_P1NODE_SCALAR_CROP_REGION,
                p1ScalarRgn);
    }

    /* if works, mapping it from rzoDomain to MfllCore domain */
    if (ret == MTRUE) {
        /* the first frame, set GMV as zero */
        if (isMain) {
            mv.x = 0;
            mv.y = 0;
        }

        MY_LOGD("GMV(x,y)=(%d,%d), unit based on resized RAW",
                mv.x, mv.y);

        MY_LOGD("p1node scalar crop rgion (width): %d, gmv domain(width): %d",
                p1ScalarRgn.s.w, rzoSize.w);
        /**
         *  the cropping crops height only, not for width. Hence, just
         *  simply uses width to calculate the ratio.
         */
        float ratio =
            static_cast<float>(p1ScalarRgn.s.w)
            /
            static_cast<float>(rzoSize.w)
            ;
        MY_LOGD("%s: ratio = %f", __FUNCTION__, ratio);

        // we don't need floating computing because GMV is formated
        // with 8 bits floating point
        mv.x *= ratio;
        mv.y *= ratio;

        /* normalization */
        mv.x = mv.x >> 8;
        mv.y = mv.y >> 8;

        // assume the ability of EIS algo, which may seach near by
        // N pixels only, so if the GMV is more than N pixels,
        // we clip it

        auto CLIP = [](int x, const int n) -> int {
            if (x < -n)     return -n;
            else if(x > n)  return n;
            else            return x;
        };

        // Hence we've already known that search region is 32 by 32
        // pixel based on RRZO domain, we can map it to full size
        // domain and makes clip if it's out-of-boundary.
        int c = static_cast<int>(ratio * 32.0f);
        mv.x = CLIP(mv.x, c);
        mv.y = CLIP(mv.y, c);

        MY_LOGI("GMV'(x,y)=(%d,%d), unit: Mfll domain", mv.x, mv.y);
    }
    return mv;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BSSNode::
updateBSSProcInfo(MSize srcSize, BSS_PARAM_STRUCT& bss_param, BSS_INPUT_DATA_G& bss_data)
{
    AutoProfileUtil proflie(PIPE_LOG_TAG, "BSS set param");

    MINT32 roiPercentage = MF_BSS_ROI_PERCENTAGE;
    MINT32 w = (srcSize.w * roiPercentage + 5) / 100;
    MINT32 h = (srcSize.h * roiPercentage + 5) / 100;
    MINT32 x = (srcSize.w - w) / 2;
    MINT32 y = (srcSize.h - h) / 2;

    bss_param.BSS_ROI_WIDTH       = w;
    bss_param.BSS_ROI_HEIGHT      = h;
    bss_param.BSS_ROI_X0          = x;
    bss_param.BSS_ROI_Y0          = y;
    bss_param.BSS_SCALE_FACTOR    = MF_BSS_SCALE_FACTOR;
    bss_param.BSS_CLIP_TH0        = MF_BSS_CLIP_TH0;
    bss_param.BSS_CLIP_TH1        = MF_BSS_CLIP_TH1;
    bss_param.BSS_ZERO            = MF_BSS_ZERO;
    bss_param.BSS_ON              = MF_BSS_ON;
    bss_param.BSS_FRAME_NUM       = getMFHRCaptureCnt();

    bss_data.inWidth              = srcSize.w;
    bss_data.inHeight             = srcSize.h;

    MY_LOGD("WxH(%dx%d) roi(%d%%)(%dx%d,%d,%d) factor:%d clip(%dx%d) zero:%d bss_on(%d) frameNo(%d)",
        bss_data.inWidth,
        bss_data.inHeight,
        roiPercentage,
        bss_param.BSS_ROI_WIDTH,
        bss_param.BSS_ROI_HEIGHT,
        bss_param.BSS_ROI_X0,
        bss_param.BSS_ROI_Y0,
        bss_param.BSS_SCALE_FACTOR,
        bss_param.BSS_CLIP_TH0,
        bss_param.BSS_CLIP_TH1,
        bss_param.BSS_ZERO,
        bss_param.BSS_ON,
        bss_param.BSS_FRAME_NUM
    );
}
/*******************************************************************************
 *
 ********************************************************************************/
ImgInfoMapPtr
BSSNode::
doBSS(
    ImgInfoMapPtr imgInfo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    ImgInfoMapPtr ret = nullptr;

    mvPendingBSSCandidates.push_back(imgInfo);

    MY_LOGD("%d/%d", mvPendingBSSCandidates.size(), getMFHRCaptureCnt());

    if(mvPendingBSSCandidates.size() == getMFHRCaptureCnt()){
        MTKBss* pBssDrv = nullptr;
        BSS_PARAM_STRUCT bss_param;
        BSS_INPUT_DATA_G input_data;
        BSS_OUTPUT_DATA result_data;
        {
            AutoProfileUtil proflie(PIPE_LOG_TAG, "BSS createInstance");
            pBssDrv = MTKBss::createInstance(DRV_BSS_OBJ_SW);
            if(pBssDrv == nullptr){
                MY_LOGE("BSS alg create failed!");
                return MFALSE;
            }
        }

        {
            AutoProfileUtil proflie(PIPE_LOG_TAG, "BSS init");
            pBssDrv->BssInit(nullptr, nullptr);
        }

        {
            AutoProfileUtil proflie(PIPE_LOG_TAG, "BSS main");
            int i = 0;
            for(auto e:mvPendingBSSCandidates){
                MINT32 isMainFrame = e->getRequestPtr()->getParam(PID_MFHR_IS_MAIN);
                sp<IImageBuffer> pBuf_RSRAW_Mono = e->getIImageBuffer(BID_INPUT_RSRAW_2);

                MY_LOGD("%d/%d, isMain(%d)", i, mvPendingBSSCandidates.size(), isMainFrame);
                if(isMainFrame == 1){
                    // shutter callback
                    handleData(SHUTTER, e->getRequestPtr());
                    // bss params
                    updateBSSProcInfo(pBuf_RSRAW_Mono->getImgSize(), bss_param, input_data);
                    pBssDrv->BssFeatureCtrl(BSS_FTCTRL_SET_PROC_INFO, &bss_param, nullptr);
                }

                input_data.apbyBssInImg[i] = (MUINT16*)pBuf_RSRAW_Mono->getBufVA(0);
                MY_LOGD("eImgFmt_FG_BAYER8:%x, eImgFmt_FG_BAYER10:%x, eImgFmt_FG_BAYER12:%x, eImgFmt_FG_BAYER14:%x",
                    eImgFmt_FG_BAYER8,
                    eImgFmt_FG_BAYER10,
                    eImgFmt_FG_BAYER12,
                    eImgFmt_FG_BAYER14
                );
                MY_LOGD("input_data.apbyBssInImg[%d]:%p, fmt(%x), BPP(%d), stride(%d)",
                    i,
                    input_data.apbyBssInImg[i],
                    pBuf_RSRAW_Mono->getImgFormat(),
                    pBuf_RSRAW_Mono->getImgBitsPerPixel(),
                    pBuf_RSRAW_Mono->getBufStridesInBytes(0)
                );

                IMetadata* pHalMeta = e->getRequestPtr()->getMetadata(BID_META_IN_HAL);
                BSSNode::GMV mv = calMotionVector(
                    pHalMeta,
                    (isMainFrame == 1) ? MTRUE : MFALSE
                );
                input_data.gmv[i].x = mv.x;
                input_data.gmv[i].y = mv.y;
                i++;
            }
            pBssDrv->BssMain(BSS_PROC2, &input_data, &result_data);
        }
        pBssDrv->destroyInstance();

        // pack the chosen one
        MY_LOGD("the best shot is:%d", result_data.originalOrder[0]);
        {
            ImgInfoMapPtr bestRequest = nullptr;
            int i = 0;
            for(auto e:mvPendingBSSCandidates){
                if(i == result_data.originalOrder[0]){
                    bestRequest = e;
                    break;
                }
                i++;
            }
            if(bestRequest == nullptr){
                MY_LOGE("cant find best shot(%d) in pendingBSScandidates?(%d)", result_data.originalOrder[0], mvPendingBSSCandidates.size());
                return nullptr;
            }
            ret = bestRequest;
        }

        // send BSS result to MFNRNode
        Vector<MINT32> bssResult;
        int i=0;
        if(SKIP_BSS == 1){
            MY_LOGD("skip bss");
            for(int j=0; j<getMFHRCaptureCnt(); j++){
                MY_LOGD("the bss result[%d]:%d", i, j);
                bssResult.add(j);
                i++;
            }
        }else{
            for(auto e:result_data.originalOrder){
                MY_LOGD("the bss result[%d]:%d", i, e);
                bssResult.add(e);
                i++;
            }
        }
        handleData(BSS_RESULT, bssResult);
        // remove used pendingBSScandidates
        mvPendingBSSCandidates.clear();
    }
    return ret;
}