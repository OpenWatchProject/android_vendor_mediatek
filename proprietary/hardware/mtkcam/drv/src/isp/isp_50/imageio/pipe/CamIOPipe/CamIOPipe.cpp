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
#include <mtkcam/utils/std/ULog.h>
//
#include "CamIOPipe.h"
#include "StatisticIOPipe.h"//for icamiopipe::createinstance & icamiopipe::destroyinstance
#include "CamsvIOPipe.h"

//
#include <cutils/properties.h>  // For property_get().

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>  // for irq status mapping
#include "cam_capibility.h"
#include "cam_dfs.h"            //for dynamic frequency switch


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

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);

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
#undef PIPE_WRN

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

#define PIPE_WRN(fmt, arg...)        do {\
        if (pipe_DbgLogEnable_ERROR  ) { \
            BASE_LOG_WRN("[0x%x]:" fmt,this->m_hwModule, ##arg); \
        }\
    } while(0)


#define PIPE_ERR(fmt, arg...)        do {\
    if (pipe_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define _STRUCT_OFST(_stype, _field) ((MUINT32)(MUINTPTR)&(((_stype*)0)->_field))

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
            return TG_A;

        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G_D:
            if (MTRUE == btwin){
                 return  TG_A;
            }
            else {
                 return TG_B;
            }

        default:
            CAM_ULOGME("_p1ConvSelectIdToTg::NotSupportModule(%d). Use Enum EPipe_P1SEL",id);
            return  EPortIndex_UNKNOW;
    }
    return  TG_A;
}
#endif

/*******************************************************************************
*
********************************************************************************/
ICamIOPipe* ICamIOPipe::createInstance( MINT8 const szUsrName[32], E_INPUT& InOutPut, E_PIPE_OPT opt, PortInfo const *portInfo)
{
    switch(InOutPut){
        case TG_A:
        case TG_B:
        case TG_CAM_MAX:
        case TG_CAMSV_0:
        case TG_CAMSV_1:
        case TG_CAMSV_2:
        case TG_CAMSV_3:
        case TG_CAMSV_4:
        case TG_CAMSV_5:
        case TG_CAMSV_MAX:
            break;
        default:
            BASE_LOG_ERR("unsupported input source:0x%x\n",InOutPut);
            return NULL;
            break;
    }

    switch(opt){
        case CAMIO:
            return  (ICamIOPipe*)CamIOPipe::Create(szUsrName,InOutPut,portInfo);
            break;
        case STTIO:
            return  (ICamIOPipe*)STTIOPipe::Create(szUsrName,InOutPut);
            break;
        case CAMSVIO:
            return (ICamIOPipe*)CamsvIOPipe::Create(szUsrName,InOutPut);
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

    this->m_FSM = op_unknown;
    this->m_bStartUniStreaming = MFALSE;
    this->m_pCmdQMgr = NULL;
    this->m_pCmdQMgr_AE = NULL;
    this->m_pHighSpeedCmdQMgr = NULL;
    this->m_pipe_opt = ICamIOPipe::CAMIO;
    this->m_DynamicRawType = MFALSE;
    this->m_bUpdate = MFALSE;
    this->m_hwModule = PHY_CAM;
    this->m_TwinMgr = NULL;
    this->m_cam_dfs = NULL;
    this->m_cam_dbs = NULL;
    this->m_PathCtrl = NULL;
    this->m_pResMgr = NULL;
    m_pCfgFrame = NULL;
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
        case op_suspend:
        case op_resume:
            if(this->m_FSM != op_start)
                ret = MFALSE;
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

E_INPUT CamIOPipe::TG_MAP(ISP_HW_MODULE _hwModule)
{
    E_INPUT TG_name;

    switch(_hwModule) {
    case CAM_A:
        TG_name = TG_A;
        break;
    case CAM_B:
        TG_name = TG_B;
        break;
    default:
        TG_name = TG_CAM_MAX;
        break;
    }

    return TG_name;
}

//return HW register format
MINT32 CamIOPipe::getOutPxlByteNFmt( MUINT32 portInx, ImgInfo::EImgFmt_t imgFmt, MINT32* pPixel_byte, MINT32* pFmt)
{
    //
    if ( NULL == pPixel_byte ) {
        PIPE_ERR("ERROR:NULL pPixel_byte");
        return -1;
    }
    //
    if (EPortIndex_RRZO == portInx){
        switch(imgFmt){
            case eImgFmt_FG_BAYER8:
            case eImgFmt_FG_BAYER10:
            case eImgFmt_FG_BAYER12:
            case eImgFmt_UFO_FG_BAYER8:
            case eImgFmt_UFO_FG_BAYER10:
            case eImgFmt_UFO_FG_BAYER12:
            case eImgFmt_UFO_BAYER10:
            case eImgFmt_UFO_BAYER12:
                break;
            default:
                PIPE_ERR("RRZO support only: eImgFmt_FG_BAYER8/10/12 UFEO_BAYER10/12 UFO_FG_BAYER8/10/12 UFO_BAYER10/12\n");
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
            if(EPortIndex_RRZO == portInx){
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
        case eImgFmt_UFO_BAYER8:
            *pPixel_byte = (1 << CAM_ISP_PIXEL_BYTE_FP);      //
            switch(portInx){
                case EPortIndex_IMGO:
                    *pFmt = IMGO_FMT_RAW8;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] portInx:[%d] NOT Support",imgFmt, portInx);
                    break;
            }
            break;
        case eImgFmt_UFO_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            switch(portInx){
                case EPortIndex_IMGO:
                    *pFmt = IMGO_FMT_RAW10;
                    break;
                case EPortIndex_RRZO:
                    *pFmt = RRZO_FMT_RAW10;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] portInx:[%d] NOT Support",imgFmt, portInx);
                    break;
            }
            break;
        case eImgFmt_UFO_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            switch(portInx){
                case EPortIndex_IMGO:
                    *pFmt = IMGO_FMT_RAW12;
                    break;
                case EPortIndex_RRZO:
                    *pFmt = RRZO_FMT_RAW12;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] portInx:[%d] NOT Support",imgFmt, portInx);
                    break;
            }
            break;
        case eImgFmt_UFO_BAYER14:
            *pPixel_byte = (7 << CAM_ISP_PIXEL_BYTE_FP) >> 2;
            switch(portInx){
                case EPortIndex_IMGO:
                    *pFmt = IMGO_FMT_RAW14;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] portInx:[%d] NOT Support",imgFmt, portInx);
                    break;
            }
            break;
        case eImgFmt_UFO_FG_BAYER8:
            *pPixel_byte = (1 << CAM_ISP_PIXEL_BYTE_FP);
            switch(portInx){
                case EPortIndex_RRZO:
                    *pFmt = RRZO_FMT_RAW8;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] portInx:[%d] NOT Support",imgFmt, portInx);
                    break;
            }
            break;
        case eImgFmt_UFO_FG_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            switch(portInx){
                case EPortIndex_RRZO:
                    *pFmt = RRZO_FMT_RAW10;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] portInx:[%d] NOT Support",imgFmt, portInx);
                    break;
            }
            break;
        case eImgFmt_UFO_FG_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            switch(portInx){
                case EPortIndex_RRZO:
                    *pFmt = RRZO_FMT_RAW12;
                    break;
                default:
                    PIPE_ERR("eImgFmt:[%d] portInx:[%d] NOT Support",imgFmt, portInx);
                    break;
            }
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
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = RRZO_FMT_RAW10;
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


MINT32 CamIOPipe::HwValToSrcFmt_TG( MUINT32 portid,MUINT32 imgFmt, MBOOL bUFDataFmt)
{
    MUINT32 bit = 0;
    switch(portid){
        case EPortIndex_IMGO:
            if(MFALSE == bUFDataFmt){
                switch(imgFmt){
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
                        PIPE_ERR("portID:0x%x bUFEnable:[%d] eImgFmt:[%d] NOT Support",portid, bUFDataFmt, imgFmt);
                        return -1;
                        break;
                }
            }else{// MTURE == bUFDataFmt
                switch(imgFmt){
                    case IMGO_FMT_RAW8: return eImgFmt_UFO_BAYER8;
                        break;
                    case IMGO_FMT_RAW10_MIPI:return eImgFmt_UFO_BAYER10;
                        break;
                    case IMGO_FMT_RAW10:return eImgFmt_UFO_BAYER10;
                        break;
                    case IMGO_FMT_RAW12:return eImgFmt_UFO_BAYER12;
                        break;
                    case IMGO_FMT_RAW14:return eImgFmt_UFO_BAYER14;
                        break;
                    case IMGO_FMT_RAW8_2B:return eImgFmt_UFO_BAYER8;
                        break;
                    case IMGO_FMT_RAW10_2B:return eImgFmt_UFO_BAYER10;
                        break;
                    case IMGO_FMT_RAW12_2B:return eImgFmt_UFO_BAYER12;
                        break;
                    case IMGO_FMT_RAW14_2B:return eImgFmt_UFO_BAYER14;
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
                        PIPE_ERR("portID:0x%x bUFEnable:[%d] eImgFmt:[%d] NOT Support",portid, bUFDataFmt, imgFmt);
                        return -1;
                        break;
                }
            }
            break;
        case EPortIndex_RRZO:
            if(MFALSE == bUFDataFmt){
                switch(imgFmt){
                    case RRZO_FMT_RAW8:return eImgFmt_FG_BAYER8;
                        break;
                    case RRZO_FMT_RAW10:return eImgFmt_FG_BAYER10;
                        break;
                    case RRZO_FMT_RAW12:return eImgFmt_FG_BAYER12;
                        break;
                    case RRZO_FMT_RAW14:return eImgFmt_FG_BAYER14;
                        break;
                    default:
                        PIPE_ERR("portID:0x%x bUFEnable:[%d] eImgFmt:[%d] bit:[%d] NOT Support",portid, bUFDataFmt, imgFmt, bit);
                        return -1;
                        break;
                }
            }else{ // MTURE == bUFDataFmt
                switch(imgFmt){
                    case RRZO_FMT_RAW8:return eImgFmt_UFO_FG_BAYER8;
                        break;
                    case RRZO_FMT_RAW10:return eImgFmt_UFO_FG_BAYER10;
                        break;
                    case RRZO_FMT_RAW12:return eImgFmt_UFO_FG_BAYER12;
                        break;
                    case RRZO_FMT_RAW14:return eImgFmt_UFO_FG_BAYER14;
                        break;
                    default:
                        PIPE_ERR("portID:0x%x bUFEnable:[%d] eImgFmt:[%d] bit:[%d] NOT Support",portid, bUFDataFmt, imgFmt, bit);
                        return -1;
                        break;
               }
            }
            break;
        default:
            PIPE_DBG("un-supported portID:0x%x\n",portid);
            break;
    }

    PIPE_DBG("portID:0x%x,HWImgFmt:[0x%x],bUFEnable:[%d] bit:[%d]",portid,  imgFmt, bUFDataFmt, bit);
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



MINT32 CamIOPipe::HwValToSrcFmt_DMA( MUINT32 portid,MUINT32 imgFmt, MBOOL bUFDataFmt)
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
            if(MFALSE == bUFDataFmt){
                switch (imgFmt) {
                    case RRZO_FMT_RAW8:return eImgFmt_FG_BAYER8;
                        break;
                    case RRZO_FMT_RAW10:return eImgFmt_FG_BAYER10;
                        break;
                    case RRZO_FMT_RAW12:return eImgFmt_FG_BAYER12;
                        break;
                    case RRZO_FMT_RAW14:return eImgFmt_FG_BAYER14;
                        break;
                    default:
                        PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                        return -1;
                        break;
                }
            }
            else{
                switch (imgFmt) {
                   case RRZO_FMT_RAW8:return eImgFmt_UFO_FG_BAYER8;
                        break;
                    case RRZO_FMT_RAW10:return eImgFmt_UFO_FG_BAYER10;
                        break;
                    case RRZO_FMT_RAW12:return eImgFmt_UFO_FG_BAYER12;
                        break;
                    case RRZO_FMT_RAW14:return eImgFmt_UFO_FG_BAYER14;
                        break;
                    default:
                        PIPE_ERR("eImgFmt:[%d] NOT Support",imgFmt);
                        return -1;
                        break;
                }
            }
            break;
        default:
            PIPE_DBG("un-supported portID:0x%x\n",portid);
            break;
    }

    PIPE_DBG("portID:0x%x,HWImgFmt:[0x%x],bUFEnable:[%d] bit:[%d]",portid,  imgFmt, bUFDataFmt, bit);
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


MINT32 CamIOPipe::PxlMode(E_CamPixelMode pxlMode,ImgInfo::EImgFmt_t fmt, MINT32* bTwin, MINT32* two_pxl, MINT32* en_p1, MINT32* two_pxl_dmx)
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

/*******************************************************************************
*
********************************************************************************/

