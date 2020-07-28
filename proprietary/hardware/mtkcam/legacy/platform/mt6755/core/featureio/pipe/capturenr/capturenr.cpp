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
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
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
#include <mtkcam/algorithm/libnr/MTKNR.h>
//
#include <utils/Mutex.h>
#include <list>
#include <math.h>

#include <sys/mman.h>
//
#include <camera_custom_capture_nr.h>
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

/*******************************************************************************
*
********************************************************************************/
static Mutex                    g_SwParamlock;
static list<SwNRParam*>         g_SwParamlist;
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
    MY_LOGD("idx %d, in %p, out %p, %p, profile %d", openId, in, out0, out1, profile);
    if( out0 )
    {
        MY_LOGD("out0: %dx%d fmt 0x%x trans %d",
                out0->getImgSize().w,
                out0->getImgSize().h,
                out0->getImgFormat(),
                transform_0);
    }
    if( out1 )
    {
        MY_LOGD("out1: %dx%d fmt 0x%x",
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
        MY_LOGE("not supported src fmt %d", in->getImgFormat() );
        goto lbExit;
    }

    pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, openId, LOG_TAG);
    if(pHal3A == NULL) {
        MY_LOGE("IHal3A:createInstance fail");
        goto lbExit;
    }

    {
        ParamIspProfile_T _3A_profile( profile, magic, MTRUE, ParamIspProfile_T::EParamValidate_P2Only);
        MY_LOGD("set ispProfile %d, # 0x%x", profile, magic);
        pHal3A->setIspProfile(_3A_profile);
    }

    if( profile == NSIspTuning::EIspProfile_VSS_Capture_MultiPass_ANR_1 ||
        profile == NSIspTuning::EIspProfile_VSS_MFB_MultiPass_ANR_1 )
    {
        pPostProcPipe = INormalStream::createInstance(LOG_TAG, ENormalStreamTag_Vss, openId);
    } else {
        pPostProcPipe = INormalStream::createInstance(LOG_TAG, ENormalStreamTag_Cap, openId);
    }

    if(pPostProcPipe == NULL) {
        MY_LOGE("INormalStream::createInstance fail");
        goto lbExit;
    }

    if( !pPostProcPipe->init() ) {
        MY_LOGE("INormalStream::init fail");
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
            MY_LOGE("enque pass2 failed");
            goto lbExit;
        }
    }

    {
        QParams dequeParams;
        if( !pPostProcPipe->deque(dequeParams, PASS2_TIMEOUT) )
        {
            MY_LOGE("deque pass2: deque fail");
            goto lbExit;
        }
    }

    if( !pPostProcPipe->uninit() ){
        MY_LOGE("INormalStream::uninit fail");
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
    MY_LOGD("idx %d, buf %p, profile %d/%d", openId, buf, profile0, profile1);
    MBOOL                     ret = MFALSE;
    sp<IImageBuffer>          pInternalBuf;

    // check params
    if( buf->getImgFormat() != eImgFmt_YUY2 ){
        MY_LOGE("not supported src fmt %d", buf->getImgFormat() );
        goto lbExit;
    }

    {
        MY_LOGD("allocate memory");
        // allocate internal memory
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();

        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        size_t bufStrideInByte[3] = { buf->getBufStridesInBytes(0), 0, 0 };

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
            MY_LOGE("lock Buffer failed\n");
            goto lbExit;
        }

        if ( !pInternalBuf->syncCache( eCACHECTRL_INVALID ) )
        {
            MY_LOGE("syncCache failed\n");
            goto lbExit;
        }

    }

    // 1st run
    if( !doHwNR(openId, buf, NULL, pInternalBuf.get(), MRect(MPoint(0), buf->getImgSize()),0, profile0) ) {
        MY_LOGE("1st run NR failed");
        goto lbExit;
    }

    // 2nd run
    if( !doHwNR(openId, pInternalBuf.get(), NULL, buf, MRect(MPoint(0), buf->getImgSize()), 0, profile1) ) {
        MY_LOGE("2nd run NR failed");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    // release internal buf
    if( pInternalBuf.get() ) {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        if( !pInternalBuf->unlockBuf( LOG_TAG ) )
        {
            MY_LOGE("unlock Buffer failed\n");
            ret = MFALSE;
        }
        allocator->free(pInternalBuf.get());
        pInternalBuf = NULL;
    }

    return ret;
}


inline MINT32 generateSeed() {
    return (MINT32)::systemTime();
}


