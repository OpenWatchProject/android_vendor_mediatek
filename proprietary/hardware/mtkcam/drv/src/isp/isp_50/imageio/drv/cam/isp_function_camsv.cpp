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
#define LOG_TAG "ifunc_camsv"

//open syscall
#include <mtkcam/def/PriorityDefs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>
//mutex
#include <pthread.h>
#include <utils/StrongPointer.h>


#include <cutils/properties.h>  // For property_get().

#include <ispio_pipe_ports.h>
#include "isp_function_camsv.h"
#include "Cam_Notify_datatype.h"



#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace NSImageio;
using namespace NSIspio;

DECLARE_DBG_LOG_VARIABLE(func_camsv);
//EXTERN_DBG_LOG_VARIABLE(func_cam);

// Clear previous define, use our own define.
#undef CAM_FUNC_VRB
#undef CAM_FUNC_DBG
#undef CAM_FUNC_INF
#undef CAM_FUNC_WRN
#undef CAM_FUNC_ERR
#undef CAM_FUNC_AST

#define CAM_FUNC_VRB(fmt, arg...)        do { if (func_camsv_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define CAM_FUNC_DBG(fmt, arg...)        do {\
    if (func_camsv_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    if (func_camsv_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do { if (func_camsv_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (func_camsv_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)
#define CAM_SFUNC_DBG(fmt, arg...)        do {\
        if (func_camsv_DbgLogEnable_DEBUG  ) { \
            BASE_LOG_DBG(fmt, ##arg); \
        }\
    } while(0)
#define CAM_SFUNC_INF(fmt, arg...)        do {\
        if (func_camsv_DbgLogEnable_INFO  ) { \
            BASE_LOG_INF(fmt, ##arg); \
        }\
    } while(0)
#define CAM_SFUNC_ERR(fmt, arg...)        do {\
        if (func_camsv_DbgLogEnable_ERROR  ) { \
            BASE_LOG_ERR(fmt, ##arg); \
        }\
    } while(0)


#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_camsv_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

FP_SEN SENINF_DBG::m_fp_Sen_Camsv= NULL;

/*//////////////////////////////////////////////////////////////////////////////
CAM_TG_CTRL
///////////////////////////////////////////////////////////////////////////////*/
CAMSV_TG_CTRL::CAMSV_TG_CTRL()
{
    m_PixMode = ePixMode_1;
    m_continuous = 0;
    m_SubSample = 0;
    m_pDrv = NULL;
}

MINT32 CAMSV_TG_CTRL::_config( void )
{
    CAM_FUNC_INF("CAMSV_TG_CTRL::_config+");
    CAM_FUNC_INF("xcropping:(0x%x_0x%x_0x%lx_0x%lx),is_continuous(0x%x),SubSample(0x%x),pixmode(0x%x)\n", \
                this->m_Crop.x, \
                this->m_Crop.y, \
                this->m_Crop.w, \
                this->m_Crop.h,  \
                this->m_continuous,  \
                this->m_SubSample,\
                this->m_PixMode);

    this->m_hwModule = this->m_pDrv->m_hwModule;

    //subsample
    //for vsync subsample function, need to make sure cmos_en is off.
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,CMOS_EN,0);
    CAM_FUNC_INF("****** %s %d, this->m_SubSample(%d)", __FUNCTION__, __LINE__, this->m_SubSample);
    if(this->m_SubSample){
        CAM_FUNC_INF("****** %s %d", __FUNCTION__, __LINE__);
        //sof
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,SOF_SUB_EN,1);
        //vsync
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,VS_SUB_EN,1);
        CAMSV_WRITE_REG(this->m_pDrv,CAMSV_TG_SUB_PERIOD,((this->m_SubSample<<8)|this->m_SubSample));
    }
    else{
        CAM_FUNC_INF("****** %s %d", __FUNCTION__, __LINE__);
        //sof
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,SOF_SUB_EN,0);
        //vsync
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,VS_SUB_EN,0);
        CAMSV_WRITE_REG(this->m_pDrv,CAMSV_TG_SUB_PERIOD,((this->m_SubSample<<8)|this->m_SubSample));
    }

    //timestamp
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,TIME_STP_EN,1);

    //trig mode
    CAM_FUNC_INF("****** %s %d, this->m_continuous(%d)", __FUNCTION__, __LINE__, this->m_continuous);
    if(this->m_continuous){
        CAM_FUNC_INF("****** %s %d", __FUNCTION__, __LINE__);
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_VF_CON,SINGLE_MODE,0);
    }
    else{
        CAM_FUNC_INF("****** %s %d", __FUNCTION__, __LINE__);
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_VF_CON,SINGLE_MODE,1);
    }

    //pix mode
    switch(this->m_PixMode){
        case ePixMode_1:
            CAM_FUNC_INF("****** %s %d, pixmode(%d) _1_pix_", __FUNCTION__, __LINE__, this->m_PixMode);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,0);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,0);
            break;
        case ePixMode_2:
            CAM_FUNC_INF("****** %s %d, pixmode(%d) _2_pix_", __FUNCTION__, __LINE__, this->m_PixMode);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,1);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,0);
            break;
        case ePixMode_4:
            CAM_FUNC_INF("****** %s %d, pixmode(%d) _4_pix_", __FUNCTION__, __LINE__, this->m_PixMode);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,DBL_DATA_BUS,1);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,DBL_DATA_BUS1,1);
            break;
        default:
            CAM_FUNC_ERR("- unsupported pix mode:0x%x\n",this->m_PixMode);
            break;
    }

    //cropping window
    CAM_FUNC_INF("****** %s %d, this->m_Crop.w(%ld 0x%08lx)", __FUNCTION__, __LINE__, this->m_Crop.w, this->m_Crop.w);
    if((this->m_Crop.w % 4) != 0){
        CAM_FUNC_ERR("TG cropping size need 4-alignment\n");
        return 1;
    }
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_TG_SEN_GRAB_PXL,(((this->m_Crop.w+this->m_Crop.x)<<16)|this->m_Crop.x));
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_TG_SEN_GRAB_LIN,(((this->m_Crop.h+this->m_Crop.y)<<16)|this->m_Crop.y));

    //
    CAM_FUNC_INF("CAMSV_TG_CTRL::_config-");
    return 0;
}

MINT32 CAMSV_TG_CTRL::_enable( void* pParam  )
{
    (void)pParam;

    CAM_FUNC_INF("CAM_TG_CTRL::_enable+");
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,CMOS_EN,1);  /* 0x1A050500 */
    CAM_FUNC_INF("CAM_TG_CTRL::_enable-");
    return 0;
}

MINT32 CAMSV_TG_CTRL::_disable( void* pParam )
{
    (void)pParam;

    CAM_FUNC_INF("CAM_TG_CTRL::_disable+");
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,CMOS_EN,0);
    CAM_FUNC_INF("CAM_TG_CTRL::_disable-");
    return 0;
}



/*/////////////////////////////////////////////////////////////////////////////
    CAM_TOP_CTRL
  /////////////////////////////////////////////////////////////////////////////*/

CAMSV_TOP_CTRL::CAMSV_TOP_CTRL()
{
    m_bBusy = MFALSE;
    SubSample = 0;
    m_PixMode = ePixMode_1;
    m_pDrv = NULL;
    DBG_LOG_CONFIG(imageio, func_camsv);
}

