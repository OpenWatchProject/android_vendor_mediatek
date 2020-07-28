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
#define LOG_TAG "PostProcPipe"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "PostProcPipe.h"
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h>
#include <mtkcam/drv/def/ICam_type.h>
#include <ispio_utility.h>
//
#include <cutils/properties.h>  // For property_get().
#include <utils/Trace.h> //for systrace
//#include <mtkcam/featureio/eis_type.h>

/*******************************************************************************
*
********************************************************************************/

#define FORCE_EN_DIFFERENR_VIEW_TEST 0

namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(PostProc );
// Clear previous define, use our own define.
#undef PIPE_VRB
#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_WRN
#undef PIPE_ERR
#undef PIPE_AST
#define PIPE_VRB(fmt, arg...)        do { if (PostProc_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define PIPE_DBG(fmt, arg...)        do { if (PostProc_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define PIPE_INF(fmt, arg...)        do { if (PostProc_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define PIPE_WRN(fmt, arg...)        do { if (PostProc_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define PIPE_ERR(fmt, arg...)        do { if (PostProc_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define PIPE_AST(cond, fmt, arg...)  do { if (PostProc_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FM_ACC_RES_ADDR 0x15022f48

#define MAX_LOG_SIZE    1024


#include <sys/types.h>
#include <unistd.h>
#include <cutils/trace.h>
#define ATRACE_TAG ATRACE_TAG_ALWAYS

static unsigned int addrList = FM_ACC_RES_ADDR;
//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);

MINT32 u4PortID[DMA_PORT_TYPE_NUM][DMA_OUT_PORT_NUM] = {/*0*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_WROTO, (MINT32)EPortIndex_JPEGO, (MINT32)EPortIndex_FEO, (MINT32)EPortIndex_VIPI},
                                                        /*1*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_VIPI, (MINT32)-1,  (MINT32)-1,             (MINT32)-1},
                                                        /*2*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WROTO, (MINT32)EPortIndex_VIPI, (MINT32)-1,  (MINT32)-1,             (MINT32)-1},
                                                        /*3*/{(MINT32)EPortIndex_WDMAO, (MINT32)-1, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*4*/{(MINT32)EPortIndex_WROTO, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*5*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_VIPI,  (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*6*/{(MINT32)EPortIndex_IMG2O, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*7*/{(MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_VIPI,  (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*8*/{(MINT32)EPortIndex_FEO,   (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        };

 Scen_Map_CropPathInfo_STRUCT mCropPathInfo[]
	=	{
    //eDrvScenario_CC
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //eDrvScenario_VSS
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss,             (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss,             (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss,             (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss,             (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //eDrvScenario_CC_MFB_Blending
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Bld, (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    //eDrvScenario_CC_MFB_Mixing
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix,   (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix,   (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix,   (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //eDrvScenario_CC_vFB_FB
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB,       (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[7][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //eDrvScenario_Bokeh
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Bokeh,       (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[7][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Bokeh,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Bokeh,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //temp
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //temp
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    //
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_COLOR_EFT,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_COLOR_EFT,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_COLOR_EFT,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_COLOR_EFT,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //Denoise
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_DeNoise,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_DeNoise,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_DeNoise,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_DeNoise,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //WUV
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_WUV,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_WUV,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_WUV,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_WUV,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
};


#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h
TransformMapping mTransformMapping[TRANSFORM_MAX]=
{
    {0,                 NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H, NSImageio::NSIspio::eImgRot_0,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V, NSImageio::NSIspio::eImgRot_180,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_180,NSImageio::NSIspio::eImgRot_180,NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90, NSImageio::NSIspio::eImgFlip_OFF},
    {eTransform_FLIP_H|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_270,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_FLIP_V|eTransform_ROT_90, NSImageio::NSIspio::eImgRot_90,  NSImageio::NSIspio::eImgFlip_ON},
    {eTransform_ROT_270,NSImageio::NSIspio::eImgRot_270,NSImageio::NSIspio::eImgFlip_OFF}
};


MUINT32 ConvertPortIndexToCapInfoIdx(
        MUINT32 portIDIdx
)
{
	MUINT32 CapInfoIdx;

    switch(portIDIdx){
        case EPortIndex_IMGO:
			CapInfoIdx = 0;
			break;
        case EPortIndex_IMG2O:
			CapInfoIdx = 1;
			break;
        case EPortIndex_IMG3O:
        case EPortIndex_IMG3BO:
        case EPortIndex_IMG3CO:
			CapInfoIdx = 2;
			break;
        case EPortIndex_WDMAO:
			CapInfoIdx = 3;
			break;
        case EPortIndex_WROTO:
			CapInfoIdx = 4;
			break;
        case EPortIndex_JPEGO:
			CapInfoIdx = 5;
            break;
        default:
            PIPE_ERR("[Error]Not support this PortIndex(%d) mapping",portIDIdx);
            CapInfoIdx = 0;
            break;
    };
	return CapInfoIdx;

}

std::string DmaPortMapping(MUINT32 index){
    std::string str = "Unknown DMA Port";
    char str_idx[32];
    switch(index){
        case EPortIndex_LSCI:
            str="LSCI";
            break;
        case EPortIndex_IMGI:
            str="IMGI";
            break;
        case EPortIndex_VIPI:
            str="VIPI";
            break;
        case EPortIndex_VIP3I:
            str="VIPCI";
            break;
        case EPortIndex_UFDI:
            str="UFDI";
            break;
        case EPortIndex_LCEI:
            str="LCEI";
            break;
        case EPortIndex_DMGI:
            str="DMGI";
            break;
        case EPortIndex_DEPI:
            str="DEPI";
            break;
        case EPortIndex_IMG2O:
            str="IMG2O";
            break;
        case EPortIndex_IMG2BO:
            str="IMG2BO";
            break;
        case EPortIndex_IMG3O:
            str="IMG3O";
            break;
        case EPortIndex_IMG3BO:
            str="IMG3BO";
            break;
        case EPortIndex_IMG3CO:
            str="IMG3CO";
            break;
        case EPortIndex_FEO:
            str="FEO";
            break;
        case EPortIndex_WROTO:
            str="WROTO";
            break;
        case EPortIndex_WDMAO:
            str="WDMAO";
            break;
        case EPortIndex_JPEGO:
            str="JPEGO";
            break;
        case EPortIndex_VENC_STREAMO:
            str="VENC_STREAMO";
            break;
        case EPortIndex_REGI:
            str="REGI";
            break;
        case EPortIndex_TIMGO:
            str="TIMGO";
            break;
        case EPortIndex_DCESO:
            str="DCESO";
            break;
        case EPortIndex_BPCI:
            str="BPCI";
            break;
        case VirDIPPortIdx_YNR_FACEI:
            str="YNR_FACEI";
            break;
        case VirDIPPortIdx_YNR_LCEI:
            str="YNR_LCEI";
            break;
        case VirDIPPortIdx_LPCNR_YUVI:
            str="LPCNR_YUVI";
            break;
        case VirDIPPortIdx_LPCNR_YUVO:
            str="LPCNR_YUVO";
            break;
        case VirDIPPortIdx_LPCNR_UVI:
            str="LPCNR_UVI";
            break;
        case VirDIPPortIdx_LPCNR_UVO:
            str="LPCNR_UVO";
            break;
        case VirDIPPortIdx_GOLDENFRMI:
            str="GOLDENFRMI";
            break;
        case VirDIPPortIdx_WEIGHTMAPI:
            str="WEIGHTMAPI";
            break;
        case VirDIPPortIdx_CNR_BLURMAPI:
            str="CNR_BLURMAPI";
            break;
        case VirDIPPortIdx_LFEOI:
            str="LFEOI";
            break;
        case VirDIPPortIdx_RFEOI:
            str="RFEOI";
            break;
        case VirDIPPortIdx_FMO:
            str="FMO";
            break;
        case EPortIndex_IMGBI:
        case EPortIndex_IMGCI:
        case EPortIndex_VIP2I:
            PIPE_ERR("[Error] P2 unexpected this PortIndex(%d)",index);
            sprintf(str_idx,"(%d)",index);
            str=str+str_idx;
            break;
        default:
            PIPE_ERR("[Error] P2 Not support this PortIndex(%d)",index);
            sprintf(str_idx,"(%d)",index);
            str=str+str_idx;
            break;
    }
    return str;
}

std::string ColorFormatMapping(EImageFormat eImgFmt){

    std::string str = "Unknown Format";
    char str_idx[32];
    switch(eImgFmt){
        case eImgFmt_BAYER8:
            str="bayer";
            break;
        case eImgFmt_BAYER10:
            str="bayer10";
            break;
        case eImgFmt_FG_BAYER8:
            str="fg_bayer8";
            break;
        case eImgFmt_FG_BAYER10:
            str="fg_bayer10";
            break;
        case eImgFmt_RGBA8888:
            str="rgba8888";
            break;
        case eImgFmt_RGBX8888:
            str="rgbx8888";
            break;
        case eImgFmt_RGB888:
            str="rgb888";
            break;
        case eImgFmt_RGB565:
            str="rgb565";
            break;
        case eImgFmt_BGRA8888:
            str="bgra8888";
            break;
        case eImgFmt_YUY2:      // YUYV = YUY2
            str="yuyv";
            break;
        case eImgFmt_YVYU:
            str="yvyu";
            break;
        case eImgFmt_UYVY:
            str="uyvy";
            break;
        case eImgFmt_VYUY:
            str="vyuy";
            break;
        case eImgFmt_YV12:
            str="yv12";
            break;
        case eImgFmt_YV16:
            str="yv16";
            break;
        case eImgFmt_NV12:
            str="nv12";
            break;
        case eImgFmt_NV12_BLK:
            str="nv12_blk";
            break;
        case eImgFmt_NV21:
            str="nv21";
            break;
        case eImgFmt_NV21_BLK:
            str="nv21_blk";
            break;
        case eImgFmt_I420:
            str="i420";
            break;
        case eImgFmt_I422:
            str="i422";
            break;
        case eImgFmt_STA_2BYTE:
            str="sta_2byte";
            break;
        case eImgFmt_STA_4BYTE:
            str="sta_4byte";
            break;
        default:
            sprintf(str_idx,"(0x%x)",eImgFmt);
            str=str+str_idx;
            break;
    }
    return str;
}

MINT32 ColorBppMapping(EImageFormat eImgFmt){

    int bpp = 0;

    switch(eImgFmt){
        case eImgFmt_BAYER8:
            bpp=32;
            break;
        case eImgFmt_BAYER10:
            bpp=40;
            break;
        case eImgFmt_FG_BAYER8:
            bpp=48;
            break;
        case eImgFmt_FG_BAYER10:
            bpp=60;
            break;
        case eImgFmt_RGBA8888:
            bpp=32;
            break;
        case eImgFmt_RGBX8888:
            bpp=32;
            break;
        case eImgFmt_RGB888:
            bpp=24;
            break;
        case eImgFmt_RGB565:
            bpp=16;
            break;
        case eImgFmt_BGRA8888:
            bpp=32;
            break;
        case eImgFmt_YUY2:      // YUYV = YUY2
            bpp=16;
            break;
        case eImgFmt_YVYU:
            bpp=16;
            break;
        case eImgFmt_UYVY:
            bpp=16;
            break;
        case eImgFmt_VYUY:
            bpp=16;
            break;
        case eImgFmt_YV12:
            bpp=12;
            break;
        case eImgFmt_YV16:
            bpp=16;
            break;
        case eImgFmt_NV12:
            bpp=12;
            break;
        case eImgFmt_NV12_BLK:
            bpp=12;
            break;
        case eImgFmt_NV21:
            bpp=12;
            break;
        case eImgFmt_NV21_BLK:
            bpp=12;
            break;
        case eImgFmt_I420:
            bpp=12;
            break;
        case eImgFmt_I422:
            bpp=16;
            break;
        case eImgFmt_STA_2BYTE:
            bpp=16;
            break;
        default:
            break;
    }
    return bpp;
}



////////////////////////////////////////////////////////////////////////////////
IPostProcPipe* IPostProcPipe::createInstance(void)
{
    PostProcPipe* pPipeImp = new PostProcPipe();
    if  ( ! pPipeImp )
    {
        PIPE_ERR("[IPostProcPipe] fail to new PostProcPipe");
        return  NULL;
    }
    return  pPipeImp;
}


/*******************************************************************************
*
********************************************************************************/
MVOID IPostProcPipe:: destroyInstance(void)
{
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
*
********************************************************************************/
PostProcPipe::
PostProcPipe()
    : m_pIspDrvShell(NULL),
      //jpg_WorkingMem_memId(-1), //no used
      //jpg_WorkingMem_Size(0), //no used
      //jpg_WorkingMem_Size(0), //no used
      //pJpgConfigVa(NULL), //no used
      //m_isImgPlaneByImgi(MFALSE), //no used
      //m_Nr3dEn(MFALSE), //no used
      //m_Nr3dDmaSel(MFALSE), //no used
      //mp2CQDupIdx(0), //no used
      pPortLogBuf(NULL),
      pMaxLogBuf(NULL),
      //m_SeeeEn(MFALSE), //no used
      m_CrzEn(MFALSE),
      m_Img3oCropInfo(NULL),
      m_RawHDRType(NSCam::NSIoPipe::EDIPRawHDRType_NONE),
      m_isMfbDL(MFALSE),
      m_pDipWorkingBuf(NULL),
      m_pDequeDipWorkingBuf(NULL)
{

    //
    DBG_LOG_CONFIG(imageio, PostProc);
    portInfo_imgi = NULL;
    portInfo_imgbi = NULL;
    portInfo_imgci = NULL;
    portInfo_vipi = NULL;
    portInfo_vip2i = NULL;
    portInfo_vip3i = NULL;
    portInfo_ufdi = NULL;
    portInfo_lcei = NULL;
    portInfo_dmgi = NULL;
    portInfo_depi = NULL;
    portInfo_regi = NULL;
    portInfo_img2o = NULL;
    portInfo_img2bo = NULL;
    portInfo_img3o = NULL;
    portInfo_img3bo = NULL;
    portInfo_img3co = NULL;
    portInfo_feo = NULL;
    portInfo_timgo = NULL;
    portInfo_dceso = NULL;
    portInfo_wroto = NULL;
    portInfo_wdmao = NULL;
    portInfo_jpego = NULL;
    portInfo_venco = NULL;

    eImgFmt_imgi = eImgFmt_UNKNOWN;
    eImgFmt_imgbi = eImgFmt_UNKNOWN;
    eImgFmt_imgci = eImgFmt_UNKNOWN;
    eImgFmt_vipi = eImgFmt_UNKNOWN;
    eImgFmt_vip2i = eImgFmt_UNKNOWN;
    eImgFmt_vip3i = eImgFmt_UNKNOWN;
    eImgFmt_ufdi = eImgFmt_UNKNOWN;
    eImgFmt_lcei = eImgFmt_UNKNOWN;
    eImgFmt_dmgi = eImgFmt_UNKNOWN;
    eImgFmt_depi = eImgFmt_UNKNOWN;
    eImgFmt_regi = eImgFmt_UNKNOWN;
    eImgFmt_img2o = eImgFmt_UNKNOWN;
    eImgFmt_img2bo = eImgFmt_UNKNOWN;
    eImgFmt_img3o = eImgFmt_UNKNOWN;
    eImgFmt_img3bo = eImgFmt_UNKNOWN;
    eImgFmt_img3co = eImgFmt_UNKNOWN;
    eImgFmt_feo = eImgFmt_UNKNOWN;
    eImgFmt_timgo = eImgFmt_UNKNOWN;
    eImgFmt_dceso = eImgFmt_UNKNOWN;
    eImgFmt_wroto = eImgFmt_UNKNOWN;
    eImgFmt_wdmao = eImgFmt_UNKNOWN;
    eImgFmt_jpego = eImgFmt_UNKNOWN;
    eImgFmt_venco = eImgFmt_UNKNOWN;
    pTuningDipReg = NULL;
    pMdpCfg = NULL;
    m_imgi_planeNum = 0;

    m_pIspDrvShell = IspDrvShell::createInstance();

    this->m_dipCmdQMgr = DipCmdQMgr::createInstance();
    m_pDipRingBuf = DipRingBuffer::createInstance((DIP_HW_MODULE)DIP_HW_A);
}

PostProcPipe::
~PostProcPipe()
{
    /*** release isp driver ***/
    m_pIspDrvShell->destroyInstance();

    if( this->m_dipCmdQMgr)
    {
        this->m_dipCmdQMgr->destroyInstance();
        this->m_dipCmdQMgr=NULL;
    }

}
/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
init(char const* szCallerName, MUINT32 secTag)
{
    MBOOL ret=MTRUE;
    PIPE_INF("+, userName(%s), szCallerName(%s)",this->m_szUsrName, szCallerName);
    //
    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->init("PostProcPipe", secTag);
        m_pDipRingBuf = m_pIspDrvShell->getDipRingBufMgr();
        ispDipPipe.m_pIspDrvShell = this->m_pIspDrvShell;
    }

    if(m_dipCmdQMgr)
    {
        ret=m_dipCmdQMgr->init();
    }
    if (pPortLogBuf == NULL)
    {
        pPortLogBuf = (char *)malloc(MAX_LOG_SIZE);
    }
    if (pMaxLogBuf == NULL)
    {
        pMaxLogBuf = (char *)malloc(MAX_LOG_SIZE);
    }
    m_bSmartTileEn = ::property_get_int32("vendor.camera.smartile.enable", 1);
    m_bMix4En = ::property_get_int32("vendor.camera.mixingmodeusemix4.enable", 0);

    m_dipConfigPipeLog = ::property_get_int32("vendor.camera.dipConfigPipeLog.enable", 1);
    m_mtkcamLogLevel = ::property_get_int32("persist.vendor.mtk.camera.log_level", 0);
    m_mmpathEn = ::property_get_int32("vendor.camera.mmpath.enable", 0);
    //
    PIPE_DBG("-");

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
uninit(char const* szCallerName)
{
    PIPE_INF("+, userName(%s), szCallerName(%s)",this->m_szUsrName, szCallerName);
    MBOOL ret=MTRUE;
    //
    if (pPortLogBuf != NULL)
    {
        free((void *)pPortLogBuf);
        pPortLogBuf = NULL;
    }
    if (pMaxLogBuf != NULL)
    {
        free((void *)pMaxLogBuf);
        pMaxLogBuf = NULL;
    }

    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->uninit("PostProcPipe");
    }
    //
    if(m_dipCmdQMgr)
    {
        m_dipCmdQMgr->uninit();
    }
    PIPE_DBG("-");

    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
start(MINT32 const p2CQ, MUINT32 const RingBufIdx, MINT32 const burstQIdx, MUINT32 const frameNum)
{
    int ret = MTRUE;

    ret= ispDipPipe.startMDP(p2CQ, RingBufIdx, burstQIdx, frameNum);


    if(ret<0)
    {
    	PIPE_ERR("start ERR");
    	return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
stop(void* pParam)
{
    (void)pParam;
    PIPE_WRN("+ we do not need to call PostProc stop");

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
//getCropFunctionEnable(EDrvScenario drvScen, MINT32 portID, MUINT32& CropGroup)
getCropFunctionEnable(MUINT32 StreamTag, MINT32 portID, MUINT32& CropGroup)
{
	MUINT32 idx,j;
//	MUINT32 numPortId;
//	ESoftwareScenario scenTmp = swScen;
	//if (eScenarioID_NUM <= drvScen)
	if (NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_total <= StreamTag)
	{
		PIPE_ERR("getCropFunctionEnable StreamTag error:(%d)",StreamTag);
		return MTRUE;
	}

	PIPE_DBG("cropPath Array Size = (%lu), Scen_Map_CropPathInfo_STRUCT Size =(%lu), StreamTag:(%d)", (unsigned long)sizeof(mCropPathInfo), (unsigned long)sizeof(Scen_Map_CropPathInfo_STRUCT), StreamTag);
	for (idx = 0; idx<((sizeof(mCropPathInfo))/(sizeof(Scen_Map_CropPathInfo_STRUCT))); idx++)
	{
		//if (drvScen == mCropPathInfo[idx].u4DrvScenId)
		if (StreamTag == mCropPathInfo[idx].u4DrvScenId)
		{
//			numPortId = mCropPathInfo[idx].u4NumPortId;
			CropGroup = mCropPathInfo[idx].u4CropGroup;

			for (j = 0 ; j < DMA_OUT_PORT_NUM; j++)
			{
			    if (portID == mCropPathInfo[idx].u4PortID[j])
			    {
			    	return MTRUE;
			    }
			}
		}
	}
	return MFALSE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
getCrzPosition(
    MUINT32 drvScen,
    MBOOL &isSl2eAheadCrz,
    MBOOL &isLceAheadCrz
)
{
    MBOOL ret = MTRUE;

    switch(drvScen){
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Bld:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_DeNoise:
            isSl2eAheadCrz = MTRUE;
            isLceAheadCrz = MTRUE;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Bokeh:
            isSl2eAheadCrz = MFALSE;
            isLceAheadCrz = MFALSE;
            break;
        default:
            PIPE_ERR("[Error]Not support this drvScen(%d)",drvScen);
            ret = MFALSE;
            break;
    };
    PIPE_DBG("drvScen(%d),isSl2eAheadCrz(%d),isLceAheadCrz(%d)",drvScen,isSl2eAheadCrz,isLceAheadCrz);

    return ret;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
queryCropPathNum(MUINT32 drvScen, MUINT32& pathNum)
{
    MUINT32 size = sizeof(mCropPathInfo)/sizeof(Scen_Map_CropPathInfo_STRUCT);
    MUINT32 i, num;

    num = 0;
    for(i=0;i<size;i++){
        if(mCropPathInfo[i].u4DrvScenId == drvScen)
            num++;
    }
    pathNum = num;

	PIPE_DBG("drvScen(%d),pathNum(%d)", drvScen, pathNum);
	return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
queryScenarioInfo(MUINT32 drvScen, vector<CropPathInfo>& vCropPaths)
{
    //temp
    //ESoftwareScenario swScen=eSoftwareScenario_Main_Normal_Stream;

	MUINT32 idx,j,i;
	MUINT32 portidx = 0;
    MUINT32 u4NumGroup;
	//ESoftwareScenario scenTmp = swScen;
	if (NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_total <= drvScen)
	{
		PIPE_ERR("query scenario info drv scen error:(%d)",drvScen);
		return MTRUE;
	}

	PIPE_DBG("cropPath Array Size = (%lu), Scen_Map_CropPathInfo_STRUCT Size =(%lu), drvScen:(%d)", (unsigned long)sizeof(mCropPathInfo), (unsigned long)sizeof(Scen_Map_CropPathInfo_STRUCT), drvScen);
	for (idx = 0; idx<((sizeof(mCropPathInfo))/(sizeof(Scen_Map_CropPathInfo_STRUCT))); idx++)
	{
		if (drvScen == mCropPathInfo[idx].u4DrvScenId)
		{
			if (portidx >= vCropPaths.size())
			{
				PIPE_ERR("query scenario info portidx error:(%d)",portidx);
				return MFALSE;
			}
            if(vCropPaths[portidx].PortIdxVec.size()>0)
			{
			    vCropPaths[portidx].PortIdxVec.clear();
            }
            u4NumGroup = 0;
            for(i=ECropGroupShiftIdx_1;i<ECropGroupShiftIdx_Num;i++){
               if((1<<i)&mCropPathInfo[idx].u4CropGroup)
                   u4NumGroup++;
            }
			vCropPaths[portidx].u4NumGroup = u4NumGroup;
            vCropPaths[portidx].u4CropGroup = mCropPathInfo[idx].u4CropGroup;
			//pCropPathInfo->PortIdxVec.resize(numPortId);
			for (j = 0 ; j < DMA_OUT_PORT_NUM; j++)
			{
                if(mCropPathInfo[idx].u4PortID[j])
                {
                    vCropPaths[portidx].PortIdxVec.push_back(mCropPathInfo[idx].u4PortID[j]);
                }
                else
                {
                    break;
                }
			}
			//vCropPaths.push_back(pathInfo);
			//vCropPaths.at(portidx) = pCropPathInfo;
			portidx++;
		}
	}
	return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
dequeMdpFrameEnd()
{
    MBOOL ret = MTRUE;
    //
    //if ( 0 != this->ispBufCtrl.dequeueMdpFrameEnd(dequeCq, dequeBurstQIdx,dequeDupCqIdx, moduleIdx) ) {
    if ( 0 != this->ispBufCtrl.dequeueMdpFrameEnd() ) {
        PIPE_ERR("ERROR:dequeMdpFrameEnd");
        ret = MFALSE;
    }

    //
    return  ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
getDipDumpInfo(MUINT32& tdriaddr, MUINT32& cmdqaddr, MUINT32& imgiaddr, MBOOL& bDequeBufIsTheSameAaGCEDump)
{
    MUINTPTR dipwkbuf_cmdqaddr;
    bDequeBufIsTheSameAaGCEDump = MFALSE;
    if ( m_pIspDrvShell ) {
        this->m_pIspDrvShell->m_pDrvDipPhy->getDipDumpInfo(tdriaddr, cmdqaddr, imgiaddr);
        if(m_pDequeDipWorkingBuf != NULL)
        {
            PIPE_INF("tdriaddr(0x%x),cmdqaddr(0x%x),imgiaddr(0x%x),m_pIspDescript_phy(0x%x),tpipeTablePa(0x%x)!!\n", tdriaddr, cmdqaddr, imgiaddr,m_pDequeDipWorkingBuf->m_pIspDescript_phy,m_pDequeDipWorkingBuf->tpipeTablePa);

            dipwkbuf_cmdqaddr = (MUINTPTR)m_pDequeDipWorkingBuf->m_pIspDescript_phy;
            if ((dipwkbuf_cmdqaddr == cmdqaddr) && (m_pDequeDipWorkingBuf->tpipeTablePa == (unsigned long)tdriaddr))
            {
                bDequeBufIsTheSameAaGCEDump = MTRUE;
            }
        }
    }
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
getVssDipWBInfo(MUINTPTR& vsstdriaddr, MUINTPTR& vssispviraddr,MINT32 p2cqIdx, MINT32 p2RingBufIdx)
{
    DipWorkingBuffer* pDipWBuf;

    pDipWBuf = m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)p2cqIdx, p2RingBufIdx, MFALSE);

    if(pDipWBuf==NULL)
    {
        PIPE_ERR("getVssDipWBInfo:: getDipWorkingBufferbyIdx error!!\n");
        return MFALSE;
    }
    vsstdriaddr = (MUINTPTR)pDipWBuf->tpipeTableVa;
    vssispviraddr = (MUINTPTR)pDipWBuf->m_pIspVirRegAddr_va;

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
PostProcPipe::
dequeBuf(MBOOL& isVencContained, MUINTPTR& tpipeVa, MUINTPTR& ispVirRegVa, MINT32 dequeCq, MUINT32 const RingBufIdx)
{
    EIspRetStatus ret;
    DipWorkingBuffer* pDipWBuf;

    PIPE_DBG("dequeCq(%d), RingBufIdx(%d)!!\n", dequeCq, RingBufIdx);
    //Acquire the buffer from ring buffer
    pDipWBuf = m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)dequeCq, RingBufIdx);

    if(pDipWBuf==NULL)
    {
        PIPE_ERR("getDipWorkingBufferbyIdx error!!\n");
        return MFALSE; //error command  enque
    }
    m_pDequeDipWorkingBuf = pDipWBuf;
    tpipeVa = (MUINTPTR)pDipWBuf->tpipeTableVa;
    ispVirRegVa = (MUINTPTR)pDipWBuf->m_pIspVirRegAddr_va;

    ret = this->ispBufCtrl.dequeHwBuf(isVencContained, pDipWBuf);

    if ( ret == eIspRetStatus_Failed ) {
        PIPE_ERR("ERROR:dequeueBuf");
        goto EXIT;
    }

    if (ret == eIspRetStatus_Success){
        if(m_pDipRingBuf->ReleaseDipWorkingBuffer(pDipWBuf, (E_ISP_DIP_CQ)dequeCq)==false)
        {
            PIPE_ERR("ReleaseDipWorkingBuffer error!!\n");
            return MFALSE;
        }
    }
    //
EXIT:
    return  ret;
}

MINT32
PostProcPipe::configMFBTPipe(NSCam::NSIoPipe::MFBConfig* pMfbConfig, SrzCfg pSrz_cfg, MFB_DRV_DMA_EN_STRUCT mfb_dma_en)
{
    PIPE_DBG("configMFBTPipe Start\n");
#if 1
    ISP_TPIPE_CONFIG_STRUCT *pTpipeCfgInfo = &(m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo);

    pTpipeCfgInfo->top.mfb_en = 1;

    pTpipeCfgInfo->top.wif2_en = 1;
    pTpipeCfgInfo->top.wif3_en = 1;

    pTpipeCfgInfo->top.mfbi_en = mfb_dma_en.mfbi_en;
    pTpipeCfgInfo->top.mfbi_b_en = mfb_dma_en.mfbi_b_en;

    pTpipeCfgInfo->top.mfb3i_en = mfb_dma_en.mfb3i_en;
    pTpipeCfgInfo->top.mfb4i_en = mfb_dma_en.mfb4i_en;
    pTpipeCfgInfo->top.mfbo_en = mfb_dma_en.mfbo_en;
    pTpipeCfgInfo->top.mfbo_b_en = mfb_dma_en.mfbo_b_en;
    pTpipeCfgInfo->top.mfb2o_en = mfb_dma_en.mfb2o_en;

    pTpipeCfgInfo->top.mfb_srz_en = pMfbConfig->Mfb_srzEn;

    pTpipeCfgInfo->mfb.bld_mbd_wt_en = (pMfbConfig->Mfb_tuningBuf[0] & 0x00000040) >> 6;

    if (mfb_dma_en.mfbi_en == 1 && pMfbConfig->Mfb_inbuf_baseFrame != 0x0) {
        pTpipeCfgInfo->mfbi.mfbi_stride = pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(0);
        pTpipeCfgInfo->mfbi.mfbi_v_flip_en = 0;
    }

    if (mfb_dma_en.mfbi_b_en == 1 && pMfbConfig->Mfb_inbuf_baseFrame != 0x0) {
        pTpipeCfgInfo->mfbi_b.mfbi_b_stride = pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(1);
        pTpipeCfgInfo->mfbi_b.mfbi_b_v_flip_en = 0;
    }

    //if (mfb_dma_en.mfb2i_en == 1 && pMfbConfig->Mfb_inbuf_refFrame != 0x0) {
    //    pTpipeCfgInfo->mfb2i.mfb2i_stride = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
    //    pTpipeCfgInfo->mfb2i.mfb2i_v_flip_en = 0;
    //}

    //if (mfb_dma_en.mfb2i_b_en == 1 && pMfbConfig->Mfb_inbuf_refFrame != 0x0) {
    //    pTpipeCfgInfo->mfb2bi.mfb2bi_stride = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(1);
    //    pTpipeCfgInfo->mfb2bi.mfb2bi_v_flip_en = 0;
    //}

    if (mfb_dma_en.mfb3i_en == 1 && pMfbConfig->Mfb_inbuf_blendWeight != 0x0) {
        pTpipeCfgInfo->mfb3i.mfb3i_stride = pMfbConfig->Mfb_inbuf_blendWeight->getBufStridesInBytes(0);
        pTpipeCfgInfo->mfb3i.mfb3i_v_flip_en = 0;
    }

    if (mfb_dma_en.mfb4i_en == 1 && pMfbConfig->Mfb_inbuf_confidenceMap != 0x0) {
        pTpipeCfgInfo->mfb4i.mfb4i_stride = pMfbConfig->Mfb_inbuf_confidenceMap->getBufStridesInBytes(0);
        pTpipeCfgInfo->mfb4i.mfb4i_v_flip_en = 0;
        pTpipeCfgInfo->mfb4i.mfb4i_xsize = pMfbConfig->Mfb_inbuf_confidenceMap->getBufStridesInBytes(0) - 1;
        pTpipeCfgInfo->mfb4i.mfb4i_ysize = pMfbConfig->Mfb_inbuf_confidenceMap->getImgSize().h - 1;
    }

    if (mfb_dma_en.mfbo_en == 1 && pMfbConfig->Mfb_outbuf_frame != 0x0) {
        pTpipeCfgInfo->mfbo.mfbo_stride = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(0);
        pTpipeCfgInfo->mfbo.mfbo_xoffset = 0;
        pTpipeCfgInfo->mfbo.mfbo_xsize = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(0) - 1;
        pTpipeCfgInfo->mfbo.mfbo_yoffset = 0;
        pTpipeCfgInfo->mfbo.mfbo_ysize = pMfbConfig->Mfb_outbuf_frame->getImgSize().h - 1;
    }

    if (mfb_dma_en.mfbo_b_en == 1 && pMfbConfig->Mfb_outbuf_frame != 0x0) {
        pTpipeCfgInfo->mfbo_b.mfbo_b_stride = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(1);
    }

    if (mfb_dma_en.mfb2o_en == 1 && pMfbConfig->Mfb_outbuf_blendWeight!= 0x0) {
        pTpipeCfgInfo->mfb2o.mfb2o_stride = pMfbConfig->Mfb_outbuf_blendWeight->getBufStridesInBytes(0);
        pTpipeCfgInfo->mfb2o.mfb2o_xoffset = 0;
        pTpipeCfgInfo->mfb2o.mfb2o_xsize = pMfbConfig->Mfb_outbuf_blendWeight->getBufStridesInBytes(0) - 1;
        pTpipeCfgInfo->mfb2o.mfb2o_yoffset = 0;
        pTpipeCfgInfo->mfb2o.mfb2o_ysize = pMfbConfig->Mfb_outbuf_blendWeight->getImgSize().h - 1;
    }

    pTpipeCfgInfo->mfb_srz.srz_input_crop_width = pSrz_cfg.inout_size.in_w;
    pTpipeCfgInfo->mfb_srz.srz_input_crop_height = pSrz_cfg.inout_size.in_h;
    pTpipeCfgInfo->mfb_srz.srz_output_width = pSrz_cfg.inout_size.out_w;
    pTpipeCfgInfo->mfb_srz.srz_output_height = pSrz_cfg.inout_size.out_h;
    pTpipeCfgInfo->mfb_srz.srz_luma_horizontal_integer_offset = 0;//pSrz_cfg.crop.x;
    pTpipeCfgInfo->mfb_srz.srz_luma_horizontal_subpixel_offset = 0;//pSrz_cfg.crop.floatX;
    pTpipeCfgInfo->mfb_srz.srz_luma_vertical_integer_offset = 0;//pSrz_cfg.crop.y;
    pTpipeCfgInfo->mfb_srz.srz_luma_vertical_subpixel_offset = 0;//pSrz_cfg.crop.floatY;
    pTpipeCfgInfo->mfb_srz.srz_horizontal_coeff_step = pSrz_cfg.h_step;
    pTpipeCfgInfo->mfb_srz.srz_vertical_coeff_step = pSrz_cfg.v_step;

    pTpipeCfgInfo->mfb_crsp.crsp_xoffset = 0x0;
    pTpipeCfgInfo->mfb_crsp.crsp_yoffset = 0x1;
    pTpipeCfgInfo->mfb_crsp.crsp_ystep = 0x4;

    if (pMfbConfig->Mfb_format_bit == NSCam::NSIoPipe::FMT_4202P_10BIT_UNPAK)
    {
        pTpipeCfgInfo->mfb_y_pak.yuv_bit = 1;
        pTpipeCfgInfo->mfb_y_pak.yuv_dng = 1;
        pTpipeCfgInfo->mfb_c_pak.yuv_bit = 1;
        pTpipeCfgInfo->mfb_c_pak.yuv_dng = 1;
        pTpipeCfgInfo->mfb_y_unp.yuv_bit = 1;
        pTpipeCfgInfo->mfb_y_unp.yuv_dng = 1;
        pTpipeCfgInfo->mfb_c_unp.yuv_bit = 1;
        pTpipeCfgInfo->mfb_c_unp.yuv_dng = 1;
    }
    else if (pMfbConfig->Mfb_format_bit == NSCam::NSIoPipe::FMT_4202P_10BIT_PAK)
    {
        pTpipeCfgInfo->mfb_y_pak.yuv_bit = 1;
        pTpipeCfgInfo->mfb_y_pak.yuv_dng = 0;
        pTpipeCfgInfo->mfb_c_pak.yuv_bit = 1;
        pTpipeCfgInfo->mfb_c_pak.yuv_dng = 0;
        pTpipeCfgInfo->mfb_y_unp.yuv_bit = 1;
        pTpipeCfgInfo->mfb_y_unp.yuv_dng = 0;
        pTpipeCfgInfo->mfb_c_unp.yuv_bit = 1;
        pTpipeCfgInfo->mfb_c_unp.yuv_dng = 0;
    }
    else if (pMfbConfig->Mfb_format_bit == NSCam::NSIoPipe::FMT_4202P_8BIT)
    {
        pTpipeCfgInfo->mfb_y_pak.yuv_bit = 0;
        pTpipeCfgInfo->mfb_y_pak.yuv_dng = 0;
        pTpipeCfgInfo->mfb_c_pak.yuv_bit = 0;
        pTpipeCfgInfo->mfb_c_pak.yuv_dng = 0;
        pTpipeCfgInfo->mfb_y_unp.yuv_bit = 0;
        pTpipeCfgInfo->mfb_y_unp.yuv_dng = 0;
        pTpipeCfgInfo->mfb_c_unp.yuv_bit = 0;
        pTpipeCfgInfo->mfb_c_unp.yuv_dng = 0;
    }

    pTpipeCfgInfo->sw.log_en = 0;
    if (pMfbConfig->Mfb_inbuf_baseFrame != 0x0 && pMfbConfig->Mfb_inbuf_refFrame != 0x0) {
        pTpipeCfgInfo->sw.src_width_mfb = pMfbConfig->Mfb_inbuf_baseFrame->getImgSize().w;
        pTpipeCfgInfo->sw.src_height_mfb = pMfbConfig->Mfb_inbuf_refFrame->getBufSizeInBytes(0)
                          / pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
    }
    pTpipeCfgInfo->sw.tpipe_irq_mode = 0; // IRQ - 0: Frame Stop, 2: Tile Stop
    pTpipeCfgInfo->sw.tpipe_width_mfb = 0;
    pTpipeCfgInfo->sw.tpipe_height_mfb = 8192;
    pTpipeCfgInfo->sw.tpipe_sel_mode = 0x100000;

    //PIPE_DBG("tpipe_main_platform start, tdri_PA: 0x%lx, tdri_VA: 0x%lx\n", g_MfbTileBuffer.phyAddr, g_MfbTileBuffer.virtAddr);
    //PIPE_DBG("tpipe_main_platform start, working_buf_VA: 0x%lx, working_buf_size: %d\n", (unsigned long)pWorkingBuffer, tpipeWorkingSize);
    //tpipe_main_platform(&pTpipeCfgInfo, &tpipeDesc, (char *)pWorkingBuffer, tpipeWorkingSize, NULL, NULL, NULL);

    /*
    if (g_isMFBLogEnable) {
        MFB_dumpTPipeCfgInfo(&pTpipeCfgInfo);
        MFB_dumpTPipeBuffer(tpipeDesc.tpipe_config_mfb);
        MFB_dumpTPipeBuffer(tpipeDesc.tpipe_config_mfb + 35 * 1);
        MFB_dumpTPipeBuffer(tpipeDesc.tpipe_config_mfb + 35 * 2);
        MFB_dumpTPipeBuffer(tpipeDesc.tpipe_config_mfb + 35 * 3);
        MFB_dumpTPipeBuffer(tpipeDesc.tpipe_config_mfb + 35 * 4);
        MFB_dumpTPipeBuffer(tpipeDesc.tpipe_config_mfb + 35 * 5);
    }
    */
#endif

    PIPE_DBG("configMFBTPipe End\n");

    return 0;
}

MINT32 MFBDL_Check4Align(unsigned int x)
{
    if ((x & 0x3) == 0) {
        return 0;
    } else {
        return 1;
    }
}

MINT32 MFBDL_Check16Align(unsigned int x)
{
    if ((x & 0xF) == 0) {
        return 0;
    } else {
        return 1;
    }
}

MFBERRCODE MFBDL_ErrorCheck(NSCam::NSIoPipe::MFBConfig* pMfbConfig)
{
    if (pMfbConfig->Mfb_inbuf_baseFrame != 0x0) {
        if (pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(0) == 0x0) {
            PIPE_INF("BaseFrame: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(1) == 0x0) {
            PIPE_INF("BaseFrame: Doesn't have PA(1)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (MFBDL_Check4Align(pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(0)) != 0) {
            PIPE_INF("BaseFrame: Stride is not 4 alignment!");
            return MFBERR_BUFFER_PARAM;
        }
    }
    else {
        PIPE_INF("No base frame!\n");
        return MFBERR_BUFFER_EXIST;
    }

    if (pMfbConfig->Mfb_inbuf_refFrame != 0x0) {
        if (pMfbConfig->Mfb_inbuf_refFrame->getBufPA(0) == 0x0) {
            PIPE_INF("RefFrame: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (pMfbConfig->Mfb_inbuf_refFrame->getBufPA(1) == 0x0) {
            PIPE_INF("RefFrame: Doesn't have PA(1)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (MFBDL_Check16Align(pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0)) != 0) {
            PIPE_INF("RefFrame: Stride is not 16 alignment!");
            return MFBERR_BUFFER_PARAM;
        }
    }
    else {
        PIPE_INF("No ref Frame!\n");
        return MFBERR_BUFFER_EXIST;
    }

    if (pMfbConfig->Mfb_inbuf_blendWeight != 0x0) {
        if (pMfbConfig->Mfb_inbuf_blendWeight->getBufPA(0) == 0x0) {
            PIPE_INF("WeightTable: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        PIPE_INF("No blend weight table\n");
        //bld mode = 0 => no need weight table input
        if (pMfbConfig->Mfb_bldmode != 0) {
            return MFBERR_BUFFER_EXIST;
        }
    }

    if (pMfbConfig->Mfb_inbuf_confidenceMap != 0x0) {
        if (pMfbConfig->Mfb_inbuf_confidenceMap->getBufPA(0) == 0x0) {
            PIPE_INF("ConfidenceMap: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        PIPE_INF("No confidence map\n");
        //srz en = 1 => need confidence map input
        if (pMfbConfig->Mfb_srzEn != 0) {
            return MFBERR_BUFFER_EXIST;
        }
    }

    if (pMfbConfig->Mfb_inbuf_mv != 0x0) {
        if (pMfbConfig->Mfb_inbuf_mv->getBufPA(0) == 0x0) {
            PIPE_INF("Motion: Doesn't have PA(0)!");
            return MFBERR_BUFFER_EXIST;
        }
        if (MFBDL_Check16Align(pMfbConfig->Mfb_inbuf_mv->getBufStridesInBytes(0)) != 0) {
            PIPE_INF("Motion: Stride is not 16 alignment!");
            return MFBERR_BUFFER_PARAM;
        }
    }
    else {
        PIPE_INF("No motion vector\n");
        return MFBERR_BUFFER_EXIST;
    }

// There are no output frame buffer in Direct Link case
#if 0
    if (pMfbConfig->Mfb_outbuf_frame != 0x0) {
        if (pMfbConfig->Mfb_outbuf_frame->getBufPA(0) == 0x0) {
            PIPE_INF("OutputFrame: Doesn't have PA(0)!\n");
            return MFBERR_BUFFER_EXIST;
        }
        if (pMfbConfig->Mfb_outbuf_frame->getBufPA(1) == 0x0) {
            PIPE_INF("OutputFrame: Doesn't have PA(1)!\n");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        PIPE_INF("No output frame!\n");
        return MFBERR_BUFFER_EXIST;
    }

    if (pMfbConfig->Mfb_outbuf_blendWeight != 0x0) {
        if (pMfbConfig->Mfb_outbuf_blendWeight->getBufPA(0) == 0x0) {
            PIPE_INF("OutputWeightTable: Doesn't have PA(0)!\n");
            return MFBERR_BUFFER_EXIST;
        }
    }
    else {
        PIPE_INF("No output blend weight table\n");
        return MFBERR_BUFFER_EXIST;
    }
#endif
    return MFBERR_NOERROR;
}

void
PostProcPipe::configMFB(NSCam::NSIoPipe::MFBConfig *pMfbConfig, MFB_Config *MfbRegConfig)
{
    SrzCfg pSrz_cfg;
    memset(&pSrz_cfg, 0, sizeof(pSrz_cfg));
    MFB_DRV_DMA_EN_STRUCT mfb_dma_en;
    memset(&mfb_dma_en, 0, sizeof(MFB_DRV_DMA_EN_STRUCT));
    unsigned int workaround_height = 0;
    MFBERRCODE ErrCode = MFBERR_NOERROR;

    PIPE_INF("MFB DL DIP Enque, BLD Mode: %d, SRZ En: %d\n", pMfbConfig->Mfb_bldmode, pMfbConfig->Mfb_srzEn);

    ErrCode = MFBDL_ErrorCheck(pMfbConfig);
    if (ErrCode != MFBERR_NOERROR) {
        PIPE_INF("MFBDL Driver Check Error! ERRCDOE (%d)\n", ErrCode);
        abort();
    }

    if (pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0) != 0) {
        workaround_height = pMfbConfig->Mfb_inbuf_refFrame->getBufSizeInBytes(0)
                          / pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
        PIPE_INF("MFB height = %d\n", workaround_height);
    }
    else {
        PIPE_INF("Abnormal RefFrame, BufSize: %d, BufferStrides: %d\n",
            pMfbConfig->Mfb_inbuf_refFrame->getBufSizeInBytes(0), pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0));
    }


    MfbRegConfig->MFBDMA_MFBI_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(0);
    MfbRegConfig->MFBDMA_MFBI_STRIDE = (pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(0));
    MfbRegConfig->MFBDMA_MFBI_YSIZE = workaround_height - 1;
    mfb_dma_en.mfbi_en = 1;

    MfbRegConfig->MFBDMA_MFBI_B_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_baseFrame->getBufPA(1);
    MfbRegConfig->MFBDMA_MFBI_B_STRIDE = (pMfbConfig->Mfb_inbuf_baseFrame->getBufStridesInBytes(1));
    MfbRegConfig->MFBDMA_MFBI_B_YSIZE = workaround_height * 0.5 - 1;
    mfb_dma_en.mfbi_b_en = 1;

    MfbRegConfig->MFBDMA_MFB2I_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(0);
    MfbRegConfig->MFBDMA_MFB2I_STRIDE = (pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0));
    MfbRegConfig->MFBDMA_MFB2I_YSIZE = workaround_height - 1;
    mfb_dma_en.mfb2i_en = 1;

    MfbRegConfig->MFBDMA_MFB2I_B_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(1);
    MfbRegConfig->MFBDMA_MFB2I_B_STRIDE = (pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(1));
    MfbRegConfig->MFBDMA_MFB2I_B_YSIZE = workaround_height * 0.5 - 1;
    mfb_dma_en.mfb2i_b_en = 1;

    if (pMfbConfig->Mfb_bypassOMC != 0) {
        MfbRegConfig->OMC_TOP = ((pMfbConfig->Mfb_format_bit & 0xFF) << 4) | ((pMfbConfig->Mfb_omc_bicub & 0x1) << 3) | 0x1;
    } else {
        MfbRegConfig->OMC_TOP = ((pMfbConfig->Mfb_format_bit & 0xFF) << 4) | ((pMfbConfig->Mfb_omc_bicub & 0x1) << 3);
    }
    MfbRegConfig->OMC_FRAME_SIZE = ((pMfbConfig->Mfb_inbuf_refFrame->getImgSize().w & 0xFFFF) << 16) |
                                   (workaround_height & 0xFFFF);
    MfbRegConfig->OMC_TILE_EDGE = 0xF;
    MfbRegConfig->OMCC_OMC_C_ADDR_GEN_BASE_ADDR_0 = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(0);
    MfbRegConfig->OMCC_OMC_C_ADDR_GEN_STRIDE_0 = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
    MfbRegConfig->OMCC_OMC_C_ADDR_GEN_BASE_ADDR_1 = (unsigned int)pMfbConfig->Mfb_inbuf_refFrame->getBufPA(1);
    MfbRegConfig->OMCC_OMC_C_ADDR_GEN_STRIDE_1 = pMfbConfig->Mfb_inbuf_refFrame->getBufStridesInBytes(1);
    MfbRegConfig->OMCC_OMC_C_CTL_FMT_SEL = 0x10000 | (pMfbConfig->Mfb_format_bit & 0xFF << 8);

    if (pMfbConfig->Mfb_inbuf_blendWeight != 0x0) {
        if (pMfbConfig->Mfb_inbuf_blendWeight->getBufPA(0) != 0x0) {
            MfbRegConfig->MFBDMA_MFB3I_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_blendWeight->getBufPA(0);
            MfbRegConfig->MFBDMA_MFB3I_STRIDE = pMfbConfig->Mfb_inbuf_blendWeight->getBufStridesInBytes(0);
            MfbRegConfig->MFBDMA_MFB3I_YSIZE = workaround_height - 1;
            mfb_dma_en.mfb3i_en = 1;
        } else {
            mfb_dma_en.mfb3i_en = 0;
        }
    }
    else {
    }

    if (pMfbConfig->Mfb_inbuf_confidenceMap != 0x0) {
        if (pMfbConfig->Mfb_inbuf_confidenceMap->getBufPA(0) != 0x0) {
            MfbRegConfig->MFBDMA_MFB4I_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_confidenceMap->getBufPA(0);
            MfbRegConfig->MFBDMA_MFB4I_STRIDE = pMfbConfig->Mfb_inbuf_confidenceMap->getBufStridesInBytes(0);
            MfbRegConfig->MFBDMA_MFB4I_YSIZE = pMfbConfig->Mfb_inbuf_confidenceMap->getImgSize().h - 1;
            mfb_dma_en.mfb4i_en = 1;
        } else {
            mfb_dma_en.mfb4i_en = 0;
        }
    }
    else {
    }

    MfbRegConfig->OMC_MV_RDMA_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_inbuf_mv->getBufPA(0);
    MfbRegConfig->OMC_MV_RDMA_STRIDE = ((pMfbConfig->Mfb_inbuf_refFrame->getImgSize().w >> 4 << 2) + 0xF) & (~0xF);
    //MfbRegConfig->OMC_MV_RDMA_STRIDE = pMfbConfig->Mfb_inbuf_mv->getImgSize().w;

    #if 0
    MfbRegConfig->MFBDMA_MFBO_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_outbuf_frame->getBufPA(0);
    MfbRegConfig->MFBDMA_MFBO_STRIDE = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(0);
    MfbRegConfig->MFBDMA_MFBO_YSIZE = workaround_height - 1;
    mfb_dma_en.mfbo_en = 1;

    MfbRegConfig->MFBDMA_MFBO_B_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_outbuf_frame->getBufPA(1);
    MfbRegConfig->MFBDMA_MFBO_B_STRIDE = pMfbConfig->Mfb_outbuf_frame->getBufStridesInBytes(1);
    MfbRegConfig->MFBDMA_MFBO_B_YSIZE = workaround_height * 0.5 - 1;
    mfb_dma_en.mfbo_b_en = 1;

    MfbRegConfig->MFBDMA_MFB2O_BASE_ADDR = (unsigned int)pMfbConfig->Mfb_outbuf_blendWeight->getBufPA(0);
    MfbRegConfig->MFBDMA_MFB2O_STRIDE = pMfbConfig->Mfb_outbuf_blendWeight->getBufStridesInBytes(0);
    MfbRegConfig->MFBDMA_MFB2O_YSIZE = workaround_height - 1;
    mfb_dma_en.mfb2o_en = 1;
    #endif

#define MFB_TUNABLE
#ifdef MFB_TUNABLE
    PIPE_DBG("MFB Set Tuning Regs!!\n");
    if (pMfbConfig->Mfb_bldmode == 0) {
        MfbRegConfig->MFB_CON = (pMfbConfig->Mfb_tuningBuf[0] & ~(1UL) & 0x0001FFF1) | 0x0;
    } else if (pMfbConfig->Mfb_bldmode == 1) {
        MfbRegConfig->MFB_CON = (pMfbConfig->Mfb_tuningBuf[0] & ~(1UL) & 0x0001FFF1) | 0x1;
    } else if (pMfbConfig->Mfb_bldmode == 2) {
        MfbRegConfig->MFB_CON = (pMfbConfig->Mfb_tuningBuf[0] & ~(1UL) & 0x0001FFF1) | 0x6;
    }
    MfbRegConfig->MFB_LL_CON1 = pMfbConfig->Mfb_tuningBuf[1];
    MfbRegConfig->MFB_LL_CON2 = pMfbConfig->Mfb_tuningBuf[2];
    MfbRegConfig->MFB_EDGE = pMfbConfig->Mfb_tuningBuf[5];
    MfbRegConfig->MFB_LL_CON5 = pMfbConfig->Mfb_tuningBuf[6];
    MfbRegConfig->MFB_LL_CON6 = pMfbConfig->Mfb_tuningBuf[7];
    MfbRegConfig->MFB_LL_CON7 = pMfbConfig->Mfb_tuningBuf[8];
    MfbRegConfig->MFB_LL_CON8 = pMfbConfig->Mfb_tuningBuf[9];
    MfbRegConfig->MFB_LL_CON9 = pMfbConfig->Mfb_tuningBuf[10];
    MfbRegConfig->MFB_LL_CON10 = pMfbConfig->Mfb_tuningBuf[11];
    MfbRegConfig->MFB_MBD_CON0 = pMfbConfig->Mfb_tuningBuf[12];
    MfbRegConfig->MFB_MBD_CON1 = pMfbConfig->Mfb_tuningBuf[13];
    MfbRegConfig->MFB_MBD_CON2 = pMfbConfig->Mfb_tuningBuf[14];
    MfbRegConfig->MFB_MBD_CON3 = pMfbConfig->Mfb_tuningBuf[15];
    MfbRegConfig->MFB_MBD_CON4 = pMfbConfig->Mfb_tuningBuf[16];
    MfbRegConfig->MFB_MBD_CON5 = pMfbConfig->Mfb_tuningBuf[17];
    MfbRegConfig->MFB_MBD_CON6 = pMfbConfig->Mfb_tuningBuf[18];
    MfbRegConfig->MFB_MBD_CON7 = pMfbConfig->Mfb_tuningBuf[19];
    MfbRegConfig->MFB_MBD_CON8 = pMfbConfig->Mfb_tuningBuf[20];
    MfbRegConfig->MFB_MBD_CON9 = pMfbConfig->Mfb_tuningBuf[21];
    MfbRegConfig->MFB_MBD_CON10 = pMfbConfig->Mfb_tuningBuf[22];
    PIPE_DBG("MFB_CON: 0x%08x\n", MfbRegConfig->MFB_CON);
    PIPE_DBG("MFB_LL_CON1: 0x%08x\n", MfbRegConfig->MFB_LL_CON1);
    PIPE_DBG("MFB_LL_CON2: 0x%08x\n", MfbRegConfig->MFB_LL_CON2);
    PIPE_DBG("MFB_EDGE: 0x%08x\n", MfbRegConfig->MFB_EDGE);
    PIPE_DBG("MFB_LL_CON5: 0x%08x\n", MfbRegConfig->MFB_LL_CON5);
    PIPE_DBG("MFB_LL_CON6: 0x%08x\n", MfbRegConfig->MFB_LL_CON6);
    PIPE_DBG("MFB_LL_CON7: 0x%08x\n", MfbRegConfig->MFB_LL_CON7);
    PIPE_DBG("MFB_LL_CON8: 0x%08x\n", MfbRegConfig->MFB_LL_CON8);
    PIPE_DBG("MFB_LL_CON9: 0x%08x\n", MfbRegConfig->MFB_LL_CON9);
    PIPE_DBG("MFB_LL_CON10: 0x%08x\n", MfbRegConfig->MFB_LL_CON10);
    PIPE_DBG("MFB_MBD_CON0: 0x%08x\n", MfbRegConfig->MFB_MBD_CON0);
    PIPE_DBG("MFB_MBD_CON1: 0x%08x\n", MfbRegConfig->MFB_MBD_CON1);
    PIPE_DBG("MFB_MBD_CON2: 0x%08x\n", MfbRegConfig->MFB_MBD_CON2);
    PIPE_DBG("MFB_MBD_CON3: 0x%08x\n", MfbRegConfig->MFB_MBD_CON3);
    PIPE_DBG("MFB_MBD_CON4: 0x%08x\n", MfbRegConfig->MFB_MBD_CON4);
    PIPE_DBG("MFB_MBD_CON5: 0x%08x\n", MfbRegConfig->MFB_MBD_CON5);
    PIPE_DBG("MFB_MBD_CON6: 0x%08x\n", MfbRegConfig->MFB_MBD_CON6);
    PIPE_DBG("MFB_MBD_CON7: 0x%08x\n", MfbRegConfig->MFB_MBD_CON7);
    PIPE_DBG("MFB_MBD_CON8: 0x%08x\n", MfbRegConfig->MFB_MBD_CON8);
    PIPE_DBG("MFB_MBD_CON9: 0x%08x\n", MfbRegConfig->MFB_MBD_CON9);
    PIPE_DBG("MFB_MBD_CON10: 0x%08x\n", MfbRegConfig->MFB_MBD_CON10);
#endif

    if (this->configSrz5(&pSrz_cfg, pMfbConfig->Mfb_srz5Info, workaround_height) != 0)
    {
        PIPE_ERR("MFB SRZ5 CONFIG FAIL\n");
    }
    MfbRegConfig->SRZ_CONTROL = pSrz_cfg.ctrl;
    MfbRegConfig->SRZ_IN_IMG = pSrz_cfg.inout_size.in_w | (pSrz_cfg.inout_size.in_h << 16);
    MfbRegConfig->SRZ_OUT_IMG = pSrz_cfg.inout_size.out_w | (pSrz_cfg.inout_size.out_h << 16);
    MfbRegConfig->SRZ_HORI_STEP = pSrz_cfg.h_step;
    MfbRegConfig->SRZ_VERT_STEP = pSrz_cfg.v_step;
    MfbRegConfig->SRZ_HORI_INT_OFST = 0;//pSrz_cfg.crop.x;
    MfbRegConfig->SRZ_HORI_SUB_OFST = 0;//pSrz_cfg.crop.floatX;
    MfbRegConfig->SRZ_VERT_INT_OFST = 0;//pSrz_cfg.crop.y;
    MfbRegConfig->SRZ_VERT_SUB_OFST = 0;//pSrz_cfg.crop.floatY;
    PIPE_DBG("MFB Set SRZ Done, SRZ Ctrl: 0x%x\n", pSrz_cfg.ctrl);

    // [4:4]: C02 Interpolation Mode is enabled in Verification, [3:0]: C02_TPIPE_EDGE
    MfbRegConfig->C02_CON = 0x1F;

    MfbRegConfig->CRSP_CTRL = 0x00000002; // 422 to 420, fixed configuration
    MfbRegConfig->CRSP_OUT_IMG = (pMfbConfig->Mfb_inbuf_baseFrame->getImgSize().w) | (workaround_height << 16);
    MfbRegConfig->CRSP_STEP_OFST = (0x1 << 24) | (0x4 << 16) | (0x2);

    MfbRegConfig->MFB_MFB_TOP_CFG0 = pMfbConfig->Mfb_srzEn << 2 | 0xB; // Open Direct Link Bit[3]
    MfbRegConfig->MFB_MFB_TOP_CFG2 = mfb_dma_en.mfbi_en
        | mfb_dma_en.mfb3i_en << 2
        | mfb_dma_en.mfb4i_en << 3
        | mfb_dma_en.mfbo_en << 4
        | mfb_dma_en.mfb2o_en << 5
        | mfb_dma_en.mfbi_b_en << 6
        | mfb_dma_en.mfbo_b_en << 8;

    if (pMfbConfig->Mfb_format_bit == NSCam::NSIoPipe::FMT_4202P_10BIT_UNPAK)
    {
        MfbRegConfig->PAK_CONT_Y = 0x000A1110;
        MfbRegConfig->PAK_CONT_C = 0x000A1100;
        MfbRegConfig->UNP_CONT_Y = 0x00010110;
        MfbRegConfig->UNP_CONT_C = 0x00010010;
    }
    else if (pMfbConfig->Mfb_format_bit == NSCam::NSIoPipe::FMT_4202P_10BIT_PAK)
    {
        MfbRegConfig->PAK_CONT_Y = 0x000A0110;
        MfbRegConfig->PAK_CONT_C = 0x000A0100;
        MfbRegConfig->UNP_CONT_Y = 0x00000110;
        MfbRegConfig->UNP_CONT_C = 0x00000010;
    }
    else if (pMfbConfig->Mfb_format_bit == NSCam::NSIoPipe::FMT_4202P_8BIT)
    {
        MfbRegConfig->PAK_CONT_Y = 0x000A0010;
        MfbRegConfig->PAK_CONT_C = 0x000A0000;
        MfbRegConfig->UNP_CONT_Y = 0x00000000;
        MfbRegConfig->UNP_CONT_C = 0x00000000;
    }

    PIPE_DBG("MFB Start Config TPipe\n");
    //MfbRegConfig->MFBDMA_TDRI_BASE_ADDR = pMfbConfig->Mfb_inbuf_tdri->getBufPA(0);
    if (this->configMFBTPipe(pMfbConfig, pSrz_cfg, mfb_dma_en) != 0)
    {
        PIPE_ERR("MFB TPIPE CONFIG FAIL\n");
    }
    //MfbRegConfig->MFBDMA_TDRI_BASE_ADDR = g_MfbTileBuffer.phyAddr;
    MfbRegConfig->MFBDMA_TDRI_XSIZE = 160 - 1;

    //for (int g = 0; g < 100; g++)
    //PIPE_DBG("TDRI BUFFER[%d]:0x%08x 0x%08x 0x%08x 0x%08x\n", g, *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g)), *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g+1)), *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g+2)), *((unsigned int *)(pMfbConfig->Mfb_inbuf_tdri->getBufVA(0) + 4*g+3)));

    MfbRegConfig->MFBDMA_SPECIAL_FUN_EN = 0x000000FF;

    PIPE_INF("MFB_CON: 0x%08x, MFB_TOP_CFG0: 0x%08x, MFB_TOP_CFG2: 0x%08x, PhyAddr: MFBI: 0x%08x, MFBI_B: 0x%08x, MFB2I: 0x%08x, MFB2I_B: 0x%08x, MFB3I: 0x%08x, MFB4I: 0x%08x, MFBO: 0x%08x, MFBO_B: 0x%08x, MFB2O: 0x%08x\n",
        MfbRegConfig->MFB_CON,
        MfbRegConfig->MFB_MFB_TOP_CFG0,
        MfbRegConfig->MFB_MFB_TOP_CFG2,
        MfbRegConfig->MFBDMA_MFBI_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFBI_B_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFB2I_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFB2I_B_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFB3I_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFB4I_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFBO_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFBO_B_BASE_ADDR,
        MfbRegConfig->MFBDMA_MFB2O_BASE_ADDR);

    PIPE_INF("STRIDE/YSIZE: MFBI: (%d,%d), MFBI_B: (%d,%d), MFB2I: (%d,%d), MFB2I_B: (%d,%d), MFB3I: (%d,%d), MFB4I: (%d,%d)\n",
        MfbRegConfig->MFBDMA_MFBI_STRIDE,
        MfbRegConfig->MFBDMA_MFBI_YSIZE,
        MfbRegConfig->MFBDMA_MFBI_B_STRIDE,
        MfbRegConfig->MFBDMA_MFBI_B_YSIZE,
        MfbRegConfig->MFBDMA_MFB2I_STRIDE,
        MfbRegConfig->MFBDMA_MFB2I_YSIZE,
        MfbRegConfig->MFBDMA_MFB2I_B_STRIDE,
        MfbRegConfig->MFBDMA_MFB2I_B_YSIZE,
        MfbRegConfig->MFBDMA_MFB3I_STRIDE,
        MfbRegConfig->MFBDMA_MFB3I_YSIZE,
        MfbRegConfig->MFBDMA_MFB4I_STRIDE,
        MfbRegConfig->MFBDMA_MFB4I_YSIZE);

    //if (g_isMFBLogEnable == 1) {
    //    MfbRegConfig->USERDUMP_EN = 1;
    //} else {
        MfbRegConfig->USERDUMP_EN = 0;
    //}
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
HandleExtraCommand(PipePackageInfo *pPipePackageInfo)
{
    //Parsing Extra Parameter.
    MBOOL ret = MTRUE;
    NSCam::NSIoPipe::ExtraParam CmdInfo;
    android::Vector<NSCam::NSIoPipe::ExtraParam>::const_iterator iter;
    dip_x_reg_t* pTuningIspReg = NULL;
    NSCam::NSIoPipe::FEInfo* pFEInfo;
    NSCam::NSIoPipe::FMInfo* pFMInfo;
    NSCam::NSIoPipe::PQParam* pPQParam;
    //pMdpPipe.WDMAPQParam = NULL;
    //pMdpPipe.WROTPQParam = NULL;
    m_Img3oCropInfo = NULL;
    m_TimgoDumpSel = NSCam::NSIoPipe::EDIPTimgoDump_NONE;
    MDPMGR_CFG_STRUCT *pMdpCfg;
    NSCam::NSIoPipe::MFBConfig* pMfbInfo;
    if(m_pDipWorkingBuf==NULL)
    {
        PIPE_ERR("HandleExtraCommand: State machine is wrong, no DIP working buffer!!\n");
        return MFALSE; //error command  enque
    }
    memset(&m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.top, 0, sizeof(m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.top));
    memset(&m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.sw, 0, sizeof(m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.sw));
    m_isMfbDL = 0;
    //should clear MFB related Tpipe Config

    if(pPipePackageInfo->pExtraParam == NULL)
    {
        PIPE_WRN("empty extraParam");
        return ret;
    }

    for(iter = pPipePackageInfo->pExtraParam->begin();iter<pPipePackageInfo->pExtraParam->end();iter++)
    {
        CmdInfo = (*iter);
        switch (CmdInfo.CmdIdx)
        {
            case NSCam::NSIoPipe::EPIPE_FE_INFO_CMD:
                pFEInfo = (NSCam::NSIoPipe::FEInfo*)CmdInfo.moduleStruct;
                if(pPipePackageInfo->pTuningQue != NULL){
                    pTuningIspReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
                    //Update FE Register
                    pTuningIspReg->DFE_D1A_DFE_FE_CTRL1.Bits.DFE_DSCR_SBIT     = pFEInfo->mFEDSCR_SBIT;
                    pTuningIspReg->DFE_D1A_DFE_FE_CTRL1.Bits.DFE_TH_C          = pFEInfo->mFETH_C;
                    pTuningIspReg->DFE_D1A_DFE_FE_CTRL1.Bits.DFE_TH_G          = pFEInfo->mFETH_G;
                    pTuningIspReg->DFE_D1A_DFE_FE_CTRL1.Bits.DFE_FLT_EN        = pFEInfo->mFEFLT_EN;
                    pTuningIspReg->DFE_D1A_DFE_FE_CTRL1.Bits.DFE_PARAM         = pFEInfo->mFEPARAM;
                    pTuningIspReg->DFE_D1A_DFE_FE_CTRL1.Bits.DFE_MODE          = pFEInfo->mFEMODE;
                    pTuningIspReg->DFE_D1A_DFE_FE_IDX_CTRL.Bits.DFE_YIDX       = pFEInfo->mFEYIDX;
                    pTuningIspReg->DFE_D1A_DFE_FE_IDX_CTRL.Bits.DFE_XIDX       = pFEInfo->mFEXIDX;
                    pTuningIspReg->DFE_D1A_DFE_FE_CROP_CTRL1.Bits.DFE_START_X  = pFEInfo->mFESTART_X;
                    pTuningIspReg->DFE_D1A_DFE_FE_CROP_CTRL1.Bits.DFE_START_Y  = pFEInfo->mFESTART_Y;
                    pTuningIspReg->DFE_D1A_DFE_FE_CROP_CTRL2.Bits.DFE_IN_HT    = pFEInfo->mFEIN_HT;
                    pTuningIspReg->DFE_D1A_DFE_FE_CROP_CTRL2.Bits.DFE_IN_WD    = pFEInfo->mFEIN_WD;
                }
                break;
            case NSCam::NSIoPipe::EPIPE_FM_INFO_CMD:
                pFMInfo = (NSCam::NSIoPipe::FMInfo*)CmdInfo.moduleStruct;
                if(pPipePackageInfo->pTuningQue != NULL){
                    pTuningIspReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
                    //Update FM Register
                    pTuningIspReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_FM_D1_EN       = 0x1;
                    pTuningIspReg->FM_D1A_FM_SIZE_CTRL.Bits.FM_HEIGHT       = pFMInfo->mFMHEIGHT;
                    pTuningIspReg->FM_D1A_FM_SIZE_CTRL.Bits.FM_WIDTH        = pFMInfo->mFMWIDTH;
                    pTuningIspReg->FM_D1A_FM_SIZE_CTRL.Bits.FM_DIS_TYPE     = 0;//FM v1.0
                    pTuningIspReg->FM_D1A_FM_SIZE_CTRL.Bits.FM_SR_TYPE      = pFMInfo->mFMSR_TYPE;
                    pTuningIspReg->FM_D1A_FM_SIZE_CTRL.Bits.FM_OFFSET_X     = pFMInfo->mFMOFFSET_X;
                    pTuningIspReg->FM_D1A_FM_SIZE_CTRL.Bits.FM_OFFSET_Y     = pFMInfo->mFMOFFSET_Y;
                    pTuningIspReg->FM_D1A_FM_TH_CON0_CTRL.Bits.FM_RES_TH    = pFMInfo->mFMRES_TH;
                    pTuningIspReg->FM_D1A_FM_TH_CON0_CTRL.Bits.FM_DIS_TH    = pFMInfo->mFMSAD_TH;
                    pTuningIspReg->FM_D1A_FM_TH_CON0_CTRL.Bits.FM_MIN_RATIO = pFMInfo->mFMMIN_RATIO;
                }
                break;
            case NSCam::NSIoPipe::EPIPE_WPE_INFO_CMD:
                break;
            case NSCam::NSIoPipe::EPIPE_MDP_PQPARAM_CMD:
                pPQParam = (NSCam::NSIoPipe::PQParam*)CmdInfo.moduleStruct;
                pMdpCfg = m_pDipWorkingBuf->m_pMdpCfg;
                // set iso value only for MDP PQ function
                pMdpCfg->mdpWDMAPQParam  = (MVOID *)pPQParam->WDMAPQParam;
                pMdpCfg->mdpWROTPQParam   = (MVOID *)pPQParam->WROTPQParam;
                //ispMdpPipe.WDMAPQParam = (MVOID *)pPQParam->WDMAPQParam;
                //ispMdpPipe.WROTPQParam = (MVOID *)pPQParam->WROTPQParam;
                break;
            case NSCam::NSIoPipe::EPIPE_IMG3O_CRSPINFO_CMD:
                m_Img3oCropInfo = (NSCam::NSIoPipe::CrspInfo*)CmdInfo.moduleStruct;
                break;

            case NSCam::NSIoPipe::EPIPE_RAW_HDRTYPE_CMD:
                m_RawHDRType = (*(MUINT32*)CmdInfo.moduleStruct);
                break;
            case NSCam::NSIoPipe::EPIPE_TIMGO_DUMP_SEL_CMD:
                m_TimgoDumpSel = (*(MUINT32*)CmdInfo.moduleStruct);
                break;
            case NSCam::NSIoPipe::EPIPE_MFB_INFO_CMD:
                pMfbInfo = (NSCam::NSIoPipe::MFBConfig*)CmdInfo.moduleStruct;
                this->configMFB(pMfbInfo, (MFB_Config *)&m_MfbRegStruct);
                m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa_mfb = m_pDipWorkingBuf->mfbtpipeTableVa;
                m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa_mfb= m_pDipWorkingBuf->mfbtpipeTablePa;
                m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.drvinfo.mfbcommand = (unsigned int *)&m_MfbRegStruct;
                m_pDipWorkingBuf->m_pMdpCfg->srcVirAddr = 0xffff;
                if (pMfbInfo->Mfb_inbuf_baseFrame != 0x0) {
                    m_pDipWorkingBuf->m_pMdpCfg->srcPhyAddr = pMfbInfo->Mfb_inbuf_baseFrame->getBufPA(0);
                    m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.sw.src_width = pMfbInfo->Mfb_inbuf_baseFrame->getImgSize().w;
                    m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.sw.src_height = pMfbInfo->Mfb_inbuf_refFrame->getBufSizeInBytes(0)
                                                                                 / pMfbInfo->Mfb_inbuf_refFrame->getBufStridesInBytes(0);
                    //m_pDipWorkingBuf->m_pMdpCfg->mdpSrcFmt = pMfbInfo->Mfb_inbuf_baseFrame->getImgFormat();
                }
                m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.top.wif2_en = 1;
                m_pDipWorkingBuf->m_pMdpCfg->ispTpipeCfgInfo.top.wif3_en = 1;
                m_isMfbDL = 1;
                break;
            default:
                PIPE_WRN("receive extra cmd(%d)\n",CmdInfo.CmdIdx);
                break;
        }
    }

    return ret;
}

MBOOL
PostProcPipe::
PrintMMPathInfo(const NSCam::NSIoPipe::FrameParams &pFrameParams)
{
    char mmpathStr[2048]="",mmpathStrOut[1024]="";

    snprintf(mmpathStr,strlen(mmpathStr),"MMPath: hw=P2, pid=%d, ",getpid());
    for (MUINT32 i = 0 ; i < pFrameParams.mvIn.size() ; i++ )
    {
        snprintf(mmpathStr,strlen(mmpathStr),"%sin_%d=0x%x, in_%d_width=%d, in_%d_height=%d, in_%d_fmt=%s, in_%d_bpp=%d, in_%d_hw=%s, ",mmpathStr,i,(unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),i,pFrameParams.mvIn[i].mBuffer->getImgSize().w,i,pFrameParams.mvIn[i].mBuffer->getImgSize().h,i, ColorFormatMapping((NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat())).c_str(),i, ColorBppMapping((NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat())),i,DmaPortMapping(pFrameParams.mvIn[i].mPortID.index).c_str());
    }

    for (MUINT32 i = 0 ; i < pFrameParams.mvOut.size() ; i++ )
    {
        if(i==0) snprintf(mmpathStrOut,strlen(mmpathStrOut),"out_%d=0x%x, out_%d_width=%d, out_%d_height=%d, out_%d_fmt=%s, out_%d_bpp=%d, out_%d_hw=%s",i,(unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),i,pFrameParams.mvOut[i].mBuffer->getImgSize().w,i,pFrameParams.mvOut[i].mBuffer->getImgSize().h,i, ColorFormatMapping((NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat())).c_str(),i, ColorBppMapping((NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat())),i,DmaPortMapping(pFrameParams.mvOut[i].mPortID.index).c_str());
        else     snprintf(mmpathStrOut,strlen(mmpathStrOut),"%s, out_%d=0x%lx, out_%d_width=%d, out_%d_height=%d, out_%d_fmt=%s, out_%d_bpp=%d, out_%d_hw=%s",mmpathStrOut,i,(unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),i,pFrameParams.mvOut[i].mBuffer->getImgSize().w,i,pFrameParams.mvOut[i].mBuffer->getImgSize().h,i, ColorFormatMapping((NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat())).c_str(),i, ColorBppMapping((NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat())),i,DmaPortMapping(pFrameParams.mvOut[i].mPortID.index).c_str());
    }

    strncat(mmpathStr, mmpathStrOut, strlen(mmpathStrOut));
    ATRACE_NAME(mmpathStr);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
PrintPipePortInfo(const NSCam::NSIoPipe::FrameParams &pFrameParams)
{
    MINT32 planeNum=0;
    MINT32 portcnt =0;
    //char *pPortLogBuf = NULL;
    //char *pMaxLogBuf = NULL;
    unsigned int PortLogBufSize = MAX_LOG_SIZE;
    unsigned int MaxLogBufSize = MAX_LOG_SIZE;
    //pPortLogBuf = (char *)malloc(PortLogBufSize);
    //pMaxLogBuf = (char *)malloc(MaxLogBufSize);


    if (pPortLogBuf == NULL)
    {
        PIPE_ERR("pPortLogBuf is NULL!!!\n");
        return MFALSE;
    }
    if (pMaxLogBuf == NULL)
    {
        PIPE_ERR("pMaxLogBuf is NULL!!!\n");
        return MFALSE;
    }

    memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
    memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);

    snprintf(pPortLogBuf, PortLogBufSize,"\n");
    strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
    for (MUINT32 i = 0 ; i < pFrameParams.mvIn.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvIn[i] ) { continue; }
        //
        planeNum = pFrameParams.mvIn[i].mBuffer->getPlaneCount();
        switch (planeNum)
        {
            case 1:
                #if 0
                PIPE_INF("vInPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,0,0),idx(%d),dir(%d),Size(0x%08x,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x)",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvIn[i].mPortID.index,
                                                            pFrameParams.mvIn[i].mPortID.inout,
                                                            pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvIn[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vInPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(%zu,0,0),idx(%d),dir(%d),Size(0x%08zx,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x),secPort(%d)\n",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            pFrameParams.UniqueKey,
                                                            pFrameParams.mSecureFra,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvIn[i].mPortID.index,
                                                            pFrameParams.mvIn[i].mPortID.inout,
                                                            pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvIn[i].mOffsetInBytes,
                                                            pFrameParams.mvIn[i].mSecureTag);
                break;
            case 2:
                #if 0
                PIPE_INF("vInPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,0),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vInPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(%zu,%zu,0),idx(%d),dir(%d),Size(0x%08zx,0x%08zx,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes,
                                                                    pFrameParams.mvIn[i].mSecureTag);
                break;
            case 3:
                #if 0
                PIPE_INF("vInPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%zu,%zu,%zu),idx(%d),dir(%d),Size(0x%08zx,0x%08zx,0x%08zx),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vInPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(%zu,%zu,%zu),idx(%d),dir(%d),Size(0x%08zx,0x%08zx,0x%08zx),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvIn[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvIn[i].mPortID.index,
                                                                    pFrameParams.mvIn[i].mPortID.inout,
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvIn[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvIn[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvIn[i].mOffsetInBytes,
                                                                    pFrameParams.mvIn[i].mSecureTag);
                break;
        }
        strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
        portcnt++;
        if (portcnt == 3)
        {
            PIPE_INF("%s",pMaxLogBuf);
            memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
            memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);
            snprintf(pPortLogBuf, PortLogBufSize,"\n");
            strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
            portcnt = 0;
        }
        //
    }
    if (portcnt > 0)
    {
        PIPE_INF("%s",pMaxLogBuf);
        portcnt = 0;
    }
    memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
    memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);
    snprintf(pPortLogBuf, PortLogBufSize,"\n");
    strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
    portcnt = 0;

    for (MUINT32 i = 0 ; i < pFrameParams.mvOut.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvOut[i] ) { continue; }
        //
        planeNum = pFrameParams.mvOut[i].mBuffer->getPlaneCount();
        switch (planeNum)
        {
            case 1:
                #if 0
                PIPE_INF("vOutPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%d,0,0),idx(%d),dir(%d),Size(0x%08x,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x)",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvOut[i].mPortID.index,
                                                            pFrameParams.mvOut[i].mPortID.inout,
                                                            pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvOut[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vOutPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(%zu,0,0),idx(%d),dir(%d),Size(0x%08zx,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0),offset(0x%x),secPort(%d)\n",\
                                                            pFrameParams.FrameNo,
                                                            pFrameParams.RequestNo,
                                                            pFrameParams.Timestamp,
                                                            pFrameParams.UniqueKey,
                                                            pFrameParams.mSecureFra,
                                                            i,
                                                            (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                            pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                            pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            pFrameParams.mvOut[i].mPortID.index,
                                                            pFrameParams.mvOut[i].mPortID.inout,
                                                            pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                            (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                            pFrameParams.mvOut[i].mOffsetInBytes,
                                                            pFrameParams.mvOut[i].mSecureTag);
                break;
            case 2:
                #if 0
                PIPE_INF("vOutPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%zu,%zu,0),idx(%d),dir(%d),Size(0x%08zx,0x%08zx,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vOutPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(%zu,%zu,0),idx(%d),dir(%d),Size(0x%08zx,0x%08zx,0x0),VA(0x%lx,0x%lx,0x0),PA(0x%lx,0x%lx,0x0),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes,
                                                                    pFrameParams.mvOut[i].mSecureTag);
                break;
            case 3:
                #if 0
                PIPE_INF("vOutPorts:FN(%d),RN(%d),TS(0x%x),[%d]:(0x%x),w(%d),h(%d),stride(%zu,%zu,%zu),idx(%d),dir(%d),Size(0x%08zx,0x%08zx,0x%08zx),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x)",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes);
                #endif
                snprintf(pPortLogBuf, PortLogBufSize, "vOutPorts:FN(%d),RN(%d),TS(0x%x),UK(0x%x),Sec(%d),[%d]:(0x%x),w(%d),h(%d),stride(%zu,%zu,%zu),idx(%d),dir(%d),Size(0x%08zx,0x%08zx,0x%08zx),VA(0x%lx,0x%lx,0x%lx),PA(0x%lx,0x%lx,0x%lx),offset(0x%x),secPort(%d)\n",\
                                                                    pFrameParams.FrameNo,
                                                                    pFrameParams.RequestNo,
                                                                    pFrameParams.Timestamp,
                                                                    pFrameParams.UniqueKey,
                                                                    pFrameParams.mSecureFra,
                                                                    i,
                                                                    (NSCam::EImageFormat)(pFrameParams.mvOut[i].mBuffer->getImgFormat()),
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().w,
                                                                    pFrameParams.mvOut[i].mBuffer->getImgSize().h,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE),
                                                                    pFrameParams.mvOut[i].mPortID.index,
                                                                    pFrameParams.mvOut[i].mPortID.inout,
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(1),
                                                                    pFrameParams.mvOut[i].mBuffer->getBufSizeInBytes(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufVA(2),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(0),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(1),
                                                                    (unsigned long)pFrameParams.mvOut[i].mBuffer->getBufPA(2),
                                                                    pFrameParams.mvOut[i].mOffsetInBytes,
                                                                    pFrameParams.mvOut[i].mSecureTag);
                break;
        }
        strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
        portcnt++;
        if (portcnt == 3)
        {
            PIPE_INF("%s",pMaxLogBuf);
            memset((MUINT8*)pPortLogBuf,0x0,PortLogBufSize);
            memset((MUINT8*)pMaxLogBuf,0x0,MaxLogBufSize);
            snprintf(pPortLogBuf, PortLogBufSize,"\n");
            strncat(pMaxLogBuf, pPortLogBuf, strlen(pPortLogBuf));
            portcnt = 0;
        }

    }
    if (portcnt > 0)
    {
        PIPE_INF("%s",pMaxLogBuf);
        portcnt = 0;
    }

//	    free((void *)pPortLogBuf);
//	    free((void *)pMaxLogBuf);

    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
DipRawPxlID
PostProcPipe::PixelIDMapping(MUINT32 pixIdP2)
{
    DipRawPxlID p2hwpixelId = DipRawPxlID_B;

    switch (pixIdP2)
    {
        case SENSOR_FORMAT_ORDER_RAW_B:
        {
         p2hwpixelId = DipRawPxlID_B;
         break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gb:
        {
         p2hwpixelId = DipRawPxlID_Gb;
         break;
        }
        case SENSOR_FORMAT_ORDER_RAW_Gr:
        {
         p2hwpixelId = DipRawPxlID_Gr;
         break;
        }
        case SENSOR_FORMAT_ORDER_RAW_R:
        {
         p2hwpixelId = DipRawPxlID_R;
         break;
        }
        default:
        {
         break;
        }
    }
    return p2hwpixelId;
}

MUINT32
PostProcPipe::
VirPortIdxMapToHWPortIdx(MUINT32 index)
{
    switch (index)
    {
        case VirDIPPortIdx_YNR_FACEI:
            return EPortIndex_DMGI;
        case VirDIPPortIdx_YNR_LCEI:
            return EPortIndex_DEPI;
        case VirDIPPortIdx_LPCNR_YUVI:
            return EPortIndex_DEPI;
        case VirDIPPortIdx_LPCNR_YUVO:
            return EPortIndex_TIMGO;
        case VirDIPPortIdx_LPCNR_UVI:
            return EPortIndex_LCEI;
        case VirDIPPortIdx_LPCNR_UVO:
            return EPortIndex_TIMGO;
        case VirDIPPortIdx_GOLDENFRMI:
            return EPortIndex_VIPI;
        case VirDIPPortIdx_WEIGHTMAPI:
            return EPortIndex_DMGI;
        case VirDIPPortIdx_CNR_BLURMAPI:
            return EPortIndex_DMGI;
        case VirDIPPortIdx_LFEOI:
            return EPortIndex_DMGI;
        case VirDIPPortIdx_RFEOI:
            return EPortIndex_DEPI;
        case VirDIPPortIdx_FMO:
            return EPortIndex_TIMGO;
        case EPortIndex_LSCI:
            return EPortIndex_IMGCI;
        case EPortIndex_BPCI:
            return EPortIndex_IMGBI;
        case EPortIndex_IMGI:
        case EPortIndex_IMGBI:
        case EPortIndex_IMGCI:
        case EPortIndex_VIPI:
        case EPortIndex_VIP2I:
        case EPortIndex_VIP3I:
        case EPortIndex_LCEI:
        case EPortIndex_IMG2O:
        case EPortIndex_IMG2BO:
        case EPortIndex_IMG3O:
        case EPortIndex_IMG3BO:
        case EPortIndex_IMG3CO:
        case EPortIndex_FEO:
        case EPortIndex_WROTO:
        case EPortIndex_WDMAO:
        case EPortIndex_JPEGO:
        case EPortIndex_VENC_STREAMO:
        case EPortIndex_TIMGO:
        case EPortIndex_DCESO:
            return index;
    }
    return 0xffffffff;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
checkFrameParamIsTheSame(const NSCam::NSIoPipe::FrameParams &pCurrFrameParams,const NSCam::NSIoPipe::FrameParams &pPrevFrameParams)
{
    int planeNum=0;
    MBOOL bFoundDMA;

    for(MUINT32 i=0;i< pCurrFrameParams.mvIn.size() ;i++)
    {
        bFoundDMA = MFALSE;
        for(MUINT32 j=0;j< pPrevFrameParams.mvIn.size() ;j++)
        {
            if(pCurrFrameParams.mvIn[i].mPortID.index == pPrevFrameParams.mvIn[j].mPortID.index)
            {
                bFoundDMA = MTRUE;
                if ( (pCurrFrameParams.mvIn[i].mBuffer->getImgSize().w == pPrevFrameParams.mvIn[i].mBuffer->getImgSize().w) &&
                     (pCurrFrameParams.mvIn[i].mBuffer->getImgSize().h == pPrevFrameParams.mvIn[i].mBuffer->getImgSize().h) &&
                     (pCurrFrameParams.mvIn[i].mBuffer->getImgFormat() == pPrevFrameParams.mvIn[i].mBuffer->getImgFormat()) &&
                     (pCurrFrameParams.mvIn[i].mBuffer->getPlaneCount() == pPrevFrameParams.mvIn[i].mBuffer->getPlaneCount()) &&
                     (pCurrFrameParams.mvIn[i].mPortID.capbility == pPrevFrameParams.mvIn[i].mPortID.capbility))
                {
                    planeNum = pCurrFrameParams.mvIn[i].mBuffer->getPlaneCount();
                    for(int k=0;k<planeNum;k++)
                    {
                        if ((pCurrFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(k) != pPrevFrameParams.mvIn[i].mBuffer->getBufStridesInBytes(k)))
                            return MFALSE;
                    }
                }
                else
                {
                    return MFALSE;
                }
            }
        }
        if (bFoundDMA == MFALSE)
        {
            return MFALSE;
        }
    }

    for(MUINT32 i=0;i< pCurrFrameParams.mvOut.size() ;i++)
    {
        bFoundDMA = MFALSE;
        for(MUINT32 j=0;j< pPrevFrameParams.mvOut.size() ;j++)
        {
            if(pCurrFrameParams.mvOut[i].mPortID.index == pPrevFrameParams.mvOut[j].mPortID.index)
            {
                bFoundDMA = MTRUE;
                if ( (pCurrFrameParams.mvOut[i].mBuffer->getImgSize().w == pPrevFrameParams.mvOut[i].mBuffer->getImgSize().w) &&
                     (pCurrFrameParams.mvOut[i].mBuffer->getImgSize().h == pPrevFrameParams.mvOut[i].mBuffer->getImgSize().h) &&
                     (pCurrFrameParams.mvOut[i].mBuffer->getImgFormat() == pPrevFrameParams.mvOut[i].mBuffer->getImgFormat()) &&
                     (pCurrFrameParams.mvOut[i].mBuffer->getPlaneCount() == pPrevFrameParams.mvOut[i].mBuffer->getPlaneCount()) &&
                     (pCurrFrameParams.mvOut[i].mPortID.capbility == pPrevFrameParams.mvOut[i].mPortID.capbility))
                {
                    planeNum = pCurrFrameParams.mvOut[i].mBuffer->getPlaneCount();
                    for(int k=0;k<planeNum;k++)
                    {
                        if ((pCurrFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(k) != pPrevFrameParams.mvOut[i].mBuffer->getBufStridesInBytes(k)))
                            return MFALSE;
                    }
                }
                else
                {
                    return MFALSE;
                }
            }
        }
        if (bFoundDMA == MFALSE)
        {
            return MFALSE;
        }

    }
    return MTRUE;
}


MUINT32
PostProcPipe::
getTuningModuleConstrainErrCode()
{
    if (pTuningDipReg != NULL)
    {
        if ((pTuningDipReg->BPC_D1A_BPC_BPC_CON.Bits.BPC_BPC_EN == 1) && (pTuningDipReg->BPC_D1A_BPC_BPC_CON.Bits.BPC_BPC_LUT_EN == 1)){
            if (portInfo_imgbi == NULL){
                PIPE_ERR("BPC_BPC_EN and BPC_BPC_LUT_EN are enabled, but MW don't provide BPCI Buffer to DIP Driver\n");
                return ERROR_LACK_BPCI_BUF;
            }
        }
        if (pTuningDipReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Bits.DIPCTL_LSC_D1_EN == 1){
            if (portInfo_imgci == NULL){
                PIPE_ERR("LSC_D1_EN is enabled, but MW don't provide LSCI Buffer to DIP Driver\n");
                return ERROR_LACK_LSCI_BUF;
            }
        }
        if ((pTuningDipReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_LCE_D1_EN == 1) && (pTuningDipReg->LCE_D1A_LCE_GLOBAL.Bits.LCE_LC_TONE == 1)){
            if (portInfo_lcei == NULL){
                PIPE_ERR("LCE_D1_EN and LCE_LC_TONE are enabled, but MW don't provide LCEI Buffer to DIP Driver\n");
                return ERROR_LACK_LCEI_BUF;
            }
        }

        if (pTuningDipReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_YNR_D1_EN == 1){
            if (pTuningDipReg->YNR_D1A_YNR_CON1.Bits.YNR_LCE_LINK == 1) {
                if (portInfo_depi == NULL){
                    PIPE_ERR("YNR_D1_EN and YNR_LCE_LINK are enabled, but MW don't provide YNR_LCEI Buffer to DIP Driver\n");
                    return ERROR_LACK_YNR_LCEI_BUF;
                }
                if (ispDipPipe.isp_top_ctl.YUV2_EN.Bits.DIPCTL_SRZ_D4_EN == 0){
                    PIPE_ERR("YNR_D1_EN and YNR_LCE_LINK are enabled, but MW don't provide SRZ4's setting to DIP Driver\n");
                    return ERROR_LACK_SRZ4_SETTING;
                }
            }
            if (pTuningDipReg->YNR_D1A_YNR_SKIN_CON.Bits.YNR_SKIN_LINK == 1) {
                if (portInfo_dmgi == NULL){
                    PIPE_ERR("YNR_D1_EN and YNR_SKIN_LINK are enabled, but MW don't provide YNR_FACEI Buffer to DIP Driver\n");
                    return ERROR_LACK_YNR_FACEI_BUF;
                }
                if (ispDipPipe.isp_top_ctl.YUV2_EN.Bits.DIPCTL_SRZ_D3_EN == 0){
                    PIPE_ERR("YNR_D1_EN and YNR_LCE_LINK are enabled, but MW don't provide SRZ3's setting to DIP Driver\n");
                    return ERROR_LACK_SRZ3_SETTING;
                }
            }

        }

        if (pTuningDipReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Bits.DIPCTL_CNR_D1_EN == 1){
            if (pTuningDipReg->CNR_D1A_CNR_CNR_CON1.Bits.CNR_MODE == 1) {
                if (portInfo_dmgi == NULL){
                    PIPE_ERR("CNR_D1_EN and CNR_MODE are enabled, but MW don't provide CNR_BLURMAPI  Buffer to DIP Driver\n");
                    return ERROR_LACK_CNR_BLURMAPI_BUF;
                }
                if (ispDipPipe.isp_top_ctl.YUV2_EN.Bits.DIPCTL_SRZ_D3_EN == 0){
                    PIPE_ERR("CNR_D1_EN and CNR_MODE are enabled, but MW don't provide SRZ3's setting to DIP Driver\n");
                    return ERROR_LACK_SRZ3_SETTING;
                }
            }

        }
    }
    //check crop setting, CropInfo_imgi
    if ((portInfo_wroto != NULL) && (pMdpCfg != NULL))
    {
        if (pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropW == 0){
            return ERROR_WROT_SRC_CROP_WIDTH;
        }
        if (pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropH == 0){
            return ERROR_WROT_SRC_CROP_WIDTH;
        }
        if(pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropW>pMdpCfg->mdpSrcW){
            return ERROR_WROT_SRC_CROP_WDITH_BIGGER_THAN_SRC_WIDTH;
        }
        if(pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropH>pMdpCfg->mdpSrcH){
            return ERROR_WROT_SRC_CROP_HEIGHT_BIGGER_THAN_SRC_HEIGHT;
        }
    }
    if ((portInfo_wdmao != NULL) && (pMdpCfg != NULL))
    {
        if (pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropW == 0)
        {
            return ERROR_WDMA_SRC_CROP_WIDTH;
        }
        if (pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropH == 0)
        {
            return ERROR_WDMA_SRC_CROP_HEIGHT;
        }
        if(pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropW>pMdpCfg->mdpSrcW){
            return ERROR_WDMA_SRC_CROP_WDITH_BIGGER_THAN_SRC_WIDTH;
        }
        if(pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropH>pMdpCfg->mdpSrcH){
            return ERROR_WDMA_SRC_CROP_HEIGHT_BIGGER_THAN_SRC_HEIGHT;
        }

    }


    return ERROR_NO_ERROR;
}

MUINT32
PostProcPipe::
getDMAConstrainErrCode(IImageBuffer* pImgBuf)
{
    MINT32 planeNum=0;
    if (pImgBuf != NULL){
        planeNum = pImgBuf->getPlaneCount();
        switch (planeNum)
        {
            case 3:
                if (pImgBuf->getBufVA(ESTRIDE_3RD_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_VIRTUAL_ADDRESS;
                }
                if (pImgBuf->getBufPA(ESTRIDE_3RD_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_PHYSICAL_ADDRESS;
                }
                if (pImgBuf->getBufStridesInBytes(ESTRIDE_3RD_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_STRIDE;
                }
            case 2:
                if (pImgBuf->getBufVA(ESTRIDE_2ND_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_VIRTUAL_ADDRESS;
                }
                if (pImgBuf->getBufPA(ESTRIDE_2ND_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_PHYSICAL_ADDRESS;
                }
                if (pImgBuf->getBufStridesInBytes(ESTRIDE_2ND_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_STRIDE;
                }
            case 1:
                if (pImgBuf->getBufVA(ESTRIDE_1ST_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_VIRTUAL_ADDRESS;
                }
                if (pImgBuf->getBufPA(ESTRIDE_1ST_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_PHYSICAL_ADDRESS;
                }
                if (pImgBuf->getBufStridesInBytes(ESTRIDE_1ST_PLANE) == 0)
                {
                    return ERROR_DMA_INVALID_STRIDE;
                }
                break;
        }
        if (pImgBuf->getImgSize().w == 0)
        {
            return ERROR_DMA_INVALID_WIDTH;
        }
        if (pImgBuf->getImgSize().h == 0)
        {
            return ERROR_DMA_INVALID_HEIGHT;
        }
    }

    return ERROR_NO_ERROR;
}

MUINT32
PostProcPipe::
checkDMAConstrain(IImageBuffer* pImgBuf, MUINT32 index, char* pAssertBuf)
{
    std::string aeestr = "";
    std::string dmastr = "";

    MUINT32 ErrCode = getDMAConstrainErrCode(pImgBuf);
    switch (ErrCode)
    {
        case ERROR_DMA_INVALID_PHYSICAL_ADDRESS:
            {
                aeestr = "CRDISPATCH_KEY:Physical Address of DMA ";
            }
            break;
        case ERROR_DMA_INVALID_VIRTUAL_ADDRESS:
            {
                aeestr = "CRDISPATCH_KEY:Virtual Address of DMA ";
            }
            break;
        case ERROR_DMA_INVALID_WIDTH:
            {
                aeestr = "CRDISPATCH_KEY:Width of DMA ";
            }
            break;
        case ERROR_DMA_INVALID_HEIGHT:
            {
                aeestr = "CRDISPATCH_KEY:Height of DMA ";
            }
            break;
        case ERROR_DMA_INVALID_STRIDE:
            {
                aeestr = "CRDISPATCH_KEY:Stride of DMA ";
            }
            break;

        case ERROR_NO_ERROR:
        default:
            aeestr = "";
            break;
    }

    switch (index)
    {
        case VirDIPPortIdx_YNR_FACEI:
            dmastr="YNR_FACEI";
            break;
        case VirDIPPortIdx_YNR_LCEI:
            dmastr="YNR_LCEI";
            break;
        case VirDIPPortIdx_LPCNR_YUVI:
            dmastr="LPCNR_YUVI";
            break;
        case VirDIPPortIdx_LPCNR_YUVO:
            dmastr="LPCNR_YUVO";
            break;
        case VirDIPPortIdx_LPCNR_UVI:
            dmastr="LPCNR_UVI";
            break;
        case VirDIPPortIdx_LPCNR_UVO:
            dmastr="LPCNR_UVO";
            break;
        case VirDIPPortIdx_GOLDENFRMI:
            dmastr="GOLDENFRMI";
            break;
        case VirDIPPortIdx_WEIGHTMAPI:
            dmastr="WEIGHTMAPI";
            break;
        case VirDIPPortIdx_CNR_BLURMAPI:
            dmastr="CNR_BLURMAPI";
            break;
        case VirDIPPortIdx_LFEOI:
            dmastr="LFEOI";
            break;
        case VirDIPPortIdx_RFEOI:
            dmastr="RFEOI";
            break;
        case VirDIPPortIdx_FMO:
            dmastr="FMO";
            break;
        case EPortIndex_LSCI:
            dmastr="LSCI";
            break;
        case EPortIndex_BPCI:
            dmastr="BPCI";
            break;
        case EPortIndex_IMGI:
            dmastr="IMGI";
            break;
        case EPortIndex_UFDI:
            dmastr="UFDI";
            break;
        case EPortIndex_VIPI:
            dmastr="VIPI";
            break;
        case EPortIndex_LCEI:
            dmastr="LCEI";
            break;
        case EPortIndex_IMG2O:
            dmastr="IMG2O";
            break;
        case EPortIndex_IMG3O:
            dmastr="IMG3O";
            break;
        case EPortIndex_FEO:
            dmastr="FEO";
            break;
        case EPortIndex_WROTO:
            dmastr="WROTO";
            break;
        case EPortIndex_WDMAO:
            dmastr="WDMAO";
            break;
        case EPortIndex_TIMGO:
            dmastr="TIMGO";
            break;
        case EPortIndex_DCESO:
            dmastr="DCESO";
            break;
        default:
            dmastr="UnKnown DMA Idx";
            break;

    }
    aeestr+= dmastr;
    aeestr+= " is InValid !!\n";

    if (ErrCode != ERROR_NO_ERROR)
    {
        memset((char*)pAssertBuf, 0x0, MAX_LOGBUF_LEN);
        pAssertBuf[MAX_LOGBUF_LEN-1]='\0';
        snprintf(pAssertBuf,MAX_LOGBUF_LEN,"%s, ErrorCode:%d",aeestr.c_str(), ErrCode);
        PIPE_ERR("%s, ErrorCode:%d", aeestr.c_str(), ErrCode);
        PIPE_ERR("%s", pAssertBuf);
        AEE_ASSERT(pAssertBuf);
        return ErrCode;
    }


    return ErrCode;
}
MUINT32
PostProcPipe::
checkTuningModuleConstrain(char* pAssertBuf)
{
    std::string str = "";
    MUINT32 ErrCode = ERROR_NO_ERROR;

    ErrCode = getTuningModuleConstrainErrCode();
    switch (ErrCode)
    {
        case ERROR_LACK_BPCI_BUF:
            {
                str = "CRDISPATCH_KEY:Lack BPCI Buffer!!";
            }
            break;
        case ERROR_LACK_LSCI_BUF:
            {
                str = "CRDISPATCH_KEY:Lack LSCI Buffer!!";
            }
            break;
        case ERROR_LACK_LCEI_BUF:
            {
                str = "CRDISPATCH_KEY:Lack LCEI Buffer!!";
            }
            break;
        case ERROR_LACK_YNR_LCEI_BUF:
            {
                str = "CRDISPATCH_KEY:Lack YNR_LCEI Buffer!!";
            }
            break;
        case ERROR_LACK_YNR_FACEI_BUF:
            {
                str = "CRDISPATCH_KEY:Lack YNR_FACEI Buffer!!";
            }
            break;
        case ERROR_LACK_CNR_BLURMAPI_BUF:
            {
                str = "CRDISPATCH_KEY:Lack BLURMAPI Buffer!!";
            }
            break;

        case ERROR_WROT_SRC_CROP_WIDTH:
            {
                str = "CRDISPATCH_KEY:WROT SRC CROP WIDTH is invalid!!";
            }
            break;

        case ERROR_WROT_SRC_CROP_HEIGHT:
            {
                str = "CRDISPATCH_KEY:WROT SRC CROP HEIGHT is invalid!!";
            }
            break;

        case ERROR_WROT_SRC_CROP_WDITH_BIGGER_THAN_SRC_WIDTH:
            {
                str = "CRDISPATCH_KEY:WROT SRC CROP WIDTH Bigger Than SRC WIDTH!!";
            }
            break;

        case ERROR_WROT_SRC_CROP_HEIGHT_BIGGER_THAN_SRC_HEIGHT:
            {
                str = "CRDISPATCH_KEY:WROT SRC CROP HEIGHT Bigger Than SRC HEIGHT!!";
            }
            break;

        case ERROR_WDMA_SRC_CROP_WIDTH:
            {
                str = "CRDISPATCH_KEY:WDMA SRC CROP WIDTH is invalid!!";
            }
            break;

        case ERROR_WDMA_SRC_CROP_HEIGHT:
            {
                str = "CRDISPATCH_KEY:WDMA SRC CROP HEIGHT is invalid!!";
            }
            break;

        case ERROR_WDMA_SRC_CROP_WDITH_BIGGER_THAN_SRC_WIDTH:
            {
                str = "CRDISPATCH_KEY:WDMA SRC CROP WIDTH Bigger Than SRC WIDTH!!";
            }
            break;

        case ERROR_WDMA_SRC_CROP_HEIGHT_BIGGER_THAN_SRC_HEIGHT:
            {
                str = "CRDISPATCH_KEY:WDMA SRC CROP HEIGHT Bigger Than SRC HEIGHT!!";
            }
            break;

        case ERROR_NO_ERROR:
        default:
            str = "";
            break;
    }


    if (ErrCode != ERROR_NO_ERROR)
    {
        memset((char*)pAssertBuf, 0x0, MAX_LOGBUF_LEN);
        pAssertBuf[MAX_LOGBUF_LEN-1]='\0';
        snprintf(pAssertBuf,MAX_LOGBUF_LEN,"%s, ErrorCode:%d",str.c_str(), ErrCode);
        PIPE_ERR("%s, ErrorCode:%d", str.c_str(), ErrCode);
        PIPE_ERR("%s", pAssertBuf);
        AEE_ASSERT(pAssertBuf);
        return ErrCode;
    }
    return ErrCode;
}

MUINT32
PostProcPipe::
checkDipHWConstrain()
{
    char szAssertBuf[MAX_LOGBUF_LEN];
    memset((char*)szAssertBuf, 0x0, MAX_LOGBUF_LEN);
    szAssertBuf[strlen(szAssertBuf)]='\0';
    MINT32 FuncRetrun = ERROR_NO_ERROR;

    FuncRetrun = checkTuningModuleConstrain(szAssertBuf);
    if (FuncRetrun != ERROR_NO_ERROR)
    {
        return FuncRetrun;
    }

    if (portInfo_imgi != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_imgi->mBuffer, portInfo_imgi->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }

    if (portInfo_ufdi != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_ufdi->mBuffer, portInfo_ufdi->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }

    if (m_imgi_planeNum == 1) {
        if (portInfo_imgbi != NULL){
            FuncRetrun = checkDMAConstrain(portInfo_imgbi->mBuffer, portInfo_imgbi->mPortID.index, szAssertBuf);
            if (FuncRetrun != ERROR_NO_ERROR)
            {
                return FuncRetrun;
            }
        }
        if (portInfo_imgci != NULL){
            FuncRetrun = checkDMAConstrain(portInfo_imgci->mBuffer, portInfo_imgci->mPortID.index, szAssertBuf);
            if (FuncRetrun != ERROR_NO_ERROR)
            {
                return FuncRetrun;
            }
        }
    }
    if (portInfo_vipi != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_vipi->mBuffer, portInfo_vipi->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_lcei != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_lcei->mBuffer, portInfo_lcei->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_dmgi != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_dmgi->mBuffer, portInfo_dmgi->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_depi != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_depi->mBuffer, portInfo_depi->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_img2o != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_img2o->mBuffer, portInfo_img2o->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_img3o != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_img3o->mBuffer, portInfo_img3o->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_wroto != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_wroto->mBuffer, portInfo_wroto->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_wdmao != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_wdmao->mBuffer, portInfo_wdmao->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_timgo != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_timgo->mBuffer, portInfo_timgo->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_feo != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_feo->mBuffer, portInfo_feo->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }
    if (portInfo_dceso != NULL){
        FuncRetrun = checkDMAConstrain(portInfo_dceso->mBuffer, portInfo_dceso->mPortID.index, szAssertBuf);
        if (FuncRetrun != ERROR_NO_ERROR)
        {
            return FuncRetrun;
        }
    }

    return FuncRetrun;

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configPipe(const NSCam::NSIoPipe::FrameParams &pFrameParams, PipePackageInfo *pPipePackageInfo, char const* szCallerName)
{
    MBOOL ret = MTRUE;

    //judge drvscenaio and p2 cq
    //EDrvScenario drvScen = pPipePackageInfo->drvScen;
    MUINT32 StreamTag = pPipePackageInfo->StreamTag;
    MINT32 p2CQ = pPipePackageInfo->p2cqIdx;
    //MINT32 p2RingBufIdx = pPipePackageInfo->p2RingBufIdx;
    MINT32 burstQIdx = pPipePackageInfo->burstQIdx;
    MBOOL lastFrame = pPipePackageInfo->lastframe;
    MINT32 planeNum = 0;
    m_imgi_planeNum = 0;
    NSCam::NSIoPipe::CrspInfo Img3oCrspTmp;
    MINT32 pixIdImgBuf = 0;
    MINT32 ErrCode = ERROR_NO_ERROR;
    //dip_x_reg_t* pTuningBuf = NULL;
    //MDPMGR_CFG_STRUCT *pMdpCfg;
    pMdpCfg = NULL;

    // TODO: hungwen modify
    if(m_pDipWorkingBuf == NULL)
    {
        PIPE_ERR("State machine is wrong, no DIP working buffer!!\n");
        return MFALSE; //error command  enque
    }
    pMdpCfg = m_pDipWorkingBuf->m_pMdpCfg;
    if(pMdpCfg == NULL)
    {
        PIPE_ERR("pMdpCfg is NULL!!\n");
        return MFALSE; //error command  enque
    }
    pMdpCfg->mdpWDMAPQParam = NULL;
    pMdpCfg->mdpWROTPQParam = NULL;
    pMdpCfg->ExpectedEndTime.tv_sec = pFrameParams.ExpectedEndTime.tv_sec;
    pMdpCfg->ExpectedEndTime.tv_usec = pFrameParams.ExpectedEndTime.tv_usec;

    if (HandleExtraCommand(pPipePackageInfo) == MFALSE)
    {
        PIPE_ERR("parsing frame cmd error!!\n");
        return MFALSE; //error command  enque
    }
    // initialize
    for(MINT32 i = 0; i < ISP_OUTPORT_NUM; i++){
        pMdpCfg->dstPortCfg[i] = 0;
    }

#if 0
    if(dupCQIdx>=MAX_DUP_CQ_NUM || dupCQIdx<0 || burstQIdx>=MAX_BURST_QUE_NUM || burstQIdx<0 ||p2CQ>=ISP_DRV_DIP_CQ_NUM || p2CQ<0 )
    {
        PIPE_ERR("exception drvScen/cq/dup/burst(%d/%d/%d/%d), last(%d)",drvScen, p2CQ, dupCQIdx, burstQIdx, lastFrame);
        return MFALSE;
    }
#endif
    m_CrzEn = MFALSE;
    //////////////////////////////////////////////////
    //variables
    //dma
    int idx_imgi = -1;
    int idx_imgbi = -1;
    int idx_imgci = -1;
    int idx_vipi = -1;
    int idx_vip2i = -1;  //vipbi
    int idx_vip3i = -1;  //vipci
    int idx_ufdi = -1;
    int idx_lcei = -1;
    int idx_dmgi = -1;
    int idx_depi = -1;
    int idx_regi = -1;
    int idx_img2o = -1;  //crzo
    int idx_img2bo = -1; //crzbo
    int idx_img3o = -1;
    int idx_img3bo = -1;
    int idx_img3co = -1;
    int idx_timgo = -1;  //mfbo, pak2o
    int idx_feo = -1;
    int idx_dceso = -1;
    int idx_wroto = -1;
    int idx_wdmao = -1;
    int idx_jpego = -1;
    int idx_venco = -1;

    portInfo_imgi = NULL;
    portInfo_imgbi = NULL;
    portInfo_imgci = NULL;
    portInfo_vipi = NULL;
    portInfo_vip2i = NULL;
    portInfo_vip3i = NULL;
    portInfo_ufdi = NULL;
    portInfo_lcei = NULL;
    portInfo_dmgi = NULL;
    portInfo_depi = NULL;
    portInfo_regi = NULL;
    portInfo_img2o = NULL;
    portInfo_img2bo = NULL;
    portInfo_img3o = NULL;
    portInfo_img3bo = NULL;
    portInfo_img3co = NULL;
    portInfo_feo = NULL;
    portInfo_timgo = NULL;
    portInfo_dceso = NULL;
    portInfo_wroto = NULL;
    portInfo_wdmao = NULL;
    portInfo_jpego = NULL;
    portInfo_venco = NULL;

    eImgFmt_imgi = eImgFmt_UNKNOWN;
    eImgFmt_imgbi = eImgFmt_UNKNOWN;
    eImgFmt_imgci = eImgFmt_UNKNOWN;
    eImgFmt_vipi = eImgFmt_UNKNOWN;
    eImgFmt_vip2i = eImgFmt_UNKNOWN;
    eImgFmt_vip3i = eImgFmt_UNKNOWN;
    eImgFmt_ufdi = eImgFmt_UNKNOWN;
    eImgFmt_lcei = eImgFmt_UNKNOWN;
    eImgFmt_dmgi = eImgFmt_UNKNOWN;
    eImgFmt_depi = eImgFmt_UNKNOWN;
    eImgFmt_regi = eImgFmt_UNKNOWN;
    eImgFmt_img2o = eImgFmt_UNKNOWN;
    eImgFmt_img2bo = eImgFmt_UNKNOWN;
    eImgFmt_img3o = eImgFmt_UNKNOWN;
    eImgFmt_img3bo = eImgFmt_UNKNOWN;
    eImgFmt_img3co = eImgFmt_UNKNOWN;
    eImgFmt_feo = eImgFmt_UNKNOWN;
    eImgFmt_timgo = eImgFmt_UNKNOWN;
    eImgFmt_dceso = eImgFmt_UNKNOWN;
    eImgFmt_wroto = eImgFmt_UNKNOWN;
    eImgFmt_wdmao = eImgFmt_UNKNOWN;
    eImgFmt_jpego = eImgFmt_UNKNOWN;
    eImgFmt_venco = eImgFmt_UNKNOWN;

    CropInfo CropInfo_imgi;

    MUINT32 dip_ctl_dma_en = 0x0;
    MUINT32 dip_ctl_dma2_en = 0x0;
    MUINT32 dip_vertical_flip_en = 0x0;
    MINT32 imgi_uv_h_ratio = 2;   // for imgbi & imgci
    MINT32 imgi_uv_v_ratio = 2;
    MBOOL imgi_uv_swap = MFALSE;
    MINT32 vipi_uv_h_ratio = 2;   // for vip2i & vip3i
    MINT32 vipi_uv_v_ratio = 2;
    MBOOL vipi_uv_swap = MFALSE;
    MINT32 img2o_uv_h_ratio = 2;  // for img2bo
    MINT32 img2o_uv_v_ratio = 2;
    //MBOOL img2o_uv_swap = MFALSE;
    MINT32 img3o_uv_h_ratio = 2;  // for img3bo & img3co
    MINT32 img3o_uv_v_ratio = 2;
    MBOOL img3o_uv_swap = MFALSE;
    MBOOL b12bitMode = MTRUE;

    MINT32 pixelInByte = 1;
    MUINT32 LBIT_MODE0 = 1, LBIT_MODE1 = 1;

    int mdp_imgxo_p2_en = 0;

    PortID portID;
    //QBufInfo rQBufInfo;
    //path, module and select
    //MINT32 dip_cq_th, dip_cq_thr_ctl;
    MINT32 dip_cq_thr_ctl;
    MUINT32 dip_ctl_rgb_en = 0x0, dip_ctl_rgb2_en = 0x0, dip_ctl_yuv_en = DIPCTL_YUV_EN1_C42_D1|DIPCTL_YUV_EN1_C24_D3|DIPCTL_YUV_EN1_C42_D2, dip_ctl_yuv2_en = DIPCTL_YUV_EN2_BS_D1;//BS always enable in every scenario, said DE.
    MINT32 imgxi_en = 0, vipxi_en = 0, img2xo_en = 0, img3xo_en = 0, ufdi_en = 0, timgo_en = 0;
    MINT32 imgi_fmt = DIP_IMGI_FMT_BAYER10, vipi_fmt = DIP_VIPI_FMT_YUV422_1P, img2o_fmt = DIP_CRZO_FMT_YUV422_1P, img3o_fmt = DIP_IMG3O_FMT_YUV422_1P, timgo_fmt = DIP_TIMGO_FMT_BAYER10;
    MINT32 ufdi_fmt = DIP_UFDI_FMT_UFO_LENGTH, dmgi_fmt = DIP_DMGI_FMT_DRZ, depi_fmt = DIP_DEPI_FMT_DRZ, dceso_fmt = 0;// dceso_fmt dont care
    MINT32 fg_mode = DIP_FG_MODE_DISABLE;
    MUINT32 dip_ctl_path_sel = 0x0, dip_ctl_path2_sel = 0x0;
    MUINT32 sram_mode = 1; //DE: default sram_mode = 1
    MUINT32 wuv_mode = 0, nbc_gmap_ltm_mode = 0;
    MINT32 traw_sel = 0, timgo_d1_sel = 0, flc_d1_sel = 0, crp_d2_sel = 0, g2cx_d1_sel = 0, feo_d1_sel = 0;
    //ynr_d1_sel, mix_d1_sel, crsp_d1_sel, mix_d3_w_sel, c24_d1_sel, crz_d1_sel, fe_d1_sel, srz_d1_sel, plnr_d1_sel, plnr_d2_sel, nr3d_d1_pre_i_sel decide by driver scenario
    MUINT32 i, tableSize, mdpCheck1, mdpCheck2, mdpCheck3;
    MINT32 crop1TableIdx = -1;
    int dmai_swap = 0;
    MBOOL isWaitBuf = MTRUE;
    MBOOL isSetMdpOut = MFALSE;
    MUINT32 rgb_en = 0, rgb2_en = 0, yuv_en = 0, yuv2_en = 0;
    MUINT32 dip_rgb_tuning_tag = DIPCTL_RGB_EN1_BPC_D1|DIPCTL_RGB_EN1_OBC_D1|DIPCTL_RGB_EN1_ZFUS_D1|DIPCTL_RGB_EN1_DGN_D1|DIPCTL_RGB_EN1_LSC_D1|DIPCTL_RGB_EN1_WB_D1|DIPCTL_RGB_EN1_HLR_D1|DIPCTL_RGB_EN1_LTM_D1\
        |DIPCTL_RGB_EN1_LDNR_D1|DIPCTL_RGB_EN1_SLK_D1|DIPCTL_RGB_EN1_SLK_D6;
    MUINT32 dip_rgb2_tuning_tag = DIPCTL_RGB_EN2_FLC_D1|DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1|DIPCTL_RGB_EN2_CCM_D2|DIPCTL_RGB_EN2_GGM_D2|DIPCTL_RGB_EN2_SW_P2_IDENDITY_EN;
    MUINT32 dip_yuv_tuning_tag = DIPCTL_YUV_EN1_G2CX_D1|DIPCTL_YUV_EN1_MIX_D3|DIPCTL_YUV_EN1_YNR_D1|DIPCTL_YUV_EN1_NDG_D1|DIPCTL_YUV_EN1_C2G_D1|DIPCTL_YUV_EN1_IGGM_D1|DIPCTL_YUV_EN1_CCM_D3|DIPCTL_YUV_EN1_LCE_D1\
        |DIPCTL_YUV_EN1_GGM_D3|DIPCTL_YUV_EN1_DCE_D1|DIPCTL_YUV_EN1_DCES_D1|DIPCTL_YUV_EN1_G2C_D1|DIPCTL_YUV_EN1_EE_D1|DIPCTL_YUV_EN1_CNR_D1|DIPCTL_YUV_EN1_NDG_D2|DIPCTL_YUV_EN1_COLOR_D1|DIPCTL_YUV_EN1_MIX_D4;
    MUINT32 dip_yuv2_tuning_tag = DIPCTL_YUV_EN2_NR3D_D1|DIPCTL_YUV_EN2_SLK_D2|DIPCTL_YUV_EN2_SLK_D3|DIPCTL_YUV_EN2_SLK_D4|DIPCTL_YUV_EN2_SLK_D5;

    MBOOL isCropG1En = MFALSE;
    MUINT32  CropGroup;
    MUINT32 WdmaoCropGroup = 0, WrotoCropGroup = 0;

    UFDG_META_INFO *pUfdParam = NULL;
    MBOOL isUfoEn = MFALSE;

    //dip_x_reg_t *pTuningDipReg = NULL;
    pTuningDipReg = NULL;
    if (pPipePackageInfo->pTuningQue != NULL)
    {
        pTuningDipReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;
    }
	if (pTuningDipReg != NULL)
	{
		if (pTuningDipReg->DIPCTL_D1A_DIPCTL_RGB_EN2.Bits.DIPCTL_SW_P2_IDENDITY_EN)
		{
			dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1|DIPCTL_YUV_EN2_C24_D2|DIPCTL_YUV_EN2_C02_D2;
		}
	}

	ispDipPipe.isSecureFra = pFrameParams.mSecureFra;
    ispDipPipe.crzPipe.conf_cdrz = MFALSE;
#if 1 //temp  //kk test default:1
    int tpipe = CAM_MODE_TPIPE;
#else   //frame mode first
    int tpipe = CAM_MODE_FRAME;
#endif
    int tdr_tcm_en = 0x0;
    int tdr_tcm2_en = 0x0;
    int tdr_tcm3_en = 0x0;
    int tdr_tcm4_en = 0x0;
    int tdr_tcm5_en = 0x0;
    int tdr_tcm6_en = 0x0;
    int tdr_ctl = 0x0;
    int tdr_sel = 0x0;
    if (pPipePackageInfo->ForceSkipConfig == 1)
    {
        DipWorkingBuffer *pPreFrmDipWorkingBuf;
        MDPMGR_CFG_STRUCT *pPreFrmMdpCfg;
        MDPMGR_CFG_STRUCT *pMdpCfg;
        unsigned int TmpRingBufIdx;
        MINT32 planeNum=0;
	    MUINTPTR WDMA_vAddr = 0;
	    MUINTPTR WDMA_pAddr = 0;
	    MUINTPTR WROT_vAddr = 0;
	    MUINTPTR WROT_pAddr = 0;

        for (MUINT32 i = 0 ; i < pFrameParams.mvIn.size() ; i++ )
        {
            switch(VirPortIdxMapToHWPortIdx(pFrameParams.mvIn[i].mPortID.index))
            {
                case EPortIndex_IMGI:   //may multi-plane
                    idx_imgi = i;
                    portInfo_imgi =  (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgi];
                    //a_dma.memBuf.base_vAddr  = portInfo_imgi->u4BufVA[ESTRIDE_1ST_PLANE];
                    //a_dma.memBuf.base_pAddr  = portInfo_imgi->u4BufPA[ESTRIDE_1ST_PLANE];
                    break;
                case EPortIndex_LCEI:
                    idx_lcei = i;
                    portInfo_lcei = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_lcei];
                    break;
                case EPortIndex_DEPI:
                    idx_depi = i;
                    portInfo_depi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_depi];
                    break;
                default:
                    //PIPE_ERR("[Error]Not support this input port(%d),i(%d)",vInPorts[i]->index,i);
                    break;
            }
        }
        for (MUINT32 i = 0 ; i < pFrameParams.mvOut.size() ; i++ )
        {

            switch(VirPortIdxMapToHWPortIdx(pFrameParams.mvOut[i].mPortID.index))
            {

                case EPortIndex_WROTO:
                        idx_wroto = i;
                        portInfo_wroto =  (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_wroto];
                    break;
                case EPortIndex_WDMAO:
                        idx_wdmao = i;
                        portInfo_wdmao =  (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_wdmao];
                    break;
                default:
                    //PIPE_ERR("[Error]do not support this vOutPorts(%d),i(%d)",vOutPorts[i]->index,i);
                    break;
            };
        }


        //Update Partial Setting
        //
        ispDipPipe.RingBufIdx = m_pDipWorkingBuf->m_BufIdx;
        ispDipPipe.m_pDipWBuf = m_pDipWorkingBuf;
        if (m_pDipWorkingBuf->m_BufIdx > 0)
        {
            TmpRingBufIdx = m_pDipWorkingBuf->m_BufIdx-1;
        }
        else
        {
            TmpRingBufIdx = MAX_CMDQ_RING_BUFFER_SIZE_NUM-1;
        }
        pPreFrmDipWorkingBuf = m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)pPipePackageInfo->p2cqIdx, TmpRingBufIdx, MFALSE);
        pPreFrmMdpCfg = pPreFrmDipWorkingBuf->m_pMdpCfg;
        pMdpCfg = m_pDipWorkingBuf->m_pMdpCfg;



        pMdpCfg->srcVirAddr  = 0xffff/*(this->dma_cfg.memBuf.base_vAddr + this->dma_cfg.memBuf.ofst_addr)*/; //kk test

        if ( (portInfo_imgi !=NULL) && (portInfo_imgi->mBuffer!=NULL))
        {
            pMdpCfg->srcPhyAddr  = portInfo_imgi->mBuffer->getBufPA(ESTRIDE_1ST_PLANE);
        }
        else
        {
            PIPE_ERR("portInfo_imgi is NULL!! p2 driver have something wrong!!");
        }

        pMdpCfg->mdpSrcFmt = pPreFrmMdpCfg->mdpSrcFmt;
        pMdpCfg->mdpSrcW = pPreFrmMdpCfg->mdpSrcW;
        pMdpCfg->mdpSrcH = pPreFrmMdpCfg->mdpSrcH;
        pMdpCfg->mdpSrcPlaneNum = pPreFrmMdpCfg->mdpSrcPlaneNum;
        pMdpCfg->mdpSrcBufSize = pPreFrmMdpCfg->mdpSrcBufSize;
        pMdpCfg->mdpSrcCBufSize = pPreFrmMdpCfg->mdpSrcCBufSize;
        pMdpCfg->mdpSrcVBufSize = pPreFrmMdpCfg->mdpSrcVBufSize;
        pMdpCfg->mdpSrcYStride = pPreFrmMdpCfg->mdpSrcYStride;
        pMdpCfg->mdpSrcUVStride = pPreFrmMdpCfg->mdpSrcUVStride;
        pMdpCfg->drvScenario = pPreFrmMdpCfg->drvScenario;
        pMdpCfg->isoValue = pPreFrmMdpCfg->isoValue;
        pMdpCfg->pDumpTuningData = pPreFrmMdpCfg->pDumpTuningData;
        pMdpCfg->mdpWDMAPQParam = pPreFrmMdpCfg->mdpWDMAPQParam;
        pMdpCfg->mdpWROTPQParam = pPreFrmMdpCfg->mdpWROTPQParam;
        pMdpCfg->isWaitBuf = pPreFrmMdpCfg->isWaitBuf;
        pMdpCfg->imgxoEnP2 = pPreFrmMdpCfg->imgxoEnP2;
        pMdpCfg->ispDmaEn = pPreFrmMdpCfg->ispDmaEn;


        for(MINT32 index = 0; index < ISP_OUTPORT_NUM; index++) {
            pMdpCfg->dstPortCfg[index] = pPreFrmMdpCfg->dstPortCfg[index];
            if(pPreFrmMdpCfg->dstPortCfg[index] == 1) {
                pMdpCfg->dstDma[index].Plane = pPreFrmMdpCfg->dstDma[index].Plane;
                pMdpCfg->dstDma[index].size.stride = pPreFrmMdpCfg->dstDma[index].size.stride;
                pMdpCfg->dstDma[index].size.w = pPreFrmMdpCfg->dstDma[index].size.w;
                pMdpCfg->dstDma[index].size.h = pPreFrmMdpCfg->dstDma[index].size.h;
                pMdpCfg->dstDma[index].size_c.stride = pPreFrmMdpCfg->dstDma[index].size_c.stride;
                pMdpCfg->dstDma[index].size_c.w = pPreFrmMdpCfg->dstDma[index].size_c.w;
                pMdpCfg->dstDma[index].size_c.h = pPreFrmMdpCfg->dstDma[index].size_c.h;
                pMdpCfg->dstDma[index].size_v.stride = pPreFrmMdpCfg->dstDma[index].size_v.stride;
                pMdpCfg->dstDma[index].size_v.w = pPreFrmMdpCfg->dstDma[index].size_v.w;
                pMdpCfg->dstDma[index].size_v.h = pPreFrmMdpCfg->dstDma[index].size_v.h;
                pMdpCfg->dstDma[index].uv_plane_swap = pPreFrmMdpCfg->dstDma[index].uv_plane_swap;
                pMdpCfg->dstDma[index].Format = pPreFrmMdpCfg->dstDma[index].Format;
                pMdpCfg->dstDma[index].capbility = pPreFrmMdpCfg->dstDma[index].capbility;

                pMdpCfg->dstDma[index].enSrcCrop = pPreFrmMdpCfg->dstDma[index].enSrcCrop;
                if(pPreFrmMdpCfg->dstDma[index].enSrcCrop) {  //set src crop if need
                    pMdpCfg->dstDma[index].srcCropX = pPreFrmMdpCfg->dstDma[index].srcCropX;
                    pMdpCfg->dstDma[index].srcCropFloatX = pPreFrmMdpCfg->dstDma[index].srcCropFloatX;
                    pMdpCfg->dstDma[index].srcCropY = pPreFrmMdpCfg->dstDma[index].srcCropY;
                    pMdpCfg->dstDma[index].srcCropFloatY = pPreFrmMdpCfg->dstDma[index].srcCropFloatY;
                    pMdpCfg->dstDma[index].srcCropW = pPreFrmMdpCfg->dstDma[index].srcCropW;
                    pMdpCfg->dstDma[index].srcCropFloatW = pPreFrmMdpCfg->dstDma[index].srcCropFloatW;
                    pMdpCfg->dstDma[index].srcCropH = pPreFrmMdpCfg->dstDma[index].srcCropH;
                    pMdpCfg->dstDma[index].srcCropFloatH = pPreFrmMdpCfg->dstDma[index].srcCropFloatH;
                }

                if (index == ISP_MDP_DL_WDMAO)
                {
                    if ( (portInfo_wdmao !=NULL) && (portInfo_wdmao->mBuffer!=NULL))
                    {
                        planeNum = portInfo_wdmao->mBuffer->getPlaneCount();
                        switch (planeNum)
                        {
                            case 3:
                                pMdpCfg->dstDma[index].memBuf_v.base_vAddr = portInfo_wdmao->mBuffer->getBufVA(ESTRIDE_3RD_PLANE);
                                pMdpCfg->dstDma[index].memBuf_v.base_pAddr = portInfo_wdmao->mBuffer->getBufPA(ESTRIDE_3RD_PLANE);
                            case 2:
                                pMdpCfg->dstDma[index].memBuf_c.base_vAddr = portInfo_wdmao->mBuffer->getBufVA(ESTRIDE_2ND_PLANE);
                                pMdpCfg->dstDma[index].memBuf_c.base_pAddr = portInfo_wdmao->mBuffer->getBufPA(ESTRIDE_2ND_PLANE);
                            case 1:
                                pMdpCfg->dstDma[index].memBuf.base_vAddr = portInfo_wdmao->mBuffer->getBufVA(ESTRIDE_1ST_PLANE);
                                pMdpCfg->dstDma[index].memBuf.base_pAddr = portInfo_wdmao->mBuffer->getBufPA(ESTRIDE_1ST_PLANE);
								WDMA_vAddr = pMdpCfg->dstDma[index].memBuf.base_vAddr;
								WDMA_pAddr = pMdpCfg->dstDma[index].memBuf.base_pAddr;
                                break;
                        }
                    }
                    else
                    {
                        PIPE_ERR("portInfo_wdmao is NULL!! p2 driver have something wrong!!");

                    }
                }
                else if (index == ISP_MDP_DL_WROTO)
                {
                    if ( (portInfo_wroto !=NULL) && (portInfo_wroto->mBuffer!=NULL))
                    {
                        planeNum = portInfo_wroto->mBuffer->getPlaneCount();
                        switch (planeNum)
                        {
                            case 3:
                                pMdpCfg->dstDma[index].memBuf_v.base_vAddr = portInfo_wroto->mBuffer->getBufVA(ESTRIDE_3RD_PLANE);
                                pMdpCfg->dstDma[index].memBuf_v.base_pAddr = portInfo_wroto->mBuffer->getBufPA(ESTRIDE_3RD_PLANE);
                            case 2:
                                pMdpCfg->dstDma[index].memBuf_c.base_vAddr = portInfo_wroto->mBuffer->getBufVA(ESTRIDE_2ND_PLANE);
                                pMdpCfg->dstDma[index].memBuf_c.base_pAddr = portInfo_wroto->mBuffer->getBufPA(ESTRIDE_2ND_PLANE);
                            case 1:
                                pMdpCfg->dstDma[index].memBuf.base_vAddr = portInfo_wroto->mBuffer->getBufVA(ESTRIDE_1ST_PLANE);
                                pMdpCfg->dstDma[index].memBuf.base_pAddr = portInfo_wroto->mBuffer->getBufPA(ESTRIDE_1ST_PLANE);
								WROT_vAddr = pMdpCfg->dstDma[index].memBuf.base_vAddr;
								WROT_pAddr = pMdpCfg->dstDma[index].memBuf.base_pAddr;
                                break;
                        }
                    }
                    else
                    {
                        PIPE_ERR("portInfo_wroto is NULL!! p2 driver have something wrong!!");

                    }
                }
                // rotation
                pMdpCfg->dstDma[index].Rotation = pPreFrmMdpCfg->dstDma[index].Rotation;

                // flip
                pMdpCfg->dstDma[index].Flip = pPreFrmMdpCfg->dstDma[index].Flip;
            }
        }


        //copy tdr for deque fail
        ::memcpy(&pMdpCfg->ispTpipeCfgInfo, &pPreFrmMdpCfg->ispTpipeCfgInfo, sizeof(ISP_TPIPE_CONFIG_STRUCT));
        //Update IMGI Address
        pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x3; //force skip
        if ( (portInfo_imgi !=NULL) && (portInfo_imgi->mBuffer!=NULL))
        {
            pMdpCfg->ispTpipeCfgInfo.drvinfo.dip_imgi_base_addr = portInfo_imgi->mBuffer->getBufPA(ESTRIDE_1ST_PLANE);
            PIPE_INF("imgi(VA/PA:0x%" PRIxPTR ",0x%" PRIxPTR "),wdmo(VA/PA:0x%" PRIxPTR ",0x%" PRIxPTR "),wroto(VA/PA:0x%" PRIxPTR ",0x%" PRIxPTR "),tv_sec(%lu),tv_usec(%lu)",
                portInfo_imgi->mBuffer->getBufVA(ESTRIDE_1ST_PLANE), portInfo_imgi->mBuffer->getBufPA(ESTRIDE_1ST_PLANE),WDMA_vAddr, WDMA_pAddr, WROT_vAddr, WROT_pAddr,
                pMdpCfg->ExpectedEndTime.tv_sec, pMdpCfg->ExpectedEndTime.tv_usec);
        }
        else
        {
            PIPE_ERR("portInfo_imgi is NULL!! or portInfo_imgi->mBuffer is NULL!! ");
        }
        if ( (portInfo_lcei !=NULL) && (portInfo_lcei->mBuffer!=NULL))
        {
            pMdpCfg->ispTpipeCfgInfo.drvinfo.dip_lcei_base_addr = portInfo_lcei->mBuffer->getBufPA(ESTRIDE_1ST_PLANE);
            PIPE_INF("lcei(VA/PA:0x%" PRIxPTR ",0x%" PRIxPTR ")",
                portInfo_lcei->mBuffer->getBufVA(ESTRIDE_1ST_PLANE), portInfo_lcei->mBuffer->getBufPA(ESTRIDE_1ST_PLANE));
        }
        if ( (portInfo_depi !=NULL) && (portInfo_depi->mBuffer!=NULL))
        {
            pMdpCfg->ispTpipeCfgInfo.drvinfo.dip_depi_base_vaddr = portInfo_depi->mBuffer->getBufPA(ESTRIDE_1ST_PLANE);
            PIPE_INF("depi(VA/PA:0x%" PRIxPTR ",0x%" PRIxPTR ")",
                portInfo_depi->mBuffer->getBufVA(ESTRIDE_1ST_PLANE), portInfo_depi->mBuffer->getBufPA(ESTRIDE_1ST_PLANE));
        }

        return MTRUE;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    //judge pass type
    dip_cq_thr_ctl = CQ_D1A_THRX_CTL_EN|CQ_D1A_THRX_CTL_MODE; //immediately mode

    ///////////////////////////////////////////////////////////////////////////////////////
    //parse the dma ports contained in the frame
    PIPE_DBG("StreamTag[%d]/cq[%d]/in[%lu]/out[%lu]", pPipePackageInfo->StreamTag, pPipePackageInfo->p2cqIdx, (unsigned long)pFrameParams.mvIn.size(), (unsigned long)pFrameParams.mvOut.size());
    if (m_mmpathEn)
            PrintMMPathInfo(pFrameParams);

    if ((MTKCAM_LOG_LEVEL_DEFAULT >= 3) || (m_mtkcamLogLevel >=3))
    {
        if (m_dipConfigPipeLog)
            PrintPipePortInfo(pFrameParams);
    }

    for (MUINT32 i = 0 ; i < pFrameParams.mvIn.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvIn[i] ) { continue; }
        //
        planeNum = pFrameParams.mvIn[i].mBuffer->getPlaneCount();

        //
        switch(VirPortIdxMapToHWPortIdx(pFrameParams.mvIn[i].mPortID.index))
        {
            case EPortIndex_IMGI:    //may multi-plane
                idx_imgi = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_IMGI_D1;
                portInfo_imgi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgi];
                eImgFmt_imgi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_imgi].mBuffer->getImgFormat());
                pixIdImgBuf = PixelIDMapping(pFrameParams.mvIn[idx_imgi].mBuffer->getColorArrangement());
                m_imgi_planeNum = planeNum;
                /*PIPE_INF("portInfo_imgi:[%d]:(0x%x),w(%d),h(%d),stride(%d,0x0,0x0),idx(%d),dir(%d),Size(0x%08x,0x0,0x0),VA(0x%lx,0x0,0x0),PA(0x%lx,0x0,0x0)",\
                                                            i,
                                                            (NSCam::EImageFormat)(portInfo_imgi->mBuffer->getImgFormat()),
                                                            portInfo_imgi->mBuffer->getImgSize().w,
                                                            portInfo_imgi->mBuffer->getImgSize().h,
                                                            portInfo_imgi->mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE),
                                                            //mPortTypeMapping[pFrameParams.mvIn[i].mPortID.type].eImgIOPortType,
                                                            portInfo_imgi->mPortID.index,
                                                            portInfo_imgi->mPortID.inout,
                                                            portInfo_imgi->mBuffer->getBufSizeInBytes(0),
                                                            (unsigned long)portInfo_imgi->mBuffer->getBufVA(0),
                                                            (unsigned long)portInfo_imgi->mBuffer->getBufPA(0));*/

                // input port limitation check for FullG format
                if ((eImgFmt_imgi == eImgFmt_FG_BAYER8) || (eImgFmt_imgi == eImgFmt_FG_BAYER10) ||
                        (eImgFmt_imgi == eImgFmt_FG_BAYER12) || (eImgFmt_imgi == eImgFmt_FG_BAYER14)){
                    //TODO: size must be align 4 for FUllG format, USER TAKE CARE THIS
                }
                //
                if(planeNum >= 2)    // check plane number
                { // check imgbi
                    switch (eImgFmt_imgi)
                    {
                        case eImgFmt_NV12:          // 420 format, 2 plane (Y),(UV)
                        case eImgFmt_MTK_YUV_P010:
                        case eImgFmt_YUV_P010:
                        case eImgFmt_NV21:          // 420 format, 2 plane (Y),(VU)
                        case eImgFmt_MTK_YVU_P010:
                        case eImgFmt_YVU_P010:
                            imgi_uv_h_ratio = 1;
                            break;
                        case eImgFmt_NV16:          // 422 format, 2 plane (Y),(UV)
                        case eImgFmt_MTK_YUV_P210:
                        case eImgFmt_YUV_P210:
                        case eImgFmt_NV61:          // 422 format, 2 plane (Y),(VU)
                        case eImgFmt_MTK_YVU_P210:
                        case eImgFmt_YVU_P210:
                            imgi_uv_h_ratio = 1;
                            imgi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_YV16:          // 422 format, 3 plane (Y),(V),(U)
                            imgi_uv_v_ratio = 1;
                            imgi_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I422:          // 422 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P210_3PLANE:
                        case eImgFmt_YUV_P210_3PLANE:
                            imgi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_YV12:          // 420 format, 3 plane (Y),(V),(U)
                            imgi_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I420:          // 420 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P010_3PLANE:
                        case eImgFmt_YUV_P010_3PLANE:
                        default:
                            break;
                    }

                    if ((eImgFmt_imgi == eImgFmt_UFO_FG_BAYER10) || (eImgFmt_imgi == eImgFmt_UFO_FG_BAYER12) || (eImgFmt_imgi == eImgFmt_UFO_BAYER10) || (eImgFmt_imgi == eImgFmt_UFO_BAYER12))
                    {
                        idx_ufdi = i;
                        dip_ctl_dma_en |= DIPCTL_DMA_EN1_UFDI_D1;
                        //if (vInPorts[idx_ufdi]->eImgFmt != eImgFmt_UFO_FG)
                        //     dip_ctl_yuv_en |=    (DIP_X_REG_CTL_YUV_EN_SRZ1|DIP_X_REG_CTL_YUV_EN_MIX1);
                        portInfo_ufdi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_ufdi];
                        eImgFmt_ufdi = eImgFmt_imgi;

                        if (planeNum ==3)
                        {
                            pUfdParam = (UFDG_META_INFO *)pFrameParams.mvIn[idx_ufdi].mBuffer->getBufVA(2);
                            PIPE_INF("UFDG_META_INFO: UFDG_BITSTREAM_OFST_ADDR(%d),UFDG_BS_AU_START(%d),UFDG_AU2_SIZE(%d),UFDG_BOND_MODE(%d)", pUfdParam->UFDG_BITSTREAM_OFST_ADDR, pUfdParam->UFDG_BS_AU_START, pUfdParam->UFDG_AU2_SIZE, pUfdParam->UFDG_BOND_MODE);
                        }
                        isUfoEn = MTRUE;
                    }
                    else
                    {
                        idx_imgbi = i;
                        dip_ctl_dma_en |= DIPCTL_DMA_EN1_IMGBI_D1;
                        portInfo_imgbi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgbi];
                        eImgFmt_imgbi = eImgFmt_imgi;

                        if(planeNum == 3)
                        { // check imgci
                            idx_imgci = i;
                            dip_ctl_dma_en |= DIPCTL_DMA_EN1_IMGCI_D1;
                            portInfo_imgci = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgci];
                            eImgFmt_imgci = eImgFmt_imgi;
                        }
                    }
                }

                // Parsing crop information +++++
                CropInfo_imgi.crop1.x      = 0;
                CropInfo_imgi.crop1.floatX = 0;
                CropInfo_imgi.crop1.y      = 0;
                CropInfo_imgi.crop1.floatY = 0;
                CropInfo_imgi.crop1.w      = 0;
                CropInfo_imgi.crop1.floatW = 0;
                CropInfo_imgi.crop1.h      = 0;
                CropInfo_imgi.crop1.floatH = 0;
                CropInfo_imgi.resize1.tar_w = 0;
                CropInfo_imgi.resize1.tar_h = 0;
                CropInfo_imgi.crop2.x = 0;
                CropInfo_imgi.crop2.floatX = 0;
                CropInfo_imgi.crop2.y = 0;
                CropInfo_imgi.crop2.floatY = 0;
                CropInfo_imgi.crop2.w      = 0;
                CropInfo_imgi.crop2.floatW = 0;
                CropInfo_imgi.crop2.h      = 0;
                CropInfo_imgi.crop2.floatH = 0;
                CropInfo_imgi.resize2.tar_w = 0;
                CropInfo_imgi.resize2.tar_h = 0;
                CropInfo_imgi.crop3.x = 0;
                CropInfo_imgi.crop3.floatX = 0;
                CropInfo_imgi.crop3.y = 0;
                CropInfo_imgi.crop3.floatY = 0;
                CropInfo_imgi.crop3.w      = 0;
                CropInfo_imgi.crop3.floatW = 0;
                CropInfo_imgi.crop3.h      = 0;
                CropInfo_imgi.crop3.floatH = 0;
                CropInfo_imgi.resize3.tar_w = 0;
                CropInfo_imgi.resize3.tar_h = 0;
                for(MUINT32 k=0;k<pFrameParams.mvCropRsInfo.size();k++)
                {
                    //crop1/crop2 may be different in each buffer node
                        switch(pFrameParams.mvCropRsInfo[k].mGroupID)
                        {
                            case 1:    // for CRZ
                                CropInfo_imgi.crop1.x      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop1.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop1.y      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop1.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop1.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop1.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop1.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop1.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize1.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize1.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                break;
                            case 2:    // for WDMAO
                                CropInfo_imgi.crop2.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop2.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop2.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop2.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop2.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop2.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop2.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop2.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize2.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize2.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                WdmaoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                                break;
                            case 3: // for WROTO
                                CropInfo_imgi.crop3.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop3.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop3.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop3.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop3.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop3.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop3.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop3.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize3.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize3.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                WrotoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                                break;
                            default:
                                PIPE_ERR("DO not support crop group (%d)",pFrameParams.mvCropRsInfo[k].mGroupID);
                            break;
                        }
                }

                // Parsing crop information -----

                PIPE_DBG("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),p2RingBufIdx(%d)", \
                    CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
                    CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
                    CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
                    CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
                    CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
                    CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
                    CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
                    CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
                    CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,pPipePackageInfo->p2RingBufIdx);

                break;
            case EPortIndex_IMGBI:    //Mixing path weighting table
                idx_imgbi = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_IMGBI_D1;
                portInfo_imgbi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgbi];
                eImgFmt_imgbi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_imgbi].mBuffer->getImgFormat());
                break;
            case EPortIndex_IMGCI:    //Mixing path weighting table
                idx_imgci = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_IMGCI_D1;
                portInfo_imgci = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_imgci];
                eImgFmt_imgci = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_imgci].mBuffer->getImgFormat());
                break;
            case EPortIndex_VIPI:    //may multi-plane, vipi~vip3i
                idx_vipi = i;
                dip_ctl_dma2_en |= DIPCTL_DMA_EN2_VIPI_D1;
                portInfo_vipi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vipi];
                eImgFmt_vipi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_vipi].mBuffer->getImgFormat());
                if(planeNum >= 2)
                { // check vip2i
                    switch (eImgFmt_vipi)
                    {
                        case eImgFmt_NV12:          // 420 format, 2 plane (Y),(UV)
                        case eImgFmt_MTK_YUV_P010:
                        case eImgFmt_YUV_P010:
                        case eImgFmt_NV21:          // 420 format, 2 plane (Y),(VU)
                        case eImgFmt_MTK_YVU_P010:
                        case eImgFmt_YVU_P010:
                            vipi_uv_h_ratio = 1;
                            break;
                        case eImgFmt_NV16:          // 422 format, 2 plane (Y),(UV)
                        case eImgFmt_MTK_YUV_P210:
                        case eImgFmt_YUV_P210:
                        case eImgFmt_NV61:          // 422 format, 2 plane (Y),(VU)
                        case eImgFmt_MTK_YVU_P210:
                        case eImgFmt_YVU_P210:
                            vipi_uv_h_ratio = 1;
                            vipi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_YV16:          // 422 format, 3 plane (Y),(V),(U)
                            vipi_uv_v_ratio = 1;
                            vipi_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I422:          // 422 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P210_3PLANE:
                        case eImgFmt_YUV_P210_3PLANE:
                            vipi_uv_v_ratio = 1;
                            break;
                        case eImgFmt_YV12:          // 420 format, 3 plane (Y),(V),(U)
                            vipi_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I420:          // 420 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P010_3PLANE:
                        case eImgFmt_YUV_P010_3PLANE:
                        default:
                            break;
                    }

                    idx_vip2i = i;
                    dip_ctl_dma2_en |= DIPCTL_DMA_EN2_VIPBI_D1;
                    portInfo_vip2i = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vip2i];
                    eImgFmt_vip2i = eImgFmt_vipi;

                    if(planeNum == 3)
                    { // check vip3i
                        idx_vip3i = i;
                        dip_ctl_dma2_en |= DIPCTL_DMA_EN2_VIPCI_D1;
                        portInfo_vip3i = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vip3i];
                        eImgFmt_vip3i = eImgFmt_vipi;
                    }
                }
                break;
            case EPortIndex_VIP3I: //only vip3i input(weighting table), there are only vipi+vip3i without vip2i for mfb mixing path
                idx_vip3i = i;
                dip_ctl_dma2_en |= DIPCTL_DMA_EN2_VIPCI_D1;

                portInfo_vip3i = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_vip3i];
                eImgFmt_vip3i = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_vip3i].mBuffer->getImgFormat());
                break;
            case EPortIndex_LCEI:
                idx_lcei = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_LCEI_D1;

                //if(drvScen == eDrvScenario_VFB_FB || drvScen == eDrvScenario_P2B_Bokeh){
                if(StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB || StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Bokeh){
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_MIX_D2;
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_SRZ_D4;
                }

                portInfo_lcei = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_lcei];
                eImgFmt_lcei = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_lcei].mBuffer->getImgFormat());
                break;
            case EPortIndex_UFDI:
                idx_ufdi = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_UFDI_D1;

                portInfo_ufdi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_ufdi];
                eImgFmt_ufdi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_ufdi].mBuffer->getImgFormat());

                if (eImgFmt_ufdi != eImgFmt_UFO_FG){
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_MIX_D1;
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_SRZ_D1;
                }

                #if 0 //TODO path
                if((meScenarioID!=eDrvScenario_CC_vFB_FB)&&(meScenarioID!=eDrvScenario_CC_MFB_Mixing))
                {
                    en_p2 |= CAM_CTL_EN_P2_UFD_EN;
                    ufd_sel = 1;
                }
                #endif
                break;
            case EPortIndex_DEPI:
                idx_depi = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_DEPI_D1;
                //if(drvScen == eDrvScenario_P2B_Bokeh) {
                //      dip_ctl_yuv2_en |=  (DIP_X_REG_CTL_YUV2_EN_SRZ4);
                //}
                portInfo_depi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_depi];
                eImgFmt_depi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_depi].mBuffer->getImgFormat());
                //if(drvScen == eDrvScenario_LPCNR_Pass2) {
                if(StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2) {
                    dip_ctl_dma_en |= DIPCTL_DMA_EN1_DMGI_D1;
                }
                break;
            case EPortIndex_DMGI:
                idx_dmgi = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_DMGI_D1;
                //if(drvScen == eDrvScenario_P2B_Bokeh) {
                //      dip_ctl_yuv2_en |=  (DIP_X_REG_CTL_YUV2_EN_SRZ3);
                //}
                portInfo_dmgi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_dmgi];
                eImgFmt_dmgi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_dmgi].mBuffer->getImgFormat());
                break;
            case EPortIndex_REGI:
                idx_regi = i;
                portInfo_regi = (NSCam::NSIoPipe::Input *)&pFrameParams.mvIn[idx_regi];
                eImgFmt_regi = (NSCam::EImageFormat)(pFrameParams.mvIn[idx_regi].mBuffer->getImgFormat());
                break;
            default:
                PIPE_ERR("[Error]Not support this input port(%d),i(%d)",pFrameParams.mvIn[i].mPortID.index,i);
                break;
        }
    }

        //
    for (MUINT32 i = 0 ; i < pFrameParams.mvOut.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvOut[i] ) { continue; }
        //
        planeNum = pFrameParams.mvOut[i].mBuffer->getPlaneCount();

        if ((MTRUE == getCropFunctionEnable(StreamTag, (MINT32)pFrameParams.mvOut[i].mPortID.index, CropGroup))
            && (CropGroup & ECropGroupIndex_1))
        {
            //Check the crop setting.
            isCropG1En = MTRUE;
        }
        switch(VirPortIdxMapToHWPortIdx(pFrameParams.mvOut[i].mPortID.index))
        {
            case EPortIndex_IMG2O: //may multi-plane, img2o+IMG2bo
                idx_img2o = i;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_CRZO_D1;
                portInfo_img2o = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img2o];
                eImgFmt_img2o = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_img2o].mBuffer->getImgFormat());

                if(planeNum == 2)
                { // check img2bo
                    switch (eImgFmt_img2o)
                    {
                        case eImgFmt_NV16:          // 422 format, 2 plane (Y),(UV)
                        case eImgFmt_NV61:          // 422 format, 2 plane (Y),(VU)
                            img2o_uv_h_ratio = 1;
                            img2o_uv_v_ratio = 1;
                            break;
                        case eImgFmt_MTK_YUV_P210:
                        case eImgFmt_YUV_P210:
                        case eImgFmt_MTK_YVU_P210:
                        case eImgFmt_YVU_P210:
                        case eImgFmt_NV12:          // 420 format, 2 plane (Y),(UV)
                        case eImgFmt_MTK_YUV_P010:
                        case eImgFmt_YUV_P010:
                        case eImgFmt_NV21:          // 420 format, 2 plane (Y),(VU)
                        case eImgFmt_MTK_YVU_P010:
                        case eImgFmt_YVU_P010:
                        case eImgFmt_YV16:          // 422 format, 3 plane (Y),(V),(U)
                        case eImgFmt_I422:          // 422 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P210_3PLANE:
                        case eImgFmt_YUV_P210_3PLANE:
                        case eImgFmt_YV12:          // 420 format, 3 plane (Y),(V),(U)
                        case eImgFmt_I420:          // 420 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P010_3PLANE:
                        case eImgFmt_YUV_P010_3PLANE:
                            PIPE_ERR("IMG2O do not support 3 plane & 420 2 plane & 10b");
                            break;
                        default:
                            break;
                    }

                    idx_img2bo = i;
                    dip_ctl_dma_en |= DIPCTL_DMA_EN1_CRZBO_D1;
                    portInfo_img2bo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img2bo];
                    eImgFmt_img2bo = eImgFmt_img2o;
                }
                break;
            case EPortIndex_IMG3O:    //may multi-plane, img3o+img3bo+img3co
                idx_img3o = i;
                dip_ctl_dma2_en |= DIPCTL_DMA_EN2_IMG3O_D1;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PAK_D6;

                portInfo_img3o = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img3o];
                eImgFmt_img3o = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_img3o].mBuffer->getImgFormat());

                if(planeNum >= 2)
                { // check img3bo
                    switch (eImgFmt_img3o)
                    {
                        case eImgFmt_NV12:          // 420 format, 2 plane (Y),(UV)
                        case eImgFmt_MTK_YUV_P010:
                        case eImgFmt_YUV_P010:
                        case eImgFmt_NV21:          // 420 format, 2 plane (Y),(VU)
                        case eImgFmt_MTK_YVU_P010:
                        case eImgFmt_YVU_P010:
                            img3o_uv_h_ratio = 1;
                            break;
                        case eImgFmt_NV16:          // 422 format, 2 plane (Y),(UV)
                        case eImgFmt_MTK_YUV_P210:
                        case eImgFmt_YUV_P210:
                        case eImgFmt_NV61:          // 422 format, 2 plane (Y),(VU)
                        case eImgFmt_MTK_YVU_P210:
                        case eImgFmt_YVU_P210:
                            img3o_uv_h_ratio = 1;
                            img3o_uv_v_ratio = 1;
                            break;
                        case eImgFmt_YV16:          // 422 format, 3 plane (Y),(V),(U)
                            img3o_uv_v_ratio = 1;
                            img3o_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I422:          // 422 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P210_3PLANE:
                        case eImgFmt_YUV_P210_3PLANE:
                            img3o_uv_v_ratio = 1;
                            break;
                        case eImgFmt_YV12:          // 420 format, 3 plane (Y),(V),(U)
                            img3o_uv_swap = MTRUE;
                            break;
                        case eImgFmt_I420:          // 420 format, 3 plane (Y),(U),(V)
                        case eImgFmt_MTK_YUV_P010_3PLANE:
                        case eImgFmt_YUV_P010_3PLANE:
                        default:
                            break;
                    }

                    idx_img3bo = i;
                    dip_ctl_dma2_en |= DIPCTL_DMA_EN2_IMG3BO_D1;
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PAK_D7;

                    portInfo_img3bo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img3bo];
                    eImgFmt_img3bo = eImgFmt_img3o;

                    if(planeNum == 3)
                    { // check img3co
                        idx_img3co = i;
                        dip_ctl_dma2_en |= DIPCTL_DMA_EN2_IMG3CO_D1;
                        dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PAK_D8;
                        portInfo_img3co =  (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_img3co];
                        eImgFmt_img3co = eImgFmt_img3o;
                    }
                }
                break;
            case EPortIndex_WROTO:
                idx_wroto = i;
                if (WrotoCropGroup == 0)
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_MCRP_D1;
                }
                else if (WrotoCropGroup == 1)
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_MCRP_D2;
                    else
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_MCRP_D2|DIPCTL_RGB_EN2_CCM_D2|DIPCTL_RGB_EN2_GGM_D2;
                }
                else
                {
                    PIPE_ERR("[Error]do not support this crop group of WROTO(%d)",WrotoCropGroup);
                    return MFALSE;
                }
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D2;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_WROTO;
                portInfo_wroto = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_wroto];
                eImgFmt_wroto = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_wroto].mBuffer->getImgFormat());
                break;
            case EPortIndex_WDMAO:
                idx_wdmao = i;
                if (WdmaoCropGroup == 0)
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_MCRP_D1;
                }
                else if (WdmaoCropGroup == 1)
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_MCRP_D2;
                    else
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_MCRP_D2|DIPCTL_RGB_EN2_CCM_D2|DIPCTL_RGB_EN2_GGM_D2;
                }
                else
                {
                    PIPE_ERR("[Error]do not support this crop group of WDMAO(%d)",WdmaoCropGroup);
                    return MFALSE;
                }
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D2;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_WDMAO;
                portInfo_wdmao = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_wdmao];
                eImgFmt_wdmao = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_wdmao].mBuffer->getImgFormat());
                break;
            case EPortIndex_JPEGO:
                idx_jpego = i;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_MCRP_D1|DIPCTL_YUV_EN2_MCRP_D1;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_JPEGO;
                portInfo_jpego = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_jpego];
                eImgFmt_jpego = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_jpego].mBuffer->getImgFormat());
                break;
            case EPortIndex_FEO:
                //if(drvScen == eDrvScenario_VFB_FB)
                if(StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB)
                {
                    PIPE_ERR("[Error]do not support FE when P2B path");
                    return MFALSE;
                }
                //else if (drvScen == eDrvScenario_Y16_Dump)
                else if (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Y16Dump)
                {
                    idx_feo = i;
                    dip_ctl_dma_en |= DIPCTL_DMA_EN1_FEO_D1;
                    portInfo_feo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_feo];
                    eImgFmt_feo = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_feo].mBuffer->getImgFormat());
                }
                else
                {
                    idx_feo = i;
                    dip_ctl_dma_en |= DIPCTL_DMA_EN1_FEO_D1;
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_DFE_D1;
                    portInfo_feo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_feo];
                    eImgFmt_feo = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_feo].mBuffer->getImgFormat());
                }
                break;
            case EPortIndex_TIMGO:
                idx_timgo = i;
                portInfo_timgo = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_timgo];
                eImgFmt_timgo = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_timgo].mBuffer->getImgFormat());
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_TIMGO_D1;
                break;
            case EPortIndex_DCESO:
                idx_dceso = i;
                portInfo_dceso = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_dceso];
                eImgFmt_dceso = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_dceso].mBuffer->getImgFormat());
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_DCESO_D1;
                break;
            case EPortIndex_VENC_STREAMO:
                idx_venco = i;
                portInfo_venco = (NSCam::NSIoPipe::Output *)&pFrameParams.mvOut[idx_venco];
                eImgFmt_venco = (NSCam::EImageFormat)(pFrameParams.mvOut[idx_venco].mBuffer->getImgFormat());
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_MCRP_D1|DIPCTL_YUV_EN2_C24_D2;
                dip_ctl_dma_en |= DIPCTL_DMA_EN1_VENCO;
                break;
            default:
                PIPE_ERR("[Error]do not support this vOutPorts(%d),i(%d)",pFrameParams.mvOut[i].mPortID.index,i);
                break;
        };
    }

    if (m_isMfbDL) {
        // Parsing crop information +++++
        CropInfo_imgi.crop1.x      = 0;
        CropInfo_imgi.crop1.floatX = 0;
        CropInfo_imgi.crop1.y      = 0;
        CropInfo_imgi.crop1.floatY = 0;
        CropInfo_imgi.crop1.w      = 0;
        CropInfo_imgi.crop1.h      = 0;
        CropInfo_imgi.resize1.tar_w = 0;
        CropInfo_imgi.resize1.tar_h = 0;
        CropInfo_imgi.crop2.x = 0;
        CropInfo_imgi.crop2.floatX = 0;
        CropInfo_imgi.crop2.y = 0;
        CropInfo_imgi.crop2.floatY = 0;
        CropInfo_imgi.crop2.w      = 0;
        CropInfo_imgi.crop2.h      = 0;
        CropInfo_imgi.resize2.tar_w = 0;
        CropInfo_imgi.resize2.tar_h = 0;
        CropInfo_imgi.crop3.x = 0;
        CropInfo_imgi.crop3.floatX = 0;
        CropInfo_imgi.crop3.y = 0;
        CropInfo_imgi.crop3.floatY = 0;
        CropInfo_imgi.crop3.w      = 0;
        CropInfo_imgi.crop3.h      = 0;
        CropInfo_imgi.resize3.tar_w = 0;
        CropInfo_imgi.resize3.tar_h = 0;
        for(MUINT32 k=0;k<pFrameParams.mvCropRsInfo.size();k++)
        {
            //crop1/crop2 may be different in each buffer node
                switch(pFrameParams.mvCropRsInfo[k].mGroupID)
                {
                    case 1:    // for CRZ
                        CropInfo_imgi.crop1.x      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                        CropInfo_imgi.crop1.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                        CropInfo_imgi.crop1.y      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                        CropInfo_imgi.crop1.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                        CropInfo_imgi.crop1.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                        CropInfo_imgi.crop1.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                        CropInfo_imgi.resize1.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                        CropInfo_imgi.resize1.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                        break;
                    case 2:    // for WDMAO
                        CropInfo_imgi.crop2.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                        CropInfo_imgi.crop2.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                        CropInfo_imgi.crop2.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                        CropInfo_imgi.crop2.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                        CropInfo_imgi.crop2.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                        CropInfo_imgi.crop2.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                        CropInfo_imgi.resize2.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                        CropInfo_imgi.resize2.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                        WdmaoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                        break;
                    case 3: // for WROTO
                        CropInfo_imgi.crop3.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                        CropInfo_imgi.crop3.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                        CropInfo_imgi.crop3.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                        CropInfo_imgi.crop3.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                        CropInfo_imgi.crop3.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                        CropInfo_imgi.crop3.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                        CropInfo_imgi.resize3.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                        CropInfo_imgi.resize3.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                        WrotoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                        break;
                    default:
                        PIPE_ERR("DO not support crop group (%d)",pFrameParams.mvCropRsInfo[k].mGroupID);
                    break;
                }
        }

        // Parsing crop information -----

        PIPE_DBG("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d),p2RingBufIdx(%d)", \
            CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
            CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.h,CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
            CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
            CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.h,CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h,
            CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
            CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.h,CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,pPipePackageInfo->p2RingBufIdx);
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    //specific module setting
    for (MUINT32 i = 0 ; i < pPipePackageInfo->vModuleParams.size() ; i++ )
    {
        switch(pPipePackageInfo->vModuleParams[i].eDipModule)
        {
            case EDipModule_SRZ1:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct))
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_SRZ_D1;

                    this->configSrz(&ispDipPipe.srz1Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ1:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz1Cfg.inout_size.in_w,ispDipPipe.srz1Cfg.inout_size.in_h,\
                        ispDipPipe.srz1Cfg.crop.x, ispDipPipe.srz1Cfg.crop.floatX, ispDipPipe.srz1Cfg.crop.y, ispDipPipe.srz1Cfg.crop.floatY,\
                        ispDipPipe.srz1Cfg.crop.w, ispDipPipe.srz1Cfg.crop.h, ispDipPipe.srz1Cfg.inout_size.out_w,ispDipPipe.srz1Cfg.inout_size.out_h,\
                        ispDipPipe.srz1Cfg.h_step, ispDipPipe.srz1Cfg.v_step);
                }
                break;
#if 0 //YWclr
            case EDipModule_SRZ2:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct))
                {
                    dip_ctl_yuv_en |= DIP_X_REG_CTL_YUV_EN_SRZ2;
                    tdr_tcm_en |= DIP_X_REG_CTL_TDR_SRZ2_TCM_EN;
                    this->configSrz(&ispDipPipe.srz2Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ2:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz2Cfg.inout_size.in_w,ispDipPipe.srz2Cfg.inout_size.in_h,\
                        ispDipPipe.srz2Cfg.crop.x, ispDipPipe.srz2Cfg.crop.floatX, ispDipPipe.srz2Cfg.crop.y, ispDipPipe.srz2Cfg.crop.floatY,\
                        ispDipPipe.srz2Cfg.crop.w, ispDipPipe.srz2Cfg.crop.h, ispDipPipe.srz2Cfg.inout_size.out_w,ispDipPipe.srz2Cfg.inout_size.out_h,\
                        ispDipPipe.srz2Cfg.h_step, ispDipPipe.srz2Cfg.v_step);
                }
                break;
#endif
            case EDipModule_SRZ3:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_SRZ_D3;
                    this->configSrz(&ispDipPipe.srz3Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ3:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz3Cfg.inout_size.in_w,ispDipPipe.srz3Cfg.inout_size.in_h,\
                        ispDipPipe.srz3Cfg.crop.x, ispDipPipe.srz3Cfg.crop.floatX, ispDipPipe.srz3Cfg.crop.y, ispDipPipe.srz3Cfg.crop.floatY,\
                        ispDipPipe.srz3Cfg.crop.w, ispDipPipe.srz3Cfg.crop.h, ispDipPipe.srz3Cfg.inout_size.out_w,ispDipPipe.srz3Cfg.inout_size.out_h,\
                        ispDipPipe.srz3Cfg.h_step, ispDipPipe.srz3Cfg.v_step);
                }
                break;
            case EDipModule_SRZ4:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_SRZ_D4;
                    this->configSrz(&ispDipPipe.srz4Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ4:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz4Cfg.inout_size.in_w,ispDipPipe.srz4Cfg.inout_size.in_h,\
                        ispDipPipe.srz4Cfg.crop.x, ispDipPipe.srz4Cfg.crop.floatX, ispDipPipe.srz4Cfg.crop.y, ispDipPipe.srz4Cfg.crop.floatY,\
                        ispDipPipe.srz4Cfg.crop.w, ispDipPipe.srz4Cfg.crop.h, ispDipPipe.srz4Cfg.inout_size.out_w,ispDipPipe.srz4Cfg.inout_size.out_h,\
                        ispDipPipe.srz4Cfg.h_step, ispDipPipe.srz4Cfg.v_step);
                }
                break;
#if 0 //YWclr
            case EDipModule_SRZ5:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct)) {
                    dip_ctl_yuv2_en |= DIP_X_REG_CTL_YUV2_EN_SRZ5;
                    tdr_tcm2_en |= DIP_X_REG_CTL_TDR_SRZ5_TCM_EN;

                    this->configSrz(&ispDipPipe.srz5Cfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(pPipePackageInfo->vModuleParams[i].moduleStruct), pPipePackageInfo->vModuleParams[i].eDipModule);

                    PIPE_INF("SRZ5:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",ispDipPipe.srz5Cfg.inout_size.in_w,ispDipPipe.srz5Cfg.inout_size.in_h,\
                        ispDipPipe.srz5Cfg.crop.x, ispDipPipe.srz5Cfg.crop.floatX, ispDipPipe.srz5Cfg.crop.y, ispDipPipe.srz5Cfg.crop.floatY,\
                        ispDipPipe.srz5Cfg.crop.w, ispDipPipe.srz5Cfg.crop.h, ispDipPipe.srz5Cfg.inout_size.out_w,ispDipPipe.srz5Cfg.inout_size.out_h,\
                        ispDipPipe.srz5Cfg.h_step, ispDipPipe.srz5Cfg.v_step);
                }
                break;
#endif
            default:
                PIPE_ERR("[Error]do not support this HWmodule(%d),i(%d)",pPipePackageInfo->vModuleParams[i].eDipModule,i);
                break;
        };
    }

    /////////////////////////////////////////////////////////////
    //judge format and multi-plane

    ufdi_en = (int)((dip_ctl_dma_en & DIPCTL_DMA_EN1_UFDI_D1)>>16);
    if (ufdi_en != 0)
    {
        switch( eImgFmt_ufdi )
        {
            //case eImgFmt_FG_BAYER8:      //TODO, revise for ufdi format
            case eImgFmt_UFO_FG:
            case eImgFmt_UFO_FG_BAYER10:
            case eImgFmt_UFO_FG_BAYER12:
            case eImgFmt_UFO_BAYER10:
            case eImgFmt_UFO_BAYER12:
                ufdi_fmt = DIP_UFDI_FMT_UFO_LENGTH;
                if(pPipePackageInfo->pTuningQue != NULL)
                {
                    dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UFD_D1|DIPCTL_RGB_EN1_DM_D1;
                }
                else
                {
                    dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UFD_D1|DIPCTL_RGB_EN1_DM_D1;
                    dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                }
                traw_sel = 1;
                break;
            case eImgFmt_STA_BYTE:
                ufdi_fmt = DIP_UFDI_FMT_WEIGHTING;
                break;
            default:
                PIPE_ERR("[Error] ufdi NOT Support this format(0x%x)",eImgFmt_ufdi);
                return MFALSE;
        }
    }
    //
    vipxi_en = (int)((dip_ctl_dma2_en & 0x00000007));
    if (vipxi_en != 0)
    {
        switch( eImgFmt_vipi )
        {
            case eImgFmt_NV21:        //420 format, 2 plane (VU)
            case eImgFmt_NV12:        //420 format, 2 plane (UV)
            case eImgFmt_MTK_YVU_P010:
            case eImgFmt_MTK_YUV_P010:
            case eImgFmt_YVU_P010:
            case eImgFmt_YUV_P010:
                vipi_fmt = DIP_VIPI_FMT_YUV420_2P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C02_D1;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D9|DIPCTL_YUV_EN2_UNP_D10);
                if(vipxi_en != 3)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YV12:        //420 format, 3 plane (YVU)
            case eImgFmt_I420:        //420 format, 3 plane (YUV)
            case eImgFmt_MTK_YUV_P010_3PLANE:
            case eImgFmt_YUV_P010_3PLANE:
                vipi_fmt = DIP_VIPI_FMT_YUV420_3P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C02_D1;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D9|DIPCTL_YUV_EN2_UNP_D10|DIPCTL_YUV_EN2_UNP_D11);
                if(vipxi_en != 7)
                {
                    PIPE_ERR("should enable vipi+vip2i+vip3i when 3-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YUY2:        //422 format, 1 plane (YUYV)
            case eImgFmt_MTK_YUYV_Y210:
            case eImgFmt_YUYV_Y210:
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D9;
                dmai_swap = 1;
                break;
            case eImgFmt_UYVY:        //422 format, 1 plane (UYVY)
            case eImgFmt_MTK_UYVY_Y210:
            case eImgFmt_UYVY_Y210:
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D9;
                dmai_swap = 0;
                break;
            case eImgFmt_YVYU:        //422 format, 1 plane (YVYU)
            case eImgFmt_MTK_YVYU_Y210:
            case eImgFmt_YVYU_Y210:
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D9;
                dmai_swap = 3;
                break;
            case eImgFmt_VYUY:        //422 format, 1 plane (VYUY)
            case eImgFmt_MTK_VYUY_Y210:
            case eImgFmt_VYUY_Y210:
                vipi_fmt = DIP_VIPI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D9;
                dmai_swap = 2;
                break;
            case eImgFmt_YV16:        //422 format, 3 plane (YVU)
            case eImgFmt_I422:        //422 format, 3 plane (YUV)
            case eImgFmt_MTK_YUV_P210_3PLANE:
            case eImgFmt_YUV_P210_3PLANE:
                vipi_fmt = DIP_VIPI_FMT_YUV422_3P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNR_D1;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D9|DIPCTL_YUV_EN2_UNP_D10|DIPCTL_YUV_EN2_UNP_D11);
                if(vipxi_en != 7)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_NV61:        //422 format, 2 plane (VU)
            case eImgFmt_NV16:        //422 format, 2 plane (UV)
            case eImgFmt_MTK_YVU_P210:
            case eImgFmt_MTK_YUV_P210:
            case eImgFmt_YVU_P210:
            case eImgFmt_YUV_P210:
                vipi_fmt = DIP_VIPI_FMT_YUV422_2P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNR_D1;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D9|DIPCTL_YUV_EN2_UNP_D10);
                if(vipxi_en != 3)
                {
                    PIPE_ERR("should enable vipi+vip2i when 2-plane input (0x%x)",vipxi_en);
                    return MFALSE;
                }
                break;
#if 0 //YWclr
            case eImgFmt_RGB565:    //= 0x00080000,   //RGB 565 (16-bit), 1 plane
                vipi_fmt = DIP_VIPI_FMT_RGB565;
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_GDR_D1;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                //g2g_sel = 2; vipi don't support rgb format
                //nbc_sel = 1; //rgb in
                break;
            case eImgFmt_RGB888:    //= 0x00100000,   //RGB 888 (24-bit), 1 plane
                vipi_fmt = DIP_VIPI_FMT_RGB888;
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_GDR_D1;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                //g2g_sel = 2; vipi don't support rgb format
                //nbc_sel = 1; //rgb in
                dmai_swap = 0; //0:RGB,2:BGR
                break;
            case eImgFmt_ARGB8888:     //= 0x00800000,   //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
                vipi_fmt = DIP_VIPI_FMT_XRGB8888;
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_GDR_D1;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                //g2g_sel = 2; vipi don't support rgb format
                //nbc_sel = 1; //rgb in
                dmai_swap = 0;    //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
                break;
#endif
            default:
                PIPE_ERR("[Error] vipxi NOT Support this format(0x%x)",eImgFmt_vipi);
                return MFALSE;
        }
    }
        //
    imgxi_en = (int)((dip_ctl_dma_en & 0x00000007));
    if (imgxi_en != 0)
    {
        switch( eImgFmt_imgi )
        {
            case eImgFmt_FG_BAYER8:      //FG Bayer format, 8-bit
                imgi_fmt = DIP_IMGI_FMT_BAYER8;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_FG_BAYER10:      //FG Bayer format, 10-bit
            case eImgFmt_UFO_FG_BAYER10:
                imgi_fmt = DIP_IMGI_FMT_BAYER10;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_FG_BAYER12:      //FG Bayer format, 12-bit
            case eImgFmt_UFO_FG_BAYER12:
                imgi_fmt = DIP_IMGI_FMT_BAYER12;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_FG_BAYER14:      //FG Bayer format, 14-bit
                imgi_fmt = DIP_IMGI_FMT_BAYER14;
                fg_mode = DIP_FG_MODE_ENABLE;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_BAYER8:      //Bayer format, 8-bit
            case eImgFmt_BAYER8_UNPAK:
                if (eImgFmt_BAYER8 == eImgFmt_imgi)
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER8;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER8_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        //TILE DRIVER don't check UNPAK, so unpak must be enabled when input is raw file.
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_BAYER10:      //Bayer format, 10-bit
            case eImgFmt_BAYER10_UNPAK:
            case eImgFmt_UFO_BAYER10:
                if ((eImgFmt_BAYER10 == eImgFmt_imgi) || (eImgFmt_UFO_BAYER10 == eImgFmt_imgi))
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER10;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER10_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_BAYER12:      //Bayer format, 12-bit
            case eImgFmt_BAYER12_UNPAK:
            case eImgFmt_UFO_BAYER12:
                if ((eImgFmt_BAYER12 == eImgFmt_imgi) || (eImgFmt_UFO_BAYER12 == eImgFmt_imgi))
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER12;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER12_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_BAYER14:      //Bayer format, 14-bit
            case eImgFmt_BAYER14_UNPAK:
                if (eImgFmt_BAYER14 == eImgFmt_imgi)
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER14;
                }
                else
                {
                    imgi_fmt = DIP_IMGI_FMT_BAYER14_2BYTEs;
                }
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_BAYER10_MIPI:      //Bayer format, 10-bit
                imgi_fmt = DIP_IMGI_FMT_BAYER10_MIPI;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                }
                if(idx_ufdi < 0) //= -1, no ufdi input
                {
                    if(pPipePackageInfo->pTuningQue != NULL)
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                    }
                    else
                    {
                        dip_ctl_rgb_en |= DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_DM_D1;
                        dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_CCM_D1|DIPCTL_RGB_EN2_GGM_D1;
                    }
                    traw_sel = 0;
                }
                g2cx_d1_sel = 0;
                flc_d1_sel = 0;
                break;
            case eImgFmt_NV21:        //420 format, 2 plane (VU)
            case eImgFmt_NV12:        //420 format, 2 plane (UV)
            case eImgFmt_MTK_YVU_P010:
            case eImgFmt_MTK_YUV_P010:
            case eImgFmt_YVU_P010:
            case eImgFmt_YUV_P010:
                imgi_fmt = DIP_IMGI_FMT_YUV420_2P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNR_D2|DIPCTL_YUV_EN2_C02_D2;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D6|DIPCTL_YUV_EN2_UNP_D7);
                g2cx_d1_sel = 1;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                if(imgxi_en != 3)
                {
                    PIPE_ERR("should enable imgi+imgbi when 2-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YV12:        //420 format, 3 plane (YVU)
            case eImgFmt_I420:        //420 format, 3 plane (YUV)
            case eImgFmt_MTK_YUV_P010_3PLANE:
            case eImgFmt_YUV_P010_3PLANE:
                imgi_fmt = DIP_IMGI_FMT_YUV420_3P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNR_D2|DIPCTL_YUV_EN2_C02_D2;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D6|DIPCTL_YUV_EN2_UNP_D7|DIPCTL_YUV_EN2_UNP_D8);
                g2cx_d1_sel = 1;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                if(imgxi_en != 7)
                {
                    PIPE_ERR("should enable imgi+imgbi+imgci when 3-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YUY2:        //422 format, 1 plane (YUYV)
            case eImgFmt_MTK_YUYV_Y210:
            case eImgFmt_YUYV_Y210:
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D6;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                g2cx_d1_sel = 1;
                break;
            case eImgFmt_UYVY:        //422 format, 1 plane (UYVY)
            case eImgFmt_MTK_UYVY_Y210:
            case eImgFmt_UYVY_Y210:
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D6;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                g2cx_d1_sel = 1;
                break;
            case eImgFmt_YVYU:        //422 format, 1 plane (YVYU)
            case eImgFmt_MTK_YVYU_Y210:
            case eImgFmt_YVYU_Y210:
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D6;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                g2cx_d1_sel = 1;
                break;
            case eImgFmt_VYUY:        //422 format, 1 plane (VYUY)
            case eImgFmt_MTK_VYUY_Y210:
            case eImgFmt_VYUY_Y210:
                imgi_fmt = DIP_IMGI_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_UNP_D6;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                g2cx_d1_sel = 1;
                break;
            case eImgFmt_YV16:        //422 format, 3 plane (YVU)
            case eImgFmt_I422:        //422 format, 3 plane (YUV)
            case eImgFmt_MTK_YUV_P210_3PLANE:
            case eImgFmt_YUV_P210_3PLANE:
                imgi_fmt = DIP_IMGI_FMT_YUV422_3P;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D6|DIPCTL_YUV_EN2_UNP_D7|DIPCTL_YUV_EN2_UNP_D8);
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNR_D2;
                g2cx_d1_sel = 1;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                if(imgxi_en != 7)
                {
                    PIPE_ERR("should enable imgi+imgbi+imgci when 3-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_NV61:        //422 format, 2 plane (VU)
            case eImgFmt_NV16:        //422 format, 2 plane (UV)
            case eImgFmt_MTK_YVU_P210:
            case eImgFmt_MTK_YUV_P210:
            case eImgFmt_YVU_P210:
            case eImgFmt_YUV_P210:
                imgi_fmt = DIP_IMGI_FMT_YUV422_2P;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNR_D2;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_UNP_D6|DIPCTL_YUV_EN2_UNP_D7);
                g2cx_d1_sel = 1;
                //if(nbc_sel == 0) //vipi rgb input
                {
                    dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_C24_D1;
                }
                if(imgxi_en != 3)
                {
                    PIPE_ERR("should enable imgi+imgbi when 2-plane input (0x%x)",imgxi_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_RGB565:       //RGB 565 (16-bit), 1 plane
                imgi_fmt = DIP_IMGI_FMT_RGB565;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                    dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_GGM_D1;
                }
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_GDR_D1;
                flc_d1_sel = 1;
                break;
            case eImgFmt_RGB888:       //RGB 888 (24-bit), 1 plane
                imgi_fmt = DIP_IMGI_FMT_RGB888;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                    dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_GGM_D1;
                }
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_GDR_D1;
                flc_d1_sel = 1;
                break;
            case eImgFmt_ARGB8888:     //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
                imgi_fmt = DIP_IMGI_FMT_XRGB8888;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                    dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_GGM_D1;
                }
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_GDR_D1;
                flc_d1_sel = 1;
                break;
            case eImgFmt_RGB48:        //RGB121212 (48-bit), 1 plane
                imgi_fmt = DIP_IMGI_FMT_RGB121212;
                if(pPipePackageInfo->pTuningQue != NULL)
                {}
                else
                {
                    dip_ctl_yuv_en |= DIPCTL_YUV_EN1_G2CX_D1 | DIPCTL_YUV_EN1_C2G_D1 | DIPCTL_YUV_EN1_IGGM_D1 | DIPCTL_YUV_EN1_GGM_D3;
                    dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_GGM_D1|DIPCTL_RGB_EN2_CCM_D1;
                }
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_GDR_D1;
                flc_d1_sel = 1;
                break;

            case eImgFmt_STA_BYTE:
                break;
            case eImgFmt_NV21_BLK:    //420 format block mode, 2 plane (UV)
            case eImgFmt_NV12_BLK:    //420 format block mode, 2 plane (VU)
            case eImgFmt_JPEG:        //JPEG format
            default:
                PIPE_ERR("[Error] imgxi NOT Support this format(0x%x)",eImgFmt_imgi);
                return MFALSE;
        }
        //Check Image Buffer PixId is equal to NormalStream query
        if ((imgi_fmt >= DIP_IMGI_FMT_BAYER8) && (imgi_fmt <= DIP_IMGI_FMT_BAYER10_MIPI))
        {
            if (pixIdImgBuf != (MINT32)pPipePackageInfo->pixIdP2)
            {
                //PIPE_ERR("[Error] ImgBuf PixelID(%d) is not the same as NormalStream Query(%d)!!",pixIdImgBuf, pPipePackageInfo->pixIdP2);
            }
        }
    }
    //
    img2xo_en = (int)((dip_ctl_dma_en & 0x00000060)>>5);
    if (img2xo_en != 0)
    {
        switch(eImgFmt_img2o)
        {
            case eImgFmt_YUY2:        //422 format, 1 plane (YUYV)
                img2o_fmt = DIP_CRZO_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_PLNW_D2);
                break;
            case eImgFmt_UYVY:        //422 format, 1 plane (UYVY)
                img2o_fmt = DIP_CRZO_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_PLNW_D2);
                break;
            case eImgFmt_YVYU:        //422 format, 1 plane (YVYU)
                img2o_fmt = DIP_CRZO_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_PLNW_D2);
                break;
            case eImgFmt_VYUY:        //422 format, 1 plane (VYUY)
                img2o_fmt = DIP_CRZO_FMT_YUV422_1P;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_PLNW_D2);
                break;
            case eImgFmt_NV16:        //422 format, 2 plane (UV)
            case eImgFmt_NV61:        //422 format, 2 plane (VU)
                img2o_fmt = DIP_CRZO_FMT_YUV422_2P;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_PLNW_D2);
                if(img2xo_en != 3)
                {
                    PIPE_ERR("should enable img2o+img2bo when 2-plane input (0x%x)",img2xo_en);
                    return MFALSE;
                }
                break;
            default:
                PIPE_ERR("[Error] img2o NOT Support this format(0x%x)",eImgFmt_img2o);
                return MFALSE;
        }
    }
    //
    img3xo_en = (int)((dip_ctl_dma2_en & 0x00000038)>>3);
    if (img3xo_en != 0)
    {
        switch(eImgFmt_img3o)
        {
            case eImgFmt_NV21:        //420 format, 2 plane (VU)
            case eImgFmt_NV12:        //420 format, 2 plane (UV)
            case eImgFmt_MTK_YVU_P010:
            case eImgFmt_MTK_YUV_P010:
            case eImgFmt_YVU_P010:
            case eImgFmt_YUV_P010:
                img3o_fmt = DIP_IMG3O_FMT_YUV420_2P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_CRSP_D1|DIPCTL_YUV_EN2_PLNW_D1);
                if(img3xo_en != 3)
                {
                    PIPE_ERR("should enable img3o+img3bo when 2-plane input (0x%x)",img3xo_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YV12:        //420 format, 3 plane (YVU)
            case eImgFmt_I420:        //420 format, 3 plane (YUV)
            case eImgFmt_MTK_YUV_P010_3PLANE:
            case eImgFmt_YUV_P010_3PLANE:
                img3o_fmt = DIP_IMG3O_FMT_YUV420_3P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= (DIPCTL_YUV_EN2_CRSP_D1|DIPCTL_YUV_EN2_PLNW_D1);
                if(img3xo_en != 7)
                {
                    PIPE_ERR("should enable img3o+img3bo+img3co when 3-plane input (0x%x)",img3xo_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_YUY2:        //422 format, 1 plane (YUYV)
            case eImgFmt_MTK_YUYV_Y210:
            case eImgFmt_YUYV_Y210:
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNW_D1;
                break;
            case eImgFmt_UYVY:        //422 format, 1 plane (UYVY)
            case eImgFmt_MTK_UYVY_Y210:
            case eImgFmt_UYVY_Y210:
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNW_D1;
                break;
            case eImgFmt_YVYU:        //422 format, 1 plane (YVYU)
            case eImgFmt_MTK_YVYU_Y210:
            case eImgFmt_YVYU_Y210:
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNW_D1;
                break;
            case eImgFmt_VYUY:        //422 format, 1 plane (VYUY)
            case eImgFmt_MTK_VYUY_Y210:
            case eImgFmt_VYUY_Y210:
                img3o_fmt = DIP_IMG3O_FMT_YUV422_1P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNW_D1;
                break;
            case eImgFmt_YV16:        //422 format, 3 plane (YVU)
            case eImgFmt_I422:        //422 format, 3 plane (YUV)
            case eImgFmt_MTK_YUV_P210_3PLANE:
            case eImgFmt_YUV_P210_3PLANE:
                img3o_fmt = DIP_IMG3O_FMT_YUV422_3P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNW_D1;
                if(img3xo_en != 7)
                {
                    PIPE_ERR("should enable img3o+img3bo+img3co when 3-plane input (0x%x)",img3xo_en);
                    return MFALSE;
                }
                break;
            case eImgFmt_NV61:        //422 format, 2 plane (VU)
            case eImgFmt_NV16:        //422 format, 2 plane (UV)
            case eImgFmt_MTK_YVU_P210:
            case eImgFmt_MTK_YUV_P210:
            case eImgFmt_YVU_P210:
            case eImgFmt_YUV_P210:
                img3o_fmt= DIP_IMG3O_FMT_YUV422_2P;
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_CRP_D1;
                dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_PLNW_D1;
                if(img3xo_en != 3)
                {
                    PIPE_ERR("should enable img3o+img3bo when 2-plane input (0x%x)",img3xo_en);
                    return MFALSE;
                }
                break;
            default:
                PIPE_ERR("[Error] img3o NOT Support this format(0x%x)",eImgFmt_img3o);
                return MFALSE;
        }
    }


    timgo_en = (int)((dip_ctl_dma_en & DIPCTL_DMA_EN1_TIMGO_D1)>>15);
    if (timgo_en != 0)
    {
        //if ((drvScen != eDrvScenario_MFB_Blending) && (drvScen != eDrvScenario_LPCNR_Pass1) && (drvScen != eDrvScenario_LPCNR_Pass2) && (m_TimgoDumpSel != NSCam::NSIoPipe::EDIPTimgoDump_AFTER_GGM)) //YWtodo
        if ((StreamTag != NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Bld) && (StreamTag != NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass1) && (StreamTag != NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2) && (m_TimgoDumpSel != NSCam::NSIoPipe::EDIPTimgoDump_AFTER_GGM)) //YWtodo
        {
            dip_ctl_rgb_en |= (DIPCTL_RGB_EN1_CRP_D2|DIPCTL_RGB_EN1_PAK_D1);
        }

        switch(eImgFmt_timgo)
        {
            case eImgFmt_BAYER8:     //Bayer format, 8-bit
                timgo_fmt = DIP_TIMGO_FMT_BAYER8;
                break;
            case eImgFmt_BAYER8_UNPAK:
                timgo_fmt = DIP_TIMGO_FMT_BAYER8_2BYTEs;
                break;
            case eImgFmt_BAYER10:    //Bayer format, 10-bit
                timgo_fmt = DIP_TIMGO_FMT_BAYER10;
                break;
            case eImgFmt_BAYER10_UNPAK:
                timgo_fmt = DIP_TIMGO_FMT_BAYER10_2BYTEs;
                break;
            case eImgFmt_BAYER12:    //Bayer format, 12-bit
                timgo_fmt = DIP_TIMGO_FMT_BAYER12;
                break;
            case eImgFmt_BAYER12_UNPAK:
                timgo_fmt = DIP_TIMGO_FMT_BAYER12_2BYTEs;
                break;
            case eImgFmt_BAYER14:    //Bayer format, 14-bit
                timgo_fmt = DIP_TIMGO_FMT_BAYER14;
                break;
            case eImgFmt_BAYER14_UNPAK:
                timgo_fmt = DIP_TIMGO_FMT_BAYER14_2BYTEs;
                break;
            case eImgFmt_STA_BYTE:
            case eImgFmt_STA_12BIT:
            case eImgFmt_NV12:
            case eImgFmt_NV21:
            case eImgFmt_MTK_YUV_P010:
            case eImgFmt_MTK_YVU_P010:
            case eImgFmt_RGB48: //for bit true dump
                timgo_fmt = DIP_TIMGO_FMT_BAYER8;
                break;
            default:
                PIPE_ERR("[Error] timgo NOT Support this format(0x%x)",eImgFmt_timgo);
                return MFALSE;
        }
    }

    /////////////////////////////////////////////////////////////
    //judge module enable and mux_sel based on different path
    //switch (drvScen)
    switch (StreamTag)
    {
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Normal:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_DeNoise:
            /*
            - sel notes               - sel2 notes
            - traw_sel = X            - mix_d3_w_sel = X
            - timgo_d1_sel = X        - c24_d1_sel = 0
            - flc_d1_sel = 2          - crz_d1_sel = 0
            - crp_d2_sel = X          - fe_d1_sel = 1
            - g2cx_d1_sel = 0         - srz_d1_sel = 1
            - ynr_d1_sel = 0          - feo_d1_sel = 0
            - mix_d1_sel = 0          - plnr_d1_sel = 0 //no use
            - crsp_d1_sel = 0         - plnr_d2_sel = 0 // no use
            - dgn_d1_sel = X          - nr3d_d1_pre_i_sel = 0
            */
            if (-1 != idx_timgo)
            {
                //timgo_sel = 0x1; //0:MFB weighting output, 1:from after PAK2, 2:FM output, 3:from after G2G
                timgo_d1_sel = 0x0; //0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
                //if (eDrvScenario_DeNoise == drvScen)
                if (NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_DeNoise == StreamTag)
                {
                    // TODO: check with algo and FPM
                    //g2g2_sel = 1;
                    //wuv_mode = 1;
                    //dip_ctl_rgb_en  |= DIP_X_REG_CTL_RGB_EN_WSYNC;
                    //rcp2_sel = 0x0; // 0 : from after PGN,      1 : from after LSC2,        2 : from before LSC2
                    //this feature is phase out!!
                    //0:from after LTMd1, 1:from HLRd1, 2:from LSCd1, 3:DGNd1
                }
                else
                {
                    //rcp2_sel = 0x1; // 0 : from after PGN,      1 : from after LSC2,        2 : from before LSC2
                    crp_d2_sel = 0x2; //0:from after LTMd1, 1:from HLRd1, 2:from LSCd1, 3:DGNd1
                }
                dip_ctl_path_sel = 0x00000000;
                dip_ctl_path2_sel = 0x00014500;
            }
            else
            {
                dip_ctl_path_sel = 0x00000000;
                dip_ctl_path2_sel = 0x00014500;
            }
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB:
            /*
            - sel notes
            - srz1_sel=0;
            - mix1_sel=1;
            - nbc_sel; (depend on vipi format)
            - nbcw_sel=0;
            - crz_sel=1;
            - nr3d_sel=1;
            - mdp_sel=0;
            - fe_sel=0;
            - crsp_sel=0;
            */
            dip_ctl_path_sel = 0x00005100;
            dip_ctl_yuv_en |= DIPCTL_YUV_EN1_MIX_D1|DIPCTL_YUV_EN1_MIX_D2;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Bokeh:
            /*
            - sel notes               - sel2 notes
            - traw_sel = X            - mix_d3_w_sel = X
            - timgo_d1_sel = X        - c24_d1_sel = 0
            - flc_d1_sel = 2          - crz_d1_sel = 1
            - crp_d2_sel = X          - fe_d1_sel = X
            - g2cx_d1_sel = 1         - srz_d1_sel = X
            - ynr_d1_sel = 0          - feo_d1_sel = X
            - mix_d1_sel = 0          - plnr_d1_sel = 0 //no use
            - crsp_d1_sel = 0         - plnr_d2_sel = 0 //no use
            - dgn_d1_sel = X          - nr3d_d1_pre_i_sel = X
            */
            dip_ctl_path_sel  = 0x00515200;   //bit 8,9 flc_sel=2 , bit20 mix1_sel=1;  bit16 g2cx_sel = 1, bit22 crsp_sel=1;
            dip_ctl_path2_sel = 0x00014010;   //bit 4,5 crz_sel=1
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Bld:
            /*
            - sel notes
            - srz1_sel=1;
            - mix1_sel=0;
            - nbc_sel=0;
            - nbcw_sel=0;
            - crz_sel=2;
            - nr3d_sel=2;
            - mdp_sel=0;
            - fe_sel=0;
            - crsp_sel=0;
            */
            timgo_d1_sel = 0x0; //0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            sram_mode = 2;
            dip_ctl_path_sel = 0x00000010;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix:
            /*
            - sel notes
            - srz1_sel=0;
            - mix1_sel=0;
            - nbc_sel=0;
            - nbcw_sel=1;
            - crz_sel=0;
            - nr3d_sel=0;
            - mdp_sel=0;
            - fe_sel=0;
            - crsp_sel=0;
            */
            //imgi_fmt = DIP_IMGI_FMT_MFB_MIX_MODE;

            if (m_isMfbDL == 1) // MFB DL P2
            {
                dip_ctl_path_sel = 0x00090240;     //bit 8,9 flc_sel=2  // bit6 timgo_sel(default) = 4 , bit16 g2cx_sel = 1 // bit18,19 ynr_sel = 2
                dip_ctl_path2_sel = 0x00000401;   //bit 8 fe_sel=0 , bit 10 srz_d1_sel = 1 // bit 1 mix3_w_sel = 1
                dip_ctl_yuv_en &= 0xFFFFFFFC;     // Close G2CX and C42
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_WIF_D2;
                dip_ctl_rgb_en |= DIPCTL_RGB_EN1_WIF_D3;
            }
            else
            {
                dip_ctl_path_sel = 0x00010240;     //bit 8,9 flc_sel=2  // bit6 timgo_sel(default) = 4 , bit16 g2cx_sel = 1 // bit18,19 ynr_sel = 0
                dip_ctl_path2_sel = 0x00000400;   //bit 8 fe_sel=0 , bit 10 srz_d1_sel = 1
            }

            if (m_bMix4En)
            {
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_MIX_D4;
            }
            else
            {
                dip_ctl_yuv_en |= DIPCTL_YUV_EN1_MIX_D3;
            }
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss:
            /*
            - sel notes
            - srz1_sel=1;
            - mix1_sel=0;
            - nbc_sel=0;
            - nbcw_sel=0;
            - crz_sel=2;
            - nr3d_sel=2;
            - mdp_sel=0;
            - fe_sel=0;
            - crsp_sel=0;
            */
            dip_ctl_path_sel = 0x00006040;
            isWaitBuf = MFALSE;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FE:
            /*
            - sel notes               - sel2 notes
            - traw_sel = X            - mix_d3_w_sel = X
            - timgo_d1_sel = X        - c24_d1_sel = 0
            - flc_d1_sel = X          - crz_d1_sel = 0
            - crp_d2_sel = X          - fe_d1_sel = 1
            - g2cx_d1_sel = X         - srz_d1_sel = 1
            - ynr_d1_sel = 1          - feo_d1_sel = 0
            - mix_d1_sel = 0          - plnr_d1_sel = ?
            - crsp_d1_sel = X         - plnr_d2_sel = ?
            - dgn_d1_sel = X          - nr3d_d1_pre_i_sel = X
            */
            dip_ctl_path_sel = 0x00000000;
            dip_ctl_path2_sel = 0x00000500;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM:
            //TODO
            //dip_ctl_path_sel = 0x00006040;
            dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_FM_D1;
            timgo_d1_sel = 0x1; //0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            sram_mode = 3;
            dmgi_fmt = DIP_DMGI_FMT_FM;
            depi_fmt = DIP_DEPI_FMT_FM;
            tpipe = CAM_MODE_FRAME;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_COLOR_EFT:
            /*
            - sel notes
            - srz1_sel=1;
            - mix1_sel=0;
            - nbc_sel=0;
            - nbcw_sel=0;
            - crz_sel=2;
            - nr3d_sel=1;
            - mdp_sel=0;
            - fe_sel=1;
            - crsp_sel=0;
            */
            dip_ctl_path_sel = 0x00016040;
            //
            flc_d1_sel = 2;
            g2cx_d1_sel = 0;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_WUV:
            /*
            - sel notes
            - srz1_sel=1;
            - mix1_sel=0;
            - nbc_sel=0;
            - nbcw_sel=0;
            - crz_sel=2;
            - nr3d_sel=1;
            - mdp_sel=0;
            - fe_sel=1;
            - crsp_sel=0;
            - g2g2_sel=1;
            */
            dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_WSYNC_D1;
            dip_ctl_path_sel = 0x01016040;
            //g2g2_sel = 1;
            wuv_mode = 1;
            if(pPipePackageInfo->pTuningQue != NULL)
            {
                dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_WSYNC_D1;
            }
            else
            {
                dip_ctl_rgb2_en |= DIPCTL_RGB_EN2_WSYNC_D1|DIPCTL_RGB_EN2_CCM_D2|DIPCTL_RGB_EN2_GGM_D2;
            }
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Y16Dump:
            /*
            - sel notes
            - srz1_sel=1;
            - mix1_sel=0;
            - nbc_sel=0;
            - nbcw_sel=0;
            - crz_sel=2;
            - nr3d_sel=1;
            - mdp_sel=0;
            - fe_sel=1;
            - crsp_sel=0;
            - feo_d1_sel=1;
            */
            dip_ctl_path_sel = 0x00816040;
            feo_d1_sel=1;
            break;
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2:
            dip_vertical_flip_en = (DIPCTL_DMGI_D1_V_FLIP_EN | DIPCTL_DEPI_D1_V_FLIP_EN | DIPCTL_LCEI_D1_V_FLIP_EN | DIPCTL_TIMGO_D1_V_FLIP_EN);
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass1:
            dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_LPCNR_D1;
            timgo_d1_sel = 0x3; //0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:LPCNR, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            sram_mode = 2;
            dmgi_fmt = DIP_DMGI_FMT_DRZ;
            depi_fmt = DIP_DEPI_FMT_DRZ;
            tpipe = CAM_MODE_FRAME;
            break;
        default:
            PIPE_ERR("NOT Support scenario(%d)",StreamTag);
            break;
    }
    //According TIMGO Dump Sel to force dump point
    switch (m_TimgoDumpSel)
    {
        case NSCam::NSIoPipe::EDIPTimgoDump_AFTER_DGN:
            timgo_d1_sel = 0;////0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            crp_d2_sel = 0x3; //0:from after LTMd1, 1:from HLRd1, 2:from LSCd1, 3:DGNd1
            break;
        case NSCam::NSIoPipe::EDIPTimgoDump_AFTER_LSC:
            timgo_d1_sel = 0;////0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            crp_d2_sel = 0x2; //0:from after LTMd1, 1:from HLRd1, 2:from LSCd1, 3:DGNd1
            break;
        case NSCam::NSIoPipe::EDIPTimgoDump_AFTER_HLR:
            timgo_d1_sel = 0;////0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            crp_d2_sel = 0x1; //0:from after LTMd1, 1:from HLRd1, 2:from LSCd1, 3:DGNd1
            break;
        case NSCam::NSIoPipe::EDIPTimgoDump_AFTER_LTM:
            timgo_d1_sel = 0;////0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            crp_d2_sel = 0x0; //0:from after LTMd1, 1:from HLRd1, 2:from LSCd1, 3:DGNd1
            break;
        case NSCam::NSIoPipe::EDIPTimgoDump_AFTER_GGM:
            timgo_d1_sel = 4;////0:frm PAKd1, 1:timgo_d1_sel_1, 2:timgo_d1_sel_2, 3:timgo_d1_sel_3, 4:timgo_d1_sel_4, 5:timgo_d1_sel_5
            break;
        default:
            break;
    }

    //Check CRZ is enable or not.
    if (MTRUE == isCropG1En)
    {
        if (-1 != idx_img2o)
        {
            m_CrzEn = MTRUE;
        }
        else
        {
            switch (StreamTag)//FE_SEL = 0
            {
                case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_vFB_FB:
                case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Bld:
                case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Mix:
                    if (( -1 != idx_img3o) || ( -1 != idx_wroto) || ( -1 != idx_wdmao) || ( -1 != idx_jpego))
                    {
                        m_CrzEn = MTRUE;
                    }
                    break;
                default:
                    if ((dip_ctl_path_sel & 0x100000) == 0x100000)//MIX1_SEL = 1   //P2B Bokeh = ENormalStreamTag_Bokeh
                    {
                        if (( -1 != idx_img3o) || ( -1 != idx_wroto) || ( -1 != idx_wdmao) || ( -1 != idx_jpego))
                        {
                            m_CrzEn = MTRUE;
                        }
                    }
                    break;
            }
        }
    }
    if (m_CrzEn == MTRUE)
    {
        dip_ctl_yuv2_en |= DIPCTL_YUV_EN2_CRZ_D1;
    }
    PIPE_DBG("isCropG1En(%d),m_CrzEn(%d)",isCropG1En,m_CrzEn);
    //Handle Smart Tile.
    //Smart Tile1 Enable
    if ((m_bSmartTileEn) && (pPipePackageInfo->pTuningQue != NULL))
    {
         if ((pTuningDipReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Raw & DIPCTL_RGB_EN1_UFD_D1) == DIPCTL_RGB_EN1_UFD_D1)
         {
             switch (imgi_fmt)
             {
                 case DIP_IMGI_FMT_BAYER8:
                 case DIP_IMGI_FMT_BAYER10:
                 case DIP_IMGI_FMT_BAYER12:
                 case DIP_IMGI_FMT_BAYER8_2BYTEs:
                 case DIP_IMGI_FMT_BAYER10_2BYTEs:
                 case DIP_IMGI_FMT_BAYER12_2BYTEs:
                     dip_ctl_rgb_en |= (DIPCTL_RGB_EN1_SMT_D1|DIPCTL_RGB_EN1_UNP_D1|DIPCTL_RGB_EN1_PAKG_D1);
                     dip_ctl_dma_en |= (DIPCTL_DMA_EN1_SMTI_D1|DIPCTL_DMA_EN1_SMTO_D1);
                     break;
                 default:
                     break;
             }
         }
         if (((dip_ctl_yuv2_en & DIPCTL_YUV_EN2_MCRP_D1) == DIPCTL_YUV_EN2_MCRP_D1) ||
             ((pTuningDipReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Raw & DIPCTL_YUV_EN2_NR3D_D1) == DIPCTL_YUV_EN2_NR3D_D1))
         {
             dip_ctl_yuv_en |= (DIPCTL_YUV_EN1_SMT_D3|DIPCTL_YUV_EN1_UNP_D3|DIPCTL_YUV_EN1_PAK_D3);
             dip_ctl_dma_en |= (DIPCTL_DMA_EN1_SMTI_D3|DIPCTL_DMA_EN1_SMTO_D3);
         }
#if 0 //SMT_D4 HW is not work
         if ((pTuningDipReg->DIPCTL_D1A_DIPCTL_RGB_EN2.Raw & DIPCTL_RGB_EN2_MCRP_D2) == DIPCTL_RGB_EN2_MCRP_D2)
         {
             //dip_ctl_rgb2_en |= (DIPCTL_RGB_EN2_SMT_D4|DIPCTL_RGB_EN2_UNP_D4|DIPCTL_RGB_EN2_PAK_D4); don't need to enable pak_d4 and  unp_d4.
             dip_ctl_rgb2_en |= (DIPCTL_RGB_EN2_SMT_D4);
             dip_ctl_dma_en |= (DIPCTL_DMA_EN1_SMTI_D4|DIPCTL_DMA_EN1_SMTO_D4);
         }
#endif
         if ((pTuningDipReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Raw & DIPCTL_YUV_EN1_CNR_D1) == DIPCTL_YUV_EN1_CNR_D1)
         {
             dip_ctl_yuv_en |= (DIPCTL_YUV_EN1_SMT_D2|DIPCTL_YUV_EN1_UNP_D2|DIPCTL_YUV_EN1_PAK_D2);
             dip_ctl_dma_en |= (DIPCTL_DMA_EN1_SMTI_D2|DIPCTL_DMA_EN1_SMTO_D2);
         }
    }

    //Check HDR Type
    if (m_RawHDRType == NSCam::NSIoPipe::EDIPRawHDRType_SENSORHDR)
    {
        LBIT_MODE0 = 0;
        LBIT_MODE1 = 0;
    }
    else if (m_RawHDRType == NSCam::NSIoPipe::EDIPRawHDRType_ISPHDR)
    {
        LBIT_MODE0 = 1;
        LBIT_MODE1 = 0;
    }
    ///////////////////////////////////////////;//////////////////
    //information collection
    ispDipPipe.moduleIdx = DIP_HW_A; //TODO, need pass from iopipe
    PIPE_DBG("moduleIdx(%d),dip_ctl_dma_en(0x%x)",ispDipPipe.moduleIdx,dip_ctl_dma_en);
    if(tpipe == CAM_MODE_TPIPE) {
        ispDipPipe.isDipOnly = MFALSE;
        tdr_ctl |= DIPCTL_TDR_CTL_TDR_EN;
        tdr_sel |= DIPCTL_TDR_SEL_CTL_EXT_EN; //enable tpipe extension in default
    } else {
        ispDipPipe.isDipOnly = MTRUE;
        tdr_sel = 0xf0000; //TILE EDGE must be set to 1 in frame mode
    }
    //involve tuning setting
    if(pPipePackageInfo->pTuningQue != NULL){
        ispDipPipe.isApplyTuning = MTRUE;
        ispDipPipe.pTuningIspReg = (dip_x_reg_t *)pPipePackageInfo->pTuningQue;// check tuning enable bit on isp_function_dip
        rgb_en = ispDipPipe.pTuningIspReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Raw;
        rgb2_en = ispDipPipe.pTuningIspReg->DIPCTL_D1A_DIPCTL_RGB_EN2.Raw;
        yuv_en = ispDipPipe.pTuningIspReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Raw;
        yuv2_en = ispDipPipe.pTuningIspReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Raw;
        b12bitMode = ispDipPipe.pTuningIspReg->YNR_D1A_YNR_CON1.Bits.YNR_LP_MODE;//YWtodo
        nbc_gmap_ltm_mode = ispDipPipe.pTuningIspReg->DIPCTL_D1A_DIPCTL_MISC_SEL.Bits.DIPCTL_YNR_GMAP_LTM_MODE;
        //error handle
        //if(drvScen == eDrvScenario_Color_Effect)
        if(StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_COLOR_EFT)
        {
            if((yuv_en & 0x00000008) >> 3)
            {}
            else
            {
                PIPE_ERR("G2C should be enabled in color effect , tuning buffer yuven(0x%x)",yuv_en);
                return MFALSE;
            }
        }


        ispDipPipe.isp_top_ctl.YUV_EN.Raw = (dip_ctl_yuv_en &(~dip_yuv_tuning_tag))|(yuv_en & dip_yuv_tuning_tag);
        ispDipPipe.isp_top_ctl.YUV2_EN.Raw = (dip_ctl_yuv2_en &(~dip_yuv2_tuning_tag))|(yuv2_en & dip_yuv2_tuning_tag);
        ispDipPipe.isp_top_ctl.RGB_EN.Raw = (dip_ctl_rgb_en &(~dip_rgb_tuning_tag))|(rgb_en & dip_rgb_tuning_tag);
        ispDipPipe.isp_top_ctl.RGB2_EN.Raw = (dip_ctl_rgb2_en &(~dip_rgb2_tuning_tag))|(rgb2_en & dip_rgb2_tuning_tag);

        //update tcm
        tdr_tcm_en = ispDipPipe.isp_top_ctl.RGB_EN.Raw;
        tdr_tcm2_en = ispDipPipe.isp_top_ctl.RGB2_EN.Raw;
        tdr_tcm3_en = ispDipPipe.isp_top_ctl.YUV_EN.Raw;
        tdr_tcm4_en = ispDipPipe.isp_top_ctl.YUV2_EN.Raw;
        tdr_tcm5_en = dip_ctl_dma_en;
        tdr_tcm6_en = dip_ctl_dma2_en;

        PIPE_DBG("pIspPhyReg(0x%lx)",(long)ispDipPipe.pTuningIspReg);
    } else {
        ispDipPipe.isApplyTuning = MFALSE;

        ispDipPipe.isp_top_ctl.YUV_EN.Raw = dip_ctl_yuv_en;
        ispDipPipe.isp_top_ctl.YUV2_EN.Raw = dip_ctl_yuv2_en;
        ispDipPipe.isp_top_ctl.RGB_EN.Raw = dip_ctl_rgb_en;
        ispDipPipe.isp_top_ctl.RGB2_EN.Raw = dip_ctl_rgb2_en;

        tdr_tcm_en = ispDipPipe.isp_top_ctl.RGB_EN.Raw;
        tdr_tcm2_en = ispDipPipe.isp_top_ctl.RGB2_EN.Raw;
        tdr_tcm3_en = ispDipPipe.isp_top_ctl.YUV_EN.Raw;
        tdr_tcm4_en = ispDipPipe.isp_top_ctl.YUV2_EN.Raw;
        tdr_tcm5_en = dip_ctl_dma_en;
        tdr_tcm6_en = dip_ctl_dma2_en;

        PIPE_WRN("[Warning]p2 tuning not be passed via imageio");
    }

    //ispMdpPipe.drvScenario = drvScen;
    ispDipPipe.burstQueIdx = burstQIdx;
    ispDipPipe.RingBufIdx = m_pDipWorkingBuf->m_BufIdx;
    //ispMdpPipe.lastframe = lastFrame;
    //ispMdpPipe.isWaitBuf = isWaitBuf;
    ispDipPipe.dipTh = p2CQ;
    ispDipPipe.dip_cq_thr_ctl = dip_cq_thr_ctl;

    ispDipPipe.isp_top_ctl.DMA_EN.Raw = dip_ctl_dma_en;
    ispDipPipe.isp_top_ctl.DMA2_EN.Raw = dip_ctl_dma2_en;
    //ispDipPipe.isp_top_ctl.MISC_SEL.Raw = dip_misc_sel;
    //ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_PIX_ID = pPipePackageInfo->pixIdP2;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_PIX_ID = pixIdImgBuf;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_FG_MODE = fg_mode;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_SRAM_MODE = sram_mode;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_WUV_MODE = wuv_mode;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_YNR_GMAP_LTM_MODE = nbc_gmap_ltm_mode;
    //hw default 1, you need to modify it to zero when init and lce enable will the value chnage to 1, and lce disalbe chnage the value to 0
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_PAKG_D1_FG_OUT = fg_mode;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_HLR_D1_LKMSB = 11;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_IMGI_D1_SOF_GATE = 0;//YWtodo
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_LBIT_MODE0 = LBIT_MODE0;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_LBIT_MODE1 = LBIT_MODE1;
    ispDipPipe.isp_top_ctl.MISC_SEL.Bits.DIPCTL_APB_CLK_GATE_BYPASS = 0;

    ispDipPipe.isp_top_ctl.FMT_SEL.Raw = 0x0;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DIPCTL_IMGI_D1_FMT = imgi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DIPCTL_UFDI_D1_FMT = ufdi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DIPCTL_DMGI_D1_FMT = dmgi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DIPCTL_DEPI_D1_FMT = depi_fmt;
    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.DIPCTL_VIPI_D1_FMT = vipi_fmt;
    ispDipPipe.isp_top_ctl.FMT2_SEL.Bits.DIPCTL_TIMGO_D1_FMT = timgo_fmt;
    ispDipPipe.isp_top_ctl.FMT2_SEL.Bits.DIPCTL_DCESO_D1_FMT = dceso_fmt;
    ispDipPipe.isp_top_ctl.FMT2_SEL.Bits.DIPCTL_CRZO_D1_FMT = img2o_fmt;
    ispDipPipe.isp_top_ctl.FMT2_SEL.Bits.DIPCTL_IMG3O_D1_FMT = img3o_fmt;
//YWtodo    ispDipPipe.isp_top_ctl.FMT_SEL.Bits.YNR_LP_MODE = b12bitMode;
    ispDipPipe.isp_top_ctl.MUX_SEL.Raw = dip_ctl_path_sel;
    ispDipPipe.isp_top_ctl.MUX2_SEL.Raw = dip_ctl_path2_sel;
    ispDipPipe.isp_top_ctl.MUX_SEL.Bits.DIPCTL_CRP_D2_SEL = crp_d2_sel;
    ispDipPipe.isp_top_ctl.MUX_SEL.Bits.DIPCTL_TIMGO_D1_SEL = timgo_d1_sel;
    ispDipPipe.isp_top_ctl.MUX_SEL.Bits.DIPCTL_G2CX_D1_SEL = g2cx_d1_sel;
    ispDipPipe.isp_top_ctl.MUX_SEL.Bits.DIPCTL_FLC_D1_SEL = flc_d1_sel;
    ispDipPipe.isp_top_ctl.MUX_SEL.Bits.DIPCTL_TRAW_SEL = traw_sel;
    ispDipPipe.isp_top_ctl.MUX2_SEL.Bits.DIPCTL_FEO_D1_SEL = feo_d1_sel;
    //TODO items
    ispDipPipe.tdr_tcm_en = tdr_tcm_en;
    ispDipPipe.tdr_tcm2_en = tdr_tcm2_en;
    ispDipPipe.tdr_tcm3_en = tdr_tcm3_en;
    ispDipPipe.tdr_tcm4_en = tdr_tcm4_en;
    ispDipPipe.tdr_tcm5_en = tdr_tcm5_en;
    ispDipPipe.tdr_tcm6_en = tdr_tcm6_en;

    ispDipPipe.tdr_ctl = tdr_ctl;
    ispDipPipe.tdr_sel = tdr_sel;

    ispDipPipe.pUfdParam = pUfdParam;
    //in order to print the log, temp solution.
    if (m_Img3oCropInfo != NULL)
    {
        Img3oCrspTmp.m_CrspInfo.p_integral.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
        Img3oCrspTmp.m_CrspInfo.p_integral.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
    }

    if (pPipePackageInfo->pTuningQue != NULL)
    {
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),p2RingBufIdx(%d),[vipi]ofst(%d),rW/H(%d/%d)\n, \
            isDipOnly(%d), moduleIdx(%d), dipCQ/ring/burst(%d/%d/%d), drvSc(%d), isWtf(%d), en_rgb/rgb2/yuv/yuv2/dma/dma2(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), fmt_sel/fmt2_sel/mux_sel/mux2_sel/misc/v_flip(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), tcm(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x)\n , \
            last(%d),CRZ_EN(%d),MDP_CROP(%d,%d),img3o ofset(%d,%d),YNR_CON1(0x%x),YYNR_SKIN_CON(0x%x),userName(%s),PIX_ID(%d),ImgBuf_PIX_ID(%d),tv_sec(%lu),tv_usec(%lu),mfb_dl(%d)",\
            CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
            CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
            CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
            CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
            CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
            CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
            CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
            CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
            CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,pPipePackageInfo->p2RingBufIdx, \
            pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Raw,pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT, \
            ispDipPipe.isDipOnly,\
            ispDipPipe.moduleIdx,\
            ispDipPipe.dipTh,ispDipPipe.RingBufIdx,ispDipPipe.burstQueIdx,\
            StreamTag,isWaitBuf,\
            ispDipPipe.isp_top_ctl.RGB_EN.Raw, ispDipPipe.isp_top_ctl.RGB2_EN.Raw, ispDipPipe.isp_top_ctl.YUV_EN.Raw, ispDipPipe.isp_top_ctl.YUV2_EN.Raw, ispDipPipe.isp_top_ctl.DMA_EN.Raw, ispDipPipe.isp_top_ctl.DMA2_EN.Raw,\
            ispDipPipe.isp_top_ctl.FMT_SEL.Raw, ispDipPipe.isp_top_ctl.FMT2_SEL.Raw, ispDipPipe.isp_top_ctl.MUX_SEL.Raw, ispDipPipe.isp_top_ctl.MUX2_SEL.Raw, ispDipPipe.isp_top_ctl.MISC_SEL.Raw,dip_vertical_flip_en,\
            ispDipPipe.tdr_ctl, ispDipPipe.tdr_sel, ispDipPipe.tdr_tcm_en, ispDipPipe.tdr_tcm2_en, ispDipPipe.tdr_tcm3_en, ispDipPipe.tdr_tcm4_en, ispDipPipe.tdr_tcm5_en, ispDipPipe.tdr_tcm6_en,\
            lastFrame, m_CrzEn, WdmaoCropGroup, WrotoCropGroup, Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y,\
            pTuningDipReg->YNR_D1A_YNR_CON1.Raw, pTuningDipReg->YNR_D1A_YNR_SKIN_CON.Raw, szCallerName, pPipePackageInfo->pixIdP2, pixIdImgBuf,\
            pFrameParams.ExpectedEndTime.tv_sec, pFrameParams.ExpectedEndTime.tv_usec, m_isMfbDL);

    }
    else
    {
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),p2RingBufIdx(%d)\n, \
            isDipOnly(%d), moduleIdx(%d), dipCQ/ring/burst(%d/%d/%d), drvSc(%d), isWtf(%d), en_rgb/rgb2/yuv/yuv2/dma/dma2(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), fmt_sel/fmt2_sel/mux_sel/mux2_sel/misc/v_flip(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), tcm(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x)\n , \
            last(%d),CRZ_EN(%d),MDP_CROP(%d,%d),img3o ofset(%d,%d),userName(%s),PIX_ID(%d),ImgBuf_PIX_ID(%d),tv_sec(%lu),tv_usec(%lu),mfb_dl(%d)",\
            CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
            CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
            CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
            CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
            CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
            CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
            CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
            CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
            CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,pPipePackageInfo->p2RingBufIdx, \
            ispDipPipe.isDipOnly,\
            ispDipPipe.moduleIdx,\
            ispDipPipe.dipTh,ispDipPipe.RingBufIdx,ispDipPipe.burstQueIdx,\
            StreamTag,isWaitBuf,\
            ispDipPipe.isp_top_ctl.RGB_EN.Raw, ispDipPipe.isp_top_ctl.RGB2_EN.Raw, ispDipPipe.isp_top_ctl.YUV_EN.Raw, ispDipPipe.isp_top_ctl.YUV2_EN.Raw, ispDipPipe.isp_top_ctl.DMA_EN.Raw, ispDipPipe.isp_top_ctl.DMA2_EN.Raw,\
            ispDipPipe.isp_top_ctl.FMT_SEL.Raw, ispDipPipe.isp_top_ctl.FMT2_SEL.Raw, ispDipPipe.isp_top_ctl.MUX_SEL.Raw, ispDipPipe.isp_top_ctl.MUX2_SEL.Raw, ispDipPipe.isp_top_ctl.MISC_SEL.Raw,dip_vertical_flip_en,\
            ispDipPipe.tdr_ctl, ispDipPipe.tdr_sel, ispDipPipe.tdr_tcm_en, ispDipPipe.tdr_tcm2_en, ispDipPipe.tdr_tcm3_en, ispDipPipe.tdr_tcm4_en, ispDipPipe.tdr_tcm5_en, ispDipPipe.tdr_tcm6_en,\
            lastFrame, m_CrzEn, WdmaoCropGroup, WrotoCropGroup, Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y,\
            szCallerName, pPipePackageInfo->pixIdP2, pixIdImgBuf, pFrameParams.ExpectedEndTime.tv_sec, pFrameParams.ExpectedEndTime.tv_usec, m_isMfbDL);
    }

    /////////////////////////////////////////////////////////////
    //config ports
    if (-1 != idx_imgi)
    {
        this->configInDmaPort(portInfo_imgi, ispDipPipe.DMAImgi.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgi);

        PIPE_DBG("[imgi]size[%lu,%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAImgi.dma_cfg.size.w,ispDipPipe.DMAImgi.dma_cfg.size.h,ispDipPipe.DMAImgi.dma_cfg.size.xsize,
                ispDipPipe.DMAImgi.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgi.dma_cfg.memBuf.base_pAddr,\
                ispDipPipe.DMAImgi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgi.dma_cfg.size.stride);

        if ((m_imgi_planeNum >= 2) && (isUfoEn == MFALSE))
        {
            if (-1 != idx_imgbi)
            {    //multi-plane input image, imgi/imgbi/imgci
                this->configInDmaPort(portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)1, (MUINT32)0 , (MUINT32)1, ESTRIDE_2ND_PLANE, eImgFmt_imgbi);
                ispDipPipe.DMAImgbi.dma_cfg.size.w /= imgi_uv_h_ratio;
                ispDipPipe.DMAImgbi.dma_cfg.size.h /= imgi_uv_v_ratio;
                ispDipPipe.DMAImgbi.dma_cfg.size.xsize /= imgi_uv_h_ratio;
                PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                    ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                    (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                    ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);

                //TODO, imgbi would be weighting table in mixing path


                if (-1 != idx_imgci)
                {
                    this->configInDmaPort(portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE, eImgFmt_imgci);
                    ispDipPipe.DMAImgci.dma_cfg.size.w /= imgi_uv_h_ratio;
                    ispDipPipe.DMAImgci.dma_cfg.size.h /= imgi_uv_v_ratio;
                    ispDipPipe.DMAImgci.dma_cfg.size.xsize /= imgi_uv_h_ratio;
                    if(imgi_uv_swap)
                    {
                        //MUINTPTR tmp_base_vAddr = ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr;
                        //MUINTPTR tmp_base_pAddr = ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr;
                        //ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr = ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr;
                        //ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr;
                        //ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr = tmp_base_vAddr;
                        //ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr = tmp_base_pAddr;
                        doUVSwap(&ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr, &ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr);
                        doUVSwap(&ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr, &ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr);
                    }
                    PIPE_DBG("[imgc]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
                }
            }
            else
            {
                if (-1 != idx_imgci)
                {
                    //special case, imgci is weighting table
                    this->configInDmaPort(portInfo_imgci,ispDipPipe.DMAImgci.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_imgci);
                    //ispDipPipe.DMAImgci.dma_cfg.size.stride = portInfo_imgci.u4Stride[ESTRIDE_1ST_PLANE];
                }
            }
        }
        else
        {
            //if (eDrvScenario_MFB_Blending == drvScen)
            if (NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_MFB_Bld == StreamTag)
            {
                //
                //Blending
                if (-1 != idx_imgbi)
                {    //multi-plane input image, imgi/imgbi/imgci
                    this->configInDmaPort(portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)2, (MUINT32)0 , (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgbi);
                    PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                        ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                        (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                        ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);
                }


                if (-1 != idx_imgci)
                {
                    this->configInDmaPort(portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgci);
                    PIPE_DBG("[imgc]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",
                            ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                            ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                            ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
                }

                ispDipPipe.mfb_cfg.out_xofst=0;

                //TODO under check?
                if(idx_imgci != -1)
                {    //other blending sequences
                    ispDipPipe.mfb_cfg.bld_mode=1;
                }
                else
                {    //first blending
                    ispDipPipe.mfb_cfg.bld_mode=0;
                }
                //m_camPass2Param.mfb_cfg.bld_mode=0;
                ispDipPipe.mfb_cfg.bld_ll_db_en=0;
                ispDipPipe.mfb_cfg.bld_ll_brz_en=1;
            }
            else  //General Case, origian: only mixing mode. add pure raw case.
            {
                if (-1 != idx_imgbi)
                {
                    this->configInDmaPort(portInfo_imgbi, ispDipPipe.DMAImgbi.dma_cfg, (MUINT32)1, (MUINT32)0 , (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgbi);
                    PIPE_DBG("[imgbi]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                        ispDipPipe.DMAImgbi.dma_cfg.size.w,ispDipPipe.DMAImgbi.dma_cfg.size.h,ispDipPipe.DMAImgbi.dma_cfg.memBuf.size,
                        (unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgbi.dma_cfg.memBuf.base_pAddr,
                        ispDipPipe.DMAImgbi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgbi.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgbi.dma_cfg.size.stride);
                }

                if (-1 != idx_imgci)
                {
                    this->configInDmaPort(portInfo_imgci, ispDipPipe.DMAImgci.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_imgci);
                    PIPE_DBG("[imgci]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAImgci.dma_cfg.size.w,ispDipPipe.DMAImgci.dma_cfg.size.h,
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImgci.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAImgci.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImgci.dma_cfg.memBuf.alignment,ispDipPipe.DMAImgci.dma_cfg.size.stride,vipi_uv_swap);
                }
            }

        }


        if (MTRUE == m_CrzEn)
        {
            MBOOL crzResult = MTRUE;   // MTRUE: success. MFALSE: fail.

            ispDipPipe.crzPipe.conf_cdrz = MTRUE;;
            ispDipPipe.crzPipe.crz_out.w = CropInfo_imgi.resize1.tar_w;
            ispDipPipe.crzPipe.crz_out.h = CropInfo_imgi.resize1.tar_h;
            ispDipPipe.crzPipe.crz_in.w = ispDipPipe.DMAImgi.dma_cfg.size.w;
            ispDipPipe.crzPipe.crz_in.h = ispDipPipe.DMAImgi.dma_cfg.size.h;
            ispDipPipe.crzPipe.crz_crop.x = CropInfo_imgi.crop1.x;
            ispDipPipe.crzPipe.crz_crop.floatX = CropInfo_imgi.crop1.floatX;
            //ispDipPipe.crzPipe.crz_crop.floatX = ((CropInfo_imgi.crop1.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
            ispDipPipe.crzPipe.crz_crop.y = CropInfo_imgi.crop1.y;
            ispDipPipe.crzPipe.crz_crop.floatY = CropInfo_imgi.crop1.floatY;
            //ispDipPipe.crzPipe.crz_crop.floatY = ((CropInfo_imgi.crop1.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* 20 bits base (bit20 ~ bit27) */
            ispDipPipe.crzPipe.crz_crop.w = CropInfo_imgi.crop1.w;
            ispDipPipe.crzPipe.crz_crop.h = CropInfo_imgi.crop1.h;

            crzResult = ispDipPipe.CalAlgoAndCStep((CRZ_DRV_MODE_ENUM)ispDipPipe.crzPipe.tpipeMode,
                                                              CRZ_DRV_RZ_CRZ,
                                                              ispDipPipe.crzPipe.crz_in.w,
                                                              ispDipPipe.crzPipe.crz_in.h,
                                                              ispDipPipe.crzPipe.crz_crop.w,
                                                              ispDipPipe.crzPipe.crz_crop.h,
                                                              ispDipPipe.crzPipe.crz_out.w,
                                                              ispDipPipe.crzPipe.crz_out.h,
                                                              (CRZ_DRV_ALGO_ENUM*)&ispDipPipe.crzPipe.hAlgo,
                                                              (CRZ_DRV_ALGO_ENUM*)&ispDipPipe.crzPipe.vAlgo,
                                                              &ispDipPipe.crzPipe.hTable,
                                                              &ispDipPipe.crzPipe.vTable,
                                                              &ispDipPipe.crzPipe.hCoeffStep,
                                                              &ispDipPipe.crzPipe.vCoeffStep);
            if(!crzResult)
            {
                PIPE_ERR(" crzPipe.CalAlgoAndCStep error");
                return MFALSE;
            }
        }
        else
        {
            ispDipPipe.crzPipe.conf_cdrz = MFALSE;
            ispDipPipe.crzPipe.crz_out.w = 0;
            ispDipPipe.crzPipe.crz_out.h = 0;
            ispDipPipe.crzPipe.crz_in.w = 0;
            ispDipPipe.crzPipe.crz_in.h = 0;
            ispDipPipe.crzPipe.crz_crop.x = 0;
            ispDipPipe.crzPipe.crz_crop.floatX = 0;
            ispDipPipe.crzPipe.crz_crop.y = 0;
            ispDipPipe.crzPipe.crz_crop.floatY = 0;
            ispDipPipe.crzPipe.crz_crop.w = 0;
            ispDipPipe.crzPipe.crz_crop.h = 0;
        }
    }

    //if ((-1 != idx_regi) && (drvScen == eDrvScenario_FM))
    if ((-1 != idx_regi) && (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM))
    {
        ispDipPipe.regCount = 0x1;
        ispDipPipe.pReadAddrList = &addrList;
        ispDipPipe.pRegiAddr  = portInfo_regi->mBuffer->getBufVA(ESTRIDE_1ST_PLANE);
    }
    else
    {
        ispDipPipe.regCount = 0x0;
        ispDipPipe.pReadAddrList = &addrList;
        ispDipPipe.pRegiAddr  = 0x0;
    }

    //
    if (-1 != idx_vipi)
    {
        this->configInDmaPort(portInfo_vipi, ispDipPipe.DMAVipi.dma_cfg, (MUINT32)1, (MUINT32)1, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_vipi);

        PIPE_DBG("[vipi]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAVipi.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAVipi.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr,\
                ispDipPipe.DMAVipi.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVipi.dma_cfg.memBuf.alignment,ispDipPipe.DMAVipi.dma_cfg.size.stride);

        if (-1 != idx_vip2i)
        {    //multi-plane input image, vipi/vip2i/vip3i
            this->configInDmaPort(portInfo_vip2i, ispDipPipe.DMAVip2i.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE, eImgFmt_vip2i);
            ispDipPipe.DMAVip2i.dma_cfg.size.w /= vipi_uv_h_ratio;
            ispDipPipe.DMAVip2i.dma_cfg.size.h /= vipi_uv_v_ratio;
            ispDipPipe.DMAVip2i.dma_cfg.size.xsize /= vipi_uv_h_ratio;
            PIPE_DBG("[vip2i]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAVip2i.dma_cfg.size.w,ispDipPipe.DMAVip2i.dma_cfg.size.h,
                    ispDipPipe.DMAVip2i.dma_cfg.memBuf.size,ispDipPipe.DMAVip2i.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVip2i.dma_cfg.memBuf.alignment,ispDipPipe.DMAVip2i.dma_cfg.size.stride);

            if (-1 != idx_vip3i)
            {
                this->configInDmaPort(portInfo_vip3i, ispDipPipe.DMAVip3i.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE, eImgFmt_vip3i);
                ispDipPipe.DMAVip3i.dma_cfg.size.w /= vipi_uv_h_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.h /= vipi_uv_v_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.xsize /= vipi_uv_h_ratio;
                if(vipi_uv_swap) {
                    //ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_2ND_PLANE];
                    //ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE];
                    doUVSwap(&ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_vAddr, &ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr);
                    doUVSwap(&ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr, &ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr);
                }
                PIPE_DBG("[vip3i]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),vipi_uv_swap(%d)",ispDipPipe.DMAVip3i.dma_cfg.size.w,ispDipPipe.DMAVip3i.dma_cfg.size.h,
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr,\
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAVip3i.dma_cfg.memBuf.alignment,ispDipPipe.DMAVip3i.dma_cfg.size.stride,vipi_uv_swap);
            }
        }
        // for NR3D set vipi information +++++
        if (pPipePackageInfo->pTuningQue != NULL)
        {
            if (pTuningDipReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_NR3D_D1_EN & pTuningDipReg->NR3D_D1A_NR3D_NR3D_TNR_ENABLE.Bits.NR3D_tnr_c_en)//YWtodo
            {
                switch(eImgFmt_vipi)
                {
                    case eImgFmt_YUY2:
                        pixelInByte=2;
                        ispDipPipe.DMAVipi.dma_cfg.offset.x = (pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Raw % ispDipPipe.DMAVipi.dma_cfg.size.stride)/pixelInByte;
                        ispDipPipe.DMAVipi.dma_cfg.offset.y = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Raw / ispDipPipe.DMAVipi.dma_cfg.size.stride;
                        ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVipi.dma_cfg.offset.y * ispDipPipe.DMAVipi.dma_cfg.size.stride + ispDipPipe.DMAVipi.dma_cfg.offset.x * pixelInByte);
                        break;
                    case eImgFmt_YV12:
                    default:
                        pixelInByte=1;
                        ispDipPipe.DMAVipi.dma_cfg.offset.x = (pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Raw % ispDipPipe.DMAVipi.dma_cfg.size.stride)/pixelInByte;
                        ispDipPipe.DMAVipi.dma_cfg.offset.y = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Raw / ispDipPipe.DMAVipi.dma_cfg.size.stride;
                        ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVipi.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVipi.dma_cfg.offset.y * ispDipPipe.DMAVipi.dma_cfg.size.stride + ispDipPipe.DMAVipi.dma_cfg.offset.x * pixelInByte);
                        ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVip2i.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVip2i.dma_cfg.offset.y/2 * ispDipPipe.DMAVip2i.dma_cfg.size.stride + ispDipPipe.DMAVip2i.dma_cfg.offset.x/2 * pixelInByte);
                        ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr = ispDipPipe.DMAVip3i.dma_cfg.memBuf.base_pAddr +
                            (ispDipPipe.DMAVip3i.dma_cfg.offset.y/2 * ispDipPipe.DMAVip3i.dma_cfg.size.stride + ispDipPipe.DMAVip3i.dma_cfg.offset.x/2 * pixelInByte);

                        //portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] +
                        //    (portInfo_vip2i.yoffset * portInfo_vip2i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip2i.xoffset * pixelInByte);
                        //portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] +
                        //    (portInfo_vip2i.yoffset/2 * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip2i.xoffset/2 * pixelInByte);
                        //portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] +
                        //    (portInfo_vip2i.yoffset/2 * portInfo_vip2i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip2i.xoffset/2 * pixelInByte);

                        //portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] +
                        //    (portInfo_vip3i.yoffset * portInfo_vip3i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip3i.xoffset * pixelInByte);
                        //portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] +
                        //    (portInfo_vip3i.yoffset/2 * portInfo_vip3i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip3i.xoffset/2 * pixelInByte);
                        //portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] +
                        //    (portInfo_vip3i.yoffset/2 * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip3i.xoffset/2 * pixelInByte);
                        break;
                }
                ispDipPipe.DMAVipi.dma_cfg.size.w = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                ispDipPipe.DMAVipi.dma_cfg.size.h = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                ispDipPipe.DMAVip2i.dma_cfg.size.w = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                ispDipPipe.DMAVip2i.dma_cfg.size.h = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                ispDipPipe.DMAVip3i.dma_cfg.size.w = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH;
                ispDipPipe.DMAVip3i.dma_cfg.size.h = pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT;
                ispDipPipe.DMAVip2i.dma_cfg.size.w /= vipi_uv_h_ratio;
                ispDipPipe.DMAVip2i.dma_cfg.size.h /= vipi_uv_v_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.w /= vipi_uv_h_ratio;
                ispDipPipe.DMAVip3i.dma_cfg.size.h /= vipi_uv_v_ratio;
                PIPE_DBG("[vipi]ofst(%d),rW/H(%d/%d)",pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Raw,pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT);
            }
        }
        // for NR3D set vipi information -----
    }

    //
    if (-1 != idx_ufdi) {
        PIPE_DBG("config ufdi");
        if ((eImgFmt_ufdi == eImgFmt_UFO_FG_BAYER10) || (eImgFmt_ufdi == eImgFmt_UFO_FG_BAYER12) || (eImgFmt_ufdi == eImgFmt_UFO_BAYER10) || (eImgFmt_ufdi == eImgFmt_UFO_BAYER12))
        {
            this->configInDmaPort(portInfo_ufdi,ispDipPipe.DMAUfdi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_2ND_PLANE, eImgFmt_ufdi);
            //ispDipPipe.DMAUfdi.dma_cfg.size.stride = portInfo_ufdi.u4Stride[ESTRIDE_2ND_PLANE];
            ispDipPipe.DMAUfdi.dma_cfg.size.xsize = (((ispDipPipe.DMAUfdi.dma_cfg.size.w+63)/64)+7)/8*8;
        }
        else
        {
            this->configInDmaPort(portInfo_ufdi,ispDipPipe.DMAUfdi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_ufdi);
            //ispDipPipe.DMAUfdi.dma_cfg.size.stride = portInfo_ufdi.u4Stride[ESTRIDE_1ST_PLANE];
        }
    }
    //
    if (-1 != idx_dmgi) {
        //PIPE_DBG("config dmgi");
        this->configInDmaPort(portInfo_dmgi,ispDipPipe.DMADmgi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_dmgi);
        //ispDipPipe.DMADmgi.dma_cfg.size.stride = portInfo_dmgi.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_depi) {
        //PIPE_DBG("config ufdi");
        //if((drvScen == eDrvScenario_LPCNR_Pass1) || (drvScen == eDrvScenario_LPCNR_Pass2)) {
        if((StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass1) || (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2)) {
            if ((eImgFmt_depi == eImgFmt_NV12) || (eImgFmt_depi == eImgFmt_NV21)) {
                eImgFmt_depi = eImgFmt_STA_BYTE;
            } else {
                eImgFmt_depi = eImgFmt_STA_10BIT;
            }
            this->configInDmaPort(portInfo_depi,ispDipPipe.DMADepi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_2ND_PLANE, eImgFmt_depi);
            ispDipPipe.DMADepi.dma_cfg.size.h /= 2;
            //if (drvScen == eDrvScenario_LPCNR_Pass2) {
            if (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2) {
                this->configInDmaPort(portInfo_depi,ispDipPipe.DMADmgi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_depi);
                ispDipPipe.DMADepi.dma_cfg.v_flip_en = MTRUE;
                ispDipPipe.DMADmgi.dma_cfg.v_flip_en = MTRUE;
            }
        } else {
            this->configInDmaPort(portInfo_depi,ispDipPipe.DMADepi.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_depi);
        }
        //ispDipPipe.DMADepi.dma_cfg.size.stride = portInfo_depi.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_lcei)
    {
        //PIPE_DBG("config lcei");
        this->configInDmaPort(portInfo_lcei,ispDipPipe.DMALcei.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE, eImgFmt_lcei);
        //if(drvScen == eDrvScenario_LPCNR_Pass2) {
        if (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2) {
            ispDipPipe.DMALcei.dma_cfg.v_flip_en = MTRUE;
        }
        //ispDipPipe.DMALcei.dma_cfg.size.stride = portInfo_lcei.u4Stride[ESTRIDE_1ST_PLANE];
    }

    //
    if (-1 != idx_wroto) {
        //PIPE_DBG("config wroto");
        //support different view angle
        pMdpCfg->dstPortCfg[ISP_MDP_DL_WROTO] = 1;
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].enSrcCrop = true;
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropX = CropInfo_imgi.crop3.x;
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropFloatX = CropInfo_imgi.crop3.floatX;
        //pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropFloatX = \
        //    ((CropInfo_imgi.crop3.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropY = CropInfo_imgi.crop3.y;
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropFloatY = CropInfo_imgi.crop3.floatY;
        //pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropFloatY =  \
        //    ((CropInfo_imgi.crop3.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropW = CropInfo_imgi.crop3.w;
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropFloatW = CropInfo_imgi.crop3.floatW;
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropH = CropInfo_imgi.crop3.h;
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].srcCropFloatH = CropInfo_imgi.crop3.floatH;
        //
        pMdpCfg->dstDma[ISP_MDP_DL_WROTO].crop_group = WrotoCropGroup;
        this->configMdpOutPort(portInfo_wroto,pMdpCfg->dstDma[ISP_MDP_DL_WROTO],eImgFmt_wroto);
        isSetMdpOut = MTRUE;
    }
    if (-1 != idx_wdmao) {
        //PIPE_DBG("config wdmao");
        //
        // for digital zoom crop
        pMdpCfg->dstPortCfg[ISP_MDP_DL_WDMAO] = 1;
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].enSrcCrop = true;
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropX = CropInfo_imgi.crop2.x;
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropFloatX = CropInfo_imgi.crop2.floatX;
        //pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropFloatX = \
        //        ((CropInfo_imgi.crop2.floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropY = CropInfo_imgi.crop2.y;
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropFloatY = CropInfo_imgi.crop2.floatY;
        //pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropFloatY =  \
        //    ((CropInfo_imgi.crop2.floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropW = CropInfo_imgi.crop2.w;
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropFloatW = CropInfo_imgi.crop2.floatW;
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropH = CropInfo_imgi.crop2.h;
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].srcCropFloatH = CropInfo_imgi.crop2.floatH;
        //
        pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].crop_group = WdmaoCropGroup;
        this->configMdpOutPort(portInfo_wdmao,pMdpCfg->dstDma[ISP_MDP_DL_WDMAO],eImgFmt_wdmao);
        isSetMdpOut = MTRUE;
    }
    if (-1 != idx_jpego) {
        //PIPE_DBG("config jpego");
        //
        //this->configMdpOutPort(vOutPorts[idx_jpego],m_camPass2Param.jpego);
        isSetMdpOut = MTRUE;
    }
    if (-1 != idx_venco) {
        //PIPE_DBG("config venco");
        // for digital zoom crop
        pMdpCfg->dstPortCfg[ISP_MDP_DL_VENCO] = 1;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].enSrcCrop = true;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropX = CropInfo_imgi.crop2.x;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropFloatX = CropInfo_imgi.crop2.floatX;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropY = CropInfo_imgi.crop2.y;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropFloatY =  CropInfo_imgi.crop2.floatY;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropW = CropInfo_imgi.crop2.w;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropFloatW = CropInfo_imgi.crop2.floatW;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropH = CropInfo_imgi.crop2.h;
        pMdpCfg->dstDma[ISP_MDP_DL_VENCO].srcCropFloatH = CropInfo_imgi.crop2.floatH;
        //
        this->configMdpOutPort(portInfo_venco,pMdpCfg->dstDma[ISP_MDP_DL_VENCO],eImgFmt_venco);
        isSetMdpOut = MTRUE;
        //we do not support wdmao if user enque venco
        if(-1 != idx_wdmao)
        {
            PIPE_ERR("we do not support wdmao if user enque venco (%d/%d)",idx_wdmao,idx_venco);
            return MFALSE;
        }

    }
    //
    if (-1 != idx_img2o)
    {
        //PIPE_DBG("img2o u4BufSize(0x%x)-u4BufVA(0x%lx)-u4BufPA(0x%lx)", vOutPorts[idx_img2o]->u4BufSize[0],(unsigned long)vOutPorts[idx_img2o]->u4BufVA[0],(unsigned long)vOutPorts[idx_img2o]->u4BufPA[0]);
        this->configOutDmaPort(portInfo_img2o,ispDipPipe.DMAImg2o.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE,eImgFmt_img2o);
        //
        //ispDipPipe.DMAImg2o.dma_cfg.size.stride = portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];  // for tpipe
        PIPE_DBG("[img2o]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg2o.dma_cfg.size.w,ispDipPipe.DMAImg2o.dma_cfg.size.h,
                ispDipPipe.DMAImg2o.dma_cfg.memBuf.size,ispDipPipe.DMAImg2o.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg2o.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg2o.dma_cfg.size.stride);

        if (-1 != idx_img2bo)
        {    //multi-plane input image, img2o/img2bo
            this->configOutDmaPort(portInfo_img2bo, ispDipPipe.DMAImg2bo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE,eImgFmt_img2bo);
            ispDipPipe.DMAImg2bo.dma_cfg.size.w /=img2o_uv_h_ratio;
            ispDipPipe.DMAImg2bo.dma_cfg.size.h /= img2o_uv_v_ratio;
            ispDipPipe.DMAImg2bo.dma_cfg.size.xsize /= img2o_uv_h_ratio;
            PIPE_DBG("[img2bo]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg2bo.dma_cfg.size.w,ispDipPipe.DMAImg2bo.dma_cfg.size.h,
                    ispDipPipe.DMAImg2bo.dma_cfg.memBuf.size,ispDipPipe.DMAImg2bo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg2bo.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg2bo.dma_cfg.size.stride);
        }

        if(isSetMdpOut == MFALSE)
        {
            pMdpCfg->dstPortCfg[ISP_ONLY_OUT_TPIPE] = 1;
            this->configMdpOutPort(portInfo_img2o,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE],eImgFmt_img2o);
            mdp_imgxo_p2_en = DIPCTL_DMA_EN1_CRZO_D1;
            isSetMdpOut = MTRUE;
        }
    }
    //
    if (-1 != idx_img3o)
    {
        this->configOutDmaPort(portInfo_img3o, ispDipPipe.DMAImg3o.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_img3o);

        if(isSetMdpOut == MFALSE){
            pMdpCfg->dstPortCfg[ISP_ONLY_OUT_TPIPE] = 1;
            this->configMdpOutPort(portInfo_img3o,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE],eImgFmt_img3o);
            mdp_imgxo_p2_en = DIPCTL_DMA_EN2_IMG3O_D1;
            isSetMdpOut = MTRUE;
        }
        PIPE_DBG("[img3o]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx)",
                ispDipPipe.DMAImg3o.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImg3o.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImg3o.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAImg3o.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3o.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3o.dma_cfg.size.stride);

        if (-1 != idx_img3bo)
        {
            this->configOutDmaPort(portInfo_img3bo, ispDipPipe.DMAImg3bo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_2ND_PLANE, eImgFmt_img3bo);
            ispDipPipe.DMAImg3bo.dma_cfg.size.w /= img3o_uv_h_ratio;
            ispDipPipe.DMAImg3bo.dma_cfg.size.h /= img3o_uv_v_ratio;
            ispDipPipe.DMAImg3bo.dma_cfg.size.xsize /= img3o_uv_h_ratio;
            PIPE_DBG("[img3bo]size[%lu,%lu](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%lx)",ispDipPipe.DMAImg3bo.dma_cfg.size.w,ispDipPipe.DMAImg3bo.dma_cfg.size.h,
                    ispDipPipe.DMAImg3bo.dma_cfg.memBuf.size,ispDipPipe.DMAImg3bo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3bo.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3bo.dma_cfg.size.stride);

            if (-1 != idx_img3co)
            {
                this->configOutDmaPort(portInfo_img3co, ispDipPipe.DMAImg3co.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_3RD_PLANE, eImgFmt_img3co);
                ispDipPipe.DMAImg3co.dma_cfg.size.w /= img3o_uv_h_ratio;
                ispDipPipe.DMAImg3co.dma_cfg.size.h /= img3o_uv_v_ratio;
                ispDipPipe.DMAImg3co.dma_cfg.size.xsize /= img3o_uv_h_ratio;
                if(img3o_uv_swap) {
                    //ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_3RD_PLANE];
                    //ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_2ND_PLANE];
                    //ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_2ND_PLANE];
                    doUVSwap(&ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_vAddr, &ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr);
                    doUVSwap(&ispDipPipe.DMAImg3bo.dma_cfg.memBuf.base_pAddr, &ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr);
                }
                PIPE_DBG("[img3co]size[%lu,%lu](0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x),stride(0x%lx),img3o_uv_swap(%d)",ispDipPipe.DMAImg3co.dma_cfg.size.w,ispDipPipe.DMAImg3co.dma_cfg.size.h,
                        ispDipPipe.DMAImg3co.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAImg3co.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAImg3co.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAImg3co.dma_cfg.memBuf.alignment,ispDipPipe.DMAImg3co.dma_cfg.size.stride,img3o_uv_swap);
            }
        }
        if (m_Img3oCropInfo != NULL)
        {
            //support different view angle
            ispDipPipe.DMAImg3o.dma_cfg.offset.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x; //img3o offset must be even
            ispDipPipe.DMAImg3o.dma_cfg.offset.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            ispDipPipe.DMAImg3bo.dma_cfg.offset.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
            ispDipPipe.DMAImg3bo.dma_cfg.offset.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            ispDipPipe.DMAImg3co.dma_cfg.offset.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
            ispDipPipe.DMAImg3co.dma_cfg.offset.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
            PIPE_DBG("IMG3O cpoft(%d/%d)",ispDipPipe.DMAImg3o.dma_cfg.offset.x,ispDipPipe.DMAImg3o.dma_cfg.offset.y);
        }
        else
        {
            //support different view angle
            ispDipPipe.DMAImg3o.dma_cfg.offset.x = ispDipPipe.DMAImg3o.dma_cfg.offset.x & (~1); //img3o offset must be even
            ispDipPipe.DMAImg3o.dma_cfg.offset.y = ispDipPipe.DMAImg3o.dma_cfg.offset.y & (~1);
            ispDipPipe.DMAImg3bo.dma_cfg.offset.x = ispDipPipe.DMAImg3o.dma_cfg.offset.x;
            ispDipPipe.DMAImg3bo.dma_cfg.offset.y = ispDipPipe.DMAImg3o.dma_cfg.offset.y;
            ispDipPipe.DMAImg3co.dma_cfg.offset.x = ispDipPipe.DMAImg3o.dma_cfg.offset.x;
            ispDipPipe.DMAImg3co.dma_cfg.offset.y = ispDipPipe.DMAImg3o.dma_cfg.offset.y;
            PIPE_DBG("IMG3O cpoft(%d/%d)",ispDipPipe.DMAImg3o.dma_cfg.offset.x,ispDipPipe.DMAImg3o.dma_cfg.offset.y);
        }

    }
    //
    if (-1 != idx_timgo)
    {
        //if(drvScen == eDrvScenario_LPCNR_Pass2) {
        if(StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2) {
            if ((eImgFmt_timgo == eImgFmt_NV12) || (eImgFmt_timgo == eImgFmt_NV21)) {
                eImgFmt_timgo = eImgFmt_STA_BYTE;
            } else {
                eImgFmt_timgo = eImgFmt_STA_10BIT;
            }
            this->configOutDmaPort(portInfo_timgo,ispDipPipe.DMATimgo.dma_cfg,(MUINT32)1,(MUINT32)0,(MUINT32)1, ESTRIDE_2ND_PLANE, eImgFmt_timgo);
            ispDipPipe.DMATimgo.dma_cfg.size.h /= 2;
            ispDipPipe.DMATimgo.dma_cfg.v_flip_en = MTRUE;
        } else {
            this->configOutDmaPort(portInfo_timgo, ispDipPipe.DMATimgo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_timgo);
        }
        if(isSetMdpOut == MFALSE){
            pMdpCfg->dstPortCfg[ISP_ONLY_OUT_TPIPE] = 1;
            this->configMdpOutPort(portInfo_timgo,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE],eImgFmt_timgo);
            mdp_imgxo_p2_en = DIPCTL_DMA_EN1_TIMGO_D1;
            isSetMdpOut = MTRUE;
        }
    }
    //
    if (-1 != idx_dceso)
    {
        this->configOutDmaPort(portInfo_dceso, ispDipPipe.DMADceso.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_dceso);
        //if(isSetMdpOut == MFALSE){
        //      this->configMdpOutPort(vOutPorts[idx_timgo],ispMdpPipe.imgxo_out);
        //      mdp_imgxo_p2_en = DIPCTL_DMA_EN1_TIMGO_D1;
        //      isSetMdpOut = MTRUE;
        //}
    }

    if (-1 != idx_feo)
    {
        //PIPE_DBG("feo u4BufSize(0x%x)-u4BufVA(0x%lx)-u4BufPA(0x%lx)", vOutPorts[idx_feo]->u4BufSize[0],(unsigned long)vOutPorts[idx_feo]->u4BufVA[0],(unsigned long)vOutPorts[idx_feo]->u4BufPA[0]);
        this->configOutDmaPort(portInfo_feo, ispDipPipe.DMAFeo.dma_cfg, (MUINT32)1, (MUINT32)0, (MUINT32)1, ESTRIDE_1ST_PLANE, eImgFmt_feo);

        PIPE_DBG("[feo]size(0x%08x),Va(0x%lx),Pa(0x%lx),offset(0x%08x),align(0x%08x)",
            ispDipPipe.DMAFeo.dma_cfg.memBuf.size,(unsigned long)ispDipPipe.DMAFeo.dma_cfg.memBuf.base_vAddr,(unsigned long)ispDipPipe.DMAFeo.dma_cfg.memBuf.base_pAddr,ispDipPipe.DMAFeo.dma_cfg.memBuf.ofst_addr,ispDipPipe.DMAFeo.dma_cfg.memBuf.alignment);

         if(isSetMdpOut == MFALSE)
         {
             pMdpCfg->dstPortCfg[ISP_ONLY_OUT_TPIPE] = 1;
             this->configMdpOutPort(portInfo_feo, pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE], eImgFmt_feo);
             mdp_imgxo_p2_en = DIPCTL_DMA_EN1_FEO_D1;
             isSetMdpOut = MTRUE;
         }

    }
    //ispMdpPipe.mdp_imgxo_p2_en = mdp_imgxo_p2_en;

    //MDP Part
    //pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x0; <==update it in isp_function_dip.cpp
    //if(ispDipPipe.isDipOnly == MFALSE) {
    //    pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag = 0x0;
    //}
    pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx = (E_ISP_DIP_CQ)p2CQ;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx = burstQIdx;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.dupCqIdx = 0;
    pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx = m_pDipWorkingBuf->m_BufIdx;

    pMdpCfg->drvScenario = StreamTag;  //kk test
    pMdpCfg->isWaitBuf = isWaitBuf;
    pMdpCfg->imgxoEnP2 = mdp_imgxo_p2_en;
    pMdpCfg->pDumpTuningData = (unsigned int *)ispDipPipe.pTuningIspReg; //QQ
    pMdpCfg->ispDmaEn = ispDipPipe.isp_top_ctl.DMA_EN.Raw;
    pMdpCfg->ispDma2En = ispDipPipe.isp_top_ctl.DMA2_EN.Raw;
    //pMdpCfg->dipCtlDmaEn = ispDipPipe.isp_top_ctl.DMA_EN.Raw;
     pMdpCfg->m_iDipMetEn = this->m_pIspDrvShell->m_iDipMetEn;

    //PmQos
    //if (drvScen == eDrvScenario_FM)
    if (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM)
    {
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_in_w = ispDipPipe.DMADmgi.dma_cfg.size.w;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_h = ispDipPipe.DMADmgi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.total_data_size = (ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h)+\
        (ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h)+(ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h);
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dmgi_data_size = ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.depi_data_size = ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.lcei_data_size = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.timgo_data_size = ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h;
    }
    //else if (drvScen == eDrvScenario_LPCNR_Pass1)
    else if (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass1)
    {
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_in_w = ispDipPipe.DMADmgi.dma_cfg.size.w;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_h = ispDipPipe.DMADmgi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.total_data_size = (ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h)+\
        (ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h)+(ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h)+\
        (ispDipPipe.DMALcei.dma_cfg.size.xsize*ispDipPipe.DMALcei.dma_cfg.size.h);
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dmgi_data_size = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.depi_data_size = ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.lcei_data_size = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.timgo_data_size = ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h;

    }
    //else if (drvScen == eDrvScenario_LPCNR_Pass2)
    else if (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2)
    {
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_in_w = ispDipPipe.DMADmgi.dma_cfg.size.w;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_h = ispDipPipe.DMADmgi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.total_data_size = (ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h)+\
        (ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h)+(ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h)+\
        (ispDipPipe.DMALcei.dma_cfg.size.xsize*ispDipPipe.DMALcei.dma_cfg.size.h);
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dmgi_data_size = ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.depi_data_size = ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.lcei_data_size = ispDipPipe.DMALcei.dma_cfg.size.xsize*ispDipPipe.DMALcei.dma_cfg.size.h;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.timgo_data_size = ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h;
    }
    else
    {
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_in_w = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_total_out_w = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.framemode_h = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.total_data_size = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.dmgi_data_size = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.depi_data_size = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.lcei_data_size = 0;
        pMdpCfg->ispTpipeCfgInfo.drvinfo.timgo_data_size = 0;
    }

    // collect src image info for MdpMgr
    pMdpCfg->srcVirAddr  = 0xffff/*(this->dma_cfg.memBuf.base_vAddr + this->dma_cfg.memBuf.ofst_addr)*/; //kk test
    if (m_isMfbDL == 0) {
        pMdpCfg->srcPhyAddr  = (ispDipPipe.DMAImgi.dma_cfg.memBuf.base_pAddr + ispDipPipe.DMAImgi.dma_cfg.memBuf.ofst_addr);
    }

    //check mdp src crop information
    mdpCheck1 = 0;
    mdpCheck2 = 0;
    mdpCheck3 = 0;
    crop1TableIdx = -1;
    tableSize = sizeof(mCropPathInfo)/sizeof(Scen_Map_CropPathInfo_STRUCT);
    for(i=0;i<tableSize;i++) {
        if(mCropPathInfo[i].u4DrvScenId==StreamTag && mCropPathInfo[i].u4CropGroup==ECropGroupIndex_1){
            mdpCheck1 = 1;
            crop1TableIdx = i;
        }
        if(mCropPathInfo[i].u4DrvScenId==StreamTag && mCropPathInfo[i].u4CropGroup==(ECropGroupIndex_1|ECropGroupIndex_2)){
            mdpCheck2 = 1;
        }

        if(mCropPathInfo[i].u4DrvScenId==StreamTag && mCropPathInfo[i].u4CropGroup==(ECropGroupIndex_1|ECropGroupIndex_3)){
            mdpCheck3 = 1;
        }
    }
    PIPE_DBG("StreamTag(%d),mdpCheck1(%d),mdpCheck2(%d),mdpCheck3(%d),crop1TableIdx(%d)",StreamTag,mdpCheck1,mdpCheck2,mdpCheck3,crop1TableIdx);

    if((mdpCheck1 && mdpCheck2) || (mdpCheck1 && mdpCheck3)){
        MBOOL isMdpSrcCropBeSet;
        //
        isMdpSrcCropBeSet = MFALSE;
        for(i=0;i<DMA_OUT_PORT_NUM;i++){
            switch(mCropPathInfo[crop1TableIdx].u4PortID[i]){
                case EPortIndex_IMG2O:
                    if(dip_ctl_dma_en & DIPCTL_DMA_EN1_CRZO_D1){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        pMdpCfg->mdpSrcW = ispDipPipe.DMAImg2o.dma_cfg.size.w;
                        pMdpCfg->mdpSrcH = ispDipPipe.DMAImg2o.dma_cfg.size.h;
                        pMdpCfg->mdpSrcFmt = eImgFmt_img2o;
                        switch(eImgFmt_img2o){
                            case eImgFmt_NV61:
                            case eImgFmt_NV16:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAImg2bo.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImg2o.dma_cfg.size.h * ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImg2bo.dma_cfg.size.h * ispDipPipe.DMAImg2bo.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcVBufSize = 0;
                                pMdpCfg->mdpSrcPlaneNum = 2;
                                break;
                            default:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = 0;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImg2o.dma_cfg.size.h * ispDipPipe.DMAImg2o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImg2bo.dma_cfg.size.h * ispDipPipe.DMAImg2bo.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcVBufSize = 0;
                                pMdpCfg->mdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                case EPortIndex_IMG3O:
                    if(dip_ctl_dma2_en & DIPCTL_DMA_EN2_IMG3O_D1){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        pMdpCfg->mdpSrcW = ispDipPipe.DMAImg3o.dma_cfg.size.w;
                        pMdpCfg->mdpSrcH = ispDipPipe.DMAImg3o.dma_cfg.size.h;
                        pMdpCfg->mdpSrcFmt = eImgFmt_img3o;
                        switch(eImgFmt_img3o){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                            case eImgFmt_MTK_YVU_P010:
                            case eImgFmt_MTK_YUV_P010:
                            case eImgFmt_YVU_P010:
                            case eImgFmt_YUV_P010:
                            case eImgFmt_NV61:
                            case eImgFmt_NV16:
                            case eImgFmt_MTK_YVU_P210:
                            case eImgFmt_MTK_YUV_P210:
                            case eImgFmt_YVU_P210:
                            case eImgFmt_YUV_P210:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImg3o.dma_cfg.size.h * ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImg3bo.dma_cfg.size.h * ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImg3co.dma_cfg.size.h * ispDipPipe.DMAImg3co.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcPlaneNum = 2;
                                break;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                            case eImgFmt_MTK_YUV_P010_3PLANE:
                            case eImgFmt_YUV_P010_3PLANE:
                            case eImgFmt_YV16:
                            case eImgFmt_I422:
                            case eImgFmt_MTK_YUV_P210_3PLANE:
                            case eImgFmt_YUV_P210_3PLANE:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImg3o.dma_cfg.size.h * ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImg3bo.dma_cfg.size.h * ispDipPipe.DMAImg3bo.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImg3co.dma_cfg.size.h * ispDipPipe.DMAImg3co.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcPlaneNum = 3;
                                break;
                            default:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = 0;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImg3o.dma_cfg.size.h * ispDipPipe.DMAImg3o.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = 0;
                                pMdpCfg->mdpSrcVBufSize = 0;
                                pMdpCfg->mdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                case EPortIndex_FEO:
                    if(dip_ctl_dma_en & DIPCTL_DMA_EN1_FEO_D1){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        pMdpCfg->mdpSrcW = ispDipPipe.DMAFeo.dma_cfg.size.w;
                        pMdpCfg->mdpSrcH = ispDipPipe.DMAFeo.dma_cfg.size.h;
                        pMdpCfg->mdpSrcYStride = ispDipPipe.DMAFeo.dma_cfg.size.stride;
                        pMdpCfg->mdpSrcUVStride = 0; // only one plane
                        pMdpCfg->mdpSrcFmt = eImgFmt_imgi; //kk test
                    }
                    break;
                case EPortIndex_VIPI:
                    if(dip_ctl_dma2_en & DIPCTL_DMA_EN2_VIPI_D1){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        pMdpCfg->mdpSrcFmt = eImgFmt_vipi;
                        pMdpCfg->mdpSrcW = ispDipPipe.DMAVipi.dma_cfg.size.w;
                        pMdpCfg->mdpSrcH = ispDipPipe.DMAVipi.dma_cfg.size.h;
                        switch(eImgFmt_vipi){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                            case eImgFmt_MTK_YVU_P010:
                            case eImgFmt_MTK_YUV_P010:
                            case eImgFmt_YVU_P010:
                            case eImgFmt_YUV_P010:
                            case eImgFmt_NV61:
                            case eImgFmt_NV16:
                            case eImgFmt_MTK_YVU_P210:
                            case eImgFmt_MTK_YUV_P210:
                            case eImgFmt_YVU_P210:
                            case eImgFmt_YUV_P210:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAVipi.dma_cfg.size.h * ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAVip2i.dma_cfg.size.h * ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAVip3i.dma_cfg.size.h * ispDipPipe.DMAVip3i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcPlaneNum = 2;
                                break;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                            case eImgFmt_MTK_YUV_P010_3PLANE:
                            case eImgFmt_YUV_P010_3PLANE:
                            case eImgFmt_YV16:
                            case eImgFmt_I422:
                            case eImgFmt_MTK_YUV_P210_3PLANE:
                            case eImgFmt_YUV_P210_3PLANE:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAVipi.dma_cfg.size.h * ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAVip2i.dma_cfg.size.h * ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAVip3i.dma_cfg.size.h * ispDipPipe.DMAVip3i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcPlaneNum = 3;
                                break;
                            default:
                                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcUVStride = 0;
                                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAVipi.dma_cfg.size.h * ispDipPipe.DMAVipi.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAVip2i.dma_cfg.size.h * ispDipPipe.DMAVip2i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAVip3i.dma_cfg.size.h * ispDipPipe.DMAVip3i.dma_cfg.size.stride;
                                pMdpCfg->mdpSrcPlaneNum = 1;
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            if(isMdpSrcCropBeSet == MTRUE)
                break;
        }


        if(isMdpSrcCropBeSet == MFALSE){
            //set mdp src info. equal to imgi
            pMdpCfg->mdpSrcW = ispDipPipe.DMAImgi.dma_cfg.size.w;
            pMdpCfg->mdpSrcH = ispDipPipe.DMAImgi.dma_cfg.size.h;
            pMdpCfg->mdpSrcFmt = eImgFmt_imgi;
            switch(eImgFmt_imgi){
                case eImgFmt_NV21:
                case eImgFmt_NV12:
                case eImgFmt_MTK_YVU_P010:
                case eImgFmt_MTK_YUV_P010:
                case eImgFmt_YVU_P010:
                case eImgFmt_YUV_P010:
                case eImgFmt_NV61:
                case eImgFmt_NV16:
                case eImgFmt_MTK_YVU_P210:
                case eImgFmt_MTK_YUV_P210:
                case eImgFmt_YVU_P210:
                case eImgFmt_YUV_P210:
                    pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcPlaneNum = 2;
                    break;
                case eImgFmt_YV12:
                case eImgFmt_I420:
                case eImgFmt_MTK_YUV_P010_3PLANE:
                case eImgFmt_YUV_P010_3PLANE:
                case eImgFmt_YV16:
                case eImgFmt_I422:
                case eImgFmt_MTK_YUV_P210_3PLANE:
                case eImgFmt_YUV_P210_3PLANE:
                    pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcPlaneNum = 3;
                    break;
                default:
                    pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcUVStride = 0;
                    pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                    pMdpCfg->mdpSrcPlaneNum = 1;
                    break;
            }
        }
    } else {
        //set mdp src info. equal to imgi
        pMdpCfg->mdpSrcW = ispDipPipe.DMAImgi.dma_cfg.size.w;
        pMdpCfg->mdpSrcH = ispDipPipe.DMAImgi.dma_cfg.size.h;
        pMdpCfg->mdpSrcFmt = eImgFmt_imgi;
        switch(eImgFmt_imgi){
            case eImgFmt_NV21:
            case eImgFmt_NV12:
            case eImgFmt_MTK_YVU_P010:
            case eImgFmt_MTK_YUV_P010:
            case eImgFmt_YVU_P010:
            case eImgFmt_YUV_P010:
            case eImgFmt_NV61:
            case eImgFmt_NV16:
            case eImgFmt_MTK_YVU_P210:
            case eImgFmt_MTK_YUV_P210:
            case eImgFmt_YVU_P210:
            case eImgFmt_YUV_P210:
                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                pMdpCfg->mdpSrcPlaneNum = 2;
                break;
            case eImgFmt_YV12:
            case eImgFmt_I420:
            case eImgFmt_MTK_YUV_P010_3PLANE:
            case eImgFmt_YUV_P010_3PLANE:
            case eImgFmt_YV16:
            case eImgFmt_I422:
            case eImgFmt_MTK_YUV_P210_3PLANE:
            case eImgFmt_YUV_P210_3PLANE:
                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcUVStride = ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                pMdpCfg->mdpSrcPlaneNum = 3;
                break;
            default:
                pMdpCfg->mdpSrcYStride = ispDipPipe.DMAImgi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcUVStride = 0;
                pMdpCfg->mdpSrcBufSize = ispDipPipe.DMAImgi.dma_cfg.size.h * ispDipPipe.DMAImgi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcCBufSize = ispDipPipe.DMAImgbi.dma_cfg.size.h * ispDipPipe.DMAImgbi.dma_cfg.size.stride;
                pMdpCfg->mdpSrcVBufSize = ispDipPipe.DMAImgci.dma_cfg.size.h * ispDipPipe.DMAImgci.dma_cfg.size.stride;
                pMdpCfg->mdpSrcPlaneNum = 1;
                break;
            }
    }

    //new add
    //DMA
    ispDipPipe.DMAImgi.bypass = ( ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_IMGI_D1_EN)? 0 : 1;
    //dma_imgbi
    ispDipPipe.DMAImgbi.bypass = ( ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_IMGBI_D1_EN)? 0 : 1;
    //dma_imgci
    ispDipPipe.DMAImgci.bypass = ( ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_IMGCI_D1_EN)? 0 : 1;
    //dma_ufdi
    ispDipPipe.DMAUfdi.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_UFDI_D1_EN)? 0 : 1;
    //dma_lcei
    ispDipPipe.DMALcei.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_LCEI_D1_EN)? 0 : 1;
    //dma_vipi
    ispDipPipe.DMAVipi.bypass = (ispDipPipe.isp_top_ctl.DMA2_EN.Bits.DIPCTL_VIPI_D1_EN)? 0 : 1;
    //dma_vip2i
    ispDipPipe.DMAVip2i.bypass = (ispDipPipe.isp_top_ctl.DMA2_EN.Bits.DIPCTL_VIPBI_D1_EN)? 0 : 1;
    //dma_vip3i
    ispDipPipe.DMAVip3i.bypass = (ispDipPipe.isp_top_ctl.DMA2_EN.Bits.DIPCTL_VIPCI_D1_EN)? 0 : 1;
    //dma_depi
    ispDipPipe.DMADepi.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_DEPI_D1_EN)? 0 : 1;
    //dma_dmgi
    ispDipPipe.DMADmgi.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_DMGI_D1_EN)? 0 : 1;
    //dma_timgo
    ispDipPipe.DMATimgo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_TIMGO_D1_EN)? 0 : 1;
    //dma_img2o
    ispDipPipe.DMAImg2o.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_CRZO_D1_EN)? 0 : 1;
    //dma_img2bo
    ispDipPipe.DMAImg2bo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_CRZBO_D1_EN)? 0 : 1;
    //dma_img3o
    ispDipPipe.DMAImg3o.bypass = (ispDipPipe.isp_top_ctl.DMA2_EN.Bits.DIPCTL_IMG3O_D1_EN)? 0 : 1;
    //dma_img3bo
    ispDipPipe.DMAImg3bo.bypass = (ispDipPipe.isp_top_ctl.DMA2_EN.Bits.DIPCTL_IMG3BO_D1_EN)? 0 : 1;
    //dma_img3co
    ispDipPipe.DMAImg3co.bypass = (ispDipPipe.isp_top_ctl.DMA2_EN.Bits.DIPCTL_IMG3CO_D1_EN)? 0 : 1;
    //dma_feo
    ispDipPipe.DMAFeo.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_FEO_D1_EN)? 0 : 1;
    //dma_dceso
    ispDipPipe.DMADceso.bypass = (ispDipPipe.isp_top_ctl.DMA_EN.Bits.DIPCTL_DCESO_D1_EN)? 0 : 1;
    ispDipPipe.MDPCallback = pPipePackageInfo->MDPCallback;
    ispDipPipe.MDPparam = pPipePackageInfo->MDPparam;
    //mdp_pipe
#if 0
    //PmQos
    if (drvScen == eDrvScenario_FM)
    {
        ispMdpPipe.framemode_total_in_w = ispDipPipe.DMADmgi.dma_cfg.size.w;
        ispMdpPipe.framemode_h = ispDipPipe.DMADmgi.dma_cfg.size.h;
        ispMdpPipe.total_data_size = (ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h)+\
        (ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h)+(ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h);
    }
    else if ((drvScen == eDrvScenario_LPCNR_Pass1) || (drvScen == eDrvScenario_LPCNR_Pass2)) //YWtodo
    {
        ispMdpPipe.framemode_total_in_w = ispDipPipe.DMADmgi.dma_cfg.size.w;
        ispMdpPipe.framemode_h = ispDipPipe.DMADmgi.dma_cfg.size.h;
        ispMdpPipe.total_data_size = (ispDipPipe.DMADmgi.dma_cfg.size.xsize*ispDipPipe.DMADmgi.dma_cfg.size.h)+\
        (ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMADepi.dma_cfg.size.h)+(ispDipPipe.DMATimgo.dma_cfg.size.xsize*ispDipPipe.DMATimgo.dma_cfg.size.h)+\
        (ispDipPipe.DMADepi.dma_cfg.size.xsize*ispDipPipe.DMALcei.dma_cfg.size.h);
    }
    else
    {
        ispMdpPipe.framemode_total_in_w = 0;
        ispMdpPipe.framemode_h = 0;
        ispMdpPipe.total_data_size = 0;
    }
#endif
    //
#if 0
    ispMdpPipe.bypass = ispDipPipe.isDipOnly;
    ispMdpPipe.dipCtlDmaEn = ispDipPipe.isp_top_ctl.DMA_EN.Raw; //duplicate parameter
    ispMdpPipe.dipCtlDma2En = ispDipPipe.isp_top_ctl.DMA2_EN.Raw; //duplicate parameter
    ispMdpPipe.dip_dma_en = ispDipPipe.isp_top_ctl.DMA_EN.Raw;
    ispMdpPipe.dip_dma2_en = ispDipPipe.isp_top_ctl.DMA2_EN.Raw;
    ispMdpPipe.imgi_dma_cfg = ispDipPipe.DMAImgi.dma_cfg;
    ispMdpPipe.yuv2_en = ispDipPipe.isp_top_ctl.YUV2_EN.Raw;

    PIPE_DBG("[ispMdpPipe]p2MdpSrcFmt(0x%x),w(%d),h(%d),stride(%d,%d),size(%d,%d,%d),planeN(%d)", \
        ispMdpPipe.p2MdpSrcFmt,ispMdpPipe.p2MdpSrcW,ispMdpPipe.p2MdpSrcH,ispMdpPipe.p2MdpSrcYStride,ispMdpPipe.p2MdpSrcUVStride,ispMdpPipe.p2MdpSrcSize,ispMdpPipe.p2MdpSrcCSize,ispMdpPipe.p2MdpSrcVSize,ispMdpPipe.p2MdpSrcPlaneNum);
    PIPE_DBG("ispDipPipe.isp_top_ctl.YUV2_EN.Bits.DIPCTL_CRZ_D1_EN %d/%d",ispDipPipe.isp_top_ctl.YUV2_EN.Bits.DIPCTL_CRZ_D1_EN, m_CrzEn);

    ispMdpPipe.dipTh = ispDipPipe.dipTh;
    //ispMdpPipe.dupCqIdx = ispDipPipe.dupCqIdx;
    ispMdpPipe.burstQueIdx = ispDipPipe.burstQueIdx;
    ispMdpPipe.RingBufIdx = ispDipPipe.RingBufIdx;
    ispMdpPipe.pTuningIspReg = ispDipPipe.pTuningIspReg;
    ispMdpPipe.isDipOnly = ispDipPipe.isDipOnly;
	ispMdpPipe.isSecureFra = ispDipPipe.isSecureFra;
#endif
    ret= ret & ispDipPipe.ConfigDipPipe(m_pDipWorkingBuf);
    //ret = ret & ispMdpPipe.configMDP(m_pDipWorkingBuf);

    ErrCode = checkDipHWConstrain();
    if (ErrCode != ERROR_NO_ERROR)
    {
        PIPE_ERR("Enque Parameter has Problem, please chectk it, ErrCode:%d", ErrCode);
    }
    PIPE_DBG("configPipe ret(%d)",ret);
    return ret;

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int    ret = 0;

    PIPE_DBG("tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%lx,0x%lx,0x%lx)", gettid(), cmd, (long)arg1, (long)arg2, (long)arg3);

    switch ( cmd ) {
        case EPIPECmd_SET_VENC_DRLINK:
            if(m_pIspDrvShell->m_pMdpMgr)
            {
                MUINT32 ret=MDPMGR_NO_ERROR;
                ret=m_pIspDrvShell->m_pMdpMgr->startVencLink((MUINT32)arg1, (MUINT32)arg2, (MUINT32)arg3);
                if(ret !=MDPMGR_NO_ERROR)
                {
                    PIPE_ERR("[Error]startVencLink fail");
                    ret=-1;
                }
            }
            else
            {
                ret=-1;
                PIPE_ERR("[Error]m_pIspDrvShell->m_pMdpMgr = NULL");
            }
            break;
        case EPIPECmd_RELEASE_VENC_DRLINK:
            if(m_pIspDrvShell->m_pMdpMgr)
            {
                MUINT32 ret=MDPMGR_NO_ERROR;
                m_pIspDrvShell->m_pMdpMgr->stopVencLink();
                if(ret !=MDPMGR_NO_ERROR)
                {
                    PIPE_ERR("[Error]stopVencLink fail");
                    ret=-1;
                }
            }
            else
            {
                ret=-1;
                PIPE_ERR("[Error]m_pIspDrvShell->m_pMdpMgr = NULL");
            }
            break;
        case EPIPECmd_SET_JPEG_CFG:
        case EPIPECmd_SET_JPEG_WORKBUF_SIZE:
        default:
            PIPE_ERR("NOT support command!(%d)",cmd);
            return MFALSE;
    }

    if( ret != 0 )
    {
        PIPE_ERR("sendCommand error!");
        return MFALSE;
    }
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
#if 0
MBOOL PostProcPipe::bufferQueCtrl(EPIPE_P2BUFQUECmd cmd, EPIPE_P2engine p2Eng, MUINT32 callerID, MINT32 p2CQ, MINT32 p2dupCQIdx, MINT32 p2burstQIdx, MINT32 frameNum, MINT32 timeout)
{
    MBOOL ret=MTRUE;

    MUINT8* data = (MUINT8*)malloc(12 *sizeof(MUINT8));
    data[0]=static_cast<MUINT8>(cmd);
    data[1]=static_cast<MUINT8>(p2Eng);
    data[2]=static_cast<MUINT8>(0);
    data[3]=static_cast<MUINT8>(callerID);
    data[4]=static_cast<MUINT8>(callerID>>8);
    data[5]=static_cast<MUINT8>(callerID>>16);
    data[6]=static_cast<MUINT8>(callerID>>24);
    data[7]=static_cast<MUINT8>(frameNum);
    data[8]=static_cast<MUINT8>(p2CQ);
    data[9]=static_cast<MUINT8>(p2dupCQIdx);
    data[10]=static_cast<MUINT8>(p2burstQIdx);
    data[11]=static_cast<MUINT8>(timeout);

    PIPE_DBG("input(%d_%d_%d_0x%x_%d_%d_%d_%d_%d)",cmd,p2Eng,0,callerID,frameNum,p2CQ,p2dupCQIdx,p2burstQIdx,timeout);
    PIPE_DBG("data(%d_%d_%d_0x%x_%x_%x_%x_%d_%d_%d_%d_%d)",data[0],data[1],data[2],data[6],data[5],data[4],data[3],data[7],data[8],data[9],data[10],data[11]);

    ret=this->m_pIspDrvShell->m_pDrvDipPhy->setDeviceInfo(_SET_DIP_BUF_INFO, data);
    PIPE_DBG("cmd(%d),ret(%d)",cmd,ret);

    free(data);

    return ret;

}
#endif

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::configSrz(SrzCfg* pSrz_cf, _SRZ_SIZE_INFO_* pSrz_size_info, EDipModule srzEnum)
{
    MUINT32 ctrl=0x3;   //srz1_vert_en=1 & srz1_hori_en=1

    pSrz_cf->inout_size.out_w=pSrz_size_info->out_w;
    pSrz_cf->inout_size.out_h=pSrz_size_info->out_h;
    pSrz_cf->crop.x=pSrz_size_info->crop_x;
    pSrz_cf->crop.y=pSrz_size_info->crop_y;
    pSrz_cf->crop.w=pSrz_size_info->crop_w;
    pSrz_cf->crop.h=pSrz_size_info->crop_h;

    if(pSrz_size_info->crop_w>pSrz_size_info->in_w)
    {
        ctrl |= 0x10;
    }
    pSrz_cf->ctrl=ctrl;
    //update floating offset for tpipe /*20 bit base*/
    pSrz_cf->crop.floatX= \
        ((pSrz_size_info->crop_floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    pSrz_cf->crop.floatY= \
        ((pSrz_size_info->crop_floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    //coeffienct value for tpipe structure
    if (EDipModule_SRZ5 == srzEnum) //in order to padding in mfb
    {
        pSrz_cf->h_step=((pSrz_size_info->crop_w-1)*32768+(((((pSrz_size_info->out_w+31)>>5)<<5)-1)>>1) )/((((pSrz_size_info->out_w+31)>>5)<<5)-1);
        pSrz_cf->v_step=((pSrz_size_info->crop_h-1)*32768+(((((pSrz_size_info->out_h+31)>>5)<<5)-1)>>1) )/((((pSrz_size_info->out_h+31)>>5)<<5)-1);
        pSrz_cf->inout_size.in_w=((((long long)pSrz_cf->h_step*(pSrz_size_info->out_w-1))+16384)>>15)+1;
        pSrz_cf->inout_size.in_h=((((long long)pSrz_cf->v_step*(pSrz_size_info->out_h-1))+16384)>>15)+1;
    }
    else
    {
        pSrz_cf->h_step=(( pSrz_size_info->crop_w-1)*32768+((pSrz_size_info->out_w-1)>>1) )/(pSrz_size_info->out_w-1);
        pSrz_cf->v_step=(( pSrz_size_info->crop_h-1)*32768+((pSrz_size_info->out_h-1)>>1) )/(pSrz_size_info->out_h-1);
        pSrz_cf->inout_size.in_w=pSrz_size_info->in_w;
        pSrz_cf->inout_size.in_h=pSrz_size_info->in_h;
    }
    //
     PIPE_DBG("SRZ1:ctrl(0x%x), in[%d, %d] crop[%d/%d_0x%x/0x%x]_(%lu, %lu) out[%d,%d, cof(0x%x/0x%x)], srz:%d ",pSrz_cf->ctrl, pSrz_cf->inout_size.in_w, pSrz_cf->inout_size.in_h, \
        pSrz_cf->crop.x, pSrz_cf->crop.y, pSrz_cf->crop.floatX, pSrz_cf->crop.floatY, \
        pSrz_cf->crop.w, pSrz_cf->crop.h, pSrz_cf->inout_size.out_w, pSrz_cf->inout_size.out_h,\
        pSrz_cf->h_step, pSrz_cf->v_step, srzEnum);
    return MTRUE;

}

MBOOL
PostProcPipe::configSrz5(SrzCfg *pSrz_cfg, NSCam::NSIoPipe::SRZSizeInfo SRZ5Info, unsigned int wkaround_h)
{
    MUINT32 srz_ctrl = 0x13; // srz_vert_en = 1, srz_hori_en = 1

    pSrz_cfg->ctrl = srz_ctrl;
    pSrz_cfg->inout_size.out_w = SRZ5Info.out_w;
    pSrz_cfg->inout_size.out_h = wkaround_h;
    pSrz_cfg->crop.x = SRZ5Info.crop_x;
    pSrz_cfg->crop.y = SRZ5Info.crop_y;
    pSrz_cfg->crop.w = SRZ5Info.crop_w;
    pSrz_cfg->crop.h = SRZ5Info.crop_h;
    pSrz_cfg->crop.floatX = ((SRZ5Info.crop_floatX>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    pSrz_cfg->crop.floatY = ((SRZ5Info.crop_floatY>>(CROP_FLOAT_PECISE_BIT-CROP_TPIPE_PECISE_BIT)) & ((1<<CROP_TPIPE_PECISE_BIT)-1));/* tpipe calculate via 0~19 bits */
    pSrz_cfg->h_step = ((SRZ5Info.crop_w-1)*32768+(((((SRZ5Info.out_w+31)>>5)<<5)-1)>>1) )/((((SRZ5Info.out_w+31)>>5)<<5)-1);
    pSrz_cfg->v_step = ((SRZ5Info.crop_h-1)*32768+(((((wkaround_h+31)>>5)<<5)-1)>>1) )/((((wkaround_h+31)>>5)<<5)-1);
    pSrz_cfg->inout_size.in_w = ((((long long)pSrz_cfg->h_step*(SRZ5Info.out_w-1))+16384)>>15)+1;
    pSrz_cfg->inout_size.in_h = ((((long long)pSrz_cfg->v_step*(wkaround_h-1))+16384)>>15)+1;

    PIPE_INF("out_w: %d, out_h: %d, in_w: %d, in_h: %d, h_step: %d, v_step: %d\n",
        pSrz_cfg->inout_size.out_w, pSrz_cfg->inout_size.out_h, pSrz_cfg->inout_size.in_w, pSrz_cfg->inout_size.in_h, pSrz_cfg->h_step, pSrz_cfg->v_step);

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::configInDmaPort(NSCam::NSIoPipe::Input const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt)
{
    (void)swap;
    ISP_QUERY_RST queryRst;
    MINT32 pixelDivNum=0, pixelMulNum=0;
    E_ISP_PIXMODE e_PixMode = ISP_QUERY_UNKNOWN_PIX_MODE;
    //
    a_dma.memBuf.size        = portInfo->mBuffer->getBufSizeInBytes(planeNum);
    a_dma.memBuf.base_vAddr  = portInfo->mBuffer->getBufVA(planeNum);
	if ((portInfo->mSecureTag != 0) & (portInfo->mSecHandle != 0))	// For tuning secure check
    {
        a_dma.memBuf.base_pAddr  = portInfo->mSecHandle;
    }
    else
    {
    a_dma.memBuf.base_pAddr  = portInfo->mBuffer->getBufPA(planeNum);
    }
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->mBuffer->getImgSize().w;
    a_dma.size.h            = portInfo->mBuffer->getImgSize().h;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->mBuffer->getBufStridesInBytes(planeNum);
    a_dma.v_flip_en = MFALSE;

    //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
    switch(eImgFmt)
    {
        case eImgFmt_YUY2:
            pixelDivNum=2;
            pixelMulNum=1;
            break;
        case eImgFmt_BAYER10:
            pixelDivNum=4;
            pixelMulNum=5;
            break;
        case eImgFmt_BAYER12:
            pixelDivNum=2;
            pixelMulNum=3;
            break;
        case eImgFmt_BAYER8:
        case eImgFmt_Y8:
        case eImgFmt_YV12:
        default:
            pixelDivNum=1;
            pixelMulNum=1;
            break;
    }
    //dma starting offset
    if (planeNum == ESTRIDE_1ST_PLANE)
       {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % a_dma.size.stride)/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / a_dma.size.stride;
    }
    else
    {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % (portInfo->mBuffer->getBufStridesInBytes(0)))/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / (portInfo->mBuffer->getBufStridesInBytes(0));
    }
    //dma port capbility
    a_dma.capbility=portInfo->mPortID.capbility;

	a_dma.secureTag=portInfo->mSecureTag;

    //input xsize unit:byte
    ISP_QuerySize(VirPortIdxMapToHWPortIdx(portInfo->mPortID.index),ISP_QUERY_XSIZE_BYTE,(EImageFormat)eImgFmt,a_dma.size.w,queryRst,e_PixMode);
    a_dma.size.xsize        =  queryRst.xsize_byte;

    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
    }
    //
    if(isBypassOffset) // for tpipe mode
        a_dma.memBuf.ofst_addr = 0;
    else
        a_dma.memBuf.ofst_addr   = ( ( ( a_dma.crop.y ? (a_dma.crop.y- 1) : 0 )*(a_dma.size.w)+a_dma.crop.x ) * pixel_Byte ) >> CAM_ISP_PIXEL_BYTE_FP;

    //swap, bus size, yuv bit
    switch( eImgFmt ) {
        case eImgFmt_YUY2:      //422 format, 1 plane (YUYV)
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_UYVY:      //422 format, 1 plane (UYVY)
            a_dma.swap=1;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YVYU:      //422 format, 1 plane (YVYU)
            a_dma.swap=2;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_VYUY:      //422 format, 1 plane (VYUY)
            a_dma.swap=3;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_NV12:      //420 format, 2 plane (Y),(UV)
        case eImgFmt_NV16:      //422 format, 2 plane (Y),(UV)
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_NV21:      //420 format, 2 plane (Y),(VU)
        case eImgFmt_NV61:      //422 format, 2 plane (Y),(VU)
            a_dma.swap=1;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        //
        case eImgFmt_MTK_YUYV_Y210:   //422 format, 10bit, 1 plane (YUYV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YUYV_Y210:   //422 format, 16bit, 1 plane (YUYV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_YVYU_Y210:   //422 format, 10bit, 1 plane (YVYU)
            a_dma.swap=2;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YVYU_Y210:   //422 format, 16bit, 1 plane (YVYU)
            a_dma.swap=2;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_UYVY_Y210:   //422 format, 10bit, 1 plane (UYVY)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_UYVY_Y210:   //422 format, 16bit, 1 plane (UYVY)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_VYUY_Y210:   //422 format, 10bit, 1 plane (VYUY)
            a_dma.swap=3;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_VYUY_Y210:   //422 format, 16bit, 1 plane (VYUY)
            a_dma.swap=3;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_YUV_P210:    //422 format, 10bit, 2 plane (Y),(UV)
        case eImgFmt_MTK_YUV_P010:    //420 format, 10bit, 2 plane (Y),(UV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YUV_P210:    //422 format, 16bit, 2 plane (Y),(UV)
        case eImgFmt_YUV_P010:    //420 format, 16bit, 2 plane (Y),(UV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_YVU_P210:    //422 format, 10bit, 2 plane (Y),(VU)
        case eImgFmt_MTK_YVU_P010:    //420 format, 10bit, 2 plane (Y),(VU)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YVU_P210:    //422 format, 16bit, 2 plane (Y),(VU)
        case eImgFmt_YVU_P010:    //420 format, 16bit, 2 plane (Y),(VU)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        //
        case eImgFmt_YV16:      //422 format, 3 plane (YVU)
        case eImgFmt_I422:      //422 format, 3 plane (YUV)
        case eImgFmt_YV12:      //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //420 format, 3 plane (YUV)
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_MTK_YUV_P210_3PLANE:    //422 format, 10bit, 3 plane (Y),(U),(V)
        case eImgFmt_MTK_YUV_P010_3PLANE:    //420 format, 10bit, 3 plane (Y),(U),(V)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YUV_P210_3PLANE:    //422 format, 16bit, 3 plane (Y),(U),(V)
        case eImgFmt_YUV_P010_3PLANE:    //420 format, 16bit, 3 plane (Y),(U),(V)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        //
        case eImgFmt_BAYER8_UNPAK:
        case eImgFmt_BAYER10_UNPAK:
        case eImgFmt_BAYER12_UNPAK:
        case eImgFmt_BAYER14_UNPAK:
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_RGB565:      //RGB 565 (16-bit), 1 plane
        case eImgFmt_RGB888:      //RGB 888 (24-bit), 1 plane
        case eImgFmt_ARGB8888:    //ARGB (32-bit), 1 plane
        case eImgFmt_RGB48:       //ARGB (32-bit), 1 plane
        case eImgFmt_BAYER8:      /*!< Bayer format, 8-bit */
        case eImgFmt_BAYER10:     /*!< Bayer format, 10-bit */
        case eImgFmt_BAYER12:     /*!< Bayer format, 12-bit */
        case eImgFmt_BAYER14:     /*!< Bayer format, 14-bit */
        case eImgFmt_BAYER10_MIPI:/*!< Bayer format, 10-bit (MIPI) */
        case eImgFmt_STA_BYTE:
        case eImgFmt_STA_2BYTE:
        case eImgFmt_STA_4BYTE:
        case eImgFmt_STA_10BIT:
        case eImgFmt_STA_12BIT:
        case eImgFmt_UFO_FG:
        default:
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
    }
    a_dma.bus_size_en=0;  //HW auto setting
    a_dma.format_en=0;    //HW auto setting
    //
    PIPE_DBG("w(%lu),h(%lu),stride(%lu),xsize(0x%lx)/h(%lu),crop(%d,%d,%lu,%lu)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%lx),pa(0x%lx),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.size.h,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        (unsigned long)a_dma.memBuf.base_vAddr, (unsigned long)a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::configOutDmaPort(NSCam::NSIoPipe::Output const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum, NSCam::EImageFormat eImgFmt)
{
    (void)swap;
    ISP_QUERY_RST queryRst;
    MINT32 pixelDivNum=0, pixelMulNum=0;
    E_ISP_PIXMODE e_PixMode = ISP_QUERY_UNKNOWN_PIX_MODE;
    //
    a_dma.memBuf.size        = portInfo->mBuffer->getBufSizeInBytes(planeNum);
    a_dma.memBuf.base_vAddr  = portInfo->mBuffer->getBufVA(planeNum);
    a_dma.memBuf.base_pAddr  = portInfo->mBuffer->getBufPA(planeNum);
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->mBuffer->getImgSize().w;
    a_dma.size.h            = portInfo->mBuffer->getImgSize().h;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->mBuffer->getBufStridesInBytes(planeNum);
    a_dma.v_flip_en = MFALSE;

    //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
    switch(eImgFmt)
    {
        case eImgFmt_YUY2:
            pixelDivNum=2;
            pixelMulNum=1;
            break;
        case eImgFmt_BAYER10:
            pixelDivNum=4;
            pixelMulNum=5;
            break;
        case eImgFmt_BAYER12:
            pixelDivNum=2;
            pixelMulNum=3;
            break;
        case eImgFmt_BAYER8:
        case eImgFmt_Y8:
        case eImgFmt_YV12:
        default:
            pixelDivNum=1;
            pixelMulNum=1;
            break;
    }
    //dma starting offset
    if (planeNum == ESTRIDE_1ST_PLANE)
       {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % a_dma.size.stride)/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / a_dma.size.stride;
    }
    else
    {
        a_dma.offset.x          =  ((portInfo->mOffsetInBytes % (portInfo->mBuffer->getBufStridesInBytes(0)))/pixelDivNum)*pixelMulNum;
        a_dma.offset.y          =  portInfo->mOffsetInBytes / (portInfo->mBuffer->getBufStridesInBytes(0));
    }
    //
    #if 0
    a_dma.format = portInfo->eImgFmt;
    #endif
    //dma port capbility
    a_dma.capbility=portInfo->mPortID.capbility;//portInfo->capbility;

	a_dma.secureTag=portInfo->mSecureTag;

    //input xsize unit:byte
    ISP_QuerySize(VirPortIdxMapToHWPortIdx(portInfo->mPortID.index),ISP_QUERY_XSIZE_BYTE,(EImageFormat)eImgFmt,a_dma.size.w,queryRst,e_PixMode);
    a_dma.size.xsize        =  queryRst.xsize_byte;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
    }
    //
    if(isBypassOffset) // for tpipe mode
        a_dma.memBuf.ofst_addr = 0;
    else
        a_dma.memBuf.ofst_addr   = ( ( ( a_dma.crop.y ? (a_dma.crop.y- 1) : 0 )*(a_dma.size.w)+a_dma.crop.x ) * pixel_Byte ) >> CAM_ISP_PIXEL_BYTE_FP;

    a_dma.pak_obit = 10; //default value is 10 based on mainpipe bit-depth
    //swap, bus size, yuv bit
    switch( eImgFmt ) {
        case eImgFmt_YUY2:      //422 format, 1 plane (YUYV)
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_UYVY:      //422 format, 1 plane (UYVY)
            a_dma.swap=1;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YVYU:      //422 format, 1 plane (YVYU)
            a_dma.swap=2;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_VYUY:      //422 format, 1 plane (VYUY)
            a_dma.swap=3;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_NV12:      //420 format, 2 plane (Y),(UV)
        case eImgFmt_NV16:      //422 format, 2 plane (Y),(UV)
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_NV21:      //420 format, 2 plane (Y),(VU)
        case eImgFmt_NV61:      //422 format, 2 plane (Y),(VU)
            a_dma.swap=1;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        //
        case eImgFmt_MTK_YUYV_Y210:   //422 format, 10bit, 1 plane (YUYV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YUYV_Y210:   //422 format, 16bit, 1 plane (YUYV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_YVYU_Y210:   //422 format, 10bit, 1 plane (YVYU)
            a_dma.swap=2;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YVYU_Y210:   //422 format, 16bit, 1 plane (YVYU)
            a_dma.swap=2;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_UYVY_Y210:   //422 format, 10bit, 1 plane (UYVY)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_UYVY_Y210:   //422 format, 16bit, 1 plane (UYVY)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_VYUY_Y210:   //422 format, 10bit, 1 plane (VYUY)
            a_dma.swap=3;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_VYUY_Y210:   //422 format, 16bit, 1 plane (VYUY)
            a_dma.swap=3;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_YUV_P210:    //422 format, 10bit, 2 plane (Y),(UV)
        case eImgFmt_MTK_YUV_P010:    //420 format, 10bit, 2 plane (Y),(UV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YUV_P210:    //422 format, 16bit, 2 plane (Y),(UV)
        case eImgFmt_YUV_P010:    //420 format, 16bit, 2 plane (Y),(UV)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        case eImgFmt_MTK_YVU_P210:    //422 format, 10bit, 2 plane (Y),(VU)
        case eImgFmt_MTK_YVU_P010:    //420 format, 10bit, 2 plane (Y),(VU)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YVU_P210:    //422 format, 16bit, 2 plane (Y),(VU)
        case eImgFmt_YVU_P010:    //420 format, 16bit, 2 plane (Y),(VU)
            a_dma.swap=1;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        //
        case eImgFmt_YV16:      //422 format, 3 plane (YVU)
        case eImgFmt_I422:      //422 format, 3 plane (YUV)
        case eImgFmt_YV12:      //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //420 format, 3 plane (YUV)
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_MTK_YUV_P210_3PLANE:    //422 format, 10bit, 3 plane (Y),(U),(V)
        case eImgFmt_MTK_YUV_P010_3PLANE:    //420 format, 10bit, 3 plane (Y),(U),(V)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=0;
            break;
        case eImgFmt_YUV_P210_3PLANE:    //422 format, 16bit, 3 plane (Y),(U),(V)
        case eImgFmt_YUV_P010_3PLANE:    //420 format, 16bit, 3 plane (Y),(U),(V)
            a_dma.swap=0;
            a_dma.yuv_bit=1;
            a_dma.yuv_dng=1;
            break;
        //
        case eImgFmt_BAYER8:      /*!< Bayer format, 8-bit */
        case eImgFmt_BAYER10:     /*!< Bayer format, 10-bit */
        case eImgFmt_BAYER12:     /*!< Bayer format, 12-bit */
        case eImgFmt_BAYER14:     /*!< Bayer format, 14-bit */
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            a_dma.pak_obit = 12; //for crp_d2_sel = 0
            break;
        case eImgFmt_BAYER8_UNPAK:
        case eImgFmt_BAYER10_UNPAK:
        case eImgFmt_BAYER12_UNPAK:
        case eImgFmt_BAYER14_UNPAK:
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=1;
            a_dma.pak_obit = 12; //for crp_d2_sel = 0
            break;
        case eImgFmt_RGB565:      //RGB 565 (16-bit), 1 plane
        case eImgFmt_RGB888:      //RGB 888 (24-bit), 1 plane
        case eImgFmt_ARGB8888:    //ARGB (32-bit), 1 plane
        case eImgFmt_RGB48:       //ARGB (32-bit), 1 plane
        case eImgFmt_BAYER10_MIPI:/*!< Bayer format, 10-bit (MIPI) */
        case eImgFmt_STA_BYTE:
        case eImgFmt_STA_2BYTE:
        case eImgFmt_STA_4BYTE:
        case eImgFmt_STA_10BIT:
        case eImgFmt_STA_12BIT:
        case eImgFmt_UFO_FG:
        default:
            a_dma.swap=0;
            a_dma.yuv_bit=0;
            a_dma.yuv_dng=0;
            break;
    }
    a_dma.bus_size_en=0;  //HW auto setting
    a_dma.format_en=0;    //HW auto setting
    //

    PIPE_DBG("w(%lu),h(%lu),stride(%lu),xsize(0x%lx)/h(%lu),crop(%d,%d,%lu,%lu)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%lx),pa(0x%lx),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.size.h,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        (unsigned long)a_dma.memBuf.base_vAddr, (unsigned long)a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configMdpOutPort(NSCam::NSIoPipe::Output const* oImgInfo, MdpRotDMACfg &a_rotDma, NSCam::EImageFormat eImgFmt)
{
    MUINT32 plane_num = 1;
    MUINT32 uv_h_ratio = 1;
    MUINT32 uv_v_ratio = 1;
    MUINT32 y_plane_size = 0;
    MUINT32 u_plane_size = 0;

    //PIPE_DBG("[oImgInfo]w(%d),h(%d),stride(%d/%d/%d),crop(%d,%d,%d,%d)_f(0x%x, 0x%x),rot(%d) eImgFmt(%d)",
    //    oImgInfo->u4ImgWidth,oImgInfo->u4ImgHeight,oImgInfo->u4Stride[ESTRIDE_1ST_PLANE],oImgInfo->u4Stride[ESTRIDE_2ND_PLANE],oImgInfo->u4Stride[ESTRIDE_3RD_PLANE], \
    //    oImgInfo->crop1.x,oImgInfo->crop1.y,oImgInfo->crop1.w,oImgInfo->crop1.h,oImgInfo->crop1.floatX,oImgInfo->crop1.floatY, \
    //    oImgInfo->eImgRot,oImgInfo->eImgFmt );
    //
    a_rotDma.Format = eImgFmt;
    a_rotDma.uv_plane_swap = 0;
    //
    switch( eImgFmt ) {
        case eImgFmt_YUY2:            //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //422 format, 1 plane (VYUY)
        case eImgFmt_MTK_YUYV_Y210:   //422 format, 10bit, 1 plane (YUYV)
        case eImgFmt_YUYV_Y210:       //422 format, 16bit, 1 plane (YUYV)
        case eImgFmt_MTK_YVYU_Y210:   //422 format, 10bit, 1 plane (YVYU)
        case eImgFmt_YVYU_Y210:       //422 format, 16bit, 1 plane (YVYU)
        case eImgFmt_MTK_UYVY_Y210:   //422 format, 10bit, 1 plane (UYVY)
        case eImgFmt_UYVY_Y210:       //422 format, 16bit, 1 plane (UYVY)
        case eImgFmt_MTK_VYUY_Y210:   //422 format, 10bit, 1 plane (VYUY)
        case eImgFmt_VYUY_Y210:       //422 format, 16bit, 1 plane (VYUY)
            a_rotDma.Plane = 0;
            break;
        //
        case eImgFmt_NV12:            //420 format, 2 plane (Y),(UV)
        case eImgFmt_NV21:            //420 format, 2 plane (Y),(VU)
        case eImgFmt_MTK_YVU_P010:    //420 format 10b, 2 plane (VU)
        case eImgFmt_YVU_P010:        //420 format 16b, 2 plane (VU)
        case eImgFmt_MTK_YUV_P010:    //420 format 10b, 2 plane (UV)
        case eImgFmt_YUV_P010:        //420 format 16b, 2 plane (UV)
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 2;
            break;
        case eImgFmt_NV16:            //422 format, 2 plane (Y),(UV)
        case eImgFmt_NV61:            //422 format, 2 plane (Y),(VU)
        case eImgFmt_MTK_YUV_P210:    //422 format, 10bit, 2 plane (Y),(UV)
        case eImgFmt_YUV_P210:        //422 format, 16bit, 2 plane (Y),(UV)
        case eImgFmt_MTK_YVU_P210:    //422 format, 10bit, 2 plane (Y),(VU)
        case eImgFmt_YVU_P210:        //422 format, 16bit, 2 plane (Y),(VU)
            a_rotDma.Plane = 1;
            plane_num = 2;
            uv_h_ratio = 1;
            uv_v_ratio = 1;
            break;
        //
        case eImgFmt_YV16:            //422 format, 3 plane (YVU)
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            a_rotDma.uv_plane_swap = 1;
            break;
        case eImgFmt_I422:            //422 format, 3 plane (YUV)
        case eImgFmt_MTK_YUV_P210_3PLANE:    //422 format, 10bit, 3 plane (Y),(U),(V)
        case eImgFmt_YUV_P210_3PLANE:        //422 format, 16bit, 3 plane (Y),(U),(V)
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            break;
        case eImgFmt_YV12:            //420 format, 3 plane (YVU)
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            a_rotDma.uv_plane_swap = 1;
            break;
        case eImgFmt_I420:            //420 format, 3 plane (YUV)
        case eImgFmt_MTK_YUV_P010_3PLANE:    //420 format, 10bit, 3 plane (Y),(U),(V)
        case eImgFmt_YUV_P010_3PLANE:        //420 format, 16bit, 3 plane (Y),(U),(V)
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        //
        case eImgFmt_RGB565:    //RGB 565 (16-bit), 1 plane
        case eImgFmt_RGB888:    //RGB 888 (24-bit), 1 plane
        case eImgFmt_ARGB8888:  //ARGB (32-bit), 1 plane
        case eImgFmt_RGBA8888:
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_Y800:      //Y plane only
        case eImgFmt_STA_BYTE:
        case eImgFmt_Y16:
            a_rotDma.Format = eImgFmt_STA_BYTE;
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_JPG_I420:
            a_rotDma.Format = eImgFmt_JPG_I420;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 2;
            break;
        case eImgFmt_JPG_I422:
            a_rotDma.Format = eImgFmt_JPG_I422;
            a_rotDma.Plane = 2;
            plane_num = 3;
            uv_h_ratio = 2;
            uv_v_ratio = 1;
            break;
        case eImgFmt_BAYER8:    //Bayer format, 8-bit     // for imgo
        case eImgFmt_BAYER10:   //Bayer format, 10-bit    // for imgo
        case eImgFmt_BAYER12:   //Bayer format, 12-bit    // for imgo
        case eImgFmt_BAYER8_UNPAK:
        case eImgFmt_BAYER10_UNPAK:
        case eImgFmt_BAYER12_UNPAK:
        case eImgFmt_BAYER14_UNPAK:
            a_rotDma.Plane = 0;
            break;
        case eImgFmt_NV21_BLK:  //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //JPEG format
        case eImgFmt_UFO_FG:
        default:
            PIPE_ERR("[configMdpOutPort] Format(0x%x) NOT Support",eImgFmt);
            break;
    }
    //
    //ROTATION, stride is after, others are before
    a_rotDma.Rotation = (MUINT32)(mTransformMapping[oImgInfo->mTransform].eImgRot);
    a_rotDma.Flip = (eImgFlip_ON == (mTransformMapping[oImgInfo->mTransform].eImgFlip))?MTRUE:MFALSE;
    //dma port capbility
    a_rotDma.capbility=oImgInfo->mPortID.capbility;
    //Y
    a_rotDma.memBuf.base_pAddr = oImgInfo->mBuffer->getBufPA(0);
    a_rotDma.memBuf.base_vAddr = oImgInfo->mBuffer->getBufVA(0);
    a_rotDma.memBuf.size = oImgInfo->mBuffer->getBufSizeInBytes(0);
    a_rotDma.memBuf.ofst_addr = 0;
    a_rotDma.memBuf.alignment = 0;
    //after ROT
    a_rotDma.size.w = oImgInfo->mBuffer->getImgSize().w;
    a_rotDma.size.h = oImgInfo->mBuffer->getImgSize().h;
    //stride info after ROT
    a_rotDma.size.stride = oImgInfo->mBuffer->getBufStridesInBytes(ESTRIDE_1ST_PLANE);
    //
    y_plane_size = oImgInfo->mBuffer->getBufSizeInBytes(ESTRIDE_1ST_PLANE);
    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}
    //
    if ( 2<=plane_num) {
        //U
        a_rotDma.memBuf_c.base_pAddr = oImgInfo->mBuffer->getBufPA(ESTRIDE_2ND_PLANE);
        a_rotDma.memBuf_c.base_vAddr = oImgInfo->mBuffer->getBufVA(ESTRIDE_2ND_PLANE);
        a_rotDma.memBuf_c.ofst_addr = 0;
        a_rotDma.memBuf_c.alignment = 0;
        //after ROT
        a_rotDma.size_c.w =         a_rotDma.size.w/uv_h_ratio;
        a_rotDma.size_c.h =         a_rotDma.size.h/uv_v_ratio;
        //stride info after ROT
        a_rotDma.size_c.stride =    oImgInfo->mBuffer->getBufStridesInBytes(ESTRIDE_2ND_PLANE);
        //
        u_plane_size = oImgInfo->mBuffer->getBufSizeInBytes(ESTRIDE_2ND_PLANE);
        //tpipemain lib need image info before ROT. stride info after ROT
        //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
        //    a_rotDma.size_c.w = a_rotDma.size.h/uv_v_ratio;
        //    a_rotDma.size_c.h = a_rotDma.size.w/uv_h_ratio;
        //}
        //
        if ( 3 == plane_num ) {
            //V
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->mBuffer->getBufPA(ESTRIDE_3RD_PLANE);
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->mBuffer->getBufVA(ESTRIDE_3RD_PLANE);
            a_rotDma.memBuf_v.ofst_addr = 0;
            a_rotDma.memBuf_v.alignment = 0;
            //after ROT
            a_rotDma.size_v.w =        a_rotDma.size_c.w;
            a_rotDma.size_v.h =        a_rotDma.size_c.h;
            //stride info after ROT
            a_rotDma.size_v.stride =   oImgInfo->mBuffer->getBufStridesInBytes(ESTRIDE_3RD_PLANE);
            //tpipemain lib need image info BEFORE ROT. stride info AFTER ROT
            //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
            //    a_rotDma.size_v.w = a_rotDma.size_c.h;
            //    a_rotDma.size_v.h = a_rotDma.size_c.w;
            //}
        }
        //
        /*
        if ( a_rotDma.uv_plane_swap ) {
            a_rotDma.memBuf_c.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_c.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_3RD_PLANE];
            a_rotDma.memBuf_v.base_pAddr = oImgInfo->u4BufPA[ESTRIDE_2ND_PLANE];
            a_rotDma.memBuf_v.base_vAddr = oImgInfo->u4BufVA[ESTRIDE_2ND_PLANE];
        }
        */
    }

    //tpipemain lib need image info before ROT. stride info after ROT
    //if ( CRZ_DRV_ROTATION_90 == a_rotDma.Rotation || CRZ_DRV_ROTATION_270 == a_rotDma.Rotation ) {
    //    //
    //    a_rotDma.size.w = oImgInfo->u4ImgHeight;
    //    a_rotDma.size.h = oImgInfo->u4ImgWidth;
    //}

    /*
        * DONOT USE CRZ CROP due to throughtput issue
        */
    //default set ENABLE, if need to be programmable, check cdp driver as well.
    a_rotDma.crop_en = 1; //always enable crop for rotation issue.
    //
    a_rotDma.crop.x = 0;
    a_rotDma.crop.floatX = 0;
    a_rotDma.crop.y = 0;
    a_rotDma.crop.floatY = 0;
    a_rotDma.crop.w = 0;
    a_rotDma.crop.h = 0;

	a_rotDma.secureTag=oImgInfo->mSecureTag;
    //
    PIPE_DBG("[a_rotDma]w(%lu),h(%lu),stride(pxl)(%lu),pa(0x%lx),va(0x%lx),size(%d)",a_rotDma.size.w,a_rotDma.size.h,a_rotDma.size.stride,(unsigned long)a_rotDma.memBuf.base_pAddr,(unsigned long)a_rotDma.memBuf.base_vAddr,a_rotDma.memBuf.size);
    PIPE_DBG("[a_rotDma]crop(%d,%d,%lu,%lu)_f(0x%x,0x%x)",a_rotDma.crop.x,a_rotDma.crop.y,a_rotDma.crop.w,a_rotDma.crop.h,a_rotDma.crop.floatX,a_rotDma.crop.floatY);
    PIPE_DBG("[a_rotDma]rot(%d),fmt(%d),plane(%d),pxlByte((0x%x))",a_rotDma.Rotation,a_rotDma.Format,a_rotDma.Plane,a_rotDma.pixel_byte);
    PIPE_DBG("[a_rotDma]c_ofst_addr(0x%x),v_ofst_addr(0x%x),uv_plane_swap(%d)",a_rotDma.memBuf_c.ofst_addr,a_rotDma.memBuf_v.ofst_addr,a_rotDma.uv_plane_swap);
    PIPE_DBG("[a_rotDma]va[0x%lx,0x%lx,0x%lx]--pa[0x%lx,0x%lx,0x%lx]",
        (unsigned long)a_rotDma.memBuf.base_vAddr,(unsigned long)a_rotDma.memBuf_c.base_vAddr,(unsigned long)a_rotDma.memBuf_v.base_vAddr,(unsigned long)a_rotDma.memBuf.base_pAddr,(unsigned long)a_rotDma.memBuf_c.base_pAddr,(unsigned long)a_rotDma.memBuf_v.base_pAddr);


    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
dumpFailFrameSetting(const NSCam::NSIoPipe::FrameParams &pFrameParams, int p2CQ, int p2RingBufIdx, bool lastframe, char const* szCallerName)
{
    NSCam::NSIoPipe::ExtraParam CmdInfo;
    SrzCfg srzCfg;
    DipModuleCfg module;
    DipWorkingBuffer* pDipWBuf;
    MDPMGR_CFG_STRUCT *pMdpCfg;
    MINT32 planeNum = 0;
    NSCam::EImageFormat eImgFmt_imgi = eImgFmt_UNKNOWN;
    UFDG_META_INFO *pUfdParam = NULL;
    CropInfo CropInfo_imgi;
    MUINT32 WdmaoCropGroup = 0, WrotoCropGroup = 0;
    dip_x_reg_t* pTuningDipReg = NULL;
    dip_x_reg_t* pWkBufDipReg = NULL;
    NSCam::NSIoPipe::CrspInfo Img3oCrspTmp;
    android::Vector<NSCam::NSIoPipe::ExtraParam>::const_iterator iter;
    MINT32 pixIdImgBuf = 0;
    MUINT32 WDMAPQEnable = 0, WROTPQEnable = 0;
    int tpipe = CAM_MODE_TPIPE;
    MBOOL isDipOnly;
    int tdr_ctl = 0x0;
    int tdr_sel = 0x0;
    MBOOL   crzEn;
    PIPE_ERR("dumpFailFrameSetting Start!!!\n");

    pDipWBuf = m_pDipRingBuf->getDipWorkingBufferbyIdx((E_ISP_DIP_CQ)p2CQ, p2RingBufIdx);
    if(pDipWBuf==NULL)
    {
        PIPE_ERR("dumpFailFrameSetting: getDipWorkingBufferbyIdx error!!\n");
        return MFALSE; //error command  enque
    }
    pMdpCfg = pDipWBuf->m_pMdpCfg;
    if(pMdpCfg==NULL)
    {
        PIPE_ERR("dumpFailFrameSetting: getDipWorkingBufferbyIdx error!!\n");
        return MFALSE; //error command  enque
    }
    pWkBufDipReg = (dip_x_reg_t*)pDipWBuf->m_pIspVirRegAddr_va;

    for(iter = pFrameParams.mvExtraParam.begin();iter<pFrameParams.mvExtraParam.end();iter++)
    {
        CmdInfo = (*iter);
        switch (CmdInfo.CmdIdx)
        {
            case NSCam::NSIoPipe::EPIPE_IMG3O_CRSPINFO_CMD:
                m_Img3oCropInfo = (NSCam::NSIoPipe::CrspInfo*)CmdInfo.moduleStruct;
                if (m_Img3oCropInfo != NULL)
                {
                    Img3oCrspTmp.m_CrspInfo.p_integral.x = m_Img3oCropInfo->m_CrspInfo.p_integral.x;
                    Img3oCrspTmp.m_CrspInfo.p_integral.y = m_Img3oCropInfo->m_CrspInfo.p_integral.y;
                }
                break;
            case NSCam::NSIoPipe::EPIPE_FE_INFO_CMD:
            case NSCam::NSIoPipe::EPIPE_FM_INFO_CMD:
            case NSCam::NSIoPipe::EPIPE_WPE_INFO_CMD:
            case NSCam::NSIoPipe::EPIPE_MDP_PQPARAM_CMD:
            case NSCam::NSIoPipe::EPIPE_RAW_HDRTYPE_CMD:
            case NSCam::NSIoPipe::EPIPE_TIMGO_DUMP_SEL_CMD:
                break;
            default:
                PIPE_WRN("receive extra cmd(%d)\n",CmdInfo.CmdIdx);
                break;
        }
    }


    PrintPipePortInfo(pFrameParams);

    for (MUINT32 i = 0 ; i < pFrameParams.mvIn.size() ; i++ )
    {
        //if ( NULL == pFrameParams.mvIn[i] ) { continue; }
        //
        planeNum = pFrameParams.mvIn[i].mBuffer->getPlaneCount();

        //
        switch(VirPortIdxMapToHWPortIdx(pFrameParams.mvIn[i].mPortID.index))
        {
            case EPortIndex_IMGI:    //may multi-plane
                eImgFmt_imgi = (NSCam::EImageFormat)(pFrameParams.mvIn[i].mBuffer->getImgFormat());
                pixIdImgBuf = PixelIDMapping(pFrameParams.mvIn[i].mBuffer->getColorArrangement());
                if(planeNum >= 2)    // check plane number
                { // check imgbi

                    if ((eImgFmt_imgi == eImgFmt_UFO_FG_BAYER10) || (eImgFmt_imgi == eImgFmt_UFO_FG_BAYER12) || (eImgFmt_imgi == eImgFmt_UFO_BAYER10) || (eImgFmt_imgi == eImgFmt_UFO_BAYER12))
                    {
                        if (planeNum ==3)
                        {
                            pUfdParam = (UFDG_META_INFO *)pFrameParams.mvIn[i].mBuffer->getBufVA(2);
                            PIPE_INF("UFDG_META_INFO: UFDG_BITSTREAM_OFST_ADDR(%d),UFDG_BS_AU_START(%d),UFDG_AU2_SIZE(%d),UFDG_BOND_MODE(%d)", pUfdParam->UFDG_BITSTREAM_OFST_ADDR, pUfdParam->UFDG_BS_AU_START, pUfdParam->UFDG_AU2_SIZE, pUfdParam->UFDG_BOND_MODE);
                        }
                    }
                }

                // Parsing crop information +++++
                CropInfo_imgi.crop1.x      = 0;
                CropInfo_imgi.crop1.floatX = 0;
                CropInfo_imgi.crop1.y      = 0;
                CropInfo_imgi.crop1.floatY = 0;
                CropInfo_imgi.crop1.w      = 0;
                CropInfo_imgi.crop1.floatW = 0;
                CropInfo_imgi.crop1.h      = 0;
                CropInfo_imgi.crop1.floatH = 0;
                CropInfo_imgi.resize1.tar_w = 0;
                CropInfo_imgi.resize1.tar_h = 0;
                CropInfo_imgi.crop2.x = 0;
                CropInfo_imgi.crop2.floatX = 0;
                CropInfo_imgi.crop2.y = 0;
                CropInfo_imgi.crop2.floatY = 0;
                CropInfo_imgi.crop2.w      = 0;
                CropInfo_imgi.crop2.floatW = 0;
                CropInfo_imgi.crop2.h      = 0;
                CropInfo_imgi.crop2.floatH = 0;
                CropInfo_imgi.resize2.tar_w = 0;
                CropInfo_imgi.resize2.tar_h = 0;
                CropInfo_imgi.crop3.x = 0;
                CropInfo_imgi.crop3.floatX = 0;
                CropInfo_imgi.crop3.y = 0;
                CropInfo_imgi.crop3.floatY = 0;
                CropInfo_imgi.crop3.w      = 0;
                CropInfo_imgi.crop3.floatW = 0;
                CropInfo_imgi.crop3.h      = 0;
                CropInfo_imgi.crop3.floatH = 0;
                CropInfo_imgi.resize3.tar_w = 0;
                CropInfo_imgi.resize3.tar_h = 0;
                for(MUINT32 k=0;k<pFrameParams.mvCropRsInfo.size();k++)
                {
                    //crop1/crop2 may be different in each buffer node
                        switch(pFrameParams.mvCropRsInfo[k].mGroupID)
                        {
                            case 1:    // for CRZ
                                CropInfo_imgi.crop1.x      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop1.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop1.y      = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop1.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop1.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop1.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop1.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop1.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize1.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize1.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                break;
                            case 2:    // for WDMAO
                                CropInfo_imgi.crop2.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop2.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop2.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop2.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop2.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop2.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop2.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop2.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize2.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize2.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                WdmaoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                                break;
                            case 3: // for WROTO
                                CropInfo_imgi.crop3.x = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                CropInfo_imgi.crop3.floatX = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                CropInfo_imgi.crop3.y = pFrameParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                CropInfo_imgi.crop3.floatY = pFrameParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                CropInfo_imgi.crop3.w      = pFrameParams.mvCropRsInfo[k].mCropRect.s.w;
                                CropInfo_imgi.crop3.floatW = pFrameParams.mvCropRsInfo[k].mCropRect.w_fractional;
                                CropInfo_imgi.crop3.h      = pFrameParams.mvCropRsInfo[k].mCropRect.s.h;
                                CropInfo_imgi.crop3.floatH = pFrameParams.mvCropRsInfo[k].mCropRect.h_fractional;
                                CropInfo_imgi.resize3.tar_w = (pFrameParams.mvCropRsInfo[k].mResizeDst.w == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.w):(pFrameParams.mvCropRsInfo[k].mResizeDst.w);
                                CropInfo_imgi.resize3.tar_h = (pFrameParams.mvCropRsInfo[k].mResizeDst.h == 0)?(pFrameParams.mvCropRsInfo[k].mCropRect.s.h):(pFrameParams.mvCropRsInfo[k].mResizeDst.h);
                                WrotoCropGroup = pFrameParams.mvCropRsInfo[k].mMdpGroup;
                                break;
                            default:
                                PIPE_ERR("DO not support crop group (%d)",pFrameParams.mvCropRsInfo[k].mGroupID);
                            break;
                        }
                }

                break;
            case EPortIndex_IMGBI:    //Mixing path weighting table
            case EPortIndex_IMGCI:    //Mixing path weighting table
            case EPortIndex_VIPI:    //may multi-plane, vipi~vip3i
            case EPortIndex_VIP3I: //only vip3i input(weighting table), there are only vipi+vip3i without vip2i for mfb mixing path
            case EPortIndex_LCEI:
            case EPortIndex_UFDI:
            case EPortIndex_DEPI:
            case EPortIndex_DMGI:
            case EPortIndex_REGI:
                break;
            default:
                PIPE_ERR("[Error]Not support this input port(%d),i(%d)",pFrameParams.mvIn[i].mPortID.index,i);
                break;
        }
    }


    //specific module setting
    for (MUINT32 i = 0 ; i < pFrameParams.mvModuleData.size() ; i++ )
    {
        module.eDipModule=static_cast<EDipModule>(pFrameParams.mvModuleData[i].moduleTag);
        module.moduleStruct=pFrameParams.mvModuleData[i].moduleStruct;

        switch(static_cast<EDipModule>(pFrameParams.mvModuleData[i].moduleTag))
        {
            case EDipModule_SRZ1:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(module.moduleStruct))
                {
                    this->configSrz(&srzCfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(module.moduleStruct), module.eDipModule);

                    PIPE_INF("SRZ1:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",srzCfg.inout_size.in_w,srzCfg.inout_size.in_h,\
                        srzCfg.crop.x, srzCfg.crop.floatX, srzCfg.crop.y, srzCfg.crop.floatY,\
                        srzCfg.crop.w, srzCfg.crop.h, srzCfg.inout_size.out_w,srzCfg.inout_size.out_h,\
                        srzCfg.h_step, srzCfg.v_step);
                }
                break;
            case EDipModule_SRZ3:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(module.moduleStruct)) {
                    this->configSrz(&srzCfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(module.moduleStruct), module.eDipModule);

                    PIPE_INF("SRZ3:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",srzCfg.inout_size.in_w,srzCfg.inout_size.in_h,\
                        srzCfg.crop.x, srzCfg.crop.floatX, srzCfg.crop.y, srzCfg.crop.floatY,\
                        srzCfg.crop.w, srzCfg.crop.h, srzCfg.inout_size.out_w,srzCfg.inout_size.out_h,\
                        srzCfg.h_step, srzCfg.v_step);
                }
                break;
            case EDipModule_SRZ4:
                if(reinterpret_cast<_SRZ_SIZE_INFO_*>(module.moduleStruct)) {
                    this->configSrz(&srzCfg, reinterpret_cast<_SRZ_SIZE_INFO_*>(module.moduleStruct), module.eDipModule);

                    PIPE_INF("SRZ4:in(%d,%d), crop(%d_0x%x/%d_0x%x, %lu, %lu),out(%d,%d),hstep(%d),vstep(%d)",srzCfg.inout_size.in_w,srzCfg.inout_size.in_h,\
                        srzCfg.crop.x, srzCfg.crop.floatX, srzCfg.crop.y, srzCfg.crop.floatY,\
                        srzCfg.crop.w, srzCfg.crop.h, srzCfg.inout_size.out_w,srzCfg.inout_size.out_h,\
                        srzCfg.h_step, srzCfg.v_step);
                }
                break;
            default:
                PIPE_ERR("[Error]do not support this HWmodule(%d),i(%d)",module.eDipModule,i);
                break;
        };
    }

    if(pFrameParams.mTuningData == NULL)
    {
        PIPE_WRN("Warning tuning Data size is NULL!!");
        pTuningDipReg = NULL;
    }
    else
    {
        pTuningDipReg = (dip_x_reg_t *)pFrameParams.mTuningData;
    }
    if (pWkBufDipReg != NULL)
    {
        crzEn = pWkBufDipReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Bits.DIPCTL_CRZ_D1_EN;
    }


    switch (pMdpCfg->drvScenario)
    {
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_FM:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass1:
        case NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2:
            tpipe = CAM_MODE_FRAME;
            break;
        default:
            tpipe = CAM_MODE_TPIPE;
            break;
    }

    if(tpipe == CAM_MODE_TPIPE) {
        isDipOnly = MFALSE;
        tdr_ctl |= DIPCTL_TDR_CTL_TDR_EN;
        tdr_sel |= DIPCTL_TDR_SEL_CTL_EXT_EN; //enable tpipe extension in default
    } else {
        isDipOnly = MTRUE;
        tdr_sel = 0xf0000; //TILE EDGE must be set to 1 in frame mode
    }


    if ((pTuningDipReg != NULL) && (pWkBufDipReg != NULL) && (pMdpCfg != NULL))
    {
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),p2RingBufIdx(%d),[vipi]ofst(%d),rW/H(%d/%d)\n, \
            isDipOnly(%d), dipCQ/ring/burst(%d/%d/%d), drvSc(%d), isWtf(%d), en_rgb/rgb2/yuv/yuv2/dma/dma2(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x), fmt_sel/fmt2_sel/mux_sel/mux2_sel/misc(0x%x/0x%x/0x%x/0x%x/0x%x), tcm(0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x)\n , \
            last(%d),CRZ_EN(%d),MDP_CROP(%d,%d),img3o ofset(%d,%d),NBC_ANR_CON1(0x%x),YYNR_SKIN_CON(0x%x),userName(%s),ImgBuf_PIX_ID(%d)",\
            CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
            CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
            CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
            CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
            CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
            CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
            CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
            CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
            CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,p2RingBufIdx, \
            pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_OFFSET.Raw,pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_WIDTH,pTuningDipReg->NR3D_D1A_NR3D_NR3D_VIPI_SIZE.Bits.NR3D_VIPI_HEIGHT, \
            isDipOnly,\
            p2CQ,p2RingBufIdx,pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx,\
            pMdpCfg->drvScenario,pMdpCfg->isWaitBuf,\
            pWkBufDipReg->DIPCTL_D1A_DIPCTL_RGB_EN1.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_RGB_EN2.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_YUV_EN1.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_YUV_EN2.Raw,\
            pWkBufDipReg->DIPCTL_D1A_DIPCTL_DMA_EN1.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_DMA_EN2.Raw,\
            pWkBufDipReg->DIPCTL_D1A_DIPCTL_FMT_SEL1.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_FMT_SEL2.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_MUX_SEL1.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_MUX_SEL2.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_MISC_SEL.Raw,\
            tdr_ctl, tdr_sel, pWkBufDipReg->DIPCTL_D1A_DIPCTL_TDR_TCM_EN1.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_TDR_TCM_EN2.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_TDR_TCM_EN3.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_TDR_TCM_EN4.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_TDR_TCM_EN5.Raw, pWkBufDipReg->DIPCTL_D1A_DIPCTL_TDR_TCM_EN6.Raw,\
            lastframe, crzEn, WdmaoCropGroup, WrotoCropGroup, Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y,\
            pTuningDipReg->YNR_D1A_YNR_CON1.Raw, pTuningDipReg->YNR_D1A_YNR_SKIN_CON.Raw, szCallerName,  pixIdImgBuf);
    }
    else
    {
        PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,f_w,h,f_h)=(%d,%d,%d,%d),(tarW,tarH)=(%d,%d),p2RingBufIdx(%d)\n, \
            isDipOnly(%d), dipCQ/ring/burst(%d/%d/%d), drvSc(%d), isWtf(%d)\n , \
            last(%d),CRZ_EN(%d),MDP_CROP(%d,%d),img3o ofset(%d,%d),userName(%s),ImgBuf_PIX_ID(%d)",\
            CropInfo_imgi.crop1.x,CropInfo_imgi.crop1.floatX,CropInfo_imgi.crop1.y,CropInfo_imgi.crop1.floatY, \
            CropInfo_imgi.crop1.w,CropInfo_imgi.crop1.floatW,CropInfo_imgi.crop1.h,CropInfo_imgi.crop1.floatH, \
            CropInfo_imgi.resize1.tar_w,CropInfo_imgi.resize1.tar_h, \
            CropInfo_imgi.crop2.x,CropInfo_imgi.crop2.floatX,CropInfo_imgi.crop2.y,CropInfo_imgi.crop2.floatY, \
            CropInfo_imgi.crop2.w,CropInfo_imgi.crop2.floatW,CropInfo_imgi.crop2.h,CropInfo_imgi.crop2.floatH, \
            CropInfo_imgi.resize2.tar_w,CropInfo_imgi.resize2.tar_h, \
            CropInfo_imgi.crop3.x,CropInfo_imgi.crop3.floatX,CropInfo_imgi.crop3.y,CropInfo_imgi.crop3.floatY, \
            CropInfo_imgi.crop3.w,CropInfo_imgi.crop3.floatW,CropInfo_imgi.crop3.h,CropInfo_imgi.crop3.floatH, \
            CropInfo_imgi.resize3.tar_w,CropInfo_imgi.resize3.tar_h,p2RingBufIdx, \
            isDipOnly,\
            p2CQ,p2RingBufIdx,pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx,\
            pMdpCfg->drvScenario,pMdpCfg->isWaitBuf,\
            tdr_ctl, tdr_sel,\
            lastframe, crzEn, WdmaoCropGroup, WrotoCropGroup, Img3oCrspTmp.m_CrspInfo.p_integral.x, Img3oCrspTmp.m_CrspInfo.p_integral.y,\
            szCallerName, pixIdImgBuf);
    }

    PIPE_INF("+sceID(%u),cqPa(0x%lx)-Va(0x%lx),tpipVa(0x%lx)-Pa(0x%lx),isWaitB(%d),venc(%d),\
     srcFt(0x%x),W(%u),H(%u),stride(%u,%u),size(0x%x-0x%x-0x%x),VA(0x%8x),PA(0x%8x),plane(%d)\n\
     wdma_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
     wrot_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),mdpidx(%d),\n\
     jpg_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),\n\
     pipe_en(%d),idx(%d),cap(%d),ft(%d),rt(%d),fp(%d),(w,h,s)=(%lu,%lu,%lu),C(w,h,s)=(%lu,%lu,%lu),V(w,h,s)=(%lu,%lu,%lu),\n\
     HMyo CQ(%d_%d_%d),(va:0x%lx,dstpa:0x%lx)\n,skip(0x%x),flag(0x%x),WDPQ(0x%8x),WRPQ(0x%8x)",\
             pMdpCfg->drvScenario,pMdpCfg->ispTpipeCfgInfo.drvinfo.DesCqPa,pMdpCfg->ispTpipeCfgInfo.drvinfo.DesCqVa,(unsigned long)pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa,pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa, \
             pMdpCfg->isWaitBuf,pMdpCfg->dstPortCfg[ISP_MDP_DL_VENCO],\
             pMdpCfg->mdpSrcFmt,pMdpCfg->mdpSrcW,pMdpCfg->mdpSrcH,pMdpCfg->mdpSrcYStride,pMdpCfg->mdpSrcUVStride,pMdpCfg->mdpSrcBufSize,pMdpCfg->mdpSrcCBufSize,pMdpCfg->mdpSrcVBufSize,pMdpCfg->srcVirAddr,pMdpCfg->srcPhyAddr,\
             pMdpCfg->mdpSrcPlaneNum,
             pMdpCfg->dstPortCfg[ISP_MDP_DL_WDMAO],ISP_MDP_DL_WDMAO, pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].capbility, pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].Format, pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].Rotation, pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].Flip,
             pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size.w,pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size.h,pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size_c.w,pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size_c.h,pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size_c.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size_v.w,pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size_v.h,pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].size_v.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_WDMAO].crop_group,
             pMdpCfg->dstPortCfg[ISP_MDP_DL_WROTO],ISP_MDP_DL_WROTO, pMdpCfg->dstDma[ISP_MDP_DL_WROTO].capbility, pMdpCfg->dstDma[ISP_MDP_DL_WROTO].Format, pMdpCfg->dstDma[ISP_MDP_DL_WROTO].Rotation, pMdpCfg->dstDma[ISP_MDP_DL_WROTO].Flip,
             pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size.w,pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size.h,pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size_c.w,pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size_c.h,pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size_c.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size_v.w,pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size_v.h,pMdpCfg->dstDma[ISP_MDP_DL_WROTO].size_v.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_WROTO].crop_group,
             pMdpCfg->dstPortCfg[ISP_MDP_DL_VENCO],ISP_MDP_DL_VENCO, pMdpCfg->dstDma[ISP_MDP_DL_VENCO].capbility, pMdpCfg->dstDma[ISP_MDP_DL_VENCO].Format, pMdpCfg->dstDma[ISP_MDP_DL_VENCO].Rotation, pMdpCfg->dstDma[ISP_MDP_DL_VENCO].Flip,
             pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size.w,pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size.h,pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size_c.w,pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size_c.h,pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size_c.stride,
             pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size_v.w,pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size_v.h,pMdpCfg->dstDma[ISP_MDP_DL_VENCO].size_v.stride,
             pMdpCfg->dstPortCfg[ISP_ONLY_OUT_TPIPE],ISP_ONLY_OUT_TPIPE, pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].capbility, pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].Format, pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].Rotation, pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].Flip,
             pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size.w,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size.h,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size.stride,
             pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size_c.w,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size_c.h,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size_c.stride,
             pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size_v.w,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size_v.h,pMdpCfg->dstDma[ISP_ONLY_OUT_TPIPE].size_v.stride,
             pMdpCfg->ispTpipeCfgInfo.drvinfo.cqIdx,pMdpCfg->ispTpipeCfgInfo.drvinfo.RingBufIdx, pMdpCfg->ispTpipeCfgInfo.drvinfo.burstCqIdx,\
             (unsigned long)pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTableVa, \
             pMdpCfg->ispTpipeCfgInfo.drvinfo.tpipeTablePa, pMdpCfg->ispTpipeCfgInfo.drvinfo.frameflag, WDMAPQEnable, WROTPQEnable);

    PIPE_ERR("dumpFailFrameSetting End!!!\n");

    return MTRUE;
}

MUINT32
PostProcPipe::
//queryCQ(EDrvScenario sDrvScen, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum, MUINT32& RingBufIndex, MUINTPTR& pVirIspAddr)
queryCQ(MUINT32 StreamTag, MUINT32 sensorDev, MINT32 p2burstQIdx, MINT32 frameNum, MUINT32& RingBufIndex, MUINTPTR& pVirIspAddr)
{
    MUINT32 CQset=0x0;
    MINT32 p2CQ=0;
    pVirIspAddr = 0x0;

    if (StreamTag == NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Vss)
    {
        p2CQ=static_cast<MUINT32>(ISP_DRV_DIP_CQ_THRE1);
    }
    else
    {
        p2CQ=static_cast<MUINT32>(ISP_DRV_DIP_CQ_THRE0);
    }
    //
   while(!m_pDipRingBuf->checkDipWorkingBufferAvaiable((E_ISP_DIP_CQ)p2CQ))
   {
       PIPE_INF("Block Enqueue Here, P2CQ (%d), StreamTag/SenDev(0x%x/0x%x)", p2CQ, StreamTag, sensorDev);
   }

   //Acquire the buffer from ring buffer
   m_pDipWorkingBuf = m_pDipRingBuf->AcquireDipWorkingBuffer((E_ISP_DIP_CQ)p2CQ);
   if(m_pDipWorkingBuf == NULL)
   {
       PIPE_ERR("AcquireDipWorkingBuffer error!!\n");
       return MFALSE; //error command  enque
   }
   pVirIspAddr = (MUINTPTR)m_pDipWorkingBuf->m_pIspVirRegAddr_va;
   RingBufIndex = m_pDipWorkingBuf->m_BufIdx;

   //dupCQIdx = m_pDipWorkingBuf->m_BufIdx;
#if 0
   dupCQIdx = mp2CQDupIdx;

   //reset the list if all frames in package are enqued done
   if(p2burstQIdx == (frameNum-1))
   {
       mp2CQDupIdx = (mp2CQDupIdx+1)  % (MAX_DUP_CQ_NUM); //update at last frame in the same package
   }
   CQset = (p2CQ<<16) | dupCQIdx;
#endif

   CQset = p2CQ;

   PIPE_DBG("StreamTag:%d, p2burstQIdx:%d, frameNum:%d, RingBufIndex:%d\n", StreamTag, p2burstQIdx, frameNum, RingBufIndex);

   return CQset;
}

MBOOL
PostProcPipe::
updateCQUser(MINT32 p2CQ)
{
    MBOOL ret=MTRUE;
    ret=m_dipCmdQMgr->releaseCQUser(p2CQ);
    return ret;
}

MBOOL
PostProcPipe::
doUVSwap(MUINTPTR *uv1, MUINTPTR *uv2)
{
    MUINTPTR tmpUV;
    tmpUV = *uv1;
    *uv1 = *uv2;
    *uv2 = tmpUV;
    return MTRUE;
}
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
