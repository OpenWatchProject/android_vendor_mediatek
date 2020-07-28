/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/SCNode"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;
using namespace NSCam::Utils::Format;

#include <semaphore.h>
#include <vector>
#include <list>
using namespace std;
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
using namespace android;
//
#include <mtkcam/v1/config/PriorityDefs.h>
#include <aee.h>
#include <sys/prctl.h>
#include <mtkcam/featureio/stereo_hal_base.h>
#include <mtkcam/featureio/fd_hal_base.h>
#include <mtkcam/camnode/StereoCtrlNode.h>
// For query stereo static data
#include <mtkcam/featureio/stereo_setting_provider.h>
//
#include "./inc/stereonodeImpl.h"
#include "./inc/IspSyncControlHw.h"
#include "mtkcam/drv/imem_drv.h"
#include <mtkcam/utils/imagebuf/BaseImageBufferHeap.h>
#include <mtkcam/utils/imagebuf/IGrallocImageBufferHeap.h>
#include <ui/gralloc_extra.h>
//
#include <cutils/properties.h>

// For capture Face detection
#include <DpBlitStream.h>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

//	#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
//	#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%d:%s] " fmt, getSensorIdx(),mInitCfg.mScenarioType, __func__, ##arg)

#define MY_LOGV2(fmt, arg...)       CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD2(fmt, arg...)       CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI2(fmt, arg...)       CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW2(fmt, arg...)       CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE2(fmt, arg...)       CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA2(fmt, arg...)       CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF2(fmt, arg...)       CAM_LOGF("[%s] " fmt, __func__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)
#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

#define FUNC_START          MY_LOGD("+")
#define FUNC_END            MY_LOGD("-")

#define ENABLE_CAMIOCONTROL_LOG (0)
#define ENABLE_BUFCONTROL_LOG   (1)
#define BUFFER_RETURN_CHECK     (1)

#define STEREO_CTRL_NODE_BUF_COUNT_PREVUEW   5
#define STEREO_CTRL_NODE_BUF_COUNT_CAPTURE   1

#ifdef HAVE_AEE_FEATURE
#include <aee.h>
#define AEE_ASSERT(String)            \
                 do {                      \
                     aee_system_exception( \
                         LOG_TAG,          \
                         NULL,             \
                         DB_OPT_DEFAULT,   \
                         String);          \
                 } while(0)
#else
#define AEE_ASSERT(String)
#endif

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_NAME        "SCtrl"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*   utilities
********************************************************************************/
#define FEO_0           (0x01)
#define IMG_0           (0x02)
#define RGB_0           (0x04)
#define FEO_1           (0x08)
#define IMG_1           (0x10)
#define RGB_1           (0x20)
#define MAIN            (0x40)
//
// #define PREVIEW_SRC     (FEO_0|FEO_1|IMG_0|IMG_1)
//	#define CAPTURE_SRC     (FEO_0|FEO_1|IMG_0|IMG_1|MAIN)      //(FEO_0|FEO_1|IMG_0|IMG_1|RGB_0|RGB_1|MAIN)
#define CAPTURE_SRC     (IMG_0|IMG_1|MAIN)      //(FEO_0|FEO_1|IMG_0|IMG_1|RGB_0|RGB_1|MAIN)
// Shane 20141211 In this platform, there is no FEO data
#define PREVIEW_SRC     (IMG_0|IMG_1)
// #define CAPTURE_SRC     (IMG_0|IMG_1|RGB_0|RGB_1|MAIN)
/*******************************************************************************
 *
 ********************************************************************************/

typedef struct BufConfig{
    MUINT32          mNodedatatype;
    EImageFormat     mFmt;
    MSize            mSize;
    MRect            mCrop;
} BufConfig_t;

class StereoCtrlNodeImpl : public StereoCtrlNode
{
    public: // ctor & dtor
        StereoCtrlNodeImpl(SCNodeInitCfg const initCfg);
        ~StereoCtrlNodeImpl();

        virtual MBOOL               init();
        virtual MBOOL               uninit();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations
        MBOOL           isReadyToAlgo() const;

        MBOOL           pushBuf(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext);

    protected:
        MINT32          getOpenId_Main() const      { return mSensorId_Main; }
        MINT32          getOpenId_Main2() const     { return mSensorId_Main2; }
        MUINT32         getSrcDataSet() const       { return mSrcDataSet; }
        void            resetSrcDataSet()           { mSrcDataSet = 0; }
        MUINT32         getStereoType() const       { return mInitCfg.mScenarioType; }
        MBOOL           isCapturePath() const
                        {
                            return ( (getStereoType() == STEREO_CTRL_CAPTURE) || (getStereoType() == STEREO_CTRL_ZSD) )
                                ? MTRUE : MFALSE;
                        }
        MBOOL           isPreviewPath() const
                        {
                            return ((getStereoType() == STEREO_CTRL_PREVIEW)? MTRUE : MFALSE);
                        }
        static void*    doThreadAlloc(void* arg);
        bool            alloc();
        bool            waitThreadDone();
        MBOOL           allocBuffers(vector<BufConfig_t> & lPortCfg);

