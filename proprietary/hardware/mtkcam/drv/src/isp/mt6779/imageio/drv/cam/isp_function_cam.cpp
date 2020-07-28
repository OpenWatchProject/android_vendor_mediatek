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

#define LOG_TAG "ifunc_cam"

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
#include "isp_function_cam.h"
#include "Cam_Notify_datatype.h"
#include "tuning_drv.h"
#include "cam_capibility.h"
#include "sec_mgr.h"

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

using namespace NSCam::NSIoPipe::NSCamIOPipe;
DECLARE_DBG_LOG_VARIABLE(func_cam);
//EXTERN_DBG_LOG_VARIABLE(func_cam);

// Clear previous define, use our own define.
#undef CAM_FUNC_VRB
#undef CAM_FUNC_DBG
#undef CAM_FUNC_INF
#undef CAM_FUNC_WRN
#undef CAM_FUNC_ERR
#undef CAM_FUNC_AST
#define CAM_FUNC_VRB(fmt, arg...)        do { if (func_cam_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define CAM_FUNC_DBG(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_INF(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_WRN(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_WARN   ) { \
        BASE_LOG_WRN("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_ERR(fmt, arg...)        do {\
    if (func_cam_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("[0x%x]:" fmt,this->m_hwModule, ##arg); \
    }\
} while(0)

#define CAM_FUNC_AST(cond, fmt, arg...)  do { if (func_cam_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


#define CAM_CB_DURATION                      (1)
#define CAM_CB_WRITE_REG(pReg,RegName,Value) ({((cam_reg_t*)pReg)->RegName.Raw = (Value);})
#define CAM_CB_READ_REG(pReg,RegName)        ({((cam_reg_t*)pReg)->RegName.Raw;})
#define CAM_CB_DURATION_US                   (3000)


MBOOL CAM_DPD_CTRL::m_pdo_en[PHY_CAM] = {MFALSE, MFALSE, MFALSE};

/*//////////////////////////////////////////////////////////////////////////////
  CAM_TG_CTRL
///////////////////////////////////////////////////////////////////////////////*/
CAM_TG_CTRL::CAM_TG_CTRL()
{
    m_PixMode = ePixMode_1;
    m_Datapat = eCAM_NORMAL;
    m_continuous = MFALSE;
    m_SubSample  = 0;
    m_pDrv = NULL;
}

MINT32 CAM_TG_CTRL::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    capibility CamInfo;
    tCAM_rst rst;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    char userName[32] = {'\0'};

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_TG_CTRL::_config +:xcropping:(0x%x_0x%x_0x%lx_0x%lx),is_continuous(0x%x),SubSample(0x%x),pixmode(0x%x),data-pattern(0x%x)\n", \
            this->m_Crop.x, \
            this->m_Crop.y, \
            this->m_Crop.w, \
            this->m_Crop.h,  \
            this->m_continuous,  \
            this->m_SubSample,\
            this->m_PixMode,\
            this->m_Datapat);
    }

    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(\
                                         (NSCam::EImageFormat)0, 0, this->m_PixMode), rst, E_CAM_pipeline_min_size);

    if((this->m_Crop.w - this->m_Crop.x) < rst.pipelineMinSize.w)
        CAM_FUNC_ERR("TG size is smaller than pipeline min width size(%d/%d)", rst.pipelineMinSize.w, this->m_Crop.w - this->m_Crop.x);

    //subsample
    //for vsync subsample function, need to make sure cmos_en is off.
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_CMOS_EN,0);
    if(this->m_SubSample){
        //sof
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_SOF_SUB_EN,1);
        //vsync
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_VS_SUB_EN,1);
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SUB_PERIOD,((this->m_SubSample<<8)|this->m_SubSample));
    }
    else{
        //sof
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_SOF_SUB_EN,0);
        //vsync
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_VS_SUB_EN,0);
        CAM_WRITE_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SUB_PERIOD,((this->m_SubSample<<8)|this->m_SubSample));
    }

    //trig mode
    if(this->m_continuous){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_VF_CON,TG_SINGLE_MODE,0);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_VF_CON,TG_SINGLE_MODE,1);
    }

    //pix mode
    switch(this->m_PixMode){
        case ePixMode_1:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS,0);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1,0);
            break;
        case ePixMode_2:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1,0);
            break;
        case ePixMode_4:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS,1);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1,1);
            break;
        default:
            CAM_FUNC_ERR("- unsupported pix mode:0x%x\n", (MUINT32)this->m_PixMode);
            break;
    }

    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL,(((this->m_Crop.w+this->m_Crop.x)<<16)|this->m_Crop.x));
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN,(((this->m_Crop.h+this->m_Crop.y)<<16)|this->m_Crop.y));

    CAM_WRITE_REG(this->m_pDrv,TG_R1_TG_SEN_GRAB_PXL,(((this->m_Crop.w+this->m_Crop.x)<<16)|this->m_Crop.x));
    CAM_WRITE_REG(this->m_pDrv,TG_R1_TG_SEN_GRAB_LIN,(((this->m_Crop.h+this->m_Crop.y)<<16)|this->m_Crop.y));

    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SRC_TG, MAP_MODULE_TG(this->m_hwModule, MTRUE, CAM_FUNC_ERR)); //set self TG

    //
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_ERR_CTL,TG_GRAB_ERR_EN,1);

    //If TG is overrun, must set TG_FULL_SEL1 to get TG overrun interrupt, Pochou
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_PATH_CFG,TG_TG_FULL_SEL,1);

    //for w+t switch: call init to add user count
    //otherwise, under switch condition, isp_drv_cam instance may be released by another cam when do CamQ detach
    sprintf(userName, "CAM_TG_CTRL:%d", this->m_hwModule);
    this->m_pDrv->init(userName);

    CAM_FUNC_DBG("-");
    return 0;
}

MINT32 CAM_TG_CTRL::pre_enable()
{
    E_ISP_CAM_CQ     cq;
    MUINT32          page = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_TG_CTRL::pre_enable +:\n");

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_MODE, TG_CMOS_EN, 1);

    return 0;
}

MINT32 CAM_TG_CTRL::_enable( void* pParam  )
{
    pParam;
    E_ISP_CAM_CQ     cq;
    MUINT32          page = 0;
    CAM_TIMESTAMP*   pTime = NULL;
    SecMgr*          pSec = SecMgr::SecMgr_GetMgrObj();
    ISP_CLEAR_IRQ_ST clr_irq = {.Status  = SW_PASS1_DON_ST,
                                .UserKey = 0x0,
                                .St_type = SIGNAL_INT,
                                };

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_TG_CTRL::_enable +:\n");

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_MODE, TG_CMOS_EN, 1);

    this->m_pDrv->clearIrq(&clr_irq);
    pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule,this->m_pDrv);

    if (pTime) {
        pTime->TimeStamp_reset();
    }

    //For secure camera
    if (pSec){
        if (pSec->SecMgr_GetSecureCamStatus(this->m_hwModule) > 0)
            this->m_pDrv->setDeviceInfo(_SET_SEC_DAPC_REG, (MUINT8*)&this->m_hwModule);
    }

    this->m_pDrv->setDeviceInfo(_SET_VF_ON,(MUINT8*)&this->m_hwModule);

    CAM_FUNC_INF("LOCK WAKE LOCK\n");

    return 0;
}

MINT32 CAM_TG_CTRL::pre_disable()
{
#define IS_TG_IDLE(_TGStat)( \
    (_TGStat.Bits.TG_TG_CAM_CS == 1) && \
    (_TGStat.Bits.TG_SYN_VF_DATA_EN == 0) && \
    (_TGStat.Bits.TG_OUT_REQ == 0))

    E_ISP_CAM_CQ  cq;
    MUINT32       page = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_TG_CTRL::pre_disable +:\n");

    if (CAM_READ_BITS(this->m_pDrv->getPhyObj(), TG_R1_TG_VF_CON, TG_VFDATA_EN) == 1) {
        MUINT32 _cnt = 0;
        REG_TG_R1_TG_INTER_ST tg_stat;
        ISP_WAIT_IRQ_ST irq = {
            .Clear   = ISP_IRQ_CLEAR_WAIT,
            .Status  = VS_INT_ST,
            .St_type = SIGNAL_INT,
            .Timeout = MIN_GRPFRM_TIME_MS * 2,
            .UserKey = 0x0
        };

        if (CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN))
            CAM_FUNC_WRN("DCIF case should be disable VF");

        this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
        do {
            ++_cnt;

            if (this->m_pDrv->getPhyObj()->waitIrq(&irq) == MFALSE)
                CAM_FUNC_ERR("wait vsync fail: %d\n", _cnt);
            else
                CAM_FUNC_INF("wait vsync %d time for TG idle\n", _cnt);

            if (_cnt > 3) {
                PIPE_CHECK ppc;
                ppc.m_pDrv = this->m_pDrv;
                ppc.Check_Start();
                break;
            }

            tg_stat.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), TG_R1_TG_INTER_ST);

            if (1 == tg_stat.Bits.TG_TG_CAM_CS) {
                if ((0 != tg_stat.Bits.TG_SYN_VF_DATA_EN) || (0 != tg_stat.Bits.TG_OUT_REQ)) {
                    CAM_FUNC_INF("WARNING: TG stat NOT IDLE: 0x%x", tg_stat.Raw);
                }
            }
        } while (!IS_TG_IDLE(tg_stat));
    }
    else {
        REG_TG_R1_TG_INTER_ST tg_stat;

        tg_stat.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(), TG_R1_TG_INTER_ST);

        if (IS_TG_IDLE(tg_stat))
            CAM_FUNC_INF("vf_en off + tg idle, no wait vsync\n");
        else
            CAM_FUNC_ERR("Wrong TG state: 0x%x\n", tg_stat.Raw);
    }

    this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);

    return 0;
}

MINT32 CAM_TG_CTRL::_disable( void* pParam )
{
    E_ISP_CAM_CQ  cq;
    MUINT32       page = 0;
    char          userName[32] = {'\0'};
    (void)pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_TG_CTRL::_disable +:\n");

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), TG_R1_TG_PATH_CFG, TG_DB_LOAD_DIS, 1);//disable TG double buffer

    if (CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_VF_CON,TG_VFDATA_EN) == 1){
        this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_CMOS_EN,0);

    //clear TG interrupt setting
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_INT1,TG_TG_INT1_PXLNO,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_INT1,TG_TG_INT1_LINENO,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL, 0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN, 0);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), TG_R1_TG_PATH_CFG, TG_DB_LOAD_DIS, 0);//enable TG double buffer

    //for w+t switch: call uninit/destroyInstance to release isp_drv_cam instance
    sprintf(userName, "CAM_TG_CTRL:%d", this->m_hwModule);
    this->m_pDrv->uninit(userName);
    this->m_pDrv->destroyInstance();

    return 0;
}

/*/////////////////////////////////////////////////////////////////////////////
  CAM_TOP_CTRL
/////////////////////////////////////////////////////////////////////////////*/

CAM_TOP_CTRL::CAM_TOP_CTRL()
{
    CAM_Path = IF_CAM_MAX;
    SubSample = 0;
    m_pDrv = NULL;
    DBG_LOG_CONFIG(imageio, func_cam);
}

MINT32 CAM_TOP_CTRL::_config( void )
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    MUINT32 _loglevel = 0, i = 0, page = 0;
    E_ISP_CAM_CQ cq;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
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
                      DMA_ERR_EN_|\
                      SW_PASS1_DON_EN_);

    CAM_RRZ_CTRL rrz_ctl;

    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    if (page == 0) { /*reduce log, only show page 0*/
        CAM_FUNC_INF("CAM_TOP_CTRL::_config +,path(0x%x),en(0x%08x_0x%08x_0x%08x_0x%08x),fmtSel(0x%08x),ctrlsel(0x%08x),intEn(0x%08x),SubSample(0x%x),cq(0x%x),page_num(0x%x)\n", \
            this->CAM_Path, \
            this->cam_top_ctl.FUNC_EN.Raw, \
            this->cam_top_ctl.FUNC_EN2.Raw, \
            this->cam_top_ctl.FUNC_EN3.Raw, \
            this->cam_top_ctl.FUNC_EN4.Raw, \
            this->cam_top_ctl.FMT_SEL.Raw, \
            this->cam_top_ctl.CTRL_SEL.Raw, \
            int_en,  \
            this->SubSample,\
            cq,page);
    }

    /* Reset DMX_SEL (no double buffer), for dynamic twin switching
       If slave cam -> master cam, slave cam are still running,
       we must reset these registers, and do SW/HW reset.
      */
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL, CAMCTL_RAW_SEL, 0x0);

    //kernel log level:
    property_get("vendor.debug.isp", value, "0");
    _loglevel = atoi(value);
    if(_loglevel != 0){
        this->m_pDrv->setDeviceInfo(_SET_DBG_INT,(MUINT8*)&_loglevel);
    }

    //avoid previous scenario exit abnormally. e.g.: exit without stop();
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 _clrPort = 2;
    MUINT32 _dma[2] = {_imgo_,_rrzo_};

    while(_clrPort--){
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
        buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
        buf_ctrl.data_ptr = NULL;
        buf_ctrl.pExtend = 0;
        if ( MTRUE != this->m_pDrv->setDeviceInfo(_SET_BUF_CTRL,(MUINT8 *) &buf_ctrl)) {
            CAM_FUNC_ERR("ERROR:clear buf ctrl fail\n");
        }
    }

    //reset
    SW_RESET(this->m_pDrv->getPhyObj());

    /********************************************************************************************************************/
    /********************************************************************************************************************/
    /********************************************************************************************************************/

    //fmt sel
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_FMT_SEL, this->cam_top_ctl.FMT_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_FMT2_SEL, this->cam_top_ctl.FMT2_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_FMT3_SEL, this->cam_top_ctl.FMT3_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_FMT_SEL, this->cam_top_ctl.FMT_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_FMT2_SEL, this->cam_top_ctl.FMT2_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_FMT3_SEL, this->cam_top_ctl.FMT3_SEL.Raw);

    //cq db setting
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_EN, 1); //enable double buffer
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DROP_FRAME_EN, 1); //bypass cq overvsync for BPCI_R2 bug

    //func en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN, this->cam_top_ctl.FUNC_EN.Raw);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN2, this->cam_top_ctl.FUNC_EN2.Raw);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN3, this->cam_top_ctl.FUNC_EN3.Raw);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN4, this->cam_top_ctl.FUNC_EN4.Raw);
    CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, this->cam_top_ctl.FUNC_EN.Raw);
    CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, this->cam_top_ctl.FUNC_EN2.Raw);
    CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN3, this->cam_top_ctl.FUNC_EN3.Raw);
    CAM_WRITE_REG(this->m_pDrv, CAMCTL_R1_CAMCTL_EN4, this->cam_top_ctl.FUNC_EN4.Raw);
#if 1 //hw issue , tg_en must be off once if stop cam with raw_sel = 3; but in slow motion case, tg_en must be on(needed to be confirmed)
    if(this->SubSample){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN, 1);
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN, 1);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN, 0);
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN, 0);
    }
#else
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN, 1);
#endif

    if (CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_RRZO_FG_MODE) == 1) {
        XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_HDS_R1_EN, 1);
        XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GSE_R1_EN, 1);
    }

    //set last p1 done
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_PASS1_DONE_SEL,0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL,GetP1DoneRef(this->m_hwModule));
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_PASS1_DONE_SEL,0);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_LAST_PASS1_DONE_CTL,GetP1DoneRef(this->m_hwModule));

    //set db_load_hold defult value, according to isp_reg_if_cam.h result
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DB_SUB_SEL,0xF3FFFB73);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DB2_SUB_SEL,0x077FD5CF);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DB3_SUB_SEL,0x6E3FDBF1);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DB4_SUB_SEL,0xFFFFFFFF);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DB5_SUB_SEL,0xFFFFFFFF);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DB6_SUB_SEL,0xFFFFFFFF);

    //ctl sel
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,this->cam_top_ctl.CTRL_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL2,this->cam_top_ctl.CTRL_SEL2.Raw);
    CAM_WRITE_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,this->cam_top_ctl.CTRL_SEL.Raw);
    CAM_WRITE_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL2,this->cam_top_ctl.CTRL_SEL2.Raw);

    //inte en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_INT_EN,int_en);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_INT4_EN,0xFFFFFFFF);
    CAM_WRITE_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_INT_EN,int_en);
    CAM_WRITE_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_INT4_EN,0xFFFFFFFF);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_INT5_EN, 0xFFFFFFFF);

    //db_en
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,1);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,1);

#ifdef LMV_DB_SW_WORK_AROUND
    //CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),LMV_R1_LMV_LMV_IMAGE_CTRL,DB_LOAD_MUX,1);
    CAM_WRITE_BITS(this->m_pDrv, LMV_R1_LMV_LMV_IMAGE_CTRL, DB_LOAD_MUX, 1);
#endif