MVOID
mapNR(NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram, NRTuningInfo* pNR)
{
#define NRMAP(A, B) pNR->A = pNvram->B
    NRMAP(ANR_Y_LUMA_SCALE_RANGE , ANR_Y_LUMA_SCALE_RANGE);
    NRMAP(ANR_C_CHROMA_SCALE     , ANR_C_CHROMA_SCALE);
    pNR->ANR_Y_SCALE_CPY0       = 0;
    pNR->ANR_Y_SCALE_CPY1       = 0;
    pNR->ANR_Y_SCALE_CPY2       = 0;
    pNR->ANR_Y_SCALE_CPY3       = 0;
    NRMAP(ANR_Y_SCALE_CPY4       , ANR_Y_SCALE_CPY4);
    pNR->ANR_Y_CPX1             = 0;
    pNR->ANR_Y_CPX2             = 0;
    pNR->ANR_Y_CPX3             = 0;
    NRMAP(ANR_CEN_GAIN_LO_TH     , ANR_CEN_GAIN_LO_TH);
    NRMAP(ANR_CEN_GAIN_HI_TH     , ANR_CEN_GAIN_HI_TH);
    NRMAP(ANR_PTY_GAIN_TH        , ANR_PTY_GAIN_TH);
    NRMAP(ANR_KSIZE_LO_TH        , ANR_KSIZE_LO_TH);
    NRMAP(ANR_KSIZE_HI_TH        , ANR_KSIZE_HI_TH);
    NRMAP(ANR_KSIZE_LO_TH_C      , ANR_KSIZE_LO_TH_C);
    NRMAP(ANR_KSIZE_HI_TH_C      , ANR_KSIZE_HI_TH_C);
    NRMAP(ITUNE_ANR_PTY_STD      , ITUNE_ANR_PTY_STD);
    NRMAP(ITUNE_ANR_PTU_STD      , ITUNE_ANR_PTU_STD);
    NRMAP(ITUNE_ANR_PTV_STD      , ITUNE_ANR_PTV_STD);
    pNR->ANR_ACT_TH_Y           = 0;
    pNR->ANR_ACT_BLD_BASE_Y     = 0;
    pNR->ANR_ACT_BLD_TH_Y       = 0;
    pNR->ANR_ACT_SLANT_Y        = 0;
    NRMAP(ANR_ACT_TH_C           , ANR_ACT_TH_C);
    NRMAP(ANR_ACT_BLD_BASE_C     , ANR_ACT_BLD_BASE_C);
    NRMAP(ANR_ACT_BLD_TH_C       , ANR_ACT_BLD_TH_C);
    NRMAP(ANR_ACT_SLANT_C        , ANR_ACT_SLANT_C);
    NRMAP(RADIUS_H               , RADIUS_H);
    NRMAP(RADIUS_V               , RADIUS_V);
    NRMAP(RADIUS_H_C             , RADIUS_H_C);
    NRMAP(RADIUS_V_C             , RADIUS_V_C);
    NRMAP(ANR_PTC_HGAIN          , ANR_PTC_HGAIN);
    pNR->ANR_PTY_HGAIN          = 0;
    NRMAP(ANR_LPF_HALFKERNEL     , ANR_LPF_HALFKERNEL);
    NRMAP(ANR_LPF_HALFKERNEL_C   , ANR_LPF_HALFKERNEL_C);
    NRMAP(ANR_ACT_MODE           , ANR_ACT_MODE);
    NRMAP(ANR_LCE_SCALE_GAIN     , ANR_LCE_SCALE_GAIN);
    NRMAP(ANR_LCE_C_GAIN         , ANR_LCE_C_GAIN);
    pNR->ANR_LCE_GAIN0          = 0;
    pNR->ANR_LCE_GAIN1          = 0;
    pNR->ANR_LCE_GAIN2          = 0;
    pNR->ANR_LCE_GAIN3          = 0;
    NRMAP(ANR_MEDIAN_LOCATION    , ANR_MEDIAN_LOCATION);
    NRMAP(ANR_CEN_X              , ANR_CEN_X);
    NRMAP(ANR_CEN_Y              , ANR_CEN_Y);
    NRMAP(ANR_R1                 , ANR_R1);
    NRMAP(ANR_R2                 , ANR_R2);
    NRMAP(ANR_R3                 , ANR_R3);
    NRMAP(LUMA_ON_OFF            , LUMA_ON_OFF);
#undef NRMAP
}


MVOID
mapIIR(NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram, IIRTuningInfo* pNR)
{
#define IIRMAP(A, B) pNR->A = pNvram->B
    IIRMAP(NR_K        , ANR_KSIZE_HI_TH);
    IIRMAP(NR_STRENGTH , ANR_Y_LUMA_SCALE_RANGE);
    pNR->NR_SEED     = generateSeed();
    IIRMAP(BLEND_W     , ANR_C_CHROMA_SCALE);
    IIRMAP(BLEND_TH    , ANR_KSIZE_LO_TH);
    IIRMAP(DITHER_S    , ANR_LPF_HALFKERNEL);
#undef IIRMap
}


MVOID
mapHFG(NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram, HFGTuningInfo* pHFG)
{
#define HFGMAP(A, B) pHFG->A = pNvram->B
    HFGMAP(HFG_ENABLE      , ANR_PTY_HGAIN);
    pHFG->HFG_NOISE_GSEED = generateSeed();
    HFGMAP(HFG_NOISE_SEED0 , ANR_ACT_BLD_BASE_Y);
    HFGMAP(HFG_NOISE_SEED1 , ANR_ACT_BLD_TH_Y);
    HFGMAP(HFG_NOISE_SEED2 , ANR_ACT_SLANT_Y);
    HFGMAP(HFG_NOISE_STD   , ANR_PTY_GAIN_TH);
    pHFG->HFG_LCE_LINK_EN = 0;
    HFGMAP(HFG_LUMA_CPX1   , ANR_Y_CPX1);
    HFGMAP(HFG_LUMA_CPX2   , ANR_Y_CPX2);
    HFGMAP(HFG_LUMA_CPX3   , ANR_Y_CPX3);
    HFGMAP(HFG_LUMA_CPY0   , ANR_Y_SCALE_CPY0);
    HFGMAP(HFG_LUMA_CPY1   , ANR_Y_SCALE_CPY1);
    HFGMAP(HFG_LUMA_CPY2   , ANR_Y_SCALE_CPY2);
    HFGMAP(HFG_LUMA_CPY3   , ANR_Y_SCALE_CPY3);
    HFGMAP(HFG_LUMA_SP0    , ANR_LCE_GAIN0);
    HFGMAP(HFG_LUMA_SP1    , ANR_LCE_GAIN1);
    HFGMAP(HFG_LUMA_SP2    , ANR_LCE_GAIN2);
    HFGMAP(HFG_LUMA_SP3    , ANR_LCE_GAIN3);
#undef HFGMAP
}


