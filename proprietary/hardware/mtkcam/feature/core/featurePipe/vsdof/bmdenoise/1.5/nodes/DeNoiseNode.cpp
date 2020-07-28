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
#include "DeNoiseNode.h"

#define PIPE_MODULE_TAG "BMDeNoise"
#define PIPE_CLASS_TAG "DeNoiseNode"
#define PIPE_LOG_TAG PIPE_MODULE_TAG PIPE_CLASS_TAG

// MET tags
#define DO_BM_DENOISE "doBMDeNoise"

// buffer alloc size
#define TUNING_ALLOC_SIZE 2
#define SHADING_GAIN_SIZE 200

// debug settings
#define USE_DEFAULT_ISP 0
#define USE_DEFAULT_SHADING_GAIN 0
#define USE_DEFAULT_AMATRIX 0

#define DEFAULT_DYNAMIC_SHADING 32

#define GRAPHIC_BUF__ALLOC_RATIO 2

#define SCENEINFO_CNT 15

// 2'comp, using ASL and ASR to do sign extension
#define TO_INT(a, bit) ((MINT32)((MINT32) (a)) << (32-(bit)) >> (32-(bit)))
#define OB_TO_INT(a) ((-(TO_INT((a), 13)))/4) /*10bit*/

#include <PipeLog.h>

#include <DpBlitStream.h>
#include "../exif/ExifWriter.h"

#include <camera_custom_nvram.h>
#include <mtkcam/feature/stereo/hal/stereo_common.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_idx.h>
#include <isp_tuning_custom.h>
#include <n3d_sync2a_tuning_param.h>
#include <lsc/ILscTbl.h>
#include <drv/isp_reg.h>

using namespace NSCam::NSCamFeature::NSFeaturePipe;
using namespace NS3Av3;
using namespace NSIoPipe;

/*******************************************************************************
 *
 ********************************************************************************/
DeNoiseNode::
DeNoiseNode(const char *name,
    Graph_T *graph,
    MINT32 openId)
    : BMDeNoisePipeNode(name, graph)
    , miOpenId(openId)
    , mBufPool(name)
{
    MY_LOGD("ctor(0x%x)", this);
    this->addWaitQueue(&mImgInfoRequests_PreProcess);
    this->addWaitQueue(&mImgInfoRequests_Depth);
    this->addWaitQueue(&mRequests);

    miDumpShadingGain = ::property_get_int32("vendor.bmdenoise.pipe.dump.shading", 0);

    mDebugDsH = ::property_get_int32("vendor.debug.bmdenoise.dsH", -1);
    mDebugDsV= ::property_get_int32("vendor.debug.bmdenoise.dsV", -1);
    mDebugPerformanceQualityOption.FPREPROC = ::property_get_int32("vendor.debug.bmdenoise.FPREPROC", -1);
    mDebugPerformanceQualityOption.FSSTEP = ::property_get_int32("vendor.debug.bmdenoise.FSSTEP", -1);
    mDebugPerformanceQualityOption.DblkRto = ::property_get_int32("vendor.debug.bmdenoise.DblkRto", -1);
    mDebugPerformanceQualityOption.DblkTH  = ::property_get_int32("vendor.debug.bmdenoise.DblkTH", -1);
}
/*******************************************************************************
 *
 ********************************************************************************/
