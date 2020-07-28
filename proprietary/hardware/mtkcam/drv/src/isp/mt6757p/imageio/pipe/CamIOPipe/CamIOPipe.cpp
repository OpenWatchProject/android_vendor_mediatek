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
#define LOG_TAG "camiopipe"

//
#include <mtkcam/utils/std/Log.h>
//
#include "CamIOPipe.h"
#include "StatisticIOPipe.h"//for icamiopipe::createinstance & icamiopipe::destroyinstance
#include "CamsvIOPipe.h"

//
#include <cutils/properties.h>  // For property_get().
//#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h> // for p1hwcfg module sel
#include <ispio_utility.h>

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>  // for irq status mapping

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(pipe);

#undef PIPE_DBG
#undef PIPE_INF
#undef PIPE_ERR


#define PIPE_DBG(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define PIPE_INF(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define PIPE_ERR(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)


/*******************************************************************************
* LOCAL PRIVATE FUNCTION
********************************************************************************/
#if 0

MUINT32 _p1GetModuleTgIdx(MUINT32 id,MBOOL bTwin) {\
    switch (id) {
        case EModule_AE     :
        case EModule_AWB    :
        case EModule_SGG1   :
        case EModule_FLK    :
        case EModule_AF     :
        case EModule_SGG2   :
        case EModule_LCS    :
        case EModule_AAO    :
        case EModule_ESFKO  :
        case EModule_AFO    :
        case EModule_EISO   :
        case EModule_LCSO   :
        case EModule_iHDR   :
             return EPortIndex_TG1I;
             break;
        case EModule_AE_D   :
        case EModule_AWB_D  :
        case EModule_SGG1_D :
        case EModule_AF_D   :
        case EModule_LCS_D  :
        case EModule_AAO_D  :
        case EModule_AFO_D  :
        case EModule_LCSO_D :
        case EModule_iHDR_D :
             if (ePxlMode_Two_Twin == bTwin){
                  return EPortIndex_TG1I;
             }
             else {
                  return EPortIndex_TG2I;
             }
             break;
        default:
            PIPE_ERR("Not support module(%d)",id);
            return EPortIndex_UNKNOW;
            break;
    }
}


MUINT32 p1ConvModuleID(MUINT32 id) {return 0;}

HWRWCTRL_P1SEL_MODULE p1ConvModuleToSelectID(MUINT32 id) {

   HWRWCTRL_P1SEL_MODULE newId = HWRWCTRL_P1SEL_RSVD;

    switch (id) {
        //raw
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG:    newId = HWRWCTRL_P1SEL_SGG; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN: newId = HWRWCTRL_P1SEL_SGG_EN; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS:    newId = HWRWCTRL_P1SEL_LSC; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN: newId = HWRWCTRL_P1SEL_LSC_EN; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG:    newId = HWRWCTRL_P1SEL_IMG; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE:    newId = HWRWCTRL_P1SEL_UFE; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G:    newId = HWRWCTRL_P1SEL_W2G; break;

        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_D:    newId = HWRWCTRL_P1SEL_SGG_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN_D: newId = HWRWCTRL_P1SEL_SGG_EN_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_D:     newId = HWRWCTRL_P1SEL_LSC_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN_D:  newId = HWRWCTRL_P1SEL_LSC_EN_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG_D:     newId = HWRWCTRL_P1SEL_IMG_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE_D:     newId = HWRWCTRL_P1SEL_UFE_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G_D:     newId = HWRWCTRL_P1SEL_W2G_D; break;

        default:
            PIPE_ERR("p1ConvModuleToSelectID::NotSupportModule(%d). Use Enum EPipe_P1SEL",id);
    }
    return newId;
}


MINT32 _p1ConvSelectIdToTg(MUINT32 id,MBOOL btwin) {

    switch (id) {
        //raw
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G:
            return EPortIndex_TG1I;

        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G_D:
            if (MTRUE == btwin){
                 return  EPortIndex_TG1I;
            }
            else {
                 return EPortIndex_TG2I;
            }

        default:
            CAM_LOGE("_p1ConvSelectIdToTg::NotSupportModule(%d). Use Enum EPipe_P1SEL",id);
            return  EPortIndex_UNKNOW;
    }
    return  EPortIndex_TG1I;
}
#endif


//////////////////////////////////////////////////////////////////////////////

/*******************************************************************************
*
********************************************************************************/
ICamIOPipe* ICamIOPipe::createInstance( MINT8 const szUsrName[32],E_INPUT InPut,E_PIPE_OPT opt)
{
    switch(InPut){
        case TG_A:
        case TG_B:
        case TG_CAMSV_0:
        case TG_CAMSV_1:
        case TG_CAMSV_2:
        case TG_CAMSV_3:
        case TG_CAMSV_4:
        case TG_CAMSV_5:
            break;
        default:
            BASE_LOG_ERR("unsupported input source:0x%x\n",InPut);
            return NULL;
            break;
    }

    switch(opt){
        case CAMIO:
            return  (ICamIOPipe*)CamIOPipe::Create(szUsrName,InPut);
            break;
        case STTIO:
            return  (ICamIOPipe*)STTIOPipe::Create(szUsrName,InPut);
            break;
        case CAMSVIO:
            return (ICamIOPipe*)CamsvIOPipe::Create(szUsrName,InPut);
            break;
        default:
            BASE_LOG_ERR("unsupported pipe:0x%x\n",opt);
            return NULL;
            break;
    }

}

ICamIOPipe* ICamIOPipe::getInstance(E_PIPE_OPT opt, E_INPUT InPut)
{
    CamIOPipe* ptr = NULL;

    switch(opt){
        case CAMIO:
            return  (ICamIOPipe*)CamIOPipe::getInstance(InPut);
            break;
        default:
            BASE_LOG_ERR("unsupported pipe:0x%x\n",opt);
            return NULL;
            break;
    }
}

MVOID ICamIOPipe::destroyInstance(void)
{
    switch(this->m_pipe_opt){
        case CAMIO:
            ((CamIOPipe*)this)->Destroy();
            break;
        case STTIO:
            ((STTIOPipe*)this)->Destroy();
            break;
        case CAMSVIO:
            ((CamsvIOPipe*)this)->Destroy();
            break;
        default:
            BASE_LOG_ERR("unsupported pipe:0x%x\n",this->m_pipe_opt);
            break;
    }
}

/*******************************************************************************
*
********************************************************************************/
CamIOPipe::CamIOPipe()
{
    //
    DBG_LOG_CONFIG(imageio, pipe);


    m_szUsrName[0] = '\0';


    m_FSM = op_unknown;
    occupied = MFALSE;
    m_bStartUniStreaming = MFALSE;
    m_pCmdQMgr = NULL;
    m_pHighSpeedCmdQMgr = NULL;
    m_pipe_opt = ICamIOPipe::CAMIO;
    m_DynamicRawType = MFALSE;
    m_bUpdate = MFALSE;
    m_FastAF = MFALSE;
    m_Stt_rstBufCtrl_CBFP = NULL;
    m_TwinMgr = NULL;
    m_hwModule = UNKNOWN_HW;
    m_Stt_CBCookie = NULL;
    PIPE_INF(":X");
}

CamIOPipe::~CamIOPipe()
{
}

/*******************************************************************************
*
********************************************************************************/


MBOOL CamIOPipe::FSM_CHECK(MUINT32 op)
{
    MBOOL ret = MTRUE;
    this->m_FSMLock.lock();
    switch(op){
        case op_unknown:
            if(this->m_FSM != op_uninit)
                ret = MFALSE;
            break;
        case op_init:
            if(this->m_FSM != op_unknown)
                ret = MFALSE;
            break;
        case op_cfg:
            if(this->m_FSM != op_init)
                ret = MFALSE;
            break;
        case op_start:
            if(this->m_FSM != op_cfg)
                ret = MFALSE;
            break;
        case op_stop:
            if(this->m_FSM != op_start)
                ret = MFALSE;
            break;
        case op_uninit:
            switch(this->m_FSM){
                case op_init:
                case op_cfg:
                case op_stop:
                    break;
                default:
                    ret = MFALSE;
                    break;
            }
            break;
        case op_cmd:
            switch(this->m_FSM){
                case op_cfg:
                case op_start:
                case op_stop:
                    break;
                default:
                    ret= MFALSE;
                    break;
            }
            break;
        case op_endeq:
            switch(this->m_FSM){
                case op_cfg:
                case op_start:
                    break;
                default:
                    ret= MFALSE;
                    break;
            }
            break;
        default:
            ret = MFALSE;
            break;
    }
    if(ret == MFALSE)
        PIPE_ERR("op error:cur:0x%x,tar:0x%x\n",this->m_FSM,op);
    this->m_FSMLock.unlock();
    return ret;
}

MBOOL CamIOPipe::FSM_UPDATE(MUINT32 op)
{
    if(op == op_cmd)
        return MTRUE;
    //
    this->m_FSMLock.lock();
    this->m_FSM = (E_FSM)op;
    this->m_FSMLock.unlock();
    return MTRUE;
}

//return HW register format
MINT32 CamIOPipe::getOutPxlByteNFmt( MUINT32 bRRZO, ImgInfo::EImgFmt_t imgFmt, MINT32* pPixel_byte, MINT32* pFmt)
{
    //
    if ( NULL == pPixel_byte ) {
        PIPE_ERR("ERROR:NULL pPixel_byte");
        return -1;
    }
    //
    if (bRRZO)    {
        switch(imgFmt){
            case eImgFmt_FG_BAYER8:
            case eImgFmt_FG_BAYER10:
            case eImgFmt_FG_BAYER12:
            case eImgFmt_UFEO_BAYER10://UFO
            case eImgFmt_UFEO_BAYER12: //UFO
                break;
            default:
                PIPE_ERR("RRZO support only: eImgFmt_FG_BAYER8/10/12\n");
                return 1;
                break;

        }
    }


    //
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = IMGO_FMT_RAW8;
            break;
        case eImgFmt_FG_BAYER8:
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = RRZO_FMT_RAW8;
            break;
        case eImgFmt_BAYER10_MIPI:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = IMGO_FMT_RAW10_MIPI;
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = IMGO_FMT_RAW10;
            break;
        case eImgFmt_FG_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = RRZO_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = IMGO_FMT_RAW12;
            break;
        case eImgFmt_FG_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = RRZO_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            if(bRRZO){
                PIPE_ERR("supported no 14bit at this dmao\n");
                return 1;
            }
            *pPixel_byte = (7 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 2 pixels-> 3 bytes, 1.5
            *pFmt = IMGO_FMT_RAW14;
            break;
        case eImgFmt_BAYER8_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            *pFmt = IMGO_FMT_RAW8_2B;
            break;
        case eImgFmt_BAYER10_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            *pFmt = IMGO_FMT_RAW10_2B;
            break;
        case eImgFmt_BAYER12_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            *pFmt = IMGO_FMT_RAW12_2B;
            break;
        case eImgFmt_BAYER14_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            *pFmt = IMGO_FMT_RAW14_2B;
            break;
        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = IMGO_FMT_YUV422_1P;
            break;
        case eImgFmt_RGB565:
            //*pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            //*pFmt = CAM_FMT_SEL_RGB565;
            //break;
        case eImgFmt_RGB888:
            //*pPixel_byte = 3 << CAM_ISP_PIXEL_BYTE_FP;
            //*pFmt = CAM_FMT_SEL_RGB888;
            //break;
        case eImgFmt_JPEG:
        case eImgFmt_BLOB:
            //*pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            //*pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW10):(CAM_FMT_SEL_BAYER10);
            //break;
        case eImgFmt_UFO_BAYER10:
            //*pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            //*pFmt = CAM_FMT_SEL_TG_FMT_RAW10;
            //break;
       case eImgFmt_UFEO_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = RRZO_FMT_RAW10;
            break;
        case eImgFmt_UFEO_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = RRZO_FMT_RAW12;
            break;
        default:
            PIPE_ERR("eImgFmt:[%d]NOT Support",imgFmt);
            return -1;
    }
    //
    PIPE_INF("input imgFmt(0x%x),output fmt:0x%x,*pPixel_byte(%d)",imgFmt,*pFmt,*pPixel_byte);
    //PIPE_INF(" ");

    return 0;
}


MINT32 CamIOPipe::SrcFmtToHwVal_TG( ImgInfo::EImgFmt_t imgFmt)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8: return TG_FMT_RAW8;
            break;
        case eImgFmt_BAYER10:return TG_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:return TG_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:return TG_FMT_RAW14;
            break;
        //case eImgFmt_RGB565: return TG_FMT_RGB565;
        //case eImgFmt_RGB888: return TG_FMT_RGB888;
        //case eImgFmt_JPEG:   return TG_FMT_JPG;

        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
             return TG_FMT_YUV422;
            break;
        default:
            PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
            return -1;
            break;
    }
    return -1;
}


MINT32 CamIOPipe::HwValToSrcFmt_TG( MUINT32 portid,MUINT32 imgFmt)
{
    MUINT32 bit = 0;
    switch(portid){
        case EPortIndex_IMGO:
            switch (imgFmt) {
                case IMGO_FMT_RAW8: return eImgFmt_BAYER8;
                    break;
                case IMGO_FMT_RAW10_MIPI:return eImgFmt_BAYER10_MIPI;
                    break;
                case IMGO_FMT_RAW10:return eImgFmt_BAYER10;
                    break;
                case IMGO_FMT_RAW12:return eImgFmt_BAYER12;
                    break;
                case IMGO_FMT_RAW14:return eImgFmt_BAYER14;
                    break;
                case IMGO_FMT_RAW8_2B:return eImgFmt_BAYER8_UNPAK;
                    break;
                case IMGO_FMT_RAW10_2B:return eImgFmt_BAYER10_UNPAK;
                    break;
                case IMGO_FMT_RAW12_2B:return eImgFmt_BAYER12_UNPAK;
                    break;
                case IMGO_FMT_RAW14_2B:return eImgFmt_BAYER14_UNPAK;
                    break;
                //case eImgFmt_RGB565: return TG_FMT_RGB565;
                //case eImgFmt_RGB888: return TG_FMT_RGB888;
                //case eImgFmt_JPEG:   return TG_FMT_JPG;

                case IMGO_FMT_YUV422_1P:
                //case eImgFmt_UYVY:
                //case eImgFmt_YVYU:
                //case eImgFmt_VYUY:
                     return eImgFmt_YUY2;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                    return -1;
                    break;
            }
            break;
        case EPortIndex_RRZO:
            bit = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,UFEG_EN);
                if(bit == 0){
            switch (imgFmt) {
                case RRZO_FMT_RAW8:return eImgFmt_FG_BAYER8;
                    break;
                case RRZO_FMT_RAW10:return eImgFmt_FG_BAYER10;
                    break;
                case RRZO_FMT_RAW12:return eImgFmt_FG_BAYER12;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                    return -1;
                    break;
            }
                }
                else{
                    switch (imgFmt) {
                        case RRZO_FMT_RAW10:return eImgFmt_UFEO_BAYER10;
                            break;
                        case RRZO_FMT_RAW12:return eImgFmt_UFEO_BAYER12;
                            break;
                    default:
                        PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                        return -1;
                        break;
                    }
                }
        break;
        case EPortIndex_UFEO:
            switch (imgFmt) {

                case RRZO_FMT_RAW10:return eImgFmt_UFEO_BAYER10;
                    break;
                case RRZO_FMT_RAW12:return eImgFmt_UFEO_BAYER12;
                    break;

                default:
                    PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                    return -1;
                    break;
            }
            break;
        default:
            PIPE_DBG("un-supported portID:0x%x\n",portid);
            break;
    }


    return -1;
}