#ifdef DCIF_STT_SW_WORK_AROUND
    // top_ctrl reg byass db
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_BYPASS_CAMCTL,1);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_BYPASS_CAMCTL,1);
#endif

    //reset FH
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,0);
    CAM_WRITE_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,0);

    //reset dma en
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA2_EN,0);
    CAM_WRITE_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,0);
    CAM_WRITE_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA2_EN,0);

    //subsample p1 done
    if(this->SubSample){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN,1);
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN,1);
    }
    else{
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN,0);
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN,0);
    }

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD,this->SubSample);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_PERIOD,this->SubSample);

    //special
    //bit31 for smi mask, blocking imgo output
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV11,(CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV11)&0x7fffffff));
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_DMA_RSV16,0xffffffff);

    //dma performance, this register in CAM_B is useless, CAM_B's performance is controlled by CAM_A's special_fun_en
    //and this register can't be modified when CAM_B is running. so always program CAM_A's special_func_en even if
    //current module is CAM_B

    switch(this->m_hwModule){
        case CAM_B:
            {
                IspDrv* ptr = IspDrvImp::createInstance(CAM_A);
                REG_CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1 spe_fun;
                ptr->init("SPEC_FUNC");
                spe_fun.Raw = CAM_READ_REG(ptr,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1);
                spe_fun.Bits.CAMDMATOP1_CQ_ULTRA_BPCI_R1_EN = 0x1;
                spe_fun.Bits.CAMDMATOP1_CQ_ULTRA_LSCI_R1_EN = 0x1;
                spe_fun.Bits.CAMDMATOP1_MULTI_PLANE_ID_EN_COMMON = 0x1;
                CAM_WRITE_REG(ptr,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,spe_fun.Raw);
                ptr->uninit("SPEC_FUNC");
                ptr->destroyInstance();

                //dma performance setting can't be runtime changing.
                //SC suggests that set default value to 1 at config stage by phy.,
                //and program it by CQ must also be 1.
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,spe_fun.Raw);
                CAM_WRITE_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,spe_fun.Raw);
            }
            break;
        case CAM_A:
        case CAM_C:
            {
                REG_CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1 spe_fun;
                spe_fun.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1);
                spe_fun.Bits.CAMDMATOP1_CQ_ULTRA_BPCI_R1_EN = 0x1;
                spe_fun.Bits.CAMDMATOP1_CQ_ULTRA_LSCI_R1_EN = 0x1;
                spe_fun.Bits.CAMDMATOP1_MULTI_PLANE_ID_EN_COMMON = 0x1;
                CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,spe_fun.Raw);
                CAM_WRITE_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_SPECIAL_FUN_EN1,spe_fun.Raw);
            }
            break;
        //case CAM_C TBD
        default:
            CAM_FUNC_ERR("unsupported hwModule(%d)", this->m_hwModule);
            return -1;
            break;
    }

    //set RRZ default setting
    rrz_ctl.m_pIspDrv = this->m_pDrv;
    rrz_ctl.setDefault();

#if TOP_DCM_SW_WORK_AROUND
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_RAW_DCM_DIS,0x1);
#endif
    //
    CAM_FUNC_DBG("-");
    return 0;
}

MINT32 CAM_TOP_CTRL::_enable(void* pParam)
{
#define SET_FIFO_FULL_DROP(DMAO,_R) do{\
    MUINT32 _syy = CAM_READ_BITS(this->m_pDrv->getPhyObj(),FBC_R1_FBC_##DMAO##_##_R##_CTL1,FBC_##DMAO##_##_R##_FBC_EN);\
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FBC_GROUP,CAMCTL_##DMAO##_##_R##_FIFO_FULL_DROP,_syy);} while(0)

    E_ISP_CAM_CQ cq;
    MUINT32 page = 0, _tmp = 0, dma_en = 0, uni_dma_en = 0, done_sel = 0, uni_sel = 0;
    MBOOL wakelock = MTRUE;
    SecMgr* pSec = SecMgr::SecMgr_GetMgrObj();
    pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_TOP_CTRL::_enable: + cq:0x%x,page:0x%x p1_done_sel(0x%x)\n",
        cq, page, CAM_READ_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE));

    //register dump before start()
    if ( MTRUE == func_cam_DbgLogEnable_VERBOSE) {
        CAM_FUNC_DBG("DUMP CAM REGISTER BEFORE START\n");
        //dump phy
        this->m_pDrv->DumpReg(MTRUE);
    }
    //dump CQ
    if ( MTRUE == func_cam_DbgLogEnable_DEBUG) {
        this->m_pDrv->dumpCQTable();
    }

    //signal control, can't run-time change
    SET_FIFO_FULL_DROP(IMGO,R1);
    SET_FIFO_FULL_DROP(RRZO,R1);
    SET_FIFO_FULL_DROP(UFEO,R1);
    SET_FIFO_FULL_DROP(UFGO,R1);
    SET_FIFO_FULL_DROP(LCESO,R1);
    SET_FIFO_FULL_DROP(LMVO,R1);
    SET_FIFO_FULL_DROP(RSSO,R1);
    SET_FIFO_FULL_DROP(YUVO,R1);
    SET_FIFO_FULL_DROP(CRZO,R1);
    SET_FIFO_FULL_DROP(CRZO,R2);

    //change CQ load mode before streaming start.
    //because this bit is shared to all cq, so put here to make sure all cq timing
    if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,CAMCTL_DOWN_SAMPLE_EN) == 1){
        //load mode = 0 if the latch-timing of the cq-baseaddress-reg is by sw p1_done
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,0);
    }
    else{
        //load mode = 1 if the latch-timing of the cq-baseaddress-reg is by hw p1_done
        //when mode = 0 , inner reg is useless
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_EN,CAMCQ_CQ_DB_LOAD_MODE,1);
    }

    CAM_FUNC_DBG("-\n");
    return 0;
}

MINT32 CAM_TOP_CTRL::_disable( void* pParam )
{
#define IS_TG_IDLE(_TGStat)( \
    (_TGStat.Bits.TG_TG_CAM_CS == 1) && \
    (_TGStat.Bits.TG_SYN_VF_DATA_EN == 0) && \
    (_TGStat.Bits.TG_OUT_REQ == 0))

    E_ISP_CAM_CQ cq;
    MUINT32 page = 0, bTwin = 0, i = 0,_clrPort = 2,_dma[2] = {_imgo_,_rrzo_};
    MBOOL wakelock = MFALSE;
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_TOP_CTRL::_disable: + cq:0x%x,page:0x%x,force:%d\n",cq,page);

    bTwin = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_EN);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_MISC,CAMCTL_DB_EN,0); //disable double buffer
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCQ_R1_CAMCQ_CQ_THR0_CTL,CAMCQ_CQ_THR0_EN,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SW_PASS1_DONE,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DONE_SEL,0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_DMA_EN,0x0);

    /// !!!!!!!!!!! Notice !!!!!!!!!!!!
    /// Must disable Salve Cam's DMA_EN as well
    /// Or, you'll see P1_Done (both SW and HW) keep coming...
    /// P1_Done criterion: (a) FBC_EN = 0, (b) DMA_EN = 1, (c) CAMCTL_TG_R1_EN = 0
    /// E.x.: Twin CAMA,CAMB + AFO CAMA,CAMB: CAMB AFO_EN = 1, AFO_FBC_CTRL = 0, CAMA CAMCTL_TG_R1_EN = 0 => CAMA keep coming P1_Done.
    if(0x1 == bTwin) {
        MUINT32 slave_num = 0;
        ISP_HW_MODULE slaveHwModule = CAM_MAX;

        slave_num = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,SLAVE_CAM_NUM);

        for(i = 0; i < slave_num; i++){
            if(i == 0){
                slaveHwModule = (ISP_HW_MODULE)CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_TWIN_STATUS,TWIN_MODULE);
            }
            else{
                CAM_FUNC_ERR("[%d]3-raw is not supported(%d)!!!\n", i, slaveHwModule);
            }
            CAM_FUNC_DBG("[%d]Close DMA_EN of Slave cam(%d)!!!\n", i, slaveHwModule);

            IspDrv* ptr = IspDrvImp::createInstance(slaveHwModule);
            if (ptr != NULL) {
                MUINT32 tmp;
                ptr->writeReg(0x8,0x0);
                ptr->destroyInstance();
            }
        }
    }

    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN,0x0);

    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2,0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN3,0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN4,0x0);

    //close  fbc
    //FBC of STT pipe will be closed at STT pipe
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_IMGO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_RRZO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_LCESO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_UFEO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_UFGO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_LMVO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_RSSO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_YUVO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_CRZO_R1_CTL1, 0x0);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), FBC_R1_FBC_CRZO_R2_CTL1, 0x0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_MISC, CAMCTL_DB_EN, 1); //Enable double buffer

    while(_clrPort--){
        buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
        buf_ctrl.buf_id = (_isp_dma_enum_)_dma[_clrPort];
        buf_ctrl.data_ptr = NULL;
        buf_ctrl.pExtend = 0;
        if ( MTRUE != this->m_pDrv->setDeviceInfo(_SET_BUF_CTRL,(MUINT8 *) &buf_ctrl)) {
            CAM_FUNC_ERR("ERROR:clear buf ctrl fail\n");
        }
    }

    //register dump after stop()
    if ( MTRUE == func_cam_DbgLogEnable_VERBOSE) {
        CAM_FUNC_DBG("DUMP CAM REGISTER BEFORE START\n");
        //dump phy
        this->m_pDrv->DumpReg(MTRUE);
        //dump CQ
        this->m_pDrv->dumpCQTable();
    }

    //free ion handle by hw module
    this->m_pDrv->setDeviceInfo(_SET_ION_FREE_BY_HWMODULE,(MUINT8*)&this->m_hwModule);

    CAM_FUNC_DBG("-\n");
    return 0;
}

MINT32 CAM_TOP_CTRL::_write2CQ(MUINT32 BurstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_TOP_CTRL::_write2CQ + cq:0x%x,page:0x%x\n",cq,page);

    /*For SMVR, only add module RCNT_INC for after 2nd burst*/
    if (0 == BurstIndex) {
        this->m_pDrv->cqAddModule(CAM_CTL_EN_);
        this->m_pDrv->cqAddModule(CAM_CTL_FMT_);
        this->m_pDrv->cqAddModule(CAM_CTL_SEL_);
        this->m_pDrv->cqAddModule(CAM_DMA_FH_EN_);
        this->m_pDrv->cqAddModule(CAM_TWIN_INFO);
#if (P1_STANDBY_DTWIN_SUPPORT == 1)
        this->m_pDrv->cqAddModule(CAM_CTL_INT_EN_);
        this->m_pDrv->cqAddModule(CAM_CTL_INT2_EN_);
        this->m_pDrv->cqAddModule(CAM_CTL_INT4_EN_);
        this->m_pDrv->cqAddModule(CAM_CTL_SW_DONE_SEL_);
#endif
    }

    this->m_pDrv->cqAddModule(CAM_CTL_FBC_RCNT_INC_);


    CAM_FUNC_DBG("-\n");

    return 0;
}

//set 0:stop pipeline. set 1:restart pipeline
MBOOL CAM_TOP_CTRL::HW_recover(MUINT32 step)
{
    string _str;
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    E_INPUT CurTG = TG_CAM_MAX;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CurTG = (E_INPUT)CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_TWIN_STATUS, SRC_TG);

    if (MAP_MODULE_TG(this->m_hwModule, MTRUE, CAM_FUNC_ERR) != CurTG) {
        CAM_FUNC_ERR("HW Recover only support Raw with self-TG");
        return MFALSE;
    }

    switch(step) {
        case 0:
            {
                MBOOL rst = MTRUE;
                MUINT32 i = 0 , flags[2];

                CAM_FUNC_INF("CAM_TOP_CTRL::stop & reset + \n");

                for (i = 0; i < _dmao_max_; i++){
                    IspFunction_B::fbcPhyLock[this->m_hwModule][i].lock();
                }

                SAVE_FBC(this->m_pDrv->getPhyObj(),IMGO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),RRZO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),UFEO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),UFGO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),AFO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),AAO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),TSFSO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),LCESO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),PDO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),LMVO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),FLKO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),RSSO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),YUVO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),YUVBO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),YUVCO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),CRZO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),CRZO_R2,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),CRZBO_R1,_str);
                SAVE_FBC(this->m_pDrv->getPhyObj(),CRZBO_R2,_str);

                CAM_FUNC_INF("CAM_TOP_CTRL::restart SAVE_FBC: %s",_str.c_str());

                //close vf
                this->m_pDrv->setDeviceInfo(_SET_VF_OFF,(MUINT8*)&this->m_hwModule);
                SW_RESET(this->m_pDrv->getPhyObj());

                return rst;
            }
            break;
        case 1:
            {
                MUINT32 i = 0;

                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),IMGO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),RRZO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),UFEO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),UFGO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),AFO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),AAO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),TSFSO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),LCESO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),PDO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),LMVO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),FLKO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),RSSO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),YUVO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),YUVBO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),YUVCO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),CRZO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),CRZO_R2,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),CRZBO_R1,_str);
                PULL_RCNT_INC(this->m_pDrv->getPhyObj(),CRZBO_R2,_str);


                for (i = 0; i < _dmao_max_; i++){
                    IspFunction_B::fbcPhyLock[this->m_hwModule][i].unlock();
                }
                CAM_FUNC_INF("CAM_TOP_CTRL::restart PHYS_FBC: %s",_str.c_str());

                /* restore FBC FIFO_FULL_DROP */
                {
                    CAM_TIMESTAMP* pTime = CAM_TIMESTAMP::getInstance(this->m_hwModule,this->m_pDrv);
                    pTime->TimeStamp_reset();
                }

                //vf
                this->m_pDrv->setDeviceInfo(_SET_VF_ON,(MUINT8*)&this->m_hwModule);

                CAM_FUNC_DBG("CAM_TOP_CTRL::- \n");
                return MTRUE;
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported:%d\n",step);
            return MFALSE;
            break;
    }
    return MTRUE;
}

MINT32 CAM_DCIF_CTRL::_config(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN,CAMCTL_TG_R1_EN, 0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_STAG_MODE , 0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_MISC, CAMCTL_CAMSV_DC_MODE, 1);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_EN_SEL, 0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_SEL , 0);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN , 1);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2, CAMDMATOP1_DC_CAMSV_STAGER_EN_RAWI_R2, 1);
    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2, CAMDMATOP1_DC_CAMSV_STAGER_DLNO_RAWI_R2, 8);

    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_TG_R1_EN, 0);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_STAG_MODE , 0);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_CAMSV_DC_MODE, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_EN_SEL, 0);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_SEL , 0);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN , 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2, CAMDMATOP1_DC_CAMSV_STAGER_EN_RAWI_R2, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2, CAMDMATOP1_DC_CAMSV_STAGER_DLNO_RAWI_R2, 8);

    // config TG size for pipe-check code re-use
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_GRAB_PXL, this->m_SrcSize.w << 16);
    CAM_WRITE_REG(this->m_pDrv->getPhyObj(), TG_R1_TG_SEN_GRAB_LIN, this->m_SrcSize.h << 16);

    if(m_bSvTwin){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_SUB_SEL , 1);
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_SUB_SEL , 1);
    }
    return 0;
}
MINT32 CAM_DCIF_CTRL::_enable(void* pParam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);


    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN, 1);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN, 1);
    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN, 1);

    if(m_bSvTwin){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_SUB_EN, 1);
        CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_SUB_EN, 1);
    }
    if(m_bRawInUF){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_UFDI_R2, CAMDMATOP1_DC_CAMSV_STAGER_EN_UFDI_R2, 1);
        CAM_WRITE_BITS(this->m_pDrv, CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_UFDI_R2, CAMDMATOP1_DC_CAMSV_STAGER_EN_UFDI_R2, 1);
    }

    return 0;
}
MINT32 CAM_DCIF_CTRL::_disable(void* pParam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_SEL2, CAMCTL_DCIF_MAIN_EN, 0);

    CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_RAWI_R2, CAMDMATOP1_DC_CAMSV_STAGER_EN_RAWI_R2, 0);

    if(m_bRawInUF){
        CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), CAMDMATOP1_R1_CAMDMATOP1_DMA_DC_CAMSV_STAGER_UFDI_R2, CAMDMATOP1_DC_CAMSV_STAGER_EN_UFDI_R2, 0);
    }

    return 0;
}
MINT32 CAM_DCIF_CTRL::_write2CQ(MUINT32 burstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    this->m_pDrv->cqAddModule(CAM_DMA_CAMSV_STAGER_);

    return 0;
}