MVOID
mapCCR(NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram, CCRTuningInfo* pCCR)
{
#define CCRMAP(A, B) pCCR->A = pNvram->B
    CCRMAP(CCR_ENABLE       , CCR_EN);
	CCRMAP(CCR_CEN_U        , CCR_CEN_U);
	CCRMAP(CCR_CEN_V        , CCR_CEN_V);
	CCRMAP(CCR_Y_CPX1       , CCR_Y_CPX1);
	CCRMAP(CCR_Y_CPX2       , CCR_Y_CPX2);
	CCRMAP(CCR_Y_CPY1       , CCR_Y_CPY1);
	CCRMAP(CCR_Y_SP1        , CCR_Y_SP1);
	CCRMAP(CCR_UV_X1        , CCR_UV_X1);
	CCRMAP(CCR_UV_X2        , CCR_UV_X2);
	CCRMAP(CCR_UV_X3        , CCR_UV_X3);
	CCRMAP(CCR_UV_GAIN1     , CCR_UV_GAIN1);
	CCRMAP(CCR_UV_GAIN2     , CCR_UV_GAIN2);
	CCRMAP(CCR_UV_GAIN_SP1  , CCR_UV_GAIN_SP1);
	CCRMAP(CCR_UV_GAIN_SP2  , CCR_UV_GAIN_SP2);
	CCRMAP(CCR_Y_CPX3       , CCR_Y_CPX3);
	CCRMAP(CCR_Y_CPY0       , CCR_Y_CPY0);
	CCRMAP(CCR_Y_CPY2       , CCR_Y_CPY2);
	CCRMAP(CCR_Y_SP0        , CCR_Y_SP0);
	CCRMAP(CCR_Y_SP2        , CCR_Y_SP2);
	CCRMAP(CCR_UV_GAIN_MODE , CCR_UV_GAIN_MODE);
	CCRMAP(CCR_MODE         , CCR_MODE);
	CCRMAP(CCR_OR_MODE      , CCR_OR_MODE);
	CCRMAP(CCR_HUE_X1       , CCR_HUE_X1);
	CCRMAP(CCR_HUE_X2       , CCR_HUE_X2);
	CCRMAP(CCR_HUE_X3       , CCR_HUE_X3);
	CCRMAP(CCR_HUE_X4       , CCR_HUE_X4);
	CCRMAP(CCR_HUE_SP1      , CCR_HUE_SP1);
	CCRMAP(CCR_HUE_SP2      , CCR_HUE_SP2);
	CCRMAP(CCR_HUE_GAIN1    , CCR_HUE_GAIN1);
	CCRMAP(CCR_HUE_GAIN2    , CCR_HUE_GAIN2);
#undef CCRMAP
}


