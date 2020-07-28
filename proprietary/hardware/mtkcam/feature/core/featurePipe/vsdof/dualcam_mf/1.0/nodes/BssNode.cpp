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
#include "../util/vsdof_util.h"

#include <unordered_map>

#include "BssNode.h"

#define PIPE_MODULE_TAG "DualCamMF"
#define PIPE_CLASS_TAG "BssNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG
#include <PipeLog.h>

#include <mtkcam/feature/stereo/hal/stereo_common.h>

#include <mtkcam/feature/eis/eis_ext.h>

#include <custom/feature/mfnr/camera_custom_mfll.h>
#include <custom/debug_exif/dbg_exif_param.h>
#if (MFLL_MF_TAG_VERSION > 0)
using namespace __namespace_mf(MFLL_MF_TAG_VERSION);
#include <tuple>
#endif

#include <mtkcam/drv/IHalSensor.h>

#include "exif/ExifWriter.h"

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NSCam::NSCamFeature::NSFeaturePipe::NSDCMF;
using namespace VSDOF::util;

#define DO_BSS "DO_BSS"

#define CHECK_OBJECT(x)  do{                                            \
    if (x == nullptr) { MY_LOGE("Null %s Object", #x); return -MFALSE;} \
} while(0)

/*******************************************************************************
 *
 ********************************************************************************/
BssNode::
BssNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : DualCamMFPipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mRequests);
}
/*******************************************************************************
 *
 ********************************************************************************/