MINT32 CAM_MAGIC_CTRL::_config(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    Header_IMGO  fh_imgo;
    Header_RRZO  fh_rrzo;
    Header_UFEO  fh_ufeo;
    Header_UFGO  fh_ufgo;
    Header_LCSO  fh_lcso;
    Header_LMVO  fh_lmvo;
    Header_RSSO  fh_rsso;
    Header_AAO   fh_aao;
    Header_TSFSO fh_tsfso;
    Header_PDO   fh_pdo;
    Header_AFO   fh_afo;
    Header_FLKO  fh_flko;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_MAGIC_CTRL::_config+ cq:0x%x,page:0x%x,magic:0x%x\n",cq,page,this->m_nMagic);

    fh_imgo.Header_Enque(Header_IMGO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_rrzo.Header_Enque(Header_RRZO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_ufeo.Header_Enque(Header_UFEO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_ufgo.Header_Enque(Header_UFGO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_lcso.Header_Enque(Header_LCSO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_aao.Header_Enque(Header_AAO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_tsfso.Header_Enque(Header_TSFSO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_pdo.Header_Enque(Header_PDO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_afo.Header_Enque(Header_AFO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_lmvo.Header_Enque(Header_LMVO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_flko.Header_Enque(Header_FLKO::E_Magic,this->m_pDrv,this->m_nMagic);
    fh_rsso.Header_Enque(Header_RSSO::E_Magic,this->m_pDrv,this->m_nMagic);
    CAM_WRITE_REG(this->m_pDrv, CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM, this->m_nMagic);

    return 0;
}

MINT32 CAM_MAGIC_CTRL::_write2CQ(MUINT32 BurstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);
    CAM_FUNC_DBG("CAM_MAGIC_CTRL::_write2CQ+ cq:0x%x,page:0x%x,Burst:%d\n",cq,page,BurstIndex);
    if (BurstIndex > 0) {
        /*For SMVR, only add module at normal page*/
        goto EXIT;
    }
    this->m_pDrv->cqAddModule(CAM_MAGIC_NUM_);
    //
    CAM_FUNC_DBG("-\n");

EXIT:

    return 0;
}

MINT32 CAM_RRZ_CTRL::_config(void)
{
    MINT32 ret = 0;
    IspSize rrz_in_size;
    IspRect rrz_in_roi_with_bin;
    MUINT32 rrz_h_step = 0, rrz_v_step = 0, rrz_nnir = 0;
    MUINT32 page = 0;
    REG_RRZ_R1_RRZ_CTL rrz_ctl;
    E_ISP_CAM_CQ cq;
    Header_RRZO fh_rrzo;
    RectData<MUINT32> Bin;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    // If DBN=1, rrz in setting must be divide by 2; if DBN = 1 and BIN = 2, imgo setting must be divide by 4
    GET_SEP_BIN(this->m_pDrv,Bin);

    rrz_in_size.w = (CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
                     CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X) + 1) >> Bin.H;
    rrz_in_size.h = CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> Bin.V;

    rrz_in_roi_with_bin.x = this->rrz_in_roi.x;
    rrz_in_roi_with_bin.y = this->rrz_in_roi.y;
    rrz_in_roi_with_bin.floatX = this->rrz_in_roi.floatX;
    rrz_in_roi_with_bin.floatY = this->rrz_in_roi.floatY;
    rrz_in_roi_with_bin.w = this->rrz_in_roi.w;
    rrz_in_roi_with_bin.h = this->rrz_in_roi.h;

    /* PicQ & one time config move to CAM_TOP_CTL::config */
    rrz_h_step = this->calCoefStep(rrz_in_size.w,rrz_in_roi_with_bin.w,rrz_in_roi_with_bin.x,rrz_in_roi_with_bin.floatX,this->rrz_out_size.w);
    rrz_v_step = this->calCoefStep(rrz_in_size.h,rrz_in_roi_with_bin.h,rrz_in_roi_with_bin.y,rrz_in_roi_with_bin.floatY,this->rrz_out_size.h);
    rrz_ctl.Bits.RRZ_HORI_TBL_SEL = this->getCoefTbl(rrz_in_roi_with_bin.w,this->rrz_out_size.w);
    rrz_ctl.Bits.RRZ_VERT_TBL_SEL = this->getCoefTbl(rrz_in_roi_with_bin.h,this->rrz_out_size.h);

    float ratio_w, ratio_h, ratio;
    MUINT32 rrz_h_tbl_sel, rrz_v_tbl_sel;
    MUINT32 rrz_h_tbl_sel_default = 12;
    MUINT32 rrz_v_tbl_sel_default = 12;

    rrz_h_tbl_sel = (rrz_ctl.Bits.RRZ_HORI_TBL_SEL >= rrz_h_tbl_sel_default) ? rrz_ctl.Bits.RRZ_HORI_TBL_SEL:rrz_h_tbl_sel_default;
    rrz_v_tbl_sel = (rrz_ctl.Bits.RRZ_VERT_TBL_SEL >= rrz_v_tbl_sel_default) ? rrz_ctl.Bits.RRZ_VERT_TBL_SEL:rrz_v_tbl_sel_default;

    ratio_w = (10 * (float)this->rrz_out_size.w / (float)rrz_in_roi_with_bin.w);
    ratio_h = (10 * (float)this->rrz_out_size.h / (float)rrz_in_roi_with_bin.h);

    ratio = (ratio_w >= ratio_h) ? ratio_w:ratio_h;

    CAM_FUNC_DBG("ratio_w:%f ratio_h:%f ratio:%f\n", ratio_w, ratio_h, ratio);

    if(ratio < 6){
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_NNIR,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_SL,16);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_TH,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_HORI_TBL_SEL, rrz_h_tbl_sel);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_VERT_TBL_SEL, rrz_v_tbl_sel);
    }
    else if((ratio >= 6) && (ratio < 7)){

        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_NNIR,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_SL,16);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_TH,256);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_HORI_TBL_SEL, rrz_h_tbl_sel);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_VERT_TBL_SEL, rrz_v_tbl_sel);
    }
    else if((ratio >= 7) && (ratio < 8)){

        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_NNIR,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_SL,16);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_TH,512);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_HORI_TBL_SEL, rrz_h_tbl_sel);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_VERT_TBL_SEL, rrz_v_tbl_sel);
    }
    else if((ratio >= 8) && (ratio < 9)){

        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_NNIR,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_SL,16);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_TH,768);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_HORI_TBL_SEL, rrz_h_tbl_sel);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_VERT_TBL_SEL, rrz_v_tbl_sel);
    }
    else if((ratio >= 9) && (ratio <= 10)){

        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_NNIR,0);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_SL,16);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_TH,1023);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_HORI_TBL_SEL, rrz_h_tbl_sel);
        CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_VERT_TBL_SEL, rrz_v_tbl_sel);
    }

    else{
        CAM_FUNC_ERR("rrz_ratio not support adjust this setting");
    }

    CAM_FUNC_DBG("RRZ reg set by ratio 1.RRZ_CL:%u 2.RRZ_NNIR:%u 3.RRZ_LBLD_SL:%u 4.RRZ_LBLD_TH:%u\n",
        CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL),
        CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_NNIR),
        CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_SL),
        CAM_READ_BITS(this->m_pDrv, RRZ_R1_RRZ_LBLD_CFG,RRZ_LBLD_TH));


    rrz_ctl.Bits.RRZ_HORI_EN = 1;
    rrz_ctl.Bits.RRZ_VERT_EN = 1;
    rrz_nnir = (this->getCoefNNIRTbl(rrz_ctl.Bits.RRZ_HORI_TBL_SEL)) | \
               (this->getCoefNNIRTbl(rrz_ctl.Bits.RRZ_VERT_TBL_SEL)<<8);

    CAM_FUNC_INF("CAM_RRZ_CTRL::_config+ cq:0x%x,page:0x%x,rrz_in_w/h(%ld/%ld),rrz_crop_x/y/fx/fy/w/h(%d/%d/%d/%d/%ld/%ld),rrz_out_w/h(%ld/%ld),rlb_offset(%d),rrz_h/v_step(%d/%d),dbin/bin/fbnd(%d/%d/%d)m_bDYUV(%d)\n",
            cq,page,\
            rrz_in_size.w, \
            rrz_in_size.h, \
            this->rrz_in_roi.x, \
            this->rrz_in_roi.y, \
            this->rrz_in_roi.floatX, \
            this->rrz_in_roi.floatY, \
            this->rrz_in_roi.w, \
            this->rrz_in_roi.h, \
            this->rrz_out_size.w, \
            this->rrz_out_size.h, \
            this->rrz_rlb_offset, \
            rrz_h_step, rrz_v_step,
            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN),
            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN),
            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN),
            m_bDYUV);

    //CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_HORI_TBL_SEL, rrz_ctl.Bits.RRZ_HORI_TBL_SEL);
    //CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_VERT_TBL_SEL, rrz_ctl.Bits.RRZ_VERT_TBL_SEL);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_HORI_EN, rrz_ctl.Bits.RRZ_HORI_EN);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_VERT_EN, rrz_ctl.Bits.RRZ_VERT_EN);
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_IN_IMG, ((rrz_in_size.h<<16) | rrz_in_size.w));
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG, ((this->rrz_out_size.h<<16) | this->rrz_out_size.w) );
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_HORI_STEP, rrz_h_step);
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_VERT_STEP, rrz_v_step);
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_HORI_INT_OFST, rrz_in_roi_with_bin.x);
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_HORI_SUB_OFST, rrz_in_roi_with_bin.floatX);
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_VERT_INT_OFST, rrz_in_roi_with_bin.y);
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_VERT_SUB_OFST, rrz_in_roi_with_bin.floatY);
    CAM_WRITE_REG(this->m_pDrv, RRZ_R1_RRZ_NNIR_TBL_SEL, rrz_nnir);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL, RRZ_MONO, (this->m_DataPat == eCAM_MONO)?1:0);

    //update rmx crop
    XOR_WRITE_REG(this->m_hwModule,this->m_pDrv,MRG_R7_MRG_CROP,( ((this->rrz_out_size.w-1)<<16) | 0) );
    XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,MRG_R7_MRG_VSIZE,MRG_HT,this->rrz_out_size.h);
    XOR_WRITE_REG(this->m_hwModule,this->m_pDrv,MRG_R11_MRG_CROP,( ((this->rrz_out_size.w-1)<<16) | 0) );
    XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,MRG_R11_MRG_VSIZE,MRG_HT,this->rrz_out_size.h);
    XOR_WRITE_REG(this->m_hwModule,this->m_pDrv,MRG_R8_MRG_CROP,( ((this->rrz_out_size.w-1)<<16) | 0) );
    XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,MRG_R8_MRG_VSIZE,MRG_HT,this->rrz_out_size.h);

    CAM_WRITE_BITS(this->m_pDrv,RRZ_R1_RRZ_RLB_AOFST,RRZ_RLB_AOFST,this->rrz_rlb_offset);

    fh_rrzo.Header_Enque(Header_RRZO::E_RRZ_CRP_START,this->m_pDrv,((rrz_in_roi_with_bin.y<<16)|rrz_in_roi_with_bin.x));
    fh_rrzo.Header_Enque(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv,((rrz_in_roi_with_bin.h<<16)|rrz_in_roi_with_bin.w));
    fh_rrzo.Header_Enque(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv,((this->rrz_out_size.h<<16) | this->rrz_out_size.w));

    //rrz out size might be changed, need update if DYUV src is through rrz
    this->m_Dyuv.updateMrgInSize();

    this->m_pDrv->cqApbModule(CAM_ISP_RRZ_);
    return ret;
}

MINT32 CAM_RRZ_CTRL::setDefault(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    RRZ_PQ PicQ;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //set RRZ & RRZ.picQ default value
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_VDM_VRZ_MODE, 1);//for power saving
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL,0);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_CL_HL,0);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_CTL,RRZ_NNIR,0);

    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_CTL,RRZ_PRF_BLD, PicQ.PRF_BLD);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_CTL,RRZ_PRF, PicQ.PRF);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_CTL,RRZ_BLD_SL, PicQ.BLD_SL);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_CTL,RRZ_CR_MODE, PicQ.CR_MODE);

    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_TH,RRZ_TH_MD, PicQ.TH_MD);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_TH,RRZ_TH_HI, PicQ.TH_HI);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_TH,RRZ_TH_LO, PicQ.TH_LO);
    CAM_WRITE_BITS(this->m_pDrv, RRZ_R1_RRZ_MODE_TH,RRZ_TH_MD2, PicQ.TH_MD2);

    return 0;

}

inline MUINT32 CAM_RRZ_CTRL::calCoefStep(MUINT32 in,MUINT32 crop,MUINT32 crop_ofst,MUINT32 crop_sub_ofst,MUINT32 out)
{
    MUINT32 const RRZ_PREC = 32768;

    // Add to fix devide by zero NE issue caused by /(out -1)
    if (out <= 1){
        out = 2;
        CAM_FUNC_INF("out <=1, reset to 2!");
    }

    MUINT32 rrz_step = ( ( in - 1 - crop_ofst - ( in- crop_ofst - crop) - (crop_sub_ofst?1:0) ) )*RRZ_PREC  / (out - 1);

    return rrz_step;
}

MUINT32 CAM_RRZ_CTRL::getCoefTbl(MUINT32 resize_in,MUINT32 resize_out)
{
    /*
       resizing ratio  suggested table
       >=1.0           31
       0.99~0.9        0~3
       0.9~0.8         4~7
       0.8~0.7         8~11
       0.7~0.6         12~15
       0.6~0.5         16~19
       0.5~0.4         20~23
       0.4~0.3         24~27
       0.3~0.25        28~30
       <0.25           30       //for non-viewing resize
     */

    struct {
        MUINT32 left;
        MUINT32 right;
    }map_tbl[10] = {{1,1},{1,1},{28,30},{24,27},{20,23},{16,19},{12,15},{8,11},{4,7},{0,3}};

    MUINT32 scale_ratio_int = (MUINT32)( (resize_out*10) / resize_in );
    MUINT32 scale_ratio_int_100 = (MUINT32)( (resize_out*100) / resize_in );
    MUINT32 ref = (scale_ratio_int == 2) ? 25 : scale_ratio_int*10;
    MUINT32 div = (scale_ratio_int == 2) ? 5 : 10;
    MUINT32 table = 0;
    capibility CamInfo;
    tCAM_rst rst;
    MBOOL ret;

    CAM_FUNC_DBG("+in/out(%d/%d),ratio(%d)",resize_in,resize_out,scale_ratio_int);

    if ( 0 == resize_in || 0 == resize_out ) {
        CAM_FUNC_ERR("rrz param error:0 size(%d/%d)",resize_in,resize_out);
        return 0;
    }
    if ( resize_out > resize_in ) {
        CAM_FUNC_ERR("rrz param error:not support size up(%d/%d)",resize_in,resize_out);
        return 0;
    }

    ret = CamInfo.GetCapibility(NSImageio::NSIspio::EPortIndex_RRZO, \
                                ENPipeQueryCmd_BS_RATIO,\
                                NormalPipe_InputInfo(),rst,E_CAM_UNKNOWNN);
    if (MFALSE == ret) {
        CAM_FUNC_ERR("GetCapibility fail");
        return 0;
    }
    if ( rst.ratio > scale_ratio_int_100 ){//unit is %
        CAM_FUNC_ERR("rrz param error:max (0.%02d)x(%d/%d)", rst.ratio, resize_in,resize_out);
        return 0;
    }
    if ( 25 > scale_ratio_int_100 ) {
        table = 0x1E; //table index 30
    }
    else if ( resize_out == resize_in ) {
        table = 0x1F; //table index 31
    }
    else {
        /* +1: for filtering averagely each interval*/
        table = map_tbl[scale_ratio_int].right - \
              ( ( scale_ratio_int_100 - ref )* \
                ( map_tbl[scale_ratio_int].right - map_tbl[scale_ratio_int].left + 1 ) ) / div;
        table = (table > 0x1f)? (0x1f):(table);
    }

    CAM_FUNC_DBG("table(%d)",table);

    return table;
}

inline MUINT32 CAM_RRZ_CTRL::getCoefNNIRTbl(MUINT32 tableSel)
{
    if(tableSel == 31)//table index 31 is for resize_out == resize_in
        return 31;
    else
        return (tableSel + 32);
}

MINT32 CAM_CRP_R1_CTRL::_config(void)
{
    MINT32 ret = 0;
    MUINT32 tmp = 0;
    RectData<MUINT32> Bin;
    E_PIX pix_mode = _1_pix_;

    GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);
    pix_mode = (E_PIX)(CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_SEPO) >> Bin.H);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_CRP_R1_EN, 1);

    //rcp crp
    tmp = (CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
           CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1;
    tmp = tmp >> (Bin.H + (MUINT32)pix_mode);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R1_CRP_X_POS,CRP_XSTART,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R1_CRP_X_POS,CRP_XEND,tmp - 1);

    tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> Bin.V;

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R1_CRP_Y_POS,CRP_YSTART,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R1_CRP_Y_POS,CRP_YEND,tmp - 1);
    return ret;
}

MINT32 CAM_CRP_R3_CTRL::_config()
{
    MINT32 ret = 0;
    E_PIX pix_mode = _1_pix_;
    RectData<MUINT32> Bin;
    MSize src_size;

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN, CAMCTL_CRP_R3_EN, 1);

    //rcp3 crp
    switch(CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_CRP_R3_SEL))
    {
        case E_CRP_R3_SEL_0:
            src_size.w = this->m_pRaw_p->m_SrcSize.w;
            src_size.h = this->m_pRaw_p->m_SrcSize.h;
            pix_mode = (E_PIX)CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAW_SELO);
            break;
        case E_CRP_R3_SEL_1:
        case E_CRP_R3_SEL_2:
        case E_CRP_R3_SEL_3:
        case E_CRP_R3_SEL_4:
        case E_CRP_R3_SEL_5:
            GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);
            src_size.w = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X) + 1;
            src_size.h = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT);
            pix_mode = (E_PIX)((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_SEPO)) >> Bin.H);
            break;
        case E_CRP_R3_SEL_6:
            GET_FULL_BIN(this->m_pRaw_p->m_pDrv,Bin,0);
            src_size.w = this->m_pRaw_p->m_SrcSize.w;
            src_size.h = this->m_pRaw_p->m_SrcSize.h;
            pix_mode = (E_PIX)((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAW_SELO)) >> Bin.H);
            break;
        case E_CRP_R3_SEL_7:
            GET_FULL_BIN(this->m_pRaw_p->m_pDrv,Bin,1);
            src_size.w = this->m_pRaw_p->m_SrcSize.w;
            src_size.h = this->m_pRaw_p->m_SrcSize.h;
            pix_mode = (E_PIX)((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAW_SELO)) >> Bin.H);
            break;
        default:
            CAM_FUNC_ERR("unsupported sel(%d), rcp3 fail\n",CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_CRP_R3_SEL));
            ret = 1;
            goto EXIT;
            break;

    }

    src_size.w = (src_size.w >> (Bin.H + (MUINT32)pix_mode));//in case of rcp3_2 + bin_on
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R3_CRP_X_POS,CRP_XSTART,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R3_CRP_X_POS,CRP_XEND,src_size.w - 1);

    src_size.h = src_size.h >> Bin.V;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R3_CRP_Y_POS,CRP_YSTART,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CRP_R3_CRP_Y_POS,CRP_YEND,src_size.h - 1);