MINT32 CAMSV_TOP_CTRL::_config( void )
{
    CAM_FUNC_INF("CAMSV_TOP_CTRL::_config+");

    this->m_hwModule = this->m_pDrv->m_hwModule;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MUINT32 _loglevel = 0, i = 0;
    MUINT32 int_en = (CAMSV_INT_EN_VS1_INT_EN | \
                      CAMSV_INT_EN_TG_ERR_INT_EN | \
                      CAMSV_INT_EN_TG_GBERR_INT_EN | \
                      CAMSV_INT_EN_TG_SOF_INT_EN | \
                      CAMSV_INT_EN_PASS1_DON_INT_EN |\
                      CAMSV_INT_EN_SW_PASS1_DON_INT_EN |\
                      CAMSV_INT_EN_IMGO_ERR_INT_EN |\
                      CAMSV_INT_EN_IMGO_OVERR_INT_EN);

    //kernel log level:
    property_get("vendor.debug.isp", value, "0");
    _loglevel = atoi(value);
    if(_loglevel != 0){
        this->m_pDrv->setDeviceInfo(_SET_DBG_INT,(MUINT8*)&_loglevel);
    }

    //avoid previous scenario exit abnormally. e.g.: exit without stop();
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 _clrPort = 1;
    MUINT32 _dma[1] = {_camsv_imgo_};

    while(_clrPort--){
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
        buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
        buf_ctrl.data_ptr = NULL;
        buf_ctrl.pExtend = 0;
        if ( MTRUE != this->m_pDrv->setDeviceInfo(_SET_BUF_CTRL,(MUINT8 *) &buf_ctrl)) {
            CAM_FUNC_ERR("ERROR:clear buf ctrl fail\n");
        }
    }

    CAM_FUNC_INF("****** %s %d", __FUNCTION__, __LINE__);

    //reset
    CAM_FUNC_DBG("CAMSV TOP reset");
    CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x4); // SW_RST: 1
    CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x0); // SW_RST: 0
    CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x1); // IMGO_RST_TRIG: 1
    while((CAMSV_READ_REG(this->m_pDrv,CAMSV_SW_CTL) != 0x3) && i++ < 10000){ // Polling IMGO_RST_ST to 1
        CAM_FUNC_INF("CAMSV reseting...CAMSV_SW_CTL:0x%x\n", CAMSV_READ_REG(this->m_pDrv,CAMSV_SW_CTL));
    }
    CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x0); // IMGO_RST_TRIG: 0

    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /********************************************************************************************************************/

    //func en
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,TG_EN,1); // enable TG
    //CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,IMGO_EN,1); // enable IMGO
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,DB_EN,1); //enable double buffer

    //fmt sel
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_FMT_SEL,this->camsv_top_ctl.FMT_SEL.Raw);

    //inte en
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_INT_EN,int_en);

    //subsample p1 done
    if(this->SubSample){
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,DOWN_SAMPLE_EN,1);
    }
    else{
        CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,DOWN_SAMPLE_EN,0);
    }

    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,DOWN_SAMPLE_PERIOD,this->SubSample);


    // PAK setting
    switch(this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT){
        case SV_TG_FMT_RAW8:
            CAM_FUNC_INF("****** %s %d, TG1_FMT(%d) SV_TG_FMT_RAW8", __FUNCTION__, __LINE__, this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_EN,1);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_SEL,0);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_PAK,PAK_MODE,0);
            break;
        case SV_TG_FMT_RAW10:
            CAM_FUNC_INF("****** %s %d, TG1_FMT(%d) SV_TG_FMT_RAW10", __FUNCTION__, __LINE__, this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_EN,1);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_SEL,0);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_PAK,PAK_MODE,1);
            break;
        case SV_TG_FMT_RAW12:
            CAM_FUNC_INF("****** %s %d, TG1_FMT(%d) SV_TG_FMT_RAW12", __FUNCTION__, __LINE__, this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_EN,1);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_SEL,0);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_PAK,PAK_MODE,2);
            break;
        case SV_TG_FMT_RAW14:
            CAM_FUNC_INF("****** %s %d, TG1_FMT(%d)", __FUNCTION__, __LINE__, this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT);
            /* Fall through */
        case SV_TG_FMT_YUV422:
            CAM_FUNC_INF("****** %s %d, TG1_FMT(%d)", __FUNCTION__, __LINE__, this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT);
            /* Fall through */
        case SV_TG_FMT_JPG:
            CAM_FUNC_INF("****** %s %d, TG1_FMT(%d)", __FUNCTION__, __LINE__, this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_EN,0);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,PAK_SEL,1);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_PAK,PAK_MODE,0);
            break;
        default:
            CAM_FUNC_ERR("Unsupported TG fmt:%d\n", this->camsv_top_ctl.FMT_SEL.Bits.TG1_FMT);
    }

    //pix mode
    switch(this->m_PixMode){
        case ePixMode_1:
            CAM_FUNC_INF("****** %s %d, pixmode(%d) _1_pix_", __FUNCTION__, __LINE__, this->m_PixMode);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_PAK,PAK_DBL_MODE,0);
            break;
        case ePixMode_2:
            CAM_FUNC_INF("****** %s %d, pixmode(%d) _2_pix_", __FUNCTION__, __LINE__, this->m_PixMode);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_PAK,PAK_DBL_MODE,1);
            break;
        case ePixMode_4:
            CAM_FUNC_INF("****** %s %d, pixmode(%d) _4_pix_", __FUNCTION__, __LINE__, this->m_PixMode);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_PAK,PAK_DBL_MODE,2);
            break;
        default:
            CAM_FUNC_ERR("- unsupported pix mode:0x%x\n",this->m_PixMode);
            break;
    }

    //reset FH
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_DMA_FRAME_HEADER_EN,0);

    //special
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_DMA_RSV1,(CAMSV_READ_REG(this->m_pDrv,CAMSV_DMA_RSV1)&0x7fffffff));//bit31 for smi mask, blocking imgo output
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_DMA_RSV6,0xffffffff);

    //dma performance
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_SPECIAL_FUN_EN,CAMSV_READ_REG(this->m_pDrv,CAMSV_SPECIAL_FUN_EN)|0x61000000);

    CAM_FUNC_INF("en(0x%08x),fmtSel(0x%08x),intEn(0x%08x),SubSample(0x%x)", \
                CAMSV_READ_REG(this->m_pDrv,CAMSV_MODULE_EN), \
                this->camsv_top_ctl.FMT_SEL.Raw, \
                int_en,  \
                this->SubSample);

    CAM_FUNC_INF("CAMSV_TOP_CTRL::_config-");

    return 0;
}


MINT32 CAMSV_TOP_CTRL::_enable( void* pParam  )
{
    (void)pParam;
    ISP_CLEAR_IRQ_ST clr_irq;
    clr_irq.Status = SW_PASS1_DON_ST;
    clr_irq.UserKey = 0x0;
    clr_irq.St_type = SIGNAL_INT;
    MUINT32 done_sel = 0;
    MUINT32 _tmp;

    CAM_FUNC_INF("CAMSV_TOP_CTRL::_enable+");

    // CAMSV clock enable
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_CLK_EN,TG_DP_CK_EN,1);
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_CLK_EN,PAK_DP_CK_EN,1);
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_CLK_EN,DMA_DP_CK_EN,1);


#if 0
    //register dump before start()
    if ( MTRUE == func_camsv_DbgLogEnable_VERBOSE) {
        CAM_FUNC_DBG("DUMP CAM REGISTER BEFORE START\n");
        //dump phy
        this->m_pDrv->DumpReg(MTRUE);
    }