BssNode::
~BssNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MBOOL ret = MFALSE;
    Mutex::Autolock _l(mLock);
    switch(id)
    {
        case ROOT_ENQUE:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
onInit()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    CAM_TRACE_CALL();
    if(!DualCamMFPipeNode::onInit()){
        MY_LOGE("DualCamMFPipeNode::onInit() failed!");
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
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
BssNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mBufPool.uninit();
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
onThreadStart()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    initBufferPool();

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    // block until queue ready, or flush() breaks the blocking state too.
    if( !waitAllQueue() ){
        return MFALSE;
    }

    list<PipeRequestPtr> vToDoRequests;
    {
        Mutex::Autolock _l(mLock);

        PipeRequestPtr pipeRequest = nullptr;
        if( !mRequests.deque(pipeRequest) )
        {
            MY_LOGD("mRequests.deque() failed");
            return MFALSE;
        }

        this->incExtThreadDependency();

        addPendingRequests(pipeRequest);

        getReadyData(vToDoRequests);
    }

    CAM_TRACE_CALL();

    if(!vToDoRequests.empty()){
        MY_LOGD("data ready");
        doBss(vToDoRequests);
    }else{
        MY_LOGD("data NOT ready");
    }

    this->decExtThreadDependency();

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
addPendingRequests(PipeRequestPtr request)
{
    mvPendingRequests.push_back(request);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
getReadyData(list<PipeRequestPtr>& rvToDoRequests)
{
    PipeRequestPtr request = mvPendingRequests.front();

    auto frameNum = request->getParam(DualCamMFParamID::PID_FRAME_NUM);

    if(mvPendingRequests.size() < frameNum){
        MY_LOGD("(%d < %d) data not ready, keep waiting", mvPendingRequests.size(), frameNum);
    }else{
        for(int i=0 ; i < frameNum ; i++){
            if(mvPendingRequests.empty()){
                MY_LOGE("mvPendingRequests is empty!");
                return;
            }

            PipeRequestPtr request = mvPendingRequests.front();
            mvPendingRequests.pop_front();

            rvToDoRequests.push_back(request);
        }
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
retrieveGmvInfo(IMetadata* pMetadatadata, int& x, int& y, MSize& size) const
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
BssNode::GMV
BssNode::
calMotionVector(IMetadata* pMetadata, MBOOL isMain) const
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    BssNode::GMV        mv;
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
BssNode::
updateBssProcInfo(IImageBuffer* pBuf, BSS_PARAM_STRUCT& p, MINT32 frameNum) const
{
#if DCMF_TEMP == 1
    // temp do nothing
#else
    MSize srcSize(pBuf->getImgSize());

    MINT32 roiPercentage = MF_BSS_ROI_PERCENTAGE;
    MINT32 w = (srcSize.w * roiPercentage + 5) / 100;
    MINT32 h = (srcSize.h * roiPercentage + 5) / 100;
    MINT32 x = (srcSize.w - w) / 2;
    MINT32 y = (srcSize.h - h) / 2;

    #define MAKE_TAG(prefix, tag, id)   prefix##tag##id
    #define MAKE_TUPLE(tag, id)         std::make_tuple(#tag, id)
    #define DECLARE_BSS_ENUM_MAP()      std::map<std::tuple<const char*, int>, MUINT32> enumMap
    #define BUILD_BSS_ENUM_MAP(tag) \
            do { \
                enumMap[MAKE_TUPLE(tag, 0)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _00); \
                enumMap[MAKE_TUPLE(tag, 1)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _01); \
                enumMap[MAKE_TUPLE(tag, 2)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _02); \
                enumMap[MAKE_TUPLE(tag, 3)] = (MUINT32)MAKE_TAG(CUST_MFLL_BSS_, tag, _03); \
            } while (0)
    #define GET_CUST_MFLL_BSS(tag) \
            [&, this]() { \
                BUILD_BSS_ENUM_MAP(tag); \
                return enumMap[MAKE_TUPLE(tag, miOpenId)]; \
            }();

    DECLARE_BSS_ENUM_MAP();

    ::memset(&p, 0x00, sizeof(decltype(p)));

    String8 str = String8::format("\n======= updateBssProcInfo start ======\n");
    p.BSS_ON            = MF_BSS_ON;
    p.BSS_VER           = MF_BSS_VER;
    p.BSS_ROI_WIDTH     = w;
    p.BSS_ROI_HEIGHT    = h;
    p.BSS_ROI_X0        = x;
    p.BSS_ROI_Y0        = y;
    p.BSS_SCALE_FACTOR  = GET_CUST_MFLL_BSS(SCALE_FACTOR);

    p.BSS_CLIP_TH0      = GET_CUST_MFLL_BSS(CLIP_TH0);
    p.BSS_CLIP_TH1      = GET_CUST_MFLL_BSS(CLIP_TH1);
    p.BSS_CLIP_TH2      = GET_CUST_MFLL_BSS(CLIP_TH2);
    p.BSS_CLIP_TH3      = GET_CUST_MFLL_BSS(CLIP_TH3);

    p.BSS_ZERO          = GET_CUST_MFLL_BSS(ZERO);
    p.BSS_FRAME_NUM     = frameNum;
    p.BSS_ADF_TH        = GET_CUST_MFLL_BSS(ADF_TH);
    p.BSS_SDF_TH        = GET_CUST_MFLL_BSS(SDF_TH);

    p.BSS_GAIN_TH0      = GET_CUST_MFLL_BSS(GAIN_TH0);
    p.BSS_GAIN_TH1      = GET_CUST_MFLL_BSS(GAIN_TH1);
    p.BSS_MIN_ISP_GAIN  = GET_CUST_MFLL_BSS(MIN_ISP_GAIN);
    p.BSS_LCSO_SIZE     = 0; // TODO: query lcso size for AE compensation

    p.BSS_YPF_EN        = 0; // TODO: for AE compensation
    p.BSS_YPF_FAC       = GET_CUST_MFLL_BSS(YPF_FAC);
    p.BSS_YPF_ADJTH     = GET_CUST_MFLL_BSS(YPF_ADJTH);
    p.BSS_YPF_DFMED0    = GET_CUST_MFLL_BSS(YPF_DFMED0);
    p.BSS_YPF_DFMED1    = GET_CUST_MFLL_BSS(YPF_DFMED1);
    p.BSS_YPF_TH0       = GET_CUST_MFLL_BSS(YPF_TH0);
    p.BSS_YPF_TH1       = GET_CUST_MFLL_BSS(YPF_TH1);
    p.BSS_YPF_TH2       = GET_CUST_MFLL_BSS(YPF_TH2);
    p.BSS_YPF_TH3       = GET_CUST_MFLL_BSS(YPF_TH3);
    p.BSS_YPF_TH4       = GET_CUST_MFLL_BSS(YPF_TH4);
    p.BSS_YPF_TH5       = GET_CUST_MFLL_BSS(YPF_TH5);
    p.BSS_YPF_TH6       = GET_CUST_MFLL_BSS(YPF_TH6);
    p.BSS_YPF_TH7       = GET_CUST_MFLL_BSS(YPF_TH7);

    p.BSS_FD_EN         = 0; // TODO: for face detection info
    p.BSS_FD_FAC        = GET_CUST_MFLL_BSS(FD_FAC);
    p.BSS_FD_FNUM       = GET_CUST_MFLL_BSS(FD_FNUM);

    p.BSS_EYE_EN        = 0; // TODO: for eyes detection info
    p.BSS_EYE_CFTH      = GET_CUST_MFLL_BSS(EYE_CFTH);
    p.BSS_EYE_RATIO0    = GET_CUST_MFLL_BSS(EYE_RATIO0);
    p.BSS_EYE_RATIO1    = GET_CUST_MFLL_BSS(EYE_RATIO1);
    p.BSS_EYE_FAC       = GET_CUST_MFLL_BSS(EYE_FAC);

    str = str + String8::format("ON(%d) VER(%d) ROI(%d,%d, %dx%d) SCALE(%d)\n",
        p.BSS_ON, p.BSS_VER, p.BSS_ROI_X0, p.BSS_ROI_Y0, p.BSS_ROI_WIDTH, p.BSS_ROI_HEIGHT, p.BSS_SCALE_FACTOR
    );
    str = str + String8::format("CLIP(%d,%d,%d,%d)\n",
        p.BSS_CLIP_TH0, p.BSS_CLIP_TH1, p.BSS_CLIP_TH2, p.BSS_CLIP_TH3
    );
    str = str + String8::format("ZERO(%d) FRAME_NUM(%d) ADF_TH(%d) SDF_TH(%d)\n",
        p.BSS_ZERO, p.BSS_FRAME_NUM, p.BSS_ADF_TH, p.BSS_SDF_TH
    );
    str = str + String8::format("GAIN0(%d) GAIN1(%d) MIN_ISP_GAIN(%d) LCSO_SIZE(%d)\n",
        p.BSS_GAIN_TH0, p.BSS_GAIN_TH1, p.BSS_MIN_ISP_GAIN, p.BSS_LCSO_SIZE
    );
    str = str + String8::format("YPF: EN(%d) FAC(%d) ADJTH(%d) DFMED0(%d) DFMED1(%d)\n",
        p.BSS_YPF_EN, p.BSS_YPF_FAC, p.BSS_YPF_ADJTH, p.BSS_YPF_DFMED0, p.BSS_YPF_DFMED1
    );
    str = str + String8::format("YPF: TH(%d,%d,%d,%d,%d,%d,%d,%d)\n",
        p.BSS_YPF_TH0, p.BSS_YPF_TH1, p.BSS_YPF_TH2, p.BSS_YPF_TH3,
        p.BSS_YPF_TH4, p.BSS_YPF_TH5, p.BSS_YPF_TH6, p.BSS_YPF_TH7
    );
    str = str + String8::format("FD_EN(%d) BSS_FD_FAC(%d) BSS_FD_FNUM(%d)\n",
        p.BSS_FD_EN, p.BSS_FD_FAC, p.BSS_FD_FNUM
    );
    str = str + String8::format("EYE: EN(%d) CFTH(%d) RATIO0(%d) RATIO1(%d) FAC(%d)\n",
        p.BSS_EYE_EN, p.BSS_EYE_CFTH, p.BSS_EYE_RATIO0, p.BSS_EYE_RATIO1, p.BSS_EYE_FAC
    );
    str = str + String8::format("======= updateBssProcInfo end ======\n");
    MY_LOGD("%s", str.string());
#endif
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
updateBssIOInfo(IImageBuffer* pBuf, BSS_INPUT_DATA_G& bss_input) const
{
    memset(&bss_input, 0, sizeof(bss_input));

    IHalSensorList* sensorList = MAKE_HalSensorList();
    if(sensorList == NULL){
        MY_LOGE("get sensor list failed");
        return;
    }else{
        int sensorDev = sensorList->querySensorDevIdx(miOpenId);

        SensorStaticInfo sensorStaticInfo;
        sensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

        bss_input.BayerOrder = sensorStaticInfo.sensorFormatOrder;
        bss_input.Bitnum = [&, this]() {
            switch (sensorStaticInfo.rawSensorBit) {
                case RAW_SENSOR_8BIT:   return 8;
                case RAW_SENSOR_10BIT:  return 10;
                case RAW_SENSOR_12BIT:  return 12;
                case RAW_SENSOR_14BIT:  return 14;
                default:
                    MY_LOGE("get sensor raw bitnum failed");
                    return 0xFF;
            }
        }();
    }

    bss_input.Stride = pBuf->getBufStridesInBytes(0);
    bss_input.inWidth = pBuf->getImgSize().w;
    bss_input.inHeight = pBuf->getImgSize().h;

    String8 str = String8::format("\n======= updateBssIOInfo start ======\n");
    str = str + String8::format("BayerOrder(%d) Bitnum(%d) Stride(%d) Size(%dx%d)\n",
        bss_input.BayerOrder, bss_input.Bitnum, bss_input.Stride,
        bss_input.inWidth, bss_input.inHeight
    );

    // TOCO:
    // add Fd info

    str = str + String8::format("======= updateBssIOInfo end ======\n");
    MY_LOGD("%s", str.string());
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
appendBSSInput(MINT32 idx, PipeRequestPtr& request, BSS_INPUT_DATA_G& bss_input) const
{
    MY_LOGD("idx(%d) reqNo(%d)", idx, request->getRequestNo());

    IMetadata* pHalMeta = request->getMetadata(DualCamMFBufferID::BID_META_IN_HAL);
    BssNode::GMV mv = calMotionVector(
        pHalMeta,
        (idx == 0) ? MTRUE : MFALSE
    );
    bss_input.gmv[idx].x = mv.x;
    bss_input.gmv[idx].y = mv.y;

    sp<IImageBuffer> pBuf_rs_raw = request->getImageBuffer(DualCamMFBufferID::BID_INPUT_RSRAW_1);
    CHECK_OBJECT(pBuf_rs_raw);

    bss_input.apbyBssInImg[idx] = (MUINT8*)pBuf_rs_raw->getBufVA(0);

    MY_LOGD("gmv(%d,%d) pBuf(%p)",
        bss_input.gmv[idx].x, bss_input.gmv[idx].y,
        bss_input.apbyBssInImg[idx]
    );

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
doBss(list<PipeRequestPtr>& rvToDoRequests)
{
#if DCMF_TEMP == 1
    // temp do nothing
#else
    MINT32 const mainReqId = rvToDoRequests.front()->getRequestNo();

    CAM_TRACE_FMT_BEGIN("doBss req(%d)", mainReqId);
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    MET_START(DO_BSS);

    MTKBss* pBssDrv = nullptr;

    MINT32 frameNum = rvToDoRequests.size();

    handleData(SHUTTER, rvToDoRequests.front());

    // enable mfb or not
    MINT32 enableMfb = rvToDoRequests.front()->getParam(DualCamMFParamID::PID_ENABLE_MFB);

    if( enableMfb != 1 ){
        while( !rvToDoRequests.empty() ){
            handleByPass(rvToDoRequests.front());
            rvToDoRequests.pop_front();
        }
        return MTRUE;
    }

    sp<IStopWatchCollection> stopWatchCollection = rvToDoRequests.front()->getStopWatchCollection();
    sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_bss:bss_all");

    {
        sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_bss:bss_createInstance");
        pBssDrv = MTKBss::createInstance(DRV_BSS_OBJ_SW);
        if(pBssDrv == nullptr){
            MY_LOGE("BSS alg create failed!");
            return MFALSE;
        }
    }

    {
        sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_bss:bss_init");
        pBssDrv->BssInit(nullptr, nullptr);
    }

    // main frame's input, rrzo
    sp<IImageBuffer> pBuf_rs_raw_1 = rvToDoRequests.front()->getImageBuffer(DualCamMFBufferID::BID_INPUT_RSRAW_1);
    CHECK_OBJECT(pBuf_rs_raw_1);

    BSS_PARAM_STRUCT bss_param;
    BSS_WB_STRUCT workingBufferInfo;
    std::unique_ptr<MUINT8[]> bss_working_buffer;

    {
        sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_bss:bss_working");

        workingBufferInfo.rProcId    = BSS_PROC2;
        workingBufferInfo.u4Width    = pBuf_rs_raw_1->getImgSize().w;
        workingBufferInfo.u4Height   = pBuf_rs_raw_1->getImgSize().h;
        workingBufferInfo.u4FrameNum = frameNum;
        workingBufferInfo.u4WKSize   = 0; //it will return working buffer require size
        workingBufferInfo.pu1BW      = nullptr; // assign working buffer latrer.

        auto b = pBssDrv->BssFeatureCtrl(BSS_FTCTRL_GET_WB_SIZE, (void*)&workingBufferInfo, NULL);
        if (b != S_BSS_OK) {
            MY_LOGE("get working buffer size from MTKBss failed (%d)", (int)b);
            return MFALSE;
        }
        if (workingBufferInfo.u4WKSize <= 0) {
            MY_LOGE("unexpected bss working buffer size: %u", workingBufferInfo.u4WKSize);
            return MFALSE;
        }

        bss_working_buffer = std::unique_ptr<MUINT8[]>(new MUINT8[workingBufferInfo.u4WKSize]{0});
        workingBufferInfo.pu1BW = bss_working_buffer.get(); // assign working buffer for bss algo.

        b = pBssDrv->BssFeatureCtrl(BSS_FTCTRL_SET_WB_SIZE, (void*)&workingBufferInfo, NULL);
        if (b != S_BSS_OK) {
            MY_LOGE("set working buffer to MTKBss failed, size=%d (%u)",(int)b, workingBufferInfo.u4WKSize);
            return MFALSE;
        }
    }

    {
        sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_bss:bss_set_param");

        updateBssProcInfo(pBuf_rs_raw_1.get(), bss_param, frameNum);

        auto b = pBssDrv->BssFeatureCtrl(BSS_FTCTRL_SET_PROC_INFO, (void*)&bss_param, NULL);
        if (b != S_BSS_OK) {
            MY_LOGE("et info to MTKBss failed (%d)", (int)b);
            return MFALSE;
        }
    }

    list<PipeRequestPtr> bssOrderedRequests;
    {
        sp<IStopWatch> stopWatch = stopWatchCollection->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_bss:bss_main");

        BSS_INPUT_DATA_G inParam;
        BSS_OUTPUT_DATA outParam;

        memset(&inParam, 0, sizeof(inParam));
        memset(&outParam, 0, sizeof(outParam));

        updateBssIOInfo(pBuf_rs_raw_1.get(), inParam);

        MY_LOGD("appendBSSInput for (%d) frames", rvToDoRequests.size());
        MINT32 i = 0;
        std::unordered_map<MINT32, PipeRequestPtr> requestsMap;
        for(auto &e : rvToDoRequests){
            if( !appendBSSInput(i, e, inParam) ){
                MY_LOGE("appendBSSInput failed!");
                return MFALSE;
            }
            requestsMap[i] = e;
            i++;
        }

        collectPreBSSExifData(rvToDoRequests, bss_param, inParam);

        auto b = pBssDrv->BssMain(BSS_PROC2, &inParam, &outParam);
        if (b != S_BSS_OK) {
            MY_LOGE("MTKBss::Main returns failed (%d)", (int)b);
            return MFALSE;
        }

        for(int resultIdx = 0 ; resultIdx < rvToDoRequests.size() ; resultIdx ++){
            if(requestsMap.count(outParam.originalOrder[resultIdx]) == 0){
                MY_LOGE("%d not belong to input requests?", outParam.originalOrder[resultIdx]);
                return MFALSE;
            }

            bssOrderedRequests.push_back(requestsMap.at(outParam.originalOrder[resultIdx]));
        }

        Vector<MINT32> newIndex;
        for (size_t i = 0; i < rvToDoRequests.size(); i++) {
            newIndex.push_back(outParam.originalOrder[i]);
        }
        collectPostBSSExifData(mainReqId, newIndex);

        // push Bss result into hal meta, all request have same information:
        // 1. MTK_STEREO_FRAME_PER_CAPTURE
        // 2. MTK_STEREO_BSS_RESULT: the result info set containing 3 integer:
        //                           1.original order
        //                           2.gmv.x
        //                           3.gmv.y
        for(auto &e : rvToDoRequests){
            IMetadata* pHalMeta = e->getMetadata(DualCamMFBufferID::BID_META_OUT_HAL);
            CHECK_OBJECT(pHalMeta);

            // frame per capture
            IMetadata::IEntry entry(MTK_STEREO_FRAME_PER_CAPTURE);
            entry.push_back(bssOrderedRequests.size(), Type2Type<MINT32>());
            pHalMeta->update(entry.tag(), entry);

            // bss result
            IMetadata::IEntry entry2(MTK_STEREO_BSS_RESULT);
            for(int bssIdx=0 ; bssIdx<bssOrderedRequests.size() ; bssIdx++){
                entry2.push_back(outParam.originalOrder[bssIdx], Type2Type<MINT32>());
                entry2.push_back(outParam.gmv[bssIdx].x, Type2Type<MINT32>());
                entry2.push_back(outParam.gmv[bssIdx].y, Type2Type<MINT32>());
            }
            pHalMeta->update(entry2.tag(), entry2);

            // enable mfb, must be 1
            IMetadata::IEntry entry3(MTK_STEREO_ENABLE_MFB);
            entry3.push_back(1, Type2Type<MINT32>());
            pHalMeta->update(entry3.tag(), entry3);

            // update feature mode to mfnr+bokeh
            IMetadata::IEntry entry4(MTK_STEREO_DCMF_FEATURE_MODE);
            entry4.push_back(MTK_DCMF_FEATURE_MFNR_BOKEH, Type2Type<MINT32>());
            pHalMeta->update(entry4.tag(), entry4);
        }
    }

    pBssDrv->destroyInstance();

    handleFinish(rvToDoRequests, bssOrderedRequests);

    CAM_TRACE_FMT_END();
    MET_END(DO_BSS);
#endif
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
collectPreBSSExifData(
    list<PipeRequestPtr>& rvToDoRequests,
    BSS_PARAM_STRUCT& p,
    BSS_INPUT_DATA_G& bss_input) const
{
#if DCMF_TEMP == 1
    // temp do nothing
#else
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    MINT32 reqId = rvToDoRequests.front()->getRequestNo();

    ExifWriter writer(PIPE_LOG_TAG);

    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_ON                 ,(uint32_t)p.BSS_ON             );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_VER                ,(uint32_t)p.BSS_VER            );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_FRAME_NUM          ,(uint32_t)rvToDoRequests.size());
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_GAIN_TH0           ,(uint32_t)p.BSS_GAIN_TH0       );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_GAIN_TH1           ,(uint32_t)p.BSS_GAIN_TH1       );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_MIN_ISP_GAIN       ,(uint32_t)p.BSS_MIN_ISP_GAIN   );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_LCSO_SIZE          ,(uint32_t)p.BSS_LCSO_SIZE      );
    /* YPF info */
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_EN             ,(uint32_t)p.BSS_YPF_EN         );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_FAC            ,(uint32_t)p.BSS_YPF_FAC        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_ADJTH          ,(uint32_t)p.BSS_YPF_ADJTH      );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_DFMED0         ,(uint32_t)p.BSS_YPF_DFMED0     );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_DFMED1         ,(uint32_t)p.BSS_YPF_DFMED1     );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH0            ,(uint32_t)p.BSS_YPF_TH0        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH1            ,(uint32_t)p.BSS_YPF_TH1        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH2            ,(uint32_t)p.BSS_YPF_TH2        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH3            ,(uint32_t)p.BSS_YPF_TH3        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH4            ,(uint32_t)p.BSS_YPF_TH4        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH5            ,(uint32_t)p.BSS_YPF_TH5        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH6            ,(uint32_t)p.BSS_YPF_TH6        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_YPF_TH7            ,(uint32_t)p.BSS_YPF_TH7        );
    /* FD & eye info*/
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_FD_EN              ,(uint32_t)p.BSS_FD_EN          );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_FD_FAC             ,(uint32_t)p.BSS_FD_FAC         );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_FD_FNUM            ,(uint32_t)p.BSS_FD_FNUM        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_EYE_EN             ,(uint32_t)p.BSS_EYE_EN         );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_EYE_CFTH           ,(uint32_t)p.BSS_EYE_CFTH       );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_EYE_RATIO0         ,(uint32_t)p.BSS_EYE_RATIO0     );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_EYE_RATIO1         ,(uint32_t)p.BSS_EYE_RATIO1     );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_EYE_FAC            ,(uint32_t)p.BSS_EYE_FAC        );
    writer.sendData(reqId, (unsigned int)MF_TAG_BSS_AEVC_EN            ,(uint32_t)p.BSS_AEVC_EN        );


    auto makeGmv32bits = [](short x, short y){
        return (uint32_t) y << 16 | (x & 0x0000FFFF);
    };
    for(auto i=0 ; i<rvToDoRequests.size() ; i++){
        if(i >= MAX_GMV_CNT){
            MY_LOGE("gmv count exceeds limitatin(%d)!", MAX_GMV_CNT);
            break;
        }
        writer.sendData(reqId,
            (unsigned int)MF_TAG_GMV_00 + i,
            (uint32_t)makeGmv32bits((short)bss_input.gmv[i].x, (short)bss_input.gmv[i].y)
        );
    }

    // set all sub requests belong to main request
    set<MINT32> mappingSet;
    for(auto &e : rvToDoRequests){
        mappingSet.insert(e->getRequestNo());
    }
    writer.addReqMapping(reqId, mappingSet);
