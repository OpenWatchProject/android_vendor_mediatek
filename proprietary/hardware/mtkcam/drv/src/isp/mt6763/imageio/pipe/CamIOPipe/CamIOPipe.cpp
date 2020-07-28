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


/*******************************************************************************
* LOCAL PRIVATE FUNCTION
********************************************************************************/
//////////////////////////////////////////////////////////////////////////////
MUINT32 CamIOPipe::ResMgr::m_available = 2;    //CAM_A /CAM_B

MBOOL CamIOPipe::ResMgr::Register_Res(char Name[32])
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);

    if(this->m_occupied == MFALSE){
        if(*Name == '\0'){
            ret = MFALSE;
            PIPE_ERR("cant have NULL userName\n");
        }
        else{
            std::strncpy((char*)this->m_User,(char const*)Name, sizeof(this->m_User)-1);
            this->m_occupied = MTRUE;
            if(strcmp((char*)this->m_User,(char*)TwinMgr::ModuleName()) != 0)
                this->m_available--;
        }
    }
    else{
        ret = MFALSE;
        PIPE_ERR("already occupied by user:%s\n",this->m_User);
    }


    return ret;
}

MBOOL CamIOPipe::ResMgr::Release_Res(void)
{
    MBOOL ret = MTRUE;
    Mutex::Autolock lock(this->mLock);

    if(this->m_occupied){
        this->m_occupied = MFALSE;
        if(strcmp((char*)this->m_User,(char*)TwinMgr::ModuleName()) != 0)
            this->m_available++;
        *this->m_User = '\0';

    }
    else{
        ret = MFALSE;
        PIPE_ERR("logic error: ,should be occupied\n");
    }
    return ret;
}

char* CamIOPipe::ResMgr::Get_Res(void)
{
    Mutex::Autolock lock(this->mLock);

    if(this->m_occupied){
        if(*this->m_User == '\0'){
            PIPE_ERR("logic error: ,username should not be NULL,(%s)\n",this->m_User);
        }
    }
    return this->m_User;
}

MUINT32 CamIOPipe::ResMgr::Get_AvailNum(void)
{
    Mutex::Autolock lock(this->mLock);

    return this->m_available;
}

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

    m_FSM = op_unknown;
    m_bStartUniStreaming = MFALSE;
    m_pCmdQMgr = NULL;
    m_pCmdQMgr_AE = NULL;
    m_pHighSpeedCmdQMgr = NULL;
    m_pipe_opt = ICamIOPipe::CAMIO;
    m_DynamicRawType = MFALSE;
    m_bUpdate = MFALSE;
    m_hwModule = CAM_A;
    m_TwinMgr = NULL;
    m_cam_dfs = NULL;
    m_bBin_On = MFALSE;
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