#define DUMP_PARAM(fp, fmt, arg...)       \
    do {                                  \
        if( fp ) fprintf(fp, fmt "\n", ##arg); \
        else MY_LOGD(fmt, ##arg);         \
    } while(0)

MVOID
dumpParams(NRTuningInfo* pNR, FILE* fp = NULL)
{
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


MVOID
dumpParams(IIRTuningInfo* pNR, FILE* fp = NULL)
{
#define DUMP_IIR(param)                              \
    do {                                             \
        DUMP_PARAM(fp, "%s:%d", #param, pNR->param); \
    } while(0)
    DUMP_IIR(NR_K);
    DUMP_IIR(NR_STRENGTH);
    DUMP_IIR(NR_SEED);
    DUMP_IIR(BLEND_W);
    DUMP_IIR(BLEND_TH);
    DUMP_IIR(DITHER_S);
#undef DUMP_IIR
}


MVOID
dumpParams(HFGTuningInfo* pHFG, FILE* fp = NULL)
{
#define DUMP_HFG(param)                               \
    do {                                              \
        DUMP_PARAM(fp, "%s:%d", #param, pHFG->param); \
    } while(0)
    DUMP_HFG(HFG_ENABLE);
    DUMP_HFG(HFG_NOISE_GSEED);
    DUMP_HFG(HFG_NOISE_SEED0);
    DUMP_HFG(HFG_NOISE_SEED1);
    DUMP_HFG(HFG_NOISE_SEED2);
    DUMP_HFG(HFG_NOISE_STD);
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


MVOID
dumpParams(CCRTuningInfo* pCCR, FILE* fp = NULL)
{
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


MVOID
dumpParams(NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram, FILE* fp = NULL)
{
#define DUMP_NVRAM(param)                               \
    do {                                                \
        DUMP_PARAM(fp, "%s:%d", #param, pNvram->param); \
    } while(0)
    DUMP_NVRAM(ANR_Y_LUMA_SCALE_RANGE);
    DUMP_NVRAM(ANR_C_CHROMA_SCALE);
    DUMP_NVRAM(ANR_Y_SCALE_CPY0);
    DUMP_NVRAM(ANR_Y_SCALE_CPY1);
    DUMP_NVRAM(ANR_Y_SCALE_CPY2);
    DUMP_NVRAM(ANR_Y_SCALE_CPY3);
    DUMP_NVRAM(ANR_Y_SCALE_CPY4);
    DUMP_NVRAM(ANR_Y_CPX1);
    DUMP_NVRAM(ANR_Y_CPX2);
    DUMP_NVRAM(ANR_Y_CPX3);
    DUMP_NVRAM(ANR_CEN_GAIN_LO_TH);
    DUMP_NVRAM(ANR_CEN_GAIN_HI_TH);
    DUMP_NVRAM(ANR_PTY_GAIN_TH);
    DUMP_NVRAM(ANR_KSIZE_LO_TH);
    DUMP_NVRAM(ANR_KSIZE_HI_TH);
    DUMP_NVRAM(ANR_KSIZE_LO_TH_C);
    DUMP_NVRAM(ANR_KSIZE_HI_TH_C);
    DUMP_NVRAM(ITUNE_ANR_PTY_STD);
    DUMP_NVRAM(ITUNE_ANR_PTU_STD);
    DUMP_NVRAM(ITUNE_ANR_PTV_STD);
    DUMP_NVRAM(ANR_ACT_TH_Y);
    DUMP_NVRAM(ANR_ACT_BLD_BASE_Y);
    DUMP_NVRAM(ANR_ACT_BLD_TH_Y);
    DUMP_NVRAM(ANR_ACT_SLANT_Y);
    DUMP_NVRAM(ANR_ACT_TH_C);
    DUMP_NVRAM(ANR_ACT_BLD_BASE_C);
    DUMP_NVRAM(ANR_ACT_BLD_TH_C);
    DUMP_NVRAM(ANR_ACT_SLANT_C);
    DUMP_NVRAM(RADIUS_H);
    DUMP_NVRAM(RADIUS_V);
    DUMP_NVRAM(RADIUS_H_C);
    DUMP_NVRAM(RADIUS_V_C);
    DUMP_NVRAM(ANR_PTC_HGAIN);
    DUMP_NVRAM(ANR_PTY_HGAIN);
    DUMP_NVRAM(ANR_LPF_HALFKERNEL);
    DUMP_NVRAM(ANR_LPF_HALFKERNEL_C);
    DUMP_NVRAM(ANR_ACT_MODE);
    DUMP_NVRAM(ANR_LCE_SCALE_GAIN);
    DUMP_NVRAM(ANR_LCE_C_GAIN);
    DUMP_NVRAM(ANR_LCE_GAIN0);
    DUMP_NVRAM(ANR_LCE_GAIN1);
    DUMP_NVRAM(ANR_LCE_GAIN2);
    DUMP_NVRAM(ANR_LCE_GAIN3);
    DUMP_NVRAM(ANR_MEDIAN_LOCATION);
    DUMP_NVRAM(ANR_CEN_X);
    DUMP_NVRAM(ANR_CEN_Y);
    DUMP_NVRAM(ANR_R1);
    DUMP_NVRAM(ANR_R2);
    DUMP_NVRAM(ANR_R3);
    DUMP_NVRAM(LUMA_ON_OFF);
    DUMP_NVRAM(CCR_EN);
    DUMP_NVRAM(CCR_CEN_U);
    DUMP_NVRAM(CCR_CEN_V);
    DUMP_NVRAM(CCR_Y_CPX1);
    DUMP_NVRAM(CCR_Y_CPX2);
    DUMP_NVRAM(CCR_Y_CPY1);
    DUMP_NVRAM(CCR_Y_SP1);
    DUMP_NVRAM(CCR_UV_X1);
    DUMP_NVRAM(CCR_UV_X2);
    DUMP_NVRAM(CCR_UV_X3);
    DUMP_NVRAM(CCR_UV_GAIN1);
    DUMP_NVRAM(CCR_UV_GAIN2);
    DUMP_NVRAM(CCR_UV_GAIN_SP1);
    DUMP_NVRAM(CCR_UV_GAIN_SP2);
    DUMP_NVRAM(CCR_Y_CPX3);
    DUMP_NVRAM(CCR_Y_CPY0);
    DUMP_NVRAM(CCR_Y_CPY2);
    DUMP_NVRAM(CCR_Y_SP0);
    DUMP_NVRAM(CCR_Y_SP2);
    DUMP_NVRAM(CCR_UV_GAIN_MODE);
    DUMP_NVRAM(CCR_MODE);
    DUMP_NVRAM(CCR_OR_MODE);
    DUMP_NVRAM(CCR_HUE_X1);
    DUMP_NVRAM(CCR_HUE_X2);
    DUMP_NVRAM(CCR_HUE_X3);
    DUMP_NVRAM(CCR_HUE_X4);
    DUMP_NVRAM(CCR_HUE_SP1);
    DUMP_NVRAM(CCR_HUE_SP2);
    DUMP_NVRAM(CCR_HUE_GAIN1);
    DUMP_NVRAM(CCR_HUE_GAIN2);
#undef DUMP_NVRAM
}
#undef DUMP_PARAM

SwNRParam*
SwNRParam::
getInstance(MUINT32 openId)
{
    Mutex::Autolock _l(g_SwParamlock);

    SwNRParam* pParam = NULL;
    list<SwNRParam*>::const_iterator iter;
    for( iter = g_SwParamlist.begin() ; iter != g_SwParamlist.end(); iter++ )
    {
        if( (*iter)->mOpenId == openId )
            pParam = (*iter);
    }

    if( pParam == NULL )
    {
        //create new
        pParam = new SwNRParam(openId);
        g_SwParamlist.push_back(pParam);
    }

    return pParam;
}


SwNRParam::
SwNRParam(MUINT32 openId)
    : mOpenId(openId)
    , mIso(0)
    , mbMfll(0)
    , miPerfLevel(-1)
    , mType(eSWNRType_NUM)
{
    mpNvramData = (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)malloc(sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT));
}


SwNRParam::
~SwNRParam()
{
    free(mpNvramData);
}


MVOID
SwNRParam::
setParam(MUINT32 const iso, MBOOL const isMfll, MINT32 const perfLevel)
{
    MY_LOGD("iso = %d, mfll %d, perf lv %d", iso, isMfll, perfLevel);
    mIso = iso;
    mbMfll = isMfll;
    miPerfLevel = perfLevel;
}

MVOID
SwNRParam::
setANR_LCE_LINK(MBOOL const /*bOn*/)
{
}


MVOID
SwNRParam::
setSL2B(
    MUINT32 const /*centrx*/,
    MUINT32 const /*centry*/,
    MUINT32 const /*rr0*/,
    MUINT32 const /*rr1*/,
    MUINT32 const /*rr2*/
    )
{
}


MBOOL
SwNRParam::
getTuningFromNvram(MUINT32 idx, MVOID** ppNvram)
{
    if( idx >= eNUM_OF_SWNR_IDX )
    {
        MY_LOGE("wrong nvram idx %d", idx);
        return MFALSE;
    }

    // load some setting from nvram
    int err;
	NVRAM_CAMERA_FEATURE_STRUCT* pNvram;

    MUINT sensorDev = SENSOR_DEV_NONE;
    {
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    }

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)pNvram);
    if( err != 0 )
    {
        MY_LOGE("getBufAndRead fail, err=%d", err);
        return MFALSE;
    }

    *ppNvram = &(pNvram->swnr[idx]);

    return MTRUE;
}


MBOOL
SwNRParam::
getTuningFromFile(MUINT32 idx, MVOID* pNvram)
{
    char filename[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.swnr.readfile", filename, "");
    //
    MY_LOGD("read from file %s, idx %d", filename, idx);
    //
    class NvramReader
    {
    public:
                NvramReader() : m_fp(NULL) {}
                ~NvramReader() { if( m_fp ) fclose(m_fp); }
    public:
        MBOOL   open(const char* filename) {
            m_fp = fopen(filename, "r"); return (m_fp != NULL);
        }
        //
        MBOOL   getNextProcessedLine(String8& line) {
            int c;
            bool comment = false;
            bool skip = false;
            line.clear();
            while( (c = fgetc(m_fp)) != EOF ) {
                if ( c == '\n' )
                    return MTRUE;
                //
                if(
                        ( comment && c == '/' ) ||
                        ( c == '#' )
                  ) {
                    skip = true;
                }
                else if ( c == '/' ) {
                    comment = true;
                }
                else
                    comment = false;
                //
                if( skip )
                    return skipUntil('\n');
                //
                if(
                        ( c == '+' || c == '-' || c == '.') ||
                        ( c >= '0' && c <= '9' ) ||
                        ( c == '{' || c == '}' || c == ',' )
                  )
                {
                    char ch = c;
                    line.append(&ch, 1);
                }
                //
            }
            return MFALSE;
        }
        MBOOL   skipUntil(const char target) {
            int c;
            while( (c = fgetc(m_fp)) != EOF )
            {
                if( c == target )
                    return MTRUE;
            }
            return MFALSE;
        }
    protected:
        FILE*   m_fp;
    } aNvramReader;

    if( ! aNvramReader.open(filename) ) {
        MY_LOGE("cannot open %s", filename);
        return MFALSE;
    }
    //
    String8 nvramfile;
    {
        String8 line;
        while( aNvramReader.getNextProcessedLine(line) )
        {
            if( line.size() == 0 )
                continue;
            //MY_LOGD("%s", line.string());
            nvramfile += line;
        }
    }
    //
    {
        class ParseHelper {
        public:
                    ParseHelper(String8& string)
                        : mString(string)
                        , mCurIdx(0)
                        , mCurBlockLevel(0)
                    {}
                    ~ParseHelper() {}
        public:
            MBOOL skip(const char c) {
                for( size_t i = mCurIdx; i < mString.size(); i++ )
                {
                    char ch = mString.string()[i];
                    if( ch == c ) {
                        mCurIdx = i + 1;
                        return MTRUE;
                    }
                }
                return MFALSE;
            }

            MBOOL skipBlock() {
                int beginLevel = mCurBlockLevel;
                for( size_t i = mCurIdx; i < mString.size(); i++ )
                {
                    char ch = mString.string()[i];
                    if( ch == '{' )
                        mCurBlockLevel++;
                    else if ( ch == '}' ) {
                        mCurBlockLevel--;
                        if( mCurBlockLevel == beginLevel ) {
                            mCurIdx = i + 1;
                            return MTRUE;
                        }
                    }
                }
                return MFALSE;
            }
            //
            MBOOL getInt(MINT32* d) {
                int c = 0;
                bool isDigit = false;
                do {
                    char ch = mString.string()[mCurIdx+c];
                    if( ch == '+' || ch =='-' || (ch >= '0' && ch <='9') ) {
                        isDigit = true;
                        c++;
                    } else
                        isDigit = false;
                }
                while( isDigit );
                //
                if( c == 0 ) return MFALSE;
                //
                String8 digit(mString.string() + mCurIdx, c);
                *d = atoi(digit.string());
                mCurIdx += (c + 1);
                return MTRUE;
            }
            //MVOID dump() { MY_LOGD("%s", mString.string() + mCurIdx); }
            //
        private:
            String8 mString;
            ssize_t mCurIdx;
            //
            int     mCurBlockLevel;
        } aParser(nvramfile);
        //
#define CHECK_PARSE( ret )  \
        do{ if ( !ret ) { MY_LOGE("not correct format"); return MFALSE; } } while(0)
        // skip 2 blocks { ... }
        CHECK_PARSE( aParser.skipBlock() );
        CHECK_PARSE( aParser.skipBlock() );
        //
        // begining of swnr part
        CHECK_PARSE( aParser.skip('{') );
        //
        for( unsigned int i = 0 ; i < idx; i++ ) {
            CHECK_PARSE( aParser.skipBlock() );
        }
        //
        CHECK_PARSE( aParser.skip('{') );
        int count = sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) / sizeof(MINT32);
        MINT32* pTarget = reinterpret_cast<MINT32*>(pNvram);
        for( int i = 0; i < count; i++ )
        {
            MINT32 d = 0;
            CHECK_PARSE( aParser.getInt(&d) );
            *(pTarget + i) = d;
        }
#undef CHECK_PARSE
    }
    //
    return MTRUE;
}


MBOOL
SwNRParam::
update()
{
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MUINT sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
    MUINT32 u4SensorID;
    {
        SensorStaticInfo rSensorStaticInfo;
        switch  ( sensorDev )
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
                MY_ERR("Invalid sensor device: %d", sensorDev);
                return MFALSE;
        }
        u4SensorID = rSensorStaticInfo.sensorDevID;
        MY_LOGD("sensorDev(%d), u4SensorID(%d)", sensorDev, u4SensorID);
    }

    // 1. update swnr type
    mType = get_swnr_type(sensorDev);
    if( mType < 0 || mType >= eSWNRType_NUM ) {
        MY_LOGE("incorrect swnr type %d", mType);
        return MFALSE;
    }
    //
    bool readFromFile = false;
    { // debug
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.type", value, "-1");
        MINT32 type = atoi(value);
        if( type != -1 ) {
            MY_LOGD("force use swnr: %s", type == 0 ? "normal" : "fast");
            mType = type;
        }
        property_get("vendor.debug.swnr.readfile", value, "");
        readFromFile = (strlen(value) != 0);
    }
    //
    // 2. read from nvram & smooth
    if( is_to_invoke_swnr_interpolation(mbMfll, mIso) )
    {
        EIdxSwNR idx_low = map_ISO_value_to_index(mIso, mbMfll, 1);
        EIdxSwNR idx_high = map_ISO_value_to_index(mIso, mbMfll, 2);
        MUINT32 iso_low = map_index_to_ISO_value(idx_low);
        MUINT32 iso_high = map_index_to_ISO_value(idx_high);
        //
        MY_LOGD("%d: iso %d, isMfll %d, idx low/high %d/%d",
                mOpenId,
                mIso,
                mbMfll,
                idx_low,
                idx_high
               );
        //
        MVOID* pNvramLow = NULL;
        MVOID* pNvramHigh = NULL;
        if( readFromFile )
        {
            pNvramLow =
                (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)malloc(sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT));
            pNvramHigh =
                (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)malloc(sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT));
            if( !getTuningFromFile(idx_low, pNvramLow) ||
                !getTuningFromFile(idx_high, pNvramHigh)
              )
            {
                MY_LOGE("update from file failed");
                free(pNvramLow);
                free(pNvramHigh);
                return MFALSE;
            }
            //
            //dumpParams(reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramLow));
            //dumpParams(reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramHigh));
        }
        else
        if( !getTuningFromNvram(idx_low, &pNvramLow) ||
            !getTuningFromNvram(idx_high, &pNvramHigh) )
        {
            MY_LOGE("update from nvram failed");
            return MFALSE;
        }
        //
        MY_LOGD("sensorDev(%d), u4SensorID(%d)", sensorDev, u4SensorID);
        IspTuningCustom* pIspTuningCustom = IspTuningCustom::createInstance(
                static_cast<NSIspTuning::ESensorDev_T>(sensorDev), u4SensorID
                );
        // interpolation
        SmoothSWNR(pIspTuningCustom->remap_ISO_value(mIso),
                iso_high,
                iso_low,
                *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramHigh),
                *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pNvramLow),
                *reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(mpNvramData)
                );
        pIspTuningCustom->destroyInstance();
        //
        if( readFromFile ) {
            free(pNvramLow);
            free(pNvramHigh);
        }
    }
    else
    {
        MUINT32 idx = map_ISO_value_to_index(mIso, mbMfll);
        MY_LOGD("%d: iso %d, isMfll %d, idx %d",
                mOpenId, mIso, mbMfll, idx
               );

        // get nvram
        MVOID* pTuningNvram = NULL;
        if( readFromFile )
        {
            pTuningNvram =
                (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)malloc(sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT));
            if( !getTuningFromFile(idx, pTuningNvram) )
            {
                MY_LOGE("update from file failed");
                return MFALSE;
            }
        }
        else
        if( !getTuningFromNvram(idx, &pTuningNvram) )
        {
            MY_LOGE("update from nvram failed");
            return MFALSE;
        }

        // update
        memcpy(mpNvramData, pTuningNvram, sizeof(NVRAM_CAMERA_FEATURE_SWNR_STRUCT) );
        //
        if( readFromFile ) {
            free(pTuningNvram);
        }
    }
    //
    return MTRUE;
}