CamIOPipe* CamIOPipe::Create(MINT8 const szUsrName[32], E_INPUT& OutPut, PortInfo const *portInfo)
{
    CamIOPipe *pCamIOPipeRet = new(std::nothrow) CamIOPipe();
    if(NULL == pCamIOPipeRet){
        delete pCamIOPipeRet;
        return NULL;
    }

    MUINT32 AdjustmentCrop1w = 0;
    if(portInfo->tgFps >= 450){
        AdjustmentCrop1w = portInfo->tgFps * portInfo->crop1.w;
    }else{
        AdjustmentCrop1w = portInfo->crop1.w;
    }

    pCamIOPipeRet->m_pResMgr = ResMgr::Res_Attach(user_camio, (char *)szUsrName, AdjustmentCrop1w);
    if(NULL != pCamIOPipeRet->m_pResMgr){
        CamIO_PathCtrl *lCamPathCtrl = new(std::nothrow) CamIO_PathCtrl(pCamIOPipeRet);
        if( NULL == lCamPathCtrl){
            delete lCamPathCtrl;
            delete pCamIOPipeRet;
            pCamIOPipeRet = NULL;
            return NULL;
        }
        pCamIOPipeRet->m_PathCtrl = lCamPathCtrl;
        switch(pCamIOPipeRet->m_pResMgr->Res_GetModule())
        {
            case(CAM_A):
            {
                pCamIOPipeRet->m_hwModule                   = CAM_A;
                pCamIOPipeRet->m_PathCtrl->m_hwModule       = CAM_A;
                pCamIOPipeRet->m_CamIO_stt.m_hwModule       = CAM_A;
                OutPut = TG_MAP(CAM_A);
                break;
            }
            case(CAM_B):
            {
                pCamIOPipeRet->m_hwModule                   = CAM_B;
                pCamIOPipeRet->m_PathCtrl->m_hwModule       = CAM_B;
                pCamIOPipeRet->m_CamIO_stt.m_hwModule       = CAM_B;
                OutPut = TG_MAP(CAM_B);
                break;
            }
            default:
               delete lCamPathCtrl;
               lCamPathCtrl = NULL;
               delete pCamIOPipeRet;
               pCamIOPipeRet = NULL;
       }
    }else{ // NULL == pCamIOPipeRet->m_pResMgr
        delete pCamIOPipeRet;
        pCamIOPipeRet = NULL;
    }
    return pCamIOPipeRet;
}

void CamIOPipe::Destroy(void)
{
    if(this->FSM_CHECK(op_unknown) == MFALSE)
        PIPE_ERR("FSM check error, destroy may be abnormal\n");

     // user can do suspend without resume,in order to avoid false alarm error, check this->m_pResMgr
    if(NULL != this->m_pResMgr){
        PIPE_INF(":E:user:%d, %d\n",this->m_pResMgr->Res_GetModule(), this->m_hwModule);
        if(this->m_pResMgr->Res_GetModule() == this->m_hwModule){
            if(MFALSE == this->m_pResMgr->Res_Detach()){
                PIPE_ERR("Res_Detach fail.\n");
            }else{
                this->m_pResMgr = NULL;
            }
        }else{
            PIPE_ERR("unknown error in uninit Res_GetModule ?\n");
        }
    }else{
        PIPE_DBG("have suspend without resume ??\n");//already release resmgr.
    }

    this->m_camPass1Param.bypass_tg = MFALSE;
    this->m_camPass1Param.bypass_imgo = MTRUE;
    this->m_camPass1Param.bypass_rrzo = MTRUE;
    this->m_camPass1Param.bypass_ispRawPipe = MFALSE;
    this->m_camPass1Param.bypass_ispYuvPipe = MTRUE;

    this->m_FSMLock.lock();
    this->m_FSM = op_unknown;
    this->m_FSMLock.unlock();

    this->m_TwinMgr = 0;

    this->mRunTimeCfgLock.lock();
    this->m_bStartUniStreaming = MFALSE;
    this->mRunTimeCfgLock.unlock();

    this->m_pCmdQMgr = NULL;
    this->m_pCmdQMgr_AE = NULL;
    this->m_pHighSpeedCmdQMgr = NULL;
    this->m_DynamicRawType =  MFALSE;
    this->m_bUpdate = MFALSE;

    this->m_cam_dfs->destroyInstance(LOG_TAG);
    delete this->m_PathCtrl; // delete heap momory form camiopipe new() in create().
    this->m_PathCtrl = NULL;

    this->m_pResMgr = NULL;
    delete this;             // delete heap momory form camiopipe new() in create().

}