#endif

    //

    if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_SEN_MODE,CMOS_EN)){
        //
        this->m_pDrv->clearIrq(&clr_irq);
        //
        this->m_pDrv->setDeviceInfo(_SET_VF_ON,(MUINT8*)&this->m_hwModule);

        CAM_FUNC_INF("LOCK WAKE LOCK");
    }
    else{
        CAM_FUNC_INF("cmos_en is still off,start fail\n");
        return -1;
    }


    this->m_bBusy = MTRUE;

    CAM_FUNC_INF("CAMSV_TOP_CTRL::_enable-");

    return 0;
}

MINT32 CAMSV_TOP_CTRL::_disable( void* pParam )
{
    ISP_WAIT_IRQ_ST irq;
    irq.Clear  = ISP_IRQ_CLEAR_WAIT;
    irq.Status = VS_INT_ST;
    irq.St_type = SIGNAL_INT;
    irq.Timeout = MIN_GRPFRM_TIME_MS * 2;
    irq.UserKey = 0x0;
    MBOOL ret;
    MUINT32 regstatus;
    MUINT waitTgIdleCount = 10;
    MUINT32 _cnt=0, i = 0;
    SENINF_DBG seninf_dbg;
    MBOOL bForce = *(MBOOL *) pParam;

    CAM_FUNC_INF("CAMSV_TOP_CTRL::_disable+");

    if(bForce == MFALSE) {
        if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN) == 1){
            this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
            do{
                ++_cnt;
                ret = this->m_pDrv->waitIrq(&irq);
                if( MFALSE == ret ) {
                    CAM_FUNC_INF("wait Irq faile(%d)\n",_cnt);
                    break;
                }
                CAM_FUNC_INF("wait vsync %d time for TG idle\n",_cnt);
                regstatus = CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_INTER_ST,TG_CAM_CS);
                CAM_FUNC_INF("regstatus = 0x%08x", regstatus);
            }while(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_INTER_ST,TG_CAM_CS) != 1 && _cnt < waitTgIdleCount);
        } else {
            if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_INTER_ST,TG_CAM_CS) == 1){
                CAM_FUNC_INF("vf_en off + tg idle, no wait vsync\n");
            }
        }
    } else {
        if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN) == 1){
            this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
        }
        //reset first for forced stop
        CAM_FUNC_DBG("CAMSV TOP reset\n");
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x4); // SW_RST: 1
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x0); // SW_RST: 0
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x1); // IMGO_RST_TRIG: 1
        while((CAMSV_READ_REG(this->m_pDrv,CAMSV_SW_CTL) != 0x3) && i++ < 10000){ // Polling IMGO_RST_ST to 1
            CAM_FUNC_INF("CAMSV reseting...CAMSV_SW_CTL:0x%x\n", CAMSV_READ_REG(this->m_pDrv,CAMSV_SW_CTL));
        }
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x0); // IMGO_RST_TRIG: 0
    }


    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,DB_EN,0); //disable double buffer


    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_MODULE_EN,0x0); //disable double buffer, TG_EN, DOWN_SAMPLE, PAK_EN, IMGO_EN
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_FMT_SEL,0x0);
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_INT_EN,0x0);

    //close  fbc
    //FBC on uni will be closed at the _disable() of uni_top
    //FBC of STT pipe will be closed at STT pipe
    CAMSV_WRITE_REG(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,0x0);


    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,DB_EN,1); //enable double buffer


    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 _clrPort = 1;
    MUINT32 _dma[1] = {_camsv_imgo_};

    while(_clrPort--){
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
        buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
        buf_ctrl.data_ptr = NULL;
        buf_ctrl.pExtend = 0;
        if ( MTRUE != this->m_pDrv->setDeviceInfo(_SET_BUF_CTRL,(MUINT8 *) &buf_ctrl)) {
            CAM_FUNC_ERR("ERROR:clear buf ctrl fail\n");
        }
    }

    //reset later for non-forced stop
    if(bForce == MFALSE) {
        CAM_FUNC_DBG("CAMSV TOP reset\n");
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x4); // SW_RST: 1
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x0); // SW_RST: 0
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x1); // IMGO_RST_TRIG: 1
        while((CAMSV_READ_REG(this->m_pDrv,CAMSV_SW_CTL) != 0x3) && i++ < 10000){ // Polling IMGO_RST_ST to 1
            CAM_FUNC_INF("CAMSV reseting...CAMSV_SW_CTL:0x%x\n", CAMSV_READ_REG(this->m_pDrv,CAMSV_SW_CTL));
        }
        CAMSV_WRITE_REG(this->m_pDrv, CAMSV_SW_CTL, 0x0); // IMGO_RST_TRIG: 0
    }

    // CAMSV clock disable
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_CLK_EN,TG_DP_CK_EN,0x0);
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_CLK_EN,PAK_DP_CK_EN,0x0);
    CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_CLK_EN,DMA_DP_CK_EN,0x0);

    this->m_bBusy = MFALSE;


    //register dump after stop()
    if ( _cnt == waitTgIdleCount) {
        CAM_FUNC_ERR("DUMP CAMSV REGISTER When cannot wait TG idle\n");
        //dump phy
        this->m_pDrv->DumpReg(MTRUE);

        if(seninf_dbg.m_fp_Sen_Camsv != NULL){

        CAM_FUNC_ERR("start dump seninf info\n");
            seninf_dbg.m_fp_Sen_Camsv((MUINT32)this->m_hwModule, 0);
        }
    }

    CAM_FUNC_INF("CAMSV_TOP_CTRL::_disable-");

    return 0;
}

MBOOL CAMSV_TOP_CTRL::checkBusy(  MUINTPTR param  )
{
    (void)param;

    return this->m_bBusy;
}



/*/////////////////////////////////////////////////////////////////////////////
    DMAO_B
/////////////////////////////////////////////////////////////////////////////*/
DMAO_CAMSV_B::DMAO_CAMSV_B()
{
    Header_Addr = 0x0;
    m_pDrv = NULL;
    m_fps = 30;
    m_PixMode = ePixMode_1;
}

MBOOL DMAO_CAMSV_B::Init(void)
{

    CAM_FUNC_INF("DMAO_B::Init+[]\n");

    switch(this->id())
    {
        default:
            CAM_FUNC_ERR("this dmao:%s is not supported\n",this->name_Str());
            return MFALSE;
            break;
    }

    return MTRUE;
}