        struct PostBufInfo
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;

            public:     ////    ctor.
                PostBufInfo(
                        MUINT32         _data= 0,
                        IImageBuffer*   _buf = NULL,
                        MUINT32         _ext = 0
                    )
                    : data(_data)
                    , buf(_buf)
                    , ext(_ext) {}
        };

        struct FeoBufInfo
        {
            MUINT32          data;
            IMEM_BUF_INFO*   buf;
            MUINT32          ext;

            public:     ////    ctor.
                FeoBufInfo(
                        MUINT32         _data= 0,
                        IMEM_BUF_INFO*  _buf = NULL,
                        MUINT32         _ext = 0
                    )
                    : data(_data)
                    , buf(_buf)
                    , ext(_ext) {}
        };

    private:
        //     init config
        SCNodeInitCfg const     mInitCfg;
        //
        MINT32                  mSensorId_Main;
        MINT32                  mSensorId_Main2;
        //
        MUINT32                 mSrcDataSet;
        //
        StereoHalBase*          mpStereoHal;
        //
        IspSyncControlHw*       mpISC_Main;
        IspSyncControlHw*       mpISC_Main2;
        //
        // ************ buffer control ************ //
        mutable Mutex           mLock;
        mutable Mutex           mAlgoLock;
        MBOOL                   mbEnable;
        MUINT32                 muPostFrameCnt;
        list<PostBufInfo>       mlPostBufMain;
        list<PostBufInfo>       mlPostBufImg;
        list<PostBufInfo>       mlPostBufImg_Main2;
        // list<FeoBufInfo>        mlPostBufFeo;
        // list<FeoBufInfo>        mlPostBufFeo_Main2;
        list<PostBufInfo>       mlPostBufFeo;
        list<PostBufInfo>       mlPostBufFeo_Main2;
        // for capture fd use.
        IImageBuffer*           mpMainImageBuf;
        //
        MBOOL                   mbAllocDone;
        pthread_t               mThreadAlloc;
        pthread_t               mThreadDoFDDetection;

        MUINT32                 mBufCount;
};


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNode*
StereoCtrlNode::
createInstance(SCNodeInitCfg const initCfg)
{
    return new StereoCtrlNodeImpl(initCfg);
}


/*******************************************************************************
 *
 ********************************************************************************/
void
StereoCtrlNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNode::
    StereoCtrlNode()
: ICamThreadNode( MODULE_NAME, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNode::
~StereoCtrlNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNodeImpl::
    StereoCtrlNodeImpl(SCNodeInitCfg const initCfg)
    : StereoCtrlNode()
    , mInitCfg(initCfg)
    , mSensorId_Main(-1)
    , mSensorId_Main2(-1)
    , mSrcDataSet(0)
    , mpStereoHal(NULL)
    , mpISC_Main(NULL)
    , mpISC_Main2(NULL)
    , mbEnable(MTRUE)
    , muPostFrameCnt(0)
    , mpMainImageBuf(NULL)
    , mThreadAlloc(NULL)
    , mThreadDoFDDetection(NULL)
    , mbAllocDone(MFALSE)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_IMG_0 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_IMG_1 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_MAIN_SRC );

    addDataSupport( ENDPOINT_DST, STEREO_CTRL_DST_M );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_MAIN_DST );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_DEPTHMAP_DST );