EXIT:
    return ret;
}

MINT32 CAM_SEP_CTRL::_config(void)
{
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;
    MINT32 ret = 0;
    MUINT32 _tmp = 0;
    RectData<MUINT32> Bin;

    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(), rst, E_CAM_SRAM_SEP);

    GET_FULL_BIN(this->m_pRaw_p->m_pDrv,Bin,1);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_SEP_R1_EN, 1);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, SEP_R1_SEP_CTL, SEP_SRAM_SIZE, rst.dlp.module.sram_sep);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, SEP_R1_SEP_CTL, SEP_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, SEP_R1_SEP_CTL, SEP_EDGE_SET, 0);

    _tmp = this->m_pRaw_p->m_SrcSize.w >> Bin.H;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, SEP_R1_SEP_CROP, SEP_STR_X,0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, SEP_R1_SEP_CROP, SEP_END_X, _tmp - 1);

    _tmp = this->m_pRaw_p->m_SrcSize.h >> Bin.V;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, SEP_R1_SEP_VSIZE, SEP_HT, _tmp);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_SEPO, MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.SEPO,CAM_FUNC_INF));
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_SEPI, MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.SEPI,CAM_FUNC_INF));

    return ret;
}

#define UPDATE_QBIN_ACC(EN,RX,pixmode)\
do {\
    if(CAM_READ_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, TWIN_EN)){\
        switch(pixmode) {\
        case _1_pix_:\
        case _2_pix_:\
            pixmode++;\
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_##EN, CAMCTL_QBN_##RX##_EN, MTRUE);\
            CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, QBIN_##RX##_QBIN_CTL, QBIN_ACC, pixmode); break;\
        default: CAM_FUNC_WRN("QBN_%s: Already use max pixel mode\n", #RX); break;\
        }\
    }\
}while(0)

MINT32 CAM_MRG_R1_CTRL::_config(void)
{
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;
    MINT32 ret = 0;
    MUINT32 scaling_ratio = 0, page = 0, _tmp = 0;
    E_ISP_CAM_CQ cq;
    RectData<MUINT32> Bin;

    this->m_pRaw_p->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);

    CamInfo.GetCapibility(0,ENPipeQueryCmd_UNKNOWN,NormalPipe_InputInfo(),rst,E_CAM_SRAM_MRG);
    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN,
        NormalPipe_InputInfo((NSCam::EImageFormat)0, 0, this->m_pRaw_p->m_dlp.SEPO), rst, E_CAM_pipeline_min_size);

    /* QBIN1 ACC, 0 -> no scaling, 1->2x scaling down, 2->4x scaling down
       QBIN1 doesn't support 1 pix mode => CAM_QBIN1_MODE.QBN_ACC can't be 0. If it is, disable QBIN1 */
    scaling_ratio = (MUINT32)MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.SEPO,CAM_FUNC_INF) >> (MUINT32)Bin.H;

    /*update ACC for twin case*/
    UPDATE_QBIN_ACC(EN2, R1, scaling_ratio);

    /* for dynamically changing QBIN1 ACC, must disable DCM*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_RAW2_DCM_DIS, CAMCTL_QBN_R1_DCM_DIS,1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC_MODE,1);//always use average

    if(scaling_ratio) {
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R1_EN, MTRUE);
        //W phy is for query , vir is for twin, NOTE: ACC dynamic change, can't W phy
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC,scaling_ratio);
    }
    else{
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R1_EN, MFALSE);
    }

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_MRG_R1_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CTL,MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CTL,MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CTL,MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CTL,MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CTL,MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CTL,MRG_SIG_MODE2, 0);

    // MRG R1 cropping
    _tmp = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
            CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> (MUINT32)Bin.H;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_CROP, MRG_END_X, _tmp - 1);
    _tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> (MUINT32)Bin.V;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R1_MRG_VSIZE, MRG_HT, _tmp);

    return ret;
}

MINT32 CAM_MRG_R2_CTRL::_config(void)
{
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;
    MINT32 ret = 0;
    MUINT32 scaling_ratio = 0, _tmp = 0;
    RectData<MUINT32> Bin;

    GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);

    CamInfo.GetCapibility(0,ENPipeQueryCmd_UNKNOWN,NormalPipe_InputInfo(),rst,E_CAM_SRAM_MRG);
    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN,
        NormalPipe_InputInfo((NSCam::EImageFormat)0, 0, this->m_pRaw_p->m_dlp.SEPO), rst, E_CAM_pipeline_min_size);

    /* QBIN1 ACC, 0 -> no scaling, 1->2x scaling down, 2->4x scaling down
       QBIN1 doesn't support 1 pix mode => CAM_QBIN1_MODE.QBN_ACC can't be 0. If it is, disable QBIN1 */
    scaling_ratio = (MUINT32)MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.SEPO,CAM_FUNC_INF) >> (MUINT32)Bin.H;

    /*update ACC for twin case*/
    UPDATE_QBIN_ACC(EN2, R2, scaling_ratio);

    /* for dynamically changing QBIN2 ACC, must disable DCM*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_RAW2_DCM_DIS, CAMCTL_QBN_R2_DCM_DIS,1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R2_QBIN_CTL,QBIN_ACC_MODE,1);//always use average

    if(scaling_ratio) {
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R2_EN, MTRUE);
        //W phy is for query , vir is for twin, NOTE: ACC dynamic change, can't W phy
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R2_QBIN_CTL,QBIN_ACC,scaling_ratio);
    }
    else{
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R2_EN, MFALSE);
    }

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_MRG_R2_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CTL,MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r2);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CTL,MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CTL,MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CTL,MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CTL,MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CTL,MRG_SIG_MODE2, 0);

    // MRG R2 cropping
    _tmp = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
           CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> (MUINT32)Bin.H;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_CROP, MRG_END_X, _tmp - 1);
    _tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> (MUINT32)Bin.V;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R2_MRG_VSIZE, MRG_HT, _tmp);

    return ret;
}

MINT32 CAM_MRG_R3_CTRL::_config(void)
{
    MINT32 ret=0;
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;
    RectData<MUINT32> Bin;
    MSize _tmp;

    GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);
    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(), rst, E_CAM_SRAM_MRG);

    switch(CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_CRP_R3_SEL)) {
        case E_CRP_R3_SEL_0:
            _tmp.w = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
                       CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> (MUINT32)Bin.H;
            _tmp.h = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> (MUINT32)Bin.V;
            break;
        case E_CRP_R3_SEL_1:
        case E_CRP_R3_SEL_2:
        case E_CRP_R3_SEL_3:
        case E_CRP_R3_SEL_4:
        case E_CRP_R3_SEL_5:
            GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);
            _tmp.w = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
                       CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> (MUINT32)Bin.H;
            _tmp.h = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> (MUINT32)Bin.V;
            break;
        case E_CRP_R3_SEL_6:
            GET_FULL_BIN(this->m_pRaw_p->m_pDrv,Bin,0);
            _tmp.w = this->m_pRaw_p->m_SrcSize.w >> (MUINT32)Bin.H;
            _tmp.h = this->m_pRaw_p->m_SrcSize.h >> (MUINT32)Bin.V;
            break;
        case E_CRP_R3_SEL_7:
            GET_FULL_BIN(this->m_pRaw_p->m_pDrv,Bin,1);
            _tmp.w = this->m_pRaw_p->m_SrcSize.w >> (MUINT32)Bin.H;
            _tmp.h = this->m_pRaw_p->m_SrcSize.h >> (MUINT32)Bin.V;
            break;
        default:
            CAM_FUNC_ERR("unsupported sel(%d), rcp3 fail\n",CAM_READ_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_CRP_R3_SEL));
            ret = -1;
            break;
    }

    /*R3*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN4, CAMCTL_MRG_R3_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CTL, MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r3);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CTL, MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CTL, MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CTL, MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CTL, MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CTL, MRG_SIG_MODE2, 0);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R3O, MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.MRG_R3,CAM_FUNC_INF));

    // MRG cropping
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_CROP, MRG_END_X, _tmp.w - 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R3_MRG_VSIZE, MRG_HT, _tmp.h);


    /*R12*/
    //MRG_R12 enable at CAM_TWIN_PIPE::_config()
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CTL, MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r12);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CTL, MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CTL, MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CTL, MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CTL, MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CTL, MRG_SIG_MODE2, 0);

    // MRG cropping
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_CROP, MRG_END_X, _tmp.w - 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R12_MRG_VSIZE, MRG_HT, _tmp.h);

    return ret;
}

MINT32 CAM_MRG_R4_CTRL::_config(void)
{
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;
    MINT32 ret = 0;
    MUINT32 scaling_ratio = 0, _tmp = 0;
    RectData<MUINT32> Bin;

    GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);

    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(),rst, E_CAM_SRAM_MRG);
    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN,
        NormalPipe_InputInfo((NSCam::EImageFormat)0, 0, this->m_pRaw_p->m_dlp.SEPO), rst, E_CAM_pipeline_min_size);

    /* QBIN1 ACC, 0 -> no scaling, 1->2x scaling down, 2->4x scaling down
       QBIN1 doesn't support 1 pix mode => CAM_QBIN1_MODE.QBN_ACC can't be 0. If it is, disable QBIN1 */
    scaling_ratio = (MUINT32)MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.SEPO,CAM_FUNC_INF) >> (MUINT32)Bin.H;

    /* for dynamically changing QBIN1 ACC, must disable DCM*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_RAW_DCM_DIS, CAMCTL_QBN_R4_DCM_DIS,1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R4_QBIN_CTL,QBIN_ACC_MODE,3); // QBIN_ACC_MODE must be 3 , if QBIN if before ltms

    /*update ACC for twin case*/
    UPDATE_QBIN_ACC(EN, R4, scaling_ratio);

    if(scaling_ratio) {
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_QBN_R4_EN, MTRUE);
        //W phy is for query , vir is for twin, NOTE: ACC dynamic change, can't W phy
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R4_QBIN_CTL,QBIN_ACC,scaling_ratio);
    }
    else{
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN,CAMCTL_QBN_R4_EN, MFALSE);
    }

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_MRG_R4_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CTL,MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r4);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CTL,MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CTL,MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CTL,MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CTL,MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CTL,MRG_SIG_MODE2, 0);

    // MRG R4 cropping
    _tmp = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
            CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> (MUINT32)Bin.H;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_CROP, MRG_END_X, _tmp - 1);
    _tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> (MUINT32)Bin.V;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R4_MRG_VSIZE, MRG_HT, _tmp);

    return ret;
}

MINT32 CAM_MRG_R5_CTRL::_config(void)
{
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;
    MINT32 ret = 0;
    MUINT32 scaling_ratio = 0, _tmp = 0;
    RectData<MUINT32> Bin;

    GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);

    CamInfo.GetCapibility(0 ,ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(), rst, E_CAM_SRAM_MRG);
    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN,
        NormalPipe_InputInfo((NSCam::EImageFormat)0, 0, this->m_pRaw_p->m_dlp.SEPO), rst, E_CAM_pipeline_min_size);

    /* QBIN1 ACC, 0 -> no scaling, 1->2x scaling down, 2->4x scaling down
       QBIN1 doesn't support 1 pix mode => CAM_QBIN1_MODE.QBN_ACC can't be 0. If it is, disable QBIN1 */
    scaling_ratio = (MUINT32)MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.SEPO,CAM_FUNC_INF) >> (MUINT32)Bin.H;

    /*update ACC for twin case*/
    UPDATE_QBIN_ACC(EN2, R5, scaling_ratio);

    /* for dynamically changing QBIN1 ACC, must disable DCM*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_RAW2_DCM_DIS, CAMCTL_QBN_R5_DCM_DIS,1);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R5_QBIN_CTL,QBIN_ACC_MODE,1);//always use average

    if(scaling_ratio) {
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R5_EN, MTRUE);
        //W phy is for query , vir is for twin, NOTE: ACC dynamic change, can't W phy
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R5_QBIN_CTL,QBIN_ACC,scaling_ratio);
    }
    else{
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_QBN_R5_EN, MFALSE);
    }

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_MRG_R5_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CTL,MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r5);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CTL,MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CTL,MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CTL,MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CTL,MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CTL,MRG_SIG_MODE2, 0);

    // MRG R5 cropping
    _tmp = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
            CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> (MUINT32)Bin.H;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_CROP, MRG_END_X, _tmp - 1);
    _tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> (MUINT32)Bin.V;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R5_MRG_VSIZE, MRG_HT, _tmp);

    return ret;
}

MINT32 CAM_MRG_R6_CTRL::_config(void)
{
    MINT32 ret=0;
    //ADL path
    CAM_FUNC_DBG("###########################");
    CAM_FUNC_DBG("ADL path not yet support !!");
    CAM_FUNC_DBG("###########################");
    return ret;
}

MINT32 CAM_MRG_R7_CTRL::_config(void)
{
    MINT32 ret=0;
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;

    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(), rst, E_CAM_SRAM_MRG);

    /*R7*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_MRG_R7_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R7_MRG_CTL, MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r7);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R7_MRG_CTL, MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R7_MRG_CTL, MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R7_MRG_CTL, MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R7_MRG_CTL, MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R7_MRG_CTL, MRG_SIG_MODE2, 0);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R7O, MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.MRG_R7,CAM_FUNC_INF));

    /*R11*/
    //MRG_R11 enable at CAM_TWIN_PIPE::_config()
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R11_MRG_CTL, MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r11);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R11_MRG_CTL, MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R11_MRG_CTL, MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R11_MRG_CTL, MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R11_MRG_CTL, MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R11_MRG_CTL, MRG_SIG_MODE2, 0);

    return ret;
}

MINT32 CAM_MRG_R8_CTRL::_config(void)
{
    MINT32 ret=0;
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;

    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(), rst, E_CAM_SRAM_MRG);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_MRG_R8_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R8_MRG_CTL, MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r8);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R8_MRG_CTL, MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R8_MRG_CTL, MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R8_MRG_CTL, MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R8_MRG_CTL, MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R8_MRG_CTL, MRG_SIG_MODE2, 0);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R8O, MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.MRG_R8,CAM_FUNC_INF));
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, HDS_R1_HDS_HDS_MODE, HDS_DS_MODE, MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.MRG_R8,CAM_FUNC_INF));

    // MRG R8 cropping in rrz_ctrl

    return ret;
}

MINT32 CAM_MRG_R10_CTRL::_config(void)
{
    capibility CamInfo(this->m_hwModule);
    tCAM_rst rst;
    MINT32 ret = 0;
    MUINT32 scaling_ratio = 0, _tmp = 0;
    RectData<MUINT32> Bin;

    GET_SEP_BIN(this->m_pRaw_p->m_pDrv,Bin);

    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN, NormalPipe_InputInfo(), rst, E_CAM_SRAM_MRG);
    CamInfo.GetCapibility(0, ENPipeQueryCmd_UNKNOWN,
        NormalPipe_InputInfo((NSCam::EImageFormat)0, 0, this->m_pRaw_p->m_dlp.SEPO), rst, E_CAM_pipeline_min_size);

    /* QBIN1 ACC, 0 -> no scaling, 1->2x scaling down, 2->4x scaling down
       QBIN1 doesn't support 1 pix mode => CAM_QBIN1_MODE.QBN_ACC can't be 0. If it is, disable QBIN1 */
    scaling_ratio = (MUINT32)MAP_IFUNC_PIXMODE(this->m_pRaw_p->m_dlp.SEPO,CAM_FUNC_INF) >> (MUINT32)Bin.H;

    /*update ACC for twin case*/
    UPDATE_QBIN_ACC(EN3, R3, scaling_ratio);

    /* for dynamically changing QBIN1 ACC, must disable DCM*/
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_RAW3_DCM_DIS, CAMCTL_QBN_R3_DCM_DIS,1);

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R3_QBIN_CTL,QBIN_ACC_MODE,1);//always use average

    if(scaling_ratio) {
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN3,CAMCTL_QBN_R3_EN, MTRUE);
        //W phy is for query , vir is for twin, NOTE: ACC dynamic change, can't W phy
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,QBIN_R3_QBIN_CTL,QBIN_ACC,scaling_ratio);
    }
    else{
        CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv,CAMCTL_R1_CAMCTL_EN3,CAMCTL_QBN_R3_EN, MFALSE);
    }

    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_MRG_R10_EN, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CTL,MRG_SRAM_SIZE, rst.dlp.module.sram_mrg_r10);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CTL,MRG_EDGE, 0xf);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CTL,MRG_EDGE_SET, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CTL,MRG_EDGE, 0xf);

    //single_mode1 & 2 are using exclusive method.   signle_mode1 is main path, signle_mode2 is from other path
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CTL,MRG_SIG_MODE1, 1);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CTL,MRG_SIG_MODE2, 0);

    // MRG R10 cropping
    _tmp = ((CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
            CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> (MUINT32)Bin.H;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CROP, MRG_STR_X, 0);
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_CROP, MRG_END_X, _tmp - 1);
    _tmp = CAM_READ_BITS(this->m_pRaw_p->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> (MUINT32)Bin.V;
    CAM_WRITE_BITS(this->m_pRaw_p->m_pDrv, MRG_R10_MRG_VSIZE, MRG_HT, _tmp);

    return ret;
}