MINT32 DMAO_CAMSV_B::_config( void )
{
    CAM_FUNC_INF("DMAO_CAMSV_B::_config+");

    this->m_hwModule = this->m_pDrv->m_hwModule;

    CAM_FUNC_INF("port id(%d), memBuf pa(%p),memBuf ofst(%08X), Header addr(%08X)",\
                  this->id(),\
                  (void*)this->dma_cfg.memBuf.base_pAddr,\
                  this->dma_cfg.memBuf.ofst_addr,\
                  this->Header_Addr);

    switch(this->id())
    {
        case CAMSV_DMA_IMGO:
            {
                MUINT32 new_crop_x = 0;
                //Header_CAMSV_IMGO fh_imgo; // camsv no need to handle header

                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_XSIZE,this->dma_cfg.size.xsize - 1);                                /* 0x1A050230 */
                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_YSIZE,this->dma_cfg.size.h - 1);                                    /* 0x1A050234 */
                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_STRIDE,this->dma_cfg.size.stride);                                  /* 0x1A050238 */
                //
                this->dma_cfg.bus_size = BusSizeCal();

                //if( this->fmt_sel.Bits.TG1_FMT != SV_TG_FMT_RAW8 ) {
                switch( this->dma_cfg.bus_size ) {
                    case 0: //  8-bit bus size
                        if(this->dma_cfg.size.stride % 1 != 0) {
                            CAM_FUNC_ERR("IMGO stride(%lu) need 1-alignment", this->dma_cfg.size.stride);
                            return -1;
                        }
                        CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_STRIDE,CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_STRIDE)|(1<<24)|(0<<16));
                        break;
                    case 1: // 16-bit bus size
                        if(this->dma_cfg.size.stride % 2 != 0) {
                            CAM_FUNC_ERR("IMGO stride(%lu) need 2-alignment", this->dma_cfg.size.stride);
                            return -1;
                        }
                        CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_STRIDE,CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_STRIDE)|(1<<24)|(1<<16));
                        break;
                    case 3:// 32-bit bus size
                        if(this->dma_cfg.size.stride % 4 != 0) {
                            CAM_FUNC_ERR("IMGO stride(%lu) need 4-alignment", this->dma_cfg.size.stride);
                            return -1;
                        }
                        CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_STRIDE,CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_STRIDE)|(1<<24)|(3<<16));
                        break;
                    case 7:// 64-bit bus size
                        if(this->dma_cfg.size.stride % 8 != 0) {
                            CAM_FUNC_ERR("IMGO stride(%lu) need 8-alignment", this->dma_cfg.size.stride);
                            return -1;
                        }
                        CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_STRIDE,CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_STRIDE)|(1<<24)|(7<<16));
                        break;
                }
                //}

                new_crop_x = GetCropXUnitBussize(this->dma_cfg.crop.x, this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);
                CAM_FUNC_INF("****** %s %d, new_crop_x(%d), GetCropXUnitBussize(x %d, pixel_byte %d, bus_size %d)", __FUNCTION__, __LINE__, new_crop_x, this->dma_cfg.crop.x, this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);

                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_CROP,((this->dma_cfg.crop.y << 16) | new_crop_x));                 /* 0x1A050248 */
                //reverse for FH below
                new_crop_x = GetCropXUnitPixel(new_crop_x, this->dma_cfg.pixel_byte, this->dma_cfg.bus_size);
                CAM_FUNC_INF("****** %s %d, new_crop_x(%d)", __FUNCTION__, __LINE__, new_crop_x);

                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_CON,0x80000080);                                                    /* 0x1A05023C */
                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_CON2,0x00020002); //request by DE:sc                                                  /* 0x1A050240 */
                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_CON3,0x00020002); //request by DE:sc                                                  /* 0x1A050244 */

                //camsv no need to handle header
                //fh_imgo.Header_Enque(Header_CAMSV_IMGO::E_CROP_START,this->m_pDrv,((this->dma_cfg.crop.y << 16) | new_crop_x));
                //fh_imgo.Header_Enque(Header_CAMSV_IMGO::E_CROP_SIZE,this->m_pDrv,((this->dma_cfg.crop.h << 16) | this->dma_cfg.crop.w));

            }
            break;
        default:
            break;
    }

    CAM_FUNC_INF("****** Reg(CAMSV_MODULE_EN)=0x%08x", CAMSV_READ_REG(this->m_pDrv,CAMSV_MODULE_EN));
    CAM_FUNC_INF("****** Reg(CAMSV_FMT_SEL)=0x%08x", CAMSV_READ_REG(this->m_pDrv,CAMSV_FMT_SEL));
    CAM_FUNC_INF("****** Reg(CAMSV_PAK)=0x%08x", CAMSV_READ_REG(this->m_pDrv,CAMSV_PAK));
    CAM_FUNC_INF("****** Reg(CAMSV_IMGO_XSIZE)=0x%08x(%d)", CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_XSIZE), CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_XSIZE));
    CAM_FUNC_INF("****** Reg(CAMSV_IMGO_YSIZE)=0x%08x(%d)", CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_YSIZE), CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_YSIZE));
    CAM_FUNC_INF("****** Reg(CAMSV_IMGO_STRIDE)=0x%08x([15:0]:%d)", CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_STRIDE), (CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_STRIDE)&0xFFFF));
    CAM_FUNC_INF("****** Reg(CAMSV_IMGO_CON)=0x%08x", CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_CON));
    CAM_FUNC_INF("****** Reg(CAMSV_IMGO_CON2)=0x%08x", CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_CON2));
    CAM_FUNC_INF("****** Reg(CAMSV_IMGO_CON3)=0x%08x", CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_CON3));
    CAM_FUNC_INF("****** Reg(CAMSV_IMGO_CROP)=0x%08x", CAMSV_READ_REG(this->m_pDrv,CAMSV_IMGO_CROP));


    CAM_FUNC_INF("size(w %lu,h %lu,stride %lu,xsize %lu), pixel_byte(%d)", \
                this->dma_cfg.size.w, \
                this->dma_cfg.size.h, \
                this->dma_cfg.size.stride, \
                this->dma_cfg.size.xsize, \
                this->dma_cfg.pixel_byte);

    CAM_FUNC_INF("crop(x %d,y %d,w %lu,h %lu),format(%d),fps(0x%x),cropX_inbussize(%d)", \
                this->dma_cfg.crop.x, \
                this->dma_cfg.crop.y,\
                this->dma_cfg.crop.w,\
                this->dma_cfg.crop.h,\
                this->dma_cfg.format,\
                this->m_fps,\
                GetCropXUnitBussize(this->dma_cfg.crop.x , this->dma_cfg.pixel_byte, this->dma_cfg.bus_size));

    CAM_FUNC_INF("DMAO_CAMSV_B::_config-");

    return 0;
}

MINT32 DMAO_CAMSV_B::_enable( void* pParam )
{
    (void)pParam;

    CAM_FUNC_INF("DMAO_CAMSV_B::_enable+");

    switch(this->id())
    {
        case CAMSV_DMA_IMGO:
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,IMGO_EN,1);                         /* 0x1A050010 */
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_IMGO,1);  /* 0x1A050C00 */
            break;
        default:
            CAM_FUNC_ERR("un-support DMAO:%s\n",this->name_Str());
            return -1;
            break;
    }
    CAM_FUNC_INF("DMAO_CAMSV_B::_enable-");
    return 0;
}

MINT32 DMAO_CAMSV_B::_disable( void* pParam )
{
    (void)pParam;

    CAM_FUNC_INF("DMAO_CAMSV_B::_disable+");

    switch(this->id())
    {
        case CAMSV_DMA_IMGO:
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_MODULE_EN,IMGO_EN,0);
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_DMA_FRAME_HEADER_EN,FRAME_HEADER_EN_IMGO,0);
            break;
        default:
            CAM_FUNC_ERR("un-support DMAO:%s\n",this->name_Str());
            return -1;
            break;
    }

    CAM_FUNC_INF("DMAO_CAMSV_B::_disable-");

    return 0;
}


MINT32 DMAO_CAMSV_B::setBaseAddr(void)
{
    CAM_FUNC_INF("DMAO_B::setBaseAddr+[%s]:pa(0x%p_0x%08X),ofst(0x%X)", \
        this->name_Str(), \
        (void*)this->dma_cfg.memBuf.base_pAddr,\
        this->Header_Addr,\
        this->dma_cfg.memBuf.ofst_addr);
    if(this->dma_cfg.memBuf.ofst_addr != 0){
        CAM_FUNC_ERR("support no damo offsetaddress,bypass offset setting\n");
    }

    switch(this->id())
    {
        case CAMSV_DMA_IMGO:
            {
                //Header_CAMSV_IMGO fh_imgo; // camsv no need to handle header
                //fh_imgo.Header_Enque(Header_IMGO::E_IMG_PA,this->m_pDrv,this->dma_cfg.memBuf.base_pAddr);

                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_OFST_ADDR,0x0);
                // Actually, we use dma ring buffer mode, so this register should not use
                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_BASE_ADDR,this->dma_cfg.memBuf.base_pAddr);

                //CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_FH_BASE_ADDR,this->Header_Addr);
            }
            break;
        default:
            break;
    }

    //CAM_FUNC_DBG("DMAO_B::_config:X ");
    return 0;
}