char * CamIOPipe::Name_MAP(ISP_HW_MODULE _hwModule)
{
    char *_name;

    switch(_hwModule) {
    case CAM_A:
        _name = "CamIO_CamA";
        break;
    case CAM_B:
        _name = "CamIO_CamB";
        break;
    default:
        _name = "CamIO_UnknownXX";
        break;
    }

    return _name;
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
            if(*gCamioPipe[CAM_A].m_ResMgr.Get_Res() != '\0'){
                BASE_LOG_ERR("TG_A is occupied by user:%s\n",gCamioPipe[CAM_A].m_ResMgr.Get_Res());
            }
            else{
                if(gCamioPipe[CAM_A].m_ResMgr.Register_Res(gCamioPipe[CAM_A].Name_MAP(CAM_A)) == MTRUE){
                    gCamioPipe[CAM_A].m_hwModule = CAM_A;
                    gCamioPipe[CAM_A].m_ResMgr.m_hwModule = CAM_A;
                    gCamioPipe[CAM_A].m_PathCtrl.m_hwModule = CAM_A;
                    ptr = &gCamioPipe[CAM_A];
                }
            }
            break;
        case TG_B:
            if(*gCamioPipe[CAM_B].m_ResMgr.Get_Res() != '\0'){
                BASE_LOG_ERR("TG_B is occupied by user:%s\n",gCamioPipe[CAM_B].m_ResMgr.Get_Res());
            }
            else{
                if(gCamioPipe[CAM_B].m_ResMgr.Register_Res(gCamioPipe[CAM_B].Name_MAP(CAM_B)) == MTRUE){
                    gCamioPipe[CAM_B].m_hwModule = CAM_B;
                    gCamioPipe[CAM_B].m_ResMgr.m_hwModule = CAM_B;
                    gCamioPipe[CAM_B].m_PathCtrl.m_hwModule = CAM_B;
                    ptr = &gCamioPipe[CAM_B];
                }
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
    PIPE_INF(":E:user:%s, %s\n",this->m_ResMgr.Get_Res(), this->Name_MAP(this->m_hwModule));

    if(this->FSM_CHECK(op_unknown) == MFALSE)
        PIPE_ERR("FSM check error, destroy may be abnormal\n");

    // because user can do suspend without resume,in order to avoid false alarm error, check occupied 1st
    if(strcmp(this->m_ResMgr.Get_Res(), this->Name_MAP(this->m_hwModule)) == 0)
        this->m_ResMgr.Release_Res();
    else
        PIPE_DBG("have suspend without resume ??\n");

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
}

/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::init()
{
    PIPE_INF("(%s):E\n",this->m_ResMgr.Get_Res());
    if(*this->m_ResMgr.Get_Res() == '0'){
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

    if(this->m_CamPathPass1.init()) {
        PIPE_ERR("campath_p1 init error!\n");
        return MFALSE;
    }

    this->FSM_UPDATE(op_init);
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::uninit()
{
    PIPE_INF("(%s):E\n",this->m_ResMgr.Get_Res());

    if(this->FSM_CHECK(op_uninit) == MFALSE)
        return MFALSE;

    //mutex protection is for isp stop & uni_stop which is driven by sendcommand
    this->mRunTimeCfgLock.lock();

    if(this->m_CamPathPass1.uninit()) {
        PIPE_ERR("isp campath_p1 uninit fail\n");
        this->mRunTimeCfgLock.unlock();
        return MFALSE;
    }

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
        if(this->m_pCmdQMgr_AE->CmdQMgr_detach()){
            PIPE_ERR("CQ_AE uninit fail\n");
            return MFALSE;
        }
    }

    if(this->m_TwinMgr->getIsInit()){
        list<ISP_HW_MODULE>::iterator it;
        list<ISP_HW_MODULE> l_module;
        if(this->m_TwinMgr->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
            return MFALSE;

        for(it = l_module.begin();it!=l_module.end();it++){

            if(strcmp(gCamioPipe[*it].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) == 0){
                gCamioPipe[*it].m_ResMgr.Release_Res();
            }
            else{
                //curmodule is dynamic info, and is sync with current pipline
                PIPE_ERR("logic error: \n");
            }
        }

        //
        if(this->m_TwinMgr->uninitTwin() == MFALSE){
            PIPE_ERR("twinmgr unint fail\n");
            return MFALSE;
        }
    }

    this->m_TwinMgr->destroyInstance();

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

    if(MTRUE == this->m_TwinMgr->getIsStart()){
        this->m_TwinMgr->stopTwin();
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
            if(this->m_TwinMgr->getIsTwin() == MTRUE)
                ret = this->m_TwinMgr->recoverTwin(eCmd_path_stop);
            else
                ret = this->m_CamPathPass1.HW_recover(eCmd_path_stop);
            break;

        case SW_SUSPEND:
            ret = this->m_CamPathPass1.suspend(CamPathPass1::_CAMPATH_SUSPEND_STOPHW);
            if (ret == MFALSE) {
                PIPE_ERR("suspend stop hw fail\n");
                break;
            }

            /* Ref to phy reg due to HW/CQ already stopped after cam_path:suspend(STOP_HW) */
            twin.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_TWIN_STATUS);
            if (twin.Bits.TWIN_EN == MTRUE) {
                ret = this->m_TwinMgr->suspend();
                if (ret == MFALSE) {
                    PIPE_ERR("twin suspend fail\n");
                    break;
                }
            }

            ret = this->m_CamPathPass1.suspend(CamPathPass1::_CAMPATH_SUSPEND_FLUSH);
            if (ret == MFALSE) {
                PIPE_ERR("suspend flush fail\n");
                break;
            }

            this->m_cam_dfs->CAM_DFS_STOP();

            break;
        default:
            PIPE_ERR("unsupported:%d\n",suspendMode);
            break;
    }

    if(ret == MTRUE){
        if(gCamioPipe[this->m_hwModule].m_ResMgr.Release_Res() == MFALSE){
            PIPE_ERR("logic error\n");
            ret = MFALSE;
        }
    }
    return ret;
}
MBOOL CamIOPipe::resume(E_SUSPEND_MODE suspendMode)
{
    MBOOL ret = MTRUE;
    CAM_REG_CTL_TWIN_STATUS twin;

    if(this->FSM_CHECK(op_resume) == MFALSE)
        return MFALSE;


    if(gCamioPipe[this->m_hwModule].m_ResMgr.Register_Res(this->Name_MAP(this->m_hwModule)) == MFALSE){
        ret = MFALSE;
        PIPE_ERR("logic error,no available module for resume\n");
        goto EXIT;
    }



    switch(suspendMode){
        case HW_RST_SUSPEND:
            if(this->m_TwinMgr->getIsTwin() == MTRUE)
                ret = this->m_TwinMgr->recoverTwin(eCmd_path_restart);
            else
                ret = this->m_CamPathPass1.HW_recover(eCmd_path_restart);
            break;
        case SW_SUSPEND:
            twin.Raw = CAM_READ_REG(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_TWIN_STATUS);

            /* Ref to phy reg due to HW/CQ already stopped after cam_path:suspend(STOP_HW) */
            if (twin.Bits.TWIN_EN == MTRUE) {
                ret = this->m_TwinMgr->resume();
            }
            if(ret == MTRUE){//if twin mode fail, no need to run master cam
                ret = this->m_CamPathPass1.resume();
            }

            this->m_cam_dfs->CAM_DFS_START();

            break;
        default:
            PIPE_ERR("unsupported:%d\n",suspendMode);
            ret = MFALSE;
            break;
    }

    if(ret != MTRUE){
        if(gCamioPipe[this->m_hwModule].m_ResMgr.Release_Res() == MFALSE){
            PIPE_ERR("logic error\n");
            ret = MFALSE;
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
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, (MUINT32)rQBufInfo.vBufInfo.size());

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
        PIPE_ERR("enque buf number is mismatched with subsample (0x%x_0x%x)\n",(MUINT32)rQBufInfo.vBufInfo.size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
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
            bufInfo.Replace.image.mem_info.magicIdx  = rQBufInfo.vBufInfo[i].m_num;

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
        bufInfo.u_op.enque.image.mem_info.magicIdx  = rQBufInfo.vBufInfo[i].m_num;

        //header
        bufInfo.u_op.enque.header.pa_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufPA;
        bufInfo.u_op.enque.header.va_addr   = rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA;
        bufInfo.u_op.enque.header.size      = rQBufInfo.vBufInfo[i].Frame_Header.u4BufSize;
        bufInfo.u_op.enque.header.memID     = rQBufInfo.vBufInfo[i].Frame_Header.memID;
        bufInfo.u_op.enque.header.bufSecu   = rQBufInfo.vBufInfo[i].Frame_Header.bufSecu;
        bufInfo.u_op.enque.header.bufCohe   = rQBufInfo.vBufInfo[i].Frame_Header.bufCohe;

        if(rQBufInfo.vBufInfo[i].replace.bReplace == MTRUE){
            PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%" PRIXPTR "),(PA:0x%" PRIXPTR "),(FH_VA:0x%" PRIXPTR "). exbufInfo,(MEMID:%d),(VA:0x%" PRIXPTR "),(PA:0x%" PRIXPTR "),(FH_VA:0x%" PRIXPTR ")\n",\
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
            PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%" PRIXPTR "),(PA:0x%" PRIXPTR "),(FH_VA:0x%" PRIXPTR ")\n",\
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
    if(MTRUE == this->m_TwinMgr->getIsTwin()){
        if ( 0 != this->m_TwinMgr->enqueTwin( dmaChannel, buf_L) ) {
            PIPE_ERR("ERROR:DMA[%d] enqueOutBuf fail\n",dmaChannel);
            return -1;
        }
    }
    else{
        if ( 0 != this->m_CamPathPass1.enqueueBuf( dmaChannel, buf_L) ) {
            PIPE_ERR("ERROR:DMA[%d] enqueOutBuf fail\n",dmaChannel);
            return -1;
        }
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

    //
    if(MTRUE == this->m_TwinMgr->getIsTwin(MTRUE)){
        switch(this->m_TwinMgr->dequeTwin( dmaChannel,rQBufInfo.vBufInfo, pNotify)){
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
                break;
        }
    }
    else{
        switch(this->m_CamPathPass1.dequeueBuf( dmaChannel,rQBufInfo.vBufInfo,u4TimeoutMs, pNotify)){
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
    }
    this->m_TwinMgr->popTwinRst(dmaChannel);

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
        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%" PRIXPTR "),(PA:0x%" PRIXPTR "),(FH_VA:0x%" PRIXPTR ")\n",\
            i,\
            portID.index, rQBufInfo.vBufInfo[i].memID[0],\
            rQBufInfo.vBufInfo[i].u4BufVA[0],\
            rQBufInfo.vBufInfo[i].u4BufPA[0],\
            rQBufInfo.vBufInfo[i].Frame_Header.u4BufVA);
    }

    //
    //PIPE_DBG("X ");
    this->FSM_UPDATE(op_cmd);
    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL CamIOPipe::configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts,CAMIO_Func func)
{
    MBOOL   bPureRaw = MFALSE;
    MBOOL   bPak = MTRUE;
    MBOOL   bVBN = MFALSE;

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
    MUINT32 dma_int_en = (
                        FLKO_DONE_EN_ | \
                        AFO_DONE_EN_  | \
                        AAO_DONE_EN_  | \
                        PDO_DONE_EN_  | \
                        PSO_DONE_EN_);

    // twin_mgr parameters: to control frontal binning, mux pixel mode, and twin enable/disable
    TWIN_MGR_PATH_CFG_OUT_PARAM cfgOutParam;

    //
    if(this->FSM_CHECK(op_cfg) == MFALSE)
        return MFALSE;

    if ( (0 == vOutPorts.size()) || (vInPorts.size() != 1)) {
        PIPE_ERR("inport:0x%x || outport:0x%x size err\n", (MUINT32)vInPorts.size(), (MUINT32)vOutPorts.size());
        return MFALSE;
    }

    //
    //ON/OFF dynamic twin , only supported in bianco/vinson
    if(func.Bits.DynamicTwin){
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

        PIPE_INF("P1 vInPorts_%d:fmt(0x%x),PM(%d),w/h(%d_%d),crop(%d_%d_%d_%d),tg_idx(%d),dir(%d),fps(%d),timeclk(0x%x),subsample(%d),bin_off(%d),dat_pat(%d),sen_num(%d)\n",\
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
                func.Bits.DATA_PATTERN,\
                func.Bits.SensorNum);

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

                if (this->getOutPxlByteNFmt(0, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo, (MINT32*)&imgo_fmt ) == 0) {

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
                }
                break;
            case EPortIndex_RRZO:
                idx_rrzo = i;

                if((vOutPorts[i]->eImgFmt == eImgFmt_UFEO_BAYER10)||(vOutPorts[i]->eImgFmt == eImgFmt_UFEO_BAYER12)){
                    en_p1 |= RRZ_EN_;
                }
                else{
                    en_p1 |= (RRZ_EN_|PAKG_EN_);
                }
                if (this->getOutPxlByteNFmt(1, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_rrzo, (MINT32*)&rrzo_fmt) == 0) {
                    if(1 == vOutPorts[i]->u4PureRaw){
                        PIPE_ERR("this dmao support no pure output,always processed raw\n");
                    }

                    this->m_camPass1Param.bypass_rrzo = MFALSE;
                }
                break;
            case EPortIndex_UFEO:
                idx_ufeo = i;
                en_p1 |= UFEG_EN_;//UFO
                if (this->getOutPxlByteNFmt(1, vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_rrzo, (MINT32*)&rrzo_fmt) == 0) {
                    this->m_camPass1Param.bypass_ufeo = MFALSE;
                }
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
    else{   //cfg cq1 for ae smoothing
        if( (this->m_pCmdQMgr_AE = DupCmdQMgr::CmdQMgr_attach(vInPorts[idx_in_src]->tgFps,\
            func.Bits.SUBSAMPLE,this->m_hwModule,ISP_DRV_CQ_THRE1)) == NULL){
            PIPE_ERR("CQ 1 init fail\n");
            this->m_pCmdQMgr_AE = NULL;
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
        MUINT32 availNumForTwin = 0;
        if((this->m_cam_dfs = CAM_DFS_Mgr::createInstance(LOG_TAG, this->m_hwModule)) == 0)
            PIPE_ERR("cam_dfs createInstance fail");

        // Prepare input param to TwinMgr once if twin is needed after stream-on.
        this->m_PathCtrl.m_cfgInParam.SrcImgFmt = InSrcFmt;
        this->m_PathCtrl.m_cfgInParam.bypass_tg = this->m_camPass1Param.bypass_tg;
        switch(this->m_camPass1Param.m_src_pixmode){
            case CAM_TG_CTRL::_1_pix_:
                this->m_PathCtrl.m_cfgInParam.pix_mode_tg = _1_pix_;
                break;
            case CAM_TG_CTRL::_2_pix_:
                this->m_PathCtrl.m_cfgInParam.pix_mode_tg = _2_pix_;
                break;
            case CAM_TG_CTRL::_4_pix_:
                this->m_PathCtrl.m_cfgInParam.pix_mode_tg = _4_pix_;
                break;
            default:
                PIPE_ERR("unsupported pix mode_%d\n",this->m_camPass1Param.m_src_pixmode);
                return MFALSE;
                break;
        }
        this->m_PathCtrl.m_cfgInParam.tg_crop_w = this->m_camPass1Param.m_src_size.w;
        this->m_PathCtrl.m_cfgInParam.tg_crop_h = this->m_camPass1Param.m_src_size.h;
        this->m_PathCtrl.m_cfgInParam.tg_fps = vInPorts[idx_in_src]->tgFps;
        if(this->m_PathCtrl.m_cfgInParam.tg_fps < 1){
            this->m_PathCtrl.m_cfgInParam.tg_fps++;
            PIPE_WRN("org fps from sensor:%d < 0.11fps\n",vInPorts[idx_in_src]->tgFps);
        }
        this->m_PathCtrl.m_cfgInParam.bypass_rrzo = this->m_camPass1Param.bypass_rrzo;
        this->m_PathCtrl.m_cfgInParam.rrz_out_w = (this->m_camPass1Param.bypass_rrzo == MFALSE) ? \
                                    vOutPorts[idx_rrzo]->u4ImgWidth : 0;
        //
        this->m_PathCtrl.m_cfgInParam.bypass_imgo = this->m_camPass1Param.bypass_imgo;
        //
        this->m_PathCtrl.m_cfgInParam.offBin = func.Bits.bin_off;
        switch(func.Bits.DATA_PATTERN){
            case ECamio_Dual_pix:
                this->m_PathCtrl.m_cfgInParam.bDbn = MTRUE;
                this->m_PathCtrl.m_cfgInParam.datPat = E_DUAL_PIX;
                this->m_camPass1Param.m_data_pat = Cam_path_sel::_dual_pix_;
                break;
            case ECamio_QuadCode:
                this->m_PathCtrl.m_cfgInParam.bDbn = MTRUE;
                this->m_PathCtrl.m_cfgInParam.datPat = E_QUAD_CODE;
                this->m_camPass1Param.m_data_pat = Cam_path_sel::_quad_code_;
                bVBN = MTRUE;
                break;
            case ECamio_Normal:
            default:
                this->m_PathCtrl.m_cfgInParam.bDbn = MFALSE;
                this->m_PathCtrl.m_cfgInParam.datPat = E_NORMAL;
                this->m_camPass1Param.m_data_pat = Cam_path_sel::_normal_;
                break;
        }
        //
        memcpy((void*)&this->m_PathCtrl.m_Src,(void*)static_cast< NSImageio::NSIspio::PortID *> ((PortInfo *) vInPorts[idx_in_src]),sizeof(NSImageio::NSIspio::PortID));

        //
        switch(func.Bits.SensorNum){
            case ECamio_1_SEN:
                availNumForTwin = this->m_ResMgr.Get_AvailNum();
                break;
            case ECamio_2_SEN:
                availNumForTwin = 0;
                break;
            default:
                PIPE_ERR("max input sensor is ECamio_2_SEN. (%d)\n",func.Bits.SensorNum);
                return MFALSE;
                break;
        }

        //
        if(func.Bits.bin_off == MTRUE)
            PIPE_INF("Frontal binning is forced to disable\n");

        if (MFALSE == this->m_TwinMgr->initPath()) {
            PIPE_ERR("TwinMgr initPath() error!!!!!!\n");
            return MFALSE;
        }

        //
        this->m_cam_dfs->m_pIspDrv = (IspDrvVir*)this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0];
        this->m_PathCtrl.m_cfgInParam.vClk.clear();
        this->m_cam_dfs->CAM_DFS_INIT(&this->m_PathCtrl.m_cfgInParam.vClk);
        cfgOutParam.clk_level = this->m_cam_dfs->CAM_DFS_GetCurLv();

        // Get configPath info via TwinMgr
        if(MFALSE == this->m_TwinMgr->configPath(this->m_PathCtrl.m_cfgInParam, cfgOutParam, availNumForTwin)) {
            PIPE_ERR("TwinMgt configPath() error!!!!!!\n");
            return MFALSE;
        }
        if((cfgOutParam.isTwin == MTRUE) && (this->m_hwModule == CAM_B)) {
            PIPE_ERR("BA twin is not supported.\n");
            return MFALSE;
        }
        //
        this->m_cam_dfs->CAM_DFS_SetLv(cfgOutParam.clk_level,MTRUE);


        // Store cfgOutParam to CamIOPipe
        // 1. pixel mode of mux
        //which is used at updatexmxo(), need be invoked after config()
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXI = cfgOutParam.xmxo.pix_mode_dmxi;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_DMXO = cfgOutParam.xmxo.pix_mode_dmxo;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_BMXO = cfgOutParam.xmxo.pix_mode_bmxo;
        this->m_camPass1Param.m_top_ctl.FMT_SEL.Bits.PIX_BUS_RMXO = cfgOutParam.xmxo.pix_mode_rmxo;
        //
        this->m_bBin_On = cfgOutParam.bin_en; //this bBin_On should be fixed under non-Dynamic-binning.

        // Store cfgOutParam to CamIOPipe
        // 1. pixel mode of mux
        //which is used at updatexmxo(), need be invoked after config()


        // 2. frontal binning
        if(this->m_PathCtrl.m_cfgInParam.bDbn == MTRUE)
            en_p1 |= DBN_EN_;

        if(cfgOutParam.bin_en== MTRUE)
            en_p1 |= BIN_EN_;

        // When frontal binning is enable, set RAW type to be "PureRaw"!!!
        if ((cfgOutParam.bin_en== MTRUE) || (this->m_camPass1Param.m_data_pat == Cam_path_sel::_quad_code_)) {
            this->m_camPass1Param.m_NonScale_RawType = CamPathPass1Parameter::_PURE_RAW;
            bPureRaw = MTRUE;
            this->m_DynamicRawType = MFALSE; // Cannot change raw type in dynamic
            PIPE_INF("Frontal binning is enable or QuadCode path !! bin(%d) path(%d), Set Raw type to PureRaw!!!\n",
                cfgOutParam.bin_en, this->m_camPass1Param.m_data_pat);
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
    {
        Cam_path_sel* ptr = new Cam_path_sel(&this->m_camPass1Param);
        switch(func.Bits.DATA_PATTERN){
            case ECamio_Dual_pix:
                ptr->Path_sel(bPureRaw,bPak,this->m_camPass1Param.bypass_tg,Cam_path_sel::_dual_pix_);
                break;
            case ECamio_QuadCode:
                ptr->Path_sel(bPureRaw,bPak,this->m_camPass1Param.bypass_tg,Cam_path_sel::_quad_code_);
                break;
            case ECamio_Normal:
            default:
                ptr->Path_sel(bPureRaw,bPak,this->m_camPass1Param.bypass_tg,Cam_path_sel::_normal_);
                break;
        }
        delete ptr;
    }


    // If twin is enable:
    // 1. Occupied another CamIOPipe hwModule for TwinMgr
    // 2. Cfg cmdQ for another CamIOPipe hwModule
    if(this->m_TwinMgr->getIsTwin() == MTRUE)
    {
        list<ISP_HW_MODULE>::iterator it;
        list<MUINT32> l_channel;
        list<ISP_HW_MODULE> l_module;
        if(this->m_TwinMgr->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
            return MFALSE;

        l_channel.clear();

        if(this->m_camPass1Param.bypass_rrzo == MFALSE)
            l_channel.push_back(_rrzo_);
        if(this->m_camPass1Param.bypass_imgo == MFALSE)
            l_channel.push_back(_imgo_);

        //
        if(this->m_TwinMgr->initTwin(this->m_pCmdQMgr, (const NSImageio::NSIspio::PortID*)vInPorts[idx_in_src],func.Bits.SUBSAMPLE,&l_channel) == MFALSE){
            PIPE_ERR("twin init fail\n");
            return MFALSE;
        }

        if(this->m_TwinMgr->configTwin() == MFALSE){
            PIPE_ERR("twin config fail\n");
            return MFALSE;
        }

        //
        for(it = l_module.begin();it!=l_module.end();it++){
            if(*gCamioPipe[*it].m_ResMgr.Get_Res() == '\0')
            {
                gCamioPipe[*it].m_ResMgr.Register_Res(TwinMgr::ModuleName());
                gCamioPipe[*it].m_PathCtrl.m_hwModule = gCamioPipe[*it].m_ResMgr.m_hwModule = gCamioPipe[*it].m_hwModule = *it;

                memcpy((void*)&gCamioPipe[*it].m_PathCtrl.m_Src,(void*)static_cast< NSImageio::NSIspio::PortID *> ((PortInfo *) vInPorts[idx_in_src]),sizeof(NSImageio::NSIspio::PortID));
                PIPE_INF("Twin enable!! Occupy CamIoPipe: module(%d) by %s.", *it,gCamioPipe[*it].m_ResMgr.Get_Res());
            }
            else{
                PIPE_ERR("twin init fail,module(%d) is occupied by user:%s\n", *it, \
                    gCamioPipe[*it].m_ResMgr.Get_Res());
                return MFALSE;
            }
        }
    }


    //func_en , default function
    this->m_camPass1Param.m_top_ctl.FUNC_EN.Raw     = en_p1;
    this->m_camPass1Param.m_top_ctl.bVBN            = bVBN;

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
        PIPE_INF("acquired offset:%d size:%d", rlb_offset, vOutPorts[idx_rrzo]->u4ImgWidth);

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

    if (this->m_camPass1Param.bypass_ispRawPipe == MFALSE) {
        //update xmx setting
        if(this->m_CamPathPass1.updateXMXO(&cfgOutParam.xmxo) == MFALSE){
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
        case _ufeo_:
        case _lcso_:
        case _eiso_:
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

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%" PRIXPTR ",0x%" PRIXPTR ",0x%" PRIXPTR ")", gettid(), cmd, arg1, arg2, arg3);

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
        case EPIPECmd_AE_SMOOTH:
            //user cq1 to support OB only
            //Arg1: ob_gain
            {
                ISPIO_REG_CFG reg[4];
                list<ISPIO_REG_CFG> input;
                for(MUINT32 i=0;i<4;i++){
                    reg[i].Addr = 0x05f0 + i*0x4;
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
                PIPE_ERR("out of range[0x%" PRIXPTR "_0x%x].can't send data at this idx\n",arg1,this->m_camPass1Param.m_subSample);
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

                if ((0 == arg1)||(0 == arg2)) {
                    PIPE_ERR("SET_RRZ NULL param(0x%" PRIXPTR "_0x%" PRIXPTR ")!\n",arg1,arg2);
                    return MFALSE;
                }

                pInput_L = (list<STImgCrop>*)arg1;
                pInput2_L = (list<STImgResize>*)arg2;

                if( (pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) || (pInput2_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ())){
                    PIPE_ERR("rrz data length mismatch:0x%x_0x%x_0x%x\n",(MUINT32)pInput_L->size(),(MUINT32)pInput2_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
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
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_w(%ld))\n", \
                            (it->w+it->x), \
                            this->m_camPass1Param.m_src_size.w);
                        return MFALSE;
                    }
                    if ( (it->h+it->y) > this->m_camPass1Param.m_src_size.h) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_h(%ld))\n", \
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
                    if(MFALSE == this->m_TwinMgr->runTwinDrv()){
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
                    PIPE_ERR("SET_IMGO NULL param!!");
                    return MFALSE;
                }

                pInput_L = (list<STImgCrop>*)arg1;
                pInput2_L = (list<MUINT32>*)arg2;


                if( (pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) || (pInput2_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()) ){
                    PIPE_ERR("imgo data length mismatch:0x%x_0x%x_0x%x\n",(MUINT32)pInput_L->size(),(MUINT32)pInput2_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
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
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_w(%ld))\n",\
                            (it->x + it->w), this->m_camPass1Param.m_src_size.w);
                        return MFALSE;
                    }
                    if ( (it->y + it->h) > this->m_camPass1Param.m_src_size.h) {
                        PIPE_ERR("ERROR: crop > input (crop(%d)>input_h(%ld))\n", \
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

                                switch (this->m_camPass1Param.m_data_pat) {
                                case Cam_path_sel::_dual_pix_:
                                case Cam_path_sel::_quad_code_:
                                    PIPE_INF("DualPix/QuadCode(%d) force pure raw path\n", (int)this->m_camPass1Param.m_data_pat);
                                    break;
                                case Cam_path_sel::_normal_:
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
                    PIPE_ERR("SET_UFEO NULL param!!");
                    return MFALSE;
                }

                pInput_L = (list<STImgResize>*)arg1;

                if( pInput_L->size() != this->m_pCmdQMgr->CmdQMgr_GetBurstQ()){
                    PIPE_ERR("ufeo data length mismatch:0x%x_0x%x\n",(MUINT32)pInput_L->size(),this->m_pCmdQMgr->CmdQMgr_GetBurstQ());
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
                    bin += (CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_DMA_EN,VBN_EN));
                    size_v = size_v>>bin;

                    //qbin1
                    pixMode = CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN1_MODE, QBN_ACC);
                    *(MUINT32*)arg1  = size_h >> pixMode;
                    *(MUINT32*)arg2  = size_v;
                    //qbin4, pix mode must be the same with qbin1
                    if(pixMode != CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN4_MODE, QBN_ACC)){
                        PIPE_ERR("QBIN1 pix mode is different from QBIN4:0x%x_0x%x\n",pixMode\
                            ,CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_QBN4_MODE, QBN_ACC));
                    }
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
                    bin += (CAM_READ_BITS(this->m_pCmdQMgr->CmdQMgr_GetCurCycleObj(0)[0]->getPhyObj(),CAM_CTL_DMA_EN,VBN_EN));
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
                        PIPE_ERR("not supported:%d\n",arg1);
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
        case EPIPECmd_SET_REGDUMP_CBFP:
            if(this->m_FSM != op_cfg){
                PIPE_ERR("REGDUMP_CB can only be hooked under cfg stage, current stage :0x%x\n",this->m_FSM);
                ret = 1;
            }
            if(this->m_CamPathPass1.P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1) == MFALSE){
                return MFALSE;
            }
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
    #if 0//in p1, dmao format use hw-auto
    a_dma.format = portInfo->eImgFmt;
    #else//this is for  ispio_utility.h query behavior  at cam_path_pass1.cpp
    a_dma.lIspColorfmt = portInfo->eImgFmt;
    #endif
    //dma port capbility
    a_dma.capbility=portInfo->capbility;
    //input xsize unit:byte
    tmp = CamInfo.GetCapibility(
            portInfo->index,
            NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE,
            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo((EImageFormat)portInfo->eImgFmt, a_dma.size.w,NSCam::NSIoPipe::NSCamIOPipe::_UNKNOWN_PIX_MODE),
            rst, E_CAM_UNKNOWNN);
    if(tmp == MFALSE) {
        PIPE_ERR("CamInfo.GetCapibility error");
    }
    a_dma.size.xsize        =  rst.stride_byte;
    //
    //
    if ( a_dma.size.stride<a_dma.size.w &&  planeNum==ESTRIDE_1ST_PLANE) {
        PIPE_ERR("[Error]:stride size(%ld) < image width(%ld) byte size",a_dma.size.stride,a_dma.size.w);
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

    PIPE_DBG("w(%ld),h(%ld),stride(%ld),xsize(%ld),crop(%d,%d,%ld,%ld)_f(0x%x,0x%x),ofst_addr(0x%x),pb((0x%x)(+<<2)),va(0x%" PRIXPTR "),pa(0x%" PRIXPTR "),pixel_Byte(%d)",\
                                                        a_dma.size.w,a_dma.size.h,a_dma.size.stride, \
                                                        a_dma.size.xsize,a_dma.crop.x,a_dma.crop.y,a_dma.crop.w,a_dma.crop.h,\
                                                        a_dma.crop.floatX,a_dma.crop.floatY, \
                                                        a_dma.memBuf.ofst_addr,a_dma.pixel_byte,\
                                                        a_dma.memBuf.base_vAddr, a_dma.memBuf.base_pAddr,\
                                                        pixel_Byte);
    PIPE_DBG("eImgFmt(0x%x),format_en(%d),format(%d),bus_size_en(%d),bus_size(%d)",portInfo->eImgFmt,a_dma.format_en,a_dma.format,a_dma.bus_size_en,a_dma.bus_size);
    //
    return MTRUE;
}


MBOOL CamIOPipe::configFrame(E_FRM_EVENT event,CAMIO_Func func)
{
    MBOOL ret = MTRUE;
    PIPE_DBG("event:%d\n",event);

    this->m_cam_dfs->CAM_DFS_Vsync();//perframe request from DFS

    switch(event){
        case CAMIO_DROP:
            //twin status change
            if(this->m_PathCtrl.m_isTwin == MTRUE){
                ret = this->m_PathCtrl.TwinPath();
            }
            else{
                ret = this->m_PathCtrl.SinglePath();
            }

            break;
        case CAMIO_D_Twin:
            {
                TWIN_MGR_PATH_CFG_OUT_PARAM cfgOutParam;
                MUINT32 availNumForTwin = 0;
                //
                switch(func.Bits.SensorNum){
                    case ECamio_1_SEN:
                        availNumForTwin = this->m_ResMgr.Get_AvailNum();
                        break;
                    case ECamio_2_SEN:
                        availNumForTwin = 0;
                        break;
                    default:
                        PIPE_ERR("max input sensor is ECamio_2_SEN. (%d)\n",func.Bits.SensorNum);
                        return MFALSE;
                        break;
                }

                //
                cfgOutParam.clk_level = this->m_cam_dfs->CAM_DFS_GetCurLv();

                //
                if(MFALSE == this->m_TwinMgr->configPath(this->m_PathCtrl.m_cfgInParam, cfgOutParam, availNumForTwin)) {
                    PIPE_ERR("TwinMgt configPath() error!!!!!!\n");
                    return MFALSE;
                }
                //keep this record if next frame is drop frame
                this->m_PathCtrl.m_isTwin = cfgOutParam.isTwin;

                //
                if(this->m_bBin_On != cfgOutParam.bin_en){
                    PIPE_ERR("Dynamic binning is not supported\n");
                    return MFALSE;
                }

                //before/after start pipeline
                if(this->m_FSM == op_start)
                    this->m_cam_dfs->CAM_DFS_SetLv(cfgOutParam.clk_level);
                else
                    this->m_cam_dfs->CAM_DFS_SetLv(cfgOutParam.clk_level,MTRUE);

                //
                if(cfgOutParam.isTwin == MTRUE){
                    ret = this->m_PathCtrl.TwinPath();
                }
                else{
                    ret = this->m_PathCtrl.SinglePath();
                }

                if (this->m_camPass1Param.bypass_ispRawPipe == MFALSE) {
                    this->m_CamPathPass1.updateXMXO(&cfgOutParam.xmxo);
                }

            }
            break;
    }

    return ret;
}

MBOOL CamIO_PathCtrl::TwinPath(void)
{
    MBOOL ret = MTRUE;
    CamIOPipe* _this = NULL;
    list<ISP_HW_MODULE>::iterator it;
    list<MUINT32> l_channel;
    list<ISP_HW_MODULE> l_module;
    _this = &gCamioPipe[this->m_hwModule];

    if(_this->m_TwinMgr->sendCommand(TWIN_CMD_GET_CUR_TWINMODULE,(MINTPTR)&l_module,0,0) == MFALSE)
        return MFALSE;

    //
    for(it = l_module.begin();it!=l_module.end();it++){
        if(*gCamioPipe[*it].m_ResMgr.Get_Res() != '\0')
        {
            if(strcmp(gCamioPipe[*it].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) != 0){
                //cam is occupied ,not occupied by twin, can't lunch twin.
                PIPE_ERR("CAM_%d is occupied by %s,lunch fail\n",*it,gCamioPipe[*it].m_ResMgr.Get_Res());
                ret = MFALSE;
            }
            else{//cam is occupied by twin , do nothing
            }
        }
        else{
            TwinMgr::E_TWIN_STATUS state;
            ret = _this->m_TwinMgr->sendCommand(TWIN_CMD_GET_TWIN_STATE,(MINTPTR)&state,0,0);
            switch(state){
                case TwinMgr::E_SUSPEND://single path 2 twin path with pipline had ran in twin path
                    ret = _this->m_TwinMgr->resumeTwin();
                    break;
                case TwinMgr::E_INIT:   //single path 2 twin path with pipline had not ran in twin path
                    //start here before 1st updateTwin , to avoid fsm error when using updateTwin.
                    ret = _this->m_TwinMgr->startTwin();
                    break;
                case TwinMgr::E_NOTWIN:

                    l_channel.clear();

                    if(_this->m_camPass1Param.bypass_rrzo == MFALSE)
                        l_channel.push_back(_rrzo_);
                    if(_this->m_camPass1Param.bypass_imgo == MFALSE)
                        l_channel.push_back(_imgo_);

                    //
                    if(_this->m_TwinMgr->initTwin(_this->m_pCmdQMgr,(const NSImageio::NSIspio::PortID*)&_this->m_PathCtrl.m_Src,_this->m_camPass1Param.m_subSample,&l_channel) == MFALSE){
                        PIPE_ERR("twin init fail\n");
                        ret = MFALSE;
                    }

                    if(_this->m_TwinMgr->configTwin() == MFALSE){
                        PIPE_ERR("twin config fail\n");
                        ret = MFALSE;
                    }

                    //start here before 1st updateTwin , to avoid fsm error when using updateTwin.
                    ret = _this->m_TwinMgr->startTwin();
                    break;
                case TwinMgr::E_START:
                    PIPE_ERR("logic error: twin is already running without user\n");
                    ret = MFALSE;
                    break;
                default:
                    ret = MFALSE;
                    PIPE_ERR("twin status error:%d\n",state);
                    break;
            }


            if(ret != MFALSE){
                //need to update hwModule here due to this resource is not created from create()
                gCamioPipe[*it].m_PathCtrl.m_hwModule = gCamioPipe[*it].m_ResMgr.m_hwModule = gCamioPipe[*it].m_hwModule = *it;
                gCamioPipe[*it].m_ResMgr.Register_Res(TwinMgr::ModuleName());
            }

        }
    }
    return ret;
}


MBOOL CamIO_PathCtrl::SinglePath(void)
{
    MBOOL ret = MTRUE;
    CamIOPipe* _this = NULL;
    TwinMgr::E_TWIN_STATUS state;
    _this = &gCamioPipe[this->m_hwModule];

    ret = _this->m_TwinMgr->sendCommand(TWIN_CMD_GET_TWIN_STATE,(MINTPTR)&state,0,0);
    switch(state){
        case TwinMgr::E_START://twin path 2 single path
            ret = _this->m_TwinMgr->suspendTwin(TwinMgr::E_SUS_START);

            //can't free resouce here, need to wait untial hw is suspended.
            break;
        case TwinMgr::E_SUSPEND:
            {
                MBOOL suspending = MFALSE;
                //free cam when pipeline is running at single path
                for(MUINT32 i=0;i<CAM_MAX;i++){
                    if(i != this->m_hwModule){
                        if(strcmp(gCamioPipe[i].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) != 0){
                            //do nothing, occupied by other sensor, or no user
                        }
                        else{//release cam
                            gCamioPipe[i].m_ResMgr.Release_Res();

                            suspending = MTRUE;
                        }
                    }
                }

                if(suspending){
                    ret = _this->m_TwinMgr->suspendTwin(TwinMgr::E_SUS_READY);
                }//no else case if perframe's result is kept on single path
            }
            break;
        case TwinMgr::E_NOTWIN:
            break;
        case TwinMgr::E_INIT:
            //free cam when pipeline is running at single path
            for(MUINT32 i=0;i<CAM_MAX;i++){
                if(i != this->m_hwModule){
                    if(strcmp(gCamioPipe[i].m_ResMgr.Get_Res(),TwinMgr::ModuleName()) != 0){
                        //do nothing, occupied by other sensor, or no user
                    }
                    else{//release cam
                        gCamioPipe[i].m_ResMgr.Release_Res();
                    }
                }
            }
            break;
        default:
            ret = MFALSE;
            PIPE_ERR("twin status error:%d\n",state);
            break;
    }

    return ret;
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
    stISP_BUF_INFO  bufInfo;

    //
    if( (dmaChannel = (MUINT32)this->PortID_MAP(portID.index)) == _cam_max_){
        return -1;
    }

    if(nBurst!= rQBufInfo.vBufInfo.size()){
        PIPE_ERR("dma:0x%x:enque buf number is mismatched with subsample (0x%x_0x%x)\n",dmaChannel,\
            (MUINT32)rQBufInfo.vBufInfo.size(),nBurst);
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


        PIPE_DBG("burstidx:0x%x: PortID=%d, bufInfo:(MEMID:%d),(VA:0x%" PRIXPTR "),(PA:0x%" PRIXPTR "),(FH_VA:0x%" PRIXPTR ")\n",\
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
    switch(this->pCamPath->dequeueBuf( dmaChannel,rQBufInfo.vBufInfo, 0, pNotify)){
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
};  //namespace NSIspio
};  //namespace NSImageio
