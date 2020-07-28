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
#define LOG_TAG "capturenr"
//
/*******************************************************************************
*
********************************************************************************/
#include <mtkcam/Log.h>
#define SWNR_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define SWNR_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
#include <mtkcam/common.h>
//
#include <utils/StrongPointer.h>
using namespace android;
//
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe;
//
#include <mtkcam/camnode/IspSyncControl.h>
//
#include <mtkcam/featureio/capturenr.h>
//
#include <utils/Mutex.h>
#include <list>
#include <math.h>

#include <sys/mman.h>
//
// custom tuning
#include <mtkcam/hal/IHalSensor.h>
#include <camera_custom_nvram.h>
#include <mtkcam/featureio/nvbuf_util.h>
//
#include <aaa/aaa_types.h>
#include <aaa/aaa_log.h>
#include <isp_tuning/isp_tuning.h>
#include <isp_tuning/isp_tuning_cam_info.h>
#include <isp_tuning/isp_tuning_idx.h>
#include <isp_tuning/isp_tuning_custom.h>
#include <isp_tuning/isp_tuning_custom_swnr.h>
#include <mtkcam/algorithm/lib3a/isp_interpolation.h>

//
#include <cutils/properties.h>
//
#define PASS2_TIMEOUT       ((MINT64)5000000000) //ns
#define MAGIC_NUM           (0x80000000|0x3FFFFFFF)

#define DUMP_PARAM(fp, fmt, arg...)            \
    do {                                       \
        if( fp ) fprintf(fp, fmt "\n", ##arg); \
        else SWNR_LOGD(fmt, ##arg);              \
    } while(0);


#define SWNR_ASSERT(x)                                                      \
do {                                                                        \
    MBOOL __x = (x);                                                        \
    if (!__x) {                                                             \
        SWNR_LOGE("[%s:%d] ASSERT FAILED: %s", __FUNCTION__, __LINE__, #x); \
        goto lbExit;                                                        \
    }                                                                       \
} while(0);


typedef enum {
    SWNR_SCENARIO_NORMAL = 0,
    SWNR_SCENARIO_MFNR,
    SWNR_SCENARIO_DUAL,
} SWNR_SCENARIO;


/*******************************************************************************
*
********************************************************************************/
// these assertions are to make sure nvram is large enough
static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) >= sizeof(SWNR_PARAM) + sizeof(FSWNR_PARAM),
        "should check NVRAM_CAMERA_FEATURE_SWNR_STRUCT"
        );

static_assert(
        sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) >=
        (sizeof(NRTuningInfo) + offsetof(NR2TuningInfo, reserved) +
        2 * (sizeof(CCRTuningInfo) + sizeof(HFGTuningInfo))),
        "[FIXME] should modify swnr nvram struct"
        );

static_assert(
        sizeof(NR_PARAM) == sizeof(NRTuningInfo),
        "[FIXME] struct size is not correct"
        );

static_assert(
        sizeof(NR2_PARAM) == sizeof(NR2TuningInfo) - member_size(NR2TuningInfo, reserved),
        "[FIXME] struct size is not correct"
        );

static_assert(
        sizeof(HFG_PARAM) == sizeof(HFGTuningInfo),
        "[FIXME] struct size is not correct"
        );

static_assert(
        sizeof(CCR_PARAM) == sizeof(CCRTuningInfo),
        "[FIXME] struct size is not correct"
        );


/*******************************************************************************
*
********************************************************************************/
class SwNR : public ISwNR
{
public:
    SwNR(MUINT32 const openId);
    virtual ~SwNR();

public:
    virtual MBOOL       doSwNR(NSCam::IImageBuffer* pBuf);
    // virtual MBOOL       getDebugInfo(NSCam::IMetadata& halMetadata) const;
    virtual MVOID       dumpParam(char * const filename);

    struct TUNINGINFO
    {
        union
        {
            NRTuningInfo    NR;
            NR2TuningInfo   NR2;
        };
        CCRTuningInfo CCR;
        HFGTuningInfo HFG;
    };

private:
    MUINT32 const       muOpenId;
    DrvNRObject_e       mNRType;

    TUNINGINFO          mTuningInfo;

    NRPerfGear          mNRPerfGear;
    NRImageInfo         mNRImageInfo;
    NRResultInfo        mNRResultInfo;
};

struct SwNRParam
{
    MUINT32 iso;
    MUINT32 scenario;
    MINT32  perfLevel;
};

static SwNRParam gSwNRParam;