MUINT32 DMAO_CAMSV_B::BusSizeCal(void)
{
    switch(this->fmt_sel.Bits.TG1_FMT){
        case SV_TG_FMT_YUV422:
            switch(this->m_PixMode){
                case ePixMode_1:
                    return 1; // 16-bit
                    break;
                case ePixMode_2:
                    return 1; // 16-bit
                    break;
                case ePixMode_4:
                    return 3; // 32-bit
                    break;
                default:
                    CAM_FUNC_ERR("Unsopported pix mode: %d\n", this->m_PixMode);
            }
            break;
        case SV_TG_FMT_JPG:
        //case SV_TG_FMT_RAW8: // jinn - remove

            switch(this->m_PixMode){
                case ePixMode_1:
                    return 0; // 8-bit
                    break;
                case ePixMode_2:
                    return 1; // 16-bit
                    break;
                case ePixMode_4:
                    return 3; // 32-bit
                    break;
                default:
                    CAM_FUNC_ERR("Unsopported pix mode: %d\n", this->m_PixMode);
            }
            break;
        case SV_TG_FMT_RAW8: // jinn - added
        case SV_TG_FMT_RAW10:
        case SV_TG_FMT_RAW12:
        case SV_TG_FMT_RAW14:
        default://bayer
           switch(this->m_PixMode){
                case ePixMode_1:
                    return 1; // 16-bit
                    break;
                case ePixMode_2:
                    return 3; // 32-bit
                    break;
                case ePixMode_4:
                    return 7; // 64-bit
                    break;
                default:
                    CAM_FUNC_ERR("Unsopported pix mode: %d\n", this->m_PixMode);
            }
            break;
    }
    CAM_FUNC_ERR("BusSizeCal fail\n");
    return 0;
}


MUINT32 CAMSV_BUF_CTRL::m_fps = 30;
MUINT32 CAMSV_BUF_CTRL::m_recentFrmTimeMs[CAMSV_MAX][MAX_RECENT_GRPFRM_TIME];

CAMSV_BUF_CTRL::CAMSV_BUF_CTRL(void)
{
    m_fps = 30;
    m_buf_cnt = 0;
    m_pDrv = NULL;
    m_pTimeStamp = NULL;

    for(MUINT32 i = CAMSV_0; i < CAMSV_MAX; i++){
        for(MUINT32 j = 0; j < MAX_RECENT_GRPFRM_TIME; j++)
            CAMSV_BUF_CTRL::m_recentFrmTimeMs[i][j] = MIN_GRPFRM_TIME_MS;
    }
}

MBOOL CAMSV_BUF_CTRL::PipeCheck(void)
{
    struct ISP_RAW_INT_STATUS err_status;
    CAMSV_PIPE_CHECK ppc;
    MBOOL chkrst = MTRUE;
    IspDrvCamsv* ptr = NULL;
    SENINF_DBG seninf_dbg;

    CAM_FUNC_ERR("start PipeCheck when deque fail at wait signal\n");

    ppc.m_pDrv = this->m_pDrv;

    //err status:
    this->m_pDrv->getDeviceInfo(_GET_INT_ERR,(MUINT8 *)&err_status);
    if(err_status.ispIntErr== 0){
    }
    else
        CAM_FUNC_ERR("accumulated err_status:0x%x\n", err_status.ispIntErr);

    //
    if(err_status.ispIntErr & CAMSV_INT_EN_IMGO_ERR_INT_EN){
        CAM_FUNC_ERR("all dmao err status:\n");
        ppc.DMAO_STATUS();
        chkrst = MFALSE;
    } else
        CAM_FUNC_INF("find no dma err\n");

    if( ppc.TG_CHECK(err_status.ispIntErr) == MTRUE){
        CAM_FUNC_INF("TG check pass!\n");
    }
    else
        chkrst = MFALSE;

    if( (chkrst == MTRUE) && (err_status.ispIntErr==0)){
        if(ppc.ENQUE_CHECK() == MFALSE){
            goto EXIT;
        }
        else{
            CAM_FUNC_ERR("find no err, plz look for previous err ,like enque flow err\n");
        }
    }

    //dump phy reg
    CAM_FUNC_ERR("start dump CAMSV_%d register\n", this->m_hwModule - CAMSV_START);

    this->m_pDrv->DumpReg(MTRUE);

    if(seninf_dbg.m_fp_Sen_Camsv != NULL){

        CAM_FUNC_ERR("start dump seninf info\n");
        seninf_dbg.m_fp_Sen_Camsv((MUINT32)this->m_hwModule, 0);
    }

EXIT:

    return MTRUE;
}

CAMSV_BUF_CTRL::E_BC_STATUS CAMSV_BUF_CTRL::waitBufReady( void )
{
    #define _LOOP_  (5)
    E_BC_STATUS ret = eCmd_Pass;

    //Header_AAO  fh_aao;

    ISP_WAIT_IRQ_ST irq;
    MUINT32 loopCnt = _LOOP_;
    char str[128] = {'\0'};

    MUINT32 FbcCnt;
    //MUINT32 _header_data[E_HEADER_MAX];

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error \n");
        return eCmd_Fail;
    }

    //fbc reg dbg log
    this->FBC_STATUS();

    irq.Clear = ISP_IRQ_CLEAR_WAIT;
    irq.UserKey = 0;
    //irq.Timeout = (1000 + (this->m_fps - 1)) / this->m_fps;
    //irq.Timeout *= 4; //amplify 3 times is for the case of enque latch timing
    irq.Timeout = this->estimateTimeout(1);

    switch(this->id()){
         case _camsv_imgo_:
             irq.St_type = SIGNAL_INT;
             irq.Status = SV_SW_PASS1_DON_ST;
             break;
         default:
             CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
             return eCmd_Fail;
         break;
     }


    do{
        if(this->m_buf_cnt != 0){
            FbcCnt = CAMSV_READ_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL2,FBC_CNT);

            if(FbcCnt < this->m_buf_cnt){
                if(loopCnt == _LOOP_){
                    CAM_FUNC_INF("dma:CAMSV_BUF_CTRL already have avail_buf on dram, bWaitBufReady = 0");
                    return eCmd_Pass;
                }
                else{
                    goto EXIT;
                }
            }
            else{
                if(loopCnt == _LOOP_)
                    snprintf(str, sizeof(str), "dma:CAMSV_BUF_CTRL start wait:[enque record:%d_%d],",this->m_buf_cnt, FbcCnt);
                else{
                    char _tmp[16];
                    snprintf(_tmp, sizeof(_tmp), "[%d]time,",((_LOOP_-loopCnt)+1));
                    strncat(str, _tmp, strlen(_tmp));
                }
            }
        }
        else{
            if(loopCnt == _LOOP_){
                snprintf(str, sizeof(str), "dma:CAMSV_BUF_CTRL bWaitBufReady = 1,wait for sw enque[%d],timeout:%d x %d ms,",this->m_buf_cnt,irq.Timeout,loopCnt);
#define FALSE_PASS1_DONE
#ifdef FALSE_PASS1_DONE
                ret =  eCmd_Fail;
                goto EXIT;
#endif
            }
            else{//save how many times of wait_irq before sw enque
                char _tmp[16];
                snprintf(_tmp, sizeof(_tmp), "[%d],",((_LOOP_-loopCnt)+1)+1);
                strncat(str, _tmp, strlen(_tmp));
            }
        }

        //if suspend
        if(this->m_FSM.GetFSM() == CAMSV_BUF_CTRL::C_FSM::E_SUSPEND){
            ret = eCmd_Suspending_Pass;
            goto EXIT;
        }

        if(this->m_pDrv->waitIrq(&irq) == MTRUE) {
            ret = eCmd_Pass;
            break;
        }
        else{
            if(this->m_FSM.GetFSM() == CAMSV_BUF_CTRL::C_FSM::E_SUSPEND){
                ret = eCmd_Suspending_Pass;
                goto EXIT;
            }
            ret = eCmd_Fail;
        }

        //
        irq.Clear = ISP_IRQ_CLEAR_NONE;

    }while(--loopCnt > 0);