//      addDataSupport( ENDPOINT_DST, STEREO_CTRL_DST_S );

    mBufCount = isCapturePath()? STEREO_CTRL_NODE_BUF_COUNT_CAPTURE : STEREO_CTRL_NODE_BUF_COUNT_PREVUEW;
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoCtrlNodeImpl::
~StereoCtrlNodeImpl()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
init()
{
    MBOOL ret = MFALSE;

    StereoSettingProvider::getStereoSensorIndex(mSensorId_Main, mSensorId_Main2);
    mpISC_Main = IspSyncControlHw::createInstance( getOpenId_Main() );
    mpISC_Main2= IspSyncControlHw::createInstance( getOpenId_Main2() );

    if( !mpISC_Main || !mpISC_Main2)
    {
        MY_LOGE("create IspSyncControlHw failed");
    }
    //
    mpStereoHal = StereoHalBase::createInstance();

    INIT_DATA_STEREO_IN_T   sDataIn;
    INIT_DATA_STEREO_OUT_T  sDataOut;
    RRZ_DATA_STEREO_T       sRrzData;
    //
    mlPostBufMain.clear();
    mlPostBufImg.clear();
    mlPostBufImg_Main2.clear();
    mlPostBufFeo.clear();
    mlPostBufFeo_Main2.clear();
    //
    switch ( getStereoType() )
    {
        case STEREO_CTRL_PREVIEW:
            sDataIn.eScenario = STEREO_SCENARIO_PREVIEW;
            MY_LOGD("sDataIn.eScenario = STEREO_SCENARIO_PREVIEW %d", STEREO_SCENARIO_PREVIEW);
            break;
        case STEREO_CTRL_RECORD:
            sDataIn.eScenario = STEREO_SCENARIO_RECORD;
            MY_LOGD("sDataIn.eScenario = STEREO_SCENARIO_RECORD %d", STEREO_SCENARIO_RECORD);
            break;
        case STEREO_CTRL_CAPTURE:
        case STEREO_CTRL_ZSD:
            sDataIn.eScenario = STEREO_SCENARIO_CAPTURE;
            MY_LOGD("sDataIn.eScenario = STEREO_SCENARIO_CAPTURE %d", STEREO_SCENARIO_CAPTURE);
            break;
        default:
            MY_LOGE("unsupport stereo type(%d)", getStereoType());
            break;
    }
    //
    sDataIn.main1_sensor_index      = getOpenId_Main();
    sDataIn.main2_sensor_index      = getOpenId_Main2();
    sDataIn.main_image_size         = MSize(mInitCfg.mMainImgWidth, mInitCfg.mMainImgHeight);
    sDataIn.algo_image_size         = MSize(mInitCfg.mAlgoImgWidth, mInitCfg.mAlgoImgHeight);
    sDataIn.orientation             = mInitCfg.mTransform;
    sDataIn.main1_sensor_scenario   = mInitCfg.mMainSensorMode;
    sDataIn.main2_sensor_scenario   = mInitCfg.mMain2SensorMode;

    // Shane Ratio Switch : This will affect stereonode(Pass2)
    // TODO: get ratio settings from APP
    if(isCapturePath()){
        if(mInitCfg.mAlgoImgWidth == 1984 && mInitCfg.mAlgoImgHeight == 1696){
            sDataIn.eImageRatio = eRatio_4_3;
        }else{
            sDataIn.eImageRatio = eRatio_16_9;
        }
    }else{
        if(mInitCfg.mAlgoImgWidth == 1920 && mInitCfg.mAlgoImgHeight == 1440){
            sDataIn.eImageRatio = eRatio_4_3;
        }else{
            sDataIn.eImageRatio = eRatio_16_9;
        }
    }


    MY_LOGD("main_image_size(%dx%d) algo_image_size(%dx%d) orientation(%d) scenario(%d,%d), sDataIn.eImageRatio",
        sDataIn.main_image_size.w, sDataIn.main_image_size.h, sDataIn.algo_image_size.w, sDataIn.algo_image_size.h, sDataIn.orientation,
        sDataIn.main1_sensor_scenario, sDataIn.main2_sensor_scenario, sDataIn.eImageRatio);
    //
    MY_LOGD("StereoCtrlNode : init() : mSensorId_Main = %d ",getOpenId_Main());
    MY_LOGD("StereoCtrlNode : init() : mSensorId_Main_2 = %d ",getOpenId_Main2());
    //
    {
        Mutex::Autolock lock(mAlgoLock);
        if( !mpStereoHal || !mpStereoHal->STEREOInit(sDataIn, sDataOut) )
        {
            MY_LOGE("STEREOInit fail");
            goto lbExit;
        }
        mbEnable = MTRUE;
    }
    if ( getStereoType() != STEREO_CTRL_ZSD )
    {
        // NOTE:
        // sRrzData.rrz_size_main1.w = stereo_hal : gStereoKernelInitInfo.remap_main.rrz_out_width
        // sRrzData.rrz_crop_main1   = stereo_hal : MRect(MPoint(gStereoKernelInitInfo.remap_main.rrz_offset_x,
        //                                                       gStereoKernelInitInfo.remap_main.rrz_offset_y),
        //                                                MSize( gStereoKernelInitInfo.remap_main.rrz_usage_width,
        //                                                       gStereoKernelInitInfo.remap_main.rrz_usage_height));
        mpStereoHal->STEREOGetRrzInfo(sRrzData);
        mpISC_Main->setVideoSize(0, 0);
        mpISC_Main->setRrzoMinSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);       // 1920x1080
        mpISC_Main->setPreviewSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);       // 1920x1080
        mpISC_Main->setPass1InitRrzoSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h); // 1920x1080
        mpISC_Main->setInitialRrzoSize(sRrzData.rrz_crop_main1, sRrzData.rrz_size_main1);       // Crop : p = (0,100), size = (2096, 1178); resize: 1920x1080

        mpISC_Main2->setVideoSize(0, 0);
        mpISC_Main2->setRrzoMinSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);      // 960x540
        mpISC_Main2->setPreviewSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);      // 960x540
        mpISC_Main2->setPass1InitRrzoSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);// 960x540
        mpISC_Main2->setInitialRrzoSize(sRrzData.rrz_crop_main2, sRrzData.rrz_size_main2);      // Crop : p = (0,100), size = (960, 540);  resize: 960x540

        //
        mpISC_Main->calRrzoMaxZoomRatio();
        mpISC_Main2->calRrzoMaxZoomRatio();
    }
    //
    ret = MTRUE;