/*/////////////////////////////////////////////////////////////////////////////
  ISP_RAW_PIPE
/////////////////////////////////////////////////////////////////////////////*/
CAM_RAW_PIPE::CAM_RAW_PIPE()
{
    m_pP1Tuning = NULL;
    m_RawType = E_FromTG;
    m_Source = IF_CAM_MAX;
    m_pDrv = NULL;
    m_bBin = MFALSE;
    m_bRRz = MFALSE;
    m_DataPat = eCAM_NORMAL;
    m_bDYUV = MFALSE;
    m_SrcTG = TG_CAM_MAX;
    m_rrz_rlb_offset = 0;

    memset(&m_input, 0, sizeof(m_input));

    //for cb use
    pIspReg= NULL;
    m_CurBin = MFALSE;
    m_CurQbn1Acc = _1_pix_;

    pIspReg = (MUINT32*)malloc(CAM_BASE_RANGE);
    if(pIspReg == NULL)
        CAM_FUNC_ERR("allocate mem for callback fail\n");

    memset((void *)&m_dumpInput, 0, sizeof(m_dumpInput));
    memset((void *)&m_SrcSize, 0, sizeof(m_SrcSize));
    m_dumpInput.size = CAM_BASE_RANGE;
    m_Dyuv.m_pMainPipe = this;
    m_NotifyFrame = 0;
}

CAM_RAW_PIPE::~CAM_RAW_PIPE()
{
    if(pIspReg){
        free(pIspReg);
        pIspReg = NULL;
    }
    if(m_dumpInput.pReg){
        free(m_dumpInput.pReg);
        m_dumpInput.pReg = NULL;
    }
}

inline void CAM_RAW_PIPE::CBTimeChk(char *str, MBOOL bStartT)
{
    struct timespec ts;
    div_t divResult;
    static MUINT32 TimehourS, TimeMinS, TimeSecS, TimeUsS;
    MUINT32 Timehour = 0, TimeMin = 0, TimeSec = 0, TimeUs = 0;

    clock_gettime(CLOCK_REALTIME, &ts);
    TimeUs = ts.tv_nsec/1000;//ms
    TimeSec  = ts.tv_sec%86400;//day=60*60*24=86400 sec

    divResult = div(TimeSec, 3600);
    Timehour = divResult.quot;
    TimeSec = divResult.rem;

    divResult = div(TimeSec, 60);
    TimeMin = divResult.quot;
    TimeSec = divResult.rem;

    if (bStartT) {
        TimehourS = Timehour;
        TimeMinS = TimeMin;
        TimeSecS = TimeSec;
        TimeUsS = TimeUs;
        CAM_FUNC_DBG("%s(m:%d)+[%02d:%02d:%02d.%06d]",
                    str, this->m_dumpInput.magic, Timehour, TimeMin, TimeSec, TimeUs);
    } else {
        CAM_FUNC_DBG("%s(m:%d)-[%02d:%02d:%02d.%06d], diff[%02d:%02d:%02d.%06d]",
                     str, this->m_dumpInput.magic, Timehour, TimeMin, TimeSec, TimeUs,
                    (Timehour-TimehourS), (TimeMin-TimeMinS),
                    (TimeSec-TimeSecS), (TimeUs-TimeUsS));

        Timehour -= TimehourS;
        TimeMin  -= TimeMinS;
        TimeSec  -= TimeSecS;
        TimeUs   -= TimeUsS;
        if((Timehour+TimeMin+TimeSec) || (TimeUs > CAM_CB_DURATION_US)){
            CAM_FUNC_WRN("%s(m:%d) takes too many time: [%02d:%02d:%02d.%06d]",
                    str, this->m_dumpInput.magic, Timehour, TimeMin, TimeSec, TimeUs);
        }
    }
}

MINT32 CAM_RAW_PIPE::configNotify(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    this->m_CurBin = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
    this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC);
    this->m_NotifyFrame = 0;//reset to 0. first enque will update this

    this->m_TG_W.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_PXL);
    this->m_TG_H.Raw = CAM_READ_REG(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_GRAB_LIN);

    CAM_FUNC_DBG("m_CurBin=%d, m_bBin=%d,"
                "m_CurQbn1Acc=%d"
                "m_NotifyFrame=%d",
                this->m_CurBin, this->m_bBin,
                this->m_CurQbn1Acc,
                this->m_NotifyFrame);

    return 0;
}

MINT32 CAM_RAW_PIPE::configCB(MBOOL bForce)
{
    MINT32 ret = 0;
    MUINT32 Qbn1Acc = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(bForce){
        this->m_NotifyFrame = Dynamic_bin_Notify_Frame;
    }

    if(this->m_CurBin != this->m_bBin){
        this->m_NotifyFrame = Dynamic_bin_Notify_Frame;
    }

    Qbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC);

    if(this->m_CurQbn1Acc != Qbn1Acc){
        this->m_NotifyFrame = Dynamic_bin_Notify_Frame;
    }

    CAM_FUNC_DBG("m_CurBin=%d, m_bBin=%d,"
                "m_CurQbn1Acc=%d, Qbn1Acc=%d,"
                "m_NotifyFrame=%d",
                this->m_CurBin, this->m_bBin,
                this->m_CurQbn1Acc, Qbn1Acc,
                this->m_NotifyFrame);
    return ret;
}

MBOOL CAM_RAW_PIPE::setNotifyDone(void)
{
    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if (this->m_NotifyFrame) {
        this->m_NotifyFrame--;
    }
    this->m_CurBin = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
    this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC);

    return ret;
}

MBOOL CAM_RAW_PIPE::dynamicPak(MUINT32 hwdmaoFmt, Pak_Func Func)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0, Obit = 0, img_sel = 0, lbit_g2 = 0, crp_r3_sel = 0;
    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    if (Func.Bits.PAK_DMAO == 0) {
        CAM_FUNC_DBG("dynamicPak+ cq:0x%x,page:0x%x,IMGO_FMT:0x%x,IMGO_STRIDE:0x%x,Physical_IMGO_FMT:0x%x,Physical_IMGO_STRIDE:0x%x\n",\
            cq,\
            page,\
            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_IMGO_SEL),\
            CAM_READ_BITS(this->m_pDrv,IMGO_R1_IMGO_STRIDE,IMGO_STRIDE),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_IMGO_R1_FMT),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),IMGO_R1_IMGO_STRIDE,IMGO_STRIDE));

        img_sel = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_IMG_SEL);
        lbit_g2 = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_LBIT_MODE_G2);
        crp_r3_sel = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_CRP_R3_SEL);

        if (img_sel == 1) {
            Obit = 14;
        }
        else{
            switch(crp_r3_sel) {
            case 1:
                Obit = 12;
                break;
            case 2:
            case 3:
            case 4:
                Obit = (lbit_g2) ? 16 : 19;
                break;
            case 5:
                Obit = (lbit_g2) ? 12 : 15;
                break;
            case 6:
                Obit = (CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_LBIT_MODE_G1)) ? 12 : 14;
                break;
            case 0:
                if (img_sel == 0) {
                    Obit = 14;
                    break;
                }
                // else follow default error case
            case 7:
            default:
                CAM_FUNC_ERR("undefine case(rcp_r3_sel:%d,img_sel:%d)",crp_r3_sel,img_sel);
                return MFALSE;
            }
        }
        CAM_WRITE_BITS(this->m_pDrv,PAK_R1_PAK_CONT, PAK_OBIT, Obit);

        switch(hwdmaoFmt)
        {
            case IMGO_FMT_RAW8:
            case IMGO_FMT_RAW10:
            case IMGO_FMT_RAW12:
            case IMGO_FMT_RAW14:
            case IMGO_FMT_RAW16:
            case IMGO_FMT_RAW19:
            case IMGO_FMT_RAW8_2B:
            case IMGO_FMT_RAW10_2B:
            case IMGO_FMT_RAW12_2B:
            case IMGO_FMT_RAW14_2B:
                CAM_FUNC_DBG("CAM_RAW_PIPE::dynamicPak hwImgoFmt:0x%x\n",hwdmaoFmt);
                break;
            default:
                CAM_FUNC_ERR("dynamicPak Imgo Fmt Error\n");
                return MFALSE;
        }

        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_IMGO_R1_FMT, hwdmaoFmt);

        CAM_FUNC_DBG("dynamicPak- cq:0x%x,page:0x%x,IMGO_FMT:0x%x,IMGO_STRIDE:0x%x,Physical_IMGO_FMT:0x%x,Physical_IMGO_STRIDE:0x%x Obit:%d\n",\
            cq,\
            page,\
            CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_IMGO_R1_FMT),\
            CAM_READ_BITS(this->m_pDrv,IMGO_R1_IMGO_STRIDE,IMGO_STRIDE),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_IMGO_R1_FMT),\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),IMGO_R1_IMGO_STRIDE,IMGO_STRIDE),Obit);
    }
    else {
        if(!m_Dyuv.dynamicPak(hwdmaoFmt, Func))
            CAM_FUNC_ERR("DYUV_PIPE dynamicPak fail");
    }

    return ret;
}