EXIT:
    switch(ret){
        case eCmd_Fail:
            if(this->m_buf_cnt != 0){
                CAM_FUNC_ERR("%s, m_buf_cnt(%d) waitbufready fail. start fail check", str, m_buf_cnt);
                this->PipeCheck();
            }
            else{
                CAM_FUNC_ERR("%s, m_buf_cnt(%d), waitbufready fail is caused by no enque", str, m_buf_cnt);
            }
            break;
        case eCmd_Pass:
            CAM_FUNC_INF("%s, waitbufready pass",str);
            break;
        case eCmd_Suspending_Pass:
            CAM_FUNC_WRN("%s, suspend pass",str);
            break;
        default:
            CAM_FUNC_ERR("unsupported status:%d\n",ret);
            ret = eCmd_Fail;
            break;
    }
    return ret;
}

MINT32 CAMSV_BUF_CTRL::_config(void)
{
    CAM_FUNC_INF("BUF_CTRL_CAMSV_IMGO:CAMSV_BUF_CTRL::config+");

    this->m_hwModule = this->m_pDrv->m_hwModule;

    this->m_Queue.init();

    switch(this->id()){
        case _camsv_imgo_:
            CAMSV_WRITE_REG(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,0);                      /* 0x1A050110 */
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,FBC_MODE,1);            /* 0x1A050110 */
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }
    //reset buf cnt
    this->m_buf_cnt = 0;

    CAM_FUNC_INF("BUF_CTRL_CAMSV_IMGO:CAMSV_BUF_CTRL::config-");

    return 0;
}

MINT32 CAMSV_BUF_CTRL::_enable( void* pParam  )
{
    CAM_FUNC_INF("BUF_CTRL_CAMSV_IMGO:CAMSV_BUF_CTRL::_enable+");
    if(pParam != NULL)
        CAM_FUNC_INF("subsample:0x%x",*(MUINT32*)pParam);
    else
        CAM_FUNC_INF("subsample:0x%x",0);

    if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN) == 1) { /* 0x1A050504 */
        CAM_FUNC_WRN("can't enable FBC at streaming\n");
        return 1;
    }

    switch(this->id()){
        /// TODO: JSS, verify it about sub ratio
        case _camsv_imgo_:
            if(pParam != NULL)
            {
                CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,SUB_RATIO,*(MUINT32*)pParam); /* 0x1A050110 */
            }
            else
            {
                CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,SUB_RATIO,0);                 /* 0x1A050110 */
            }
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,FBC_EN,1);                        /* 0x1A050110 */
            CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,DMA_RING_EN,1);                   /* 0x1A050110 */
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }
    CAM_FUNC_INF("BUF_CTRL_CAMSV_IMGO:CAMSV_BUF_CTRL::_enable-");
    return 0;
}

MINT32 CAMSV_BUF_CTRL::_disable( void* pParam )
{
    (void)pParam;

    CAM_FUNC_INF("CAMSV_BUF_CTRL::_disable+");

    if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN) == 1){
        CAM_FUNC_WRN("FBC at streaming");
    }

    switch(this->id()){
        case _camsv_imgo_:
            CAMSV_WRITE_REG(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,0);
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            return 1;
            break;
    }

    CAM_FUNC_INF("CAMSV_BUF_CTRL::_disable-");

    return 0;
}

MBOOL CAMSV_BUF_CTRL::suspend(void)
{
    MBOOL ret = MTRUE;
    //lock state
    this->m_FSM.SetFSM(CAMSV_BUF_CTRL::C_FSM::E_NORMAL , MTRUE);

    CAM_FUNC_INF("CAMSV_BUF_CTRL::suspend+");
    if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN) == 0){
        ret = MFALSE;
        //unlock
        this->m_FSM.SetFSM(CAMSV_BUF_CTRL::C_FSM::E_NORMAL , MFALSE);
    }
    else{
        MUINT32 _size;
        //
        this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
        //no need to wait vsync.   pipe with cam's suspend
        //clr buf record
        this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
        for(;_size>0;_size--){
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop,0,0);
        }
        this->m_buf_cnt = 0;
        //unlock && change state
        this->m_FSM.SetFSM(CAMSV_BUF_CTRL::C_FSM::E_SUSPEND , MFALSE);
    }

    return ret;
}

MBOOL CAMSV_BUF_CTRL::resume(void)
{
    MBOOL ret = MTRUE;
    //lock state
    this->m_FSM.SetFSM(CAMSV_BUF_CTRL::C_FSM::E_NORMAL , MTRUE);

    CAM_SFUNC_INF("CAMSV_BUF_CTRL::resume+");
    if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN) == 1){
        ret = MFALSE;
        //unlock
        this->m_FSM.SetFSM(CAMSV_BUF_CTRL::C_FSM::E_NORMAL , MFALSE);
    }
    else{
        MUINT32 _size;
        //
        this->m_pDrv->setDeviceInfo(_SET_VF_ON,(MUINT8*)&this->m_hwModule);
        //unlock
        this->m_FSM.SetFSM(CAMSV_BUF_CTRL::C_FSM::E_RESUME , MFALSE);
    }

    return ret;
}

MBOOL CAMSV_BUF_CTRL::C_FSM::SetFSM(E_STATE state,MBOOL lock)
{
    MBOOL ret = MTRUE;

    if(lock){
        this->m_lock.lock();
        return ret;
    }
    else
        this->m_lock.unlock();

    switch(state){
        case E_NORMAL:
            if(this->m_State != E_NORMAL)
                ret = MFALSE;
            break;
        case E_SUSPEND:
            if(this->m_State == E_NORMAL)
                this->m_State = E_SUSPEND;
            else
                ret = MFALSE;
            break;
        case E_RESUME:
            if(this->m_State == E_SUSPEND)
                this->m_State = E_NORMAL;
            break;
        default:
            ret = MFALSE;
            break;

    }

    if(ret != MTRUE){
        CAM_SFUNC_ERR("error: target op:%d, current op:%d\n",state,this->m_State);
    }
    return ret;
}

CAMSV_BUF_CTRL::C_FSM::E_STATE CAMSV_BUF_CTRL::C_FSM::GetFSM(void)
{
    Mutex::Autolock lock(this->m_lock);

    return this->m_State;
}