MINT32 CamIOPipe::SrcFmtToHwVal_DMA( ImgInfo::EImgFmt_t imgFmt,MUINT32* pPixel_byte)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8:
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            return RAWI_FMT_RAW8;
            break;
        case eImgFmt_BAYER10_MIPI:
        case eImgFmt_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            return RAWI_FMT_RAW10;
            break;
        case eImgFmt_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            return RAWI_FMT_RAW12;
            break;
        case eImgFmt_BAYER14:
            *pPixel_byte = (7 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 2 pixels-> 3 bytes, 1.5
            return RAWI_FMT_RAW14;
            break;
        case eImgFmt_BAYER8_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            return RAWI_FMT_RAW8_2B;
            break;
        case eImgFmt_BAYER10_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            return RAWI_FMT_RAW10_2B;
            break;
        case eImgFmt_BAYER12_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            return RAWI_FMT_RAW12_2B;
            break;
        case eImgFmt_BAYER14_UNPAK:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP; // 1 pixels-> 2 bytes, 2
            return RAWI_FMT_RAW14_2B;
            break;
        default:
            PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
            return -1;
            break;
    }
    return -1;
}



MINT32 CamIOPipe::HwValToSrcFmt_DMA( MUINT32 portid,MUINT32 imgFmt)
{
    MUINT32 bit = 0;
    switch(portid){
        case EPortIndex_IMGO:
            switch (imgFmt) {
                case IMGO_FMT_RAW8: return eImgFmt_BAYER8;
                    break;
                case IMGO_FMT_RAW10_MIPI:return eImgFmt_BAYER10_MIPI;
                    break;
                case IMGO_FMT_RAW10:return eImgFmt_BAYER10;
                    break;
                case IMGO_FMT_RAW12:return eImgFmt_BAYER12;
                    break;
                case IMGO_FMT_RAW14:return eImgFmt_BAYER14;
                    break;
                case IMGO_FMT_RAW8_2B:return eImgFmt_BAYER8_UNPAK;
                    break;
                case IMGO_FMT_RAW10_2B:return eImgFmt_BAYER10_UNPAK;
                    break;
                case IMGO_FMT_RAW12_2B:return eImgFmt_BAYER12_UNPAK;
                    break;
                case IMGO_FMT_RAW14_2B:return eImgFmt_BAYER14_UNPAK;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                    return -1;
                    break;
            }
            break;
        case EPortIndex_RRZO:
                bit = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,UFEG_EN);
			if(bit == 0){
            switch (imgFmt) {
                case RRZO_FMT_RAW8:return eImgFmt_FG_BAYER8;
                    break;
                case RRZO_FMT_RAW10:return eImgFmt_FG_BAYER10;
                    break;
                case RRZO_FMT_RAW12:return eImgFmt_FG_BAYER12;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                    return -1;
                    break;
            }
			}
			else{
				switch (imgFmt) {
					case RRZO_FMT_RAW10:return eImgFmt_UFEO_BAYER10;
						break;
					case RRZO_FMT_RAW12:return eImgFmt_UFEO_BAYER12;
						break;
					default:
						PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
						return -1;
				break;
				}
			}
			break;
		case EPortIndex_UFEO:
                switch (imgFmt) {
                    case RRZO_FMT_RAW10:return eImgFmt_UFEO_BAYER10;
                        break;
                    case RRZO_FMT_RAW12:return eImgFmt_UFEO_BAYER12;
                        break;

                    default:
                        PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                        return -1;
                        break;
                }
            break;
        default:
            PIPE_DBG("un-supported portID:0x%x\n",portid);
            break;
    }
    return -1;
}


MINT32 CamIOPipe::SrcPixIDToHwVal(ImgInfo::ERawPxlID_t pixId)
{
    switch(pixId){
        case ERawPxlID_B:   return B_G_G_R;
            break;
        case ERawPxlID_Gb:  return G_B_R_G;
            break;
        case ERawPxlID_Gr:  return G_R_B_G;
            break;
        case ERawPxlID_R:   return R_G_G_B;
            break;
        default:
            PIPE_ERR("pixId:[%d] NOT Support",pixId);
            return -1;
            break;
    }
    return -1;
}

MINT32 CamIOPipe::HwValToSrcPixID(MUINT32 pixId)
{
    switch(pixId){
        case B_G_G_R:   return ERawPxlID_B;
            break;
        case G_B_R_G:  return ERawPxlID_Gb;
            break;
        case G_R_B_G:  return ERawPxlID_Gr;
            break;
        case R_G_G_B:   return ERawPxlID_R;
            break;
        default:
            PIPE_ERR("pixId:[%d] NOT Support",pixId);
            return -1;
            break;
    }
    return -1;
}


MINT32 CamIOPipe::PxlMode(EPxlMode pxlMode,ImgInfo::EImgFmt_t fmt, MINT32* bTwin, MINT32* two_pxl, MINT32* en_p1, MINT32* two_pxl_dmx)
{
#if 0
    switch (pxlMode) {
        case ePxlMode_One_Single:
            break;
        case ePxlMode_One_Twin:
            *bTwin = 1;
            *en_p1 |= (CAM_CTL_EN_P1_DMX_EN|CAM_CTL_EN_P1_RMX_EN|CAM_CTL_EN_P1_BMX_EN);
            break;
        case ePxlMode_Two_Single:
        case ePxlMode_Two_Twin:
            if (eImgFmt_YUY2==fmt || eImgFmt_YVYU==fmt || eImgFmt_VYUY==fmt || eImgFmt_UYVY==fmt) {
                PIPE_ERR("ERROR:Not support YUV+2-pixel");
                return -1;
            }

            if ( ePxlMode_Two_Single == pxlMode ) {
                *two_pxl = 1;
                *en_p1 |= CAM_CTL_EN_P1_WBN_EN; //CAM_CTL_EN_P1_D_WBN_D_EN == CAM_CTL_EN_P1_WBN_EN
                *two_pxl_dmx = 1;
            }
            else if ( ePxlMode_Two_Twin == pxlMode ) {
                *bTwin = 1;
                *two_pxl = 1;
                *en_p1 |= (CAM_CTL_EN_P1_DMX_EN|CAM_CTL_EN_P1_RMX_EN|CAM_CTL_EN_P1_BMX_EN);
            }
            break;
        default:
            PIPE_ERR("Not Support pxlMode(%d)",pxlMode);
            return -1;
    }
    return 0;
#else
    pxlMode;fmt;bTwin;two_pxl;en_p1;two_pxl_dmx;
    return 0;
#endif
}

//twin mode case is not take into consideration here yet
//no UFE
//no before LSC
MINT32 CamIOPipe::pathSelCtrl(MBOOL bPureRaw,MBOOL bPak,MBOOL bRawI,MUINT32 data_pat){

    this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw = 0x0;

    if(bRawI){  //DMAI

        //DMX_SEL
        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= DMX_SEL_2;
    }
    else{   // TG IN.

        //DMX_SEL
        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= DMX_SEL_0;
    }

    if(this->m_camPass1Param.bypass_ispRawPipe == MFALSE){    //bayer

        if(this->m_camPass1Param.bypass_rrzo)
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= HDS1_SEL_0;
        else
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= HDS1_SEL_1;

        if(this->m_camPass1Param.bypass_ufeo){
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= UFEG_SEL_0;
        }
        else{
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= UFEG_SEL_1;
        }
        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= SGG_SEL_1;

        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= LCS_SEL_1;

        //IMG_SEL + UFE_SEL
        if(bPureRaw){
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= IMG_SEL_1;

            if(bPak)
                this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_0;
            else
                this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_2;
        }
        else{
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= IMG_SEL_0;
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_0;
        }

        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= FLK1_SEL_0;
    }
    else{   // YUV

        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= HDS1_SEL_0;

        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= SGG_SEL_0;

        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= LCS_SEL_0;

        //IMG_SEL + UFE_SEL
        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= IMG_SEL_1;
        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= UFE_SEL_2;

        this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= FLK1_SEL_1;
    }

    //PMX & RCP3 here is fixed , currently, support no CAC
    switch(data_pat){
        case ECamio_Dual_pix:
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= PMX_SEL_1;
            break;
        case ECamio_Normal:
        default:
            this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= PMX_SEL_0;
            break;
    }
    this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= RCP3_SEL_0;

    //bmx_sel is fixed currently, default is before lsc
    this->m_camPass1Param.m_top_ctl.CTRL_SEL.Raw |= BMX_SEL_0;

    return 0;
}

/*******************************************************************************
*
********************************************************************************/

static CamIOPipe gCamioPipe[CAM_MAX];
CamIOPipe* CamIOPipe::Create(MINT8 const szUsrName[32], E_INPUT InPut)
{
    CamIOPipe* ptr = NULL;
    BASE_LOG_INF(":E:user:%s, with input source:0x%x\n",szUsrName,InPut);

    switch(InPut){
        case TG_A:
            if(gCamioPipe[CAM_A].occupied == MTRUE){
                BASE_LOG_ERR("TG_A is occupied by user:%s\n",gCamioPipe[CAM_A].m_szUsrName);
            }
            else{
                gCamioPipe[CAM_A].m_hwModule = CAM_A;
                gCamioPipe[CAM_A].occupied = MTRUE;
                std::strncpy((char*)gCamioPipe[CAM_A].m_szUsrName,(char const*)szUsrName,strlen((char const*)szUsrName));

                ptr = &gCamioPipe[CAM_A];
            }
            break;
        case TG_B:
            if(gCamioPipe[CAM_B].occupied == MTRUE){
                BASE_LOG_ERR("TG_B is occupied by user:%s\n",gCamioPipe[CAM_B].m_szUsrName);
            }
            else{
                gCamioPipe[CAM_B].m_hwModule = CAM_B;
                gCamioPipe[CAM_B].occupied = MTRUE;
                std::strncpy((char*)gCamioPipe[CAM_B].m_szUsrName,(char const*)szUsrName,strlen((char const*)szUsrName));

                ptr = &gCamioPipe[CAM_B];
            }
            break;
        default:
            BASE_LOG_ERR("unsupported input source:0x%x\n",InPut);
            break;
    }
    if(ptr == NULL)
        return NULL;
    //
    ptr->m_CamIO_stt.pCamPath = &ptr->m_CamPathPass1;
    ptr->m_CamIO_stt.m_hwModule = ptr->m_hwModule;

    return ptr;
}

void CamIOPipe::Destroy(void)
{
    PIPE_INF(":E:user:%sx\n",this->m_szUsrName);

    if(this->FSM_CHECK(op_unknown) == MFALSE)
        PIPE_ERR("FSM check error, destroy may be abnormal\n");

    this->occupied = MFALSE;

    //
    if(this->m_TwinMgr->getIsTwin() == MTRUE) {
        list<ISP_HW_MODULE>::iterator it;
        for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
            if(gCamioPipe[*it].occupied == MTRUE){
                gCamioPipe[*it].occupied = MFALSE;
            }
            else{
                PIPE_ERR("twin destroy fail,module(%d) is not occupied,user:%s\n", *it, \
                    gCamioPipe[*it].m_szUsrName);
            }
            gCamioPipe[*it].m_szUsrName[0] = '\0';
        }
    }
    this->m_TwinMgr->destroyInstance();

    this->m_camPass1Param.bypass_tg = MFALSE;
    this->m_camPass1Param.bypass_imgo = MTRUE;
    this->m_camPass1Param.bypass_rrzo = MTRUE;
    this->m_camPass1Param.bypass_ispRawPipe = MFALSE;
    this->m_camPass1Param.bypass_ispYuvPipe = MTRUE;

    this->m_szUsrName[0] = '\0';

    this->m_FSMLock.lock();
    this->m_FSM = op_unknown;
    this->m_FSMLock.unlock();
    this->mRunTimeCfgLock.lock();
    this->m_bStartUniStreaming = MFALSE;
    this->mRunTimeCfgLock.unlock();
    this->m_pCmdQMgr = NULL;
    this->m_pHighSpeedCmdQMgr = NULL;
    this->m_DynamicRawType =  MFALSE;
    this->m_bUpdate = MFALSE;
}