lbExit:
    MY_LOGD("-");
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
uninit()
{
    Mutex::Autolock lock(mAlgoLock);
    MY_LOGD("+");
    MBOOL ret = MTRUE;
    mbEnable = MFALSE;
    if ( mpStereoHal )
    {
        ret = mpStereoHal->STEREODestroy();
    }

    if(mpISC_Main)
    {
        mpISC_Main->destroyInstance();
        mpISC_Main = NULL;
    }
    if(mpISC_Main2)
    {
        mpISC_Main2->destroyInstance();
        mpISC_Main2 = NULL;
    }

    if ( mpStereoHal )
    {
        mpStereoHal->destroyInstance();
        mpStereoHal = NULL;
    }
    MY_LOGD("-");
    return ret;
}


/******************************************************************************
*
 ******************************************************************************/
bool
StereoCtrlNodeImpl::
waitThreadDone()
{
    void* threadRet = NULL;
    if( pthread_join(mThreadAlloc, &threadRet) != 0 )
    {
        MY_LOGE("pthread join fail");
    }
    mbAllocDone = MTRUE;
    return MTRUE;
}


/******************************************************************************
*
 ******************************************************************************/
bool
StereoCtrlNodeImpl::
alloc()
{
    MY_LOGD("stereo type(0x%x), alloc buffer = %d x %d ",
            getStereoType(), mInitCfg.mAlgoImgWidth, mInitCfg.mAlgoImgHeight);

    // create buffer for algo output
    if ( isPreviewPath() )
    {
        MSize algoSize(mInitCfg.mAlgoImgWidth,mInitCfg.mAlgoImgHeight) ;
        vector<BufConfig_t> lHwPortCfg;
        BufConfig_t cfgImg = {
            STEREO_CTRL_DST_M,
            eImgFmt_YV12,
            algoSize,
            MRect( MPoint(0, 0), algoSize )
        };
        lHwPortCfg.push_back(cfgImg);
        if( !allocBuffers(lHwPortCfg) )
        {
            MY_LOGE("alloc buffers failed");
        }
    }
    if(isCapturePath())
    {
        vector<BufConfig_t> lHwPortCfg;

        // Bokeh output image size (the same with Main1 image size)
        MSize MainImageSize(mInitCfg.mMainImgWidth,mInitCfg.mMainImgHeight) ;
        BufConfig_t cfgImg = {
            STEREO_CTRL_MAIN_DST,
            eImgFmt_YV12,
            MainImageSize,
            MRect( MPoint(0, 0), MainImageSize )
        };
        lHwPortCfg.push_back(cfgImg);

        // Depthmap output image size
        MSize DepthmapSize(mInitCfg.mDepthmapWidth,mInitCfg.mDepthmapHeight) ;
        BufConfig_t cfgDepthmap = {
            STEREO_CTRL_DEPTHMAP_DST,
            eImgFmt_Y8,
            DepthmapSize,
            MRect( MPoint(0, 0), DepthmapSize )
        };
        lHwPortCfg.push_back(cfgDepthmap);

        if( !allocBuffers(lHwPortCfg) )
        {
            MY_LOGE("alloc buffers failed");
        }
    }

    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
allocBuffers(vector<BufConfig_t> & lPortCfg)
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    vector< BufConfig_t >::const_iterator pConfig = lPortCfg.begin();
    //
    while( pConfig != lPortCfg.end() )
    {
        MUINT32 nodedatatype = pConfig->mNodedatatype;
        ICamBufHandler* pBufHdl = getBufferHandler(nodedatatype);
        MY_LOGD("handle(%p) data(%d) S(%dx%d)F(0x%x)", pBufHdl, nodedatatype, pConfig->mSize.w, pConfig->mSize.h, pConfig->mFmt);
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data(%d)", nodedatatype);
            goto lbExit;
        }
        //alloc buffer
        AllocInfo allocinfo(pConfig->mSize.w, pConfig->mSize.h, pConfig->mFmt,
                (eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK |
                eBUFFER_USAGE_SW_WRITE_OFTEN|eBUFFER_USAGE_SW_READ_OFTEN));

        for(MUINT32 i = 0; i < mBufCount; i++ )
        {
            if((nodedatatype == STEREO_CTRL_DST_M)||
                (nodedatatype == STEREO_CTRL_MAIN_DST)||
                (nodedatatype == STEREO_CTRL_DEPTHMAP_DST))
            {
                pBufHdl->updateRequest(nodedatatype,
                                        mpStereoHal->getAlgoInputTransform(),
                                        isPreviewPath()?
                                        (NSImageio::NSIspio::EPortCapbility_Disp):
                                        (NSImageio::NSIspio::EPortCapbility_Cap));
                if( mpStereoHal->getAlgoInputTransform() == eTransform_ROT_90  ||
                    mpStereoHal->getAlgoInputTransform() == eTransform_ROT_270 ||
                    mpStereoHal->getAlgoInputTransform() == (eTransform_FLIP_V |eTransform_ROT_90)||
                    mpStereoHal->getAlgoInputTransform() == (eTransform_FLIP_H |eTransform_ROT_90))
                {
                    allocinfo.w = pConfig->mSize.h;
                    allocinfo.h = pConfig->mSize.w;
                }
            }
            if( !pBufHdl->requestBuffer(nodedatatype, allocinfo) )
            {
                MY_LOGE("request buffer failed: data %d", nodedatatype);
                goto lbExit;
            }
        }
        //
        pConfig++;
    }
    //
    ret = MTRUE;