/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::init()
{
    PIPE_INF("(%d):E\n",this->m_pResMgr->Res_GetModule());

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
    if(NULL != this->m_pResMgr){
        PIPE_INF("(%d):E\n",this->m_pResMgr->Res_GetModule());
    }

    if(NULL != this->m_pCfgFrame){
        delete this->m_pCfgFrame;
        this->m_pCfgFrame = NULL;
    }

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
    else{
        if (this->m_pCmdQMgr_AE) {
            if (this->m_pCmdQMgr_AE->CmdQMgr_detach()) {
                PIPE_ERR("CQ_AE uninit fail\n");
                return MFALSE;
            }
        }
    }

    if (this->m_TwinMgr) {
        if(this->m_TwinMgr->uninit()) {
            PIPE_ERR("isp pathMgr uninit fail\n");
            return MFALSE;
        }
        this->m_TwinMgr->destroyInstance();
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
        if(0 != this->m_TwinMgr->setP1Notify()){
            PIPE_ERR("some notified-module fail\n");
        }
    }

    if(this->m_TwinMgr->start(ptr)){
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
MBOOL CamIOPipe::stop(MBOOL bForce)
{
    PIPE_DBG(":E\n");

    if(this->FSM_CHECK(op_stop) == MFALSE)
        return MFALSE;


    //
    //mutex protection is for isp stop & uni_stop which is driven by sendcommand
    this->mRunTimeCfgLock.lock();
    PIPE_DBG("stop pass1 +\n");


    if(this->m_TwinMgr->stop(&bForce)){
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

    //detach cmdq11
    if(this->m_pHighSpeedCmdQMgr){
        if(this->m_pHighSpeedCmdQMgr->CmdQMgr_stop()){
            PIPE_ERR("CQ11 stop fail\n");
            return MFALSE;
        }
    }
    else{
        if(this->m_pCmdQMgr_AE->CmdQMgr_stop()){
            PIPE_ERR("CQ_AE stop fail\n");
            return MFALSE;
        }
    }

    if(this->m_camPass1Param.bypass_rrzo == MFALSE){
        RlbMgr  *pRlb = RlbMgr::getInstance(this->m_hwModule);

        if (pRlb->release(this->m_hwModule) != E_RlbRet_OK) {
            PIPE_ERR("[0x%x]release RRZ line buffer failed\n", this->m_hwModule);
        }
    }


    this->FSM_UPDATE(op_stop);
    PIPE_DBG("-\n");
    return  MTRUE;
}


MBOOL CamIOPipe::suspend(E_SUSPEND_MODE suspendMode)
{
    MBOOL ret = MTRUE;
    CAM_REG_CTL_TWIN_STATUS twin;

    if(this->FSM_CHECK(op_suspend) == MFALSE)
        return MFALSE;

    switch(suspendMode){
        case HW_RST_SUSPEND:
            ret = this->m_TwinMgr->HW_recover(eCmd_path_stop);
            break;
        case SW_SUSPEND:
            ret = this->m_TwinMgr->suspend();
            break;
        default:
            PIPE_ERR("unsupported:%d\n",suspendMode);
            break;
    }

    if(ret == MTRUE){
        if(MFALSE == this->m_pResMgr->Res_Detach()){
            PIPE_ERR("logic error\n");
            ret = MFALSE;
        }else{
            this->m_pResMgr = NULL;
        }
    }
    return ret;
}

MBOOL CamIOPipe::preresume(MINT8 const szUsrName[32], MUINT32 cropw )
{
    MBOOL ret = MTRUE;
    if(this->FSM_CHECK(op_resume) == MFALSE){
        return MFALSE;
    }

    MUINT32 AdjustmentCrop1w = 0;
    if(this->m_PathCtrl->m_cfgInParam.tg_fps >= 450){
        AdjustmentCrop1w = this->m_PathCtrl->m_cfgInParam.tg_fps * cropw;
    }else{
        AdjustmentCrop1w = cropw;
    }

    this->m_pResMgr = ResMgr::Res_Attach(user_camio, (char *)szUsrName, AdjustmentCrop1w);

    if(NULL == this->m_pResMgr){
        PIPE_ERR("preresume Res_Attach fail.\n");
        return MFALSE;
    }

    if(this->m_pResMgr->Res_GetModule() != (this->m_hwModule)){
        PIPE_ERR("logic error,no available module for preresume\n");
        ret = MFALSE;
    }

    return ret;
}

MBOOL CamIOPipe::resume(E_SUSPEND_MODE suspendMode)
{
    MBOOL ret = MTRUE;
    CAM_REG_CTL_TWIN_STATUS twin;

    if(this->FSM_CHECK(op_resume) == MFALSE)
        return MFALSE;


    switch(suspendMode){
        case HW_RST_SUSPEND:
            ret = this->m_TwinMgr->HW_recover(eCmd_path_restart);
            break;
        case SW_SUSPEND:
            ret = this->m_TwinMgr->resume();
            break;
        default:
            PIPE_ERR("unsupported:%d\n",suspendMode);
            ret = MFALSE;
            break;
    }

    if(ret != MTRUE){
        if(MFALSE == this->m_pResMgr->Res_Detach()){
            PIPE_ERR("logic error\n");
            ret = MFALSE;
        }else{
            this->m_pResMgr = NULL;
        }
    }
EXIT:
    return ret;
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
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %zd)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

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
    ST_BUF_INFO     imgaeAndHeaderInfo;
    MUINT32         PlaneMax = 0;
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
        PIPE_ERR("enque buf number is mismatched with subsample (0x%zx_0x%x)\n",rQBufInfo.vBufInfo.size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
        return -1;
    }
    for(MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++){
        stISP_BUF_INFO  bufInfo;

        //repalce
        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            bufInfo.Replace.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].replace.u4BufPA[ePlane_1st];
            bufInfo.Replace.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].replace.u4BufVA[ePlane_1st];
            bufInfo.Replace.image.mem_info.size      = rQBufInfo.vBufInfo[i].replace.u4BufSize[ePlane_1st];
            bufInfo.Replace.image.mem_info.memID     = rQBufInfo.vBufInfo[i].replace.memID[ePlane_1st];
            bufInfo.Replace.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].replace.bufSecu[ePlane_1st];
            bufInfo.Replace.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].replace.bufCohe[ePlane_1st];
            bufInfo.Replace.image.mem_info.magicIdx  = rQBufInfo.vBufInfo[i].m_num;

            //header
            bufInfo.Replace.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st];
            bufInfo.Replace.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st];
            bufInfo.Replace.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st];
            bufInfo.Replace.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID[ePlane_1st];
            bufInfo.Replace.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu[ePlane_1st];
            bufInfo.Replace.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe[ePlane_1st];

            bufInfo.bReplace = MTRUE;
        }

        switch (rQBufInfo.vBufInfo[i].img_fmt)
        {
           case (EImageFormat)eImgFmt_UFO_BAYER8:
           case (EImageFormat)eImgFmt_UFO_BAYER10:
           case (EImageFormat)eImgFmt_UFO_BAYER12:
           case (EImageFormat)eImgFmt_UFO_BAYER14:
           case (EImageFormat)eImgFmt_UFO_FG_BAYER8:
           case (EImageFormat)eImgFmt_UFO_FG_BAYER10:
           case (EImageFormat)eImgFmt_UFO_FG_BAYER12:
           case (EImageFormat)eImgFmt_UFO_FG_BAYER14:
               PlaneMax = (MUINT32)ePlane_3rd;
               break;
           default:
               PlaneMax = (MUINT32)ePlane_1st;
               break;
       }

        for(MUINT32 loopInx = 0; loopInx <= PlaneMax; loopInx++){
            if(ePlane_3rd == loopInx){
                /*only 3nd plane*/
                imgaeAndHeaderInfo.image.mem_info.va_addr   = (MUINTPTR)rQBufInfo.vBufInfo[i].m_pPrivate;
            }else{// 1st plane , 2nd plane
                imgaeAndHeaderInfo.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].u4BufPA[loopInx];
                imgaeAndHeaderInfo.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].u4BufVA[loopInx];
                imgaeAndHeaderInfo.image.mem_info.size      = rQBufInfo.vBufInfo[i].u4BufSize[loopInx];
                imgaeAndHeaderInfo.image.mem_info.memID     = rQBufInfo.vBufInfo[i].memID[loopInx];
                imgaeAndHeaderInfo.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].bufSecu[loopInx];
                imgaeAndHeaderInfo.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].bufCohe[loopInx];
                imgaeAndHeaderInfo.image.mem_info.magicIdx  = rQBufInfo.vBufInfo[i].m_num;
            }

            imgaeAndHeaderInfo.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[loopInx];
            imgaeAndHeaderInfo.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[loopInx];
            imgaeAndHeaderInfo.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[loopInx];
            imgaeAndHeaderInfo.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID[loopInx];
            imgaeAndHeaderInfo.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu[loopInx];
            imgaeAndHeaderInfo.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe[loopInx];



            bufInfo.u_op.enque.push_back(imgaeAndHeaderInfo);
            if(!bufInfo.u_op.enque.empty()){
                if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
                    PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(PlaneInx:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx). exbufInfo,(MEMID:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx)\n",\
                    i,\
                    portID.index, bufInfo.u_op.enque.at(loopInx).image.mem_info.memID,\
                    loopInx,\
                    bufInfo.u_op.enque.at(loopInx).image.mem_info.va_addr,\
                    bufInfo.u_op.enque.at(loopInx).image.mem_info.pa_addr,\
                    bufInfo.u_op.enque.at(loopInx).header.va_addr,\
                    bufInfo.Replace.image.mem_info.memID,\
                    bufInfo.Replace.image.mem_info.va_addr,\
                    bufInfo.Replace.image.mem_info.pa_addr,\
                    bufInfo.Replace.header.va_addr);
                }else{
                    PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%zd),(PlaneInx:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx)\n",\
                    i,\
                    portID.index, bufInfo.u_op.enque.at(loopInx).image.mem_info.memID,\
                    loopInx,\
                    bufInfo.u_op.enque.at(loopInx).image.mem_info.va_addr,\
                    bufInfo.u_op.enque.at(loopInx).image.mem_info.pa_addr,\
                    bufInfo.u_op.enque.at(loopInx).header.va_addr);
                }
            }else{
                PIPE_ERR("CamIOPipe bufInfo.u_op.enque empty\n");
                return -1;
            }
        }
        //
        buf_L.push_back(bufInfo);
    }

    if (this->m_TwinMgr->enqueueBuf(dmaChannel, buf_L) != 0) {
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

E_BUF_STATUS CamIOPipe::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs, CAM_STATE_NOTIFY *pNotify)
{
    E_BUF_STATUS ret = eBuf_Pass;
    MUINT32 dmaChannel = 0;
    //warning free
    u4TimeoutMs;
    //
    if(this->FSM_CHECK(op_endeq) == MFALSE) {
        if(this->m_FSM == op_stop)
            return eBuf_stopped;
        return eBuf_Fail;
    }
    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        //EIS/RSS/LCS
        return this->m_CamIO_stt.dequeOutBuf(portID,rQBufInfo,this->m_pCmdQMgr->CmdQMgr_GetBurstQ(), pNotify);
    }

    //clear remained data in container
    rQBufInfo.vBufInfo.clear();


    switch(this->m_TwinMgr->dequeueBuf(dmaChannel, rQBufInfo.vBufInfo, pNotify)) {
    case -1:
        PIPE_ERR("ERROR:dma[%d] dequeOutBuf fail\n", dmaChannel);
        return eBuf_Fail;
        break;
    case 1:
        PIPE_DBG("deq dma[%d] FSM=stop(%d)", dmaChannel, this->m_FSM);
        ret = eBuf_stopped;
        break;
    case 2:
        PIPE_DBG("deq dma[%d] FSM=suspending(%d)", dmaChannel, this->m_FSM);
        ret = eBuf_suspending;
        break;
    default:
        if(rQBufInfo.vBufInfo.size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) {
            PIPE_ERR("dequeout data length is mismatch(%d_%d)\n",(MUINT32)rQBufInfo.vBufInfo.size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
            ret = eBuf_Fail;
        }
        break;
    }

    for ( MUINT32 i = 0; i < rQBufInfo.vBufInfo.size() ; i++) {

        //
        if(this->m_camPass1Param.bypass_tg == MTRUE){
            rQBufInfo.vBufInfo[i].img_fmt =
                (MUINT32)this->HwValToSrcFmt_DMA(portID.index, rQBufInfo.vBufInfo[i].img_fmt, rQBufInfo.vBufInfo[i].bUF_DataFmt);
        }else{
            rQBufInfo.vBufInfo[i].img_fmt =
                (MUINT32)this->HwValToSrcFmt_TG(portID.index, rQBufInfo.vBufInfo[i].img_fmt, rQBufInfo.vBufInfo[i].bUF_DataFmt);
        }
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
        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx),(img_fmt:0x%x)\n",\
            i,\
            portID.index, rQBufInfo.vBufInfo[i].memID[ePlane_1st],\
            rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st],\
            rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st],\
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st],\
            rQBufInfo.vBufInfo[i].img_fmt);
    }

    //
    //PIPE_DBG("X ");
    this->FSM_UPDATE(op_cmd);
    return  ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, CAMIO_Func const *pInFunc, CAMIO_Func *pOutFunc)
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

    //
    if(this->FSM_CHECK(op_cfg) == MFALSE)
        return MFALSE;

    if ( (0 == vOutPorts.size()) || (vInPorts.size() != 1)) {
        PIPE_ERR("inport:0x%zx || outport:0x%zx size err\n", vInPorts.size(), vOutPorts.size());
        return MFALSE;
    }

    //
    //ON/OFF dynamic twin , only supported in bianco/vinson
    if(pInFunc->Bits.DynamicTwin){
        capibility CamInfo;
        CamInfo.m_DTwin.SetDTwin(MTRUE);
    }
    else{
        capibility CamInfo;
        CamInfo.m_DTwin.SetDTwin(MFALSE);
    }

    //input source
    //inport support only 1 at current driver design
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) {
            PIPE_INF("dummy input vector at:0x%x\n",i);
            continue;
        }

        PIPE_INF("P1 vInPorts_%d:fmt(0x%x),PM(%d),w/h(%d_%d),crop(%d_%d_%d_%d),tg_idx(%d),dir(%d),fps(%d),timeclk(0x%x),MIPI_PixRate(0x%x),subsample(%d),bin_off(%d),dat_pat(%d),sen_num(%d)\n",\
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
                vInPorts[i]->MIPI_PixRate,\
                pInFunc->Bits.SUBSAMPLE,\
                pInFunc->Bits.bin_off,\
                pInFunc->Bits.DATA_PATTERN,\
                pInFunc->Bits.SensorNum);

        //
        switch ( vInPorts[i]->index ) {
            case TG_A:
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
            case TG_B:
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
            case RAWI:

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
                 vOutPorts[i]->u4Stride[ePlane_1st], \
                 vOutPorts[i]->index, vOutPorts[i]->inout);

        //
        switch(vOutPorts[i]->index){
            case EPortIndex_IMGO:
                idx_imgo = i;

                if (this->getOutPxlByteNFmt(EPortIndex_IMGO, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo, (MINT32*)&imgo_fmt ) == 0) {

                    if(imgo_fmt == IMGO_FMT_YUV422_1P){//YUV format
                        //ONLY YUV sensor support always fix in pureraw path under 1/2/4 pix mode
                        this->m_DynamicRawType = MFALSE;
                        bPureRaw = MTRUE;
                    }
                    else{   //bayer format
                        en_p1     |= PAK_EN_;
                        if(1 == vOutPorts[i]->u4PureRaw){
                            bPureRaw = MTRUE;
                            if(1 == vOutPorts[i]->u4PureRawPak) {
                                bPak = MTRUE;
                            }else{
#if 0
                                bPak = MFALSE;
                                //disable pak
                                en_p1 &= (~PAK_EN_);
                                if( (1 << CAM_ISP_PIXEL_BYTE_FP) !=  pixel_byte_imgo) {
                                    pixel_byte_imgo= (2 << CAM_ISP_PIXEL_BYTE_FP);
                                }
#else
                                PIPE_ERR("unpak only support use unpak fmt from imagefromat.h");
#endif
                            }
                        }
                        this->m_DynamicRawType = MTRUE;
                    }

                    this->m_camPass1Param.bypass_imgo = MFALSE;
                }
                break;
            case EPortIndex_RRZO:
                idx_rrzo = i;

                en_p1 |= (RRZ_EN_|PAKG_EN_);
                if (this->getOutPxlByteNFmt(EPortIndex_RRZO, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_rrzo, (MINT32*)&rrzo_fmt) == 0) {
                    if(1 == vOutPorts[i]->u4PureRaw){
                        PIPE_ERR("this dmao support no pure output,always processed raw\n");
                    }

                    this->m_camPass1Param.bypass_rrzo = MFALSE;
                }
                break;
        }
    }

    //cfg cmdQ
    if((this->m_pCmdQMgr = DupCmdQMgr::CmdQMgr_attach(vInPorts[idx_in_src]->tgFps,pInFunc->Bits.SUBSAMPLE ,this->m_hwModule,ISP_DRV_CQ_THRE0)) == NULL){
        PIPE_ERR("CQ init fail\n");
        return MFALSE;
    }

    //cfg cmdq11
    if(pInFunc->Bits.SUBSAMPLE){
        if( (this->m_pHighSpeedCmdQMgr = DupCmdQMgr::CmdQMgr_attach(vInPorts[idx_in_src]->tgFps,\
            pInFunc->Bits.SUBSAMPLE,this->m_hwModule,ISP_DRV_CQ_THRE11)) == NULL){
            PIPE_ERR("CQ 11 init fail\n");
            this->m_pHighSpeedCmdQMgr = NULL;
        }
    }
    else{   //cfg cq1 for ae smoothing
        if( (this->m_pCmdQMgr_AE = DupCmdQMgr::CmdQMgr_attach(vInPorts[idx_in_src]->tgFps,\
            pInFunc->Bits.SUBSAMPLE,this->m_hwModule,ISP_DRV_CQ_THRE1)) == NULL){
            PIPE_ERR("CQ 1 init fail\n");
            this->m_pCmdQMgr_AE = NULL;
        }
    }

    //
    /*-----------------------------------------------------------------------------
      m_camPass1Param
      -----------------------------------------------------------------------------*/
    //path
    if(this->m_camPass1Param.bypass_tg == MTRUE)
        this->m_camPass1Param.m_Path = CamPathPass1Parameter::_DRAM_IN;
    else
        this->m_camPass1Param.m_Path = CamPathPass1Parameter::_TG_IN;

    //subsample
    this->m_camPass1Param.m_subSample = pInFunc->Bits.SUBSAMPLE;

    //fps
    if(vInPorts[idx_in_src]->tgFps ==0){
        PIPE_ERR("fps can't be 0\n");
        return MFALSE;
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
        this->m_camPass1Param.m_src_pixmode = vInPorts[idx_in_src]->ePxlMode;
    }


    //fmt sel
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Raw             = 0x00;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_ID     = pix_id_tg;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.TG_FMT     = InSrcFmt;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.RRZO_FMT   = rrzo_fmt;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.IMGO_FMT   = imgo_fmt;
    this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.RRZO_FG_MODE   = 0x1;  //always FG format
    this->m_camPass1Param.m_data_pat = (E_CamPattern)pInFunc->Bits.DATA_PATTERN;

    /// Notice!!! Must call TwinMgr before this->pathSelCtrl(bPureRaw,bPak,this->m_camPass1Param.bypass_tg);
    ///           Cause when frontal binning is enable. IMGO will be set to PURE RAW
    ///
    /// Get path config info from twin_mgr
    ///
    this->m_TwinMgr = TwinMgr::createInstance(this->m_hwModule);
    if (this->m_TwinMgr == NULL) {
        PIPE_ERR("TwinMgr createInstance fail!!!\n");
        return MFALSE;
    }
    if (this->m_TwinMgr->init(this->m_pCmdQMgr)) {
        PIPE_ERR("TwinMgr initPath() error!!!!!!\n");
        return MFALSE;
    }

    this->m_CamIO_stt.pTwinMgr = this->m_TwinMgr;

    if(this->m_camPass1Param.bypass_tg == MFALSE)
    {
        MUINT32 availNumForTwin = 0;
        if((this->m_cam_dfs = CAM_DFS_Mgr::createInstance(LOG_TAG, this->m_hwModule)) == 0)
            PIPE_ERR("cam_dfs createInstance fail");

        // Prepare input param to TwinMgr once if twin is needed after stream-on.
        this->m_PathCtrl->m_cfgInParam.SrcImgFmt = (E_CAM_CTL_TG_FMT)InSrcFmt;
        this->m_PathCtrl->m_cfgInParam.bypass_tg = this->m_camPass1Param.bypass_tg;
        this->m_PathCtrl->m_cfgInParam.pix_mode_tg = this->m_camPass1Param.m_src_pixmode;
        this->m_PathCtrl->m_cfgInParam.tg_crop_w = this->m_camPass1Param.m_src_size.w;
        this->m_PathCtrl->m_cfgInParam.tg_crop_h = this->m_camPass1Param.m_src_size.h;
        this->m_PathCtrl->m_cfgInParam.tg_fps = vInPorts[idx_in_src]->tgFps ;
        if (vInPorts[idx_in_src]->MIPI_PixRate) {
           this->m_PathCtrl->m_cfgInParam.MIPI_pixrate = vInPorts[idx_in_src]->MIPI_PixRate;
        }
        if(this->m_PathCtrl->m_cfgInParam.tg_fps < 1){
            this->m_PathCtrl->m_cfgInParam.tg_fps++;
            PIPE_WRN("org fps from sensor:%d < 0.11fps\n",vInPorts[idx_in_src]->tgFps);
        }
        this->m_PathCtrl->m_cfgInParam.bypass_rrzo = this->m_camPass1Param.bypass_rrzo;
        this->m_PathCtrl->m_cfgInParam.rrz_out_w   = (this->m_camPass1Param.bypass_rrzo == MFALSE) ? \
                                    vOutPorts[idx_rrzo]->u4ImgWidth : 0;
        //
        this->m_PathCtrl->m_cfgInParam.offBin  = pInFunc->Bits.bin_off;
        this->m_PathCtrl->m_cfgInParam.pattern = (E_CamPattern)pInFunc->Bits.DATA_PATTERN;
        this->m_PathCtrl->m_cfgInParam.SenNum = (E_CAMIO_SEN)pInFunc->Bits.SensorNum;

        switch(pInFunc->Bits.DATA_PATTERN){
            case eCAM_DUAL_PIX:
                this->m_PathCtrl->m_bDbn = MTRUE;
                break;
            case eCAM_4CELL:
                this->m_PathCtrl->m_bDbn = MTRUE;
                break;
            case eCAM_NORMAL:
            default:
                this->m_PathCtrl->m_bDbn = MFALSE;
                break;
        }
        //

        {
            NSImageio::NSIspio::PortID *CastPtr = dynamic_cast<NSImageio::NSIspio::PortID *> ((PortInfo *) vInPorts[idx_in_src]);

            if(NULL == CastPtr){
                PIPE_ERR("dynamic_cast fail\n");
                return MFALSE;
            }
            else{
                memcpy((void*)&this->m_PathCtrl->m_Src,(void*)CastPtr,sizeof(NSImageio::NSIspio::PortID));
            }
        }

        //
        if(pInFunc->Bits.bin_off == MTRUE)
            PIPE_INF("Frontal binning is forced to disable\n");

        //
        this->m_cam_dfs->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_PathCtrl->m_cfgInParam.vClk.clear();
        this->m_cam_dfs->CAM_DFS_INIT(&this->m_PathCtrl->m_cfgInParam.vClk);

        this->m_PathCtrl->m_resCfgOutParam.clk_level = this->m_cam_dfs->CAM_DFS_GetCurLv(); // Must add before Res_Meter

        if(MFALSE == this->m_pResMgr->Res_Meter(this->m_PathCtrl->m_cfgInParam, this->m_PathCtrl->m_resCfgOutParam)) {
            PIPE_ERR("Res_Meter() error!!!!!!\n");
            return MFALSE;
        }

        //
        this->m_PathCtrl->ResToTwinParamUpdate(this->m_PathCtrl->m_cfgInParam,
                                              this->m_PathCtrl->m_resCfgOutParam,
                                              this->m_PathCtrl->m_twinCfgInParam);


        if(MFALSE == this->m_TwinMgr->pathControl(this->m_PathCtrl->m_twinCfgInParam)){
            PIPE_ERR("TwinMgr pathControl() error!!!!!!\n");
        }

        //
        this->m_cam_dfs->CAM_DFS_SetLv(this->m_PathCtrl->m_resCfgOutParam.clk_level,MTRUE);


        // Store cfgOutParam to CamIOPipe
        // 1. pixel mode of mux
        //which is used at updatePath(), need be invoked after config()
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXI = this->m_PathCtrl->m_resCfgOutParam.xmxo.pix_mode_dmxi;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXO = this->m_PathCtrl->m_resCfgOutParam.xmxo.pix_mode_dmxo;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_BMXO = this->m_PathCtrl->m_resCfgOutParam.xmxo.pix_mode_bmxo;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_RMXO = this->m_PathCtrl->m_resCfgOutParam.xmxo.pix_mode_rmxo;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_AMXO = this->m_PathCtrl->m_resCfgOutParam.xmxo.pix_mode_amxo;

        // Store cfgOutParam to CamIOPipe
        // 1. pixel mode of mux
        //which is used at updatePath(), need be invoked after config()


        // 2. frontal binning
        if(this->m_PathCtrl->m_bDbn == MTRUE)
            en_p1 |= DBN_EN_;

        if(this->m_PathCtrl->m_resCfgOutParam.bin_en== MTRUE)
            en_p1 |= BIN_EN_;

        // When frontal binning is enable, set RAW type to be "PureRaw"!!!
        if((this->m_PathCtrl->m_resCfgOutParam.bin_en== MTRUE) || (this->m_camPass1Param.m_data_pat == eCAM_4CELL)) {
            this->m_camPass1Param.m_NonScale_RawType = CamPathPass1Parameter::_PURE_RAW;
            bPureRaw = MTRUE;
            this->m_DynamicRawType = MFALSE; // Cannot change raw type in dynamic
            PIPE_INF("Frontal binning is enable or QuadCode path !! bin(%d) path(%d), Set Raw type to PureRaw!!!\n",
                this->m_PathCtrl->m_resCfgOutParam.bin_en, this->m_camPass1Param.m_data_pat);
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
        return MFALSE;
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
    {
        Cam_path_sel* ptr = new Cam_path_sel(&this->m_camPass1Param);
        switch(pInFunc->Bits.DATA_PATTERN){
            case eCAM_DUAL_PIX:
                ptr->Path_sel(bPureRaw,bPak,this->m_camPass1Param.bypass_tg,eCAM_DUAL_PIX);
                break;
            case eCAM_4CELL:
                ptr->Path_sel(bPureRaw,bPak,this->m_camPass1Param.bypass_tg,eCAM_4CELL);
                break;
            case eCAM_NORMAL:
            default:
                ptr->Path_sel(bPureRaw,bPak,this->m_camPass1Param.bypass_tg,eCAM_NORMAL);
                break;
        }
        delete ptr;
    }

    //func_en , default function
    this->m_camPass1Param.m_top_ctl.FUNC_EN.Raw     = en_p1;

    //uni_rawi
    this->mRunTimeCfgLock.lock();

    if(this->m_camPass1Param.bypass_tg == MTRUE){
        IspDMACfg RawI;
        // use input dma crop
        this->configDmaPort(vInPorts[idx_in_src],RawI,(MUINT32)pixel_byte_imgi);
        RawI.format = InSrcFmt;//borrow this para to set format in uni_mgr
        //
        if(this->m_UniMgr.UniMgr_attach(UNI_A,this->m_pCmdQMgr,&RawI) != 0){
            PIPE_ERR("rawi cfg fail\n");
            this->mRunTimeCfgLock.unlock();
            return MFALSE;
        }
        //
        this->m_camPass1Param.m_RawIFmt = (E_UNI_TOP_FMT)InSrcFmt;

        //keep this record for debug or sendcommand determine size constraint.
        this->m_camPass1Param.m_src_size.x = 0;
        this->m_camPass1Param.m_src_size.y = 0;
        this->m_camPass1Param.m_src_size.w = vInPorts[idx_in_src]->u4ImgWidth;
        this->m_camPass1Param.m_src_size.h = vInPorts[idx_in_src]->u4ImgHeight;
        this->mRunTimeCfgLock.unlock();
    }
    else{
        if(this->m_UniMgr.UniMgr_attach(UNI_A,this->m_pCmdQMgr,NULL) == NULL){
            PIPE_ERR("UniMgr fail\n");
            this->mRunTimeCfgLock.unlock();
            return MFALSE;
        }
        else {
            this->m_bStartUniStreaming = MTRUE;
            this->mRunTimeCfgLock.unlock();
        }
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
        MUINT32 rlb_offset = 0;
        E_RlbRetStatus rlbRet;
        RlbMgr  *pRlb = RlbMgr::getInstance(this->m_hwModule);

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

        // acquire rrz sram from RlbMgr
        rlbRet = pRlb->acquire(this->m_hwModule, vOutPorts[idx_rrzo]->u4ImgWidth, &rlb_offset);
        if (rlbRet != E_RlbRet_OK) {
            PIPE_ERR("acquire RRZ line buffer failed size: %d\n", vOutPorts[idx_rrzo]->u4ImgWidth);
            return MFALSE;
        }
        PIPE_DBG("acquired offset:%d size:%d", rlb_offset, vOutPorts[idx_rrzo]->u4ImgWidth);

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
        this->m_camPass1Param.rrz_rlb_offset = rlb_offset;
    }

    //
    if (0 != this->m_TwinMgr->config(&this->m_camPass1Param))
    {
        PIPE_ERR("config error!\n");
        return MFALSE;
    }

    // If twin is enable:
    // 1. Occupied another CamIOPipe hwModule for TwinMgr
    // 2. Cfg cmdQ for another CamIOPipe hwModule
    if ((this->m_PathCtrl->m_resCfgOutParam.isTwin == MTRUE) || (pInFunc->Bits.DynamicTwin == 1)) {
        list<MUINT32> l_channel;

        l_channel.clear();

        if (this->m_camPass1Param.bypass_rrzo == MFALSE)
            l_channel.push_back(_rrzo_);
        if (this->m_camPass1Param.bypass_imgo == MFALSE)
            l_channel.push_back(_imgo_);

        if (this->m_TwinMgr->initTwin(this->m_pCmdQMgr, (const NSImageio::NSIspio::PortID*)vInPorts[idx_in_src], pInFunc->Bits.SUBSAMPLE, &l_channel) == MFALSE) {
            PIPE_ERR("twin init fail\n");
            return MFALSE;
        }
    }

    if (this->m_camPass1Param.bypass_ispRawPipe == MFALSE) {
        //update xmx setting
        if(this->m_TwinMgr->updatePath(UpdatePathParam(&this->m_PathCtrl->m_resCfgOutParam.xmxo, this->m_PathCtrl->m_resCfgOutParam.bin_en)) == MFALSE){
            PIPE_ERR("config error!\n");
            return MFALSE;
        }
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
        dmaChannel = this->m_CamIO_stt.QueryPort(port.index);
    }

    switch(dmaChannel){
        case _imgo_:
        case _rrzo_:
        case _lcso_:
        case _lmvo_:
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
    if(bCurPhy)
        return this->m_UniMgr.UniMgr_GetCurObj()->readReg(addr);
    else
        return this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(this->m_pCmdQMgr->CmdQMgr_GetDuqQIdx())[this->m_pCmdQMgr->CmdQMgr_GetBurstQIdx()]->readReg(addr,UNI_A);
}


/*******************************************************************************
*
********************************************************************************/
#define CAM_MAP(input,output) {\
    switch(input){\
        case CAM_A:\
            output = (E_CAMIO_CAM)((MUINT32)output|EPipe_CAM_A);\
            break;\
        case CAM_B:\
            output = (E_CAMIO_CAM)((MUINT32)output|EPipe_CAM_B);\
            break;\
        default:\
            PIPE_ERR("this cam(%d) is not supported\n",input);\
            break;\
    }\
}

MBOOL CamIOPipe::sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int    ret = 0; // 0 for ok , -1 for fail

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08zx,0x%08zx,0x%08zx)", gettid(), cmd, arg1, arg2, arg3);

    if(this->FSM_CHECK(op_cmd) == MFALSE)
        return MFALSE;

    switch ( cmd ) {
        case EPIPECmd_DEALLOC_UNI:
            PIPE_DBG("not a valid cmd at 50\n");
            break;
        case EPIPECmd_ALLOC_UNI:
            PIPE_DBG("not a valid cmd at 50\n");
            break;
        case EPIPECmd_SET_EIS_CBFP:
            {
                if(this->m_FSM != op_cfg){
                    //because of sw_p1 done grping
                    PIPE_ERR("EIS_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                    ret = 1;
                }
                switch(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT)){
                case TG_FMT_RAW8:
                case TG_FMT_RAW10:
                case TG_FMT_RAW12:
                case TG_FMT_RAW14:
                    break;
                default:
                    //quality is no good, no need to support by FPM
                    PIPE_ERR("EIS r not supported at current fmt(%d)\n",
                        CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT));
                    return MFALSE;
                    break;
                }

                if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                    return MFALSE;
                }
            }
            break;
        case EPIPECmd_SET_SGG2_CBFP:
            if(this->m_FSM != op_cfg){
                //because of sw_p1 done grping
                PIPE_ERR("SGG2_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }

            if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }
            break;
        case EPIPECmd_SET_LCS_CBFP:
            if(this->m_FSM != op_cfg){
                //because of sw_p1 done grping
                PIPE_ERR("LCS_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            switch(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT)){
            case TG_FMT_RAW8:
            case TG_FMT_RAW10:
            case TG_FMT_RAW12:
            case TG_FMT_RAW14:
                break;
            default:
                //Because DIP's SRZ/MFB..etc r not supported at current fmt, LCSO is useless.
                PIPE_ERR("LCS r not supported at current fmt(%d)\n",
                    CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT));
                return MFALSE;
                break;
            }
            if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }

            break;
        case EPIPECmd_SET_RSS_CBFP:
            {
                if(this->m_FSM != op_cfg){
                    //because of sw_p1 done grping
                    PIPE_ERR("RSS_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                    ret = 1;
                }

                if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                    return MFALSE;
                }
            }
            break;
        case EPIPECmd_SET_IQ_CBFP:
            {
                if(0 != arg1){
                    this->m_PathCtrl->m_CAMIO_CB = (CAMIO_CB)arg1;
                }else{
                    PIPE_ERR("SET_IQ_CBFP CB address must != NULL \n");
                }
            }
            break;

        case EPIPECmd_SET_RRZ_CBFP:
            if(this->m_FSM != op_cfg){
                PIPE_ERR("RRZ_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }

            break;
        case EPIPECmd_SET_TUNING_CBFP:
            if(this->m_FSM != op_cfg){
                PIPE_ERR("TUNING_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }

            break;
        case EPIPECmd_SET_REGDUMP_CBFP:
            if(this->m_FSM != op_cfg){
                PIPE_ERR("REGDUMP_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }
            break;
        case EPIPECmd_SET_AWB_CBFP:
            if(this->m_FSM != op_cfg){
                PIPE_ERR("REGDUMP_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            if(this->m_TwinMgr->P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }
            break;
        case EPIPECmd_AE_SMOOTH:
            //user cq1 to support OB only
            //Arg1: ob_gain
            {
                ISPIO_REG_CFG reg[4];
                list<ISPIO_REG_CFG> input;
                for(MUINT32 i=0;i<4;i++){
                    reg[i].Addr = _STRUCT_OFST(cam_reg_t, CAM_OBC_OFFST0) + i*0x4;
                    reg[i].Data = (MUINT32)arg1;
                    input.push_back(reg[i]);
                }

                if(this->m_pCmdQMgr_AE){
                    ret = this->m_pCmdQMgr_AE->CmdQMgr_Cfg(0,(MINTPTR)&input);
                    if(this->m_TwinMgr->getIsTwin())
                        ret += this->m_TwinMgr->CQ_cfg(arg1,(MINTPTR)&input);
                    if(ret == 0){
                        ret = this->m_pCmdQMgr_AE->CmdQMgr_start();
                        if(this->m_TwinMgr->getIsTwin())
                            ret += this->m_TwinMgr->CQ_Trig();
                    }
                    else{
                        PIPE_ERR("thread1 CFG fail\n");
                    }
                }
                else{
                    PIPE_ERR("only supported under non-high-speed mode\n");
                    return MFALSE;
                }
                break;
            }

        case EPIPECmd_HIGHSPEED_AE:
            //arg1: MUINT32 :signal idx, start from 0,
            //arg2: list<ISPIO_REG_CFG>* data for isp
            if((MUINT32)arg1 > this->m_camPass1Param.m_subSample){
                PIPE_ERR("out of range[0x%zx_0x%x].can't send data at this idx\n",arg1,this->m_camPass1Param.m_subSample);
                return MFALSE;
            }
            if(this->m_pHighSpeedCmdQMgr){
                //
                ret = this->m_pHighSpeedCmdQMgr->CmdQMgr_Cfg(arg1,arg2);
                if(this->m_TwinMgr->getIsTwin())
                    ret += this->m_TwinMgr->CQ_cfg(arg1,arg2);

                if(ret == 0){
                    ret = this->m_pHighSpeedCmdQMgr->CmdQMgr_start();
                    if(this->m_TwinMgr->getIsTwin())
                        ret += this->m_TwinMgr->CQ_Trig();
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
                MUINT32 pixMode = (E_CamPixelMode)arg2;
                MUINT32 size_h,size_v;

                PIPE_DBG("set TgLine(%d)pxmod(%d)\n", lineno, pixMode);

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

                /*sensor pix mode*/
                if(pixMode == ePixMode_2)
                    size_h *=2;
                else if(pixMode == ePixMode_4)
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
                    PIPE_ERR("SET_RRZ NULL param(0x%zx_0x%zx)!\n",arg1,arg2);
                    return MFALSE;
                }

                pInput_L = (list<STImgCrop>*)arg1;
                pInput2_L = (list<STImgResize>*)arg2;

                if( (pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) || (pInput2_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ())){
                    PIPE_ERR("rrz data length mismatch:0x%zx_0x%zx_0x%x\n",pInput_L->size(),pInput2_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
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
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_w(%lu))\n", \
                            (it->w+it->x), \
                            this->m_camPass1Param.m_src_size.w);
                        return MFALSE;
                    }
                    if ( (it->h+it->y) > this->m_camPass1Param.m_src_size.h) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_h(%lu))\n", \
                            (it->h+it->y), \
                            this->m_camPass1Param.m_src_size.h);
                        return MFALSE;
                    }

                    // If DBN=1, rrz setting must be divide by 2; if DBN = 1 and BIN = 1, imgo setting must be divide by 4
                    // This is performed in CAM_RRZ_CTRL::_config(void) of isp_function_cam cause rrz_in size need to be processed in it

                    // Reminder!! multiply rrz_in_roi to 2 when BIN = 1 cause MW passed this param according to RRZ input
                    // But CAM_RRZ_CTRL::_config(void) will divide it still
                    rrz.rrz_in_roi.x        = it->x;
                    rrz.rrz_in_roi.y        = it->y;
                    rrz.rrz_in_roi.floatX   = 0;
                    rrz.rrz_in_roi.floatY   = 0;
                    rrz.rrz_in_roi.w        = it->w;
                    rrz.rrz_in_roi.h        = it->h;
                    rrz.rrz_out_size.w      = it2->tar_w;
                    rrz.rrz_out_size.h      = it2->tar_h;
                    rrz.img_fmt             = it->img_fmt;

                    rrz_L.push_back(rrz);
                }

                if ( 0 != this->m_TwinMgr->setP1RrzCfg(&rrz_L) ){
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
                MUINT32 dmao_bw = 0;
                MUINT32 dupq = 0;

                for(it = pInput_L->begin();it!= pInput_L->end(); it++){
                    cfg.magic = *it;
                    cfg.SenDev = arg3;

                    cfg_L.push_back(cfg);
                }

                /* Note: setP1TuneCfg() must call BEFORE setP1Notify(),
                   Reason: setLCS CB needs magic#
                */
                if ( 0 != this->m_TwinMgr->setP1TuneCfg(&cfg_L) ){
                    PIPE_ERR("P1_UPDATE setP1TuneCfg Fail\n");
                    return MFALSE;
                }

                if(0 != this->m_TwinMgr->setP1Notify()){
                    PIPE_ERR("some notified-module fail\n");
                }
                //calculate BW and control DFS here.
                dupq = this->m_pCmdQMgr->CmdQMgr_GetDuqQIdx();
                for (MUINT32 burstq = 0; burstq < this->m_pCmdQMgr->CmdQMgr_GetBurstQ(); burstq++) {
                    //IMGO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,IMGO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_IMGO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_IMGO_STRIDE);
                        PIPE_DBG("Total BW:%d IMGO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_IMGO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_IMGO_YSIZE)+1);
                    }
                    //UFEO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,UFEO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFEO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFEO_STRIDE);
                        PIPE_DBG("Total BW:%d UEFO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFEO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFEO_YSIZE)+1);
                    }
                    //RRZO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,RRZO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RRZO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RRZO_STRIDE);
                        PIPE_DBG("Total BW:%d RRZO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RRZO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RRZO_YSIZE)+1);
                    }
                    //AFO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,AFO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AFO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AFO_STRIDE);
                        PIPE_DBG("Total BW:%d AFO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AFO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AFO_YSIZE)+1);
                    }
                    //AAO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,AAO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AAO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AAO_STRIDE);
                        PIPE_DBG("Total BW:%d AAO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AAO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_AAO_YSIZE)+1);
                    }
                    //PSO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,PSO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PSO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PSO_STRIDE);
                        PIPE_DBG("Total BW:%d PSO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PSO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PSO_YSIZE)+1);
                    }
                    //PDO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,PDO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PDO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PDO_STRIDE);
                        PIPE_DBG("Total BW:%d PDO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PDO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_PDO_YSIZE)+1);
                    }
                    //FLKO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,FLKO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_FLKO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_FLKO_STRIDE);
                        PIPE_DBG("Total BW:%d FLKO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_FLKO_STRIDE),\
                                                 CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_FLKO_YSIZE)+1);
                    }
                    //LMVO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,LMVO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_LMVO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_LMVO_STRIDE);
                        PIPE_DBG("Total BW:%d LMVO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_LMVO_STRIDE),\
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_LMVO_YSIZE)+1);
                    }
                    //RSSO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,RSSO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RSSO_A_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RSSO_A_STRIDE);
                        PIPE_DBG("Total BW:%d RSSO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RSSO_A_STRIDE),\
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_RSSO_A_YSIZE)+1);
                    }
                    //UFGO
                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_CTL_DMA_EN,UFGO_EN) == 1){
                        dmao_bw += (CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFGO_YSIZE)+1) * \
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFGO_STRIDE);
                        PIPE_DBG("Total BW:%d UFGO(%d*%d)",dmao_bw,CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFGO_STRIDE),\
                                   CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(dupq)[burstq],CAM_UFGO_YSIZE)+1);
                    }
                }
                this->m_cam_dfs->CAM_DFS_SetBw(dmao_bw,this->m_PathCtrl->m_cfgInParam.tg_fps,MFALSE);
                // If twin is enable:
                // 1. Compute the CQ data via twin_drv (dual_isp_driver) -- runTwin()
                // 2. Only twin module: CmdQMgr_update
                if (this->m_TwinMgr->setP1Update()) {
                    PIPE_ERR("setP1Udpate failed\n");
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
                MUINT32 imgo_fmt = 0;

                if (0 == arg1) {
                    PIPE_ERR("SET_IMGO NULL param!");
                    return MFALSE;
                }

                pInput_L = (list<STImgCrop>*)arg1;
                pInput2_L = (list<MUINT32>*)arg2;


                if( (pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) || (pInput2_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) ){
                    PIPE_ERR("imgo data length mismatch:0x%zx_0x%zx_0x%x\n",pInput_L->size(),pInput2_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
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
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_w(%lu))\n",\
                            (it->x + it->w), this->m_camPass1Param.m_src_size.w);
                        return MFALSE;
                    }
                    if ( (it->y + it->h) > this->m_camPass1Param.m_src_size.h) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_h(%lu))\n", \
                            (it->y + it->h), this->m_camPass1Param.m_src_size.h);
                        return MFALSE;
                    }

                    // If DBN=1, imgo setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
                    // This is performed in DMAO_B::_config(void) of isp_function_cam.dmax cause vOutPorts in configPipe() is read-only object (constant)

                    imgo.crop.x      = it->x;
                    imgo.crop.y      = it->y;
                    imgo.crop.w      = it->w;
                    imgo.crop.h      = it->h;
                    imgo.crop.floatX = imgo.crop.floatY = 0;
                    imgo.img_fmt            = it->img_fmt;                              //enqueue's img format
                    imgo.enqueue_img_stride = it->enqueue_img_stride;                   //enqueue's img stride
                    imgo.img_fmt_configpipe = m_camPass1Param.m_imgo.dmao.lIspColorfmt; //configpipe's img format
                    imgo.out.stride         = m_camPass1Param.m_imgo.dmao.size.stride;  //configpipe's img stride
                    imgo.tgCropW            = m_camPass1Param.m_src_size.w;

                    this->getOutPxlByteNFmt(EPortIndex_IMGO, imgo.img_fmt, (MINT32*)&imgo.pixel_byte, (MINT32*)&imgo_fmt);

                    PIPE_DBG("(configpipe_img_format,enque_img_format,configpipe_img_stride,enque_img_stride,tgCropW)=(0x%x,0x%x,%d,%d,%d), pixel_byte(%d)\n",\
                               imgo.img_fmt_configpipe, imgo.img_fmt, imgo.out.stride, imgo.enqueue_img_stride, imgo.tgCropW, imgo.pixel_byte);

                    if(this->m_DynamicRawType==MFALSE){
                        switch(this->m_camPass1Param.m_NonScale_RawType){
                            case CamPathPass1Parameter::_PURE_RAW:
                                imgo.rawType = 1;

                                switch (this->m_camPass1Param.m_data_pat) {
                                case eCAM_DUAL_PIX:
                                case eCAM_4CELL:
                                    PIPE_INF("DualPix/QuadCode(%d) force pure raw path\n", (int)this->m_camPass1Param.m_data_pat);
                                    break;
                                case eCAM_NORMAL:
                                default:
                                    //can't read phy, to avoid read before cq-trig at the very 1st enque before isp_start
                                    if(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0],CAM_CTL_EN,DBN_EN) == 1){
                                        PIPE_ERR("support no IMGO_PURE_RAW + DUAL_PIX data pattern\n");
                                        return MFALSE;
                                    }
                                    break;
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

                if ( 0 != this->m_TwinMgr->setP1ImgoCfg(&input_L) ){
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

            if (MFALSE == this->m_TwinMgr->updateFrameTime((MUINT32)arg1, (MUINT32)arg2)) {
                PIPE_ERR("Update frame time fail: arg1:%d arg2:%d", (MUINT32)arg1, (MUINT32)arg2);
                ret = MFALSE;
                break;
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
        case EPIPECmd_GET_FLK_INFO:
            //arg1: output MTRUE or MFALSE.
            // Due to hardware design: each cam has its own flk module.
            *(MUINT32*)arg1 = MTRUE;
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

                switch(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT)) {
                case TG_FMT_YUV422:
                    pixMode = ((CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) +
                        CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);//+1 for YUV fmt
                    *(MUINT32*)arg1  = size_h >> pixMode;
                    *(MUINT32*)arg2  = size_v;
                    break;

                case TG_FMT_RAW8:
                case TG_FMT_RAW10:
                case TG_FMT_RAW12:
                case TG_FMT_RAW14:
                    bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                    bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,DBN_EN);
                    size_h = size_h>>bin;
                    bin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                    bin += (CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN2,VBN_EN));
                    size_v = size_v>>bin;

                    //qbin1
                    pixMode = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN1_MODE, QBN_ACC);
                    *(MUINT32*)arg1  = size_h >> pixMode;
                    *(MUINT32*)arg2  = size_v;
                    break;

                default:
                    PIPE_ERR("unsupproted TG fmt(%d)\n",CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT));
                    ret = MFALSE;
                    break;
                }

            }
            break;
        case EPIPECmd_GET_RMB_INFO:
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

                switch(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT)) {
                case TG_FMT_YUV422:
                    pixMode = ((CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) +
                        CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);//+1 for YUV fmt
                    *(MUINT32*)arg1  = size_h >> pixMode;
                    *(MUINT32*)arg2  = size_v;
                    break;

                case TG_FMT_RAW8:
                case TG_FMT_RAW10:
                case TG_FMT_RAW12:
                case TG_FMT_RAW14:
                    bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                    bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,DBN_EN);
                    size_h = size_h>>bin;
                    bin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                    bin += (CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN2,VBN_EN));
                    size_v = size_v>>bin;

                    //rmb
                    pixMode = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_RMB_MODE, ACC);
                    *(MUINT32*)arg1  = size_h >> pixMode;
                    *(MUINT32*)arg2  = size_v;
                    break;

                default:
                    PIPE_ERR("unsupproted TG fmt(%d)\n",CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT));
                    ret = MFALSE;
                    break;
                }
            }
            break;
        case EPIPECmd_GET_BIN_INFO:
            //arg1: output size_w
            //arg2: output size_h
            {
                CAM_REG_TG_SEN_GRAB_PXL TG_W;
                CAM_REG_TG_SEN_GRAB_LIN TG_H;
                MUINT32 pixMode = 0;
                MUINT32 bin=0;
                MUINT32 size_h,size_v;

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;

                switch(CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT)){
                case TG_FMT_RAW8:
                case TG_FMT_RAW10:
                case TG_FMT_RAW12:
                case TG_FMT_RAW14:
                    bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                    bin += CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,DBN_EN);
                    *(MUINT32*)arg1 = size_h>>bin;
                    bin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                    bin += (CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN2,VBN_EN));
                    *(MUINT32*)arg2 = size_v>>bin;
                    break;

                case TG_FMT_YUV422:
                    pixMode = ((CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS) +
                        CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_MODE,DBL_DATA_BUS1)) + 1);//+1 for YUV fmt

                    *(MUINT32*)arg1  = size_h >> pixMode;
                    *(MUINT32*)arg2  = size_v;
                    break;

                default:
                    PIPE_ERR("unsupproted TG fmt(%d)\n",CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_FMT_SEL,TG_FMT));
                    ret = MFALSE;
                    break;
                }

            }
            break;
        case EPIPECmd_GET_CQUPDATECNT:
            //arg1: MTRUE for get phy reg, MFALSE for get vir reg.
            //arg2: return output info.
            *(MUINT32*)arg2 = this->m_pCmdQMgr->CmdQMgr_GetCounter(arg1);

            break;
        case EPIPECmd_GET_HEADER_SIZE:
            //arg1: input portID, eg:NSImageio::NSIspio::EPortIndex_IMGO
            //arg2: return size of Header, unit: byte
            {
                capibility CamInfo;
                tCAM_rst rst;
                MUINT32 channel;
                MUINT32 size;

                if(CamInfo.GetCapibility(arg1,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo(),
                                            rst,E_CAM_HEADER_size) == MFALSE) {
                    ret = 1;
                    goto EXIT;
                }

                if((channel = (MUINT32)this->PortID_MAP(arg1)) == _cam_max_){
                    //EIS/RSS/LCS
                    channel = this->m_CamIO_stt.QueryPort(arg1);
                }

                //always add twin'size due to dynamic twin
                if(this->m_TwinMgr->sendCommand(TWIN_CMD_GET_SLAVE_HEADERSIZE,(MINTPTR)channel,(MINTPTR)&size,0) == MFALSE){
                    ret = 1;
                    goto EXIT;
                }
                else
                    *(MUINT32*)arg2 = rst.HeaderSize + size;
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

                TG_W.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_PXL);
                TG_H.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_TG_SEN_GRAB_LIN);
                size_h = (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S);
                size_v = TG_H.Bits.LIN_E - TG_H.Bits.LIN_S;
                bin = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,BIN_EN);
                dbn = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_EN,DBN_EN);

                MSize *szIMGO = (MSize*)arg1;
                //element:0 -> stand for processed raw size
                //element:1 -> stnad for pire raw size
                for(MUINT32 i=0; i<2; i++){
                    if(i==1){//pure raw
                        szIMGO[i].w = size_h>>(dbn);
                        szIMGO[i].h = size_v;
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
        case EPIPECmd_GET_RCP_INFO:
            {
                V_CAMIO_CROP_INFO*          pVec = (V_CAMIO_CROP_INFO*)arg1;
                ST_CAMIO_CROP_INFO          crop;
                ST_RECT                     rect;
                vector<vector<ISP_HW_MODULE>>   v_v_module;
                vector<vector<STImgCrop>>       v_v_STImgCrop;
                vector<STImgCrop>::iterator     it;

                pVec->clear();
                //single path,support no dynamic bin
                crop.Pipe_path = EPipe_UNKNOWN;
                CAM_MAP(this->m_hwModule,crop.Pipe_path);
                crop.v_rect.clear();
                rect.x = 0;
                rect.y = 0;
                if(this->sendCommand(EPIPECmd_GET_BIN_INFO,(MINTPTR)&rect.w,(MINTPTR)&rect.h,0) == MFALSE){
                    ret = -1;
                    goto EXIT;
                }
                crop.v_rect.push_back(rect);

                pVec->push_back(crop);
                //twin path
                if(this->m_TwinMgr == NULL){
                    PIPE_ERR("twin is not supported with cam:%d\n",this->m_hwModule);//due to createinstance at configpipe fail
                    ret = -1;
                    goto EXIT;
                }
                if(this->m_TwinMgr->sendCommand(TWIN_CMD_GET_TWIN_CROPINFO,(MINTPTR)&v_v_module,\
                    (MINTPTR)&v_v_STImgCrop,0) == MFALSE){
                    ret = -1;
                    goto EXIT;
                }

                //twinmgr should gurrente that size of module should be equal to size of ImgCrop, use module only.
                for(MUINT32 i=0;i<v_v_module.size();i++){
                    crop.Pipe_path = EPipe_UNKNOWN;
                    crop.v_rect.clear();
                    it = v_v_STImgCrop.at(i).begin();
                    for(MUINT32 j=0;j<v_v_module.at(i).size();j++,it++){
                        CAM_MAP(v_v_module.at(i).at(j),crop.Pipe_path);//or twin path
                        rect.x = it->x;
                        rect.y = it->y;
                        rect.w = it->w;
                        rect.h = it->h;
                        crop.v_rect.push_back(rect);
                    }
                    pVec->push_back(crop);
                }

                PIPE_DBG("vec size:%d\n",(MUINT32)pVec->size());
                for(MUINT32 i=0;i<pVec->size();i++){
                    PIPE_DBG("vec_%d: path:0x%x,size:%d_%d_%d_%d\n",i,pVec->at(i).Pipe_path,\
                        pVec->at(i).v_rect.at(0).x,pVec->at(i).v_rect.at(0).y,\
                        pVec->at(i).v_rect.at(0).w,pVec->at(i).v_rect.at(0).h);
                }
            }
            break;
        case EPipeCmd_GET_MAGIC_REG_ADDR:
            {
                vector<E_CAMIO_CAM>* ptr1 = (vector<E_CAMIO_CAM>*)arg1;
                vector<MUINT32>* ptr2 = (vector<MUINT32>*)arg2;
                if(ptr1 && ptr2){
                    //currently , rrz will be always enabled. so return rrz's magic number address is enough.
                    switch(this->m_hwModule){
                        case CAM_A:
                            ptr1->push_back(EPipe_CAM_A);
                            break;
                        case CAM_B:
                            ptr1->push_back(EPipe_CAM_B);
                            break;
                        default:
                            ret = -1;
                            PIPE_ERR("unsupported cam\n");
                            ptr1->push_back(EPipe_UNKNOWN);
                            break;
                    }
                    ptr2->push_back(Header_RRZO::GetTagAddr(Header_RRZO::E_Magic));
                }
                else{
                    PIPE_ERR("arg1 & arg2 can't be 0\n");
                    ret = -1;
                }
            }
            break;
        case EPipeCmd_DTwin_INFO:
            {
                capibility CamInfo;
                switch(arg1){
                    case 0:
                        if(arg2){
                            *(MUINT32*)arg2 = CamInfo.m_DTwin.GetDTwin();
                        }
                        else{
                            PIPE_ERR("arg2 can't be 0\n");
                            ret = -1;
                        }
                        break;
                    case 1:
                        if(arg2){
                            CamInfo.m_DTwin.SetDTwin((MBOOL)(*(MUINT32*)arg2));
                        }
                        else{
                            PIPE_ERR("arg2 can't be 0\n");
                            ret = -1;
                        }
                        break;
                    default:
                        PIPE_ERR("not supported:%zd\n",arg1);
                        ret = -1;
                        break;
                }
            }
            break;
        case EPipeCmd_GET_TWIN_REG_ADDR:
            {
                vector<E_CAMIO_CAM>* ptr1 = (vector<E_CAMIO_CAM>*)arg1;
                vector<MUINT32>* ptr2 = (vector<MUINT32>*)arg2;
                if((ptr1 == NULL) || (ptr2 == NULL)){
                    ret = -1;
                    goto EXIT;
                }

                MINT32 addr1=-1,addr2=-1,addr3=-1;
                if(this->m_TwinMgr->sendCommand(TWIN_CMD_GET_TWIN_REG,(MINTPTR)&addr1,(MINTPTR)&addr2,(MINTPTR)&addr3) == MFALSE){
                    ret = -1;
                    goto EXIT;
                }
                if(addr1 != -1){
                    ptr1->push_back(EPipe_CAM_A);
                    ptr2->push_back(addr1);
                }
                if(addr2 != -1){
                    ptr1->push_back(EPipe_CAM_B);
                    ptr2->push_back(addr2);
                }
                #if 0
                if(addr3 != -1){
                    ptr1->push_back(EPipe_CAM_C);
                    ptr2->push_back(addr3);
                }
                #endif
            }
            break;
        case EPIPECmd_SET_PM_QOS_INFO:
            {
                ISP_PM_QOS_INFO_STRUCT pm_qos_info;
                pm_qos_info.bw_value = *(MUINT32*)arg1;
                pm_qos_info.fps = *(MUINT32*)arg2;
                pm_qos_info.module = this->m_hwModule;
                if (MFALSE == this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->setDeviceInfo(_SET_PM_QOS_INFO, (MUINT8*)&pm_qos_info)) {
                    PIPE_ERR("EPIPECmd_SET_PM_QOS_INFO failed!\n");
                }
            }
            break;
        case EPIPECmd_SET_PM_QOS_RESET:
            if (MFALSE == this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->setDeviceInfo(_SET_PM_QOS_RESET, (MUINT8*)&this->m_hwModule)) {
                PIPE_ERR("EPIPECmd_SET_PM_QOS_RESET failed!\n");
            }
            break;
        // reserve for NormalPipe IP based.
        case EPIPECmd_GET_UNI_INFO:
        case EPIPECmd_UNI_SWITCHOUT:
        case EPIPECmd_UNI_SWITCHOUTRIG:
        case EPIPECmd_GET_UNI_SWITCHOUTRIG_DONE:
        case EPIPECmd_UNI_SWITCHIN:
        case EPIPECmd_UNI_SWITCHINTRIG:
        case EPIPECmd_GET_UNI_ATTACH_MODULE:
        case EPIPECmd_SET_UFEO:
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

MBOOL CamIOPipe::configDmaPort(PortInfo const* portInfo,IspDMACfg &a_dma,MUINT32 pixel_Byte,MUINT32 swap, MUINT32 isBypassOffset,E_BufPlaneID planeNum)
{
    capibility CamInfo;
    tCAM_rst rst;
    MBOOL tmp;

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
    a_dma.lIspColorfmt = portInfo->eImgFmt;

    //dma port capbility
    a_dma.capbility=portInfo->capbility;
    //input xsize unit:byte
    tmp = CamInfo.GetCapibility(
            portInfo->index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE,
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)portInfo->eImgFmt, a_dma.size.w, ePixMode_NONE),
            rst, E_CAM_UNKNOWNN);
    if(tmp == MFALSE) {
        PIPE_ERR("CamInfo.GetCapibility error");
    }
    a_dma.size.xsize        =  rst.xsize_byte[0];
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum == ePlane_1st) {
        PIPE_ERR("[Error]:stride size(%lu) < image width(%lu) byte size",a_dma.size.stride,a_dma.size.w);
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

    PIPE_DBG("w(%lu),h(%lu),stride(%lu),xsize(%lu),crop(%d,%d,%lu,%lu)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%zx),pa(0x%zx),pixel_Byte(%d)",a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        a_dma.memBuf.base_vAddr, a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",portInfo->eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}

inline void cpyRes_Meter_IN(Res_Meter_IN* output, Res_Meter_IN* input)
{
    output->bypass_tg = input->bypass_tg;
    output->pix_mode_tg = input->pix_mode_tg;
    output->tg_crop_w = input->tg_crop_w;
    output->tg_crop_h = input->tg_crop_h;
    output->tg_fps = input->tg_fps;
    output->SrcImgFmt = input->SrcImgFmt;
    output->bypass_rrzo = input->bypass_rrzo;
    output->rrz_out_w = input->rrz_out_w;
    output->offBin = input->offBin;
    output->offTwin = input->offTwin;
    output->pattern = input->pattern;
    output->MIPI_pixrate = input->MIPI_pixrate;
    output->vClk.assign(input->vClk.begin(),input->vClk.end());
}

inline void cpyRes_Meter_OUT(Res_Meter_OUT* output, Res_Meter_OUT* input)
{
    output->xmxo = input->xmxo;
    output->bin_en = input->bin_en;
    output->isTwin = input->isTwin;
    output->clk_level = input->clk_level;
    output->slave_module.assign(input->slave_module.begin(),input->slave_module.end());
}

MBOOL CamIOPipe::configFrame(E_FRM_EVENT event,CAMIO_Func func)
{
    MBOOL ret = MTRUE;
    PIPE_DBG("event:%d\n",event);

    if(this->m_pCfgFrame == NULL)
        this->m_pCfgFrame = new CamIOPipe_cfgFrame((MVOID*)this);

    this->m_cam_dfs->CAM_DFS_Vsync();//perframe request from DFS
    this->m_cam_dfs->CAM_DBS_Vsync();

    switch(event){
        case CAMIO_DROP:
        {
            break;
        }
        case CAMIO_D_Twin:
        {
            capibility CamInfo;
            if(this->m_PathCtrl->m_ThreeRawStage == CamIO_PathCtrl::NoThreeRaw){
                //fix twin path if no D-twin
                if((this->m_PathCtrl->m_resCfgOutParam.isTwin == MFALSE) && (CamInfo.m_DTwin.GetDTwin() == MFALSE))
                    this->m_PathCtrl->m_cfgInParam.offTwin = MTRUE;
                //
                this->m_PathCtrl->m_cfgInParam.SenNum = (E_CAMIO_SEN)func.Bits.SensorNum;
                //
                if(MFALSE == this->m_pResMgr->Res_Meter(this->m_PathCtrl->m_cfgInParam, this->m_PathCtrl->m_resCfgOutParam)) {
                    PIPE_ERR("ResMgr Res_Meter() error!!!!!!\n");
                    return MFALSE;
                }

                //before/after start pipeline
                if(this->m_FSM == op_start){
                    this->m_cam_dfs->CAM_DFS_SetLv(this->m_PathCtrl->m_resCfgOutParam.clk_level);
                }else{
                    this->m_cam_dfs->CAM_DFS_SetLv(this->m_PathCtrl->m_resCfgOutParam.clk_level,MTRUE);
                }
                //
                if(this->m_PathCtrl->m_resCfgOutParam.isTwin == MTRUE){
                    ret = this->m_PathCtrl->TwinPath(this->m_PathCtrl->m_resCfgOutParam);
                }else{
                    ret = this->m_PathCtrl->SinglePath(this->m_PathCtrl->m_resCfgOutParam);
                }
            }else{// do IQ HL stage
                switch(this->m_PathCtrl->m_IQlv){
                    case eCamIQ_L:
                        ret = this->m_PathCtrl->ThreeRawIQLPath();
                        break;
                    case eCamIQ_H:
                        ret = this->m_PathCtrl->ThreeRawIQHPath();
                        break;
                    default:
                        PIPE_ERR("PathCtrl IQlv error %d\n", event);
                        ret = MFALSE;
                        break;
                }
            }
            break;
        }
        case CAMIO_IQ:
            //
            this->m_PathCtrl->m_cfgInParam.SenNum = (E_CAMIO_SEN)func.Bits.SensorNum;
            //
            switch(func.Bits.IQlv){
                case eCamIQ_L:
                    this->m_PathCtrl->m_IQlv = eCamIQ_L;
                    if(MFALSE == this->m_PathCtrl->ThreeRawFSMCheck(CamIO_PathCtrl::ThreeRawInitStage1)){
                        return MFALSE;
                    }
                    this->m_PathCtrl->m_ThreeRawStage = CamIO_PathCtrl::ThreeRawInitStage1;
                    ret = this->m_PathCtrl->ThreeRawIQLPath();
                    break;

                case eCamIQ_H:
                    this->m_PathCtrl->m_IQlv = eCamIQ_H;
                    if(MFALSE == this->m_PathCtrl->ThreeRawFSMCheck(CamIO_PathCtrl::ThreeRawInitStage1)){
                        return MFALSE;
                    }
                    this->m_PathCtrl->m_ThreeRawStage = CamIO_PathCtrl::ThreeRawInitStage1;
                    ret = this->m_PathCtrl->ThreeRawIQHPath();
                    break;
                default:
                    PIPE_ERR("func.Bits.IQlv %d\n", event);
                    ret = MFALSE;
                    break;
            }
            break;

        default:
            PIPE_ERR("Error E_FRM_EVENT %d\n", event);
            ret = MFALSE;
            break;
    }
    return ret;
}

MVOID CamIO_PathCtrl::ResToTwinParamUpdate(Res_Meter_IN const &resCfgInParam,
                                           Res_Meter_OUT const &resCfgOutParam,
                                           TWIN_MGR_PATH_CFG_IN_PARAM &twinCfgInParam)
{
    twinCfgInParam.tg_crop_w   = resCfgInParam.tg_crop_w;
    twinCfgInParam.tg_crop_h   = resCfgInParam.tg_crop_h;
    twinCfgInParam.bypass_rrzo = resCfgInParam.bypass_rrzo;
    twinCfgInParam.bypass_imgo = this->m_pCamIOpipe->m_camPass1Param.bypass_imgo;
    twinCfgInParam.bDbn        = this->m_bDbn;
    twinCfgInParam.bBin        = resCfgOutParam.bin_en;
    twinCfgInParam.isTwin      = resCfgOutParam.isTwin;
    twinCfgInParam.slave_mod   = resCfgOutParam.slave_module;
    twinCfgInParam.pTwinCB     = this->m_pCamIOpipe->m_pCfgFrame;
}

MBOOL CamIO_PathCtrl::TwinPath(Res_Meter_OUT &resCfgOutParam)
{
    MBOOL ret = MTRUE;
    list<MUINT32> l_channel;

    this->ResToTwinParamUpdate(this->m_cfgInParam, resCfgOutParam, this->m_twinCfgInParam);
    if(MFALSE == this->m_pCamIOpipe->m_TwinMgr->pathControl(this->m_twinCfgInParam)){
        PIPE_ERR("TwinPath pathControl Error \n");
        return MFALSE;
    }

    if (op_start == this->m_pCamIOpipe->m_FSM) {
        TwinMgr::E_TWIN_STATUS state;

        ret = this->m_pCamIOpipe->m_TwinMgr->sendCommand(TWIN_CMD_GET_TWIN_STATE,(MINTPTR)&state,0,0);

        switch (state) {
        case TwinMgr::E_INIT:   //single path 2 twin path with pipline had not ran in twin path
            //start here before 1st updateTwin , to avoid fsm error when using updateTwin.
            ret = this->m_pCamIOpipe->m_TwinMgr->startTwin();
            break;
        default:
            break;
        }
    }

    if(MFALSE == this->m_pCamIOpipe->m_TwinMgr->updatePath(
        UpdatePathParam(&resCfgOutParam.xmxo, resCfgOutParam.bin_en))){
        PIPE_ERR("twin path updatePathParam failed\n");
        ret = MFALSE;
    }

    return ret;
}

MBOOL CamIO_PathCtrl::SinglePath(Res_Meter_OUT &resCfgOutParam)
{
    MBOOL ret = MTRUE;

    TwinMgr::E_TWIN_STATUS state;

    this->ResToTwinParamUpdate(this->m_cfgInParam, resCfgOutParam, this->m_twinCfgInParam);
    if(MFALSE == this->m_pCamIOpipe->m_TwinMgr->pathControl(this->m_twinCfgInParam)){
        PIPE_ERR("SinglePath pathControl Error \n");
        return MFALSE;
    }

    if (this->m_pCamIOpipe->m_camPass1Param.bypass_ispRawPipe == MFALSE) {
        if(MFALSE == this->m_pCamIOpipe->m_TwinMgr->updatePath(
            UpdatePathParam(&resCfgOutParam.xmxo, resCfgOutParam.bin_en))){
            PIPE_ERR("Single path updatePathParam failed\n");
            ret = MFALSE;
        }
    }

    return ret;
}


MBOOL CamIO_PathCtrl::ThreeRawFSMCheck(MUINT32 op)
{
    MBOOL ret = MTRUE;
    this->m_ThreeRawFSMLock.lock();
    switch(op){
        case NoThreeRaw:
            if(this->m_ThreeRawStage != ThreeRawTwinReady){
               ret = MFALSE;
            }
            break;
        case ThreeRawInitStage1:
            if(this->m_ThreeRawStage != NoThreeRaw){
               ret = MFALSE;
            }
            break;
        case ThreeRawInitStage2:
            if(this->m_ThreeRawStage != ThreeRawInitStage1){
               ret = MFALSE;
            }
            break;
        case ThreeRawTwinReady:
            if(this->m_ThreeRawStage != ThreeRawInitStage2){
                ret = MFALSE;
            }
            break;
        default:
            ret = MFALSE;
            break;
    }
    if(ret == MFALSE){
        PIPE_ERR("m_ThreeRawStage error:cur:0x%x,tar:0x%x\n",this->m_ThreeRawStage,op);
    }
    this->m_ThreeRawFSMLock.unlock();
    return ret;
}

MBOOL CamIO_PathCtrl::ThreeRawIQLPath(void)
{
    Res_Meter_OUT lResCfgOutParam;
    Res_Meter_IN  lResCfgInParam;
    MBOOL ret = MTRUE;
    PIPE_INF("current ThreeRawState:%d\n",this->m_ThreeRawStage);

    cpyRes_Meter_IN(&lResCfgInParam, &this->m_cfgInParam);

    switch(this->m_ThreeRawStage){
        case NoThreeRaw:
            break;
        case ThreeRawInitStage1:
        {
            // To determine LineBuffer is enough
            capibility CamInfo(this->capibility_Map(this->m_pCamIOpipe->m_hwModule));
            tCAM_rst rst;
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo info;
            info.width = this->m_cfgInParam.tg_crop_w;

            if(MFALSE == CamInfo.GetCapibility(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                    info, rst, E_CAM_pipeline_size)){
                    PIPE_ERR("ThreeRawInit GetCapibility fail.\n");
                    return MFALSE;
            }

            if(rst.pipeSize > this->m_cfgInParam.tg_crop_w){
                 this->m_LineBufferEnough = MTRUE;
            }
            //
            if(MFALSE == this->m_LineBufferEnough){
                lResCfgInParam.offBin  = 0; // Bin  On
                lResCfgInParam.offTwin = 1; // Twin off
            }else{
                lResCfgInParam.offBin  = 1; // Bin  off
                lResCfgInParam.offTwin = 1; // Twin off
            }
            //
            lResCfgOutParam.clk_level = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_GetCurLv();// Must add before Res_Meter

            if(MTRUE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)){
                if(MFALSE == lResCfgOutParam.isTwin){
                    // update new res In and Out Param.
                    cpyRes_Meter_OUT(&this->m_resCfgOutParam, &lResCfgOutParam);
                    cpyRes_Meter_IN(&this->m_cfgInParam, &lResCfgInParam);
                    // before/after start pipeline
                    if(op_start == this->m_pCamIOpipe->m_FSM){
                        this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level);
                    }else{
                        this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level,MTRUE);
                    }

                    if(this->SinglePath(lResCfgOutParam)){
                        this->m_ThreeRawStage = ThreeRawInitStage2;
                    }else{
                        PIPE_ERR("ThreeRawInitStage1 IQL switch Single Fail.\n");
                        ret = MFALSE;
                    }
                }else{// MTRUE == lResCfgOutParam.isTwin
                    PIPE_ERR("ThreeRawInitStage1 IQL open Single Fail.\n");
                    ret = MFALSE;
                }
            }else{// MFLASE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)
                PIPE_ERR("ThreeRawInitStage1 IQL Res_Meter Fail.\n");
                ret = MFALSE;
            }
            break;
        }
        case ThreeRawInitStage2:
        {
            if(MFALSE == this->m_LineBufferEnough){
                lResCfgOutParam.clk_level = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_GetCurLv();// Must add before Res_Meter
                if(MTRUE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(this->m_cfgInParam,lResCfgOutParam)){
                    if(MFALSE == lResCfgOutParam.isTwin){
                        // update new res In and Out Param.
                        cpyRes_Meter_OUT(&this->m_resCfgOutParam, &lResCfgOutParam);

                        // before/after start pipeline
                        if(op_start == this->m_pCamIOpipe->m_FSM){
                            this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level);
                        }else{
                            this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level,MTRUE);
                        }

                        if(this->SinglePath(lResCfgOutParam)){
                            this->m_ThreeRawStage = ThreeRawTwinReady;
                        }else{
                            PIPE_ERR("ThreeRawInitStage2 IQL switch Single Fail.\n");
                            ret = MFALSE;
                        }

                    }else{// MTRUE == lResCfgOutParam.isTwin
                        PIPE_ERR("ThreeRawInitStage2 IQL open Single Fail.\n");
                        ret = MFALSE;
                    }
                }else{// MFALSE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(this->m_cfgInParam,lResCfgOutParam)
                    PIPE_ERR("ThreeRawInitStage2 IQL Res_Meter Fail.\n");
                    ret = MFALSE;
                }
            }else{// MTURE == this->m_LineBufferEnough
                lResCfgInParam.offBin  = 0; // Bin On
                lResCfgInParam.offTwin = 1; // Twin off

                lResCfgOutParam.clk_level = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_GetCurLv();// Must add before Res_Meter
                if(MTRUE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)){
                    if(MFALSE == lResCfgOutParam.isTwin){
                        // update new res In and Out Param.
                        cpyRes_Meter_OUT(&this->m_resCfgOutParam, &lResCfgOutParam);
                        cpyRes_Meter_IN(&this->m_cfgInParam, &lResCfgInParam);

                        // before/after start pipeline
                        if(op_start == this->m_pCamIOpipe->m_FSM){
                            this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level);
                        }else{
                            this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level,MTRUE);
                        }

                        if(this->SinglePath(lResCfgOutParam)){
                            this->m_ThreeRawStage = ThreeRawTwinReady;
                        }else{
                            PIPE_ERR("ThreeRawInitStage2 IQL switch Single Fail.\n");
                            ret = MFALSE;
                        }

                    }else{// MTURE == lResCfgOutParam.isTwin
                        PIPE_ERR("ThreeRawInitStage2 IQL open Single Fail.\n");
                        ret = MFALSE;
                    }
                }else{// MFALSE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)
                    PIPE_ERR("ThreeRawInitStage2 IQL Res_Meter Fail.\n");
                    ret = MFALSE;
                }
            }
            break;
        }
        case ThreeRawTwinReady: //Twin off ready
        {
            MBOOL retDFSSetLv = MTRUE;

            lResCfgOutParam.clk_level = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_GetCurLv();// Must add before Res_Meter
            if(MTRUE != this->m_pCamIOpipe->m_pResMgr->Res_Meter(this->m_cfgInParam,lResCfgOutParam)){
                 PIPE_ERR("ThreeRawTwinReady IQL Init Res_Meter Fail.\n");
                 ret = MFALSE;
            }

            //before/after start pipeline
            if(op_start == this->m_pCamIOpipe->m_FSM){
                retDFSSetLv = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level);
            }else{
                retDFSSetLv = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level,MTRUE);
            }

            if(NULL != this->m_CAMIO_CB){
                this->m_CAMIO_CB(this->m_hwModule); // set Call back function
                this->m_ThreeRawStage = NoThreeRaw;
                if(MFALSE == this->SinglePath(this->m_resCfgOutParam)){
                    PIPE_ERR("ThreeRawTwinReady IQL single Fail.\n");
                    ret = MFALSE;
                }
            }else{
                PIPE_ERR("ThreeRawTwinReady IQL CAMIO_CB equal NULL.\n");
                ret = MFALSE;
            }

            break;
        }
        default:
            PIPE_ERR("ThreeRaw IQL states error.\n");
            ret = MFALSE;
            break;
    }
    return ret;
}