MBOOL
SwNRParam::
getTuning(NRTuningInfo* pNR, HFGTuningInfo* pHFG, CCRTuningInfo* pCCR)
{
    if( ! pNR || ! pHFG || ! pCCR ) {
        MY_LOGE("pNR(%p), pHFG(%p), pCCR(%p)", pNR, pHFG, pCCR );
        return MFALSE;
    }
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram = (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)mpNvramData;
    // mapping
    mapNR(pNvram, pNR);
    mapHFG(pNvram, pHFG);
    mapCCR(pNvram, pCCR);
    //
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug ) {
            dumpParams(pNR);
            dumpParams(pHFG);
            dumpParams(pCCR);
        }
    }
    return MTRUE;
}


MBOOL
SwNRParam::
getTuning(IIRTuningInfo* pNR, HFGTuningInfo* pHFG, CCRTuningInfo* pCCR)
{
    if( ! pNR || ! pHFG || ! pCCR ) {
        MY_LOGE("pNR(%p), pHFG(%p), pCCR(%p)", pNR, pHFG, pCCR );
        return MFALSE;
    }
    NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram = (NVRAM_CAMERA_FEATURE_SWNR_STRUCT*)mpNvramData;
    // mapping
    mapIIR(pNvram, pNR);
    mapHFG(pNvram, pHFG);
    mapCCR(pNvram, pCCR);
    //
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("vendor.debug.swnr.enable", value, "0");
        MBOOL bDebug = atoi(value);

        if( bDebug ) {
            dumpParams(pNR);
            dumpParams(pHFG);
            dumpParams(pCCR);
        }
    }
    return MTRUE;
}