lbExit:
    if( !ret ) {
        MY_LOGE("allocBuffers failed");
    }
    FUNC_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
void*
StereoCtrlNodeImpl::
doThreadAlloc(void* arg)
{
    ::prctl(PR_SET_NAME,"allocThread", 0, 0, 0);
    StereoCtrlNodeImpl* pSelf = reinterpret_cast<StereoCtrlNodeImpl*>(arg);
    return (void*)pSelf->alloc();
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    // Shane DEBUG
    MINT32  mSensorId_test_MAIN = -1;
    MINT32  mSensorId_test_SUB = -1;
    MINT32  mSensorId_test_MAIN2 = -1;
    String8 const debugIDKey_MAIN("MTK_SENSOR_DEV_MAIN");
    String8 const debugIDKey_MAIN2("MTK_SENSOR_DEV_MAIN_2");
    String8 const debugIDKey_SUB("MTK_SENSOR_DEV_SUB");
    Utils::Property::tryGet(debugIDKey_MAIN, mSensorId_test_MAIN);
    Utils::Property::tryGet(debugIDKey_MAIN2, mSensorId_test_MAIN2);
    Utils::Property::tryGet(debugIDKey_SUB, mSensorId_test_SUB);
    MY_LOGD("StereoCtrlNode : onInit() : mSensorId_test_MAIN = %d ",mSensorId_test_MAIN);
    MY_LOGD("StereoCtrlNode : onInit() : mSensorId_test_MAIN2 = %d ",mSensorId_test_MAIN2);
    MY_LOGD("StereoCtrlNode : onInit() : mSensorId_test_SUB = %d ",mSensorId_test_SUB);

    init();
    //
    ret = MTRUE;

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = uninit();
    // if(mpISC_Main)
    // {
    //     mpISC_Main->destroyInstance();
    //     mpISC_Main = NULL;
    // }
    // if(mpISC_Main2)
    // {
    //     mpISC_Main2->destroyInstance();
    //     mpISC_Main2 = NULL;
    // }

    // if ( mpStereoHal )
    // {
    //     mpStereoHal->destroyInstance();
    //     mpStereoHal = NULL;
    // }
    //

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    if( pthread_create(&mThreadAlloc, NULL, doThreadAlloc, this) != 0 )
    {
        MY_LOGE("pthread create failed");
        goto lbExit;
    }
    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onStop()
{
    Mutex::Autolock lock(mLock);
    FUNC_START;
    void* threadRet = NULL;
    list<PostBufInfo>::iterator bufIter;
    list<FeoBufInfo>::iterator feoIter;
    //
#define RET_BUFFER( postbuf, iter )                                             \
    for(iter = postbuf.begin(); iter != postbuf.end(); iter++)                  \
    {                                                                           \
        MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)", (*iter).data, (*iter).buf); \
        handleReturnBuffer((*iter).data, (MUINTPTR)((*iter).buf));               \
    }

    RET_BUFFER(mlPostBufMain, bufIter)
    RET_BUFFER(mlPostBufImg, bufIter)
    RET_BUFFER(mlPostBufImg_Main2, bufIter)
    RET_BUFFER(mlPostBufFeo, bufIter)
    RET_BUFFER(mlPostBufFeo_Main2, bufIter)

#undef RET_BUFFER
    if ( !mbAllocDone )
    {
        waitThreadDone();
    }

    FUNC_END;
    return syncWithThread(); //wait for jobs done
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
//	    if(isCapturePath())
    {
        MY_LOGD_IF(StereoSettingProvider::bEnableLog,
                    "SCNode:onPostBuffer sensor=%d, data=%d isCapture=%d",
                    getSensorIdx(), data, isCapturePath());
    }
    return pushBuf(data, buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL ret = MTRUE;
    MY_LOGD("data %d, buf 0x%x ext 0x%08X", data, buf, ext);
    switch(data)
    {
        case STEREO_CTRL_MAIN_DST:
            if(isCapturePath())
            {
                // return buffer to buffer handler
                ICamBufHandler* pBufHdl = getBufferHandler(STEREO_CTRL_MAIN_DST);
                IImageBuffer* pBuf = (IImageBuffer*)buf;
                if( !pBufHdl )
                {
                    MY_LOGE("no buffer hdl for data %d, buf 0x%x", data, buf);
                    return MFALSE;
                }
                pBufHdl->enqueBuffer(STEREO_CTRL_MAIN_DST,pBuf);
            }
            else
            {
                // return buffer to last node
                handleReturnBuffer(STEREO_CTRL_MAIN_SRC, buf);
            }
            break;
        case STEREO_CTRL_DST_M:
            if(isPreviewPath())
            {
                // return buffer to buffer handler
                ICamBufHandler* pBufHdl = getBufferHandler(STEREO_CTRL_DST_M);
                IImageBuffer* pBuf = (IImageBuffer*)buf;
                if( !pBufHdl )
                {
                    MY_LOGE("no buffer hdl for data %d, buf 0x%x", data, buf);
                    return MFALSE;
                }
                pBufHdl->enqueBuffer(STEREO_CTRL_DST_M,pBuf);
            }
            else
            {
                // return buffer to last node
                handleReturnBuffer(STEREO_CTRL_IMG_0, buf);
            }
            break;
        case STEREO_CTRL_DEPTHMAP_DST:
            if(isCapturePath())
            {
                // return buffer to buffer handler
                ICamBufHandler* pBufHdl = getBufferHandler(STEREO_CTRL_DEPTHMAP_DST);
                IImageBuffer* pBuf = (IImageBuffer*)buf;
                if( !pBufHdl )
                {
                    MY_LOGE("no buffer hdl for data %d, buf 0x%x", data, buf);
                    return MFALSE;
                }
                pBufHdl->enqueBuffer(STEREO_CTRL_DEPTHMAP_DST,pBuf);
            }
            else
            {
                MY_LOGE("something wrong, should not receive STEREO_CTRL_DEPTHMAP_DST in preview path: %d", data);
            }
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    //
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
StereoCtrlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
    char dumppath[256];
    sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt, cnt)                \
        do{                                                         \
            IImageBuffer* buffer = (IImageBuffer*)pbuf;             \
            char filename[256];                                     \
            sprintf(filename, "%s%s_%d_%dx%d_%d.%s",                \
                    dumppath,                                       \
                    #type,                                          \
                    getSensorIdx(),                                 \
                    buffer->getImgSize().w,buffer->getImgSize().h,  \
                    cnt,                                            \
                    fileExt                                         \
                   );                                               \
            buffer->saveToFile(filename);                           \
        }while(0)

    if(!makePath(dumppath,0660))
    {
        MY_LOGE("makePath [%s] fail",dumppath);
        return;
    }

    switch( data )
    {
        case STEREO_CTRL_IMG_0:
            DUMP_IImageBuffer( STEREO_CTRL_IMG_0, buf, "yuv", muPostFrameCnt );
            break;
        case STEREO_CTRL_IMG_1:
            DUMP_IImageBuffer( STEREO_CTRL_IMG_1, buf, "yuv", muPostFrameCnt );
            break;
        case STEREO_CTRL_RGB_0:
            DUMP_IImageBuffer( STEREO_CTRL_RGB_0, buf, "rgb", muPostFrameCnt );
            break;
        case STEREO_CTRL_RGB_1:
            DUMP_IImageBuffer( STEREO_CTRL_RGB_1, buf, "rgb", muPostFrameCnt );
            break;
        default:
            MY_LOGE("not handle this yet data(%d)", data);
            break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
pushBuf(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);
    MUINT32 postBufSize = 0;
    MUINT32 memID = 0;

#define push_case(data, key, src, dst)          \
    case data:                                  \
        dst.push_back(src);                     \
        postBufSize = dst.size();               \
        mSrcDataSet |= key;                     \
        break;

    if ( data == STEREO_CTRL_FEO_0 || data == STEREO_CTRL_FEO_1 )
    {
        if(isCapturePath()){
            // FeoBufInfo feoBufData(data, (IMEM_BUF_INFO*)buf, ext);
            PostBufInfo postBufData(data, (IImageBuffer*)buf, ext);
            memID = postBufData.buf->getFD();
            switch (data)
            {
                push_case(STEREO_CTRL_FEO_0, FEO_0, postBufData, mlPostBufFeo);
                push_case(STEREO_CTRL_FEO_1, FEO_1, postBufData, mlPostBufFeo_Main2);
                default:
                    MY_LOGW("unsupported data(%d)", data);
                break;
            }
        }else{
            handlePostBuffer( data , (MUINTPTR)buf, 0 );
        }
    }
    else
    {
        PostBufInfo postBufData(data, (IImageBuffer*)buf, ext);
        memID = postBufData.buf->getFD();
        switch (data)
        {
            push_case(STEREO_CTRL_MAIN_SRC, MAIN,   postBufData, mlPostBufMain);
            push_case(STEREO_CTRL_IMG_0,    IMG_0,  postBufData, mlPostBufImg);
            push_case(STEREO_CTRL_IMG_1,    IMG_1,  postBufData, mlPostBufImg_Main2);
            default:
                MY_LOGW("unsupported data(%d)", data);
                break;
        }
    }
#undef push_case

    // MY_LOGD("size(%d), data(%d), buf(0x%x), ID(%d), mlPostBufImg.size()=%d,mlPostBufImg_Main2.size()=%d", postBufSize, data, buf, memID,mlPostBufImg.size(),mlPostBufImg_Main2.size());
    // MY_LOGD("isReadyToAlgo() sizes: mlPostBufMain     =%d",mlPostBufMain.size());
    // MY_LOGD("isReadyToAlgo() sizes: mlPostBufImg      =%d",mlPostBufImg.size());
    // MY_LOGD("isReadyToAlgo() sizes: mlPostBufImg_Main2=%d",mlPostBufImg_Main2.size());
    // MY_LOGD("isReadyToAlgo() sizes: mlPostBufRgb      =%d",mlPostBufRgb.size());
    // MY_LOGD("isReadyToAlgo() sizes: mlPostBufRgb_Main2=%d",mlPostBufRgb_Main2.size());
    // MY_LOGD("isReadyToAlgo() sizes: mlPostBufFeo      =%d",mlPostBufFeo.size());
    // MY_LOGD("isReadyToAlgo() sizes: mlPostBufFeo_Main2=%d",mlPostBufFeo_Main2.size());

    MY_LOGD("data=%d , mSrcDataSet=0x%x",data, mSrcDataSet);

    if( isReadyToAlgo() )
    {
        triggerLoop();
        resetSrcDataSet();
    }

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
isReadyToAlgo() const
{
    MBOOL ret = MFALSE;

    MY_LOGD("isReadyToAlgo() getStereoType()=%d",getStereoType());

    switch ( getStereoType() )
    {
        case STEREO_CTRL_PREVIEW: //1
        case STEREO_CTRL_RECORD:  //2
            // ret = ( PREVIEW_SRC == getSrcDataSet() );
            if(mlPostBufImg.size() > 0 && mlPostBufImg_Main2.size() > 0){
                ret = true;
            }
            break;
        case STEREO_CTRL_CAPTURE: //4
        case STEREO_CTRL_ZSD:     //8
            ret = ( CAPTURE_SRC == getSrcDataSet() );
            break;
        default:
            MY_LOGE("unsupported stereo type(%d)", getStereoType());
            break;
    }
    MY_LOGD("isReadyToAlgo() exit: getStereoType()=%d, ret=%d",getStereoType(), ret);
    return ret;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoCtrlNodeImpl::
threadLoopUpdate()
{
    MY_LOGD("StereoCtrlNodeImpl::threadLoopUpdate() start");
    MBOOL ret = MTRUE;
    //
    MBOOL isZoom = MFALSE;
    MUINT32 magicNum = 0;
    SET_DATA_STEREO_T sDataIn;
    OUT_DATA_STEREO_T sDataOut;
    PostBufInfo postBufMain;
    PostBufInfo postBufImg, postBufImg_Main2;
    {
        Mutex::Autolock lock(mLock);
        if ( mlPostBufImg.size() == 0 || mlPostBufImg_Main2.size() == 0)
        {
            MY_LOGW("skip threadloop: img(%d) img2(%d) feo(%d) feo2(%d)",
                mlPostBufImg.size(), mlPostBufImg_Main2.size(), mlPostBufFeo.size(), mlPostBufFeo_Main2.size());
            return ret;
        }
        if ( isCapturePath() )
        {
            if ( mlPostBufMain.size() == 0)
            {
                MY_LOGW("skip threadloop: main(%d)", mlPostBufMain.size());
                return ret;
            }
        }
        //
        postBufImg            = mlPostBufImg.front();
        postBufImg_Main2      = mlPostBufImg_Main2.front();

        mlPostBufImg.pop_front();
        mlPostBufImg_Main2.pop_front();
        // for preview data path
        if(isPreviewPath())
        {
            ICamBufHandler* pBufHdl = getBufferHandler(STEREO_CTRL_DST_M);
            ImgRequest outRequest;
            IImageBuffer* pDstBuf = NULL;
            if( !pBufHdl->dequeBuffer(STEREO_CTRL_DST_M, &outRequest) )
            {
                MY_LOGE("deque buffer failed: data %d", STEREO_CTRL_DST_M);
                goto lbExit;
            }
            pDstBuf = const_cast<IImageBuffer*>(outRequest.mBuffer);

            sDataIn.eScenario = STEREO_SCENARIO_PREVIEW;
            sDataIn.AlgoSrcBuf_main1_prv_rrz = (void*)postBufImg.buf;
            sDataIn.AlgoSrcBuf_main2_prv_rrz = (void*)postBufImg_Main2.buf;
            sDataIn.AlgoDstBuf_prv = (void*)pDstBuf;

            // setup stereo hal parameters
            if( !mpStereoHal || !mpStereoHal->STEREOSetParams(sDataIn) )
            {
                MY_LOGE("STEREOSetParams fail");
                goto lbExit;
            }

            // run stereo hal algo
            sDataOut.eScenario = STEREO_SCENARIO_PREVIEW;
            if( !mpStereoHal || !mpStereoHal->STEREORun(sDataOut, !isZoom) )
            {
                MY_LOGE("STEREORun fail");
                goto lbExit;
            }

            // send stereo algo result to next node
            pDstBuf->setTimestamp(postBufImg.buf->getTimestamp());      // update time stamp
            handlePostBuffer( STEREO_CTRL_DST_M , (MUINTPTR)pDstBuf, 0 );   // post preview image to next node

            // return buffer to last node
            handleReturnBuffer(STEREO_CTRL_IMG_0, (MUINTPTR)postBufImg.buf);
            handleReturnBuffer(STEREO_CTRL_IMG_1, (MUINTPTR)postBufImg_Main2.buf);
            muPostFrameCnt++;
            return ret;
        }
        else if ( isCapturePath() )
        {
            if (getSensorIdx() == 0)
            {
                MY_LOGD("Stereo Capture sensor timestamp (timediff) : buf1: %lld , buf2: %lld ,",
                postBufImg.buf->getTimestamp() ,
                postBufImg_Main2.buf->getTimestamp() );
            }

            // for capture data path
            if ( !mbAllocDone )
            {
                waitThreadDone();
            }
            // get main1 full size image
            postBufMain     = mlPostBufMain.front();
            mpMainImageBuf = postBufMain.buf;
            mlPostBufMain.pop_front();

            ICamBufHandler* pBufHdl = getBufferHandler(STEREO_CTRL_MAIN_DST);
            ImgRequest BokehoutRequest;
            if( !pBufHdl->dequeBuffer(STEREO_CTRL_MAIN_DST, &BokehoutRequest) )
            {
                MY_LOGE("deque bokeh out buffer failed: data %d", STEREO_CTRL_MAIN_DST);
                goto lbExit;
            }
            ImgRequest DepthmapoutRequest;
            if( !pBufHdl->dequeBuffer(STEREO_CTRL_DEPTHMAP_DST, &DepthmapoutRequest) )
            {
                MY_LOGE("deque depthmap out buffer failed: data %d", STEREO_CTRL_DEPTHMAP_DST);
                goto lbExit;
            }

            sDataIn.eScenario                     = STEREO_SCENARIO_CAPTURE;
            sDataIn.AlgoSrcBuf_main1_cap_fullsize = (void*)mpMainImageBuf;
            sDataIn.AlgoSrcBuf_main1_cap_rrz      = (void*)postBufImg.buf;
            sDataIn.AlgoSrcBuf_main2_cap_rrz      = (void*)postBufImg_Main2.buf;
            IImageBuffer* pAlgo_dst_buf           = const_cast<IImageBuffer*>(BokehoutRequest.mBuffer);
            sDataIn.AlgoDstBuf_cap                = (void*)pAlgo_dst_buf;
            IImageBuffer* pDepthmap_dst_buf       = const_cast<IImageBuffer*>(DepthmapoutRequest.mBuffer);
            sDataIn.DepthmapDstBuf_cap            = (void*)pDepthmap_dst_buf;

            // process algo in STEREO_HAL
            {
                Mutex::Autolock lock(mAlgoLock);
                if( !mbEnable )
                {
                    MY_LOGD("bypass STEREO HAL");
                    goto lbExit;
                }
                MY_LOGD("isCapturePath()");

                MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREOSetParams +");
                if( !mpStereoHal || !mpStereoHal->STEREOSetParams(sDataIn) )
                {
                    MY_LOGE("STEREOSetParams fail");
                    goto lbExit;
                }
                MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREOSetParams -");

                MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREORun +");
                sDataOut.eScenario = STEREO_SCENARIO_CAPTURE;
                if( !mpStereoHal || !mpStereoHal->STEREORun(sDataOut, !isZoom) )
                {
                    MY_LOGE("STEREORun fail");
                    goto lbExit;
                }
                MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREORun -");
            }

            // need update time stamp for output buffer, or it will cause error operation for call back node
            pAlgo_dst_buf->setTimestamp(mpMainImageBuf->getTimestamp());
            mpMainImageBuf->setTimestamp(mpMainImageBuf->getTimestamp());
            pDepthmap_dst_buf->setTimestamp(mpMainImageBuf->getTimestamp());

            // post buffer (send buffer to next nodes)
            handlePostBuffer( STEREO_CTRL_MAIN_DST , (MUINTPTR)pAlgo_dst_buf, 0 );      // post bokeh image
            handlePostBuffer( STEREO_CTRL_DST_M , (MUINTPTR)mpMainImageBuf, 0 );        // post clean image
            handlePostBuffer( STEREO_CTRL_DEPTHMAP_DST , (MUINTPTR)pDepthmap_dst_buf, 0 );   // post depthmap

            // return buffer
            handleReturnBuffer(postBufImg_Main2.data, (MUINTPTR)postBufImg_Main2.buf);  // return main2 rrz YUV image
            handleReturnBuffer(postBufImg.data, (MUINTPTR)postBufImg.buf);              // return main1 rrz YUV image

            muPostFrameCnt++;
        }
    }

    return ret;

lbExit:
    handleReturnBuffer(postBufImg.data,         (MUINTPTR)postBufImg.buf);
    handleReturnBuffer(postBufImg_Main2.data,   (MUINTPTR)postBufImg_Main2.buf);
    if ( isCapturePath() )
    {
        handleReturnBuffer(postBufMain.data,        (MUINTPTR)postBufMain.buf);
    }
    return MFALSE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