MBOOL CamIO_PathCtrl::ThreeRawIQHPath(void)
{
    Res_Meter_OUT lResCfgOutParam;
    Res_Meter_IN  lResCfgInParam;
    // use previous res In Param
    MBOOL ret = MTRUE;
    PIPE_INF("current ThreeRawState:%d\n",this->m_ThreeRawStage);

    cpyRes_Meter_IN(&lResCfgInParam, &this->m_cfgInParam);

    switch(this->m_ThreeRawStage){
        case NoThreeRaw:
            break;
        case ThreeRawInitStage1:
        {
            // To determine LineBuffer is enough
            capibility CamInfo(this->capibility_Map(this->m_pCamIOpipe->m_hwModule));
            tCAM_rst rst;
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo info;
            info.width = this->m_cfgInParam.tg_crop_w;
            if(MFALSE == CamInfo.GetCapibility(0, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN,
                                    info, rst, E_CAM_pipeline_size)){
                    PIPE_ERR("ThreeRawInit GetCapibility fail.\n");
                    return MFALSE;
            }

            if(rst.pipeSize > this->m_cfgInParam.tg_crop_w){
                 this->m_LineBufferEnough = MTRUE;
            }
            //
            if(MFALSE == this->m_LineBufferEnough){
                lResCfgInParam.offTwin  = 0; // Twin on
                lResCfgInParam.offBin   = 0; // Bin  auto on
            }else{
                lResCfgInParam.offBin   = 1; // Bin off
                lResCfgInParam.offTwin  = 0; // Twin on
            }
            //
            lResCfgOutParam.clk_level = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_GetCurLv();// Must add before Res_Meter
            if(MTRUE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)){

               // before/after start pipeline
               if(op_start == this->m_pCamIOpipe->m_FSM){
                   this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level);
               }else{
                   this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level,MTRUE);
               }

                if(MTRUE == lResCfgOutParam.isTwin){
                    // update new res In and Out Param.
                    cpyRes_Meter_OUT(&this->m_resCfgOutParam, &lResCfgOutParam);
                    cpyRes_Meter_IN(&this->m_cfgInParam, &lResCfgInParam);

                    if(this->TwinPath(lResCfgOutParam)){
                        this->m_ThreeRawStage = ThreeRawInitStage2;
                    }else{
                        PIPE_ERR("ThreeRawInitStage1 IQH switch Twin Fail.\n");
                        ret = MFALSE;
                    }
                }else{// MFALSE == lResCfgOutParam.isTwin
                    // ThreeRawInitStage1 IQH wait Twin.
                    this->SinglePath(lResCfgOutParam);
                    this->m_ThreeRawStage = ThreeRawInitStage2;
                }
            }else{// MFALSE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)
                 PIPE_ERR("ThreeRawInitStage1 IQH Res_Meter Fail.\n");
                 ret = MFALSE;
            }

            break;
        }
        case ThreeRawInitStage2:
        {
            if(MTRUE == this->m_LineBufferEnough){
                lResCfgInParam.offTwin = 0; // Twin on
                lResCfgInParam.offBin  = 1; // Bin off
            }else{
                lResCfgInParam.offTwin = 0; // Twin on
                lResCfgInParam.offBin  = 0; // Bin auto on //
            }

            lResCfgOutParam.clk_level = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_GetCurLv();// Must add before Res_Meter
            if(MTRUE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)){
                // before/after start pipeline
                if(op_start == this->m_pCamIOpipe->m_FSM){
                    this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level);
                }else{
                    this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level,MTRUE);
                }

                if(MTRUE == lResCfgOutParam.isTwin){
                    // update new res In and Out Param.
                    cpyRes_Meter_OUT(&this->m_resCfgOutParam, &lResCfgOutParam);
                    cpyRes_Meter_IN(&this->m_cfgInParam, &lResCfgInParam);

                    if(this->TwinPath(lResCfgOutParam)){
                        this->m_ThreeRawStage = ThreeRawTwinReady;
                    }else{
                        PIPE_ERR("ThreeRawInitStage2 IQH switch Twin Fail.\n");
                        ret = MFALSE;
                    }
                }else{// MFALSE == lResCfgOutParam.isTwin
                    PIPE_WRN("ThreeRawInitStage2 IQH get no Twin.\n");
                    this->SinglePath(lResCfgOutParam);
                }
            }else{// MFALSE == this->m_pCamIOpipe->m_pResMgr->Res_Meter(lResCfgInParam,lResCfgOutParam)
                 PIPE_ERR("ThreeRawInitStage2 IQH Res_Meter Fail.\n");
                 ret = MFALSE;
            }
            break;
        }
        case ThreeRawTwinReady: // Twin on Ready
        {
            MBOOL retDFSSetLv = MTRUE;
            lResCfgOutParam.clk_level = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_GetCurLv();// Must add before Res_Meter
            if(MTRUE != this->m_pCamIOpipe->m_pResMgr->Res_Meter(this->m_cfgInParam,lResCfgOutParam)){
                 PIPE_ERR("ThreeRawTwinReady IQH Init Res_Meter Fail.\n");
                 ret = MFALSE;
            }

            // before/after start pipeline
            if(op_start == this->m_pCamIOpipe->m_FSM){
                retDFSSetLv = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level);
            }else{
                retDFSSetLv = this->m_pCamIOpipe->m_cam_dfs->CAM_DFS_SetLv(lResCfgOutParam.clk_level,MTRUE);
            }

            if(NULL != this->m_CAMIO_CB){
                this->m_CAMIO_CB(this->m_hwModule);
                this->m_ThreeRawStage = NoThreeRaw;
                if(MFALSE == this->TwinPath(this->m_resCfgOutParam)){
                    PIPE_ERR("ThreeRawTwinReady IQH Twin Fail.\n");
                    ret = MFALSE;
                }
            }else{
                PIPE_ERR("ThreeRawTwinReady IQH CAMIO_CB equal NULL.\n");
                ret = MFALSE;
            }

            break;
        }
        default:
            PIPE_ERR("ThreeRaw IQH states error.\n");
            ret = MFALSE;
            break;
    }
    return ret;
}


NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM CamIO_PathCtrl::capibility_Map(const ISP_HW_MODULE hwModule)
{
    NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM lModule;
    switch(hwModule){
        case CAM_A:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_A;
            break;
        case CAM_B:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_B;
            break;
        case CAM_C:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C;
            break;
        default:
            lModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_MAX;
            break;
    }
    return lModule;
};


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
            return _lmvo_;
            break;
        case EPortIndex_RSSO:
            return _rsso_;
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
MUINT32 CamIOPipe::CAMIO_STT::QueryPort(MUINT32 PortID)
{
    return (MUINT32)this->PortID_MAP(PortID);
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
    ST_BUF_INFO     headerImageInfo;
    MUINT           PlaneMax = 0;

    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        return -1;
    }

    if(nBurst!= rQBufInfo.vBufInfo.size()){
        PIPE_ERR("dma:0x%x:enque buf number is mismatched with subsample (0x%zx_0x%x)\n",dmaChannel,\
            rQBufInfo.vBufInfo.size(),nBurst);
        return -1;
    }
    for(MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++){
        stISP_BUF_INFO  bufInfo;

        //repalce
        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            PIPE_ERR("replace function are not supported in CAMIO_STT\n");
            return -1;
        }

        headerImageInfo.image.mem_info.pa_addr   = rQBufInfo.vBufInfo[i].u4BufPA[ePlane_1st];
        headerImageInfo.image.mem_info.va_addr   = rQBufInfo.vBufInfo[i].u4BufVA[ePlane_1st];
        headerImageInfo.image.mem_info.size      = rQBufInfo.vBufInfo[i].u4BufSize[ePlane_1st];
        headerImageInfo.image.mem_info.memID     = rQBufInfo.vBufInfo[i].memID[ePlane_1st];
        headerImageInfo.image.mem_info.bufSecu   = rQBufInfo.vBufInfo[i].bufSecu[ePlane_1st];
        headerImageInfo.image.mem_info.bufCohe   = rQBufInfo.vBufInfo[i].bufCohe[ePlane_1st];

        //header
        headerImageInfo.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA[ePlane_1st];
        headerImageInfo.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA[ePlane_1st];
        headerImageInfo.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize[ePlane_1st];
        headerImageInfo.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID[ePlane_1st];
        headerImageInfo.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu[ePlane_1st];
        headerImageInfo.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe[ePlane_1st];

        bufInfo.u_op.enque.push_back(headerImageInfo);

        if(!bufInfo.u_op.enque.empty()){
            PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%08zx),(PA:0x%08zx),(FH_VA:0x%08zx)\n",\
            i,\
            portID.index, bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.memID,\
            bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.va_addr,\
            bufInfo.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr,\
            bufInfo.u_op.enque.at(ePlane_1st).header.va_addr);
        }else{
            PIPE_ERR("CAMIO_STT bufInfo.u_op.enque empty\n");
            return -1;
        }

        //
        buf_L.push_back(bufInfo);
    }

    //
    if ( 0 != this->pTwinMgr->enqueueBuf( dmaChannel, buf_L) ) {
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

E_BUF_STATUS CamIOPipe::CAMIO_STT::dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo,MUINT32 nBurst, CAM_STATE_NOTIFY *pNotify)
{
    MUINT32 dmaChannel = 0;

    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        return eBuf_Fail;
    }

    //clear remained data in container
    rQBufInfo.vBufInfo.clear();

    //
    switch(this->pTwinMgr->dequeueBuf(dmaChannel, rQBufInfo.vBufInfo, pNotify)) {
        case -1:
            PIPE_ERR("ERROR:dma[%d] dequeOutBuf fail\n", dmaChannel);
            return eBuf_Fail;
            break;
        case 1:
            PIPE_DBG("deq dma[%d] \n", dmaChannel);
            return eBuf_stopped;
            break;
        case 2:
            PIPE_DBG("deq dma[%d] \n", dmaChannel);
            return eBuf_suspending;
            break;
        default:
            if(rQBufInfo.vBufInfo.size() != nBurst){
                PIPE_ERR("dequeout data length is mismatch\n");
                return eBuf_Fail;
            }
            break;
    }


    return  eBuf_Pass;
}

CamIOPipe_cfgFrame::CamIOPipe_cfgFrame(MVOID* pCamIO)
{
    _this = (CamIOPipe*)pCamIO;
}

void CamIOPipe_cfgFrame::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
    V_CAM_THRU_PUT* pIn = (V_CAM_THRU_PUT*)pInput;
    MINT32  clk;
    pOutput;

    if(_this == NULL || _this->m_pResMgr == NULL){
        BASE_LOG_ERR("code flow error\n");
        return;
    }


    clk = _this->m_pResMgr->Res_Recursive(pIn);
    if(clk >= 0){
        //before/after start pipeline
        if(_this->m_FSM == op_start){
            _this->m_cam_dfs->CAM_DFS_SetLv(clk);
        }else{
            _this->m_cam_dfs->CAM_DFS_SetLv(clk,MTRUE);
        }
    }
}

};  //namespace NSIspio
};  //namespace NSImageio