/*******************************************************************************
*
********************************************************************************/
ISwNR* ISwNR::createInstance(MUINT32 openId)
{
    return new SwNR(openId);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL doHwNR(
        MUINT32 const openId,
        IImageBuffer* in,
        IImageBuffer* out0,
        IImageBuffer* out1,
        MRect const crop,
        MUINT32 const transform_0,
        NS3A::EIspProfile_T const profile
        )
{
    SWNR_LOGD("idx %d, in %p, out %p, %p, profile %d", openId, in, out0, out1, profile);
    if( out0 )
    {
        SWNR_LOGD("out0: %dx%d fmt 0x%x trans %d",
                out0->getImgSize().w,
                out0->getImgSize().h,
                out0->getImgFormat(),
                transform_0);
    }
    if( out1 )
    {
        SWNR_LOGD("out1: %dx%d fmt 0x%x",
                out1->getImgSize().w,
                out1->getImgSize().h,
                out1->getImgFormat() );
    }

    MBOOL                   ret = MFALSE;
    IHal3A*                 pHal3A = NULL;
    IHalPostProcPipe*       pPostProcPipe = NULL;
    MUINT32 const           magic = MAGIC_NUM;

    // check params
    if( in->getImgFormat() != eImgFmt_YUY2 ){
        SWNR_LOGE("not supported src fmt %d", in->getImgFormat() );
        goto lbExit;
    }

    pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, openId, LOG_TAG);
    if(pHal3A == NULL) {
        SWNR_LOGE("IHal3A:createInstance fail");
        goto lbExit;
    }

    {
        ParamIspProfile_T _3A_profile( profile, magic, MTRUE, ParamIspProfile_T::EParamValidate_P2Only);
        SWNR_LOGD("set ispProfile %d, # 0x%x", profile, magic);
        pHal3A->setIspProfile(_3A_profile);
    }

    if( profile == NSIspTuning::EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
        profile == NSIspTuning::EIspProfile_VSS_Capture_MultiPass_ANR_2 ||
        profile == NSIspTuning::EIspProfile_VSS_MFB_MultiPass_ANR_1 ||
        profile == NSIspTuning::EIspProfile_VSS_MFB_MultiPass_ANR_2 )
    {
        pPostProcPipe = INormalStream::createInstance(LOG_TAG, ENormalStreamTag_Vss, openId);
    } else {
        pPostProcPipe = INormalStream::createInstance(LOG_TAG, ENormalStreamTag_Cap, openId);
    }

    if(pPostProcPipe == NULL) {
        SWNR_LOGE("INormalStream::createInstance fail");
        goto lbExit;
    }

    if( !pPostProcPipe->init() ) {
        SWNR_LOGE("INormalStream::init fail");
        goto lbExit;
    }

    {
        QParams enqueParams;
        enqueParams.mvIn.reserve(1);
        enqueParams.mFrameNo = magic; // use un-used magic #

        {   // input
            Input src;
            src.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
            src.mBuffer = in;
            enqueParams.mvIn.push_back(src);
        }

        if( out0 )
        {   // output
            Output dst;
            dst.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WROTO, 1);
            dst.mPortID.capbility   = NSIoPipe::EPortCapbility_Cap;
            //
            dst.mBuffer = out0;
            dst.mTransform  = transform_0;
            //
            enqueParams.mvOut.push_back(dst);
        }

        if( out1 )
        {   // output
            Output dst;
            dst.mPortID = PortID(NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);
            if( out1->getImgFormat() != eImgFmt_YUY2 )
            {
                dst.mPortID.capbility   = NSIoPipe::EPortCapbility_Disp;
            }
            else
            {   // workaround: Disp will enable tdsharp. However, thumbnail is too small
                dst.mPortID.capbility   = NSIoPipe::EPortCapbility_Cap;
            }
            //
            dst.mBuffer = out1;
            dst.mTransform  = 0;
            //
            enqueParams.mvOut.push_back(dst);
        }

        {   // crop
            MCrpRsInfo crop1;
            crop1.mGroupID    = 1;
            crop1.mCropRect.s = in->getImgSize();
            crop1.mResizeDst  = in->getImgSize();
            //
            MCrpRsInfo crop2;
            crop2.mGroupID    = 2;
            crop2.mCropRect.s = crop.s;
            crop2.mCropRect.p_integral = crop.p;
            crop2.mCropRect.p_fractional = MPoint(0);

            enqueParams.mvCropRsInfo.push_back( crop1 );
            enqueParams.mvCropRsInfo.push_back( crop2 );
        }

        if( !pPostProcPipe->enque(enqueParams) )
        {
            SWNR_LOGE("enque pass2 failed");
            goto lbExit;
        }
    }

    {
        QParams dequeParams;
        if( !pPostProcPipe->deque(dequeParams, PASS2_TIMEOUT) )
        {
            SWNR_LOGE("deque pass2: deque fail");
            goto lbExit;
        }
    }

    if( !pPostProcPipe->uninit() ){
        SWNR_LOGE("INormalStream::uninit fail");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    if( pHal3A )
        pHal3A->destroyInstance(LOG_TAG);

    if( pPostProcPipe )
        pPostProcPipe->destroyInstance(LOG_TAG);

    return ret;
}