#endif
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
collectPostBSSExifData(MINT32 reqId, Vector<MINT32>& vNewIndex)
{
#if DCMF_TEMP == 1
    // temp do nothing
#else
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    ExifWriter writer(PIPE_LOG_TAG);

    // bss order
    {
        // encoding for bss order
        /** MF_TAG_BSS_ORDER_IDX
         *
         *  BSS order for top 8 frames (MSB -> LSB)
         *
         *  |     4       |     4       |     4       |     4       |     4       |     4       |     4       |     4       |
         *  | bssOrder[0] | bssOrder[1] | bssOrder[2] | bssOrder[3] | bssOrder[4] | bssOrder[5] | bssOrder[6] | bssOrder[7] |
         */

        uint32_t bssOrder = 0x0;
        size_t i = 0;

        for ( ; i < vNewIndex.size() && i < 8 ; i++)
            bssOrder = (bssOrder << 4) | ((uint32_t)vNewIndex[i]<0xf?(uint32_t)vNewIndex[i]:0xf);
        for ( ; i < 8 ; i++)
            bssOrder = (bssOrder << 4) | 0xf;

        writer.sendData(reqId, (unsigned int)MF_TAG_BSS_ORDER_IDX           ,bssOrder                     );
        writer.sendData(reqId, (unsigned int)MF_TAG_BSS_BEST_IDX            ,(uint32_t)vNewIndex[0]        );
    }
#endif
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
handleFinish(
    list<PipeRequestPtr>& rvRequest, list<PipeRequestPtr>& rvBssOrderedRequests)
{
    if(rvRequest.size() != rvBssOrderedRequests.size()){
        MY_LOGE("input(%d) != result(%d)", rvRequest.size(), rvBssOrderedRequests.size());
        return;
    }

    // handle data to next node whihin 2 kinds of sequence ordering
    while( !rvRequest.empty() && !rvBssOrderedRequests.empty() ){
        PipeRequestPtr request = rvRequest.front();
        rvRequest.pop_front();

        CAM_TRACE_FMT_BEGIN("BssNode::handleFinish req(%d)", request->getRequestNo());

        PipeRequestPtr orderedRequest = rvBssOrderedRequests.front();
        rvBssOrderedRequests.pop_front();

        MY_LOGD("handle request orig(%d) ordered(%d)", request->getRequestNo(), orderedRequest->getRequestNo());
        handleData(BSS_RESULT_ORIGIN, request);
        handleData(BSS_RESULT_ORDERED, orderedRequest);

        CAM_TRACE_FMT_END();
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
handleByPass(
    PipeRequestPtr request)
{
    CAM_TRACE_CALL();

    MY_LOGD("no need mfb, by pass bss node");

    // handle data to next node whihin 2 kinds of sequence ordering
    MY_LOGD("handle request orig(%d) ordered(%d)", request->getRequestNo(), request->getRequestNo());
    handleData(BSS_RESULT_ORIGIN, request);
    handleData(BSS_RESULT_ORDERED, request);
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
BssNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    Vector<NSDCMF::BufferConfig> vBufConfig;

    // alloc working buffer if needed
    // int allocateSize = 1;
    // {
    //     NSBMDN::BufferConfig c = {
    //         "BID_XXX",
    //         BID_XXX,
    //         (MUINT32)some_width,
    //         (MUINT32)some_height,
    //         eImgFmt_XXX,
    //         ImageBufferPool::USAGE_HW,
    //         MTRUE,
    //         MFALSE,
    //         (MUINT32)allocateSize
    //     };
    //     vBufConfig.push_back(c);
    // }

    if(!mBufPool.init(vBufConfig)){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
BssNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}