MBOOL
SwNRParam::
dumpParamsToFile(char * const filename)
{
    // write to file
    FILE *fp = fopen(filename, "w");
    if (!fp) {
        MY_LOGE("fopen fail: %s", filename);
        return MFALSE;
    }
    else {
//doesn't have ANR_ENC control
#if 0
        fprintf(fp, "%d\n", get_normal_SWNR_ENC_enable_ISO_threshold());
        fprintf(fp, "%d\n", get_MFB_SWNR_ENC_enable_ISO_threshold());
        fprintf(fp, "\n");
#endif
        {
            IHalSensorList* const pHalSensorList = IHalSensorList::get();
            MUINT sensorDev = pHalSensorList->querySensorDevIdx(mOpenId);
            mType = get_swnr_type(sensorDev);
            //
            fprintf(fp, "swnr:%s\n\n", mType == 0 ? "normal" : "fast");
        }
        for( MUINT32 i = 0; i < NVRAM_SWNR_TBL_NUM; i++ )
        {
            MVOID* pTuningNvram = NULL;
            NVRAM_CAMERA_FEATURE_SWNR_STRUCT* pNvram= NULL;
            if( !getTuningFromNvram(i, &pTuningNvram) ) {
                MY_LOGE("cannot get tuning from nvram");
                break;
            }
            //
            pNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_SWNR_STRUCT*>(pTuningNvram);
            dumpParams(pNvram, fp);
            fprintf(fp, "\n");

        }
        fclose(fp);
    }

    return MTRUE;
}