/*******************************************************************************
*
********************************************************************************/
CamIOPipe* CamIOPipe::getInstance(E_INPUT InPut)
{
    CamIOPipe* ptr = NULL;
    switch(InPut){
        case TG_A:
            if(gCamioPipe[CAM_A].occupied == MFALSE)
                BASE_LOG_ERR("TG_A is not occupied by user\n");
            else
                ptr = &gCamioPipe[CAM_A];
            break;
        case TG_B:
            if(gCamioPipe[CAM_B].occupied == MFALSE)
                BASE_LOG_ERR("TG_B is not occupied by user:%s\n");
            else
                ptr = &gCamioPipe[CAM_B];
            break;
        default:
            BASE_LOG_ERR("unsupported input source:0x%x\n",InPut);
            break;
    }

    return ptr;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::init()
{
    PIPE_INF("(%s):E\n",this->m_szUsrName);
    if(this->m_szUsrName[0] == '0'){
        PIPE_ERR("can't null username\n");
        return MFALSE;
    }
    if(this->FSM_CHECK(op_init) == MFALSE)
        return MFALSE;
    //attach cmdQMgr
    //move into configPipe,  because of some input para.
    //this->m_pCmdQMgr->CmdQMgr_attach(UINT32 fps,MUINT32 subsample,ISP_HW_MODULE module,E_ISP_CAM_CQ cq,void * cq_ptr)

    //attach uni_mgr
    //move into configpipe when rawi, otherwise, put in sendcommand.

    //init twin_mgr
    //remove to configpipe because of some input para.
    //this->m_TwinMgr->initTwin(...);
    this->FSM_UPDATE(op_init);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::uninit()
{
    PIPE_INF("(%s):E\n",this->m_szUsrName);

    if(this->FSM_CHECK(op_uninit) == MFALSE)
        return MFALSE;

    //mutex protection is for isp stop & uni_stop which is driven by sendcommand
    this->mRunTimeCfgLock.lock();

    if(MTRUE == this->m_bStartUniStreaming){
        PIPE_DBG("detach UniMgr +\n");
        if(this->m_UniMgr.UniMgr_detach(this->m_hwModule) != 0){
            this->mRunTimeCfgLock.unlock();
            return MFALSE;
        }
        PIPE_DBG("detach UniMgr -\n");
        this->m_bStartUniStreaming = MFALSE;
    }

    this->mRunTimeCfgLock.unlock();

    //datach cmdqMgr
    if(this->m_pCmdQMgr){
        if(this->m_pCmdQMgr->CmdQMgr_detach()){
            PIPE_ERR("uninit fail\n");
            return MFALSE;
        }
    }
    //detach cmdq11
    if(this->m_pHighSpeedCmdQMgr){
        if(this->m_pHighSpeedCmdQMgr->CmdQMgr_detach()){
            PIPE_ERR("highspeedCQ uninit fail\n");
            return MFALSE;
        }
    }

    if(this->m_TwinMgr->getIsTwin()){
        if(this->m_TwinMgr->uninitTwin() == MFALSE){
            PIPE_ERR("twinmgr unint fail\n");
            return MFALSE;
        }
    }


    //detach uni_mgr
    if(this->m_camPass1Param.bypass_tg == MTRUE){
        if(this->m_UniMgr.UniMgr_detach(this->m_hwModule)){
            PIPE_ERR("uninit fail\n");
            return MFALSE;
        }
    }

    this->FSM_UPDATE(op_uninit);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::start()
{
    void* ptr = NULL;

    if(this->FSM_CHECK(op_start) == MFALSE)
        return MFALSE;
    //uni start with streaming
    this->mRunTimeCfgLock.lock();
    if(this->m_bStartUniStreaming){
        if(this->m_UniMgr.UniMgr_start(this->m_hwModule)){
            PIPE_ERR("uni start fail\n");
            this->mRunTimeCfgLock.unlock();
            return MFALSE;
        }
        ptr = (void*)this->m_UniMgr.UniMgr_GetCurObj();
        PIPE_INF(":E, uni link with current cam\n");
    } else
        PIPE_INF(":E\n");
    this->mRunTimeCfgLock.unlock();

    //because of whole control flow issue, EIS can't pass information at configpipe.
    //And it's no gurrentee that all notified-module can book notify before rrz/updatep1 ,
    //so ,need to involk notify before pass1 start and flush the setting into phy addr
    if(this->m_bUpdate == MFALSE){
        if(0 != this->m_CamPathPass1.setP1Notify()){
            PIPE_ERR("some notified-module fail\n");
        }
    }

    // If twin is enable, call startTwin() of TwinMgr to
    // 1. Compute the CQ data via twin_drv (dual_isp_driver)
    // 2. Only twin hwModule: flush whole isp setting into phy address
    if(MTRUE == this->m_TwinMgr->getIsTwin()){
        if(MFALSE == this->m_TwinMgr->startTwin()){
            PIPE_ERR("Start Twin fail\n");
            return MFALSE;
        }
    }

    //flush whole isp setting into phy address
    //enforce cq load into reg before VF_EN if update rrz/tuning is needed
    if(this->m_pCmdQMgr->CmdQMgr_start()){
        PIPE_ERR("CQ start fail\n");
        return MFALSE;
    }

    if(this->m_CamPathPass1.start(ptr)){
        PIPE_ERR("isp start fail\n");
        return MFALSE;
    }

    //
    if(this->m_camPass1Param.bypass_tg == MTRUE){
        if(this->m_UniMgr.UniMgr_start(this->m_hwModule)){
            PIPE_ERR("rawi start fail\n");
            return MFALSE;
        }
    }

    this->FSM_UPDATE(op_start);
    PIPE_DBG("-\n");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::stop(MBOOL bForce, MBOOL detachUni)
{
    PIPE_DBG(":E\n");

    if(this->FSM_CHECK(op_stop) == MFALSE)
        return MFALSE;


    //
    //mutex protection is for isp stop & uni_stop which is driven by sendcommand
    this->mRunTimeCfgLock.lock();
    PIPE_DBG("stop pass1 +\n");


    if(this->m_CamPathPass1.stop(&bForce)){
        PIPE_ERR("isp stop fail\n");
        this->mRunTimeCfgLock.unlock();
        return MFALSE;
    }


    if(this->m_bStartUniStreaming){
        //
        PIPE_DBG("stop UniMgr +\n");
        if(this->m_UniMgr.UniMgr_stop(this->m_hwModule)){
            this->mRunTimeCfgLock.unlock();
            return MFALSE;
        }
        PIPE_DBG("stop UniMgr -\n");
        //
        if ((!bForce) && detachUni) {
            if(this->m_UniMgr.UniMgr_detach(this->m_hwModule) != 0){
                this->mRunTimeCfgLock.unlock();
                return MFALSE;
            }
            this->m_bStartUniStreaming = MFALSE;
        }
    }

    PIPE_DBG("stop pass1 -\n");



    this->mRunTimeCfgLock.unlock();


    //
    if(this->m_camPass1Param.bypass_tg == MTRUE){
        if(this->m_UniMgr.UniMgr_stop(this->m_hwModule)){
            PIPE_ERR("rawi stop fail\n");
            return MFALSE;
        }
    }

    //
    if(this->m_pCmdQMgr->CmdQMgr_stop()){
        PIPE_ERR("CQ stop fail\n");
        return MFALSE;
    }
    //stop cmdq11
    if(this->m_pHighSpeedCmdQMgr){
        if(this->m_pHighSpeedCmdQMgr->CmdQMgr_stop()){
            PIPE_ERR("CQ11 stop fail\n");
            return MFALSE;
        }
    }

    if(MTRUE == this->m_TwinMgr->getIsTwin()){
        this->m_TwinMgr->stopTwin();
    }


    this->FSM_UPDATE(op_stop);
    PIPE_DBG("-\n");
    return  MTRUE;
}


MBOOL CamIOPipe::suspend(E_SUSPEND_MODE suspendMode)
{
    MBOOL ret, rst = MFALSE;

    if(this->FSM_CHECK(op_stop) == MFALSE)
        return MFALSE;

    //HW_RST mode, notify STT to update TS_Base "before" suspend() for getting correct fbc_cnt
    if(suspendMode == HW_RST_SUSPEND) {
        rst = (this->m_pCmdQMgr->CmdQMgr_GetCounter(MFALSE) > this->m_pCmdQMgr->CmdQMgr_GetCounter(MTRUE));
        this->m_Stt_rstBufCtrl_CBFP(this->m_Stt_CBCookie,rst);
    }

    ret = this->m_CamPathPass1.suspend( (suspendMode == HW_RST_SUSPEND) ?
        CamPathPass1::CAMPATH_HW_RST_SUSPEND : CamPathPass1::CAMPATH_SW_SUSPEND
    );

    //Standy mode, notify STT to update TS_Base "after" suspend()
    if(suspendMode != HW_RST_SUSPEND && this->m_Stt_rstBufCtrl_CBFP)
        this->m_Stt_rstBufCtrl_CBFP(this->m_Stt_CBCookie,rst);

    return ret;

}
MBOOL CamIOPipe::resume(E_SUSPEND_MODE suspendMode)
{

    if(this->FSM_CHECK(op_stop) == MFALSE)
        return MFALSE;

    return this->m_CamPathPass1.resume( (suspendMode == HW_RST_SUSPEND) ?
        CamPathPass1::CAMPATH_HW_RST_SUSPEND : CamPathPass1::CAMPATH_SW_SUSPEND
    );
}



/*******************************************************************************
*
********************************************************************************/
_isp_dma_enum_ CamIOPipe::PortID_MAP(MUINT32 PortID)
{
    switch(PortID){
        case EPortIndex_IMGO:
            return _imgo_;
            break;
        case EPortIndex_RRZO:
            return _rrzo_;
            break;
        case EPortIndex_UFEO:
            return _ufeo_;
            break;
        default:
            PIPE_DBG("un-supported portID:0x%x\n",PortID);
            break;
    }
    return _cam_max_;
}

MBOOL CamIOPipe::enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MUINT32 dmaChannel = 0;
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    if(this->FSM_CHECK(op_endeq) == MFALSE)
        return MFALSE;

    //
    PIPE_ERR("not supported yet\n");

    this->FSM_UPDATE(op_cmd);
    return  MFALSE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    PIPE_DBG("+ tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    PIPE_ERR("not supported yet\n");
    rQBufInfo;
    if(this->FSM_CHECK(op_endeq) == MFALSE)
        return MFALSE;

    this->FSM_UPDATE(op_cmd);
    return  MFALSE;
}

/*******************************************************************************
* return value:
* 1: isp is already stopped
* 0: sucessed
*-1: fail
********************************************************************************/
MINT32 CamIOPipe::enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MBOOL bImdMode)
{
    MUINT32         dmaChannel = 0;
    ISP_BUF_INFO_L  buf_L;
    stISP_BUF_INFO  bufInfo;
    //warning free
    bImdMode;

    //
    if(this->FSM_CHECK(op_endeq) == MFALSE){
        if(this->m_FSM == op_stop)
            return 1;
        return -1;
    }
    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        //EIS/RSS/LCS
        return this->m_CamIO_stt.enqueOutBuf(portID,rQBufInfo,this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
    }

    if(this->m_pCmdQMgr->CmdQMgr_GetBurstQ()!= rQBufInfo.vBufInfo.size()){
        PIPE_ERR("enque buf number is mismatched with subsample (0x%x_0x%x)\n",rQBufInfo.vBufInfo.size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
        return -1;
    }
    for(MUINT32 i=0;i<rQBufInfo.vBufInfo.size();i++){
        //repalce
        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            bufInfo.Replace.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].replace.u4BufPA;
            bufInfo.Replace.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].replace.u4BufVA;
            bufInfo.Replace.image.mem_info.size      = rQBufInfo.vBufInfo[i].replace.u4BufSize;
            bufInfo.Replace.image.mem_info.memID     = rQBufInfo.vBufInfo[i].replace.memID;
            bufInfo.Replace.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].replace.bufSecu;
            bufInfo.Replace.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].replace.bufCohe;

            //header
            bufInfo.Replace.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA;
            bufInfo.Replace.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA;
            bufInfo.Replace.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize;
            bufInfo.Replace.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID;
            bufInfo.Replace.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu;
            bufInfo.Replace.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe;

            bufInfo.bReplace = MTRUE;
        }
        bufInfo.u_op.enque.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].u4BufPA[0];
        bufInfo.u_op.enque.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].u4BufVA[0];
        bufInfo.u_op.enque.image.mem_info.size      = rQBufInfo.vBufInfo[i].u4BufSize[0];
        bufInfo.u_op.enque.image.mem_info.memID     = rQBufInfo.vBufInfo[i].memID[0];
        bufInfo.u_op.enque.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].bufSecu[0];
        bufInfo.u_op.enque.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].bufCohe[0];

        //header
        bufInfo.u_op.enque.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA;
        bufInfo.u_op.enque.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA;
        bufInfo.u_op.enque.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize;
        bufInfo.u_op.enque.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID;
        bufInfo.u_op.enque.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu;
        bufInfo.u_op.enque.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe;

        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08x),(PA:0x%08x),(FH_VA:0x%08x). exbufInfo,(MEMID:%d),(VA:0x%08x),(PA:0x%08x),(FH_VA:0x%08x)\n",\
            i,\
            portID.index, bufInfo.u_op.enque.image.mem_info.memID,\
            bufInfo.u_op.enque.image.mem_info.va_addr,\
            bufInfo.u_op.enque.image.mem_info.pa_addr,\
            bufInfo.u_op.enque.header.va_addr,\
            bufInfo.Replace.image.mem_info.memID,\
            bufInfo.Replace.image.mem_info.va_addr,\
            bufInfo.Replace.image.mem_info.pa_addr,\
            bufInfo.Replace.header.va_addr);
        }
        else{
            PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08x),(PA:0x%08x),(FH_VA:0x%08x)\n",\
            i,\
            portID.index, bufInfo.u_op.enque.image.mem_info.memID,\
            bufInfo.u_op.enque.image.mem_info.va_addr,\
            bufInfo.u_op.enque.image.mem_info.pa_addr,\
            bufInfo.u_op.enque.header.va_addr);
        }
        //
        buf_L.push_back(bufInfo);
    }

    //
    if ( 0 != this->m_CamPathPass1.enqueueBuf( dmaChannel, buf_L) ) {
        PIPE_ERR("ERROR:DMA[%d] enqueOutBuf fail\n",dmaChannel);
        return -1;
    }

    this->FSM_UPDATE(op_cmd);
    return  0;
}


/*******************************************************************************
* return value:
* 1: isp is already stopped
* 0: sucessed
*-1: fail
********************************************************************************/