DeNoiseNode::
~DeNoiseNode()
{
    MY_LOGD("dctor(0x%x)", this);
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
onData(
    DataID id,
    PipeRequestPtr &request)
{
    MY_LOGD("onData(request) +");
    Mutex::Autolock _l(mLock);

    MBOOL ret = MFALSE;
    switch(id)
    {
        case ROOT_TO_DENOISE:
            mRequests.enque(request);
            ret = MTRUE;
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unknown data id :%d", id);
            break;
    }

    MY_LOGD("onData(request) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
onData(
    DataID id,
    ImgInfoMapPtr& pImgInfo)
{
    MY_LOGD("onData(ImgInfo) +");
    Mutex::Autolock _l(mLock);

    MBOOL ret = MFALSE;
    switch(id)
    {
        case PREPROCESS_TO_DENOISE:
            mImgInfoRequests_PreProcess.enque(pImgInfo);
            ret = MTRUE;
            break;
        case DPE_TO_DENOISE_DMP_RESULT:
            mImgInfoRequests_Depth.enque(pImgInfo);
            ret = MTRUE;
            break;
        default:
            MY_LOGE("onData(ImgInfoMapPtr) unknown data id :%d", id);
            ret = MFALSE;
            break;
    }

    if(!mRequests.empty() && !mImgInfoRequests_PreProcess.empty() && !mImgInfoRequests_Depth.empty()){
        if(mRequests.size() == mImgInfoRequests_PreProcess.size() == mImgInfoRequests_Depth.size()){
            if(!shouldDumpRequest(pImgInfo->getRequestPtr())){
                MY_LOGD("P2_DONE for reqNo:%d phase1", pImgInfo->getRequestPtr()->getRequestNo());
                handleData(P2_DONE,       pImgInfo->getRequestPtr());
            }
        }
    }

    MY_LOGD("onData(ImgInfo) -");
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
onInit()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(!BMDeNoisePipeNode::onInit()){
        MY_LOGE("BMDeNoisePipeNode::onInit() failed!");
        return MFALSE;
    }

    StereoTuningProvider::getDPETuningInfo(&mDVEConig);

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
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
DeNoiseNode::
cleanUp()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    if(mp3AHal_Main1 != nullptr)
    {
        mp3AHal_Main1->destroyInstance("BMDENOISE_3A_MAIN1");
        mp3AHal_Main1 = nullptr;
    }
    if(mp3AHal_Main2 != nullptr)
    {
        mp3AHal_Main2->destroyInstance("BMDENOISE_3A_MAIN2");
        mp3AHal_Main2 = nullptr;
    }
    if(mpBWDNHAL != nullptr){
        MY_LOGE("should call releaseALG() by BMDNPipe to ensure the relese order!");
    }
    mBufPool.uninit();
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DeNoiseNode::
collectExifData(map<MINT32, MINT32>& exifData, BWDN_HAL_PARAMS &rInData, MINT32* pSceneInfo)
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    if(pSceneInfo != nullptr)
    {
        for(int i=0; i<SCENEINFO_CNT; i++)
        {
            MY_LOGD("SceneInfo[%d]=%d", i , pSceneInfo[i]);
            exifData[BM_TAG_SCENE_INFO1 + i] = pSceneInfo[i];
        }
    }
    else
    {
        MY_LOGW("no SceneInfo!");
    }

    exifData[BM_TAG_DENOISE_VERSION] = 2;
    exifData[BM_TAG_DENOISE_TYPE] = DN_TYPE_NORMAL;
    exifData[BM_TAG_DENOISE_COMPOSITION] = COMP_BM_DENOISE_AND_SWNR;

    std::vector<int> cpu_core = get_stereo_cam_cpu_num(ENUM_STEREO_CAM_FEATURE::eStereoBMDnoise, ENUM_STEREO_CAM_SCENARIO::eStereoCamCapture);
    exifData[BM_TAG_DENOISE_CORENUM] = cpu_core.size();
    if(StereoSettingProvider::getModuleRotation() == eRotate_90 || StereoSettingProvider::getModuleRotation() == eRotate_270){
        exifData[BM_TAG_DENOISE_SPLITSIZE] = 336;
        exifData[BM_TAG_DENOISE_ISROTATE] = 1;
    }else{
        exifData[BM_TAG_DENOISE_SPLITSIZE] = 256;
        exifData[BM_TAG_DENOISE_ISROTATE] = 0;
    }
    const strSyncAEInitInfo* pSyncAEInitInfo = getSyncAEInitInfo();
    float tempB2W0 = pSyncAEInitInfo->RGB2YCoef_main[0];
    float tempB2W1 = pSyncAEInitInfo->RGB2YCoef_main[1];
    float tempB2W2 = pSyncAEInitInfo->RGB2YCoef_main[2];

    exifData[BM_TAG_DENOISE_B2W0] = *reinterpret_cast<MINT32*>(&tempB2W0);
    exifData[BM_TAG_DENOISE_B2W1] = *reinterpret_cast<MINT32*>(&tempB2W1);
    exifData[BM_TAG_DENOISE_B2W2] = *reinterpret_cast<MINT32*>(&tempB2W2);

    exifData[BM_TAG_DENOISE_BORDER] = rInData.bayerOrder;

    exifData[BM_TAG_DENOISE_BOBOFST] = rInData.OBOffsetBayer[0];
    exifData[BM_TAG_DENOISE_MOBOFST] = rInData.OBOffsetMono[0];

    exifData[BM_TAG_DENOISE_PGN1] = rInData.preGainBayer[0];
    exifData[BM_TAG_DENOISE_PGN2] = rInData.preGainBayer[1];
    exifData[BM_TAG_DENOISE_PGN3] = rInData.preGainBayer[2];

    exifData[BM_TAG_DENOISE_BSENSORGAIN] = rInData.sensorGainBayer;
    exifData[BM_TAG_DENOISE_MSENSORGAIN] = rInData.sensorGainMono;

    exifData[BM_TAG_DENOISE_BISPGAIN] = rInData.ispGainBayer;
    exifData[BM_TAG_DENOISE_MISPGAIN] = rInData.ispGainMono;

    exifData[BM_TAG_DENOISE_RA] = rInData.RA;

    exifData[BM_TAG_DENOISE_BMSINGLERANGE] = rInData.BW_SingleRange;
    exifData[BM_TAG_DENOISE_BMOCCRANGE] = rInData.BW_OccRange;
    exifData[BM_TAG_DENOISE_BMRANGE] = rInData.BW_Range;
    exifData[BM_TAG_DENOISE_BMKERNEL] = rInData.BW_Kernel;
    exifData[BM_TAG_DENOISE_BRANGE] = rInData.B_Range;
    exifData[BM_TAG_DENOISE_BKERNEL] = rInData.B_Kernel;
    exifData[BM_TAG_DENOISE_WRANGE] = rInData.W_Range;
    exifData[BM_TAG_DENOISE_WKERNEL] = rInData.W_Kernel;
    exifData[BM_TAG_DENOISE_VSCALE] = rInData.VSL;
    exifData[BM_TAG_DENOISE_VOFST] = rInData.VOFT;
    exifData[BM_TAG_DENOISE_VGAIN] = rInData.VGAIN;

    //Float to int
    exifData[BM_TAG_DENOISE_AMATRIX1] = *reinterpret_cast<MINT32*>(&rInData.Trans[0]);
    exifData[BM_TAG_DENOISE_AMATRIX2] = *reinterpret_cast<MINT32*>(&rInData.Trans[1]);
    exifData[BM_TAG_DENOISE_AMATRIX3] = *reinterpret_cast<MINT32*>(&rInData.Trans[2]);
    exifData[BM_TAG_DENOISE_AMATRIX4] = *reinterpret_cast<MINT32*>(&rInData.Trans[3]);
    exifData[BM_TAG_DENOISE_AMATRIX5] = *reinterpret_cast<MINT32*>(&rInData.Trans[4]);
    exifData[BM_TAG_DENOISE_AMATRIX6] = *reinterpret_cast<MINT32*>(&rInData.Trans[5]);
    exifData[BM_TAG_DENOISE_AMATRIX7] = *reinterpret_cast<MINT32*>(&rInData.Trans[6]);
    exifData[BM_TAG_DENOISE_AMATRIX8] = *reinterpret_cast<MINT32*>(&rInData.Trans[7]);
    exifData[BM_TAG_DENOISE_AMATRIX9] = *reinterpret_cast<MINT32*>(&rInData.Trans[8]);

    exifData[BM_TAG_DENOISE_WPADDING1] = rInData.dPadding[0];
    exifData[BM_TAG_DENOISE_WPADDING2] = rInData.dPadding[1];

    exifData[BM_TAG_DENOISE_OPT_FPREPROC] = rInData.FPREPROC;
    exifData[BM_TAG_DENOISE_OPT_FSSTEP] = rInData.FSSTEP;

    exifData[BM_TAG_DENOISE_OPT_DBLKRTO] = rInData.DblkRto;
    exifData[BM_TAG_DENOISE_OPT_DBLKTH]  = rInData.DblkTH;

    exifData[BM_TAG_DENOISE_OPT_QSEARCH] = rInData.dsV;
    exifData[BM_TAG_DENOISE_OPT_BITMODE] = rInData.BitMode;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
onThreadStart()
{
    CAM_TRACE_CALL();
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    mp3AHal_Main1 = MAKE_Hal3A(mSensorIdx_Main1, "BMDENOISE_3A_MAIN1");
    mp3AHal_Main2 = MAKE_Hal3A(mSensorIdx_Main2, "BMDENOISE_3A_MAIN2");
    MY_LOGD("3A create instance, Main1: %x, Main2: %x", mp3AHal_Main1, mp3AHal_Main2);

    // query some sensor static info
    {
        IHalSensorList* sensorList = MAKE_HalSensorList();
        int sensorDev_Main1 = sensorList->querySensorDevIdx(mSensorIdx_Main1);

        SensorStaticInfo sensorStaticInfo;
        memset(&sensorStaticInfo, 0, sizeof(SensorStaticInfo));
        sensorList->querySensorStaticInfo(sensorDev_Main1, &sensorStaticInfo);

        mBayerOrder_main1 = sensorStaticInfo.sensorFormatOrder;
    }

    {
        AutoProfileUtil proflie(PIPE_LOG_TAG, "BWDN createInstance");

        // should be moved to alg caller
        if( ::property_get_int32("vendor.bwdn.dump.io", -1) == 1){
            char filepathAlg[1024];
            snprintf(filepathAlg, 1024, "/sdcard/bmdenoise/CModelData");
            VSDOF_LOGD("makePath: %s", filepathAlg);
            makePath(filepathAlg, 0660);
        }

        // init alg
        if(::property_get_int32("vendor.bmdenoise.pipe.bwdn", 1) == 1){
            mpBWDNHAL = BWDN_HAL::createInstance();
            if(mpBWDNHAL == nullptr){
                MY_LOGE("failed creating BWDN instance");
                return MFALSE;
            }
        }else{
            MY_LOGD("by-pass BWDN ALG init");
        }
    }

    initBufferPool();

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
onThreadStop()
{
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
onThreadLoop()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    ImgInfoMapPtr ImgInfo_PreProcess = nullptr;
    ImgInfoMapPtr imgInfo_Depth = nullptr;
    PipeRequestPtr pipeRequest = nullptr;

    if( !waitAllQueue() )// block until queue ready, or flush() breaks the blocking state too.
    {
        return MFALSE;
    }

    {
        Mutex::Autolock _l(mLock);

        if( !mImgInfoRequests_PreProcess.deque(ImgInfo_PreProcess) )
        {
            MY_LOGD("mImgInfoRequests_PreProcess.deque() failed");
            return MFALSE;
        }
        if( !mImgInfoRequests_Depth.deque(imgInfo_Depth) )
        {
            MY_LOGD("mImgInfoRequests_Depth.deque() failed");
            return MFALSE;
        }
        if( !mRequests.deque(pipeRequest) )
        {
            MY_LOGD("mRequests.deque() failed");
            return MFALSE;
        }
    }

    CAM_TRACE_CALL();

    this->incExtThreadDependency();

    ImgInfoMapPtr imgInfo_BMDN = nullptr;
    {
        sp<IStopWatch> stopWatch = pipeRequest->getStopWatchCollection()->GetStopWatch(eStopWatchType::eSTOPWATCHTYPE_AUTO, "do_bm_denoise");
        imgInfo_BMDN = doBMDeNoise(ImgInfo_PreProcess, imgInfo_Depth, pipeRequest);
    }

    if(imgInfo_BMDN != nullptr){
        imgInfo_BMDN->addSmartBuffer(
            BID_PRE_PROCESS_OUT_YUV,
            ImgInfo_PreProcess->getSmartBuffer(BID_PRE_PROCESS_OUT_YUV)
        );
        imgInfo_BMDN->addSmartBuffer(
            BID_LCSO_1_BUF,
            ImgInfo_PreProcess->getSmartBuffer(BID_LCSO_1_BUF)
        );
        handleData(DENOISE_TO_POSTPROCESS, imgInfo_BMDN);
        handleData(DENOISE_TO_POSTPROCESS, pipeRequest);
        this->decExtThreadDependency();
    }else{
        MY_LOGE("failed doing bmdenoise, please check above errors!");
        this->decExtThreadDependency();
        return MFALSE;
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
ImgInfoMapPtr
DeNoiseNode::
doBMDeNoise(ImgInfoMapPtr imgInfo_PreProcess, ImgInfoMapPtr imgInfo_Depth, PipeRequestPtr request)
{
    auto getDisparityMapFromSensorRelativePosition = [this] (const ImgInfoMapPtr& imgInfo) -> SmartGraphicBuffer
    {
        ENUM_STEREO_SENSOR_RELATIVE_POSITION sensorRelativePosition = StereoSettingProvider::getSensorRelativePosition();
        if((sensorRelativePosition == STEREO_SENSOR_REAR_MAIN_TOP) || (sensorRelativePosition == STEREO_SENSOR_FRONT_MAIN_RIGHT))
        {
            MY_LOGD("get the disparity map L (%s)", (sensorRelativePosition == STEREO_SENSOR_REAR_MAIN_TOP)?"STEREO_SENSOR_REAR_MAIN_TOP":"STEREO_SENSOR_FRONT_MAIN_RIGHT");
            return imgInfo->getGraphicBuffer(BID_DPE_OUT_DMP_L);
        }
        else if((sensorRelativePosition == STEREO_SENSOR_REAR_MAIN_BOTTOM) || (sensorRelativePosition == STEREO_SENSOR_FRONT_MAIN_LEFT))
        {
            MY_LOGD("get the disparity map R (%s)", (sensorRelativePosition == STEREO_SENSOR_REAR_MAIN_BOTTOM)?"STEREO_SENSOR_REAR_MAIN_BOTTOM":"STEREO_SENSOR_FRONT_MAIN_LEFT");
            return imgInfo->getGraphicBuffer(BID_DPE_OUT_DMP_R);
        }
        else
        {
            MY_LOGE("unknow sensor relative position: %d", sensorRelativePosition);
            return nullptr;
        }
    };

    CAM_TRACE_CALL();
    MY_LOGD("+, reqID=%d", request->getRequestNo());
    AutoProfileUtil proflie(PIPE_LOG_TAG, __FUNCTION__);
    MET_START(DO_BM_DENOISE);

    handleData(ENTER_HIGH_PERF, request->getRequestNo());

    // preprare data
    // input
    SmartGraphicBuffer smgpBuf_w1 = imgInfo_PreProcess->getGraphicBuffer(BID_PRE_PROCESS_OUT_W_1);
    SmartGraphicBuffer smgpBuf_mfbo_1 = imgInfo_PreProcess->getGraphicBuffer(BID_PRE_PROCESS_OUT_MFBO_FINAL_1);
    SmartGraphicBuffer smgpBuf_mfbo_2 = imgInfo_PreProcess->getGraphicBuffer(BID_PRE_PROCESS_OUT_MFBO_FINAL_2);
    SmartGraphicBuffer smgpBuf_disparityMap = getDisparityMapFromSensorRelativePosition(imgInfo_Depth);

    SmartImageBuffer smpBuf_warpingMatrix = imgInfo_Depth->getSmartBuffer(BID_N3D_OUT_WARPING_MATRIX);

    IMetadata* pMeta_InHal_main1 = request->getMetadata(BID_META_IN_HAL);
    IMetadata* pMeta_InHal_main2 = request->getMetadata(BID_META_IN_HAL_MAIN2);
    IMetadata* pMeta_inApp = request->getMetadata(BID_META_IN_APP);

    SmartTuningBuffer tuningBuf_main1 = mBufPool.getTuningBufPool()->request();
    SmartTuningBuffer tuningBuf_main2 = mBufPool.getTuningBufPool()->request();

    TuningParam rTuningParam_main1;
    TuningParam rTuningParam_main2;
    dip_x_reg_t* dip_reg_main1 = nullptr;
    dip_x_reg_t* dip_reg_main2 = nullptr;

    // output
    SmartGraphicBuffer smgpBMDN_HAL_out = mBufPool.getGraphicBufPool(BID_DENOISE_HAL_OUT)->request();

    // prepare input params
    BWDN_HAL_PARAMS input;
    {
        ISPTuningConfig ispConfig = {pMeta_inApp, pMeta_InHal_main1, mp3AHal_Main1, MFALSE};
        rTuningParam_main1 = applyISPTuning(PIPE_LOG_TAG, tuningBuf_main1, ispConfig, USE_DEFAULT_ISP);
        dip_reg_main1 = (dip_x_reg_t*)(rTuningParam_main1.pRegBuf);
    }
    {
        ISPTuningConfig ispConfig = {pMeta_inApp, pMeta_InHal_main2, mp3AHal_Main2, MFALSE};
        rTuningParam_main2 = applyISPTuning(PIPE_LOG_TAG, tuningBuf_main2, ispConfig, USE_DEFAULT_ISP);
        dip_reg_main2 = (dip_x_reg_t*)(rTuningParam_main2.pRegBuf);
    }
    // rotate and byaer order
    std::unique_ptr<ILscTable> outGain_main1(MAKE_LscTable(ILscTable::GAIN_FLOAT));
    std::unique_ptr<ILscTable> outGain_main2(MAKE_LscTable(ILscTable::GAIN_FLOAT));

    // OBOffset
    input.OBOffsetBayer[0] = OB_TO_INT(dip_reg_main1->DIP_X_OBC2_OFFST0.Raw);
    input.OBOffsetBayer[1] = OB_TO_INT(dip_reg_main1->DIP_X_OBC2_OFFST1.Raw);
    input.OBOffsetBayer[2] = OB_TO_INT(dip_reg_main1->DIP_X_OBC2_OFFST2.Raw);
    input.OBOffsetBayer[3] = OB_TO_INT(dip_reg_main1->DIP_X_OBC2_OFFST3.Raw);

    input.OBOffsetMono[0] = OB_TO_INT(dip_reg_main2->DIP_X_OBC2_OFFST0.Raw);
    input.OBOffsetMono[1] = OB_TO_INT(dip_reg_main2->DIP_X_OBC2_OFFST1.Raw);
    input.OBOffsetMono[2] = OB_TO_INT(dip_reg_main2->DIP_X_OBC2_OFFST2.Raw);
    input.OBOffsetMono[3] = OB_TO_INT(dip_reg_main2->DIP_X_OBC2_OFFST3.Raw);

    // PreGain
    MY_LOGD("preGainBayer in RGB order");
    input.preGainBayer[0] = dip_reg_main1->DIP_X_PGN_GAIN_2.Bits.PGN_GAIN_R;
    input.preGainBayer[1] = dip_reg_main1->DIP_X_PGN_GAIN_1.Bits.PGN_GAIN_GB;
    input.preGainBayer[2] = dip_reg_main1->DIP_X_PGN_GAIN_1.Bits.PGN_GAIN_B;

    getIsRotateAndBayerOrder(input);

    // Sensor gain
    getSensorGainAndShadingRA(pMeta_InHal_main1, input, MTRUE);
    getSensorGainAndShadingRA(pMeta_InHal_main2, input, MFALSE);

    StereoArea areaRatioCrop = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);
    input.OffsetX = areaRatioCrop.startPt.x;
    input.OffsetY = areaRatioCrop.startPt.y;

    // IspGain, use GAIN0 directly
    input.ispGainBayer = dip_reg_main1->DIP_X_OBC2_GAIN0.Raw;
    input.ispGainMono = dip_reg_main2->DIP_X_OBC2_GAIN0.Raw;

    //ISO dependent params
    getISODependentParams(pMeta_InHal_main1, input);
    getPerformanceQualityOption(input);

    // Affine Matrix, the warping matrix from N3D
    getAffineMatrix((float*)smpBuf_warpingMatrix->mImageBuffer->getBufVA(0), input);
    // dPadding, currently always 0
    input.dPadding[0] = 0;
    input.dPadding[1] = 0;

    // shading Gain
    #if defined(GTEST) || USE_DEFAULT_SHADING_GAIN == 1
        // use default shading gain
        MY_LOGD("Use default shading gain, filled with 1.0");
        {
            float* buffer = new float[SHADING_GAIN_SIZE*SHADING_GAIN_SIZE*4];
            if (buffer == nullptr){
                MY_LOGE("failed allocating shading gain!");
                return nullptr;
            }
            // filled with 1.0 for debugging
            std::fill(buffer,buffer+SHADING_GAIN_SIZE*SHADING_GAIN_SIZE*4-1,1.0);
            input.bayerGain = buffer;
            input.monoGain = buffer;
        }
    #else
        getShadingGain(pMeta_InHal_main1, rTuningParam_main1, *outGain_main1, MTRUE);
        input.bayerGain = (float*)outGain_main1->getData();
        getShadingGain(pMeta_InHal_main2, rTuningParam_main2, *outGain_main2, MFALSE);
        input.monoGain = (float*)outGain_main2->getData();
    #endif

    // input image buffers
    input.bayerProcessedRaw = &smgpBuf_mfbo_1->mGraphicBuffer;
    input.bayerW = &smgpBuf_w1->mGraphicBuffer;
    input.monoProcessedRaw = &smgpBuf_mfbo_2->mGraphicBuffer;

    // we pass the ROI of disparityMap to ALG
    StereoArea areaDisparityMap = mSizePrvider->getBufferSize(E_DMP_H, eSTEREO_SCENARIO_CAPTURE);
    // Add offset to point to the ROI
    // input.depth = (short*)smgpBuf_disparityMap->mImageBuffer->getBufVA(0);
    // input.depth = input.depth +
    //               smgpBuf_disparityMap->mImageBuffer->getImgSize().w*(areaDisparityMap.padding.h/2) +
    //               areaDisparityMap.padding.w/2;
    input.depth = &smgpBuf_disparityMap->mGraphicBuffer;

    // output parameters
    BWDN_HAL_OUTPUT output;
    output.outBuffer = &smgpBMDN_HAL_out->mGraphicBuffer;

    // keep the output image buffer in imgInfo
    sp<ImageBufInfoMap> ImgBufInfo = new ImageBufInfoMap(request);
    ImgBufInfo->addGraphicBuffer(BID_DENOISE_HAL_OUT, smgpBMDN_HAL_out);

    // SceneInfo
    SmartImageBuffer smpBuf_sceneInfo = imgInfo_Depth->getSmartBuffer(BID_N3D_OUT_SCENEINFO);

    input.BitMode = getBMDNBitMode();

    map<MINT32, MINT32> exifData;
    collectExifData(exifData, input, (MINT32*) smpBuf_sceneInfo->mImageBuffer->getBufVA(0));

    // run alg
    if(::property_get_int32("vendor.bmdenoise.pipe.bwdn", 1) == 1){
        mpBWDNHAL->BWDNHALRun(input, output);
    }else{
        MY_LOGD("by-pass BWDN ALG run");
    }

    // update exif meta
    ExifWriter writer(PIPE_LOG_TAG);
    for(auto iter = exifData.begin(); iter != exifData.end(); iter++){
        writer.sendData(request->getRequestNo(), iter->first, iter->second);
    }

    MY_LOGD("-, reqID=%d", request->getRequestNo());
    MET_END(DO_BM_DENOISE);
    return ImgBufInfo;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
getIsRotateAndBayerOrder(BWDN_HAL_PARAMS &rInData)
{
    // bayer order
    rInData.bayerOrder = mBayerOrder_main1;

    // isRotate
    switch(mModuleTrans)
    {
        case 0:
            rInData.isRotate = 0;
            break;
        case eTransform_ROT_90:
            rInData.isRotate = 1;
            break;
        // currently not supported rotation
        // case eTransform_ROT_180:
        //     inData.isRotate = 1;
        //     break;
        // case eTransform_ROT_270:
        //     inData.isRotate = 1;
        //     break;
        default:
            MY_LOGE("ALG not support module rotation(%d) for now!", mModuleTrans);
            return MFALSE;
    }

    MY_LOGD("bayerOrder:%d, rotate:%d", rInData.bayerOrder, rInData.isRotate);

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
getSensorGainAndShadingRA(IMetadata* pMeta, BWDN_HAL_PARAMS &rInData, MBOOL isBayer)
{
    IMetadata::Memory meta;
    if(tryGetMetadata<IMetadata::Memory>(pMeta, MTK_PROCESSOR_CAMINFO, meta))
    {
        RAWIspCamInfo pCamInfo;
        ::memcpy(&pCamInfo, meta.array(), sizeof(NSIspTuning::RAWIspCamInfo));

        if(isBayer){
            rInData.sensorGainBayer = pCamInfo.rAEInfo.u4AfeGain;

            MINT32 ra = -1;
            IspTuningCustom* pTuningCustom = IspTuningCustom::createInstance(ESensorDev_Main, mSensorIdx_Main1);
            ra = pTuningCustom->evaluate_Shading_Ratio(pCamInfo);
            rInData.RA = ra;
            MY_LOGD("dynamic RA:%d", rInData.RA);
        }else{
            rInData.sensorGainMono = pCamInfo.rAEInfo.u4AfeGain;
        }
        return MTRUE;
    }else{
        MY_LOGE("Get MTK_PROCESSOR_CAMINFO failed.");
        if(isBayer){
            rInData.sensorGainBayer = 0;
            rInData.RA = 32;
        }else{
            rInData.sensorGainMono = 0;
        }
        return MFALSE;
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
getISODependentParams(IMetadata* pMeta, BWDN_HAL_PARAMS &rInData)
{
    MINT32 ISO = 0;
    MINT32 debugISO = 0;
    IMetadata exifMeta;

    if( tryGetMetadata<IMetadata>(pMeta, MTK_3A_EXIF_METADATA, exifMeta) ) {
        if(!tryGetMetadata<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, ISO)){
            MY_LOGE("Get ISO from meta failed, use default value:%d", ISO);
        }
    }
    else {
        MY_LOGE("no tag: MTK_3A_EXIF_METADATA, use default value:%d", ISO);
    }

    debugISO = ::property_get_int32("vendor.debug.bmdenoise.iso", -1);

    MY_LOGD("metaISO:%d/debugISO:%d", ISO, debugISO);

    if(debugISO != -1 && debugISO >= 0){
        ISO = debugISO;
    }

    BMDeNoiseISODependentParam ISODependentParams = getBMDeNoiseISODePendentParams(ISO);

    rInData.BW_SingleRange = ISODependentParams.BW_SingleRange;
    rInData.BW_OccRange = ISODependentParams.BW_OccRange;
    rInData.BW_Range = ISODependentParams.BW_Range;
    rInData.BW_Kernel = ISODependentParams.BW_Kernel;
    rInData.B_Range = ISODependentParams.B_Range;
    rInData.B_Kernel = ISODependentParams.B_Kernel;
    rInData.W_Range = ISODependentParams.W_Range;
    rInData.W_Kernel = ISODependentParams.W_Kernel;
    rInData.VSL = ISODependentParams.VSL;
    rInData.VOFT = ISODependentParams.VOFT;
    rInData.VGAIN = ISODependentParams.VGAIN;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
getDynamicShadingRA(BWDN_HAL_PARAMS &rInData)
{
    MUINT32 ret = -1;
    ret = DEFAULT_DYNAMIC_SHADING;
    MY_LOGD("Failed to create IspTuningCustom Instance. use default value");
    MY_LOGD("get dynamic shading RA:%d", ret);
    rInData.RA = ret;
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
getPerformanceQualityOption(BWDN_HAL_PARAMS &rInData)
{
    BMDeNoiseQualityPerformanceParam param = getBMDeNoiseQualityPerformanceParam();

    rInData.dsH = (mDebugDsH == -1) ? getDVEFactorFromDSMode(mDVEConig.Dve_Horz_Ds_Mode)    : mDebugDsH;
    rInData.dsV = (mDebugDsV == -1) ? getDVEFactorFromDSMode(mDVEConig.Dve_Vert_Ds_Mode)    : mDebugDsV;

    rInData.FPREPROC = (mDebugPerformanceQualityOption.FPREPROC == -1) ?    param.FPREPROC  : mDebugPerformanceQualityOption.FPREPROC;
    rInData.FSSTEP = (mDebugPerformanceQualityOption.FSSTEP == -1) ?        param.FSSTEP    : mDebugPerformanceQualityOption.FSSTEP;
    rInData.DblkRto = (mDebugPerformanceQualityOption.DblkRto == -1) ?      param.DblkRto   : mDebugPerformanceQualityOption.DblkRto;
    rInData.DblkTH = (mDebugPerformanceQualityOption.DblkTH == -1) ?        param.DblkTH    : mDebugPerformanceQualityOption.DblkTH;

    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
getAffineMatrix(float* warpingMatrix, BWDN_HAL_PARAMS &rInData)
{
    #if USE_DEFAULT_AMATRIX
        rInData.Trans[0] = 1;
        rInData.Trans[1] = 0;
        rInData.Trans[2] = 0;
        rInData.Trans[3] = 0;
        rInData.Trans[4] = 1;
        rInData.Trans[5] = 0;
        rInData.Trans[6] = 0;
        rInData.Trans[7] = 0;
        rInData.Trans[8] = 1;
        MY_LOGD("Test mode, use identity matrix");
    #else
        rInData.Trans[0] = warpingMatrix[0];
        rInData.Trans[1] = warpingMatrix[1];
        rInData.Trans[2] = warpingMatrix[2];
        rInData.Trans[3] = warpingMatrix[3];
        rInData.Trans[4] = warpingMatrix[4];
        rInData.Trans[5] = warpingMatrix[5];
        rInData.Trans[6] = warpingMatrix[6];
        rInData.Trans[7] = warpingMatrix[7];
        rInData.Trans[8] = warpingMatrix[8];
    #endif

    MY_LOGD("warpingMatrix: [%f,%f,%f][%f,%f,%f][%f,%f,%f]",
        rInData.Trans[0],
        rInData.Trans[1],
        rInData.Trans[2],
        rInData.Trans[3],
        rInData.Trans[4],
        rInData.Trans[5],
        rInData.Trans[6],
        rInData.Trans[7],
        rInData.Trans[8]
    );
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
getShadingGain(IMetadata* pMeta, TuningParam& tuningParam, ILscTable& outGain, MBOOL isBayer)
{
    CAM_TRACE_NAME("DeNoiseNode::getShadingGain");
    StereoArea area_src;
    StereoArea area_dst;

    std::unique_ptr<ILscTable> tmpOutGain(MAKE_LscTable(ILscTable::GAIN_FIXED));

    if(isBayer){
        area_src = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_1);
        area_dst = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_1);
    }else{
        area_src = mSizePrvider->getBufferSize(E_BM_PREPROCESS_MFBO_2);
        area_dst = mSizePrvider->getBufferSize(E_BM_PREPROCESS_FULLRAW_CROP_2);
    }
    MY_LOGW("should check correctness of shading tables");
    MINT32 i4W = area_src.size.w;
    MINT32 i4H = area_src.size.h;

    // get MTK_LSC_TBL_DATA from hal meta
    IMetadata::Memory rLscData;
    if( !tryGetMetadata<IMetadata::Memory>(pMeta, MTK_LSC_TBL_DATA, rLscData) ) {
        MY_LOGE("no MTK_LSC_TBL_DATA!");
        return MFALSE;
    }
    std::vector<MUINT8> LscData = std::vector<MUINT8>(rLscData.array(), rLscData.array()+rLscData.size());

    // create HWTBL
    ILscTable::Config rCfg;
    ::memcpy(&rCfg, LscData.data(), sizeof(ILscTable::Config));
    std::unique_ptr<ILscTable> inputHw(
        MAKE_LscTable(
            ILscTable::HWTBL,
            i4W,i4H,
            rCfg.i4GridX, rCfg.i4GridY
        )
    );

    // temp code, will be removed after ILscTbl::RsvdData is revealed to us
    struct tempRsvdData{
        tempRsvdData();
        tempRsvdData(MUINT32 u4HwRto);
        MUINT32 u4HwRto;
    };

    inputHw->setData(LscData.data() + sizeof(ILscTable::Config) +sizeof(tempRsvdData), inputHw->getSize());

    MY_LOGD("ShadingGain srcResize:%d,%d, dstCrop:%d,%d,%d,%d, gridSize:%d,%d",
        // step1. src resize, the same as p1 full out
        area_src.size.w, area_src.size.h,
        // step2. dst crop to be 16:9 or 4:3, the same as alg size
        area_dst.startPt.x, area_dst.startPt.y, area_dst.size.w, area_dst.size.h,
        // step3. result shading gain table grid. No matter what the aspect ratio of dst crop is, it is always a 1:1 square
        SHADING_GAIN_SIZE, SHADING_GAIN_SIZE
    );
    inputHw->cropOut(
        ILscTable::TransformCfg_T(
            area_src.size.w, area_src.size.h,
            SHADING_GAIN_SIZE, SHADING_GAIN_SIZE,
            area_dst.startPt.x, area_dst.startPt.y, area_dst.size.w, area_dst.size.h
        ),
        (*tmpOutGain)
    );

    tmpOutGain->convert(outGain);

    if(miDumpShadingGain == 1){
        if(isBayer){
            tmpOutGain->dump("sdcard/bmdenoise/shadingBayer_fixed.raw");
            outGain.dump("sdcard/bmdenoise/shadingBayer_float.raw");

            float* pTmpOutGain = (float*)tmpOutGain->getData();
            float* pOutGain = (float*)outGain.getData();

            MY_LOGD("shadingBayer Fixed(%d):%f,%f,%f,%f,%f  Float(%d):%f,%f,%f,%f,%f",
                tmpOutGain->getSize(),
                pTmpOutGain[0], pTmpOutGain[1], pTmpOutGain[2], pTmpOutGain[3], pTmpOutGain[4],
                outGain.getSize(),
                pOutGain[0], pOutGain[1], pOutGain[2], pOutGain[3], pOutGain[4]
            );

        }else{
            tmpOutGain->dump("sdcard/bmdenoise/shadingMono_fixed.raw");
            outGain.dump("sdcard/bmdenoise/shadingMono_float.raw");

            float* pTmpOutGain = (float*)tmpOutGain->getData();
            float* pOutGain = (float*)outGain.getData();

            MY_LOGD("shadingMono Fixed(%d):%f,%f,%f,%f,%f  Float(%d):%f,%f,%f,%f,%f",
                tmpOutGain->getSize(),
                pTmpOutGain[0], pTmpOutGain[1], pTmpOutGain[2], pTmpOutGain[3], pTmpOutGain[4],
                outGain.getSize(),
                pOutGain[0], pOutGain[1], pOutGain[2], pOutGain[3], pOutGain[4]
            );
        }
    }
    return MTRUE;
}
/*******************************************************************************
 *
 ********************************************************************************/
MINT32
DeNoiseNode::
getDVEFactorFromDSMode(MUINT32 dsMode)
{
    switch(dsMode){
        case 0:
            return 8;
        case 1:
            return 4;
        default:
            MY_LOGW("unknown dsMode(%d)! return 8", dsMode);
            return 8;
    }
}
/*******************************************************************************
 *
 ********************************************************************************/
sp<ImageBufferHeap>
DeNoiseNode::
createImageBufferHeapInRGB48(sp<IImageBuffer> pImgBuf)
{
    StereoSizeProvider* sizePrvider = StereoSizeProvider::getInstance();
    StereoArea area;
    area = sizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT_ROT_BACK);

    MSize newSize = area.size;

    MUINT32 bufStridesInBytes[3] = {
        (MUINT32)newSize.w*6, //bytes-per-pixel of RGB48
        0,
        0
    };

    MINT32 bufBoundaryInBytes[3] = {0,0,0};

    MY_LOGD("create new heap with size:%dx%d, bufStridesInBytes [%d,%d,%d]",
        newSize.w, newSize.h,
        bufStridesInBytes[0],
        bufStridesInBytes[1],
        bufStridesInBytes[2]
    );

    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
        eImgFmt_RGB48,
        newSize,
        bufStridesInBytes,
        bufBoundaryInBytes,
        1
    );
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
        pImgBuf->getFD(0),
        (MUINTPTR)pImgBuf->getBufVA(0),
        0,
        0,
        0
    );

    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
        LOG_TAG,
        imgParam,
        portBufInfo
    );

    if(pHeap == nullptr){
        MY_LOGE("creating pHeap heap in eImgFmt_RGB48 failed!");
    }
    return pHeap;
}
/*******************************************************************************
 *
 ********************************************************************************/
MVOID
DeNoiseNode::
initBufferPool()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);

    int allocateSize = get_stereo_bmdenoise_capture_buffer_cnt();
    int tuningAllocSize = TUNING_ALLOC_SIZE*get_stereo_bmdenoise_capture_buffer_cnt();

    StereoArea area;

    MY_LOGD("GRAPHIC_BUF__ALLOC_RATIO:%d", GRAPHIC_BUF__ALLOC_RATIO);

    Vector<NSBMDN::BufferConfig> vBufConfig;
    {
        area = mSizePrvider->getBufferSize(E_BM_DENOISE_HAL_OUT);
        NSBMDN::BufferConfig c = {
            "BID_DENOISE_HAL_OUT",
            BID_DENOISE_HAL_OUT,
            // RGB48
            // (MUINT32)area.size.w*3,
            // UNPAK14
            (MUINT32)area.size.w,
            (MUINT32)area.size.h * GRAPHIC_BUF__ALLOC_RATIO,
            eImgFmt_Y8,
            GraphicBufferPool::USAGE_HW_RENDER,
            MTRUE,
            MTRUE,
            (MUINT32)allocateSize
        };
        vBufConfig.push_back(c);
    }

    if(!mBufPool.init(vBufConfig, tuningAllocSize)){
        MY_LOGE("Error! Please check above errors!");
    }
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
doBufferPoolAllocation(MUINT32 count)
{
    return mBufPool.doAllocate(count);
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DeNoiseNode::
releaseALG()
{
    ScopeLogger logger(PIPE_LOG_TAG, __FUNCTION__);
    if(mpBWDNHAL != nullptr){
        mpBWDNHAL->destroyInstance();
        mpBWDNHAL = nullptr;
    }
    return MTRUE;
}