MINT32 CAM_RAW_PIPE::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;
    Header_IMGO  fh_imgo;
    Header_RRZO  fh_rrzo;
    Header_UFEO  fh_ufeo;
    Header_UFGO  fh_ufgo;
    Header_LCSO  fh_lcso;
    Header_LMVO  fh_lmvo;
    Header_RSSO  fh_rsso;
    Header_AAO   fh_aao;
    Header_TSFSO fh_tsfso;
    Header_PDO   fh_pdo;
    Header_AFO   fh_afo;
    Header_FLKO  fh_flko;
    MUINT32 dbn_en = 0, Qbn1Acc = 0;
    char pcm_ufe[PROPERTY_VALUE_MAX] = {'\0'};
    char pcm_ufeg[PROPERTY_VALUE_MAX] = {'\0'};
    MBOOL UFE_PCM_EN = MFALSE, UFG_PCM_EN = MFALSE;
    RectData<MUINT32> Bin;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    property_get("vendor.isp.forcepcm.ufe", pcm_ufe, "0");
    property_get("vendor.isp.forcepcm.ufeg", pcm_ufeg, "0");
    UFE_PCM_EN = atoi(pcm_ufe);
    UFG_PCM_EN = atoi(pcm_ufeg);

    CAM_FUNC_DBG("CAM_RAW_PIPE::_config+ cq:0x%x,page:0x%x,rrz_en:%d,tg_pix_mode:0x%x,cam source:0x%x,bin:0x%x_0x%x,pcm(%d/%d),src_size_w/h(%d/%d),pattern(%d)\n",cq,page,\
        CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_RRZ_R1_EN),\
        this->m_dlp.SEPI,\
        this->m_Source,\
        this->m_bBin,this->m_CurBin,
        UFE_PCM_EN,
        UFG_PCM_EN,
        this->m_SrcSize.w,
        this->m_SrcSize.h,
        this->m_DataPat);

    //dlp cropping and single_mode under tiwn mode is cal. by twin_Drv
    //here is non-twin setting

    CAM_WRITE_REG(this->m_pDrv, CAMDMATOP1_R1_CAMDMATOP1_DMA_DATA_PATTERN, this->m_DataPat);

    //bininig ctrl
    if(this->m_bBin){
        switch(this->m_DataPat){
            case eCAM_4CELL:
            case eCAM_4CELL_IVHDR:
            case eCAM_4CELL_ZVHDR:
                CAM_FUNC_ERR("4cell(%d) pattern can't enable frontal bin, force disable it\n", \
                    this->m_DataPat);
                this->m_bBin = MFALSE;
                break;

            case eCAM_DUAL_PIX:
                CAM_FUNC_ERR("Dual pix(%d) pattern can't enable frontal bin, force disable it\n", \
                    this->m_DataPat);
                this->m_bBin = MFALSE;
                break;
            default:
                //do nothing
                break;
        }
    }

    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN, this->m_bBin);
    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_RRZ_R1_EN, this->m_bRRz);  //this is for 3-raw + h-ratio zoom in, rrz will be disabled by twin drv

    //DatPat will decide scm/dbn/vbn settings
    switch(this->m_DataPat){
        case eCAM_DUAL_PIX:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2, CAMCTL_DBN_R1_EN, 1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_DBN_R1_EN, 1);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),DBN_R1_DBN_GAIN, 0x200);
            CAM_WRITE_REG(this->m_pDrv->getPhyObj(),DBN_R1_DBN_OFST, 0);
            break;
        case eCAM_MONO:
        case eCAM_4CELL:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2, CAMCTL_FBND_R1_EN, 1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_FBND_R1_EN, 1);
            //fbnd
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), FBND_R1_FBND_CFG, FBND_MASK0, 0xF);
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(), FBND_R1_FBND_CFG, FBND_DIV0, 3);
            break;
        case eCAM_4CELL_IVHDR:
        case eCAM_4CELL_ZVHDR:
            CAM_WRITE_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_EN2, CAMCTL_FBND_R1_EN, 1);
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_FBND_R1_EN, 1);
            //LafiteEP_TODO: fbnd cfg
            break;
        case eCAM_NORMAL:
        case eCAM_NORMAL_PD:
        default:
            //bypass
            break;
    }

    //input for CB, set "before" BIN_EN/QBN1_ACC/RMB_ACC changing
    this->m_input.TgOut_W = this->m_SrcSize.w;
    this->m_input.TgOut_H = this->m_SrcSize.h;
    this->m_input.CurBinOut_W = this->m_input.TgOut_W >> (this->m_CurBin + CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN) \
                                                                         + CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));
    this->m_input.CurBinOut_H = this->m_input.TgOut_H >> (this->m_CurBin + CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN));
    this->m_input.CurQBNOut_W = this->m_input.CurBinOut_W >> ((CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC));

    //amx in single mode {MODE_1,MODE_2} = 2b10 same as bmx

    this->m_Sep.config(this);

    if (CAM_C != this->m_hwModule) {
        this->m_MrgR1.config(this);
        this->m_MrgR2.config(this);
        this->m_MrgR3.config(this);
        this->m_MrgR4.config(this);
        this->m_MrgR5.config(this);
        this->m_MrgR6.config(this);
        this->m_MrgR7.config(this);
        this->m_MrgR8.config(this);
        this->m_MrgR10.config(this);
    }

    //rcrop
    this->m_RcpR1.config(this);

    //rcrop3
    this->m_RcpR3.config(this);

    //Set RAW_TG_SEL
    this->setRAW_SEL();

    //bin
    CAM_WRITE_BITS(this->m_pDrv,BIN_R1_BIN_CTL, BIN_OV_TH,0xC);//from CY
    CAM_WRITE_BITS(this->m_pDrv,BIN_R1_BIN_CTL, BIN_TYPE, 0x2);//from Yu-Hsiang
    CAM_WRITE_REG(this->m_pDrv,BIN_R1_BIN_FTH, 0x0C090603);
    CAM_WRITE_REG(this->m_pDrv,BIN_R1_BIN_SPARE, 0x00000000);
    CAM_WRITE_BITS(this->m_pDrv,BIN_R1_BIN_CTL, BIN_OV_TH,0xC);//from CY
    CAM_WRITE_BITS(this->m_pDrv,BIN_R1_BIN_CTL, BIN_TYPE, 0x2);
    CAM_WRITE_REG(this->m_pDrv,BIN_R1_BIN_FTH, 0x0C090603);
    CAM_WRITE_REG(this->m_pDrv,BIN_R1_BIN_SPARE, 0x00000000);

    //image header
    fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL);
    fh_imgo.Header_Enque(Header_IMGO::E_PIX_ID,this->m_pDrv,fmt_sel.Bits.CAMCTL_PIX_ID);
    fh_imgo.Header_Enque(Header_IMGO::E_FMT,this->m_pDrv,fmt_sel.Bits.CAMCTL_IMGO_R1_FMT);
    fh_ufeo.Header_Enque(Header_UFEO::E_FMT,this->m_pDrv,fmt_sel.Bits.CAMCTL_IMGO_R1_FMT);
    dbn_en = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN);
    fh_imgo.Header_Enque(Header_IMGO::E_SRC_SIZE,this->m_pDrv,
                         ((dbn_en<<31) || (this->m_SrcSize.h << 16) || this->m_SrcSize.w));

    fh_rrzo.Header_Enque(Header_RRZO::E_PIX_ID,this->m_pDrv,fmt_sel.Bits.CAMCTL_PIX_ID);
    fh_rrzo.Header_Enque(Header_RRZO::E_FMT,this->m_pDrv,fmt_sel.Bits.CAMCTL_RRZO_R1_FMT);
    fh_ufgo.Header_Enque(Header_UFGO::E_FMT,this->m_pDrv,fmt_sel.Bits.CAMCTL_RRZO_R1_FMT);

    //IQ
    E_CamIQLevel CamIqL = this->m_bBin ? eCamIQ_L : eCamIQ_H;
    fh_imgo.Header_Enque(Header_IMGO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_rrzo.Header_Enque(Header_RRZO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_ufeo.Header_Enque(Header_UFEO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_ufgo.Header_Enque(Header_UFGO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_lcso.Header_Enque(Header_LCSO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_lmvo.Header_Enque(Header_LMVO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_rsso.Header_Enque(Header_RSSO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_aao.Header_Enque(Header_AAO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_tsfso.Header_Enque(Header_TSFSO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_pdo.Header_Enque(Header_PDO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_afo.Header_Enque(Header_AFO::E_IQ_LEVEL, this->m_pDrv, CamIqL);
    fh_flko.Header_Enque(Header_FLKO::E_IQ_LEVEL, this->m_pDrv, CamIqL);

    /*check if qbin1/rmb pixel mode are changed.
      ACC of qbin1 & rmb both are refer to pix_mode_amxo, so we can just choose one of them to check acc.*/
    Qbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC);

    //set input of CB fun, set "after" BIN_EN/QBN1_ACC/RMB_ACC changing
    this->m_input.Bin_MD = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN);//0: average, 1: sum
    this->m_input.TarBin_EN = this->m_bBin;
    GET_FULL_BIN(this->m_pDrv,Bin,2);
    this->m_input.TarBinOut_W = this->m_input.TgOut_W >> Bin.H;
    this->m_input.TarBinOut_H = this->m_input.TgOut_H >> Bin.V;
    this->m_input.TarQBNOut_W = this->m_input.TarBinOut_W >> Qbn1Acc;

#if UFE_HW_DEFAULT_WORK_AROUND
    CAM_WRITE_BITS(this->m_pDrv, UFG_R1_UFG_CON, UFG_TCCT_BYP, 0);
    CAM_WRITE_BITS(this->m_pDrv, UFE_R1_UFE_CON, UFE_TCCT_BYP, 1);
#endif
    /*for debug*/
    CAM_WRITE_BITS(this->m_pDrv, UFG_R1_UFG_CON, UFG_FORCE_PCM, UFG_PCM_EN);
    CAM_WRITE_BITS(this->m_pDrv, UFE_R1_UFE_CON, UFE_FORCE_PCM, UFE_PCM_EN);

    if (m_bDYUV) {
        if(!this->m_Dyuv.config())
            CAM_FUNC_ERR("DYUV_PIPE config fail");
    }

    return 0;
}

MINT32 CAM_RAW_PIPE::_write2CQ(MUINT32 BurstIndex)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("CAM_RAW_PIPE::_write2CQ+ cq:0x%x,page:0x%x,burst:0x%x\n",
                 cq, page, BurstIndex);

    if (BurstIndex > 0) {
        /*For SMVR, only add module at normal page*/
        goto EXIT;
    }

    this->m_pDrv->cqAddModule(CAM_DATA_PAT_);
    this->m_pDrv->cqAddModule(CAM_CTL_SPECIAL_FUN_);

    this->m_pDrv->cqAddModule(CAM_ISP_SEP_);
    this->m_pDrv->cqAddModule(CAM_ISP_CRP_R1_);
    this->m_pDrv->cqAddModule(CAM_ISP_CRP_R3_);

    this->m_pDrv->cqAddModule(CAM_ISP_RRZ_);
    this->m_pDrv->cqAddModule(CAM_ISP_PDE_);
    this->m_pDrv->cqAddModule(CAM_ISP_DBN_);
    this->m_pDrv->cqAddModule(CAM_ISP_BIN_);

    //
    this->m_pDrv->cqAddModule(CAM_ISP_BPC_);
    this->m_pDrv->cqAddModule(CAM_ISP_LSC_);
    this->m_pDrv->cqAddModule(CAM_ISP_UFE_);
    this->m_pDrv->cqAddModule(CAM_ISP_UFG_);
    this->m_pDrv->cqAddModule(CAM_ISP_UFD_R2_);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_SIZE);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_LRZR);
    this->m_pDrv->cqAddModule(CAM_ISP_AE_);
    this->m_pDrv->cqAddModule(CAM_ISP_AE2_);
    this->m_pDrv->cqAddModule(CAM_ISP_AWB_);
    this->m_pDrv->cqAddModule(CAM_ISP_AF_);
    this->m_pDrv->cqAddModule(CAM_ISP_FBND_);
    this->m_pDrv->cqAddModule(CAM_ISP_WB_);
    this->m_pDrv->cqAddModule(CAM_ISP_DGN_);
    this->m_pDrv->cqAddModule(CAM_ISP_TSFS_);
    this->m_pDrv->cqAddModule(CAM_DMA_BPCI_);
    this->m_pDrv->cqAddModule(CAM_DMA_BPCI_R2_);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_LOG_);
    this->m_pDrv->cqAddModule(CAM_ISP_LCS_FLC0_);
    this->m_pDrv->cqAddModule(CAM_ISP_LTM_SECTION1);
    this->m_pDrv->cqAddModule(CAM_ISP_LTM_MAX_DIV);
    this->m_pDrv->cqAddModule(CAM_ISP_LTM_SECTION2);
    this->m_pDrv->cqAddModule(CAM_ISP_LTMTC_CURVE_1);
    this->m_pDrv->cqAddModule(CAM_ISP_LTMTC_CURVE_2);
    this->m_pDrv->cqAddModule(CAM_ISP_LTM_PINGPONG_);
    this->m_pDrv->cqAddModule(CAM_ISP_HLR_);
    this->m_pDrv->cqAddModule(CAM_CTL_HLR_LKMSB_);


    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R3_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R4_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R5_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R6_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R7_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R8_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R10_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R11_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MRG_R12_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_PBN_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_QBIN_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_QBIN_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_QBIN_R3_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_QBIN_R4_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_QBIN_R5_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_OBC_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_RSS_A_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_LMV_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_FLK_A_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_HDS_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MOBC_R2_);
    XOR_ADD_MODULE(this->m_hwModule,this->m_pDrv,CAM_ISP_MOBC_R3_);

    //NOP function , these func should apb when being used at tuning
    this->m_pDrv->cqNopModule(CAM_ISP_RRZ_);
    this->m_pDrv->cqNopModule(CAM_ISP_PDE_);
    this->m_pDrv->cqNopModule(CAM_ISP_PBN_);
    this->m_pDrv->cqNopModule(CAM_ISP_DBN_);
    this->m_pDrv->cqNopModule(CAM_ISP_BIN_);
    this->m_pDrv->cqNopModule(CAM_ISP_OBC_);
    this->m_pDrv->cqNopModule(CAM_ISP_BPC_);
    this->m_pDrv->cqNopModule(CAM_ISP_LSC_);
#if UFE_HW_DEFAULT_WORK_AROUND
#else
    this->m_pDrv->cqNopModule(CAM_ISP_UFE_);
    this->m_pDrv->cqNopModule(CAM_ISP_UFG_);
#endif
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_SIZE);
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_LRZR);
    this->m_pDrv->cqNopModule(CAM_ISP_AE_);
    this->m_pDrv->cqNopModule(CAM_ISP_AE2_);
    this->m_pDrv->cqNopModule(CAM_ISP_AWB_);
    this->m_pDrv->cqNopModule(CAM_ISP_AF_);
    this->m_pDrv->cqNopModule(CAM_ISP_FBND_);


    this->m_pDrv->cqNopModule(CAM_ISP_RSS_A_);
    this->m_pDrv->cqNopModule(CAM_ISP_LMV_);
    this->m_pDrv->cqNopModule(CAM_ISP_FLK_A_);

    this->m_pDrv->cqNopModule(CAM_ISP_MOBC_R2_);
    this->m_pDrv->cqNopModule(CAM_ISP_MOBC_R3_ );
    this->m_pDrv->cqNopModule(CAM_ISP_WB_ );
    this->m_pDrv->cqNopModule(CAM_ISP_DGN_);
    this->m_pDrv->cqNopModule(CAM_ISP_TSFS_);
    this->m_pDrv->cqNopModule(CAM_DMA_BPCI_);
    this->m_pDrv->cqNopModule(CAM_DMA_BPCI_R2_);
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_LOG_);
    this->m_pDrv->cqNopModule(CAM_ISP_LCS_FLC0_);
    this->m_pDrv->cqNopModule(CAM_ISP_LTM_SECTION1);
    this->m_pDrv->cqNopModule(CAM_ISP_LTM_MAX_DIV);
    this->m_pDrv->cqNopModule(CAM_ISP_LTM_SECTION2);
    this->m_pDrv->cqNopModule(CAM_ISP_LTMTC_CURVE_1);
    this->m_pDrv->cqNopModule(CAM_ISP_LTMTC_CURVE_2);
    this->m_pDrv->cqNopModule(CAM_ISP_LTM_PINGPONG_);
    this->m_pDrv->cqNopModule(CAM_ISP_HLR_);
    this->m_pDrv->cqNopModule(CAM_CTL_HLR_LKMSB_);


    if (m_bDYUV) {
        this->m_Dyuv.write2CQ();
    }

EXIT:

    return 0;
}

MINT32 CAM_RAW_PIPE::DoneCollector(MBOOL En, E_INPUT InPut)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0, P1DoneRef = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    P1DoneRef = GetP1DoneRef(this->m_hwModule);

    if (En == MTRUE) {
        P1DoneRef |= GetP1DoneRef(MAP_MODULE_TG(InPut, MFALSE, CAM_FUNC_ERR));
    }
    CAM_FUNC_DBG("P1DoneRef:0x%x", P1DoneRef);

    CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_MISC, CAMCTL_LAST_PASS1_DONE_CTL, P1DoneRef);

    return 0;
}

MINT32 CAM_RAW_PIPE::setNotify(E_SET_NOTIFY_OP op)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page = 0;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    switch(op){
        case E_Notify_Config:
            this->m_CurBin = CAM_READ_BITS(this->m_pDrv->getPhyObj(), CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
            this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC);
            XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC, this->m_CurQbn1Acc);
            break;
        case E_Notify_Update:
            this->m_CurBin = CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_BIN_R1_EN);
            this->m_CurQbn1Acc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv, QBIN_R1_QBIN_CTL, QBIN_ACC);
            break;
        default:
            CAM_FUNC_ERR("unsupport cmd(%d)", op);
            break;
    }

    return 0;
}

MBOOL CAM_RAW_PIPE::setIMG_SEL(void)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    Header_IMGO fh_imgo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //dBin check, if bin on, force IMGO to pure raw
    if(this->m_bBin && (this->m_RawType != E_FromTG)){
        CAM_FUNC_DBG("bin on, force pure raw\n");
        this->m_RawType = E_FromTG;
    }

    switch(this->m_RawType){
    case E_FromTG:
        CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData from TG(%d)\n",cq,page,this->m_DataPat);
        switch(this->m_DataPat){
        case eCAM_DUAL_PIX:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, 6);
            break;
        default:
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, 0);
            break;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_IMG_SEL, 0);

        this->m_RcpR3.config(this);

        fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_FromTG);
        break;
    case E_BeforLSC:
        CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData before LSC\n",cq,page);
        switch(this->m_DataPat){
        case eCAM_4CELL:
        case eCAM_4CELL_IVHDR:
        case eCAM_4CELL_ZVHDR:
            CAM_FUNC_ERR("CAM_RAW_PIPE::setIMG_SEL do quad_code es not support this path: %d\n",
                (int)this->m_RawType);
            return MFALSE;
            break;
        default:
            //do nothing
            break;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_IMG_SEL, 2);
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, 4);
        fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_BeforLSC);
        break;
    case E_AFTERLSC:
        CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData after LSC\n",cq,page);
        switch(this->m_DataPat){
        case eCAM_4CELL:
        case eCAM_4CELL_IVHDR:
        case eCAM_4CELL_ZVHDR:
            CAM_FUNC_ERR("CAM_RAW_PIPE::setIMG_SEL do quad_code es not support this path: %d\n",
                (int)this->m_RawType);
            return MFALSE;
            break;
        default:
            //do nothing
            break;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_IMG_SEL, 2);
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, 1);
        fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_AFTERLSC);
        break;
    case E_BetweenLSCWB:
        CAM_FUNC_DBG("CAM_RAW_PIPE::setIMG_SEL+ cq:0x%x,page:0x%x,RawData between LSC and WB\n",cq,page);
        switch(this->m_DataPat){
        case eCAM_4CELL:
        case eCAM_4CELL_IVHDR:
        case eCAM_4CELL_ZVHDR:
            CAM_FUNC_ERR("CAM_RAW_PIPE::setIMG_SEL do quad_code es not support this path: %d\n",
                (int)this->m_RawType);
            return MFALSE;
            break;
        default:
            //do nothing
            break;
        }
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_IMG_SEL, 2);
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL, CAMCTL_CRP_R3_SEL, 3);
        fh_imgo.Header_Enque(Header_IMGO::E_RAW_TYPE,this->m_pDrv,E_BetweenLSCWB);
        break;
    default:
        CAM_FUNC_ERR("Raw Data support only 0:from TG,1:before LSC,2:after LSC,3:between LSC and WB\n");
        return MFALSE;
    }

    return MTRUE;
}

MBOOL CAM_RAW_PIPE::setRAW_SEL(void)
{
    CAM_FUNC_DBG("path:%d,sourcetg:%d\n",this->m_Source,this->m_SrcTG);

    switch(this->m_Source){
        case IF_CAM_A:
        case IF_CAM_B:
        case IF_CAM_C:
            if ((MUINT32)this->m_SrcTG == MAP_MODULE_TG(this->m_hwModule, MTRUE, CAM_FUNC_ERR)) {
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_RAW_SEL, 0); //Set RAW_SEL from self TG
            }
            else {
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_RAW_SEL, 3); //Set RAW_SEL from other TG
            }

            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL2, CAMCTL_RAW_TG_SEL, MAP_RAW_TG_SEL(this->m_SrcTG));
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_TWIN_STATUS, SRC_TG, this->m_SrcTG);
            break;
        case IF_RAWI_CAM_A:
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_SEL, CAMCTL_RAW_SEL, 2);
            break;
        default:
            CAM_FUNC_ERR("unsupported path:%d\n",this->m_Source);
            return MFALSE;
            break;
    }

    return MTRUE;
}

MBOOL CAM_RAW_PIPE::setLMV(MBOOL &bWdmaEn)
{
    MBOOL ret = MTRUE;
    DMA_EISO lmvo;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if (CAM_C == this->m_hwModule) {
        CAM_FUNC_DBG("Simple raw_c is not support LMV");
        return MTRUE;
    }

    if (this->m_pP1Tuning){
        LMV_CFG  lmv_cfg;
        LMV_INPUT_INFO input;
        Header_RRZO fh_rrzo;
        MUINT32 tmp = 0;
        RectData<MUINT32> Bin;

        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_RRZ_R1_EN) != 1){
            CAM_FUNC_ERR("RRZ must be opened in Bayer domain when EIS is needed\n");
            return MFALSE;
        }

        GET_FULL_BIN(this->m_pDrv,Bin, 2);

        //only hds , bin information is not needed
        input.pixMode = CAM_READ_BITS(this->m_pDrv,HDS_R1_HDS_HDS_MODE,HDS_DS_MODE);

        input.sTGOut.w = this->m_SrcSize.w;
        input.sTGOut.h = this->m_SrcSize.h;

        if(CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_HDS_SEL) == HDS_SEL_0){    //rrz have no enabled
            input.sHBINOut.w = input.sTGOut.w >> input.pixMode;
            input.sHBINOut.h = input.sTGOut.h;
        }
        else{
            tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
            input.sHBINOut.w = (tmp & 0xffff) >> input.pixMode;
            input.sHBINOut.h = (tmp>>16);
        }

        input.RRZ_IN_CROP.in_size_w = ((CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_CROP,SEP_END_X) - \
                                         CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_CROP,SEP_STR_X)) + 1) >> Bin.H;
        input.RRZ_IN_CROP.in_size_h = CAM_READ_BITS(this->m_pDrv,SEP_R1_SEP_VSIZE,SEP_HT) >> Bin.V;

        input.RRZ_IN_CROP.start_x = CAM_READ_BITS(this->m_pDrv,RRZ_R1_RRZ_HORI_INT_OFST,RRZ_HORI_INT_OFST);
        input.RRZ_IN_CROP.start_y = CAM_READ_BITS(this->m_pDrv,RRZ_R1_RRZ_VERT_INT_OFST,RRZ_VERT_INT_OFST);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
        input.RRZ_IN_CROP.crop_size_w = tmp & 0xffff;
        input.RRZ_IN_CROP.crop_size_h = (tmp>>16);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
        input.sRMXOut.w = tmp & 0xffff;
        input.sRMXOut.h = (tmp>>16);
        input.bYUVFmt = MFALSE;

        CAM_FUNC_DBG("CAM_RAW_PIPE::setLMV:w/h(hbn:%d_%d,tg:%d_%d,rmx:%d_%d,rrz:%d_%d_%d_%d)\n",input.sHBINOut.w,\
                input.sHBINOut.h,input.sTGOut.w,input.sTGOut.h,\
                input.sRMXOut.w,input.sRMXOut.h,\
                input.RRZ_IN_CROP.start_x,input.RRZ_IN_CROP.start_y,input.RRZ_IN_CROP.crop_size_w,input.RRZ_IN_CROP.crop_size_h);

        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&lmv_cfg);

        CAM_FUNC_DBG("-\n");

        if(lmv_cfg.bypassLMV == 0){
            if(lmv_cfg.enLMV){
                MUINT32 addrost,modulesize;
                ISP_DRV_REG_IO_STRUCT* pReg;
                MUINT32* ptr = (MUINT32*)&lmv_cfg.cfg_lmv_prep_me_ctrl1;
                if(CAM_READ_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GSE_R1_EN) != 1){
                     CAM_FUNC_ERR("need to enable GSE before enable EIS\n");
                     return MFALSE;
                }
                this->m_pDrv->getCQModuleInfo(CAM_ISP_LMV_,addrost,modulesize);
                pReg = (ISP_DRV_REG_IO_STRUCT*)malloc(sizeof(ISP_DRV_REG_IO_STRUCT)*modulesize);

                for(MUINT32 i=0;i<modulesize;i++){
                    pReg[i].Data = *(ptr++);
                }
                CAM_BURST_WRITE_REGS(this->m_pDrv,LMV_R1_LMV_LMV_PREP_ME_CTRL1,pReg,modulesize);
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_LMV_R1_EN, 1);

                lmvo.m_pIspDrv = this->m_pIspDrv;
                lmvo.config();
                lmvo.write2CQ(0);
                lmvo.enable(NULL);
                bWdmaEn = MTRUE;

                this->m_pDrv->cqApbModule(CAM_ISP_LMV_);
                free(pReg);
            }
            else{
                CAM_FUNC_ERR("eis must be opened if notification is registered\n");
                ret = MFALSE;
            }
        }
        else{
            this->m_pDrv->cqNopModule(CAM_ISP_LMV_);
        }
    }
    return ret;
}