SwNR::
SwNR(MUINT32 const openId)
    : muOpenId(openId)
    , mNRType(0)
    , mpNRTuningInfo(NULL)
    , mpIIRTuningInfo(NULL)
    , mpHFGTuningInfo(NULL)
    , mpCCRTuningInfo(NULL)
    , muWorkingBufSize(0)
    , mpWorkingBuf(NULL)
{
    mpNRInitInfo   = (NRInitInfo*)malloc(sizeof(NRInitInfo));
    mpNRImageInfo  = (NRImageInfo*)malloc(sizeof(NRImageInfo));
    mpNRResultInfo = (NRResultInfo*)malloc(sizeof(NRResultInfo));
    // initial
    mpNRInitInfo->pWorkingBuff    = 0;
    mpNRInitInfo->WorkingBuffSize = 0;
    mpNRInitInfo->CoreNumber      = 4;
    mpNRInitInfo->NumOfExecution  = 1;
    mpNRInitInfo->pTuningInfo     = NULL;
    mpNRInitInfo->pCCRTuningInfo  = NULL;
    mpNRInitInfo->pHFGTuningInfo  = NULL;
    mpNRInitInfo->pPerfInfo       = NULL;
}


SwNR::
~SwNR()
{
#define MY_FREE( ptr ) \
    if( ptr )          \
    {                  \
        free(ptr);     \
        ptr = NULL;    \
    }

    MY_FREE(mpNRInitInfo->pPerfInfo);
    MY_FREE(mpNRInitInfo);
    MY_FREE(mpNRTuningInfo);
    MY_FREE(mpIIRTuningInfo);
    MY_FREE(mpHFGTuningInfo);
    MY_FREE(mpCCRTuningInfo);
    MY_FREE(mpNRImageInfo);
    MY_FREE(mpNRResultInfo);

#undef MY_FREE
    if( mpWorkingBuf )
    {
        munmap(mpWorkingBuf, sizeof(MUINT8) * muWorkingBufSize);
        mpWorkingBuf = NULL;
    }
}

MBOOL
SwNR::
prepareTuningInfo()
{
    MBOOL ret = MFALSE;
    SwNRParam* pParam = SwNRParam::getInstance(muOpenId);
    MINT32 perfLevel = -1;
    //
    if( !pParam ) {
        MY_LOGE("cannot get SwNRParam");
        goto lbExit;
    }
    //
    if( ! pParam->update() ) {
        MY_LOGW("cannot update properly");
        goto lbExit;
    }
    //
    perfLevel = pParam->getPerfLevel();
    //
    switch( pParam->getNRType() )
    {
        case eSWNRType_Default:
            mNRType = DRV_NR_OBJ_SW;
            //
            if( ! mpNRTuningInfo ) mpNRTuningInfo = (NRTuningInfo*)malloc(sizeof(NRTuningInfo));
            if( mpIIRTuningInfo ) { free(mpIIRTuningInfo); mpIIRTuningInfo = NULL; }
            if( ! mpHFGTuningInfo ) mpHFGTuningInfo = (HFGTuningInfo*)malloc(sizeof(HFGTuningInfo));
            if( ! mpCCRTuningInfo ) mpCCRTuningInfo = (CCRTuningInfo*)malloc(sizeof(CCRTuningInfo));
            //
            if( ! pParam->getTuning(mpNRTuningInfo, mpHFGTuningInfo, mpCCRTuningInfo) ) {
                MY_LOGE("getTuning failed");
                goto lbExit;
            }
            //
            mpNRInitInfo->pTuningInfo    = mpNRTuningInfo;
            mpNRInitInfo->pCCRTuningInfo = mpCCRTuningInfo;
            mpNRInitInfo->pHFGTuningInfo = mpHFGTuningInfo;
            break;
        case eSWNRType_IIR:
        default:
            mNRType = DRV_NR_OBJ_IIR;
            //
            if( mpNRTuningInfo ) { free(mpNRTuningInfo); mpNRTuningInfo = NULL; }
            if( ! mpIIRTuningInfo ) mpIIRTuningInfo = (IIRTuningInfo*)malloc(sizeof(IIRTuningInfo));
            if( ! mpHFGTuningInfo ) mpHFGTuningInfo = (HFGTuningInfo*)malloc(sizeof(HFGTuningInfo));
            if( ! mpCCRTuningInfo ) mpCCRTuningInfo = (CCRTuningInfo*)malloc(sizeof(CCRTuningInfo));
            //
            if( ! pParam->getTuning(mpIIRTuningInfo, mpHFGTuningInfo, mpCCRTuningInfo) ) {
                MY_LOGE("getTuning failed");
                goto lbExit;
            }
            //
            mpNRInitInfo->pIIRTuningInfo = mpIIRTuningInfo;
            mpNRInitInfo->pCCRTuningInfo = mpCCRTuningInfo;
            mpNRInitInfo->pHFGTuningInfo = mpHFGTuningInfo;
            break;
    }
    //
    ret = MTRUE;
lbExit:
    //
    // set performance level
    if( perfLevel != -1 ) {
        if( mpNRInitInfo->pPerfInfo == NULL ) {
            mpNRInitInfo->pPerfInfo = new NRPerfGear;
        }
        //
        switch(perfLevel)
        {
#define PerfCase(val, level)                             \
            case val:                                    \
                mpNRInitInfo->pPerfInfo->enable = 1;     \
                mpNRInitInfo->pPerfInfo->option = level; \
                break;
            PerfCase(2, NR_PERF_GEAR_PERF_FIRST)
            PerfCase(1, NR_PERF_GEAR_CUSTOM_0)
            PerfCase(0, NR_PERF_GEAR_POWER_FIRST)
            default:
                MY_LOGW("not valid level %d", perfLevel);
                delete mpNRInitInfo->pPerfInfo;
                mpNRInitInfo->pPerfInfo = NULL;
#undef PerfCase
        }
    }
    //
    return ret;
}


