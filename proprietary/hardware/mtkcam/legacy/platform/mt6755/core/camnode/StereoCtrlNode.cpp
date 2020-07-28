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
#include <mtkcam/utils/imagebuf/IGrallocImageBufferHeap.h>
#include <ui/gralloc_extra.h>
#include <vndk/hardware_buffer.h>
//
#include <cutils/properties.h>

// For capture Face detection
#include <DpBlitStream.h>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)

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
#define CAPTURE_SRC     (FEO_0|FEO_1|IMG_0|IMG_1|RGB_0|RGB_1|MAIN)
// Shane 20141211 In this platform, there is no FEO data
#define PREVIEW_SRC     (IMG_0|IMG_1)
// #define CAPTURE_SRC     (IMG_0|IMG_1|RGB_0|RGB_1|MAIN)
/*******************************************************************************
 *
 ********************************************************************************/
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

        bool           doFaceDetection();
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
        static void*    doThreadAlloc(void* arg);
        static void*    doThreadDoFDDetection(void* arg);
        bool            alloc();
        bool            waitThreadDone();

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
        list<PostBufInfo>       mlPostBufRgb;
        list<PostBufInfo>       mlPostBufRgb_Main2;
        // for capture fd use.
        IImageBuffer*           mpMainImageBuf;
        //
        IImageBuffer*           mpAlgoSrcImgBuf;
        IImageBuffer*           mpAlgoDstImgBuf;
        IImageBuffer*           mpFDSrcImgBuf;
        MBOOL                   mbAllocDone;
        pthread_t               mThreadAlloc;
        pthread_t               mThreadDoFDDetection;
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
    //delete this;
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
    , mpAlgoSrcImgBuf(NULL)
    , mpAlgoDstImgBuf(NULL)
    , mpMainImageBuf(NULL)
    , mpFDSrcImgBuf(NULL)
    , mThreadAlloc(NULL)
    , mThreadDoFDDetection(NULL)
    , mbAllocDone(MFALSE)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_FEO_0 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_FEO_1 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_IMG_0 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_IMG_1 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_RGB_0 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_RGB_1 );
    addDataSupport( ENDPOINT_SRC, STEREO_CTRL_MAIN_SRC );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_DST_M );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_DST_S );
    addDataSupport( ENDPOINT_DST, STEREO_CTRL_MAIN_DST );
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
    mlPostBufRgb.clear();
    mlPostBufRgb_Main2.clear();
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
        mpISC_Main->setRrzoMinSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);
        mpISC_Main->setPreviewSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);
        mpISC_Main->setPass1InitRrzoSize(sRrzData.rrz_size_main1.w, sRrzData.rrz_size_main1.h);
        mpISC_Main->setInitialRrzoSize(sRrzData.rrz_crop_main1, sRrzData.rrz_size_main1);

        mpISC_Main2->setVideoSize(0, 0);
        mpISC_Main2->setRrzoMinSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);
        mpISC_Main2->setPreviewSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);
        mpISC_Main2->setPass1InitRrzoSize(sRrzData.rrz_size_main2.w, sRrzData.rrz_size_main2.h);
        mpISC_Main2->setInitialRrzoSize(sRrzData.rrz_crop_main2, sRrzData.rrz_size_main2);

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
    MY_LOGD("stereo type(0x%x)", getStereoType());
    if ( isCapturePath() )
    {
        MSize const     imgSize = MSize(mInitCfg.mAlgoImgWidth, mInitCfg.mAlgoImgHeight);
        MINT32 const    usage   = eBUFFER_USAGE_HW_RENDER|eBUFFER_USAGE_HW_TEXTURE|eBUFFER_USAGE_SW_WRITE_RARELY|eBUFFER_USAGE_SW_READ_RARELY;
        MUINT32 const   format  = eImgFmt_RGBA8888;
        MUINT32 const planeCount= queryPlaneCount(format);
        MUINT32 bufStridesInBytes[] = {0,0,0};
        MINT32 bufBoundaryInBytes[] = {0,0,0};
        for (MUINT32 i = 0; i < planeCount; i++)
        {
            bufStridesInBytes[i] =
                (queryPlaneWidthInPixels(format,i, imgSize.w)*queryPlaneBitsPerPixel(format,i))>>3;
        }
        IImageBufferAllocator::ImgParam imgParam(
                format,
                imgSize,
                bufStridesInBytes,
                bufBoundaryInBytes,
                planeCount);
        IImageBufferAllocator::ExtraParam extParam(usage);
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        mpAlgoDstImgBuf = allocator->alloc_gb(LOG_TAG, imgParam, extParam);


        // Shane 201501131343
        // Enable GPU full mode
        AHardwareBuffer* temp;
        IImageBufferHeap* pDstHeap  = mpAlgoDstImgBuf->getImageBufferHeap();
        temp = (AHardwareBuffer*)(pDstHeap->getHWBuffer());

        gralloc_extra_ion_sf_info_t info;
        gralloc_extra_query(AHardwareBuffer_getNativeHandle(temp), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
        gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
        gralloc_extra_perform(AHardwareBuffer_getNativeHandle(temp), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);

        // Debug
        gralloc_extra_query(AHardwareBuffer_getNativeHandle(temp), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
        /* remap to MDP enum */
        switch (info.status & GRALLOC_EXTRA_MASK_YUV_COLORSPACE)
        {
            case GRALLOC_EXTRA_BIT_YUV_NOT_SET:  /* set to BT601_REDUCE as default */
            case GRALLOC_EXTRA_BIT_YUV_BT601_NARROW:
                MY_LOGD("GRALLOC_EXTRA_BIT_YUV_BT601_NARROW");
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT601_FULL:
                MY_LOGD("GRALLOC_EXTRA_BIT_YUV_BT601_FULL");
                break;
            case GRALLOC_EXTRA_BIT_YUV_BT709_NARROW:
                MY_LOGD("GRALLOC_EXTRA_BIT_YUV_BT709_NARROW");
                break;
        }


        mpAlgoDstImgBuf->lockBuf(LOG_TAG, usage);
        CHECK_OBJECT(mpAlgoDstImgBuf);

        // FD src img
        MY_LOGD("[stereo captureFD] prepare FD src img +");
        // Here we allocate 640x640 because FD HW need memory alignment(though the image will be scale down to 640x480 or 640x360)
        // If you want to know the detail please consult FD owners
        MSize const     imgSize_fd      = MSize(640, 640);
        MUINT32 const   format_fd       = eImgFmt_YUY2;
        MUINT32 const   planeCount_fd   = queryPlaneCount(format_fd);
        MUINT32 bufStridesInBytes_fd[] = {0,0,0};
        MINT32 bufBoundaryInBytes_fd[] = {0,0,0};
        for (MUINT32 i = 0; i < planeCount_fd; i++)
        {
            bufStridesInBytes_fd[i] =
                (queryPlaneWidthInPixels(format_fd,i, imgSize_fd.w)*queryPlaneBitsPerPixel(format_fd,i))>>3;
        }
        IImageBufferAllocator::ImgParam imgParam_fd(
                format_fd,
                imgSize_fd,
                bufStridesInBytes_fd,
                bufBoundaryInBytes_fd,
                planeCount_fd);
        IImageBufferAllocator* allocator_fd = IImageBufferAllocator::getInstance();
        mpFDSrcImgBuf = allocator_fd->alloc_ion(LOG_TAG, imgParam_fd);
        mpFDSrcImgBuf->lockBuf(LOG_TAG, usage);
        CHECK_OBJECT(mpFDSrcImgBuf);
        MY_LOGD("[stereo captureFD] prepare FD src img -");
    }
    return MTRUE;
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


/******************************************************************************
 *
 ******************************************************************************/
void*
StereoCtrlNodeImpl::
doThreadDoFDDetection(void* arg)
{
    ::prctl(PR_SET_NAME,"doFDDetectionThread", 0, 0, 0);
    StereoCtrlNodeImpl* pSelf = reinterpret_cast<StereoCtrlNodeImpl*>(arg);
    return (void*)pSelf->doFaceDetection();
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
    //
    // StereoSettingProvider::getStereoSensorIndex(mSensorId_Main, mSensorId_Main2);

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

    // mpISC_Main = IspSyncControlHw::createInstance( getOpenId_Main() );
    // mpISC_Main2= IspSyncControlHw::createInstance( getOpenId_Main2() );
    // if( !mpISC_Main || !mpISC_Main2)
    // {
    //     MY_LOGE("create IspSyncControlHw failed");
    //     goto lbExit;
    // }
    // //
    // mpStereoHal = StereoHalBase::createInstance();
    init();
    //
    ret = MTRUE;
// lbExit:
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
    RET_BUFFER(mlPostBufRgb, bufIter)
    RET_BUFFER(mlPostBufRgb_Main2, bufIter)
    // RET_BUFFER(mlPostBufFeo, feoIter)
    // RET_BUFFER(mlPostBufFeo_Main2, feoIter)
    RET_BUFFER(mlPostBufFeo, bufIter)
    RET_BUFFER(mlPostBufFeo_Main2, bufIter)

#undef RET_BUFFER
    if ( !mbAllocDone )
    {
        waitThreadDone();
    }
    if ( mpAlgoDstImgBuf != NULL )
    {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        mpAlgoDstImgBuf->unlockBuf(LOG_TAG);
        allocator->free(mpAlgoDstImgBuf);
        mpAlgoDstImgBuf = NULL;
    }
    if ( mpFDSrcImgBuf != NULL )
    {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        mpFDSrcImgBuf->unlockBuf(LOG_TAG);
        allocator->free(mpFDSrcImgBuf);
        mpFDSrcImgBuf = NULL;
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
    if(isCapturePath()){
        MY_LOGD_IF(StereoSettingProvider::bEnableLog, "SCNode:onPostBuffer sensor=%d, data=%d isCapturePath", getSensorIdx(), data);
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
            handleReturnBuffer(STEREO_CTRL_MAIN_SRC, buf);
            break;
        case STEREO_CTRL_DST_M:
            handleReturnBuffer(STEREO_CTRL_IMG_0, buf);
            break;
        case STEREO_CTRL_DST_S:
            if ( isCapturePath() )
            {
                handleReturnBuffer(STEREO_CTRL_IMG_1, (MUINTPTR)mpAlgoSrcImgBuf);
            }
            else
            {
                handleReturnBuffer(STEREO_CTRL_IMG_1, buf);
            }
            break;
        case STEREO_CTRL_RGB_0:
        case STEREO_CTRL_RGB_1:
        case STEREO_CTRL_FEO_0:
        case STEREO_CTRL_FEO_1:
            handleReturnBuffer(data, buf);
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
            push_case(STEREO_CTRL_RGB_0,    RGB_0,  postBufData, mlPostBufRgb);
            push_case(STEREO_CTRL_RGB_1,    RGB_1,  postBufData, mlPostBufRgb_Main2);
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
    return ret;
}
/*******************************************************************************
 *
 ********************************************************************************/
bool
StereoCtrlNodeImpl::
doFaceDetection(){
    MY_LOGD("[stereo captureFD] +");
    IImageBuffer* Srcbufinfo = mpMainImageBuf;
    // MDP variables
    DpBlitStream* pStream;
    int32_t       srcWidth;
    int32_t       srcHeight;
    int32_t       dstWidth;
    int32_t       dstHeight;
    void           *srcBuffer[3];
    void           *dstBuffer[3];
    uint32_t       srcSize[3];
    uint32_t       dstSize[3];

    // FD variables
    MUINT8* m_pWorkBuf = 0;
    halFDBase* mpFDHalObj = 0;
    MUINT8* mpFDWorkingBuffer = 0;
    MUINT8* qVGABuffer = new unsigned char[640*480*2];
    MINT32 numFace;
    MUINT8 count = 0;
    MtkCameraFace doFDFaceInfo[15];
    MtkFaceInfo doFDPoseInfo[15];
    MtkCameraFaceMetadata  doFDFaceMetadata;
    doFDFaceMetadata.faces=(MtkCameraFace *)doFDFaceInfo;
    doFDFaceMetadata.posInfo=(MtkFaceInfo *)doFDPoseInfo;
    vector<FD_DATA_STEREO_T> outFDFaceInfos;
    MUINT32 mTransform = mInitCfg.mTransform;
    MINT32 rotation = (mTransform == 0)? 0:
                            ((mTransform == NSCam::eTransform_ROT_90)? 90:
                            ((mTransform == NSCam::eTransform_ROT_180)? 180: 270));

    // MDP I/O sizes
    srcWidth = Srcbufinfo->getImgSize().w;
    srcHeight = Srcbufinfo->getImgSize().h;
    dstWidth = mpFDSrcImgBuf->getImgSize().w;
    dstHeight = 360;//mpFDSrcImgBuf->getImgSize().h;
    MY_LOGD("[stereo captureFD] MDP in(%dx%d) out(%dx%d)", srcWidth, srcHeight, dstWidth, dstHeight);

    // Use MDP to scale down the image for FD hal
    MY_LOGD("[stereo captureFD] prepare MDP");
    pStream = new DpBlitStream();
    // src data
    srcBuffer[0] =  (void*)Srcbufinfo->getBufVA(0);
    srcBuffer[1] =  0;
    srcBuffer[2] =  0;
    srcSize[0] = srcWidth*srcHeight*2; // 2 bytes per stride for YUY2 => width*2*height
    srcSize[1] = 0;
    srcSize[2] = 0;
    pStream->setSrcBuffer(srcBuffer, srcSize, 1);
    pStream->setSrcConfig(srcWidth, srcHeight, DP_COLOR_YUY2);
    // dst data
    dstBuffer[0] = (void*)mpFDSrcImgBuf->getBufVA(0);
    dstBuffer[1] = 0;
    dstBuffer[2] = 0;
    dstSize[0] = dstWidth*dstHeight*2; // 2 bytes per stride for YUY2 => width*2*height
    dstSize[1] = 0;
    dstSize[2] = 0;
    pStream->setDstBuffer(dstBuffer, dstSize, 1);
    pStream->setDstConfig(dstWidth, dstHeight, DP_COLOR_YUY2);

    // run MDP
    MY_LOGD("[stereo captureFD] run MDP +");
    pStream->invalidate();
    MY_LOGD("[stereo captureFD] run MDP -");

    delete pStream;

    // dump MDP result
    // char filename[256];
    // IImageBuffer* buffer = (IImageBuffer*)mpFDSrcImgBuf;
    // sprintf(filename, "/sdcard/mtklog/cfd_dump/test.yuv");
    // if(!makePath("/sdcard/mtklog/cfd_dump/",0660))
    // {
    //     MY_LOGE("ShaneTest makePath [%s] fail","/sdcard/mtklog/cfd_dump/");
    //     return MTRUE;
    // }
    // buffer->saveToFile(filename);

    // Do FD
    MY_LOGD("[stereo captureFD] do FD rotation=%d", rotation);
    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_HW);
    if (mpFDHalObj == NULL)
    {
        MY_LOGE("[stereo captureFD] can't get halFDBase instance.");
        delete [] qVGABuffer;
        return false;
    }
    mpFDWorkingBuffer = new unsigned char[1024*1024*10];
    mpFDHalObj->halFDInit(dstWidth, dstHeight, mpFDWorkingBuffer, 1024*1024*10, 1, 0);

    m_pWorkBuf = (MUINT8*)malloc(dstWidth*dstHeight) ;
    mpFDHalObj->halFDYUYV2ExtractY(m_pWorkBuf, (MUINT8*)mpFDSrcImgBuf->getBufVA(0), dstWidth, dstHeight);

    do {
        MY_LOGD("[stereo captureFD] Face detection try %d", count + 1);
        mpFDHalObj->halFDDo(0, qVGABuffer, m_pWorkBuf, false, rotation, (MUINT8*)mpFDSrcImgBuf->getBufPA(0));
        numFace = mpFDHalObj->halFDGetFaceResult(&doFDFaceMetadata);
    } while ((numFace == 0) && (++count <= 3));

    MY_LOGD("[stereo captureFD] FD Done");

    delete qVGABuffer;
    mpFDHalObj->halFDUninit();
    mpFDHalObj->destroyInstance();
    delete mpFDWorkingBuffer;
    free(m_pWorkBuf);

    if (numFace == 0)
    {
        MY_LOGD("[stereo captureFD] No face is detected on captured image, use preview FD info");
    }
    else
    {
        MY_LOGD("[stereo captureFD] %d faces are detected on captured image", doFDFaceMetadata.number_of_faces);
        for(int i = 0; i < numFace; i++)
        {
            FD_DATA_STEREO_T outFDFaceInfo;
            //     MY_LOGD("[stereo captureFD] %d faces are detected on captured image", doFDFaceMetadata.number_of_faces);
                // MY_LOGD("[stereo captureFD] rect(%d, %d, %d, %d) rop(%d) rip(%d)",
                //     doFDFaceMetadata.faces[i].rect[0],
                //     doFDFaceMetadata.faces[i].rect[1],
                //     doFDFaceMetadata.faces[i].rect[2],
                //     doFDFaceMetadata.faces[i].rect[3],
                //     doFDFaceMetadata.posInfo[i].rop_dir,
                //     doFDFaceMetadata.posInfo[i].rip_dir
            //     );
            // }
            // Rotate FD resuilt according to image rotation
            // We use the first result only(The firtst result in the list means it is the closest one to the image center)
            // We also convert it to be in the size of src image
            // ex: (1000,1000) => (3072, 0), (1000,-1000)=>(0,1728)
            switch(mTransform){
                case 0:{
                    // rotation = 0 ; mirror = 0          
                    outFDFaceInfo.left = (MUINT32)(doFDFaceMetadata.faces[i].rect[0]);
                    outFDFaceInfo.top = (MUINT32)(doFDFaceMetadata.faces[i].rect[1]);
                    outFDFaceInfo.right = (MUINT32)(doFDFaceMetadata.faces[i].rect[2]);
                    outFDFaceInfo.bottom = (MUINT32)(doFDFaceMetadata.faces[i].rect[3]);
                    break;
                }
                case 4:{
                    // rotation = 90 ; mirror = 0
                    outFDFaceInfo.left = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[3]));
                    outFDFaceInfo.top = (MUINT32)(doFDFaceMetadata.faces[i].rect[0]);
                    outFDFaceInfo.right = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[1]));
                    outFDFaceInfo.bottom = (MUINT32)(doFDFaceMetadata.faces[i].rect[2]);
                    break;
                }
                case 3:{
                    // rotation = 180 ; mirror = 0
                    outFDFaceInfo.left = (MUINT32)(0- (doFDFaceMetadata.faces[i].rect[2]));
                    outFDFaceInfo.top = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[3]));
                    outFDFaceInfo.right = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[0]));
                    outFDFaceInfo.bottom = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[1]));
                    break;
                }
                case 7:{
                    // rotation = 270 ; mirror = 0           
                    outFDFaceInfo.left = (MUINT32)(doFDFaceMetadata.faces[i].rect[1]);
                    outFDFaceInfo.top = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[2]));
                    outFDFaceInfo.right = (MUINT32)(doFDFaceMetadata.faces[i].rect[3]);
                    outFDFaceInfo.bottom = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[0]));
                    break;
                }
                case 1:{
                    // rotation = 0 ; mirror = 1           
                    outFDFaceInfo.left = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[2]));
                    outFDFaceInfo.top = (MUINT32)(doFDFaceMetadata.faces[i].rect[1]);
                    outFDFaceInfo.right = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[0]));
                    outFDFaceInfo.bottom = (MUINT32)(doFDFaceMetadata.faces[i].rect[3]);
                    break;
                }
                case 2:{
                    // rotation = 180 ; mirror = 1
                    // => equal to rotaion = 0; flip = 1         
                    outFDFaceInfo.left = (MUINT32)(doFDFaceMetadata.faces[i].rect[0]);
                    outFDFaceInfo.top = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[3]));
                    outFDFaceInfo.right = (MUINT32)(doFDFaceMetadata.faces[i].rect[2]);
                    outFDFaceInfo.bottom = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[1]));
                    break;
                }
                case 5:{
                    // rotation = 270 ; mirror = 1
                    // => equal to rotaion = 90 ; mirror = 1         
                    outFDFaceInfo.left = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[3]));
                    outFDFaceInfo.top = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[2]));
                    outFDFaceInfo.right = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[1]));
                    outFDFaceInfo.bottom = (MUINT32)(0 - (doFDFaceMetadata.faces[i].rect[0]));
                    break;
                }
                case 6:{
                    // rotation = 90 ; mirror = 1
                    // => equal to rotaion = 90 ; flip = 1         
                    outFDFaceInfo.left = (MUINT32)(doFDFaceMetadata.faces[i].rect[1]);
                    outFDFaceInfo.top = (MUINT32)(doFDFaceMetadata.faces[i].rect[0]);
                    outFDFaceInfo.right = (MUINT32)(doFDFaceMetadata.faces[i].rect[3]);
                    outFDFaceInfo.bottom = (MUINT32)(doFDFaceMetadata.faces[i].rect[2]);
                    break;
                }
                default:{
                    MY_LOGE("mTransform=%d, no such case, should not happend!");
                    break;
                }

            }

            outFDFaceInfo.rotation = doFDFaceMetadata.posInfo[i].rip_dir;

            MY_LOGD("[stereo captureFD] original rect(%d, %d, %d, %d)",
                doFDFaceMetadata.faces[i].rect[0],
                doFDFaceMetadata.faces[i].rect[1],
                doFDFaceMetadata.faces[i].rect[2],
                doFDFaceMetadata.faces[i].rect[3]
            );
            MY_LOGD("[stereo captureFD] modified rect(%d, %d, %d, %d)",
                outFDFaceInfo.left,
                outFDFaceInfo.top,
                outFDFaceInfo.right,
                outFDFaceInfo.bottom
            );

            outFDFaceInfos.push_back(outFDFaceInfo);
        }
    }


    // Send FD info to stereo hal
    // then stereo_hal will set it into debugInfo
    mpStereoHal->STEREOSetFDInfo(outFDFaceInfos);

    MY_LOGD("[stereo captureFD] -");
    return true;
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
    // FeoBufInfo  postBufFeo, postBufFeo_Main2;
    PostBufInfo postBufFeo, postBufFeo_Main2;
    PostBufInfo postBufRgb, postBufRgb_Main2;
    {
        Mutex::Autolock lock(mLock);
        // Shane 20141211 We have NO feo in this platform
        // MY_LOGD("Shane 201412161304: img(%d) img2(%d) feo(%d) feo2(%d)",
                // mlPostBufImg.size(), mlPostBufImg_Main2.size(), mlPostBufFeo.size(), mlPostBufFeo_Main2.size());
        if ( mlPostBufImg.size() == 0 || mlPostBufImg_Main2.size() == 0)
        {
            MY_LOGW("skip threadloop: img(%d) img2(%d) feo(%d) feo2(%d)",
                mlPostBufImg.size(), mlPostBufImg_Main2.size(), mlPostBufFeo.size(), mlPostBufFeo_Main2.size());
            return ret;
        }
        if ( isCapturePath() )
        {
            if ( mlPostBufMain.size() == 0
                || mlPostBufRgb.size() == 0 || mlPostBufRgb_Main2.size() == 0 )
            {
                MY_LOGW("skip threadloop: main(%d) rgb(%d) rgb2(%d)",
                    mlPostBufMain.size(), mlPostBufRgb.size(), mlPostBufRgb_Main2.size());
                return ret;
            }
        }
        //
        postBufImg            = mlPostBufImg.front();
        postBufImg_Main2      = mlPostBufImg_Main2.front();

        // postBufFeo.data       = postBufImg.data;
        // postBufFeo.buf        = postBufImg.buf;
        // postBufFeo_Main2.data = postBufImg_Main2.data;
        // postBufFeo_Main2.buf  = postBufImg_Main2.buf;
        // magicNum = postBufImg.ext;
        // postBufFeo      = mlPostBufFeo.front();
        // postBufFeo_Main2= mlPostBufFeo_Main2.front();
        mlPostBufImg.pop_front();
        mlPostBufImg_Main2.pop_front();

        // mlPostBufFeo.pop_front();
        // mlPostBufFeo_Main2.pop_front();
        isZoom      = (MBOOL)postBufImg.ext;
        // magicNum    = postBufFeo.ext;
        if ( isCapturePath() )
        {
            if ( !mbAllocDone )
            {
                waitThreadDone();
            }
            postBufMain     = mlPostBufMain.front();
            mpMainImageBuf = postBufMain.buf;
            postBufRgb      = mlPostBufRgb.front();
            postBufRgb_Main2= mlPostBufRgb_Main2.front();
            mlPostBufMain.pop_front();
            mlPostBufRgb.pop_front();
            mlPostBufRgb_Main2.pop_front();
            mpAlgoSrcImgBuf = postBufImg_Main2.buf;
            IImageBufferHeap* pSrcHeap  = mpAlgoSrcImgBuf->getImageBufferHeap();
            IImageBufferHeap* pDstHeap  = mpAlgoDstImgBuf->getImageBufferHeap();
            // Enable GPU FULL Mode
            AHardwareBuffer* temp;
            temp = (AHardwareBuffer*)(pSrcHeap->getHWBuffer());
            gralloc_extra_ion_sf_info_t info;
            gralloc_extra_query(AHardwareBuffer_getNativeHandle(temp), GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
            gralloc_extra_sf_set_status(&info, GRALLOC_EXTRA_MASK_YUV_COLORSPACE, GRALLOC_EXTRA_BIT_YUV_BT601_FULL);
            gralloc_extra_perform(AHardwareBuffer_getNativeHandle(temp), GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);

            sDataIn.mSrcGraphicBufferVA = mpAlgoSrcImgBuf->getBufVA(0);
            sDataIn.mDstGraphicBufferVA = mpAlgoDstImgBuf->getBufVA(0);
            sDataIn.mSrcGraphicBuffer   = pSrcHeap->getHWBuffer();
            sDataIn.mDstGraphicBuffer   = pDstHeap->getHWBuffer();
            sDataIn.u4RgbaAddr_main1    = (void*)postBufRgb.buf->getBufVA(0);
            sDataIn.u4RgbaAddr_main2    = (void*)postBufRgb_Main2.buf->getBufVA(0);

            // Shane 20150108
            MY_LOGD("Send new FEO buffer(in fact, new FEO are just scale down original images)");
            postBufFeo      = mlPostBufFeo.front();
            postBufFeo_Main2= mlPostBufFeo_Main2.front();
            mlPostBufFeo.pop_front();
            mlPostBufFeo_Main2.pop_front();
            // sDataIn.YV12BufAddr_main1    = (void*)postBufImg.buf->getBufVA(0);
            // sDataIn.YV12BufAddr_main2    = (void*)postBufImg_Main2.buf->getBufVA(0);
            sDataIn.YV12BufAddr_main1 = (void*)postBufFeo.buf->getBufVA(0);
            sDataIn.YV12BufAddr_main2 = (void*)postBufFeo_Main2.buf->getBufVA(0);

        }
        //
        // sDataIn.u4FEBufAddr_main1   = (void*)postBufFeo.buf->virtAddr;
        // sDataIn.u4FEBufAddr_main2   = (void*)postBufFeo_Main2.buf->virtAddr;
        // sDataIn.mMagicNum           = magicNum;
        //
        MY_LOGD("sDataIn.YV12BufAddr_main1=0x%x  sDataIn.YV12BufAddr_main2=0x%x",sDataIn.YV12BufAddr_main1,sDataIn.YV12BufAddr_main2);
        MY_LOGD("sDataIn.u4FEBufAddr_main1=0x%x ,sDataIn.u4FEBufAddr_main2=0x%x, sDataIn.mMagicNum=%d",sDataIn.u4FEBufAddr_main1,sDataIn.u4FEBufAddr_main2,sDataIn.mMagicNum);
        MY_LOGD("postCnt:%d #(0x%x) main(0x%x) img(%d:0x%x/%d:0x%x) rgb(0x%x/0x%x), isZoom(%d),",
            muPostFrameCnt,
            magicNum,
            postBufMain.buf,
            postBufImg.buf->getFD(),        postBufImg.buf,
            postBufImg_Main2.buf->getFD(),  postBufImg_Main2.buf,
            postBufRgb.buf,
            postBufRgb_Main2.buf,
            isZoom);
        //
        if (getSensorIdx() == 0) {
                MY_LOGD("Stereo Capture sensor timestamp (timediff) : buf1: %lld , buf2: %lld ,",
                postBufImg.buf->getTimestamp() ,
                postBufImg_Main2.buf->getTimestamp()
                );
        }

    }
    //
    {
        Mutex::Autolock lock(mAlgoLock);
        if( !mbEnable )
        {
            MY_LOGD("bypass STEREO");
            goto lbExit;
        }
        MY_LOGD("Only do STEREOSetParams and STEREORun when preview");
        if (isCapturePath()){
            MY_LOGD("isCapturePath()");

            //MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: doFaceDetection +");
            if( pthread_create(&mThreadDoFDDetection, NULL, doThreadDoFDDetection, this))
            {
                MY_LOGE("mThreadDoFDDetection thread create failed.");
                goto lbExit;
            }
            //doFaceDetection(postBufMain.buf);
            //MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: doFaceDetection -");

            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREOSetParams +");
            if( !mpStereoHal || !mpStereoHal->STEREOSetParams(sDataIn) )
            {
                MY_LOGE("STEREOSetParams fail");
                goto lbExit;
            }
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREOSetParams -");

            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREORun +");
            if( !mpStereoHal || !mpStereoHal->STEREORun(sDataOut, !isZoom) )
            {
                MY_LOGE("STEREORun fail");
                goto lbExit;
            }
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: STEREORun -");

            void* threadRet = NULL;
            if( pthread_join(mThreadDoFDDetection, &threadRet) != 0 )
            {
                MY_LOGE("pthread join fail (mThreadDoFDDetection)");
                goto lbExit;
            }
        }
    }
    //
    #if 1 //AARON FIXME
    MY_LOGD("SCNode:handlePostBuffer sensor=%d, data=%d", getSensorIdx(), STEREO_CTRL_DST_M);
    // char szFileName_a[512];
    // sprintf(szFileName_a, "/sdcard/mtklog/SCNode_%dx%d_type_%d.yuv", postBufImg.buf->getImgSize().w, postBufImg.buf->getImgSize().h, STEREO_CTRL_DST_M);
    // postBufImg.buf->saveToFile(szFileName_a);
    if(isCapturePath())
    {
        MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: Resize MainImage for SBS buf: 0x%08X", postBufImg.buf);
    }
    handlePostBuffer( STEREO_CTRL_DST_M , (MUINTPTR)postBufImg.buf, (MUINTPTR)&sDataOut.algo_main1 );
    #else
    handlePostBuffer( STEREO_CTRL_DST_M , (MUINTPTR)postBufImg.buf, 0 );
    #endif
    if ( isCapturePath())
    {
        #if 1 //AARON FIXME
        MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: FullSize MainImage buf: 0x%08X", postBufMain.buf);
        handlePostBuffer( STEREO_CTRL_MAIN_DST , (MUINTPTR)postBufMain.buf, (MUINTPTR)&sDataOut.main_crop );

        int i4DebugStereoRectifyID = -1;
        // try get property from system property
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "vendor.debug.stereo.disablerect", value, "-1");
        i4DebugStereoRectifyID = atoi(value);
        if( 1 == i4DebugStereoRectifyID )
        {
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: Main2Image for SBS buf: 0x%08X - disable rectify", postBufImg_Main2.buf);
            handlePostBuffer( STEREO_CTRL_DST_S , (MUINTPTR)postBufImg_Main2.buf, (MUINTPTR)&sDataOut.algo_main2 );
            //MY_LOGD("disable rectify");
        }
        else
        {
            MY_LOGD_IF(StereoSettingProvider::bEnableLog, "Stereo_Profile: Main2Image for SBS buf: 0x%08X - enable rectify", mpAlgoDstImgBuf);
            handlePostBuffer( STEREO_CTRL_DST_S , (MUINTPTR)mpAlgoDstImgBuf, (MUINTPTR)&sDataOut.algo_main2 );
            //MY_LOGD("enable rectify");
        }
        //
        #else
        handlePostBuffer( STEREO_CTRL_MAIN_DST , (MUINTPTR)postBufMain.buf, 0 );
        handlePostBuffer( STEREO_CTRL_DST_S , (MUINTPTR)mpAlgoDstImgBuf, 0 );
        #endif
        handleReturnBuffer(postBufRgb.data, (MUINTPTR)postBufRgb.buf);
        handleReturnBuffer(postBufRgb_Main2.data, (MUINTPTR)postBufRgb_Main2.buf);

		// need return feo buffer to stereo node.
		// Only return FEO buffer in capture path.
		// In preview path, FEO data will return directly.
		handleReturnBuffer(postBufFeo.data,         (MUINTPTR)postBufFeo.buf);
		handleReturnBuffer(postBufFeo_Main2.data,   (MUINTPTR)postBufFeo_Main2.buf);
    }
    else
    {
        handlePostBuffer( STEREO_CTRL_DST_S , (MUINTPTR)postBufImg_Main2.buf, (MUINTPTR)&sDataOut.algo_main2 );
    }
    //
    muPostFrameCnt++;
    //
    return ret;
lbExit:
//    FUNC_END;
    handleReturnBuffer(postBufImg.data,         (MUINTPTR)postBufImg.buf);
    handleReturnBuffer(postBufImg_Main2.data,   (MUINTPTR)postBufImg_Main2.buf);
    if ( isCapturePath() )
    {
        handleReturnBuffer(postBufMain.data,        (MUINTPTR)postBufMain.buf);
        handleReturnBuffer(postBufRgb.data,         (MUINTPTR)postBufRgb.buf);
        handleReturnBuffer(postBufRgb_Main2.data,   (MUINTPTR)postBufRgb_Main2.buf);
		handleReturnBuffer(postBufFeo.data,         (MUINTPTR)postBufFeo.buf);
		handleReturnBuffer(postBufFeo_Main2.data,   (MUINTPTR)postBufFeo_Main2.buf);
    }
    return MFALSE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