MBOOL CAMSV_BUF_CTRL::enqueueHwBuf( stISP_BUF_INFO& buf_info,MBOOL bImdMode)
{
    ST_BUF_INFO* ptr = NULL;
    MUINT32 _sof_idx;
    MUINT32 _size;
    MUINT32 _reg_time_stamp;
    ST_BUF_INFO _buf;

    (void)bImdMode;

    Mutex::Autolock lock(this->m_bufctrl_lock);


    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error\n");
        return MFALSE;
    }

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);
    //fbc reg dbg log
    this->FBC_STATUS();

    // CamsvIOPipe not support replacemode
    if(buf_info.bReplace == MTRUE){
        ptr = &buf_info.Replace;
    }
    else{
        ptr = &buf_info.u_op.enque.at(ePlane_1st);
    }

    //check over enque or not
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize,(MUINTPTR)&_size,0);
    for(MUINT32 i=0;i<_size;i++){
        if(MFALSE == this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_at,i,(MUINTPTR)&_buf))
            CAM_FUNC_ERR("%s: get no element at %d\n",this->name_Str(),i);
        else{
            if(_buf.image.mem_info.pa_addr == ptr->image.mem_info.pa_addr){
                CAM_FUNC_ERR("%s: over enque at %d.[addr:0x%p]\n",this->name_Str(),i,(void*)_buf.image.mem_info.pa_addr);
                return MFALSE;
            }
        }
    }

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_push,(MUINTPTR)ptr,0);

    //
    this->m_buf_cnt++;

    //
    switch(this->id()){
        case _camsv_imgo_:
            {
                // enque to hw directly
                if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL1,FBC_EN) != 1){
                    CAM_FUNC_ERR("%s: can't enque before enabe fbc\n",this->name_Str());
                    return MFALSE;
                }
                //enque buf pa
                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_FBC_IMGO_ENQ_ADDR,(MUINT32)ptr->image.mem_info.pa_addr);
                //enque buf header pa
                CAMSV_WRITE_REG(this->m_pDrv,CAMSV_IMGO_FH_BASE_ADDR,(MUINT32)ptr->header.pa_addr);

                CAMSV_WRITE_BITS(this->m_pDrv,CAMSV_IMGO_FBC,RCNT_INC, 1);
            }
            break;
        default:
            CAM_FUNC_ERR("un-supported DMAO:0x%x\n",this->id());
            this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop  ,0,0);
            return MFALSE;
            break;
    }

    //
    if(buf_info.bReplace == MFALSE){
        _reg_time_stamp = CAMSV_READ_REG(this->m_pDrv, CAMSV_TG_TIME_STAMP);
        CAM_FUNC_INF("PA(0x%p_0x%p),FH_VA(%p),size(0x%x),enque_sof(%d),reg_timestmp(0x%08x)",\
            (void*)ptr->image.mem_info.pa_addr ,\
            (void*)ptr->header.pa_addr ,\
            (void*)ptr->header.va_addr ,\
            ptr->image.mem_info.size ,\
            _sof_idx ,\
            _reg_time_stamp);
    }
    else{//replace
        CAM_FUNC_INF("PA(0x%p_0x%p,new:0x%p_0x%p),FH_VA(%p,new:%p),size(0x%x),enque_sof(%d)",\
            (void*)ptr->image.mem_info.pa_addr ,\
            (void*)ptr->header.pa_addr ,\
            (void*)buf_info.u_op.enque.at(ePlane_1st).image.mem_info.pa_addr ,\
            (void*)buf_info.u_op.enque.at(ePlane_1st).header.pa_addr ,\
            (void*)ptr->header.va_addr ,\
            (void*)buf_info.u_op.enque.at(ePlane_1st).header.va_addr ,\
            ptr->image.mem_info.size ,\
            _sof_idx);
    }
    //
    return MTRUE;

}



//
CAMSV_BUF_CTRL::E_BC_STATUS CAMSV_BUF_CTRL::dequeueHwBuf(NSImageio::NSIspio::BufInfo& buf_info )
{
    E_BC_STATUS ret = CAMSV_BUF_CTRL::eCmd_Pass;
    MUINT32 _size;
    MUINT32 _sof_idx;
    MUINT32 _reg_time_stamp;
    ST_BUF_INFO _buf;
    MUINT32 _header_data[E_HEADER_MAX];
    Header_CAMSV_IMGO  fh_camsv_imgo;
    char str[128] = {'\0'};
    MUINT64 tmp;

    Mutex::Autolock lock(this->m_bufctrl_lock);

    //cause all sw record will be clear when suspend()
    if(this->m_FSM.GetFSM() != CAMSV_BUF_CTRL::C_FSM::E_NORMAL){
        CAM_FUNC_WRN("can't deque ,FSM not in normal mode\n");
        return CAMSV_BUF_CTRL::eCmd_Suspending_Pass;
    }

    if ( 0xffffffff == this->id() ) {
        CAM_FUNC_ERR("dma channel error ");
        return CAMSV_BUF_CTRL::eCmd_Fail;
    }

    //fbc reg dbg log
    this->FBC_STATUS();
    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_getsize, (MUINTPTR)&_size, 0);
    if(_size <1){
        CAM_FUNC_ERR("queue depth is empty for deque\n");
        return CAMSV_BUF_CTRL::eCmd_Fail;
    }
    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_front, (MUINTPTR)&_buf, 0);

    //
    this->m_pDrv->getDeviceInfo(_GET_SOF_CNT,(MUINT8 *) &_sof_idx);

    //
    buf_info.memID[ePlane_1st]                  = _buf.image.mem_info.memID;
    buf_info.u4BufSize[ePlane_1st]              = _buf.image.mem_info.size;
    buf_info.u4BufVA[ePlane_1st]                = _buf.image.mem_info.va_addr;
    buf_info.u4BufPA[ePlane_1st]                = _buf.image.mem_info.pa_addr;
    buf_info.bufCohe[ePlane_1st]                = _buf.image.mem_info.bufCohe;
    buf_info.bufSecu[ePlane_1st]                = _buf.image.mem_info.bufSecu;
    buf_info.Frame_Header.u4BufPA[ePlane_1st]   = _buf.header.pa_addr;
    buf_info.Frame_Header.u4BufVA[ePlane_1st]   = _buf.header.va_addr;
    buf_info.Frame_Header.u4BufSize[ePlane_1st] = _buf.header.size;
    buf_info.Frame_Header.memID[ePlane_1st]     = _buf.header.memID;
    buf_info.Frame_Header.bufCohe[ePlane_1st]   = _buf.header.bufCohe;
    buf_info.Frame_Header.bufSecu[ePlane_1st]   = _buf.header.bufSecu;
#if 0
    buf_info.img_w                  = ;
    buf_info.img_h                  = ;
    buf_info.crop_win.p.x           = ;
    buf_info..crop_win.p.y          = ;
    buf_info.crop_win.s.w           = ;
    buf_info.crop_win.s.h           = ;
#endif
    buf_info.img_stride             = 0;

    //timestamp
    fh_camsv_imgo.m_hwModule = this->m_hwModule;
    fh_camsv_imgo.Header_Deque(_buf.header.va_addr, (void*)_header_data);
    fh_camsv_imgo.Header_Flash(_buf.header.va_addr);

    buf_info.i4TimeStamp_us = _header_data[Header_CAMSV_IMGO::E_TimeStamp_LSB];
    tmp = _header_data[Header_CAMSV_IMGO::E_TimeStamp_MSB];
    buf_info.i4TimeStamp_us += (tmp << 32);
    this->m_pTimeStamp->TimeStamp_mapping(&buf_info.i4TimeStamp_us,&buf_info.i4TimeStamp_us_B,(_isp_dma_enum_)this->id());

    //
    this->m_Queue.sendCmd(QueueMgr<ST_BUF_INFO>::eCmd_pop, 0, 0);
    this->m_buf_cnt--;