MBOOL
SwNR::
setImageInfo(IImageBuffer* pBuf)
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
            MY_LOGE("not supported format: 0x%x", pBuf->getImgFormat() );
            goto lbExit;
    }

    // set image info
    mpNRImageInfo->Width = pBuf->getImgSize().w;
    mpNRImageInfo->Height = pBuf->getImgSize().h;
    mpNRImageInfo->pImg[0] = (void*)pBuf->getBufVA(0);
    if( NRFmt == NR_IMAGE_PACKET_YUY2 )
    {
        mpNRImageInfo->pImg[1] = (void*)0;
        mpNRImageInfo->pImg[2] = (void*)0;
    }
    else
    {
        mpNRImageInfo->pImg[1] = (void*)pBuf->getBufVA(1);
        mpNRImageInfo->pImg[2] = (void*)pBuf->getBufVA(2);
    }
    mpNRImageInfo->ImgNum = 1;
    mpNRImageInfo->ImgFmt = NRFmt;

    ret = MTRUE;
lbExit:
    return ret;
}


MBOOL
SwNR::
allocWorkingBuf(MUINT32 size)
{
    MBOOL ret = MFALSE;
    // allocate working buffer
    if( size != muWorkingBufSize )
    {
        if( mpWorkingBuf )
            free(mpWorkingBuf);
        mpWorkingBuf = (MUINT8*)mmap(NULL, sizeof(MUINT8) * size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
        if( mpWorkingBuf == MAP_FAILED )
        {
            MY_LOGE("allocate working buffer failed");
            goto lbExit;
        }
        MY_LOGD("allocate working buffer %p, size %d", mpWorkingBuf, size);
        muWorkingBufSize = size;
    }

    mpNRInitInfo->pWorkingBuff = mpWorkingBuf;
    mpNRInitInfo->WorkingBuffSize = muWorkingBufSize;

    ret = MTRUE;
lbExit:
    return ret;
}


MBOOL
SwNR::
doSwNR(IImageBuffer* pBuf)
{
#define CHECK_NR_RET( NR_ret )                     \
    do{                                            \
        MRESULT nr_ret = (NR_ret);                 \
        if( nr_ret != S_NR_OK ) {                  \
            MY_LOGE("NR with error 0x%x", nr_ret); \
            goto lbExit;                           \
        }                                          \
    }                                              \
    while(0);

    MY_LOGD("idx %d, buf %p", muOpenId, pBuf);
    MBOOL ret = MFALSE;
    //
    MUINT32 workingbufsize = 0;
    MTKNR* pMTKNR = NULL;

    // get tuning info
    if( !prepareTuningInfo() )
        goto lbExit;
    //
    MY_LOGD("swnr type %s", mNRType == DRV_NR_OBJ_SW ? "normal" : "fast");
    pMTKNR = MTKNR::createInstance(static_cast<DrvNRObject_e>(mNRType));
    //
    CHECK_NR_RET( pMTKNR->NRInit((MUINT32*) mpNRInitInfo, 0) );

    if( !setImageInfo(pBuf) )
        goto lbExit;

    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_GET_WORKBUF_SIZE,
                (void *)mpNRImageInfo,
                (void *)&workingbufsize)
            );

    if( !allocWorkingBuf(workingbufsize) )
        goto lbExit;

    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_SET_WORKBUF_ADDR,
                mpNRInitInfo->pWorkingBuff,
                NULL)
            );

    // add img
    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_ADD_IMG,
                (void *)mpNRImageInfo,
                NULL)
            );

    MY_LOGD("start+");
    // main
    CHECK_NR_RET( pMTKNR->NRMain() );
    MY_LOGD("start-");

    CHECK_NR_RET(
            pMTKNR->NRFeatureCtrl(
                NR_FEATURE_GET_RESULT,
                NULL,
                (void *)mpNRResultInfo)
            );

    pBuf->syncCache(eCACHECTRL_FLUSH);

    pMTKNR->NRReset();

#undef CHECK_NR_RET
    ret = MTRUE;
lbExit:
    if( pMTKNR ) {
        pMTKNR->destroyInstance(pMTKNR);
        pMTKNR = NULL;
    }

    return MTRUE; //FIXME, workaround to prevent user has no error-handling
}