MBOOL doTwoPassHwNR(
        MUINT32 openId,
        IImageBuffer* buf,
        NS3A::EIspProfile_T const profile0,
        NS3A::EIspProfile_T const profile1)
{
    SWNR_LOGD("idx %d, buf %p, profile %d/%d", openId, buf, profile0, profile1);
    MBOOL                     ret = MFALSE;
    sp<IImageBuffer>          pInternalBuf;

    // check params
    if( buf->getImgFormat() != eImgFmt_YUY2 ){
        SWNR_LOGE("not supported src fmt %d", buf->getImgFormat() );
        goto lbExit;
    }

    {
        SWNR_LOGD("allocate memory");
        // allocate internal memory
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();

        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        MUINT32 bufStrideInByte[3] = {static_cast<MUINT32>(buf->getBufStridesInBytes(0)), 0, 0 };

        IImageBufferAllocator::ImgParam imgParam(
                buf->getImgFormat(),
                buf->getImgSize(),
                bufStrideInByte,
                bufBoundaryInBytes,
                buf->getPlaneCount()
                );

        pInternalBuf = allocator->alloc_ion(LOG_TAG, imgParam);

        if ( !pInternalBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
        {
            SWNR_LOGE("lock Buffer failed\n");
            goto lbExit;
        }

        if ( !pInternalBuf->syncCache( eCACHECTRL_INVALID ) )
        {
            SWNR_LOGE("syncCache failed\n");
            goto lbExit;
        }

    }

    // 1st run
    if( !doHwNR(openId, buf, NULL, pInternalBuf.get(), MRect(MPoint(0), buf->getImgSize()),0, profile0) ) {
        SWNR_LOGE("1st run NR failed");
        goto lbExit;
    }

    // 2nd run
    if( !doHwNR(openId, pInternalBuf.get(), NULL, buf, MRect(MPoint(0), buf->getImgSize()), 0, profile1) ) {
        SWNR_LOGE("2nd run NR failed");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    // release internal buf
    if( pInternalBuf.get() ) {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        if( !pInternalBuf->unlockBuf( LOG_TAG ) )
        {
            SWNR_LOGE("unlock Buffer failed\n");
            ret = MFALSE;
        }
        allocator->free(pInternalBuf.get());
        pInternalBuf = NULL;
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
inline MINT32 generateSeed() { return (MINT32)::systemTime(); }


static MVOID dumpParams(NRTuningInfo* pNR, FILE* fp = NULL)
{
    if( ! pNR ) {
        SWNR_LOGE("null");
        return;
    }
#define DUMP_NR(param)                               \
    do {                                             \
        DUMP_PARAM(fp, "%s:%d", #param, pNR->param); \
    } while(0)
    DUMP_NR(ANR_Y_LUMA_SCALE_RANGE);
    DUMP_NR(ANR_C_CHROMA_SCALE);
    DUMP_NR(ANR_Y_SCALE_CPY0);
    DUMP_NR(ANR_Y_SCALE_CPY1);
    DUMP_NR(ANR_Y_SCALE_CPY2);
    DUMP_NR(ANR_Y_SCALE_CPY3);
    DUMP_NR(ANR_Y_SCALE_CPY4);
    DUMP_NR(ANR_Y_CPX1);
    DUMP_NR(ANR_Y_CPX2);
    DUMP_NR(ANR_Y_CPX3);
    DUMP_NR(ANR_CEN_GAIN_LO_TH);
    DUMP_NR(ANR_CEN_GAIN_HI_TH);
    DUMP_NR(ANR_PTY_GAIN_TH);
    DUMP_NR(ANR_KSIZE_LO_TH);
    DUMP_NR(ANR_KSIZE_HI_TH);
    DUMP_NR(ANR_KSIZE_LO_TH_C);
    DUMP_NR(ANR_KSIZE_HI_TH_C);
    DUMP_NR(ITUNE_ANR_PTY_STD);
    DUMP_NR(ITUNE_ANR_PTU_STD);
    DUMP_NR(ITUNE_ANR_PTV_STD);
    DUMP_NR(ANR_ACT_TH_Y);
    DUMP_NR(ANR_ACT_BLD_BASE_Y);
    DUMP_NR(ANR_ACT_BLD_TH_Y);
    DUMP_NR(ANR_ACT_SLANT_Y);
    DUMP_NR(ANR_ACT_TH_C);
    DUMP_NR(ANR_ACT_BLD_BASE_C);
    DUMP_NR(ANR_ACT_BLD_TH_C);
    DUMP_NR(ANR_ACT_SLANT_C);
    DUMP_NR(RADIUS_H);
    DUMP_NR(RADIUS_V);
    DUMP_NR(RADIUS_H_C);
    DUMP_NR(RADIUS_V_C);
    DUMP_NR(ANR_PTC_HGAIN);
    DUMP_NR(ANR_PTY_HGAIN);
    DUMP_NR(ANR_LPF_HALFKERNEL);
    DUMP_NR(ANR_LPF_HALFKERNEL_C);
    DUMP_NR(ANR_ACT_MODE);
    DUMP_NR(ANR_LCE_SCALE_GAIN);
    DUMP_NR(ANR_LCE_C_GAIN);
    DUMP_NR(ANR_LCE_GAIN0);
    DUMP_NR(ANR_LCE_GAIN1);
    DUMP_NR(ANR_LCE_GAIN2);
    DUMP_NR(ANR_LCE_GAIN3);
    DUMP_NR(ANR_MEDIAN_LOCATION);
    DUMP_NR(ANR_CEN_X);
    DUMP_NR(ANR_CEN_Y);
    DUMP_NR(ANR_R1);
    DUMP_NR(ANR_R2);
    DUMP_NR(ANR_R3);
    DUMP_NR(LUMA_ON_OFF);
#undef DUMP_NR
}


static MVOID dumpParams(NR2TuningInfo* pNR, FILE* fp = NULL)
{
    if( ! pNR ) {
        SWNR_LOGE("null");
        return;
    }
#define DUMP_NR2(param)                              \
    do {                                             \
        DUMP_PARAM(fp, "%s:%d", #param, pNR->param); \
    } while(0)
    DUMP_NR2(NR_K);
    DUMP_NR2(NR_S);
    DUMP_NR2(NR_SD);
    DUMP_NR2(NR_BLD_W);
    DUMP_NR2(NR_BLD_TH);
    DUMP_NR2(NR_SMTH);
#undef DUMP_NR2
}


static MVOID dumpParams(HFGTuningInfo* pHFG, FILE* fp = NULL)
{
    if( ! pHFG ) {
        SWNR_LOGE("null");
        return;
    }
#define DUMP_HFG(param)                               \
    do {                                              \
        DUMP_PARAM(fp, "%s:%d", #param, pHFG->param); \
    } while(0)
    DUMP_HFG(HFG_ENABLE);
    DUMP_HFG(HFG_GSD);
    DUMP_HFG(HFG_SD0);
    DUMP_HFG(HFG_SD1);
    DUMP_HFG(HFG_SD2);
    DUMP_HFG(HFG_TX_S);
    DUMP_HFG(HFG_LCE_LINK_EN);
    DUMP_HFG(HFG_LUMA_CPX1);
    DUMP_HFG(HFG_LUMA_CPX2);
    DUMP_HFG(HFG_LUMA_CPX3);
    DUMP_HFG(HFG_LUMA_CPY0);
    DUMP_HFG(HFG_LUMA_CPY1);
    DUMP_HFG(HFG_LUMA_CPY2);
    DUMP_HFG(HFG_LUMA_CPY3);
    DUMP_HFG(HFG_LUMA_SP0);
    DUMP_HFG(HFG_LUMA_SP1);
    DUMP_HFG(HFG_LUMA_SP2);
    DUMP_HFG(HFG_LUMA_SP3);
#undef DUMP_HFG
}


static MVOID dumpParams(CCRTuningInfo* pCCR, FILE* fp = NULL)
{
    if( ! pCCR ) {
        SWNR_LOGE("null");
        return;
    }
#define DUMP_CCR(param)                               \
    do {                                              \
        DUMP_PARAM(fp, "%s:%d", #param, pCCR->param); \
    } while(0)
    DUMP_CCR(CCR_ENABLE);
    DUMP_CCR(CCR_CEN_U);
    DUMP_CCR(CCR_CEN_V);
    DUMP_CCR(CCR_Y_CPX1);
    DUMP_CCR(CCR_Y_CPX2);
    DUMP_CCR(CCR_Y_CPY1);
    DUMP_CCR(CCR_Y_SP1);
    DUMP_CCR(CCR_UV_X1);
    DUMP_CCR(CCR_UV_X2);
    DUMP_CCR(CCR_UV_X3);
    DUMP_CCR(CCR_UV_GAIN1);
    DUMP_CCR(CCR_UV_GAIN2);
    DUMP_CCR(CCR_UV_GAIN_SP1);
    DUMP_CCR(CCR_UV_GAIN_SP2);
    DUMP_CCR(CCR_Y_CPX3);
    DUMP_CCR(CCR_Y_CPY0);
    DUMP_CCR(CCR_Y_CPY2);
    DUMP_CCR(CCR_Y_SP0);
    DUMP_CCR(CCR_Y_SP2);
    DUMP_CCR(CCR_UV_GAIN_MODE);
    DUMP_CCR(CCR_MODE);
    DUMP_CCR(CCR_OR_MODE);
    DUMP_CCR(CCR_HUE_X1);
    DUMP_CCR(CCR_HUE_X2);
    DUMP_CCR(CCR_HUE_X3);
    DUMP_CCR(CCR_HUE_X4);
    DUMP_CCR(CCR_HUE_SP1);
    DUMP_CCR(CCR_HUE_SP2);
    DUMP_CCR(CCR_HUE_GAIN1);
    DUMP_CCR(CCR_HUE_GAIN2);
#undef DUMP_CCR
}
#undef DUMP_PARAM

static const char* Scenario_toString(MUINT32 scenario)
{
    if (scenario==*(MUINT32*)"MFNR")
        return "MFNR";
    else if (scenario==*(MUINT32*)"DUAL")
        return "DUAL";
    else if (scenario==*(MUINT32*)"NORM")
        return "NORM";
    else
        return "UNKNOWN";
}

static const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *getTuningFromNvram(MUINT32 openId, MUINT32 scenario, MUINT32 idx)
{
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT *pNvram;
    const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pNRNvram;

    if (idx >= eNUM_OF_ISO_IDX) {
        SWNR_LOGE("wrong nvram idx %d", idx);
        return NULL;
    }

    // load some setting from nvram
    MUINT sensorDev = IHalSensorList::get()->querySensorDevIdx(openId);
    //
    err = NvBufUtil::getInstance().getBufAndRead(
            CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)pNvram);
    if (err != 0)
    {
        SWNR_LOGE("getBufAndRead fail, err=%d", err);
        return NULL;
    }


    if (scenario == *(MUINT32*)"MFNR") {
        pNRNvram = &(pNvram->nr_mfll[idx]);
#if (MTKCAM_STEREO_DENOISE_SUPPORT=='1')
    } else if (scenario == *(MUINT32*)"DUAL") {
        pNRNvram = &(pNvram->nr_dual[idx]);
#endif
    } else if (scenario == *(MUINT32*)"NORM") {
        pNRNvram = &(pNvram->nr_normal[idx]);
    } else {
        SWNR_LOGW("Unknown scenario, apply normal tuning");
        pNRNvram = &(pNvram->nr_normal[idx]);
    }

    return pNRNvram;
}


static MBOOL getSensorDevice(MUINT* pSensorDev, MUINT *pSensorDevId, MUINT32 openId)
{
    MBOOL ret = MFALSE;
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    *pSensorDev = pHalSensorList->querySensorDevIdx(openId);
    SensorStaticInfo rSensorStaticInfo;
    switch (*pSensorDev)
    {
    case SENSOR_DEV_MAIN:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
        break;

    case SENSOR_DEV_SUB:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
        break;

    case SENSOR_DEV_MAIN_2:
        pHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
        break;

    default:    //  Shouldn't happen.
        MY_ERR("Invalid sensor device: %d", *pSensorDev);
        SWNR_ASSERT(0);
    }

    *pSensorDevId = rSensorStaticInfo.sensorDevID;
    SWNR_LOGD("sensorDev(%d), u4SensorID(%d)", *pSensorDev, *pSensorDevId);

    ret = MTRUE;
lbExit:
    return ret;
}


static void *allocWorkingBuf(size_t size)
{
    void *pWorkBuf = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (pWorkBuf == MAP_FAILED) return NULL;
    return pWorkBuf;
}


static void freeWorkingBuf(void *pWorkBuf, size_t size)
{
    if (pWorkBuf) {
        munmap(pWorkBuf, size);
    }
}


static void setPerfLevel(NRPerfGear *pPerfInfo, MINT32 perfLevel)
{
    // set performance level
    switch(perfLevel)
    {
    case eSWNRPerf_Performance_First:
        pPerfInfo->enable = 1;
        pPerfInfo->option = NR_PERF_GEAR_PERF_FIRST;
        break;

    case eSWNRPerf_Custom0:
        pPerfInfo->enable = 1;
        pPerfInfo->option = NR_PERF_GEAR_CUSTOM_0;
        break;

    case eSWNRPerf_Power_First:
        pPerfInfo->enable = 1;
        pPerfInfo->option = NR_PERF_GEAR_POWER_FIRST;
        break;

    default:
        SWNR_LOGW("not valid level %d", perfLevel);
    }
}


static MBOOL getTuningInfo(
    SwNR::TUNINGINFO *pTuningInfo,  // output
    DrvNRObject_e *pNRType,         // output
    MUINT32 iso,
    MUINT32 scenario,
    MUINT32 openId)
{
    MBOOL ret = MFALSE;
    IspTuningCustom *pIspTuningCustom = NULL;
    DrvNRObject_e NRType = DRV_NR_OBJ_NONE;
    MUINT32 idx, iso_low, iso_high;

    MUINT sensorDev;
    MUINT sensorDevID;
    MINT32 CustomerNRType;
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT NvramData;

    SWNR_ASSERT(getSensorDevice(&sensorDev, &sensorDevID, openId) == MTRUE);

    pIspTuningCustom = IspTuningCustom::createInstance(
            static_cast<NSIspTuning::ESensorDev_T>(sensorDev),
            sensorDevID);
    SWNR_ASSERT(pIspTuningCustom != NULL);

    // 1. update swnr type
    CustomerNRType = get_swnr_type(sensorDev);
    {
        // force use swnr (for debug)
        MINT32 type = property_get_int32("vendor.debug.swnr.type", -1);
        if (type != -1) {
            SWNR_LOGD("force use swnr: %s", type == eSWNRType_SW ? "normal" : "fast");
            CustomerNRType = type;
        }
    }

    SWNR_ASSERT(CustomerNRType >= 0);
    SWNR_ASSERT(CustomerNRType < eSWNRType_NUM);

    iso = pIspTuningCustom->remap_ISO_value(iso);
    idx = map_ISO_value_to_index(iso);
    iso_low = map_index_to_ISO_value(idx);
    iso_high = map_index_to_ISO_value( (idx+1)>=eNUM_OF_ISO_IDX ? idx : idx+1);
    if (iso_high == (MUINT32)-1) {
        SWNR_LOGW("iso overflow, force to %d", iso_low);
        iso_high = iso_low;
    }

    // 2. read from nvram & smooth
    if (is_to_invoke_swnr_interpolation(scenario == *(MUINT32*)"MFNR", iso) && iso_low < iso_high) {
        SWNR_LOGD("%d: iso %d(%d~%d), scenario %s, idx low/high %d/%d", openId, iso, iso_low, iso_high, Scenario_toString(scenario), idx);

        const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pNvramLow   = getTuningFromNvram(openId, scenario, idx);
        const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pNvramHigh  = getTuningFromNvram(openId, scenario, idx+1);
        SWNR_ASSERT(pNvramLow!=NULL);
        SWNR_ASSERT(pNvramHigh!=NULL);

        // interpolation
        auto interpolation_func = (CustomerNRType == eSWNRType_SW) ? SmoothSWNR : SmoothFSWNR;
        interpolation_func(
                iso,
                iso_high, iso_low,
                *pNvramHigh,
                *pNvramLow,
                NvramData);

    } else {
        SWNR_LOGD("%d: iso %d(%d), scenario %s, idx %d", openId, iso, iso_low, Scenario_toString(scenario), idx);

        // get nvram
        const NVRAM_CAMERA_FEATURE_SWNR_STRUCT *pTuningNvram = getTuningFromNvram(openId, scenario, idx);
        if (!pTuningNvram) {
            SWNR_LOGE("update from nvram failed");
            return MFALSE;
        }

        // update
        memcpy(&NvramData, pTuningNvram, sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT));
    }

    // get tuning info from NVRam
    if (CustomerNRType == eSWNRType_SW)
    {
        NRType = DRV_NR_OBJ_SW;

        memcpy(&pTuningInfo->NR , &NvramData.swnr.NR , sizeof(NR_PARAM));
        memcpy(&pTuningInfo->HFG, &NvramData.swnr.HFG, sizeof(HFG_PARAM));
        memcpy(&pTuningInfo->CCR, &NvramData.swnr.CCR, sizeof(CCR_PARAM));

        // update some parameters
        pTuningInfo->HFG.HFG_GSD = generateSeed();
    } else {
        NRType = DRV_NR_OBJ_SW2;

        memcpy(&pTuningInfo->NR2, &(NvramData.fswnr.NR) , sizeof(NR2_PARAM));
        memcpy(&pTuningInfo->HFG, &(NvramData.fswnr.HFG), sizeof(HFG_PARAM));
        memcpy(&pTuningInfo->CCR, &(NvramData.fswnr.CCR), sizeof(CCR_PARAM));

        // update some parameters
        pTuningInfo->NR2.NR_SD      = generateSeed();
        pTuningInfo->HFG.HFG_GSD    = generateSeed();
    }

    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if (bDebug) {
            if (NRType == DRV_NR_OBJ_SW) {
                dumpParams(&pTuningInfo->NR);
            } else {
                dumpParams(&pTuningInfo->NR2);
            }
            dumpParams(&pTuningInfo->HFG);
            dumpParams(&pTuningInfo->CCR);
        }
    }

    ret = MTRUE;
lbExit:
    *pNRType = NRType;
    if (pIspTuningCustom) pIspTuningCustom->destroyInstance();
    return ret;
}


static MBOOL setImageInfo(NRImageInfo *pNRImageInfo, const IImageBuffer* pBuf)
{
    MBOOL ret = MFALSE;
    MTK_NR_IMAGE_FMT_ENUM NRFmt = NR_IMAGE_PACKET_YUY2;
    switch( pBuf->getImgFormat() )
    {
        case eImgFmt_YUY2:
            NRFmt = NR_IMAGE_PACKET_YUY2;
            break;
        case eImgFmt_I420:
            NRFmt = NR_IMAGE_YUV420;
            break;
        case eImgFmt_YV12:
            NRFmt = NR_IMAGE_YV12;
            break;
        default:
            SWNR_LOGE("not supported format: 0x%x", pBuf->getImgFormat() );
            goto lbExit;
    }

    // set image info
    pNRImageInfo->Width = pBuf->getImgSize().w;
    pNRImageInfo->Height = pBuf->getImgSize().h;
    pNRImageInfo->pImg[0] = (void*)pBuf->getBufVA(0);
    pNRImageInfo->pImg[1] = pBuf->getPlaneCount() > 1 ? (void*)pBuf->getBufVA(1) : (void*)0,
    pNRImageInfo->pImg[2] = pBuf->getPlaneCount() > 2 ? (void*)pBuf->getBufVA(2) : (void*)0,

    pNRImageInfo->ImgNum = 1;
    pNRImageInfo->ImgFmt = NRFmt;

    ret = MTRUE;
lbExit:
    return ret;
}

void SwNR_setParam(MUINT32 iso, MUINT32 scenario, MINT32 perfLevel)
{
    if (scenario == 0) {
        scenario = *(MUINT32*)"NORM";
    } else if (scenario == 1) {
        scenario = *(MUINT32*)"MFNR";
    }

    gSwNRParam.iso = iso;
    gSwNRParam.scenario = scenario;
    gSwNRParam.perfLevel = perfLevel;
};


SwNR::SwNR(MUINT32 const openId)
    : muOpenId(openId)
    , mNRType(DRV_NR_OBJ_NONE)
{
    memset(&mTuningInfo, 0, sizeof(mTuningInfo));
    memset(&mNRImageInfo, 0, sizeof(mNRImageInfo));
    memset(&mNRResultInfo, 0, sizeof(mNRResultInfo));
    memset(&mNRPerfGear, 0, sizeof(mNRPerfGear));
}


SwNR::~SwNR()
{
}


MBOOL SwNR::doSwNR(NSCam::IImageBuffer* pBuf)
{
    MBOOL ret = MFALSE;
    MTKNR* pMTKNR = NULL;
    NRInitInfo SwNRInitInfo;

    // set SwNRInitInfo
    SwNRInitInfo.pWorkingBuff       = NULL;
    SwNRInitInfo.WorkingBuffSize    = 0;
    SwNRInitInfo.CoreNumber         = NR_MAX_CORE_NO;
    SwNRInitInfo.NumOfExecution     = 1;
    SwNRInitInfo.ForcedFreq         = 0;

    SWNR_LOGD("idx %d: iso %d scenario %s perf %d, buf %p",
        muOpenId,
        gSwNRParam.iso,
        Scenario_toString(gSwNRParam.scenario),
        gSwNRParam.perfLevel,
        pBuf);

    // set performance level
    setPerfLevel(&mNRPerfGear, gSwNRParam.perfLevel);

    // get tuning info
    SWNR_ASSERT(getTuningInfo(&mTuningInfo, &mNRType, gSwNRParam.iso, gSwNRParam.scenario, muOpenId) != 0);
    SWNR_LOGD("swnr type %s", mNRType == DRV_NR_OBJ_SW ? "normal" : "fast");

    // update SwNRInitInfo
    SwNRInitInfo.pPerfInfo          = &mNRPerfGear;
    SwNRInitInfo.pTuningInfo        = &mTuningInfo.NR;
    SwNRInitInfo.pCCRTuningInfo     = &mTuningInfo.CCR;
    SwNRInitInfo.pHFGTuningInfo     = &mTuningInfo.HFG;

    // create MTKNR
    pMTKNR = MTKNR::createInstance(mNRType);
    SWNR_ASSERT(pMTKNR != NULL);

    // init
    SWNR_ASSERT(pMTKNR->NRInit((MUINT32*) &SwNRInitInfo, 0) == S_NR_OK);

    // setImageInfo
    SWNR_ASSERT(setImageInfo(&mNRImageInfo, pBuf) != 0);

    // GetWorkbufSize
    SWNR_ASSERT(pMTKNR->NRFeatureCtrl(NR_FEATURE_GET_WORKBUF_SIZE, (void*)&mNRImageInfo, (void*)&SwNRInitInfo.WorkingBuffSize)==S_NR_OK);

    // allocWorkingBuf
    SwNRInitInfo.pWorkingBuff = allocWorkingBuf(SwNRInitInfo.WorkingBuffSize);
    SWNR_ASSERT(SwNRInitInfo.pWorkingBuff != NULL);

    // SetWorkBufAddr
    SWNR_ASSERT(pMTKNR->NRFeatureCtrl(NR_FEATURE_SET_WORKBUF_ADDR, SwNRInitInfo.pWorkingBuff, NULL) == S_NR_OK);

    // AddImg
    SWNR_ASSERT(pMTKNR->NRFeatureCtrl(NR_FEATURE_ADD_IMG, (void*)&mNRImageInfo, NULL) == S_NR_OK);

    // NRMain
    SWNR_LOGD("start+");
    SWNR_ASSERT(pMTKNR->NRMain() == S_NR_OK);
    SWNR_LOGD("start-");

    // GetResult
    SWNR_ASSERT(pMTKNR->NRFeatureCtrl(NR_FEATURE_GET_RESULT, NULL, (void*)&mNRResultInfo) == S_NR_OK);

    pBuf->syncCache(eCACHECTRL_FLUSH);

    pMTKNR->NRReset();

    ret = MTRUE;
lbExit:
    // freeWorkingBuf
    if (SwNRInitInfo.pWorkingBuff) {
        freeWorkingBuf(SwNRInitInfo.pWorkingBuff, SwNRInitInfo.WorkingBuffSize);
        SwNRInitInfo.pWorkingBuff = NULL;
        SwNRInitInfo.WorkingBuffSize = 0;
    }

    // destroy MTKNR
    if (pMTKNR) {
        pMTKNR->destroyInstance(pMTKNR);
        pMTKNR = NULL;
    }

    return ret;
}

#if 0
MBOOL SwNR::getDebugInfo(IMetadata& halMetadata) const
{
    bool haveExif = false;
    {
        IMetadata::IEntry entry = halMetadata.entryFor(MTK_HAL_REQUEST_REQUIRE_EXIF);
        if (! entry.isEmpty()  && entry.itemAt(0, Type2Type<MUINT8>()) )
                haveExif = true;
    }
    //
    if (haveExif )
    {
        IMetadata::Memory memory_dbgInfo;
        memory_dbgInfo.resize(sizeof(DEBUG_RESERVEA_INFO_T));
        DEBUG_RESERVEA_INFO_T& dbgInfo =
            *reinterpret_cast<DEBUG_RESERVEA_INFO_T*>(memory_dbgInfo.editArray());
        ssize_t idx = 0;
#define addPair(debug_info, index, id, value)           \
        do{                                             \
            debug_info.Tag[index].u4FieldID = id;       \
            debug_info.Tag[index].u4FieldValue = value; \
            index++;                                    \
        } while(0)
        //
        addPair(dbgInfo, idx, RESERVEA_TAG_VERSION, RESERVEA_DEBUG_TAG_VERSION);
        //
        if (mNRType == DRV_NR_OBJ_SW ) // normal swnr
        {
            addPair(dbgInfo , idx , ANR_Y_LUMA_SCALE_RANGE , mTuningInfo.NR.ANR_Y_LUMA_SCALE_RANGE );
            addPair(dbgInfo , idx , ANR_C_CHROMA_SCALE     , mTuningInfo.NR.ANR_C_CHROMA_SCALE     );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY0       , mTuningInfo.NR.ANR_Y_SCALE_CPY0       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY1       , mTuningInfo.NR.ANR_Y_SCALE_CPY1       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY2       , mTuningInfo.NR.ANR_Y_SCALE_CPY2       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY3       , mTuningInfo.NR.ANR_Y_SCALE_CPY3       );
            addPair(dbgInfo , idx , ANR_Y_SCALE_CPY4       , mTuningInfo.NR.ANR_Y_SCALE_CPY4       );
            addPair(dbgInfo , idx , ANR_Y_CPX1             , mTuningInfo.NR.ANR_Y_CPX1             );
            addPair(dbgInfo , idx , ANR_Y_CPX2             , mTuningInfo.NR.ANR_Y_CPX2             );
            addPair(dbgInfo , idx , ANR_Y_CPX3             , mTuningInfo.NR.ANR_Y_CPX3             );
            addPair(dbgInfo , idx , ANR_CEN_GAIN_LO_TH     , mTuningInfo.NR.ANR_CEN_GAIN_LO_TH     );
            addPair(dbgInfo , idx , ANR_CEN_GAIN_HI_TH     , mTuningInfo.NR.ANR_CEN_GAIN_HI_TH     );
            addPair(dbgInfo , idx , ANR_PTY_GAIN_TH        , mTuningInfo.NR.ANR_PTY_GAIN_TH        );
            addPair(dbgInfo , idx , ANR_KSIZE_LO_TH        , mTuningInfo.NR.ANR_KSIZE_LO_TH        );
            addPair(dbgInfo , idx , ANR_KSIZE_HI_TH        , mTuningInfo.NR.ANR_KSIZE_HI_TH        );
            addPair(dbgInfo , idx , ANR_KSIZE_LO_TH_C      , mTuningInfo.NR.ANR_KSIZE_LO_TH_C      );
            addPair(dbgInfo , idx , ANR_KSIZE_HI_TH_C      , mTuningInfo.NR.ANR_KSIZE_HI_TH_C      );
            addPair(dbgInfo , idx , ITUNE_ANR_PTY_STD      , mTuningInfo.NR.ITUNE_ANR_PTY_STD      );
            addPair(dbgInfo , idx , ITUNE_ANR_PTU_STD      , mTuningInfo.NR.ITUNE_ANR_PTU_STD      );
            addPair(dbgInfo , idx , ITUNE_ANR_PTV_STD      , mTuningInfo.NR.ITUNE_ANR_PTV_STD      );
            addPair(dbgInfo , idx , ANR_ACT_TH_Y           , mTuningInfo.NR.ANR_ACT_TH_Y           );
            addPair(dbgInfo , idx , ANR_ACT_BLD_BASE_Y     , mTuningInfo.NR.ANR_ACT_BLD_BASE_Y     );
            addPair(dbgInfo , idx , ANR_ACT_BLD_TH_Y       , mTuningInfo.NR.ANR_ACT_BLD_TH_Y       );
            addPair(dbgInfo , idx , ANR_ACT_SLANT_Y        , mTuningInfo.NR.ANR_ACT_SLANT_Y        );
            addPair(dbgInfo , idx , ANR_ACT_TH_C           , mTuningInfo.NR.ANR_ACT_TH_C           );
            addPair(dbgInfo , idx , ANR_ACT_BLD_BASE_C     , mTuningInfo.NR.ANR_ACT_BLD_BASE_C     );
            addPair(dbgInfo , idx , ANR_ACT_BLD_TH_C       , mTuningInfo.NR.ANR_ACT_BLD_TH_C       );
            addPair(dbgInfo , idx , ANR_ACT_SLANT_C        , mTuningInfo.NR.ANR_ACT_SLANT_C        );
            addPair(dbgInfo , idx , RADIUS_H               , mTuningInfo.NR.RADIUS_H               );
            addPair(dbgInfo , idx , RADIUS_V               , mTuningInfo.NR.RADIUS_V               );
            addPair(dbgInfo , idx , RADIUS_H_C             , mTuningInfo.NR.RADIUS_H_C             );
            addPair(dbgInfo , idx , RADIUS_V_C             , mTuningInfo.NR.RADIUS_V_C             );
            addPair(dbgInfo , idx , ANR_PTC_HGAIN          , mTuningInfo.NR.ANR_PTC_HGAIN          );
            addPair(dbgInfo , idx , ANR_PTY_HGAIN          , mTuningInfo.NR.ANR_PTY_HGAIN          );
            addPair(dbgInfo , idx , ANR_LPF_HALFKERNEL     , mTuningInfo.NR.ANR_LPF_HALFKERNEL     );
            addPair(dbgInfo , idx , ANR_LPF_HALFKERNEL_C   , mTuningInfo.NR.ANR_LPF_HALFKERNEL_C   );
            addPair(dbgInfo , idx , ANR_ACT_MODE           , mTuningInfo.NR.ANR_ACT_MODE           );
            addPair(dbgInfo , idx , ANR_LCE_SCALE_GAIN     , mTuningInfo.NR.ANR_LCE_SCALE_GAIN     );
            addPair(dbgInfo , idx , ANR_LCE_C_GAIN         , mTuningInfo.NR.ANR_LCE_C_GAIN         );
            addPair(dbgInfo , idx , ANR_LCE_GAIN0          , mTuningInfo.NR.ANR_LCE_GAIN0          );
            addPair(dbgInfo , idx , ANR_LCE_GAIN1          , mTuningInfo.NR.ANR_LCE_GAIN1          );
            addPair(dbgInfo , idx , ANR_LCE_GAIN2          , mTuningInfo.NR.ANR_LCE_GAIN2          );
            addPair(dbgInfo , idx , ANR_LCE_GAIN3          , mTuningInfo.NR.ANR_LCE_GAIN3          );
            addPair(dbgInfo , idx , ANR_MEDIAN_LOCATION    , mTuningInfo.NR.ANR_MEDIAN_LOCATION    );
            addPair(dbgInfo , idx , ANR_CEN_X              , mTuningInfo.NR.ANR_CEN_X              );
            addPair(dbgInfo , idx , ANR_CEN_Y              , mTuningInfo.NR.ANR_CEN_Y              );
            addPair(dbgInfo , idx , ANR_R1                 , mTuningInfo.NR.ANR_R1                 );
            addPair(dbgInfo , idx , ANR_R2                 , mTuningInfo.NR.ANR_R2                 );
            addPair(dbgInfo , idx , ANR_R3                 , mTuningInfo.NR.ANR_R3                 );
            addPair(dbgInfo , idx , LUMA_ON_OFF            , mTuningInfo.NR.LUMA_ON_OFF            );
        }
        else if (mNRType == DRV_NR_OBJ_SW2 ) // fast swnr
        {
            addPair(dbgInfo , idx , NR_K      , mTuningInfo.NR2.NR_K);
            addPair(dbgInfo , idx , NR_S      , mTuningInfo.NR2.NR_S);
            addPair(dbgInfo , idx , NR_SD     , mTuningInfo.NR2.NR_SD);
            addPair(dbgInfo , idx , NR_BLD_W  , mTuningInfo.NR2.NR_BLD_W);
            addPair(dbgInfo , idx , NR_BLD_TH , mTuningInfo.NR2.NR_BLD_TH);
            addPair(dbgInfo , idx , NR_SMTH   , mTuningInfo.NR2.NR_SMTH);
        }
        else {
            SWNR_LOGE("not supported type %d", mNRType);
            return MFALSE;
        }

        // HFG
        addPair(dbgInfo , idx , HFG_ENABLE      , mTuningInfo.HFG.HFG_ENABLE);
        addPair(dbgInfo , idx , HFG_GSD         , mTuningInfo.HFG.HFG_GSD);
        addPair(dbgInfo , idx , HFG_SD0         , mTuningInfo.HFG.HFG_SD0);
        addPair(dbgInfo , idx , HFG_SD1         , mTuningInfo.HFG.HFG_SD1);
        addPair(dbgInfo , idx , HFG_SD2         , mTuningInfo.HFG.HFG_SD2);
        addPair(dbgInfo , idx , HFG_TX_S        , mTuningInfo.HFG.HFG_TX_S);
        addPair(dbgInfo , idx , HFG_LCE_LINK_EN , mTuningInfo.HFG.HFG_LCE_LINK_EN);
        addPair(dbgInfo , idx , HFG_LUMA_CPX1   , mTuningInfo.HFG.HFG_LUMA_CPX1);
        addPair(dbgInfo , idx , HFG_LUMA_CPX2   , mTuningInfo.HFG.HFG_LUMA_CPX2);
        addPair(dbgInfo , idx , HFG_LUMA_CPX3   , mTuningInfo.HFG.HFG_LUMA_CPX3);
        addPair(dbgInfo , idx , HFG_LUMA_CPY0   , mTuningInfo.HFG.HFG_LUMA_CPY0);
        addPair(dbgInfo , idx , HFG_LUMA_CPY1   , mTuningInfo.HFG.HFG_LUMA_CPY1);
        addPair(dbgInfo , idx , HFG_LUMA_CPY2   , mTuningInfo.HFG.HFG_LUMA_CPY2);
        addPair(dbgInfo , idx , HFG_LUMA_CPY3   , mTuningInfo.HFG.HFG_LUMA_CPY3);
        addPair(dbgInfo , idx , HFG_LUMA_SP0    , mTuningInfo.HFG.HFG_LUMA_SP0);
        addPair(dbgInfo , idx , HFG_LUMA_SP1    , mTuningInfo.HFG.HFG_LUMA_SP1);
        addPair(dbgInfo , idx , HFG_LUMA_SP2    , mTuningInfo.HFG.HFG_LUMA_SP2);
        addPair(dbgInfo , idx , HFG_LUMA_SP3    , mTuningInfo.HFG.HFG_LUMA_SP3);

        // CCR
        addPair(dbgInfo , idx , CCR_ENABLE       , mTuningInfo.CCR.CCR_ENABLE);
        addPair(dbgInfo , idx , CCR_CEN_U        , mTuningInfo.CCR.CCR_CEN_U);
        addPair(dbgInfo , idx , CCR_CEN_V        , mTuningInfo.CCR.CCR_CEN_V);
        addPair(dbgInfo , idx , CCR_Y_CPX1       , mTuningInfo.CCR.CCR_Y_CPX1);
        addPair(dbgInfo , idx , CCR_Y_CPX2       , mTuningInfo.CCR.CCR_Y_CPX2);
        addPair(dbgInfo , idx , CCR_Y_CPY1       , mTuningInfo.CCR.CCR_Y_CPY1);
        addPair(dbgInfo , idx , CCR_Y_SP1        , mTuningInfo.CCR.CCR_Y_SP1);
        addPair(dbgInfo , idx , CCR_UV_X1        , mTuningInfo.CCR.CCR_UV_X1);
        addPair(dbgInfo , idx , CCR_UV_X2        , mTuningInfo.CCR.CCR_UV_X2);
        addPair(dbgInfo , idx , CCR_UV_X3        , mTuningInfo.CCR.CCR_UV_X3);
        addPair(dbgInfo , idx , CCR_UV_GAIN1     , mTuningInfo.CCR.CCR_UV_GAIN1);
        addPair(dbgInfo , idx , CCR_UV_GAIN2     , mTuningInfo.CCR.CCR_UV_GAIN2);
        addPair(dbgInfo , idx , CCR_UV_GAIN_SP1  , mTuningInfo.CCR.CCR_UV_GAIN_SP1);
        addPair(dbgInfo , idx , CCR_UV_GAIN_SP2  , mTuningInfo.CCR.CCR_UV_GAIN_SP2);
        addPair(dbgInfo , idx , CCR_Y_CPX3       , mTuningInfo.CCR.CCR_Y_CPX3);
        addPair(dbgInfo , idx , CCR_Y_CPY0       , mTuningInfo.CCR.CCR_Y_CPY0);
        addPair(dbgInfo , idx , CCR_Y_CPY2       , mTuningInfo.CCR.CCR_Y_CPY2);
        addPair(dbgInfo , idx , CCR_Y_SP0        , mTuningInfo.CCR.CCR_Y_SP0);
        addPair(dbgInfo , idx , CCR_Y_SP2        , mTuningInfo.CCR.CCR_Y_SP2);
        addPair(dbgInfo , idx , CCR_UV_GAIN_MODE , mTuningInfo.CCR.CCR_UV_GAIN_MODE);
        addPair(dbgInfo , idx , CCR_MODE         , mTuningInfo.CCR.CCR_MODE);
        addPair(dbgInfo , idx , CCR_OR_MODE      , mTuningInfo.CCR.CCR_OR_MODE);
        addPair(dbgInfo , idx , CCR_HUE_X1       , mTuningInfo.CCR.CCR_HUE_X1);
        addPair(dbgInfo , idx , CCR_HUE_X2       , mTuningInfo.CCR.CCR_HUE_X2);
        addPair(dbgInfo , idx , CCR_HUE_X3       , mTuningInfo.CCR.CCR_HUE_X3);
        addPair(dbgInfo , idx , CCR_HUE_X4       , mTuningInfo.CCR.CCR_HUE_X4);
        addPair(dbgInfo , idx , CCR_HUE_SP1      , mTuningInfo.CCR.CCR_HUE_SP1);
        addPair(dbgInfo , idx , CCR_HUE_SP2      , mTuningInfo.CCR.CCR_HUE_SP2);
        addPair(dbgInfo , idx , CCR_HUE_GAIN1    , mTuningInfo.CCR.CCR_HUE_GAIN1);
        addPair(dbgInfo , idx , CCR_HUE_GAIN2    , mTuningInfo.CCR.CCR_HUE_GAIN2);

        // set zeros to remaining
        while( idx < RESERVEA_DEBUG_TAG_SIZE) {
            addPair(dbgInfo , idx , 0, 0);
        }
        //
#undef addPair
        //
        IMetadata exifMeta;
        // query from hal metadata first
        {
            IMetadata::IEntry entry= halMetadata.entryFor(MTK_3A_EXIF_METADATA);
            if (! entry.isEmpty() )
                exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
        }
        // update
        IMetadata::IEntry entry_key(MTK_POSTNR_EXIF_DBGINFO_NR_KEY);
        entry_key.push_back(DEBUG_CAM_RESERVE1_MID, Type2Type<MINT32>());
        exifMeta.update(entry_key.tag(), entry_key);
        //
        IMetadata::IEntry entry_data(MTK_POSTNR_EXIF_DBGINFO_NR_DATA);
        entry_data.push_back(memory_dbgInfo, Type2Type<IMetadata::Memory>());
        exifMeta.update(entry_data.tag(), entry_data);
        //
        IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
        entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
        halMetadata.update(entry_exif.tag(), entry_exif);
    }
    else
    {
        SWNR_LOGD("no need to dump exif");
    }
    //
    return MTRUE;
}
#endif

MVOID SwNR::dumpParam(char * const filename)
{
    // write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        SWNR_LOGE("fopen fail: %s", filename);
        return;
    }

    fprintf(fp, "swnr:%s\n\n", (mNRType == DRV_NR_OBJ_SW) ? "normal" : "fast");

    if (mNRType == DRV_NR_OBJ_SW) {
        dumpParams(&mTuningInfo.NR, fp);
        dumpParams(&mTuningInfo.HFG, fp);
        dumpParams(&mTuningInfo.CCR, fp);
    } else {
        dumpParams(&mTuningInfo.NR2, fp);
        dumpParams(&mTuningInfo.HFG, fp);
        dumpParams(&mTuningInfo.CCR, fp);
    }

    fclose(fp);
}