EXIT:
    switch(this->id()){
        case _camsv_imgo_:
            CAM_FUNC_INF("PA(0x%p_0x%p),FH_VA(%p_0x%p),size(0x%08x),cur sof(%d),timestamp:%" PRIu64 ,\
                (void*)_buf.image.mem_info.pa_addr ,\
                (void*)_buf.header.pa_addr ,\
                (void*)_buf.image.mem_info.va_addr ,\
                (void*)_buf.header.va_addr ,\
                _buf.image.mem_info.size ,\
                _sof_idx,\
                buf_info.i4TimeStamp_us);
            break;
        default:
            CAM_FUNC_DBG("PA(0x%p_0x%p),FH_VA(%p_0x%p),size(0x%x),cur sof(%d),timestamp:%" PRIu64 ,\
                (void*)_buf.image.mem_info.pa_addr ,\
                (void*)_buf.header.pa_addr ,\
                (void*)_buf.image.mem_info.va_addr ,\
                (void*)_buf.header.va_addr ,\
                _buf.image.mem_info.size ,\
                _sof_idx,\
                buf_info.i4TimeStamp_us);
            break;
    }

    return ret;
}

MUINT32 CAMSV_BUF_CTRL::estimateTimeout(MUINT32 subSample)
{
    MUINT32 _t_ms = (m_recentFrmTimeMs[this->m_hwModule][0] >= m_recentFrmTimeMs[this->m_hwModule][1])?
        m_recentFrmTimeMs[this->m_hwModule][0]: m_recentFrmTimeMs[this->m_hwModule][1];

    _t_ms *= subSample;

    _t_ms = (MIN_GRPFRM_TIME_MS > _t_ms)? MIN_GRPFRM_TIME_MS: _t_ms;

    _t_ms *= 2;

    return _t_ms;
}

MBOOL CAMSV_BUF_CTRL::updateFrameTime(MUINT32 timeInMs, MUINT32 reqOffset, ISP_HW_MODULE camModule)
{
    if (reqOffset >= MAX_RECENT_GRPFRM_TIME) {
        return MFALSE;
    }

    m_recentFrmTimeMs[camModule][reqOffset] = timeInMs;

    return MTRUE;
}

/**
    check whether this fail is caused by some dmao waiting for enque or not
    no rsso check
*/
MBOOL CAMSV_PIPE_CHECK::ENQUE_CHECK(void)
{
    MBOOL rst = MTRUE;
    MUINT32 imgo_en;
    MUINT32 imgo_fbc;

    imgo_en = CAMSV_READ_BITS(this->m_pDrv,CAMSV_MODULE_EN,IMGO_EN);

    if(imgo_en){
        imgo_fbc = CAMSV_READ_BITS(this->m_pDrv,CAMSV_FBC_IMGO_CTL2,FBC_CNT);
        if(imgo_fbc == 0){
            CAM_FUNC_ERR("IMGO have no enque record\n");
            rst = MFALSE;
        }
    }

    return rst;
}


MBOOL CAMSV_PIPE_CHECK::DMAO_STATUS(void)
{
    MUINT32 dma_err[16];
    this->m_pDrv->getDeviceInfo(_GET_DMA_ERR,(MUINT8 *)dma_err);

    if(dma_err[0] & 0xffff){
        CAM_FUNC_ERR("[cam dmao:0x%x]IMGO ERR:0x%x\n",\
            CAMSV_READ_REG(this->m_pDrv,CAMSV_MODULE_EN),\
            dma_err[0]);
    }
    else{
        CAM_FUNC_ERR("read no cur dma_err err status\n");
    }

    return MTRUE;
}

MBOOL CAMSV_PIPE_CHECK::TG_CHECK(MUINT32 intErrStatus)
{
    MBOOL rst = MTRUE;
    CAMSV_REG_TG_SEN_GRAB_PXL TG_W;
    CAMSV_REG_TG_SEN_GRAB_LIN TG_H;
    MUINT32 TG_IN_W,TG_IN_V;
    MUINT32 TG_IN_W_R,TG_IN_V_R;
    MUINT32 DATA_CNT_R;
    MUINT32 tmp;
    CAMSV_REG_INT_STATUS irqStatCheck;

    if(CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_VF_CON,VFDATA_EN) == 0){
        CAM_FUNC_ERR("viewfinder is not opened yet,flase alarm\n");
        rst = MTRUE;
        goto EXIT;
    }

    irqStatCheck.Raw = intErrStatus;

    if (irqStatCheck.Bits.TG_GBERR_ST) {
        rst = MFALSE;
    }

    TG_W.Raw = CAMSV_READ_REG(this->m_pDrv,CAMSV_TG_SEN_GRAB_PXL);
    TG_H.Raw = CAMSV_READ_REG(this->m_pDrv,CAMSV_TG_SEN_GRAB_LIN);

    TG_IN_W = CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_FRMSIZE_ST,PXL_CNT);
    TG_IN_V = CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_FRMSIZE_ST,LINE_CNT);


    TG_IN_W_R = CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_FRMSIZE_ST_R,PXL_CNT);
    TG_IN_V_R = CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_FRMSIZE_ST_R,LINE_CNT);

    DATA_CNT_R = CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_DAT_NO_R,DAT_NO);

    if(TG_IN_W < (TG_W.Bits.PXL_E - TG_W.Bits.PXL_S)){
        CAM_FUNC_ERR("seninf horizontal data is small than grab window_w:%d_%d\n",TG_IN_W,(TG_W.Bits.PXL_E - TG_W.Bits.PXL_S));
        rst = MFALSE;
    }
    if(TG_IN_V < (TG_H.Bits.LIN_E - TG_H.Bits.LIN_S)){
        CAM_FUNC_ERR("seninf vertical data is small than grab window_v:%d_%d\n",TG_IN_V,(TG_H.Bits.LIN_E - TG_H.Bits.LIN_S));
        rst = MFALSE;
    }

    tmp = CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_INTER_ST,TG_CAM_CS);
    switch(tmp){
        case 2: //
        case 16:
            usleep(1000);
            if(TG_IN_W_R != TG_IN_W){
                if(DATA_CNT_R == CAMSV_READ_BITS(this->m_pDrv,CAMSV_TG_DAT_NO_R,DAT_NO)){
                    CAM_FUNC_ERR("seninf have no input data for over 1ms when TG is under exposure,cur data:0x%x\n",DATA_CNT_R);
                    rst = MFALSE;
                }
            }
            break;
        default:
            CAM_FUNC_ERR("TG is in idle status:0x%x\n",tmp);
            rst = MTRUE;
            break;

    }

    if(rst == MFALSE){
        CAM_FUNC_ERR("TG checkl fail\n");
    }


EXIT:

    return rst;

}

void BUF_CTRL_CAMSV_IMGO::FBC_STATUS()
{
    CAM_FUNC_DBG("BUF_CTRL_CAMSV_IMGO::FBC_STATUS: 0x%08x_0x%08x, m_buf_cnt(%d)",\
        CAMSV_READ_REG(this->m_pDrv,CAMSV_FBC_IMGO_CTL1),\
        CAMSV_READ_REG(this->m_pDrv,CAMSV_FBC_IMGO_CTL2),\
        this->m_buf_cnt
        );
}

void Header_CAMSV_IMGO::Header_Deque(MUINTPTR FH_VA,void* pOut)
{
    void* ptr = (void*)FH_VA;
    memcpy(pOut,ptr,sizeof(MUINT32)*E_MAX);
}

void Header_CAMSV_IMGO::Header_Flash(MUINTPTR FH_VA)
{
    void* ptr = (void*)FH_VA;
    memset(ptr,0,sizeof(MUINT32)*E_MAX);

}