MINT32 CamIOPipe::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs, CAM_STATE_NOTIFY *pNotify)
{
    MUINT32 dmaChannel = 0;
    MINT32  ret = 0;
    //warning free
    u4TimeoutMs;
    //
    if(this->FSM_CHECK(op_endeq) == MFALSE) {
        if(this->m_FSM == op_stop)
            return 1;
        return -1;
    }
    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        //EIS/RSS/LCS
        return this->m_CamIO_stt.dequeOutBuf(portID,rQBufInfo,this->m_pCmdQMgr->CmdQMgr_GetBurstQ(), pNotify);
    }

    //clear remained data in container
    rQBufInfo.vBufInfo.clear();

    //
    if ( (ret = this->m_CamPathPass1.dequeueBuf( dmaChannel,rQBufInfo.vBufInfo,u4TimeoutMs, pNotify)) != 0 ) {
        if ( ret > 0 )
            PIPE_DBG("deq dma[%d] FSM=stop(%d)", dmaChannel, this->m_FSM);
        else
            PIPE_ERR("ERROR:dma[%d] dequeOutBuf fail\n", dmaChannel);
        return ret;
    }


    if(rQBufInfo.vBufInfo.size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ())
        PIPE_ERR("dequeout data length is mismatch(%d_%d)\n", rQBufInfo.vBufInfo.size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());

    for ( MUINT32 i = 0; i < rQBufInfo.vBufInfo.size() ; i++) {

        //
        if(this->m_camPass1Param.bypass_tg == MTRUE)
            rQBufInfo.vBufInfo[i].img_fmt = (MUINT32)this->HwValToSrcFmt_DMA(portID.index, rQBufInfo.vBufInfo[i].img_fmt);
        else
            rQBufInfo.vBufInfo[i].img_fmt = (MUINT32)this->HwValToSrcFmt_TG(portID.index, rQBufInfo.vBufInfo[i].img_fmt);

        rQBufInfo.vBufInfo[i].img_pxl_id = (MUINT32)this->HwValToSrcPixID(rQBufInfo.vBufInfo[i].img_pxl_id);


        switch(rQBufInfo.vBufInfo[i].raw_type){
            case CAM_RAW_PIPE::E_FromTG:
                rQBufInfo.vBufInfo[i].raw_type = 1;
                break;
            case CAM_RAW_PIPE::E_AFTERLSC:
                rQBufInfo.vBufInfo[i].raw_type = 0;
                break;
            case CAM_RAW_PIPE::E_BeforLSC:
                rQBufInfo.vBufInfo[i].raw_type = 2;
                break;
            default:
                PIPE_ERR("unsupported raw_type:0x%x\n",rQBufInfo.vBufInfo[i].raw_type);
                break;
        }

        //
        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08x),(PA:0x%08x),(FH_VA:0x%08x)\n",\
            i,\
            portID.index, rQBufInfo.vBufInfo[i].memID[0],\
            rQBufInfo.vBufInfo[i].u4BufVA[0],\
            rQBufInfo.vBufInfo[i].u4BufPA[0],\
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA);
    }

    //
    //PIPE_DBG("X ");
    this->FSM_UPDATE(op_cmd);
    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts,CAMIO_Func func)
{
    MBOOL   bPureRaw = MFALSE;
    MBOOL   bPak = MTRUE;

    MUINT32 imgo_fmt = 0;
    MUINT32 rrzo_fmt = 0;

    MINT32 idx_in_src = -1;
    MINT32 InSrcFmt = -1;

    MINT32 idx_imgo = -1;
    MINT32 idx_rrzo = -1;
    MINT32 idx_ufeo = -1;
    MINT32 pix_id_tg = -1;
    MINT32 pixel_byte_imgo = -1;
    MINT32 pixel_byte_rrzo = -1;
    MINT32 pixel_byte_imgi = -1;

    //ENUM_CAM_CTL_EN
    MUINT32 en_p1 = 0;
    //ENUM_CAM_CTL_RAW_INT_EN
    MUINT32 int_en = (VS_INT_EN_ | \
                    TG_INT1_EN_ | \
                    TG_ERR_EN_ | \
                    TG_GBERR_EN_ | \
                    CQ_CODE_ERR_EN_|\
                    CQ_APB_ERR_EN_ |\
                    CQ_VS_ERR_EN_|\
                    HW_PASS1_DON_EN_|\
                    SOF_INT_EN_|\
                    RRZO_ERR_EN_|\
                    AFO_ERR_EN_|\
                    IMGO_ERR_EN_|\
                    AAO_ERR_EN_|\
                    PSO_ERR_EN_|\
                    LCSO_ERR_EN_|\
                    BPC_ERR_EN_|\
                    LSC_ERR_EN_|\
                    DMA_ERR_EN_|\
                    SW_PASS1_DON_EN_);

    //ENUM_CAM_CTL_RAW_INT2_EN
    MUINT32 dma_int_en = (IMGO_DONE_EN_  | \
                        UFEO_DONE_EN_ | \
                        RRZO_DONE_EN_ | \
                        EISO_DONE_EN_ | \
                        FLKO_DONE_EN_ | \
                        AFO_DONE_EN_  | \
                        LCSO_DONE_EN_ | \
                        AAO_DONE_EN_  | \
                        PDO_DONE_EN_  | \
                        PSO_DONE_EN_);

    // twin_mgr parameters: to control frontal binning, mux pixel mode, and twin enable/disable
    TWIN_MGR_PATH_CFG_IN_PARAM cfgInParam;
    TWIN_MGR_PATH_CFG_OUT_PARAM cfgOutParam;

    //
    if(this->FSM_CHECK(op_cfg) == MFALSE)
        return MFALSE;

    if ( (0 == vOutPorts.size()) || (vInPorts.size() != 1)) {
        PIPE_ERR("inport:0x%x || outport:x%x size err\n", vInPorts.size(), vOutPorts.size());
        return MFALSE;
    }


    //input source
    //inport support only 1 at current driver design
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) {
            PIPE_INF("dummy input vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("P1 vInPorts_%d:fmt(0x%x),PM(%d),w/h(%d_%d),crop(%d_%d_%d_%d),tg_idx(%d),dir(%d),fps(%d),timeclk(0x%x),subsample(%d),bin_off(%d),dat_pat(%d)\n",\
                i, \
                vInPorts[i]->eImgFmt, \
                vInPorts[i]->ePxlMode,  \
                vInPorts[i]->u4ImgWidth, vInPorts[i]->u4ImgHeight,\
                vInPorts[i]->crop1.x,\
                vInPorts[i]->crop1.y,\
                vInPorts[i]->crop1.w,\
                vInPorts[i]->crop1.h,\
                vInPorts[i]->index, \
                vInPorts[i]->inout,\
                vInPorts[i]->tgFps,\
                vInPorts[i]->tTimeClk,\
                func.Bits.SUBSAMPLE,\
                func.Bits.bin_off,\
                func.Bits.DATA_PATTERN);

        //
        switch ( vInPorts[i]->index ) {
            case EPortIndex_TG1I:
                if(this->m_hwModule != CAM_A){
                    PIPE_ERR("TG1 must be with CAM_A\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG1 input fmt error\n");
                    return MFALSE;
                }
                else{
                    switch(InSrcFmt){
                        case TG_FMT_YUV422:
                            this->m_camPass1Param.bypass_ispRawPipe = MTRUE;
                            this->m_camPass1Param.bypass_ispYuvPipe = MFALSE;
                            break;
                        default:
                            this->m_camPass1Param.bypass_ispRawPipe = MFALSE;
                            this->m_camPass1Param.bypass_ispYuvPipe = MTRUE;
                            break;
                    }
                }

                en_p1 |= TG_EN_;
                break;
            case EPortIndex_TG2I:
                if(this->m_hwModule != CAM_B){
                    PIPE_ERR("TG2 must be with CAM_B\n");
                    return MFALSE;
                }

                if((InSrcFmt = this->SrcFmtToHwVal_TG(vInPorts[i]->eImgFmt)) == -1){
                    PIPE_ERR("TG2 input fmt error\n");
                    return MFALSE;
                }
                else{
                    switch(InSrcFmt){
                        case TG_FMT_YUV422:
                            this->m_camPass1Param.bypass_ispRawPipe = MTRUE;
                            this->m_camPass1Param.bypass_ispYuvPipe = MFALSE;
                            break;
                        default:
                            this->m_camPass1Param.bypass_ispRawPipe = MFALSE;
                            this->m_camPass1Param.bypass_ispYuvPipe = MTRUE;
                            break;
                    }
                }

                //
                if(vInPorts[i]->bOver4lane == MTRUE){
                    PIPE_ERR("can't support over 4 lane sensor by CAM_B master\n");
                    return MFALSE;
                }
                en_p1 |= TG_EN_;
                break;
            case EPortIndex_RAWI:

                if((InSrcFmt = this->SrcFmtToHwVal_DMA(vInPorts[i]->eImgFmt,(MUINT32*)&pixel_byte_imgi)) == -1){
                    PIPE_ERR("DMAI input fmt error\n");
                    return MFALSE;
                }
                else{
                    switch(InSrcFmt){
                        case RAWI_FMT_RAW8:
                        case RAWI_FMT_RAW10:
                        case RAWI_FMT_RAW12:
                        case RAWI_FMT_RAW14:
                        case RAWI_FMT_RAW8_2B:
                        case RAWI_FMT_RAW10_2B:
                        case RAWI_FMT_RAW12_2B:
                        case RAWI_FMT_RAW14_2B:
                            this->m_camPass1Param.bypass_ispRawPipe = MFALSE;
                            this->m_camPass1Param.bypass_ispYuvPipe = MTRUE;
                            break;
                        default:
                            PIPE_ERR("DMAI input fmt error\n");
                            return MFALSE;
                            break;
                    }


                }
                //
                if(vInPorts[i]->bOver4lane == MTRUE){
                    PIPE_ERR("RAWI & over4Lane setting is conflicted\n");
                    return MFALSE;
                }

                this->m_camPass1Param.bypass_tg = MTRUE;
                break;
            default:
                PIPE_ERR("Not supported input source:0x%x !!!!!!!!!!!!!!!!!!!!!!",vInPorts[i]->index);
                return MFALSE;
                break;
        }

        idx_in_src = i;

        if((pix_id_tg = this->SrcPixIDToHwVal(vInPorts[i]->eRawPxlID)) == -1){
            PIPE_ERR("unsupported pix id\n");
            return MFALSE;
        }
#if 0
        if (0!=this->PxlMode(vInPorts[i]->ePxlMode, vInPorts[i]->eImgFmt, (MINT32*) &pass1_twin, (MINT32*)&two_pxl, (MINT32*)&en_p1, (MINT32*)&two_pxl_dmx )) {
            PIPE_ERR("ERROR:in param");
            return MFALSE;
        }
#endif
    }

    //output port
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {
        if ( 0 == vOutPorts[i] ) {
            PIPE_INF("dummy output vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("P1 vOutPorts:[%d]:fmt(0x%x),pure(%d),w(%d),h(%d),stirde(%d),dmao(%d),dir(%d)\n", \
                 i, vOutPorts[i]->eImgFmt, vOutPorts[i]->u4PureRaw, \
                 vOutPorts[i]->u4ImgWidth, vOutPorts[i]->u4ImgHeight,   \
                 vOutPorts[i]->u4Stride[ESTRIDE_1ST_PLANE], \
                 vOutPorts[i]->index, vOutPorts[i]->inout);

        //
        switch(vOutPorts[i]->index){
            case EPortIndex_IMGO:
                idx_imgo = i;

                this->getOutPxlByteNFmt(0, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo, (MINT32*)&imgo_fmt );

                if(imgo_fmt == IMGO_FMT_YUV422_1P){//YUV format
                    //ONLY YUV sensor support always fix in pureraw path under 1/2/4 pix mode
                    this->m_DynamicRawType = MFALSE;
                    bPureRaw = MTRUE;
                }
                else{   //bayer format
                    en_p1     |= PAK_EN_;
                    if(1 == vOutPorts[i]->u4PureRaw){
                        //can't output pure raw with data_pattern = dual_pix,  tile-raw have no DBN
                        if(func.Bits.DATA_PATTERN == ECamio_Dual_pix){
                            PIPE_ERR("support no IMGO_PURE_RAW + DUAL_PIX data pattern\n");
                            return MFALSE;
                        }

                        bPureRaw = MTRUE;
                        if(1 == vOutPorts[i]->u4PureRawPak) {
                            bPak = MTRUE;
                        }else{
                            bPak = MFALSE;
                            //disable pak
                            en_p1 &= (~PAK_EN_);
                            if( (1 << CAM_ISP_PIXEL_BYTE_FP) !=  pixel_byte_imgo) {
                                pixel_byte_imgo= (2 << CAM_ISP_PIXEL_BYTE_FP);
                            }
                        }
                    }
                    this->m_DynamicRawType = MTRUE;
                }

                this->m_camPass1Param.bypass_imgo = MFALSE;
                break;
            case EPortIndex_RRZO:
                idx_rrzo = i;

                if((vOutPorts[i]->eImgFmt == eImgFmt_UFEO_BAYER10)||(vOutPorts[i]->eImgFmt == eImgFmt_UFEO_BAYER12)){
                    en_p1     |= RRZ_EN_;
                }
                else{
                en_p1     |= (RRZ_EN_|PAKG_EN_);
                }
                this->getOutPxlByteNFmt(1, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_rrzo, (MINT32*)&rrzo_fmt);
                if(1 == vOutPorts[i]->u4PureRaw){
                    PIPE_ERR("this dmao support no pure output,always processed raw\n");
                }

                this->m_camPass1Param.bypass_rrzo = MFALSE;
                break;
            case EPortIndex_UFEO:
                idx_ufeo = i;
                en_p1 |= UFEG_EN_;//UFO
                this->getOutPxlByteNFmt(1, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_rrzo, (MINT32*)&rrzo_fmt);
                this->m_camPass1Param.bypass_ufeo = MFALSE;
				break;
        }
    }

    //cfg cmdQ
    if((this->m_pCmdQMgr = DupCmdQMgr::CmdQMgr_attach(vInPorts[idx_in_src]->tgFps,func.Bits.SUBSAMPLE ,this->m_hwModule,ISP_DRV_CQ_THRE0)) == NULL){
        PIPE_ERR("CQ init fail\n");
        this->m_CamPathPass1.m_pCmdQdrv = NULL;
        return MFALSE;
    }
    else
        this->m_CamPathPass1.m_pCmdQdrv = this->m_pCmdQMgr;

    //cfg cmdq11
    if(func.Bits.SUBSAMPLE){
        if( (this->m_pHighSpeedCmdQMgr = DupCmdQMgr::CmdQMgr_attach(vInPorts[idx_in_src]->tgFps,\
            func.Bits.SUBSAMPLE,this->m_hwModule,ISP_DRV_CQ_THRE11)) == NULL){
            PIPE_ERR("CQ 11 init fail\n");
            this->m_pHighSpeedCmdQMgr = NULL;
        }
    }
    //
    /*-----------------------------------------------------------------------------
      m_camPass1Param
      -----------------------------------------------------------------------------*/
    //setting module
    this->m_CamPathPass1.m_hwModule = this->m_hwModule;

    //path
    if(this->m_camPass1Param.bypass_tg == MTRUE)
        this->m_camPass1Param.m_Path = CamPathPass1Parameter::_DRAM_IN;
    else
        this->m_camPass1Param.m_Path = CamPathPass1Parameter::_TG_IN;

    //subsample
    this->m_camPass1Param.m_subSample = func.Bits.SUBSAMPLE;

    //fps
    this->m_camPass1Param.m_fps = vInPorts[idx_in_src]->tgFps;
    if(this->m_camPass1Param.m_fps ==0){
        this->m_camPass1Param.m_fps +=1;
        PIPE_ERR("fps can't be 0\n");
    }
    //raw type
    if(bPureRaw)
        this->m_camPass1Param.m_NonScale_RawType = CamPathPass1Parameter::_PURE_RAW;
    else
        this->m_camPass1Param.m_NonScale_RawType = CamPathPass1Parameter::_PROC_RAW;
    ////////////////////
    //TG
    /////////////////////
    if(this->m_camPass1Param.bypass_tg == MFALSE){

        if((vInPorts[idx_in_src]->crop1.w==0) || (vInPorts[idx_in_src]->crop1.h == 0)){
            PIPE_ERR("TG[%d] crop size can't be 0\n",vInPorts[idx_in_src]->index);
            return MFALSE;
        }

        this->m_camPass1Param.m_tTimeClk = vInPorts[idx_in_src]->tTimeClk;
        if(this->m_camPass1Param.m_tTimeClk == 0){
            this->m_camPass1Param.m_tTimeClk += 1;
            PIPE_ERR("timestamp clk source can't be 0, change to 1mhz to avoid NE\n");
            PIPE_ERR("following timestamp info will be all error\n");
        }
        this->m_camPass1Param.m_src_size.x = vInPorts[idx_in_src]->crop1.x;
        this->m_camPass1Param.m_src_size.y = vInPorts[idx_in_src]->crop1.y;
        this->m_camPass1Param.m_src_size.w = vInPorts[idx_in_src]->crop1.w;
        this->m_camPass1Param.m_src_size.h = vInPorts[idx_in_src]->crop1.h;

        switch(vInPorts[idx_in_src]->ePxlMode){
            case ePxlMode_One_:
                this->m_camPass1Param.m_src_pixmode = CAM_TG_CTRL::_1_pix_;
                break;
            case ePxlMode_Two_:
                this->m_camPass1Param.m_src_pixmode = CAM_TG_CTRL::_2_pix_;
                break;
            case ePxlMode_Four_:
                this->m_camPass1Param.m_src_pixmode = CAM_TG_CTRL::_4_pix_;
                break;
            default:
                PIPE_ERR("TG[%d] unknown pix mode:0x%x\n",vInPorts[idx_in_src]->index,vInPorts[idx_in_src]->ePxlMode);
                return MFALSE;
                break;
        }

    }


    //fmt sel
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Raw             = 0x00;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_ID     = pix_id_tg;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.TG_FMT     = InSrcFmt;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.RRZO_FMT   = rrzo_fmt;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.IMGO_FMT   = imgo_fmt;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.RRZO_FG_MODE   = 0x1;  //always FG format

    /// Notice!!! Must call TwinMgr before this->pathSelCtrl(bPureRaw,bPak,this->m_camPass1Param.bypass_tg);
    ///           Cause when frontal binning is enable. IMGO will be set to PURE RAW
    ///
    /// Get path config info from twin_mgr
    ///
    this->m_TwinMgr = TwinMgr::createInstance(this->m_hwModule);
    if(this->m_camPass1Param.bypass_tg == MFALSE)
    {
        // Prepare input param to TwinMgr
        cfgInParam.bypass_tg = this->m_camPass1Param.bypass_tg;
        cfgInParam.pix_mode_tg = this->m_camPass1Param.m_src_pixmode;
        cfgInParam.tg_crop_w = this->m_camPass1Param.m_src_size.w;
        cfgInParam.bypass_rrzo = this->m_camPass1Param.bypass_rrzo;
        cfgInParam.max_rrz_out_w = (this->m_camPass1Param.bypass_rrzo == MFALSE) ? \
                                    vOutPorts[idx_rrzo]->u4ImgWidth : 0;
        //
        cfgInParam.offBin = func.Bits.bin_off;
        switch(func.Bits.DATA_PATTERN){
            case ECamio_Dual_pix:
                cfgInParam.bDbn = MTRUE;
                break;
            case ECamio_Normal:
            default:
                cfgInParam.bDbn = MFALSE;
                break;
        }
        if(func.Bits.bin_off == MTRUE)
            PIPE_INF("Frontal binning is forced to disable\n");

        // Get configPath info via TwinMgr
        if(MFALSE == this->m_TwinMgr->configPath(cfgInParam, cfgOutParam)) {
            PIPE_ERR("TwinMgt configPath() error!!!!!!\n");
            return MFALSE;
        }


        // Store cfgOutParam to CamIOPipe
        // 1. pixel mode of mux
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXI = cfgOutParam.pix_mode_dmxi;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXO = cfgOutParam.pix_mode_dmxo;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_BMXO = cfgOutParam.pix_mode_bmxo;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_RMXO = cfgOutParam.pix_mode_rmxo;

        // 2. frontal binning
        if(cfgOutParam.dbn_en == MTRUE)
            en_p1 |= (DBN_EN_|PBN_EN_);

        if(cfgOutParam.bin_en== MTRUE)
            en_p1 |= BIN_EN_;

        // When frontal binning is enable, set RAW type to be "PureRaw"!!!
        if(cfgOutParam.bin_en== MTRUE) {
            this->m_camPass1Param.m_NonScale_RawType = CamPathPass1Parameter::_PURE_RAW;
            bPureRaw = MTRUE;
            this->m_DynamicRawType = MFALSE; // Cannot change raw type in dynamic
            PIPE_DBG("Frontal binning is enable!! bin(%d), Set Raw type to PureRaw!!!\n",  cfgOutParam.bin_en);
        }

    }
    /* TODO: need to check RAWI case */
    else
    {
        PIPE_ERR("##################################\n");
        PIPE_ERR("##################################\n");
        PIPE_ERR("TwinMgr configPath() not support RAWI path yet.\n");
        PIPE_ERR("##################################\n");
        PIPE_ERR("##################################\n");
    }

    //////////////////////
    //top
    //////////////////////
    // 4 lane ctrl
    if(vInPorts[idx_in_src]->bOver4lane){
        this->m_camPass1Param.m_bOver4lane = MTRUE;
        PIPE_INF("over 4 lane input\n");

        PIPE_ERR("###################\n");
        PIPE_ERR("###################\n");
        PIPE_ERR("driver not support yet\n");
        PIPE_ERR("###################\n");
        PIPE_ERR("###################\n");
        //need occupied cam_a & cam_b both .
        return MFALSE;
    }
    else
        this->m_camPass1Param.m_bOver4lane = MFALSE;

    //ctrl sel
    this->pathSelCtrl(bPureRaw,bPak,this->m_camPass1Param.bypass_tg,func.Bits.DATA_PATTERN);


    // If twin is enable:
    // 1. Occupied another CamIOPipe hwModule for TwinMgr
    // 2. Cfg cmdQ for another CamIOPipe hwModule
    if(this->m_TwinMgr->getIsTwin() == MTRUE)
    {
        list<ISP_HW_MODULE>::iterator it;
        char str[32];
        str[0] = '\0';

        //
        if(this->m_TwinMgr->initTwin(this->m_pCmdQMgr, vInPorts[idx_in_src]->tgFps,func.Bits.SUBSAMPLE) == MFALSE){
            PIPE_ERR("twin init fail\n");
            return MFALSE;
        }

        if(this->m_TwinMgr->configTwin() == MFALSE){
            PIPE_ERR("twin config fail\n");
            return MFALSE;
        }

        //
        for(it = this->m_TwinMgr->getCurModule()->begin();it!=this->m_TwinMgr->getCurModule()->end();it++){
            if(gCamioPipe[*it].occupied != MTRUE)
            {
                gCamioPipe[*it].m_hwModule = *it;
                gCamioPipe[*it].occupied = MTRUE;
                sprintf((char*)gCamioPipe[*it].m_szUsrName,"TwinMgr:%d",*it);

                PIPE_INF("Twin enable!! Occupy CamIoPipe: module(%d) by %s.", *it,gCamioPipe[*it].m_szUsrName);
            }
            else{
                PIPE_ERR("twin init fail,module(%d) is occupied by user:%s\n", *it, \
                    gCamioPipe[*it].m_szUsrName);
                return MFALSE;
            }
        }
    }


    //func_en , default function
    this->m_camPass1Param.m_top_ctl.FUNC_EN.Raw     = en_p1;

    //inte en
    this->m_camPass1Param.m_top_ctl.INTE_EN.Raw     = int_en;
    this->m_camPass1Param.m_top_ctl.DMA_INTE_EN.Raw = dma_int_en;


    //uni_rawi
    if(this->m_camPass1Param.bypass_tg == MTRUE){
        IspDMACfg RawI;
        // use input dma crop
        this->configDmaPort(vInPorts[idx_in_src],RawI,(MUINT32)pixel_byte_imgi);
        RawI.format = InSrcFmt;//borrow this para to set format in uni_mgr
        //
        if(this->m_UniMgr.UniMgr_attach(UNI_A,this->m_pCmdQMgr,&RawI) != 0){
            PIPE_ERR("rawi cfg fail\n");
            return MFALSE;
        }
        //
        this->m_camPass1Param.m_RawIFmt = (E_UNI_TOP_FMT)InSrcFmt;
        this->m_camPass1Param.m_pUniMgr = &this->m_UniMgr;

        //keep this record for debug or sendcommand determine size constraint.
        this->m_camPass1Param.m_src_size.x = 0;
        this->m_camPass1Param.m_src_size.y = 0;
        this->m_camPass1Param.m_src_size.w = vInPorts[idx_in_src]->u4ImgWidth;
        this->m_camPass1Param.m_src_size.h = vInPorts[idx_in_src]->u4ImgHeight;
    }


    //imgo
    if(this->m_camPass1Param.bypass_imgo == MFALSE){
        if(vOutPorts[idx_imgo]->crop1.floatX || vOutPorts[idx_imgo]->crop1.floatY){
            PIPE_ERR("imgo support no floating-crop_start , replaced by 0\n");
        }

        if(vOutPorts[idx_imgo]->crop1.w != vOutPorts[idx_imgo]->u4ImgWidth){
            PIPE_ERR("imgo: crop_w != dstsize_w(0x%x,0x%x), this port have no build-in resizer\n",\
                vOutPorts[idx_imgo]->crop1.w,vOutPorts[idx_imgo]->u4ImgWidth);
            return MFALSE;
        }
        if(vOutPorts[idx_imgo]->crop1.h != vOutPorts[idx_imgo]->u4ImgHeight){
            PIPE_ERR("imgo: crop_h != dstsize_h(0x%x,0x%x), this port have no build-in resizer\n",\
                vOutPorts[idx_imgo]->crop1.h,vOutPorts[idx_imgo]->u4ImgHeight);
            return MFALSE;
        }

        if((vOutPorts[idx_imgo]->crop1.w + vOutPorts[idx_imgo]->crop1.x) > vInPorts[idx_in_src]->u4ImgWidth){
            PIPE_ERR("crop_w+crop_x > input_w(0x%x,0x%x)\n",\
                (vOutPorts[idx_imgo]->crop1.w + vOutPorts[idx_imgo]->crop1.x),\
                vInPorts[idx_in_src]->u4ImgWidth);
            return MFALSE;
        }
        if((vOutPorts[idx_imgo]->crop1.h + vOutPorts[idx_imgo]->crop1.y) > vInPorts[idx_in_src]->u4ImgHeight){
            PIPE_ERR("crop_h+crop_y > input_h(0x%x,0x%x)\n",\
                (vOutPorts[idx_imgo]->crop1.h + vOutPorts[idx_imgo]->crop1.y),\
                vInPorts[idx_in_src]->u4ImgHeight);
            return MFALSE;
        }


        // use output dma crop
        this->configDmaPort(vOutPorts[idx_imgo],this->m_camPass1Param.m_imgo.dmao,(MUINT32)pixel_byte_imgo);
    }

    //rrzo
    if(this->m_camPass1Param.bypass_rrzo == MFALSE){
        if(vOutPorts[idx_rrzo]->crop1.floatX || vOutPorts[idx_rrzo]->crop1.floatY){
            PIPE_ERR("rrzo support no floating-crop_start , replaced by 0\n");
        }

        if(vOutPorts[idx_rrzo]->crop1.w < vOutPorts[idx_rrzo]->u4ImgWidth){
            PIPE_ERR("rrzo: crop_w < dstsize_w(0x%x,0x%x), resizer support only scale down\n",\
                vOutPorts[idx_rrzo]->crop1.w,vOutPorts[idx_rrzo]->u4ImgWidth);
            return MFALSE;
        }
        if(vOutPorts[idx_rrzo]->crop1.h < vOutPorts[idx_rrzo]->u4ImgHeight){
            PIPE_ERR("rrzo: crop_h < dstsize_h(0x%x,0x%x), resizer support only scale down\n",\
                vOutPorts[idx_rrzo]->crop1.h,vOutPorts[idx_rrzo]->u4ImgHeight);
            return MFALSE;
        }

        if((vOutPorts[idx_rrzo]->crop1.w + vOutPorts[idx_rrzo]->crop1.x) > vInPorts[idx_in_src]->u4ImgWidth){
            PIPE_ERR("crop_w+crop_x > input_w(0x%x,0x%x)\n",\
                (vOutPorts[idx_rrzo]->crop1.w + vOutPorts[idx_rrzo]->crop1.x),\
                vInPorts[idx_in_src]->u4ImgWidth);
            return MFALSE;
        }
        if((vOutPorts[idx_rrzo]->crop1.h + vOutPorts[idx_rrzo]->crop1.y) > vInPorts[idx_in_src]->u4ImgHeight){
            PIPE_ERR("crop_h+crop_y > input_h(0x%x,0x%x)\n",\
                (vOutPorts[idx_rrzo]->crop1.h + vOutPorts[idx_rrzo]->crop1.y),\
                vInPorts[idx_in_src]->u4ImgHeight);
            return MFALSE;
        }

        // If DBN=1, rrz setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
        // This is performed in CAM_RRZ_CTRL::_config(void) of isp_function_cam cause rrz_in size need to be processed in it

        //
        this->configDmaPort(vOutPorts[idx_rrzo],this->m_camPass1Param.m_rrzo.dmao,(MUINT32)pixel_byte_rrzo);
        //rrzo support no dmao cropping , use rrz input crop to replace dmao cropping
        this->m_camPass1Param.rrz_in_roi = this->m_camPass1Param.m_rrzo.dmao.crop;

        this->m_camPass1Param.m_rrzo.dmao.crop.x =0;
        this->m_camPass1Param.m_rrzo.dmao.crop.y =0;
        this->m_camPass1Param.m_rrzo.dmao.crop.w = vOutPorts[idx_rrzo]->u4ImgWidth;
        this->m_camPass1Param.m_rrzo.dmao.crop.h = vOutPorts[idx_rrzo]->u4ImgHeight;

        this->m_camPass1Param.m_Scaler.w = vOutPorts[idx_rrzo]->u4ImgWidth;
        this->m_camPass1Param.m_Scaler.h = vOutPorts[idx_rrzo]->u4ImgHeight;
    }

    //ufeo
    if(this->m_camPass1Param.bypass_ufeo == MFALSE){
        if(vOutPorts[idx_ufeo]->crop1.floatX || vOutPorts[idx_ufeo]->crop1.floatY){
            PIPE_ERR("ufeo support no floating-crop_start , replaced by 0\n");
        }

        if(vOutPorts[idx_ufeo]->crop1.w < vOutPorts[idx_ufeo]->u4ImgWidth){
            PIPE_ERR("ufeo: crop_w < dstsize_w(0x%x,0x%x), rrzo resizer support only scale down\n",\
            vOutPorts[idx_ufeo]->crop1.w,vOutPorts[idx_ufeo]->u4ImgWidth);
            return MFALSE;
        }
         if(vOutPorts[idx_ufeo]->crop1.h < vOutPorts[idx_ufeo]->u4ImgHeight){
             PIPE_ERR("ufeo: crop_h < dstsize_h(0x%x,0x%x), rrzo resizer support only scale down\n",\
                 vOutPorts[idx_ufeo]->crop1.h,vOutPorts[idx_ufeo]->u4ImgHeight);
             return MFALSE;
         }

         if((vOutPorts[idx_ufeo]->crop1.w + vOutPorts[idx_ufeo]->crop1.x) > vInPorts[idx_in_src]->u4ImgWidth){
             PIPE_ERR("crop_w+crop_x > input_w(0x%x,0x%x)\n",\
                 (vOutPorts[idx_ufeo]->crop1.w + vOutPorts[idx_ufeo]->crop1.x),\
                 vInPorts[idx_in_src]->u4ImgWidth);
             return MFALSE;
         }
         if((vOutPorts[idx_ufeo]->crop1.h + vOutPorts[idx_ufeo]->crop1.y) > vInPorts[idx_in_src]->u4ImgHeight){
             PIPE_ERR("crop_h+crop_y > input_h(0x%x,0x%x)\n",\
                 (vOutPorts[idx_ufeo]->crop1.h + vOutPorts[idx_ufeo]->crop1.y),\
                 vInPorts[idx_in_src]->u4ImgHeight);
             return MFALSE;
         }

         // If DBN=1, rrz setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
         // This is performed in CAM_RRZ_CTRL::_config(void) of isp_function_cam cause rrz_in size need to be processed in it

         //UFEO use RRZO pixel byte
         this->configDmaPort(vOutPorts[idx_ufeo],this->m_camPass1Param.m_ufeo.dmao,(MUINT32)pixel_byte_rrzo);

         //ufeo support no dmao cropping
         this->m_camPass1Param.m_ufeo.dmao.crop.x =0;
         this->m_camPass1Param.m_ufeo.dmao.crop.y =0;
         this->m_camPass1Param.m_ufeo.dmao.crop.w = vOutPorts[idx_ufeo]->u4ImgWidth;
         this->m_camPass1Param.m_ufeo.dmao.crop.h = vOutPorts[idx_ufeo]->u4ImgHeight;
     }

    //
    if( 0 !=  this->m_CamPathPass1.config( &this->m_camPass1Param ) )
    {
        PIPE_ERR("config error!\n");
        return MFALSE;
    }

    PIPE_DBG("-\n");
    this->FSM_UPDATE(op_cfg);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 CamIOPipe::registerIrq(MINT8 const szUsrName[32])
{
    ISP_REGISTER_USERKEY_STRUCT _irq;
    //
    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    memcpy((void*)_irq.userName,(void*)szUsrName,sizeof(_irq.userName));

    if( this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->registerIrq(&_irq) == MFALSE )
    {
        PIPE_ERR("registerIrq error!");
        return  -1;
    }

    this->FSM_UPDATE(op_cmd);
    return _irq.userKey;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::irq(Irq_t* pIrq)
{
    ISP_WAIT_IRQ_ST _irq;

    //
    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    switch(pIrq->Type){
        case Irq_t::_CLEAR_NONE:
            _irq.Clear = ISP_IRQ_CLEAR_NONE;
            break;
        case Irq_t::_CLEAR_WAIT:
            _irq.Clear = ISP_IRQ_CLEAR_WAIT;
            break;
        default:
            PIPE_ERR("unsupported type:0x%x\n",pIrq->Type);
            return MFALSE;
            break;
    }

    switch(pIrq->StatusType){
        case Irq_t::_SIGNAL_INT_:
            switch(pIrq->Status){
                case Irq_t::_VSYNC_:
                    _irq.Status = (unsigned int)VS_INT_ST;
                    break;
                case Irq_t::_SOF_:
                    _irq.Status = (unsigned int)SOF_INT_ST;
                    break;
                case Irq_t::_EOF_:
                    _irq.Status = (unsigned int)SW_PASS1_DON_ST;
                    break;
                case Irq_t::_TG_INT_:
                    if(0==CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_INT1,TG_INT1_LINENO)){
                        PIPE_ERR("ERROR: Plz SET_TG_INT first!\n");
                        return MFALSE;
                    }
                    _irq.Status = (unsigned int)TG_INT1_ST;
                    break;
                default:
                    PIPE_ERR("unsupported signal:0x%x\n",pIrq->Status);
                    return MFALSE;
                    break;
            }
            _irq.St_type = SIGNAL_INT;
            break;
        case Irq_t::_DMA_INT_:
            switch(pIrq->Status){
                case Irq_t::_AF_DONE_:
                    _irq.Status = (unsigned int)AFO_DONE_ST;
                    break;

                default:
                    PIPE_ERR("unsupported signal:0x%x\n",pIrq->Status);
                    return MFALSE;
                    break;
            }
            _irq.St_type = DMA_INT;
            break;
        default:
            PIPE_ERR("unsupported status type:0x%x\n",pIrq->StatusType);
            return MFALSE;
            break;
    }
    _irq.UserKey = pIrq->UserKey;
    _irq.Timeout = pIrq->Timeout;


    if( this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->waitIrq(&_irq) == MFALSE )
    {
        PIPE_ERR("waitirq error!\n");
        return  MFALSE;
    }

    pIrq->TimeInfo.tLastSig_sec = (MUINT32)_irq.TimeInfo.tLastSig_sec;
    pIrq->TimeInfo.tLastSig_usec = (MUINT32)_irq.TimeInfo.tLastSig_usec;
    pIrq->TimeInfo.tMark2WaitSig_sec = (MUINT32)_irq.TimeInfo.tMark2WaitSig_sec;
    pIrq->TimeInfo.tMark2WaitSig_usec = (MUINT32)_irq.TimeInfo.tMark2WaitSig_usec;
    pIrq->TimeInfo.tLastSig2GetSig_sec = (MUINT32)_irq.TimeInfo.tLastSig2GetSig_sec;
    pIrq->TimeInfo.tLastSig2GetSig_usec = (MUINT32)_irq.TimeInfo.tLastSig2GetSig_usec;
    pIrq->TimeInfo.passedbySigcnt = (MUINT32)_irq.TimeInfo.passedbySigcnt;


    this->FSM_UPDATE(op_cmd);
    return MTRUE;
}

MBOOL CamIOPipe::signalIrq(Irq_t irq)
{
    ISP_WAIT_IRQ_ST _irq;

    //
    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    switch(irq.StatusType){
        case Irq_t::_SIGNAL_INT_:
            switch(irq.Status){
                case Irq_t::_VSYNC_:
                    _irq.Status = (unsigned int)VS_INT_ST;
                    break;
                case Irq_t::_SOF_:
                    _irq.Status = (unsigned int)SOF_INT_ST;
                    break;
                case Irq_t::_EOF_:
                    _irq.Status = (unsigned int)SW_PASS1_DON_ST;
                    break;
                case Irq_t::_TG_INT_:
                    _irq.Status = (unsigned int)TG_INT1_ST;
                    break;
                default:
                    PIPE_ERR("unsupported signal:0x%x\n",irq.Status);
                    return MFALSE;
                    break;
            }
            _irq.St_type = SIGNAL_INT;
            break;
        case Irq_t::_DMA_INT_:
            switch(irq.Status){
                case Irq_t::_AF_DONE_:
                    _irq.Status = (unsigned int)AFO_DONE_ST;
                    break;
                default:
                    PIPE_ERR("unsupported signal:0x%x\n",irq.Status);
                    return MFALSE;
                    break;
            }
            _irq.St_type = DMA_INT;
            break;
        default:
            PIPE_ERR("unsupported status type:0x%x\n",irq.StatusType);
            return MFALSE;
            break;
    }

    _irq.UserKey = irq.UserKey;


    if( this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->signalIrq(&_irq) == MFALSE )
    {
        PIPE_ERR("signalIrq error!");
        return  MFALSE;
    }

    this->FSM_UPDATE(op_cmd);
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::abortDma(PortID const port)
{
    MUINT32         dmaChannel = 0;
    ISP_WAIT_IRQ_ST irq;

    //
    if(this->m_FSM != op_stop){
        PIPE_ERR("CamIOPipe FSM(%d)", this->m_FSM);
        return MFALSE;
    }

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    irq.Timeout = 1000;

    if((dmaChannel = (MUINT32)this->PortID_MAP(port.index)) == _cam_max_){
        //EIS/RSS/LCS
        dmaChannel = this->m_CamIO_stt.QueryPort(port);
    }

    switch(dmaChannel){
        case _imgo_:
            irq.St_type = DMA_INT;
            irq.Status = IMGO_DONE_ST;
            break;
        case _rrzo_:
            irq.St_type = DMA_INT;
            irq.Status = RRZO_DONE_ST;
            break;
        case _ufeo_:
            irq.St_type = DMA_INT;
            irq.Status = UFEO_DONE_ST;
            break;
        case _lcso_:
            irq.St_type = DMA_INT;
            irq.Status = LCSO_DONE_ST;
            break;
        case _eiso_:
            irq.St_type = DMA_INT;
            irq.Status = EISO_DONE_ST;
            break;
        case _rsso_:
            irq.St_type = SIGNAL_INT;
            irq.Status = SW_PASS1_DON_ST;
            break;
        default:
            PIPE_ERR("Not support dma(0x%x)\n", port.index);
            return MFALSE;
    }

    if( this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->signalIrq(&irq) == MFALSE )
    {
        PIPE_ERR("abort dma error(0x%x)!", port.index);
        return  MFALSE;
    }

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 CamIOPipe::ReadReg(MUINT32 addr,MBOOL bCurPhy)
{
    if(bCurPhy)
        return this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj()->readReg(addr);
    else
        return this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(this->m_pCmdQMgr->CmdQMgr_GetDuqQIdx())[this->m_pCmdQMgr->CmdQMgr_GetBurstQIdx()]->readReg(addr);
}

MUINT32 CamIOPipe::UNI_ReadReg(MUINT32 addr,MBOOL bCurPhy)
{
    if(this->m_UniMgr.UniMgr_GetCurLink() !=  this->m_hwModule){
        PIPE_ERR("uni is not connected with cur module, [addr:0x%x]\n",addr);
        return 0;
    }

    if(bCurPhy)
        return this->m_UniMgr.UniMgr_GetCurObj()->readReg(addr);
    else
        return this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(this->m_pCmdQMgr->CmdQMgr_GetDuqQIdx())[this->m_pCmdQMgr->CmdQMgr_GetBurstQIdx()]->readReg(addr,UNI_A);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int    ret = 0; // 0 for ok , -1 for fail

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    switch ( cmd ) {
        case EPIPECmd_DEALLOC_UNI:
            //if cam is under running
            //mutex protection is for isp stop & uni stop
            this->mRunTimeCfgLock.lock();
            if(this->m_bStartUniStreaming){
                PIPE_INF("stop uni streaming +\n");
                //
                if(this->m_UniMgr.UniMgr_stop(this->m_hwModule)){
                    ret = 1;
                }

                //
                if(this->m_UniMgr.UniMgr_detach(this->m_hwModule)){
                    ret = 1;
                }
                //
                if(ret == 0)
                    this->m_bStartUniStreaming = MFALSE;

                PIPE_INF("stop uni streaming -\n");
            }

            this->mRunTimeCfgLock.unlock();
            break;
        case EPIPECmd_ALLOC_UNI:
            //mutex protection is for isp start & uni start
            this->mRunTimeCfgLock.lock();
            PIPE_DBG("start uni streaming +\n");
            if(this->m_camPass1Param.bypass_tg == MTRUE){
                PIPE_ERR("uni can't be allocated under rawi\n");
                ret = 1;
            }
            else{
                if(this->m_FSM == op_cfg){
                    //uni_mgr start is at isp_start
                    if(this->m_UniMgr.UniMgr_attach(UNI_A,this->m_pCmdQMgr,NULL) != 0){
                        ret = 1;
                    }
                }
                else{
                    PIPE_ERR("uni can only be allocated under cfg stage, current stage :0x%x\n",this->m_FSM);
                    ret = 1;
                }
            }

            this->m_CamPathPass1.m_UniMgr.m_module = m_UniMgr.UniMgr_getUniModule(this->m_hwModule);
            this->m_CamIO_stt.m_UniMgr.m_module = m_UniMgr.UniMgr_getUniModule(this->m_hwModule);

            //
            if(ret == 0)
                this->m_bStartUniStreaming = MTRUE;

            PIPE_DBG("start uni streaming -\n");

            this->mRunTimeCfgLock.unlock();
            break;
        case EPIPECmd_SET_EIS_CBFP:
            {
                MVOID* ptr = NULL;
                if(this->m_FSM != op_cfg){
                    //because of sw_p1 done grping
                    PIPE_ERR("EIS_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                    ret = 1;
                }

                //check hook is ok or not
                if(this->m_UniMgr.UniMgr_SetCB(this->m_hwModule,UniMgr::_EIS_CB_,(MVOID*)arg1) == MFALSE){
                    return MFALSE;
                }

                //hook into cam_path
                if((ptr = this->m_UniMgr.UniMgr_GetCB(this->m_hwModule,UniMgr::_EIS_CB_))!= NULL){
                    this->m_CamPathPass1.P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)ptr);
                }
                else
                    return MFALSE;
            }
            break;
        case EPIPECmd_SET_SGG2_CBFP:
            {
                MVOID* ptr = NULL;
                //check hook is ok or not
                if(this->m_UniMgr.UniMgr_SetCB(this->m_hwModule,UniMgr::_SGG2_CB_,(MVOID*)arg1) == MFALSE){
                    return MFALSE;
                }

                //hook into cam_path
                if((ptr = this->m_UniMgr.UniMgr_GetCB(this->m_hwModule,UniMgr::_SGG2_CB_))!= NULL){
                    this->m_CamPathPass1.P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)ptr);
                }
                else
                    return MFALSE;
            }
            break;
        case EPIPECmd_SET_LCS_CBFP:
            if(this->m_FSM != op_cfg){
                //because of sw_p1 done grping
                PIPE_ERR("LCS_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            if(this->m_CamPathPass1.P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }

            break;
        case EPIPECmd_SET_RSS_CBFP:
            {
                MVOID* ptr = NULL;

                if(this->m_FSM != op_cfg){
                    //because of sw_p1 done grping
                    PIPE_ERR("RSS_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                    ret = 1;
                }

                //check hook is ok or not
                if(this->m_UniMgr.UniMgr_SetCB(this->m_hwModule,UniMgr::_RSS_CB_,(MVOID*)arg1) == MFALSE){
                    return MFALSE;
                }

                //hook into cam_path
                if((ptr = this->m_UniMgr.UniMgr_GetCB(this->m_hwModule,UniMgr::_RSS_CB_))!= NULL){
                    this->m_CamPathPass1.P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)ptr);
                }
                else
                    return MFALSE;
            }
            break;
        case EPIPECmd_SET_STT_SUSPEND_CB:
            //CBFP for clear m_TimeClk of BUF_CTRL
            if(!arg1 || !arg2){
               PIPE_ERR("null STT_CBFP(%x_%x)\n",arg1,arg2);
                    return MFALSE;
            }
            this->m_Stt_rstBufCtrl_CBFP  = (fp_CB)arg1;
            this->m_Stt_CBCookie = (void*)arg2;
            break;
        case EPIPECmd_GET_TOP_INFO:
            //for Get top reg info (CqTuningMgr)
            //arg1: [output] MUINT32*, cam_ctl_en
            //arg2: [output] MUINT32*, ctl_dma_en
            //arg3: [output] MUINT32*, mSubsample
            {
                if(!arg1|| !arg2 ||!arg3){
                    PIPE_ERR("null parameter (%x_%x_%x)\n", arg1,arg2,arg3);
                    return MFALSE;
                }

                *(MUINT32*)arg1 = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0],CAM_CTL_EN);
                *(MUINT32*)arg2 = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0],CAM_CTL_DMA_EN);
                *(MUINT32*)arg3 = this->m_camPass1Param.m_subSample;
            }
            break;
        case EPIPECmd_SET_FAST_AF:
            //StatisticPipe::configPipe will use this cmd to set FastAF be TRUE
            if (this->m_FSM != op_cfg){
                PIPE_ERR("set Fast AF mode must set between configPipe~ 1st Enque\n");
                return MFALSE;
            }
            this->m_FastAF = MTRUE;
            break;
        case EPIPECmd_SET_REGDUMP_CBFP:
            if(this->m_FSM != op_cfg){
                PIPE_ERR("REGDUMP_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            if(this->m_CamPathPass1.P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }
            break;
        case EPIPECmd_HIGHSPEED_AE:
            //arg1: MUINT32 :signal idx, start from 0,
            //arg2: list<ISPIO_REG_CFG>* data for isp
            if((MUINT32)arg1 > this->m_camPass1Param.m_subSample){
                PIPE_ERR("out of range[0x%x_0x%x].can't send data at this idx\n",arg1,this->m_camPass1Param.m_subSample);
                return MFALSE;
            }
            if(this->m_pHighSpeedCmdQMgr){
                //
                ret = this->m_pHighSpeedCmdQMgr->CmdQMgr_Cfg(arg1,arg2);
                if(this->m_TwinMgr->getIsTwin())
                    ret += this->m_TwinMgr->CQ_cfg(ISP_DRV_CQ_THRE11,arg1,arg2);

                if(ret == 0){
                    ret = this->m_pHighSpeedCmdQMgr->CmdQMgr_start();
                    if(this->m_TwinMgr->getIsTwin())
                        ret += this->m_TwinMgr->CQ_Trig(ISP_DRV_CQ_THRE11);
                }
                else{
                    PIPE_ERR("thread11 CFG fail\n");
                }
            }
            else{
                PIPE_ERR("only supported under high-speed mode\n");
                return MFALSE;
            }


            break;
        case EPIPECmd_SET_TG_INT_LINE:
            //arg1: TG_INT1_LINENO
            //arg2: sensor pxlMode
            {
                CAM_REG_TG_SEN_GRAB_PXL TG_W;
                CAM_REG_TG_SEN_GRAB_LIN TG_H;
                MUINT32 lineno = (MUINT32)arg1;
                MUINT32 pixMode = (NSImageio::NSIspio::EPxlMode)arg2;
                MUINT32 size_h,size_v;

                PIPE_DBG("set TgLine(%d)pxmod(%d)\n", lineno, pixMode);

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

                /*sensor pix mode*/
                if(pixMode == NSImageio::NSIspio::ePxlMode_Two_)
                    size_h *=2;
                else if(pixMode == NSImageio::NSIspio::ePxlMode_Four_)
                    size_h *=4;

                if(lineno > size_v) {
                    PIPE_ERR("ERROR: TG lineno(%d) must < %d\n",lineno,size_v);
                    return MFALSE;
                }

                CAM_WRITE_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_INT1,TG_INT1_LINENO,lineno);
                CAM_WRITE_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_INT1,TG_INT1_PXLNO,(size_h>>1));

                break;
            }
        case EPIPECmd_SET_RRZ:
            //Arg1: list<STImgCrop>*
            //arg2: list<STImgResize>*  //only tar_w & tar_h r useful here.
            {
                list<STImgCrop>* pInput_L;
                list<STImgResize>* pInput2_L;
                list<STImgCrop>::iterator it;
                list<STImgResize>::iterator it2;

                list<IspRrzCfg> rrz_L;
                IspRrzCfg rrz;

                MUINT32 bin_en;

                if ((0 == arg1)||(0 == arg2)) {
                    PIPE_ERR("SET_RRZ NULL param(0x%x_0x%x)!\n",arg1,arg2);
                    return MFALSE;
                }

                pInput_L = (list<STImgCrop>*)arg1;
                pInput2_L = (list<STImgResize>*)arg2;

                if( (pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) || (pInput2_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ())){
                    PIPE_ERR("rrz data length mismatch:0x%x_0x%x_0x%x\n",pInput_L->size(),pInput2_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
                    return MFALSE;
                }

                for(it=pInput_L->begin(),it2=pInput2_L->begin();it!=pInput_L->end();it++,it2++){
                    if (0 == ((MUINT64)it->w * it->h * it2->tar_w * it2->tar_h)) {
                        PIPE_ERR("SET_RRZ Invalid Para(0x%x/0x%x/0x%x/0x%x)!\n",\
                            it->w, it->h,\
                            it2->tar_w , it2->tar_h);
                        return MFALSE;
                    }


                    if (it->w < it2->tar_w) {
                        PIPE_ERR("ERROR: Not Support Scale up(in(%d)<out_w(%d))\n",it->w, it2->tar_w);
                        return MFALSE;
                    }
                    if (it->h < it2->tar_h) {
                        PIPE_ERR("ERROR: Not Support Scale up(in(%d)<out_h(%d))\n",it->h, it2->tar_h);
                        return MFALSE;
                    }
                    if ( (it->w+it->x) > this->m_camPass1Param.m_src_size.w) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_w(%d))\n", \
                            (it->w+it->x), \
                            this->m_camPass1Param.m_src_size.w);
                        return MFALSE;
                    }
                    if ( (it->h+it->y) > this->m_camPass1Param.m_src_size.h) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_h(%d))\n", \
                            (it->h+it->y), \
                            this->m_camPass1Param.m_src_size.h);
                        return MFALSE;
                    }

                    // If DBN=1, rrz setting must be divide by 2; if DBN = 1 and BIN = 1, imgo setting must be divide by 4
                    // This is performed in CAM_RRZ_CTRL::_config(void) of isp_function_cam cause rrz_in size need to be processed in it

                    // Reminder!! multiply rrz_in_roi to 2 when BIN = 1 cause MW passed this param according to RRZ input
                    // But CAM_RRZ_CTRL::_config(void) will divide it still
                    bin_en = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);

                    rrz.rrz_in_roi.x        = it->x << (bin_en);
                    rrz.rrz_in_roi.y        = it->y << bin_en;
                    rrz.rrz_in_roi.floatX   = 0;
                    rrz.rrz_in_roi.floatY   = 0;
                    rrz.rrz_in_roi.w        = it->w << (bin_en);
                    rrz.rrz_in_roi.h        = it->h << bin_en;
                    rrz.rrz_out_size.w      = it2->tar_w;
                    rrz.rrz_out_size.h      = it2->tar_h;

                    rrz_L.push_back(rrz);
                }

                if ( 0 != this->m_CamPathPass1.setP1RrzCfg(&rrz_L) ){
                    PIPE_ERR("SET_RRZ setP1RrzCfg Fail\n");
                    return MFALSE;
                }
                break;
            }
        case EPIPECmd_SET_P1_UPDATE:
            //Arg2: magic number, list<MUINT32>
            //Arg3: sensorDev,MUINT32, not a list
            {
                list<MUINT32>* pInput_L = (list<MUINT32>*)arg2;
                list<MUINT32>::iterator it;
                IspP1TuningCfg cfg;
                list<IspP1TuningCfg> cfg_L;

                for(it = pInput_L->begin();it!= pInput_L->end(); it++){
                    cfg.magic = *it;
                    cfg.SenDev = arg3;

                    cfg_L.push_back(cfg);
                }

                /* Note: setP1TuneCfg() must call BEFORE setP1Notify(),
                   Reason: setLCS CB needs magic#
                */
                if ( 0 != this->m_CamPathPass1.setP1TuneCfg(&cfg_L) ){
                    PIPE_ERR("P1_UPDATE setP1TuneCfg Fail\n");
                    return MFALSE;
                }

                if(0 != this->m_CamPathPass1.setP1Notify()){
                    PIPE_ERR("some notified-module fail\n");
                }

                // If twin is enable:
                // 1. Compute the CQ data via twin_drv (dual_isp_driver) -- runTwin()
                // 2. Only twin module: CmdQMgr_update
                if(MTRUE == this->m_TwinMgr->getIsTwin()){
                    if(MFALSE == this->m_TwinMgr->runTwinDrv(this->m_FastAF ? TWN_SCENARIO_AF_FAST_P1: TWN_SCENARIO_NORMAL)){
                        PIPE_ERR("Run Twin drv fail\n");
                        return MFALSE;
                    }

                    if(MFALSE == this->m_TwinMgr->updateTwin()){
                        PIPE_ERR("Update Twin CQ fail\n");
                        return MFALSE;
                    }
                }

                if(this->m_pCmdQMgr->CmdQMgr_update()){
                    PIPE_ERR("CQ undate fail\n");
                    return MFALSE;
                }

                if(ret ==0 )
                    this->m_bUpdate = MTRUE;
            }
            break;
        case EPIPECmd_SET_IMGO:
            //Arg1: list<STImgCrop>*
            //arg2: rawtype: list<MUINT32>*, 1:proc, 0:pure
            {
                list<STImgCrop>* pInput_L;
                list<MUINT32>* pInput2_L;
                list<STImgCrop>::iterator it;
                list<MUINT32>::iterator it2;
                DMACfg imgo;
                list<DMACfg> input_L;

                if (0 == arg1) {
                    PIPE_ERR("SET_IMGO NULL param(0x%x)!");
                    return MFALSE;
                }

                pInput_L = (list<STImgCrop>*)arg1;
                pInput2_L = (list<MUINT32>*)arg2;


                if( (pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) || (pInput2_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) ){
                    PIPE_ERR("imgo data length mismatch:0x%x_0x%x_0x%x\n",pInput_L->size(),pInput2_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
                    return MFALSE;
                }

                for(it2=pInput2_L->begin(),it=pInput_L->begin();it!=pInput_L->end();it++,it2++){
                    //out size don't case
                    if (0 == (it->w * it->h )) {
                        PIPE_ERR("SET_IMGO Invalid Para(0x%x/0x%x)!\n",\
                            it->w, it->h);
                        return MFALSE;
                    }


                    if ( (it->x + it->w) > this->m_camPass1Param.m_src_size.w) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_w(%d))\n",\
                            (it->x + it->w), this->m_camPass1Param.m_src_size.w);
                        return MFALSE;
                    }
                    if ( (it->y + it->h) > this->m_camPass1Param.m_src_size.h) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_h(%d))\n", \
                            (it->y + it->h), this->m_camPass1Param.m_src_size.h);
                        return MFALSE;
                    }

                    // If DBN=1, imgo setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
                    // This is performed in DMAO_B::_config(void) of isp_function_cam.dmax cause vOutPorts in configPipe() is read-only object (constant)

                    imgo.crop.x = it->x;
                    imgo.crop.y = it->y;
                    imgo.crop.w = it->w;
                    imgo.crop.h = it->h;
                    imgo.crop.floatX = imgo.crop.floatY = 0;

                    if(this->m_DynamicRawType==MFALSE){
                        switch(this->m_camPass1Param.m_NonScale_RawType){
                            case CamPathPass1Parameter::_PURE_RAW:
                                imgo.rawType = 1;
                                //can't read phy, to avoid read before cq-trig at the very 1st enque before isp_start
                                if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0],CAM_CTL_EN,DBN_EN) == 1){
                                    PIPE_ERR("support no IMGO_PURE_RAW + DUAL_PIX data pattern\n");
                                    return MFALSE;
                                }
                                break;
                            case CamPathPass1Parameter::_NON_LSC_RAW:
                                imgo.rawType = 2;
                                break;
                            case CamPathPass1Parameter::_PROC_RAW:
                                imgo.rawType = 0;
                                break;
                        }
                    }
                    else{
                        imgo.rawType = *it2;
                    }

                    input_L.push_back(imgo);
                }

                if ( 0 != this->m_CamPathPass1.setP1ImgoCfg(&input_L) ){
                    PIPE_ERR("SET_IMGO setP1ImgoCfg fail\n");
                    return MFALSE;
                }

            }
            break;
        case EPIPECmd_SET_FRM_TIME:
            if (this->FSM_CHECK(op_cmd) == MFALSE) {
                PIPE_ERR("EPIPECmd_SET_FRM_TIME FSM error");
                ret = MFALSE;
                break;
            }

            if (MFALSE == this->m_CamPathPass1.updateFrameTime((MUINT32)arg1, (MUINT32)arg2)) {
                PIPE_ERR("Update frame time fail: arg1:%d arg2:%d", (MUINT32)arg1, (MUINT32)arg2);
                ret = MFALSE;
                break;
            }

            break;
        case EPIPECmd_SET_UFEO:
            {
                list<STImgResize>* pInput_L;
                list<STImgResize>::iterator it;
                DMACfg ufeo;
                list<DMACfg> ufeo_L;

                if (0 == arg1) {
                    PIPE_ERR("SET_UFEO NULL param(0x%x)!");
                    return MFALSE;
                }

                pInput_L = (list<STImgResize>*)arg1;

                if( pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()){
                    PIPE_ERR("ufeo data length mismatch:0x%x_0x%x\n",pInput_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
                    return MFALSE;
                }

                for(it = pInput_L->begin(); it != pInput_L->end(); it++){
                    if (0 == ((MUINT64)it->tar_w * it->tar_h)) {
                        PIPE_ERR("SET_UFEO Invalid Para(0x%x/0x%x)!\n",\
                            it->tar_w, it->tar_h);
                        return MFALSE;
                    }
                    ufeo.out.w      = it->tar_w;
                    ufeo.out.h      = it->tar_h;

                    ufeo_L.push_back(ufeo);
                }

                if ( 0 != this->m_CamPathPass1.setP1UfeoCfg(&ufeo_L) ){
                    PIPE_ERR("SET_Ufeo setP1UfeoCfg Fail\n");
                    return MFALSE;
                }
            }
			break;
        case EPIPECmd_GET_CUR_FRM_STATUS:
            //arg1: output 32bit data.
            //          CAM_FST_NORMAL              = 0,
            //          CAM_FST_DROP_FRAME          = 1,
            //          CAM_FST_LAST_WORKING_FRAME  = 2,
            {
                MUINT32 _flag = 0;
                switch(this->m_hwModule){
                    case CAM_A:
                        _flag = ISP_IRQ_TYPE_INT_CAM_A_ST;
                        break;
                    case CAM_B:
                        _flag = ISP_IRQ_TYPE_INT_CAM_B_ST;
                        break;
                    default:
                        PIPE_ERR("unsupported module\n");
                        return MFALSE;
                        break;
                }
                this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getDeviceInfo(_GET_DROP_FRAME_STATUS,(MUINT8 *)&_flag);
                *(MUINT32*)arg1 = _flag;
            }
            break;
        case EPIPECmd_GET_CUR_SOF_IDX:
            //arg1: output 32bit data.[sof counter:8bit]
            {
                MUINT32 _flag = 0;
                switch(this->m_hwModule){
                    case CAM_A:
                        _flag = ISP_IRQ_TYPE_INT_CAM_A_ST;
                        break;
                    case CAM_B:
                        _flag = ISP_IRQ_TYPE_INT_CAM_B_ST;
                        break;
                    default:
                        PIPE_ERR("unsupported module\n");
                        return MFALSE;
                        break;
                }
                this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *)&_flag);
                *(MUINT32*)arg1 = _flag;
            }
            break;
        case EPIPECmd_GET_UNI_INFO:
            //arg1: output MTRUE or MFALSE.
            //         MTRUE: this path support uni at current cfg
            if(this->m_UniMgr.UniMgr_GetCurLink() ==  this->m_hwModule){
                *(MUINT32*)arg1 = MTRUE;
            }
            else{
                PIPE_DBG("UNI is connected with cam:%d\n",this->m_UniMgr.UniMgr_GetCurLink());
                *(MUINT32*)arg1 = MFALSE;
            }
            break;
        case EPIPECmd_GET_FLK_INFO:
            //arg1: output MTRUE or MFALSE.
            //         MTRUE: this path support uni at current cfg
            if(this->m_UniMgr.UniMgr_GetCurAttachModule() ==  this->m_hwModule){
                *(MUINT32*)arg1 = MTRUE;
            }
            else{
                PIPE_DBG("FLK is connected with cam:%d\n",this->m_UniMgr.UniMgr_GetCurAttachModule());
                *(MUINT32*)arg1 = MFALSE;
            }
            break;
        case EPIPECmd_GET_HBIN_INFO:
            //arg1: output size_w
            //arg2: output size_h
            {
                CAM_REG_TG_SEN_GRAB_PXL TG_W;
                CAM_REG_TG_SEN_GRAB_LIN TG_H;
                MUINT32 pixMode;
                MUINT32 bin=0;
                MUINT32 size_h,size_v;

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
                bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,DBN_EN);
                size_h = size_h>>bin;
                bin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                size_v = size_v>>bin;

                //qbin1
                pixMode = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN1_MODE, QBN_ACC);
                *(MUINT32*)arg1  = size_h >> pixMode;
                *(MUINT32*)arg2  = size_v;
                //qbin2, pix mode must be the same with qbin1
                if(pixMode != CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN2_MODE, QBN_ACC)){
                    PIPE_ERR("QBIN1 pix mode is different from QBIN2:0x%x_0x%x\n",pixMode\
                        ,CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN2_MODE, QBN_ACC));
                }
                //pixMode = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN2_MODE, QBN_ACC);
                //*(MUINT32*)arg1  = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> pixMode;
                //*(MUINT32*)arg2  = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
                //qbin3,pix mode must be the same with qbin1, but it should read virReg before isp start
                if(this->m_UniMgr.UniMgr_GetCurLink() == this->m_hwModule){
                    MUINT32 dupIdx = this->m_pCmdQMgr->CmdQMgr_GetDuqQIdx();
                    MUINT32 burstIdx = this->m_pCmdQMgr->CmdQMgr_GetBurstQIdx();
                    if(pixMode != UNI_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupIdx)[burstIdx],CAM_UNI_QBN3_A_MODE, QBN_ACC)){
                        PIPE_ERR("QBIN1 pix mode is different from QBIN3:0x%x_0x%x\n",pixMode\
                            ,UNI_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupIdx)[burstIdx],CAM_UNI_QBN3_A_MODE, QBN_ACC));
                    }
                }
                //pixMode = UNI_READ_BITS(this->m_UniMgr.UniMgr_GetCurObj(),CAM_UNI_QBN3_A_MODE, QBN_ACC);
                //*(MUINT32*)arg1  = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S) >> pixMode;
                //*(MUINT32*)arg2  = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

                //qbin4, pix mode must be the same with qbin1
                if(pixMode != CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN4_MODE, QBN_ACC)){
                    PIPE_ERR("QBIN1 pix mode is different from QBIN4:0x%x_0x%x\n",pixMode\
                        ,CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN4_MODE, QBN_ACC));
                }

            }
            break;
        case EPIPECmd_GET_BIN_INFO:
            //arg1: output size_w
            //arg2: output size_h
            {
                CAM_REG_TG_SEN_GRAB_PXL TG_W;
                CAM_REG_TG_SEN_GRAB_LIN TG_H;
                MUINT32 pixMode;
                MUINT32 bin=0;
                MUINT32 size_h,size_v;

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
                bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,DBN_EN);
                *(MUINT32*)arg1 = size_h>>bin;
                bin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                *(MUINT32*)arg2 = size_v>>bin;
            }
            break;
        case EPIPECmd_GET_IMGO_INFO:
            /* for MW set IMGO crop size before enque,
               MW don't know what raw type driver will force IMGO to
            -----------------------------------------------------------
            arg1: [out] - MSize[2]
                  [0] processed taw IMGO size
                  [1] pure raw IMGO size                             */
            {
                if(!arg1){
                    PIPE_ERR("[Error]arg1=NULL");
                    return MFALSE;
                }
                CAM_REG_TG_SEN_GRAB_PXL TG_W;
                CAM_REG_TG_SEN_GRAB_LIN TG_H;
                MUINT32 bin=0,dbn=0;
                MUINT32 size_h,size_v;
                MBOOL bImgoPure = (MBOOL)arg1;

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
                bin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                dbn = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,DBN_EN);

                MSize *szIMGO = (MSize*)arg1;
                for(MUINT32 i=0; i<2; i++){
                    if(i==1){//pure raw
                        if(dbn){//enable dbn, force imgo as processed raw
                            szIMGO[i].w = 0;
                            szIMGO[i].h = 0;
                        } else {
                            szIMGO[i].w = size_h;
                            szIMGO[i].h = size_v;
                        }
                    } else {//processed raw
                        if(bin){//enable bin, force imgo as pure raw
                            szIMGO[i].w = 0;
                            szIMGO[i].h = 0;
                        } else {
                            szIMGO[i].w = size_h>>(bin + dbn);
                            szIMGO[i].h = size_v>>bin;
                        }
                    }
                }
            }
            break;
        case EPIPECmd_GET_PMX_INFO:
            //arg1: output size_w
            //arg2: output size_h
            {
                CAM_REG_TG_SEN_GRAB_PXL TG_W;
                CAM_REG_TG_SEN_GRAB_LIN TG_H;
                MUINT32 pbin=0, pmx_sel=0;
                MUINT32 size_h,size_v;

                if ((0 == arg1)||(0 == arg2)) {
                    PIPE_ERR("GET_PMX_INFO NULL param(0x%x_0x%x)!\n",arg1,arg2);
                    return MFALSE;
                }

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S);

                pmx_sel = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_SEL,PMX_SEL);
                pbin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,PBN_EN);

                if(!pmx_sel || !pbin){
                    *(MUINT32*)arg1 = size_h;
                    *(MUINT32*)arg2 = size_v;
                }
                else {
                    *(MUINT32*)arg1 = PBIN_WIDTH(size_h);
                    *(MUINT32*)arg2 = PBIN_HEIGHT(size_v);
                }
            }
            break;
        case EPIPECmd_GET_CQUPDATECNT:
            //arg1: MTRUE for get phy reg, MFALSE for get vir reg.
            //arg2: return output info.
            *(MUINT32*)arg2 = this->m_pCmdQMgr->CmdQMgr_GetCounter(arg1);

            break;
        case EPIPECmd_UNI_SWITCHOUT:
            if(this->m_UniMgr.UniMgr_SwitchOut(this->m_hwModule)) {
                ret = -1;
                PIPE_ERR("UniMgr_SwitchOut error!");
            }
            break;
        case EPIPECmd_UNI_SWITCHOUTRIG:
            if(this->m_UniMgr.UniMgr_SwitchOutTrig(this->m_hwModule)) {
                ret = -1;
                PIPE_ERR("UniMgr_SwitchOutTrig error!");
            }
            break;
        case EPIPECmd_GET_UNI_SWITCHOUTRIG_DONE:
            if(this->m_UniMgr.UniMgr_GetCurLink() == this->m_hwModule){
                *(MUINT32*)arg1 = !(UNI_READ_BITS(this->m_UniMgr.UniMgr_GetCurObj(), CAM_UNI_TOP_MOD_EN, EIS_A_EN) | \
                                                    UNI_READ_BITS(this->m_UniMgr.UniMgr_GetCurObj(), CAM_UNI_TOP_MOD_EN, RSS_A_EN) | \
                                                    UNI_READ_BITS(this->m_UniMgr.UniMgr_GetCurObj(), CAM_UNI_TOP_DMA_EN, EISO_A_EN) | \
                                                    UNI_READ_BITS(this->m_UniMgr.UniMgr_GetCurObj(), CAM_UNI_TOP_DMA_EN, RSSO_A_EN));
            } else {
                ret = -1;
            }
            break;
        case EPIPECmd_UNI_SWITCHIN:
            if(this->m_UniMgr.UniMgr_SwitchIn(this->m_hwModule)) {
                ret = -1;
                PIPE_ERR("UniMgr_SwitchIn error!");
            }
            break;
        case EPIPECmd_UNI_SWITCHINTRIG:
            if(this->m_UniMgr.UniMgr_SwitchInTrig(this->m_hwModule)) {
                ret = -1;
                PIPE_ERR("UniMgr_SwitchInTrig error!");
            }
            break;
        case EPIPECmd_GET_UNI_ATTACH_MODULE:
            *(MUINT32*)arg1 = (MUINT32)this->m_UniMgr.UniMgr_GetCurAttachModule();
            break;
        default:
            PIPE_ERR("NOT support command!");
            ret = -1;
            break;
    }