MBOOL CAM_RAW_PIPE::setLCS(MBOOL &bWdmaEn)
{
    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(this->m_pP1Tuning)
    {
        MUINT32 tmp = 0, qbinRatio = 0,qbinAcc = 0;
        LCES_REG_CFG     _lces_cfg;
        LCES_INPUT_INFO  input;
        Header_RRZO fh_rrzo;
        RectData<MUINT32> Bin;

        GET_FULL_BIN(this->m_pDrv,Bin, 2);

        input.sTGOut.w = this->m_SrcSize.w;
        input.sTGOut.h = this->m_SrcSize.h;
        qbinAcc = (CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R5_QBIN_CTL,QBIN_ACC);

        //
        input.sHBINOut.w = this->m_SrcSize.w >> qbinAcc;//acc is changed to represent scaling ratio
        input.sHBINOut.h = this->m_SrcSize.h;

        input.sHBINOut.w = input.sHBINOut.w >> Bin.H;
        input.sHBINOut.h = input.sHBINOut.h >> Bin.V;

        input.magic = fh_rrzo.GetRegInfo(Header_RRZO::E_Magic,this->m_pDrv);

        input.bIsDbin = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN);
        input.bIsbin = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN);
        input.bIsfbnd = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_FBND_R1_EN);
        input.bIsHbin = Bin.H;

        qbinRatio = CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC);
        input.bQbinRatio = (qbinRatio == 0) ? 1 : ((qbinRatio == 1) ? 2 : 4); //return qbin ratio 1x, 2x or 4x to 3A

        //this info is request from lsc
        input.RRZ_IN_CROP.in_size_w = this->m_SrcSize.w >> Bin.H;
        input.RRZ_IN_CROP.in_size_h = this->m_SrcSize.h >> Bin.V;
        input.RRZ_IN_CROP.start_x = CAM_READ_BITS(this->m_pDrv,RRZ_R1_RRZ_HORI_INT_OFST,RRZ_HORI_INT_OFST);
        input.RRZ_IN_CROP.start_y = CAM_READ_BITS(this->m_pDrv,RRZ_R1_RRZ_VERT_INT_OFST,RRZ_VERT_INT_OFST);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
        input.RRZ_IN_CROP.crop_size_w = tmp & 0xffff;
        input.RRZ_IN_CROP.crop_size_h = (tmp>>16);
        tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
        input.sRRZOut.w = tmp & 0xffff;
        input.sRRZOut.h = (tmp>>16);

        CAM_FUNC_DBG("+");
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_lces_cfg);
        CAM_FUNC_DBG("CAM_RAW_PIPE::setLCES:in:%d_%d, LCES_REG(%d_%d): m(%d)\n",input.sHBINOut.w,input.sHBINOut.h,\
                _lces_cfg.bLCES_Bypass,_lces_cfg.bLCES_EN,input.magic);

        if(_lces_cfg.bLCES_Bypass == 0)
        {
            if(_lces_cfg.bLCES_EN){
                DMA_LCSO lcso;

                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_IN_SIZE, _lces_cfg._LCES_REG.LCES_IN_SIZE );
                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_OUT_SIZE, _lces_cfg._LCES_REG.LCES_OUT_SIZE );
                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_LRZRX, _lces_cfg._LCES_REG.LCES_LRZRX );
                CAM_WRITE_REG(this->m_pDrv, LCES_R1_LCES_LRZRY, _lces_cfg._LCES_REG.LCES_LRZRY );
                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN, CAMCTL_LCES_R1_EN, 1);

                lcso.m_pIspDrv = this->m_pIspDrv;
                lcso.dma_cfg.size.xsize = CAM_READ_BITS(this->m_pDrv,LCES_R1_LCES_OUT_SIZE,LCES_OUT_WD)*2;
                lcso.dma_cfg.size.h = CAM_READ_BITS(this->m_pDrv,LCES_R1_LCES_OUT_SIZE,LCES_OUT_HT);
                lcso.dma_cfg.size.stride = _lces_cfg.u4LCESO_Stride;
                lcso.config();
                lcso.write2CQ(0);
                lcso.enable(NULL);
                bWdmaEn = MTRUE;
                this->m_pDrv->cqApbModule(CAM_ISP_LCS_SIZE);
                this->m_pDrv->cqApbModule(CAM_ISP_LCS_LRZR);
            }
            else{
                CAM_FUNC_ERR("lcs must be opened if notification is registered\n");
                ret = MFALSE;
            }

        }
        else {
            this->m_pDrv->cqNopModule(CAM_ISP_LCS_SIZE);
            this->m_pDrv->cqNopModule(CAM_ISP_LCS_LRZR);
        }

    }
    return ret;

}

MBOOL CAM_RAW_PIPE::setGSE(void)
{
    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;


    if (CAM_C == this->m_hwModule) {
        CAM_FUNC_DBG("Simple raw_c is not support GSE");
        return MTRUE;
    }

    if(this->m_pP1Tuning){
        LMV_SGG_CFG lmv_sgg_cfg;

        CAM_FUNC_DBG("CAM_RAW_PIPE::setGSE:\n");
        this->m_pP1Tuning->p1TuningNotify(NULL,(MVOID*)&lmv_sgg_cfg);
        CAM_FUNC_DBG("-\n");

        if(lmv_sgg_cfg.bSGG2_Bypass == 0){
            CAM_WRITE_REG(this->m_pDrv,GSE_R1_GSE_GSE_00,lmv_sgg_cfg.PGN);
            CAM_WRITE_REG(this->m_pDrv,GSE_R1_GSE_GSE_01,lmv_sgg_cfg.GMRC_1);
            CAM_WRITE_REG(this->m_pDrv,GSE_R1_GSE_GSE_02,lmv_sgg_cfg.GMRC_2);
            this->m_pDrv->cqApbModule(CAM_ISP_GSE_);
        }
        else{
            this->m_pDrv->cqNopModule(CAM_ISP_GSE_);
        }

        if(lmv_sgg_cfg.bSGG2_EN){
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GSE_R1_EN, 1);
        }
        else{
            CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_GSE_R1_EN, 0);
        }

    }

    return ret;
}

MBOOL CAM_RAW_PIPE::setRSS(MBOOL &bWdmaEn)
{
#define EN_APL_DATA 1 // for eis used rsso, apl data must be turned on

    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if (CAM_C == this->m_hwModule) {
        CAM_FUNC_DBG("Simple raw_c is not support RSS");
        return MTRUE;
    }

    if(this->m_pP1Tuning){
        RSS_CFG _rss_cfg;
        RSS_INPUT_INFO input;
        Header_RRZO fh_rrzo;
        MUINT32 tmp = 0, pixMode = 0;
        MUINT32 hds1_sel = CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_HDS_SEL);

        if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_RRZ_R1_EN) != 1){
            CAM_FUNC_ERR("RRZ must be opened in Bayer domain when RSS is needed\n");
            return MFALSE;
        }

        //only hds , bin information is not needed
        pixMode = CAM_READ_BITS(this->m_pDrv,HDS_R1_HDS_HDS_MODE,HDS_DS_MODE);

        if(!hds1_sel){ //rrz is not enabled. data is from tg
            input.tg_out_w = this->m_SrcSize.w;
            input.tg_out_h = this->m_SrcSize.h;
            input.rss_in_w = input.tg_out_w >> pixMode;
            input.rss_in_h = input.tg_out_h;
            input.rrz_out_w = input.tg_out_w;
            input.rrz_out_h = input.tg_out_h;
        } else { //from rrz
            tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_DST_SIZE,this->m_pDrv);
            input.tg_out_w = 0;
            input.tg_out_h = 0;
            input.rss_in_w = (tmp & 0xffff) >> pixMode;
            input.rss_in_h = (tmp>>16);
            input.rrz_out_w = (tmp & 0xffff);
            input.rrz_out_h = (tmp>>16);
        }

        input.rss_scale_up_factor = 100; //default:100, un-support scale up
        input.bYUVFmt = MFALSE;

        CAM_FUNC_DBG("%s:hds1_sel:%d, pixmode=%d, rssin.w:%d, rssin.h:%d, tg.w=%d, tg.h=%d, rrzout.w:%d rrzout.h:%d\n",\
            __PRETTY_FUNCTION__, hds1_sel, pixMode, \
            input.rss_in_w, input.rss_in_h, input.tg_out_w, input.tg_out_h, \
            input.rrz_out_w, input.rrz_out_h);

        // pre-config rss input image size
        _rss_cfg.cfg_rss_in_img = (input.rss_in_h << 16) | (input.rss_in_w);

        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input,(MVOID*)&_rss_cfg);

        if (_rss_cfg.bypassRSS == 0){
            if (_rss_cfg.enRSS){
                #define RSS_DRV_RZ_UINT (1048576) //2^20
                MUINT32 Mm1_H, Mm1_V, Nm1_H, Nm1_V;
                REG_RSS_R1_RSS_CONTROL ctrl;
                REG_RSS_R1_RSS_HORI_STEP hori_step;
                REG_RSS_R1_RSS_VERT_STEP vert_step;
                DMA_RSSO rsso;

                //rss ctrl
                ctrl.Bits.RSS_HORI_EN = _rss_cfg.cfg_rss_ctrl_hori_en & 0x1;
                ctrl.Bits.RSS_VERT_EN = _rss_cfg.cfg_rss_ctrl_vert_en & 0x1;
                ctrl.Bits.RSS_OUTPUT_WAIT_EN = _rss_cfg.cfg_rss_ctrl_output_wait_en & 0x1;
                ctrl.Bits.RSS_APL_EN = (EN_APL_DATA) ? 0x1 : 0x0;
                ctrl.Bits.RSS_VERT_FIRST = _rss_cfg.cfg_rss_ctrl_vert_first & 0x1;
                ctrl.Bits.RSS_HORI_TBL_SEL = _rss_cfg.cfg_rss_ctrl_hori_tbl_sel & 0x1F;
                ctrl.Bits.RSS_VERT_TBL_SEL = _rss_cfg.cfg_rss_ctrl_vert_tbl_sel & 0x1F;

                //rss step and ofst
                Mm1_H = (_rss_cfg.cfg_rss_in_img & 0xFFFF) - 1;
                Mm1_V = (_rss_cfg.cfg_rss_in_img >> 16) - 1;
                Nm1_H = (_rss_cfg.cfg_rss_out_img & 0xFFFF) - 1;
                Nm1_V = (_rss_cfg.cfg_rss_out_img >> 16) - 1;
                hori_step.Raw = (Mm1_H)?(MUINT32)((Nm1_H * RSS_DRV_RZ_UINT + Mm1_H - 1) / Mm1_H):0;
                vert_step.Raw = (Mm1_V)?(MUINT32)((Nm1_V * RSS_DRV_RZ_UINT + Mm1_V - 1) / Mm1_V):0;

                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_CONTROL, ctrl.Raw);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_IN_IMG, _rss_cfg.cfg_rss_in_img);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_OUT_IMG, _rss_cfg.cfg_rss_out_img);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_HORI_STEP, hori_step.Raw);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_VERT_STEP, vert_step.Raw);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_LUMA_HORI_INT_OFST, 0);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_LUMA_HORI_SUB_OFST, 0);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_LUMA_VERT_INT_OFST, 0);
                CAM_WRITE_REG(this->m_pDrv, RSS_R1_RSS_LUMA_VERT_SUB_OFST, 0);

                CAM_WRITE_BITS(this->m_pDrv, CAMCTL_R1_CAMCTL_EN2, CAMCTL_RSS_R1_EN, 1);

                rsso.m_pIspDrv = this->m_pIspDrv;

                rsso.dma_cfg.size.xsize = (_rss_cfg.cfg_rss_out_img & 0xFFFF);
                rsso.dma_cfg.size.h = (_rss_cfg.cfg_rss_out_img >> 16);
                rsso.dma_cfg.size.h += (EN_APL_DATA) ? 1 : 0; //+1 is for APL data that is appended to last line.
                rsso.dma_cfg.size.stride = (_rss_cfg.cfg_rss_out_img & 0xFFFF);

                rsso.config();
                rsso.write2CQ(0);
                rsso.enable(NULL);
                bWdmaEn = MTRUE;

                this->m_pDrv->cqApbModule(CAM_ISP_RSS_A_);
                #undef RSS_DRV_RZ_UINT
            }
            else{
                CAM_FUNC_ERR("rss must be opened if notification is registered\n");
                ret = MFALSE;
            }

        }
        else{
            this->m_pDrv->cqNopModule(CAM_ISP_RSS_A_);
        }
    }
    return ret;
}

MBOOL CAM_RAW_PIPE::setRRZ(void)
{
    MBOOL ret = MTRUE;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    /* per frame */
    if(this->m_pP1Tuning){

        BIN_INPUT_INFO  input;
        RRZ_REG_CFG     _rrz_cfg;
        CAM_RRZ_CTRL    Rrz;
        DMA_RRZO        Rrzo;
        capibility      CamInfo;
        tCAM_rst        rst;
        E_CamPixelMode  pixMode = ePixMode_NONE;

        this->m_input.Magic = CAM_READ_REG(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_MAGIC_NUM);
        _rrz_cfg.bRRZ_Bypass = MTRUE;

        CAM_FUNC_DBG("%s:tg:%dx%d,bin_en:%d_%d,bin_size=%dx%d_%dx%d,magic:%d\n",\
            __FUNCTION__, this->m_input.TgOut_W, this->m_input.TgOut_H, \
            this->m_CurBin, this->m_input.TarBin_EN, \
            this->m_input.CurBinOut_W, this->m_input.CurBinOut_H, \
            this->m_input.TarBinOut_W, this->m_input.TarBinOut_H,\
            this->m_input.Magic);

        //this->m_input update in CAM_RAW_PIPE::config
        this->m_pP1Tuning->p1TuningNotify((MVOID*)&this->m_input,(MVOID*)&_rrz_cfg);

        Rrz.rrz_in_roi.x        = _rrz_cfg.src_x;
        Rrz.rrz_in_roi.y        = _rrz_cfg.src_y;
        Rrz.rrz_in_roi.floatX   = 0;
        Rrz.rrz_in_roi.floatY   = 0;
        Rrz.rrz_in_roi.w        = _rrz_cfg.src_w;
        Rrz.rrz_in_roi.h        = _rrz_cfg.src_h;
        Rrz.rrz_out_size.w      = _rrz_cfg.tar_w;
        Rrz.rrz_out_size.h      = _rrz_cfg.tar_h;

        //update rrz rlb offset
        Rrz.rrz_rlb_offset = this->m_rrz_rlb_offset;

        CAM_FUNC_DBG("rrz out:0x%x_0x%x, roi:0x%x_0x%x_0x%x_0x%x, byapss:%d\n",\
            Rrz.rrz_out_size.w,\
            Rrz.rrz_out_size.h,\
            Rrz.rrz_in_roi.x,\
            Rrz.rrz_in_roi.y,\
            Rrz.rrz_in_roi.w,\
            Rrz.rrz_in_roi.h,\
            _rrz_cfg.bRRZ_Bypass);

        if (_rrz_cfg.bRRZ_Bypass){
            Header_RRZO fh_rrzo;
            MUINT32 tmp;
            if (this->m_CurBin != this->m_input.TarBin_EN){
                CAM_FUNC_WRN("bin_en has changed, but bypass rrz:%d_%d\n",
                    this->m_CurBin, this->m_input.TarBin_EN);
            }
            //copy rrz configration from RRZ Config by Enque
            this->m_input.CurRrzCfg.src_x = CAM_READ_REG(this->m_pDrv, RRZ_R1_RRZ_HORI_INT_OFST);
            this->m_input.CurRrzCfg.src_y = CAM_READ_REG(this->m_pDrv, RRZ_R1_RRZ_VERT_INT_OFST);
            tmp = fh_rrzo.GetRegInfo(Header_RRZO::E_RRZ_CRP_SIZE,this->m_pDrv);
            this->m_input.CurRrzCfg.src_w = tmp & 0xFFFF;
            this->m_input.CurRrzCfg.src_h = tmp >> 16;
            tmp = CAM_READ_REG(this->m_pDrv, RRZ_R1_RRZ_OUT_IMG);
            this->m_input.CurRrzCfg.tar_w = tmp & 0xFFFF;
            this->m_input.CurRrzCfg.tar_h = tmp >> 16;
        } else {
            REG_CAMCTL_R1_CAMCTL_FMT_SEL fmt_sel;

            //copy rrz configration from MW CB
            memcpy((void *) &this->m_input.CurRrzCfg, (void *) &_rrz_cfg, sizeof(RRZ_REG_CFG));

            //rrzo use no dmao cropping function, cropping function is depended on rrz
            Rrzo.dma_cfg.size.h = Rrz.rrz_out_size.h;
            Rrzo.dma_cfg.size.stride = CAM_READ_REG(this->m_pDrv,RRZO_R1_RRZO_STRIDE);

            //rrzo fmt
            fmt_sel.Raw = CAM_READ_REG(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL);
            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_UNKNOWN;
            if(fmt_sel.Bits.CAMCTL_RRZO_FG_MODE == RRZO_MODE_FG){
                if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2, CAMCTL_UFG_R1_EN)){
                    switch(fmt_sel.Bits.CAMCTL_RRZO_R1_FMT){
                        case RRZO_FMT_RAW8:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_UFO_FG_BAYER8;
                            break;
                        case RRZO_FMT_RAW10:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_UFO_FG_BAYER10;
                            break;
                        case RRZO_FMT_RAW12:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_UFO_FG_BAYER12;
                            break;
                        case RRZO_FMT_RAW14:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_UFO_FG_BAYER14;
                            break;
                        default:
                            break;
                    }
                } else {
                    switch(fmt_sel.Bits.CAMCTL_RRZO_R1_FMT){
                        case RRZO_FMT_RAW8:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER8;
                            break;
                        case RRZO_FMT_RAW10:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER10;
                            break;
                        case RRZO_FMT_RAW12:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER12;
                            break;
                        case RRZO_FMT_RAW14:
                            Rrzo.dma_cfg.lIspColorfmt = eImgFmt_FG_BAYER14;
                            break;
                        default:
                            break;
                    }
                }
            } else {
                CAM_FUNC_ERR("rrzo fmt(%d) is not supported\n", fmt_sel.Bits.CAMCTL_RRZO_R1_FMT);
                ret = MFALSE;
            }

            //mark, use original stride which is configured at init phase
            //DMARrzo.dma_cfg.size.stride = rrz_cfg.rrz_out_size.stride;
            pixMode = (E_CamPixelMode)this->getCurPixMode(_rrzo_);
            if (CamInfo.GetCapibility(
                    NSImageio::NSIspio::EPortIndex_RRZO,
                    ENPipeQueryCmd_X_BYTE,
                    NormalPipe_InputInfo((EImageFormat)Rrzo.dma_cfg.lIspColorfmt,
                                                                        Rrz.rrz_out_size.w,
                                                                        pixMode),
                    rst, E_CAM_UNKNOWNN) == MFALSE) {
                CAM_FUNC_ERR("rrzo size err(0x%x)\n",Rrz.rrz_out_size.w);
                ret = MFALSE;
            }
            Rrzo.dma_cfg.size.xsize      =  rst.xsize_byte[0];

            //update xsize cfg
            Rrzo.m_pIspDrv = this->m_pDrv;
            if(Rrzo.config()!=0){
                CAM_FUNC_ERR("rrzo config error\n");
                ret = MFALSE;
            }

            //update rrz cfg
            Rrz.m_pIspDrv = this->m_pDrv;
            if(Rrz.config() != 0){
                CAM_FUNC_ERR("rrz config error\n");
                ret = MFALSE;
            }

            //update ufgo dma cfg
            if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN, CAMCTL_UFGO_R1_EN)){
                DMA_UFGO Ufgo;

                Ufgo.dma_cfg.size.stride = CAM_READ_REG(this->m_pDrv,UFGO_R1_UFGO_STRIDE);
                Ufgo.dma_cfg.size.xsize = rst.xsize_byte[1];
                Ufgo.dma_cfg.size.h = Rrzo.dma_cfg.size.h;

                Ufgo.m_pIspDrv = this->m_pDrv;
                if(Ufgo.config()!=0){
                    CAM_FUNC_ERR("ufgo config error\n");
                    ret = MFALSE;
                }
            }

        }
    }
    return ret;
}

MBOOL CAM_RAW_PIPE::set3A(void)
{
    MBOOL ret = MTRUE;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    if(!this->m_NotifyFrame) {
        return ret;
    }

    if (this->m_pP1Tuning) {
        RectData<MUINT32> Bin;
        BIN_INPUT_INFO input;
        MUINT32 Qbn1Acc = ((CAM_C == this->m_hwModule) ? _1_pix_ : CAM_READ_BITS(this->m_pDrv,QBIN_R1_QBIN_CTL,QBIN_ACC));

        this->m_input.TarBin_EN = this->m_bBin;

        this->m_input.TgOut_W = this->m_SrcSize.w;
        this->m_input.TgOut_H = this->m_SrcSize.h;

        GET_FULL_BIN(this->m_pDrv, Bin, 2);
        this->m_input.TarBinOut_W = this->m_input.TgOut_W >> Bin.H;
        this->m_input.TarBinOut_H = this->m_input.TgOut_H >> Bin.V;

        this->m_input.TarQBNOut_W = this->m_input.TarBinOut_W >> Qbn1Acc;

        this->m_input.bSwitchDone = (this->m_NotifyFrame == 1) ? MTRUE : MFALSE;

        memcpy((void*)&input, (void*)&this->m_input, sizeof(BIN_INPUT_INFO));
        input.TgOut_W = (this->m_DataPat == eCAM_DUAL_PIX) ? (input.TgOut_W / 2) : (input.TgOut_W);

        CAM_FUNC_DBG("TgW:%d TgH:%d BinM:%d TarBinEn:%d TarBinW:%d TarBinH:%d CurBinW:%d CurBinH:%d Mag:%d TarQBNW:%d TarRMBW:%d CurQBNW:%d CurRMBW:%d",\
                input.TgOut_W, input.TgOut_H, input.Bin_MD, input.TarBin_EN, input.TarBinOut_W, input.TarBinOut_H, input.CurBinOut_W,\
                input.CurBinOut_H, input.Magic, input.TarQBNOut_W, input.TarRMBOut_W, input.CurQBNOut_W, input.CurRMBOut_W);

        this->m_pP1Tuning->p1TuningNotify((MVOID*)&input, (MVOID*)NULL);
    }
    else{
        CAM_FUNC_ERR("m_pP1Tuning is null\n");
    }

    return ret;
}

/* getCurPixMode() must be after setIMG_SEL*/
MUINT32 CAM_RAW_PIPE::getCurPixMode(_isp_dma_enum_ dmao)
{
    MUINT32 img_sel, imgo_sel, crp_r3_sel;
    MUINT32 PixMode = ePixMode_1;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;

    switch(dmao){
        case _imgo_:
            img_sel = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_IMG_SEL);
            imgo_sel = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_IMGO_SEL);
            crp_r3_sel = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_CRP_R3_SEL);
            switch(this->m_RawType){
                case E_FromTG:
                    {
                        if(imgo_sel == 2) {
                            return MAP_CAM_PIXMODE(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_TGO),CAM_FUNC_INF);
                        } else {
                            if(img_sel == 1) {
                                return MAP_CAM_PIXMODE(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL,CAMCTL_PIX_BUS_TGO),CAM_FUNC_INF);
                            } else {
                                MUINT32 raw_selo = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT2_SEL,CAMCTL_PIX_BUS_RAW_SELO);
                                MUINT32 dbn_en = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_DBN_R1_EN);
                                MUINT32 bin_en = CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_BIN_R1_EN);
                                if(crp_r3_sel == 0) {
                                    return MAP_CAM_PIXMODE(raw_selo,CAM_FUNC_INF);
                                } else if(crp_r3_sel == 6) {
                                    return MAP_CAM_PIXMODE((raw_selo>>dbn_en),CAM_FUNC_INF);
                                } else if(crp_r3_sel == 7) {
                                    return MAP_CAM_PIXMODE((raw_selo>>(dbn_en+bin_en)),CAM_FUNC_INF);
                                } else {
                                    return ePixMode_1;
                                }
                            }
                        }
                    }
                    break;
                case E_BeforLSC:
                    if (img_sel == 0)
                        CAM_FUNC_ERR("img_sel=0, not support RawType=%d\n", this->m_RawType);
                    return ePixMode_1;
                    break;
                case E_AFTERLSC:
                    if (img_sel == 0)
                        CAM_FUNC_ERR("img_sel=0, not support RawType=%d\n", this->m_RawType);
                    return ePixMode_1;
                    break;
                case E_BetweenLSCWB:
                    if (img_sel == 0)
                        CAM_FUNC_ERR("img_sel=0, not support RawType=%d\n", this->m_RawType);
                    return ePixMode_1;
                    break;
                default:
                    CAM_FUNC_ERR("Raw Data support only 0:from TG,1:before LSC,2:after LSC,3:between LSC and WB\n");
                    return MFALSE;
                    break;
            }
            break;
        case _rrzo_:
            if (CAM_C == this->m_hwModule){
                return ePixMode_1;
            }
            else{
                switch(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_FMT_SEL, CAMCTL_PIX_BUS_MRG_R7O)){
                    case 0:
                        return ePixMode_1;
                        break;
                    case 1:
                        return ePixMode_2;
                        break;
                    case 2:
                        return ePixMode_4;
                        break;
                    default:
                        CAM_FUNC_ERR("MRG_R7O setting error\n");
                        return MFALSE;
                        break;
                }
            }
            break;
        case _yuvo_:
        case _crzo_:
        case _crzbo_:
        case _crzo_r2_:
        case _crzbo_r2_:
            if (m_bDYUV) {
                return this->m_Dyuv.getCurPixMode(dmao);
            }
            else {
                CAM_FUNC_ERR("DYUV is not enable, can't get dmao(%d)",dmao);
                return ePixMode_1;
            }
            break;
        default:
            CAM_FUNC_ERR("unsupported dmao:0x%x,return unknown-pix\n",dmao);
            return ePixMode_1;
            break;
    }
}

MINT32 CAM_DPD_CTRL::_config( void )
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    E_PIX pixmode = _1_pix_;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    //Dual PD path is configured from statisticpipe not from twin_drv. Therefore need to reference TG pixel mode.
    pixmode = (E_PIX)(CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS) + \
        CAM_READ_BITS(this->m_pDrv->getPhyObj(),TG_R1_TG_SEN_MODE,TG_DBL_DATA_BUS1));

    this->m_dlp.SEPI = MAP_CAM_PIXMODE(pixmode,CAM_FUNC_INF);

    CAM_FUNC_DBG("CAM_DPD_CTRL::_config+ cq:0x%x,page:0x%x,tg_pix_mode:0x%x,cam source:0x%x\n",cq,page,\
            this->m_dlp.SEPI,\
            CAM_READ_BITS(this->m_pDrv->getPhyObj(),CAMCTL_R1_CAMCTL_SEL,CAMCTL_RAW_SEL));

    if(CAM_READ_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_PDO_SEL) == ePDOSel_0){
        switch(this->m_Density){
            case _level_1:
                XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PBN_R1_EN,1);
                XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_CRP_R8_EN,0);
                this->m_pDrv->cqNopModule(CAM_ISP_CRP_R8_);
                break;
            case _level_2:
                XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PBN_R1_EN,0);
                XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_CRP_R8_EN,1);//cropping is applied via tuning
                this->m_pDrv->cqApbModule(CAM_ISP_CRP_R8_);
                break;
            default:
                CAM_FUNC_ERR("unsupported Density:%d\n",this->m_Density);
                return 1;
                break;
        }
    }
    else{
        XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_CRP_R8_EN,0);
        XOR_WRITE_BITS(this->m_hwModule,this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PBN_R1_EN,0);
        this->m_pDrv->cqNopModule(CAM_ISP_CRP_R8_);
    }

    return 0;
}

MINT32 CAM_DPD_CTRL::_write2CQ(MUINT32 BurstIndex)
{
    if (BurstIndex > 0) {
        /*For SMVR, only add module at normal page*/
        goto EXIT;
    }

    this->m_pDrv->cqAddModule(CAM_ISP_CRP_R8_);
    this->m_pDrv->cqNopModule(CAM_ISP_CRP_R8_);

    //descriptor of psb/pbn is applied at tuning
EXIT:

    return 0;
}


MINT32 CAM_DPD_CTRL::_enable(void* pParam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    this->m_pdo_en[this->m_hwModule] = MTRUE;

    return 0;
}

MINT32 CAM_DPD_CTRL::_disable(void* pParam)
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    pParam;

    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    this->m_pdo_en[this->m_hwModule] = MFALSE;

    return 0;
}

MINT32 CAM_DPD_CTRL::PD_PATH_EN(MBOOL enable)
{
    MINT32 ret = 0;
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    //dbg log
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("PD_PATH_EN_%d cq:0x%x,page:0x%x\n",enable,cq,page);
    if(enable){
        if(this->m_pdo_en[this->m_hwModule]){
            //no need to enable pdi/pde, which is configured by tuning
            CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN,1);
            CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_PDO_R1,1);
            CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_FBC_EN,1);

            CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_runnable, this->m_hwModule,CAM_BUF_CTRL::eCamDmaType_stt);
        }
        else{
            if(CAM_BUF_CTRL::m_SttState[this->m_hwModule] == CAM_BUF_CTRL::eState_forPDOnly)//ref note1 below
                CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eCamState_stop, this->m_hwModule,CAM_BUF_CTRL::eCamDmaType_stt);
            else{
                ret = 1;
                CAM_FUNC_WRN("PDO is not enabled when configure, shouldn't enable PDO %d\n", CAM_BUF_CTRL::m_SttState[this->m_hwModule]);
            }
        }
    }
    else{
        //these reg will be configured by tuning
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDI_R1_EN,0);
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_EN2,CAMCTL_PDE_R1_EN,0);
        //
        CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_DMA_EN,CAMCTL_PDO_R1_EN,0);
        CAM_WRITE_BITS(this->m_pDrv,CAMDMATOP1_R1_CAMDMATOP1_DMA_FRAME_HEADER_EN1,CAMDMATOP1_FRAME_HEADER_EN_PDO_R1,0);
        CAM_WRITE_BITS(this->m_pDrv,FBC_R1_FBC_PDO_R1_CTL1,FBC_PDO_R1_FBC_EN,0);
        //note1:
        //enter pd suspend mode even if pd is not enabled for case :  pd-sen + raw_a | non-pd-sen + raw_c -> pd-sen + raw_c | non-pd-sen + raw_a
        //need to change raw_c's state for user to get dummy output.
        CAM_BUF_CTRL::updateState(CAM_BUF_CTRL::eState_forPDOnly, this->m_hwModule,CAM_BUF_CTRL::eCamDmaType_stt);
    }

    this->m_pDrv->cqAddModule(CAM_FBC_PDO_);

    return ret;
}


MINT32 STT_SEL_CTRL::_config()
{
    E_ISP_CAM_CQ cq;
    MUINT32 page;
    MINT32  ret = 0;
    this->m_pDrv = (ISP_DRV_CAM*)this->m_pIspDrv;
    this->m_pDrv->getCurObjInfo(&this->m_hwModule,&cq,&page);

    CAM_FUNC_DBG("STT_SEL_CTRL::_config+ cq:0x%x,page:0x%x\n",cq,page);

    for(MUINT32 i = 0; i < E_STT_SEL_MAX; i++){
        switch((E_STT_SEL)i){
            case E_FLK_SEL:
                if(this->m_SttSel[i] <= this->m_kSelMax[i]) {
                    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_FLK_SEL,this->m_SttSel[i]);
                }
                else {
                    CAM_FUNC_ERR("undefine Sel:%d\n",this->m_SttSel[i]);
                    ret = -1;
                }
                break;
            case E_CRP_R1_SEL:
                if(this->m_SttSel[i] <= this->m_kSelMax[i]) {
                    CAM_WRITE_BITS(this->m_pDrv,CAMCTL_R1_CAMCTL_SEL,CAMCTL_CRP_R1_SEL,this->m_SttSel[i]);
                }
                else {
                    CAM_FUNC_ERR("undefine Sel:%d\n",this->m_SttSel[i]);
                    ret = -1;
                }
                break;
            default:
                CAM_FUNC_ERR("unsupported Sel:%d\n",i);
                ret = -1;
                break;
        }
    }
    return ret;
}