EXIT:
    if( ret != 0 )
    {
        PIPE_ERR("sendCommand(0x%x) error!",cmd);
        return MFALSE;
    }
    return  MTRUE;
}

MBOOL CamIOPipe::configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,EIMAGE_STRIDE planeNum)
{

    ISP_QUERY_RST queryRst;
    isBypassOffset;
    //
    a_dma.memBuf.size        = portInfo->u4BufSize[planeNum];
    a_dma.memBuf.base_vAddr  = portInfo->u4BufVA[planeNum];
    a_dma.memBuf.base_pAddr  = portInfo->u4BufPA[planeNum];
    //
    a_dma.memBuf.alignment  = 0;
    a_dma.pixel_byte        = pixel_Byte;
    //original dimension  unit:PIXEL
    a_dma.size.w            = portInfo->u4ImgWidth;
    a_dma.size.h            = portInfo->u4ImgHeight;
    //input stride unit:PIXEL
    a_dma.size.stride       =  portInfo->u4Stride[planeNum];

    //
    #if 0//in p1, dmao format use hw-auto
    a_dma.format = portInfo->eImgFmt;
    #else//this is for  ispio_utility.h query behavior  at cam_path_pass1.cpp
    a_dma.lIspColorfmt = portInfo->eImgFmt;
    #endif
    //dma port capbility
    a_dma.capbility=portInfo->capbility;
    //input xsize unit:byte
    ISP_QuerySize(portInfo->index,ISP_QUERY_STRIDE_BYTE,(EImageFormat)portInfo->eImgFmt,a_dma.size.w,queryRst);

    a_dma.size.xsize        =  queryRst.stride_byte;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%d) < image width(%d) byte size",a_dma.size.stride,a_dma.size.w);
    }
    //
    a_dma.crop.x            = portInfo->crop1.x;
    a_dma.crop.floatX       = 0;//portInfo->crop1.floatX;
    a_dma.crop.y            = portInfo->crop1.y;
    a_dma.crop.floatY       = 0;//portInfo->crop1.floatY;
    a_dma.crop.w            = portInfo->crop1.w;
    a_dma.crop.h            = portInfo->crop1.h;
    //
    a_dma.swap = swap;
    //
    a_dma.memBuf.ofst_addr = 0;//offset at isp function
    //

    switch( portInfo->eImgFmt ) {
        case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=1;
            break;
        case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=0;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=3;
            break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
            a_dma.format_en=1;
            a_dma.format=1;
            a_dma.bus_size_en=1;
            a_dma.bus_size=1;
            a_dma.swap=2;
            break;
        case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=1;
            //break;
        case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=2;
            //break;
        case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
            //a_dma.format_en=1;
            //a_dma.format=2;
            //a_dma.bus_size_en=1;
            //a_dma.bus_size=3;
            //break;
        case eImgFmt_YV16:      //422 format, 3 plane
        case eImgFmt_NV16:      //422 format, 2 plane
            PIPE_ERR("NOT support this format(0x%x) in cam\n",portInfo->eImgFmt);
            break;
        case eImgFmt_BAYER8:    /*!< Bayer format, 8-bit */
        case eImgFmt_BAYER10:   /*!< Bayer format, 10-bit */
        case eImgFmt_BAYER12:   /*!< Bayer format, 12-bit */
        case eImgFmt_BAYER14:   /*!< Bayer format, 14-bit */
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=1;
            break;
        case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
        case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
        default:
            a_dma.format_en=0;
            a_dma.bus_size_en=0;
            a_dma.format=0;
            a_dma.bus_size=0;
            break;
    }
    //

    PIPE_DBG("w(%d),h(%d),stride(%d),xsize(%d),crop(%d,%d,%d,%d)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%x),pa(0x%x),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        a_dma.memBuf.base_vAddr, a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",portInfo->eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
_isp_dma_enum_ CamIOPipe::CAMIO_STT::PortID_MAP(MUINT32 PortID)
{
    switch(PortID){
        case EPortIndex_LCSO:
            return _lcso_;
            break;
        case EPortIndex_EISO:
            if(this->m_UniMgr.UniMgr_GetCurLink() ==  this->m_hwModule)
                return _eiso_;
            else if(this->m_UniMgr.UniMgr_GetCurLink() == CAM_MAX){
                PIPE_WRN("EIS is not linked to any cam");
                return _eiso_;
            }
            else
                PIPE_ERR("EIS is occupied by cam:0x%x",this->m_UniMgr.UniMgr_GetCurAttachModule());
            break;
        case EPortIndex_RSSO:
            if(this->m_UniMgr.UniMgr_GetCurLink() ==  this->m_hwModule)
                return _rsso_;
            else if(this->m_UniMgr.UniMgr_GetCurLink() == CAM_MAX){
                PIPE_WRN("RSS is not linked to any cam");
                return _rsso_;
            }
            else
                PIPE_ERR("RSS is occupied by cam:0x%x",this->m_UniMgr.UniMgr_GetCurAttachModule());
            break;
        default:
            PIPE_ERR("un-supported portID:0x%x\n",PortID);
            break;
    }
    return _cam_max_;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 CamIOPipe::CAMIO_STT::QueryPort(PortID const portID)
{
    return (MUINT32)this->PortID_MAP(portID.index);
}

/*******************************************************************************
* return value:
* 1: isp is already stopped
* 0: sucessed
*-1: fail
********************************************************************************/
MINT32 CamIOPipe::CAMIO_STT::enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MUINT32 nBurst)
{
    MUINT32         dmaChannel = 0;
    ISP_BUF_INFO_L  buf_L;
    stISP_BUF_INFO  bufInfo;

    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        return -1;
    }

    if(nBurst!= rQBufInfo.vBufInfo.size()){
        PIPE_ERR("dma:0x%x:enque buf number is mismatched with subsample (0x%x_0x%x)\n",dmaChannel,\
            rQBufInfo.vBufInfo.size(),nBurst);
        return -1;
    }

    for(MUINT32 i=0;i<rQBufInfo.vBufInfo.size();i++){
        //repalce
        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            PIPE_ERR("replace function are not supported in CAMIO_STT\n");
            return -1;
        }

        bufInfo.u_op.enque.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].u4BufPA[0];
        bufInfo.u_op.enque.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].u4BufVA[0];
        bufInfo.u_op.enque.image.mem_info.size      = rQBufInfo.vBufInfo[i].u4BufSize[0];
        bufInfo.u_op.enque.image.mem_info.memID     = rQBufInfo.vBufInfo[i].memID[0];
        bufInfo.u_op.enque.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].bufSecu[0];
        bufInfo.u_op.enque.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].bufCohe[0];

        //header
        bufInfo.u_op.enque.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA;
        bufInfo.u_op.enque.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA;
        bufInfo.u_op.enque.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize;
        bufInfo.u_op.enque.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID;
        bufInfo.u_op.enque.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu;
        bufInfo.u_op.enque.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe;

        if(this->m_UniMgr.UniMgr_GetCurAttachModule() != this->m_hwModule && (dmaChannel == _eiso_ || dmaChannel == _rsso_)) {
            bufInfo.u_op.enque.image.mem_info.size = 0;
        }

        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08x),(PA:0x%08x),(FH_VA:0x%08x)\n",\
            i,\
            portID.index, bufInfo.u_op.enque.image.mem_info.memID,\
            bufInfo.u_op.enque.image.mem_info.va_addr,\
            bufInfo.u_op.enque.image.mem_info.pa_addr,\
            bufInfo.u_op.enque.header.va_addr);
        //
        buf_L.push_back(bufInfo);
    }

    //
    if ( 0 != this->pCamPath->enqueueBuf( dmaChannel, buf_L) ) {
        PIPE_ERR("ERROR:DMA[%d] enqueOutBuf fail\n",dmaChannel);
        return -1;
    }

    return 0;
}



/*******************************************************************************
* return value:
* 1: isp is already stopped
* 0: sucessed
*-1: fail
********************************************************************************/

MINT32 CamIOPipe::CAMIO_STT::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo,MUINT32 nBurst, CAM_STATE_NOTIFY *pNotify)
{
    MUINT32 dmaChannel = 0;
    MINT32 ret = 0;

    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        return -1;
    }

    //clear remained data in container
    rQBufInfo.vBufInfo.clear();

    //
    if ( (ret = this->pCamPath->dequeueBuf( dmaChannel,rQBufInfo.vBufInfo, 0, pNotify)) != 0) {
        if ( ret > 0 )
            PIPE_DBG("deq dma[%d] FSM=stop", dmaChannel);
        else
            PIPE_ERR("ERROR:dma[%d] dequeOutBuf fail\n", dmaChannel);
        return ret;
    }


    if(rQBufInfo.vBufInfo.size() != nBurst)
        PIPE_ERR("dequeout data length is mismatch\n");


    return  ret;
}
/*******************************************************************************
*
********************************************************************************/

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

